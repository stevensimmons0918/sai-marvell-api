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
* @file smemAldrin2.h
*
* @brief Aldrin2 memory mapping implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __smemAldrin2h
#define __smemAldrin2h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemBobcat3.h>


/**
* @internal smemAldrin2Init function
* @endinternal
*
* @brief   Init memory module for a Aldrin2 device.
*/
void smemAldrin2Init
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemAldrin2Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemAldrin2Init2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemAldrin2SpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemAldrin2SpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemAldrin2h */


