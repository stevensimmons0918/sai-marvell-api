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
* @file smemPipe.h
*
* @brief Pipe memory mapping implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __smemPipeh
#define __smemPipeh

#include <asicSimulation/SKernel/smem/smem.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PPG_OFFSET 0x80000
#define PPN_OFFSET  0x4000

/**
* @internal smemPipeInit function
* @endinternal
*
* @brief   Init memory module for a PIPE device.
*/
void smemPipeInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemPipeInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemPipeInit2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemPipeInterruptTreeInit function
* @endinternal
*
* @brief   Init the interrupts tree for the PIPE device
*
* @param[in] devObjPtr                - pointer to device object.
*/
GT_VOID smemPipeInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
);

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemPipeActiveWriteFastDumpReg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemPipeh */


