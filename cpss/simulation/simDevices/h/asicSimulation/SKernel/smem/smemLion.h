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
* @file smemLion.h
*
* @brief Lion memory mapping implementation: the pipe and the shared memory
*
* @version   12
********************************************************************************
*/
#ifndef __smemLionh
#define __smemLionh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemCheetah3.h>

/**
* @internal smemLionInit function
* @endinternal
*
* @brief   Init memory module for a Lion device.
*
* @param[in] deviceObj                - pointer to device object.
*/
void smemLionInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemLionInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] deviceObj                - pointer to device object.
*/
void smemLionInit2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemLionTableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemLionTableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);


/*write active memory*/
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLionActiveWriteSchedulerConfigReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLionActiveWriteTodGlobalReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemLionActiveWriteDequeueEnableReg);

/*read active memory*/
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemLionActiveReadPtpMessage);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemLionActiveReadPtpTodSeconds);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemLionActiveReadPtpTodNanoSeconds);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemLionh */


