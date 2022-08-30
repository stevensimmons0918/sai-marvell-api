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
* @file simOsWin32Sync.c
*
* @brief Win32 Operating System Simulation. Semaphore facility.
*
* @version   4
********************************************************************************
*/

/* WA to avoid next warning :
   due to include to : #include <windows.h>
    c:\program files\microsoft visual studio\vc98\include\rpcasync.h(45) :
    warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
*/
struct _RPC_ASYNC_STATE;

#include <windows.h>
#include <process.h>
#include <time.h>

#include <stdio.h>
#include <assert.h>

#include <os/simTypesBind.h>

#define SIM_OS_STR_NAMED_PIPE_CNS  \
   "\\\\.\\pipe\\PipeLineBetweenWhiteModeAndVisualAsic"

/************ Public Functions ************************************************/

/**
* @internal simOsSemCreate function
* @endinternal
*
* @brief   Create and initialize a binary semaphore.
*
* @retval GT_SEM                   - semaphor id
*/

GT_SEM  simOsSemCreate
(
    IN  GT_U32           initCount,
    IN  GT_U32           maxCount
)
{
    GT_SEM returnSem;

    returnSem = (GT_SEM)CreateSemaphore(NULL,       /* no security attributes */
                          (unsigned long)initCount, /* initial count          */
                          (unsigned long)maxCount,  /* maximum count          */
                           NULL);                   /* semaphore name         */

    return returnSem;
}

/**
* @internal simOsSemDelete function
* @endinternal
*
* @brief   Delete semaphore.
*
* @param[in] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS simOsSemDelete
(
    IN GT_SEM smid
)
{
    if (CloseHandle((HANDLE)smid) == 0)
      return GT_FAIL;

    return GT_OK;
}

/**
* @internal simOsSemWait function
* @endinternal
*
* @brief   Wait on semaphore.
*
* @param[in] smid                     - semaphore Id
* @param[in] timeOut                  - time out in milliseconds or 0 to wait forever
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval OS_TIMEOUT               - on time out
*/
GT_STATUS simOsSemWait
(
    IN GT_SEM smid,
    IN GT_U32 timeOut
)
{
    if (timeOut == 0)
        timeOut = INFINITE;

    switch (WaitForSingleObject((HANDLE)smid, timeOut))
    {
        case WAIT_FAILED:
            return GT_FAIL;

        case WAIT_TIMEOUT:
            return GT_TIMEOUT;

        case WAIT_OBJECT_0:
            return GT_OK;

        default: break;
    }

  return GT_FAIL;
}

/**
* @internal simOsSemSignal function
* @endinternal
*
* @brief   Signal a semaphore.
*
* @param[in] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS simOsSemSignal
(
    IN GT_SEM smid
)
{
    ReleaseSemaphore((HANDLE)smid,1,NULL);

    return GT_OK;
}
/**
* @internal simOsMutexCreate function
* @endinternal
*
* @brief   Create and initialize mutex (critical section).
*         This object is recursive: the owner task can lock it again without
*         wait. simOsMutexUnlock() must be called for every time that mutex
*         successfully locked
*
* @retval GT_SEM                   - mutex id
*/
GT_MUTEX  simOsMutexCreate
(
    void
)
{
    GT_MUTEX returnSem;

    returnSem = (GT_MUTEX)CreateMutex(NULL, /* no security attributes */
                            FALSE,        /* bInitialOwner          */
                            NULL);        /* semaphore name         */
    return returnSem;
}
/**
* @internal simOsMutexDelete function
* @endinternal
*
* @brief   Delete mutex.
*
* @param[in] mid                      - mutex id
*                                       None
*/
void simOsMutexDelete
(
    IN GT_MUTEX mid
)
{
    simOsSemDelete(mid);
}
/**
* @internal simOsMutexUnlock function
* @endinternal
*
* @brief   Leave critical section.
*
* @param[in] mid                      - mutex id
*                                       None
*/
void simOsMutexUnlock
(
    IN GT_MUTEX mid
)
{
    ReleaseMutex((HANDLE)mid);
}
/**
* @internal simOsMutexLock function
* @endinternal
*
* @brief   Enter a critical section.
*
* @param[in] mid                      - mutex id
*                                       None
*/
void simOsMutexLock
(
    IN GT_MUTEX mid
)
{
    simOsSemWait(mid, 0 /* wait forever */);
}

/**
* @internal simOsEventCreate function
* @endinternal
*
* @brief   Create an event.
*
* @retval GT_SEM                   - event id
*/
GT_SEM  simOsEventCreate
(
    void
)
{
    GT_SEM retEvent;
    retEvent = (GT_SEM)(CreateEvent(NULL,FALSE,FALSE,NULL));
    return retEvent;
}

/**
* @internal simOsEventSet function
* @endinternal
*
* @brief   Signal that the event was initialized.
*
* @param[in] eventHandle              - Handle of the event
*
* @retval GT_SEM                   - event id
*/
GT_STATUS  simOsEventSet
(
    IN GT_SEM eventHandle
)
{
    if(SetEvent((HANDLE)eventHandle) == 0)
    {
        return GT_FALSE;
    }
    return GT_OK;
}

/**
* @internal simOsEventWait function
* @endinternal
*
* @brief   Wait on event.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on time out
*/
GT_STATUS simOsEventWait
(
    IN GT_SEM emid,
    IN GT_U32 timeOut
)
{

    if (timeOut == SIM_OS_WAIT_FOREVER)
        timeOut = INFINITE;

    switch (WaitForSingleObject((HANDLE)emid, timeOut))
    {
        case WAIT_FAILED:
            return GT_FAIL;

        case WAIT_TIMEOUT:
            return GT_TIMEOUT;

        case WAIT_OBJECT_0:
            return GT_OK;

        default: break;
    }

    return GT_FAIL;

}

/**
* @internal simOsSendDataToVisualAsic function
* @endinternal
*
* @brief   Connects to a message-type pipe and writes to it.
*
* @param[in] bufferPtr                - pointer to the data buffer
* @param[in] bufferLen                - buffer length
*                                       None
*/
void simOsSendDataToVisualAsic
(
    IN void **bufferPtr,
    IN GT_U32 bufferLen
)
{

    CallNamedPipe(SIM_OS_STR_NAMED_PIPE_CNS, bufferPtr, bufferLen, NULL,
                  0,NULL, NMPWAIT_NOWAIT);
}


