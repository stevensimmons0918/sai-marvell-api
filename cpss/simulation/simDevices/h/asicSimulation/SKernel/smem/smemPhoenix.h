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
* @file smemPhoenix.h
*
* @brief Phoenix memory mapping implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __smemPhoenixh
#define __smemPhoenixh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemFalcon.h>

/* DFX active memory */
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSip6_30ActiveWriteDfxServerDeviceCtrl50Reg);

/**
* @internal smemPhoenixInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
void smemPhoenixInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemPhoenixInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemPhoenixInit2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/*******************************************************************************
*   smemPhoenixInterruptTreeInit
*
* DESCRIPTION:
*       Init the interrupts tree for the Phoenix device
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
GT_VOID smemPhoenixInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemPhoenixSpecificDeviceUnitAlloc_SIP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         SIP units
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemPhoenixSpecificDeviceUnitAlloc_SIP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemPhoenixSpecificDeviceUnitAlloc_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemPhoenixSpecificDeviceUnitAlloc_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemPhoenixSpecificDeviceUnitAllocDerived_DP_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
*         Evaluation version of smemPhoenixSpecificDeviceUnitAlloc_DP_units
*         replacing derived units by original.
* @param[in] devObjPtr                - pointer to device object.
* @param[in] skipUnitsArrPtr          - pointer to array of aleady allocated units to skip
*/
void smemPhoenixSpecificDeviceUnitAllocDerived_DP_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SMEM_DEVICE_UNIT_ALLOCATION_STC *skipUnitsArrPtr
);

/**
* @internal smemPhoenixSpecificDeviceUnitAlloc_Cnm_units function
* @endinternal
*
* @brief   specific initialization units allocation that called before alloc units
*         of any device.
*         DP units -- (non sip units)
* @param[in] devObjPtr                - pointer to device object.
*                                       Pointer to object for specific subunit
*/
void smemPhoenixSpecificDeviceUnitAlloc_Cnm_units
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

void smemPhoenixInit_debug_allowTraffic
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

void update_mem_chunk_from_size_to_size(
    IN SMEM_CHUNK_BASIC_STC    chunksMemArr[],
    IN GT_U32                  numOfChunks,
    IN GT_U32                  origNumOfBytes,
    IN GT_U32                  newNumOfBytes
);

void update_list_register_default_size_to_size(
    IN SMEM_REGISTER_DEFAULT_VALUE_STC    registersDefaultValueArr[],
    IN GT_U32                  origNumOfRepetitions,
    IN GT_U32                  newNumOfRepetitions
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemPhoenixh */


