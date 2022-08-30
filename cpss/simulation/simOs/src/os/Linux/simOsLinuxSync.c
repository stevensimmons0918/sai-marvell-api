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
*       Linux Operating System Simulation. Semaphore facility.
*
* DEPENDENCIES:
*       CPU independed.
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*******************************************************************************/

#include <time.h>

#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include <gtOs/gtOsSem.h>
#include <os/simTypes.h>
#include <os/simTypesBind.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsSync.h>

#if 0
#include <vxw_hdrs.h>
#endif

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

    if (osSemCCreate(NULL, initCount, &returnSem) != GT_OK)
    {
        return 0;
    }

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
    osSemDelete(smid);

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
    if (timeOut == SIM_OS_WAIT_FOREVER)
        timeOut = OS_WAIT_FOREVER;

    return osSemWait(smid, timeOut);
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
    return osSemSignal(smid);
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
    GT_MUTEX returnMtx;

    if (osMutexCreate(NULL, &returnMtx) != GT_OK)
        return 0;
    return returnMtx;
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
    osMutexDelete(mid);

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
    osMutexUnlock(mid);
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
    osMutexLock(mid);
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
    GT_SEM returnSem;

    if (osSemBinCreate(NULL, OS_SEMB_EMPTY, &returnSem) != GT_OK)
    {
        return 0;
    }

    return returnSem;
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
    return osSemSignal(eventHandle);
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
        timeOut = OS_WAIT_FOREVER;

    return osSemWait(emid, timeOut);
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
}

