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
* @file smemAc5.h
*
* @brief AC5 memory mapping implementation
*
* @version   7
********************************************************************************
*/
#ifndef __smemAc5h
#define __smemAc5h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemXCat3.h>

/**
* @internal smemAc5Init function
* @endinternal
*
* @brief   Init memory module for the AC5 device.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemAc5Init
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemAc5Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr             - pointer to device object.
*/
void smemAc5Init2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemAc5AllocSpecMemory_additions function
* @endinternal
*
* @brief   Allocate addition AC5 address type specific memories , that are not in AC3
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemAc5AllocSpecMemory_additions
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);



/**
* @internal smemAc5MemUpdates function
* @endinternal
*
* @brief   AC5 updates to add/override AC3 registers/tables
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemAc5MemUpdates
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemAc5PclTcamClientContextSet function
* @endinternal
*
* @brief   AC5 : state that the PCL-TCAM client (IPCL 0/1/2 or EPCL) is starting
            it's access . this needed for memory access to 'SMEM_XCAT3_UNIT_TCC_LOWER_E'
*           address to access proper pcl_tcam unit (can be changed to SMEM_AC5_UNIT_PCL_TCC_1_E)
* @param[in] devObjPtr                - pointer to device object.
* @param[in] tcamClient              -  one of: IPCL 0/1/2 / EPCL , LAST last means 'out of context'
*
* return - None
*
* @note function MUST be called before the client start to access the TCAM UNIT
*       memory/registers
*
*/
void smemAc5PclTcamClientContextSet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SIP5_TCAM_CLIENT_ENT    tcamClient
);


/**
* @internal smemAc5PclTcamByAddrContextSet function
* @endinternal
*
* @brief   AC5 : state that the PCL-TCAM is accessed from the CPU by address in one of the TCAM[0/1]
*           this needed for memory access to 'SMEM_XCAT3_UNIT_TCC_LOWER_E'
*           address to access proper pcl_tcam unit (can be changed to SMEM_AC5_UNIT_PCL_TCC_1_E)
* @param[in] devObjPtr            - pointer to device object.
* @param[in] address              - address in TCAM[0] or in TCAM[1]
*
* return - None
*
* @note function MUST be called before the client start to access the TCAM UNIT
*       memory/registers
*       function is called from the 'active memory'
*
*       NOTE: to 'end' the context , call smemAc5PclTcamClientContextSet with  SIP5_TCAM_CLIENT_LAST_E
*/
void smemAc5PclTcamByAddrContextSet(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32    address
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemAc5h */


