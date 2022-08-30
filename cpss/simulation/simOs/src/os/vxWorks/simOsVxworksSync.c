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

/*******************************************************************************
* simOsWin32Sync.c
*
* DESCRIPTION:
*       Win32 Operating System Simulation. Semaphore facility.
*
* DEPENDENCIES:
*       Win32, CPU independed.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#include <vxWorks.h>
#include <objLib.h>
#include <semLib.h>
#include <sysLib.h>


#include <os/simTypes.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <asicSimulation/SInit/sinit.h>
#include <os/simOsSync.h>

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

    returnSem = (GT_SEM)semCCreate(SEM_Q_FIFO,initCount);

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
    semDelete((SEM_ID)smid);

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
    STATUS rc;

    if (timeOut == 0)
        rc = semTake ((SEM_ID) smid, WAIT_FOREVER);
    else
    {
        int num, delay;

        num = sysClkRateGet();
        delay = (num * timeOut) / 1000;
        if (delay < 1)
            rc = semTake ((SEM_ID) smid, 1);
        else
            rc = semTake ((SEM_ID) smid, delay);
    }

    if (rc != OK)
    {
        if (errno == S_objLib_OBJ_TIMEOUT)
            return GT_TIMEOUT;
        else
            return GT_FAIL;
    }

    return GT_OK;
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
    if (semGive((SEM_ID)smid) == 0)
        return GT_OK;

    return GT_FAIL;
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

    returnSem = (GT_MUTEX)semMCreate (SEM_Q_FIFO);

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
    semDelete((SEM_ID)mid);
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
    semGive((SEM_ID)mid);
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
    semTake((SEM_ID)mid, WAIT_FOREVER);
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
    FUNCTION_NOT_SUPPORTED(simOsEventCreate);

    return NULL;
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
    FUNCTION_NOT_SUPPORTED(simOsEventSet);

    return GT_NOT_SUPPORTED;
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
    FUNCTION_NOT_SUPPORTED(simOsEventWait);

    return GT_NOT_SUPPORTED;
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
    FUNCTION_NOT_SUPPORTED(simOsSendDataToVisualAsic);
}

