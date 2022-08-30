/**
********************************************************************************
* @file lmsgqLib.c
*
* @brief pthread implementation of message queues
*
* @version   3
********************************************************************************
*/
/*******************************************************************************
* lmsgqLib.c
*
* DESCRIPTION:
*       pthread implementation of message queues
*
* DEPENDENCIES:
*       CPU independed , and posix threads implementation.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#define _GNU_SOURCE
#include <gtOs/gtOsMsgQ.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>

#include <gtOs/osObjIdLib.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/linuxLmsgqLib.h>
#include "mainOsPthread.h"

/************* Defines ********************************************************/

/* enable statistic collection for userspace mutexes */
#undef OS_MESSAGE_QUEUES_STAT

/************ Internal Typedefs ***********************************************/
#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLinuxPthrLsmggLibSrc

/************ Implementation defines ******************************************/
#define MSGQ_CHECK() \
    OS_MSGQ_STC   *q; \
    if (msgqId < 1 || msgqId >= (GT_UINTPTR)PRV_SHARED_DB.mqList.list.allocated) \
        return GT_FAIL; \
    q = (OS_MSGQ_STC*)(PRV_SHARED_DB.mqList.list.list[msgqId]); \
    if (!q || !q->header.type) \
        return GT_FAIL;

/************ Public Functions ************************************************/

/**
* @internal V2L_lmsgQInit function
* @endinternal
*
* @brief   Initialize message queues
*
* @retval OK                       - on success
* @retval ERROR                    - on error
*
* @note This function called from osStartEngine()
*
*/
int V2L_lmsgQInit(void)
{
#ifdef SHARED_MEMORY
    if (!PRV_SHARED_DB.initialized)
    {
        CREATE_MTX(&PRV_SHARED_DB.mqList.mutex);
        PRV_SHARED_DB.initialized = 1;
    }
#endif
    return 0;
}

/**
* @internal CPSS_osMsgQCreate function
* @endinternal
*
* @brief   Create and initialize a message queue.
*
* @param[in] name                     - message queue name
* @param[in] maxMsgs                  - max messages in queue
* @param[in] maxMsgSize               - max length of single message
*
* @param[out] msgqId                   - message queue id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS CPSS_osMsgQCreate
(
    IN  const char    *name,
    IN  GT_U32        maxMsgs,
    IN  GT_U32        maxMsgSize,
    OUT GT_MSGQ_ID    *msgqId
)
{
    int qnum;
    OS_OBJECT_HEADER_STC *h = NULL;
    OS_MSGQ_STC *q = NULL;

    DO_LOCK_MUTEX(&PRV_SHARED_DB.mqList.mutex);
#ifdef SHARED_MEMORY
    qnum = osObjLibGetNewObject(&PRV_SHARED_DB.mqList.list, 1, name, &h,V2L_ts_malloc,V2L_ts_free);
#else
    qnum = osObjLibGetNewObject(&PRV_SHARED_DB.mqList.list, 1, name, &h);
#endif
    if (qnum <= 0 || h == NULL)
    {
ret_fail:
        pthread_mutex_unlock(&PRV_SHARED_DB.mqList.mutex);
        return GT_FAIL;
    }


    /* align max message size by 4 bytes */
    q = (OS_MSGQ_STC*)h;
    maxMsgSize = (maxMsgSize+3) & ~3;
    q->buffer = (char*)V2L_ts_malloc((maxMsgSize + sizeof(int))*maxMsgs);
    if (q->buffer == NULL)
    {
        q->header.type = 0;
        goto ret_fail;
    }
    CREATE_MTX(&q->mtx);
    CREATE_COND(&q->rxSem);
    CREATE_COND(&q->txSem);

    q->maxMsgs = maxMsgs;
    q->maxMsgSize = maxMsgSize;
    pthread_mutex_unlock(&PRV_SHARED_DB.mqList.mutex);

    *msgqId = (GT_MSGQ_ID)qnum;
    return GT_OK;
}

/**
* @internal CPSS_osMsgQDelete function
* @endinternal
*
* @brief   Delete message queue
*
* @param[in] msgqId                   - message queue Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS CPSS_osMsgQDelete
(
    IN GT_MSGQ_ID msgqId
)
{
    int timeOut;

    MSGQ_CHECK();

    DO_LOCK_MUTEX(&q->mtx);
    q->header.type = 2; /* deleting */
    pthread_mutex_unlock(&q->mtx);

    for (timeOut = 1000; q->waitRx && timeOut; timeOut--)
    {
        pthread_cond_broadcast(&q->rxSem);
        sched_yield();
    }
    for (timeOut = 1000; q->waitTx && timeOut; timeOut--)
    {
        pthread_cond_broadcast(&q->txSem);
        sched_yield();
    }
    V2L_ts_free(q->buffer);
    pthread_mutex_destroy(&q->mtx);
    pthread_cond_destroy(&q->rxSem);
    pthread_cond_destroy(&q->txSem);

    q->header.type = 0;

    return GT_OK;
}

/**
* @internal CPSS_osMsgQSend function
* @endinternal
*
* @brief   Send message to queue
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
* @param[in] message                  -  data pointer
* @param[in] messageSize              - message size
* @param[in] timeOut                  - time out in miliseconds or
*                                      OS_MSGQ_WAIT_FOREVER or OS_MSGQ_NO_WAIT
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on time out
*/
GT_STATUS CPSS_osMsgQSend
(
    IN GT_MSGQ_ID   msgqId,
    IN GT_PTR       message,
    IN GT_U32       messageSize,
    IN GT_U32       timeOut
)
{
    char    *msg;
    struct timespec ts;

    MSGQ_CHECK();

    if (timeOut != OS_MSGQ_NO_WAIT && timeOut != OS_MSGQ_WAIT_FOREVER)
    {
        struct timespec delta;
        milliseconds2timespec(timeOut, &delta);
        clock_gettime(CPSS_CLOCK, &ts);
        timespec_add(&ts, &delta);
    }

    DO_LOCK_MUTEX(&q->mtx);
    while (q->messages == q->maxMsgs)
    {
        /* queue full */
        if (timeOut == OS_MSGQ_NO_WAIT)
        {
            pthread_mutex_unlock(&q->mtx);
            return GT_FAIL; /* ??? GT_TIMEOUT */
        }
        q->waitTx++;
        if (timeOut == OS_MSGQ_WAIT_FOREVER)
        {
            DO_COND_WAIT(&q->txSem, &q->mtx);
        }
        else
        {
            if (DO_COND_TIMEDWAIT(&q->txSem, &q->mtx, &ts) != 0)
            {
                /* timeout */
                pthread_mutex_unlock(&q->mtx);
                return GT_TIMEOUT;
            }
        }
        q->waitTx--;
        if (q->header.type != 1)
        {
            /* deleting */
            pthread_mutex_unlock(&q->mtx);
            return GT_FAIL;
        }
    }

    /* put message */
    msg = q->buffer + q->head * (q->maxMsgSize + sizeof(GT_U32));
    if (messageSize > (GT_U32)q->maxMsgSize)
        messageSize = q->maxMsgSize;

    *((GT_U32*)msg) = messageSize;
    memcpy(msg+sizeof(GT_U32), message, messageSize);
    q->head++;
    if (q->head >= q->maxMsgs) /* round up */
        q->head = 0;
    q->messages++;

    /* signal to Recv thread if any */
    if (q->waitRx)
    {
        pthread_cond_signal(&q->rxSem);
    }

    pthread_mutex_unlock(&q->mtx);
    return GT_OK;
}

/**
* @internal osMsgQRecv function
* @endinternal
*
* @brief   Receive message from queuee
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
* @param[in,out] messageSize              - size of buffer pointed by message
* @param[in] timeOut                  - time out in miliseconds or
*                                      OS_MSGQ_WAIT_FOREVER or OS_MSGQ_NO_WAIT
*
* @param[out] message                  -  data pointer
* @param[in,out] messageSize              - actual message size
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on time out
*/
GT_STATUS osMsgQRecv
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_PTR       message,
    INOUT GT_U32       *messageSize,
    IN    GT_U32       timeOut
)
{
    char    *msg;
    GT_U32  msgSize;
    struct timespec ts;

    MSGQ_CHECK();

    if (timeOut != OS_MSGQ_NO_WAIT && timeOut != OS_MSGQ_WAIT_FOREVER)
    {
        struct timespec delta;
        milliseconds2timespec(timeOut, &delta);
        clock_gettime(CPSS_CLOCK, &ts);
        timespec_add(&ts, &delta);
    }

    DO_LOCK_MUTEX(&q->mtx);
    while (q->messages == 0)
    {
        /* queue empty */
        if (timeOut == OS_MSGQ_NO_WAIT)
        {
            pthread_mutex_unlock(&q->mtx);
            return GT_FAIL; /* ??? GT_TIMEOUT */
        }
        q->waitRx++;
        if (timeOut == OS_MSGQ_WAIT_FOREVER)
        {
            DO_COND_WAIT(&q->rxSem, &q->mtx);
        }
        else
        {
            if (DO_COND_TIMEDWAIT(&q->rxSem, &q->mtx, &ts) != 0)
            {
                /* timeout */
                pthread_mutex_unlock(&q->mtx);
                return GT_TIMEOUT;
            }
        }
        q->waitRx--;
        if (q->header.type != 1)
        {
            /* deleting */
            pthread_mutex_unlock(&q->mtx);
            return GT_FAIL;
        }
    }
    /* get message */
    msg = q->buffer + q->tail * (q->maxMsgSize + sizeof(GT_U32));
    msgSize = *((GT_U32*)msg);
    if (msgSize > *messageSize)
        msgSize = *messageSize;

    memcpy(message, msg+sizeof(GT_U32), msgSize);
    *messageSize = msgSize;
    q->tail++;
    if (q->tail >= q->maxMsgs) /* round up */
        q->tail = 0;
    q->messages--;

    /* signal to Recv thread if any */
    if (q->waitTx)
    {
        pthread_cond_signal(&q->txSem);
    }

    pthread_mutex_unlock(&q->mtx);
    return GT_OK;
}

/**
* @internal CPSS_osMsgQNumMsgs function
* @endinternal
*
* @brief   Return number of messages pending in queue
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
*
* @param[out] numMessages              - number of messages pending in queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS CPSS_osMsgQNumMsgs
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_U32       *numMessages
)
{

    MSGQ_CHECK();

    DO_LOCK_MUTEX(&q->mtx);
    *numMessages = q->messages;
    pthread_mutex_unlock(&q->mtx);

    return GT_OK;
}

