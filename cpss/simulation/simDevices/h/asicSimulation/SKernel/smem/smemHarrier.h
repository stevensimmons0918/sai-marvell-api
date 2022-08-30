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
* @file smemHarrier.h
*
* @brief Harrier memory mapping implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __smemHarrierh
#define __smemHarrierh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemHawk.h>
#include <asicSimulation/SKernel/smem/smemPhoenix.h>

/**
* @internal smemHarrierInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
void smemHarrierInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemHarrierInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemHarrierInit2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/*******************************************************************************
*   smemHarrierInterruptTreeInit
*
* DESCRIPTION:
*       Init the interrupts tree for the Hawk device
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
GT_VOID smemHarrierInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
);


/**
* @internal smemHarrierSpecificDeviceUnitAlloc_main function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemHarrierSpecificDeviceUnitAlloc_main
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemHarrierPexAndBar0DeviceUnitAlloc function
* @endinternal
*
* @brief   allocate 'PEX config space' and 'BAR0' -- if not allocated already
* @param[in] devObjPtr                - pointer to device object.
*
*/
void smemHarrierPexAndBar0DeviceUnitAlloc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemsmemHarrierh */


