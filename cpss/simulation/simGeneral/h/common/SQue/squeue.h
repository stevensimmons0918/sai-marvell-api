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
* @file squeue.h
*
* @brief This is a API definition for SQueue module of the Simulation.
*
*
* @version   6
********************************************************************************
*/
#ifndef __squeueh
#define __squeueh

#include <os/simTypes.h>
#include <os/simTypesBind.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Typedef: struct SIM_BUFFER_STC
 *
 * Description:
 *      Describe the casting type of the buffers managed in the SQUEU lib
 *
 * Fields:
 *      magic           : Magic number for consistence check.
 *      nextBufPtr      : Pointer to the next buffer in the pool or queue.
 * Comments:
 */
typedef struct SIM_BUFFER_STCT{
    GT_U32  magic;
    struct SIM_BUFFER_STCT *nextBufPtr;
}SIM_BUFFER_STC;

typedef SIM_BUFFER_STC * SIM_BUFFER_ID;

/* Queue ID typedef */
typedef  void *     SQUE_QUEUE_ID;
/* API functions */

/**
* @internal squeInit function
* @endinternal
*
* @brief   Init Squeue library.
*/
void squeInit
(
    void
);

/**
* @internal squeCreate function
* @endinternal
*
* @brief   Create new queue.
*
* @retval SQUE_QUEUE_ID            - new queue ID
*
* @note In the case of memory lack the function aborts application.
*
*/
SQUE_QUEUE_ID squeCreate
(
    void
);

/**
* @internal squeDestroy function
* @endinternal
*
* @brief   Free memory allocated for queue.
*
* @param[in] queId                    - id of a queue.
*/
void squeDestroy
(
    IN  SQUE_QUEUE_ID    queId
);
/**
* @internal squeBufPut function
* @endinternal
*
* @brief   Put SBuf buffer to a queue.
*
* @param[in] queId                    - id of queue.
* @param[in] bufId                    - id of buffer.
*/
void squeBufPut
(
    IN  SQUE_QUEUE_ID    queId,
    IN  SIM_BUFFER_ID    bufId
);
/**
* @internal squeStatusGet function
* @endinternal
*
* @brief   Get queue's status
*
* @param[in] queId                    - id of queue.
*
* @retval 0                        - queue is empty
* @retval other value              - queue is not empty , or queue is suspended
*/
GT_U32 squeStatusGet
(
    IN  SQUE_QUEUE_ID    queId
);

/**
* @internal squeBufGet function
* @endinternal
*
* @brief   Get Sbuf from a queue, if no buffers wait for it forever.
*
* @param[in] queId                    - id of queue.
*
* @retval SBUF_BUF_ID              - buffer id
*/
SIM_BUFFER_ID squeBufGet
(
    IN  SQUE_QUEUE_ID    queId
);

/**
* @internal squeSuspend function
* @endinternal
*
* @brief   Suspend a queue queue. (cause 'squeBufPut' to put inside the queue
*         message that will be ignored instead of the original buffer)
* @param[in] queId                    - id of queue.
*/
void squeSuspend
(
    IN  SQUE_QUEUE_ID    queId
);

/**
* @internal squeResume function
* @endinternal
*
* @brief   squeResume a queue that was suspended by squeSuspend or by squeBufPutAndQueueSuspend.
*         (allow 'squeBufPut' to put buffers into the queue)
* @param[in] queId                    - id of queue.
*/
void squeResume
(
    IN  SQUE_QUEUE_ID    queId
);

/**
* @internal squeBufPutAndQueueSuspend function
* @endinternal
*
* @brief   Put SBuf buffer to a queue and then 'suspend' the queue (for any next buffers)
*
* @param[in] queId                    - id of queue.
* @param[in] bufId                    - id of buffer.
*/
void squeBufPutAndQueueSuspend
(
    IN  SQUE_QUEUE_ID    queId,
    IN  SIM_BUFFER_ID    bufId
);

/**
* @internal squeFlush function
* @endinternal
*
* @brief   flush all messages that are in the queue.
*         NOTE: operation valid only if queue is suspended !!!
* @param[in] queId                    - id of queue.
*
* @note this function not free the buffers ... for that use sbufPoolFlush(...)
*
*/
void squeFlush
(
    IN  SQUE_QUEUE_ID    queId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __squeueh */



