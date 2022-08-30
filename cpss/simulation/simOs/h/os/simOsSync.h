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
* @file simOsSync.h
*
* @brief Operating System wrapper. Semaphore facility.
*
* @version   2
********************************************************************************
*/

#ifndef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #error "include to those H files should be only for bind purposes"
#endif /*!EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES*/

#ifndef __simOsSynch
#define __simOsSynch
/************* Includes *******************************************************/



#ifdef __cplusplus
extern "C" {
#endif


/************* Functions ******************************************************/

/**
* @internal simOsSemCreate function
* @endinternal
*
* @brief   Create and initialize universal semaphore.
*
* @retval GT_SEM                   - semaphor id
*/
GT_SEM  simOsSemCreate
(
    IN  GT_U32           initCount,
    IN  GT_U32           maxCount
);


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
);

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
* @retval GT_TIMEOUT               - on time out
*/
GT_STATUS simOsSemWait
(
    IN GT_SEM smid,
    IN GT_U32 timeOut
);

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
);

/************* Functions ******************************************************/

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
);


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
);

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
);

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
);
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
);
/**
* @internal simOsEventSet function
* @endinternal
*
* @brief   Signal that the event was initialized.
*
* @param[in] eventId                  - Id of the event
*
* @retval GT_SEM                   - event id
*/
GT_STATUS  simOsEventSet
(
    IN GT_SEM eventId
);

/**
* @internal simOsEventWait function
* @endinternal
*
* @brief   Wait on event.
*
* @param[in] eventId                  - event Id
* @param[in] timeOut                  - time out in milliseconds or 0 to wait forever
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on time out
*/
GT_STATUS simOsEventWait
(
    IN GT_SEM eventId,
    IN GT_U32 timeOut
);

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
);

/**
* @internal simOsTime function
* @endinternal
*
* @brief   Gets number of seconds passed since system boot
*/
GT_U32 simOsTime(void);

#ifdef __cplusplus
}
#endif

#endif  /* __simOsSynch */



