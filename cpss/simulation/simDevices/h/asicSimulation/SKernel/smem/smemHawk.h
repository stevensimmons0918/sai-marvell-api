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
* @file smemHawk.h
*
* @brief Hawk memory mapping implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __smemHawkh
#define __smemHawkh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemFalcon.h>


enum{
    sip6_MTI_EXTERNAL_representativePortIndex_base_hawk100GPortsArr = 0,/*0..3*/
    sip6_MTI_EXTERNAL_representativePortIndex_base_hawkUsxPortsArr  = 4,/*4..9*/
    sip6_MTI_EXTERNAL_representativePortIndex_base_hawk___last      = 10
};

/**
* @internal smemHawkInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
void smemHawkInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemHawkInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemHawkInit2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/*******************************************************************************
*   smemHawkInterruptTreeInit
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
GT_VOID smemHawkInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemHawkSpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemHawkSpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);
/**
* @internal smemHawkSpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemHawkSpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/*******************************************************************************
*   smemHawkBindFindMemoryFunc
*
* DESCRIPTION:
*       Hawk - bind special 'find memory' functions
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
void smemHawkBindFindMemoryFunc(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr
);
/**
* @internal smemHawkGopRegDbInit function
* @endinternal
*
* @brief   Init GOP regDB registers for Hawk.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemHawkGopRegDbInit(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemHawkInit_debug_allowTraffic function
* @endinternal
*
* @brief   debug function to allow traffic (flooding) in the device.
*         to allow debug before the CPSS know how to configure the device.
*         (without the cpssInitSystem)
* @param[in] devObjPtr                - pointer to device object.
*/
void smemHawkInit_debug_allowTraffic
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

ACTIVE_READ_FUNC_PROTOTYPE_MAC (smemHawkActiveReadLatencyProfileStatTable);

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC (smemChtActiveHawkWriteMifChannelMappingReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC (smemChtActiveHawkWriteMifTxControlReg);

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC (smemHawkActiveWriteToSdwPuPll);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC (smemHawkActiveWriteToSdwTxRxTraining);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC (smemHawkActiveWriteToSdwExpectedChecksum1);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC (smemHawkActiveWriteToSdwExpectedChecksum2);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemHawkh */


