/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file simLogTask.c
*
* @brief Log file writer task. Required to write log from different processes
* (SHARED LIBRARY)
*
* @version   1
********************************************************************************
*/
#include <os/simTypesBind.h>
#include <asicSimulation/SLog/simLog.h>
#include <stdio.h>
#include <string.h>

#ifdef SHARED_MEMORY

#define SIM_LOG_QUEUE_LEN 10
#define SIM_LOG_MSG_SIZE 512

/* osMsgQ */
#define OS_MSGQ_WAIT_FOREVER 0
#define OS_MSGQ_NO_WAIT      0xffffffff
typedef GT_UINTPTR GT_MSGQ_ID;
GT_STATUS CPSS_osMsgQCreate
(
    IN  const char    *name,
    IN  GT_U32        maxMsgs,
    IN  GT_U32        maxMsgSize,
    OUT GT_MSGQ_ID    *msgqId
);
GT_STATUS CPSS_osMsgQDelete
(
    IN GT_MSGQ_ID msgqId
);
GT_STATUS CPSS_osMsgQSend
(
    IN GT_MSGQ_ID   msgqId,
    IN GT_PTR       message,
    IN GT_U32       messageSize,
    IN GT_U32       timeOut
);
GT_STATUS CPSS_osMsgQRecv
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_PTR       message,
    INOUT GT_U32       *messageSize,
    IN    GT_U32       timeOut
);



/****************** private vars **********************/
/* logger file pointer */
static GT_MSGQ_ID loggerMsgQ = 0;
static int loggerFileOpened = 0;
#endif /* SHARED_MEMORY */
static FILE* loggerFilePointer = NULL;


#ifdef SHARED_MEMORY
/*******************************************************************************
* simLogTask
*
* DESCRIPTION:
*       Logger task. This task handle log file events -
*          0 - open file
*          1 - close file
*          2 - write data to file
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       None.
*
* COMMENTS:
*       None.
*******************************************************************************/
static void __TASKCONV simLogTask(void)
{
    GT_STATUS rc;
    char    msg[SIM_LOG_MSG_SIZE];
    GT_U32  msgSize;

    for (;;)
    {
        msgSize = sizeof(msg);
        rc = CPSS_osMsgQRecv(loggerMsgQ, msg, &msgSize, OS_MSGQ_WAIT_FOREVER);
        if (rc != GT_OK && rc != GT_TIMEOUT)
        {
            break;
        }
        if (rc != GT_OK)
        {
            continue; /* break??? */
        }
        /* handle msg */
        if (msg[0] == 0 || msg[0] == 1) /* open || close */
        {
            if (loggerFilePointer != NULL)
            {
                fclose(loggerFilePointer);
                loggerFilePointer = NULL;
            }
        }
        if (msg[0] == 0) /* open file */
        {
            msg[msgSize] = 0;
            loggerFilePointer = fopen(msg+1, "wt");
            if (NULL == loggerFilePointer)
            {
                printf("SIMULATION ERROR: failed to open LOG file: [%s] \n", msg+1);
            }
            else
            {
                setbuf(loggerFilePointer, NULL);
            }
        }
        if (msg[0] == 2 && loggerFilePointer != NULL)
        {
            fwrite(msg+1, msgSize-1, 1, loggerFilePointer);
        }
    }
}
#endif /* SHARED_MEMORY */

/**
* @internal simLogTaskInit function
* @endinternal
*
* @brief   Initialize logger task. This task will handle all writes to log file
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS simLogTaskInit(void)
{
#ifdef SHARED_MEMORY
    GT_STATUS ret;
    GT_TASK_HANDLE taskHandl;
    static int initialized = 0;

    if (initialized)
        return GT_OK;
    ret = CPSS_osMsgQCreate("simLogTaskQ",
            SIM_LOG_QUEUE_LEN, SIM_LOG_MSG_SIZE,
            &loggerMsgQ);
    if (ret != GT_OK)
        return GT_FAIL;

    taskHandl = SIM_OS_MAC(simOsTaskCreate)(GT_TASK_PRIORITY_NORMAL,
            (unsigned (__TASKCONV *)(void*))simLogTask, NULL);
    if (taskHandl == NULL)
    {
        CPSS_osMsgQDelete(loggerMsgQ);
        return GT_FAIL;
    }
    initialized = 1;
#endif /* SHARED_MEMORY */
    return GT_OK;
}

/**
* @internal simLogTaskOpenFile function
* @endinternal
*
* @brief   Open log file
*
* @param[in] fname                    - File name
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS simLogTaskOpenFile(
    IN const char *fname
)
{
#ifdef SHARED_MEMORY
    char msg[SIM_LOG_MSG_SIZE];
    GT_U32 msgLen;
    GT_STATUS rc;

    msg[0] = 0; /* open file */
    strcpy(msg + 1, fname);
    msgLen = strlen(fname) + 1;
    rc = CPSS_osMsgQSend(loggerMsgQ, msg, msgLen, OS_MSGQ_WAIT_FOREVER);
    if (rc == GT_OK)
    {
        loggerFileOpened = 1;
        simLogIsOpenFlag = simLogIsOpen();
    }
    return rc;
#else /* !defined(SHARED_MEMORY) */
    if (loggerFilePointer != NULL)
    {
        fclose(loggerFilePointer);
    }
    loggerFilePointer = fopen(fname, "wt");
    simLogIsOpenFlag = simLogIsOpen();
    if (NULL == loggerFilePointer)
    {
        printf("SIMULATION ERROR: failed to open LOG file: [%s] \n", fname);
        return GT_FAIL;
    }
    setbuf(loggerFilePointer, NULL);
    return GT_OK;
#endif /* !defined(SHARED_MEMORY) */
}

/**
* @internal simLogTaskFileOpened function
* @endinternal
*
* @brief   Check if log file opened
*
* @retval GT_TRUE                  - if log file opened
* @retval GT_FAIL                  - if log file not opened
*/
GT_BOOL simLogTaskFileOpened(void)
{
#ifdef SHARED_MEMORY
    return (loggerFileOpened == 1) ? GT_TRUE : GT_FALSE;
#else /* !defined(SHARED_MEMORY) */
    return (loggerFilePointer == NULL) ? GT_FALSE : GT_TRUE;
#endif /* !defined(SHARED_MEMORY) */
}

/**
* @internal simLogTaskCloseFile function
* @endinternal
*
* @brief   Close log file
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS simLogTaskCloseFile(void)
{
#ifdef SHARED_MEMORY
    char msg[1];
    GT_STATUS rc;

    msg[0] = 1; /* close file */
    rc = CPSS_osMsgQSend(loggerMsgQ, msg, 1, OS_MSGQ_WAIT_FOREVER);
    if (rc == GT_OK)
    {
        loggerFileOpened = 0;
        simLogIsOpenFlag = simLogIsOpen();
    }
    return rc;
#else /* !defined(SHARED_MEMORY) */
    if (loggerFilePointer != NULL)
    {
        fclose(loggerFilePointer);
    }
    loggerFilePointer = NULL;
    simLogIsOpenFlag = simLogIsOpen();
    return GT_OK;
#endif /* !defined(SHARED_MEMORY) */
}

/**
* @internal simLogTaskAddLog function
* @endinternal
*
* @brief   Write string to log file
*
* @param[in] str                      - string to write
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS simLogTaskAddLog(
    IN const char *str
)
{
#ifdef SHARED_MEMORY
    int len, alen;
    char msg[SIM_LOG_MSG_SIZE];
    GT_STATUS rc = GT_OK;

    if (!loggerFileOpened)
        return GT_OK;
    len = strlen(str);
    while (len > 0)
    {
        msg[0] = 2; /* message */
        alen = (len > SIM_LOG_MSG_SIZE-1)
            ? SIM_LOG_MSG_SIZE-1
            : len;

        strncpy(msg+1, str, alen);
        str += alen;
        len -= alen;
        rc = CPSS_osMsgQSend(loggerMsgQ, msg, alen + 1, OS_MSGQ_WAIT_FOREVER);
        if (rc != GT_OK)
            break;
    }
    return rc;
#else /* !defined(SHARED_MEMORY) */
    if (loggerFilePointer == NULL)
        return GT_OK;
    fprintf(loggerFilePointer,"%s",str);

    /* allow to trace the LOG also on external mechanism */
    /* may be needed in ASIM system                      */
    scibRemoteTraceInfo(str);

    return GT_OK;
#endif /* !defined(SHARED_MEMORY) */
}

