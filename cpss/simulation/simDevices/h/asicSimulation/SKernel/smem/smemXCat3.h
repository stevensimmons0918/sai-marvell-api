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
* @file smemXCat3.h
*
* @brief xCat3 memory mapping implementation
*
* @version   7
********************************************************************************
*/
#ifndef __smemXCat3h
#define __smemXCat3h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemLion.h>

/**
* @enum SMEM_XCAT3_UNIT_NAME_ENT
 *
 * @brief Memory units names
*/
typedef enum{
    SMEM_XCAT3_UNIT_MG_E,
    SMEM_XCAT3_UNIT_EGR_TXQ_E,
    SMEM_XCAT3_UNIT_L2I_E,
    SMEM_XCAT3_UNIT_IPVX_E,
    SMEM_XCAT3_UNIT_BM_E,
    SMEM_XCAT3_UNIT_EPLR_E,
    SMEM_XCAT3_UNIT_LMS_E,
    SMEM_XCAT3_UNIT_FDB_E,
    SMEM_XCAT3_UNIT_MPPM_BANK0_E,
    SMEM_XCAT3_UNIT_MPPM_BANK1_E,
    SMEM_XCAT3_UNIT_MEM_E,
    SMEM_XCAT3_UNIT_CENTRALIZED_COUNT_E,
    SMEM_XCAT3_UNIT_MSM_E,
    SMEM_XCAT3_UNIT_GOP_E,
    SMEM_XCAT3_UNIT_SERDES_E,
    SMEM_XCAT3_UNIT_VLAN_MC_E,
    SMEM_XCAT3_UNIT_EQ_E,
    SMEM_XCAT3_UNIT_IPCL_E,
    SMEM_XCAT3_UNIT_TTI_E,
    SMEM_XCAT3_UNIT_IPLR0_E,
    SMEM_XCAT3_UNIT_IPLR1_E,
    SMEM_XCAT3_UNIT_MLL_E,
    SMEM_XCAT3_UNIT_TCC_LOWER_E,
    SMEM_XCAT3_UNIT_TCC_UPPER_E,
    SMEM_XCAT3_UNIT_HA_E,
    SMEM_XCAT3_UNIT_EPCL_E,
    SMEM_XCAT3_UNIT_CCFC_E,

    SMEM_XCAT3_UNIT_LAST_E,

    SMEM_AC5_UNIT_CNC_1_E = SMEM_XCAT3_UNIT_LAST_E, /* duplicate unit of 'SMEM_XCAT3_UNIT_CENTRALIZED_COUNT_E' */
    SMEM_AC5_UNIT_PCL_TCC_1_E,                      /* duplicate unit of 'SMEM_XCAT3_UNIT_TCC_LOWER_E'         */
    SMEM_AC5_UNIT_DFX_SERVER_E,                     /* DFX server that replace the one in AC3 that was in dedicated memory space */
    SMEM_AC5_UNIT_MG0_E,                            /* MG0,1,2 units */
    SMEM_AC5_UNIT_MPP_E,                            /* MPP unit */

    SMEM_AC5_UNIT_LAST_E
}SMEM_XCAT3_UNIT_NAME_ENT;

/**
* @internal smemXCat3Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] deviceObjPtr             - pointer to device object.
*/
void smemXCat3Init2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObjPtr
);

/**
* @internal smemXCat3Init function
* @endinternal
*
* @brief   Init memory module for the xCat2 device.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void smemXCat3Init
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemXCat3TableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemXCat3TableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemXCat3TableInfoSetPart2 function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*         AFTER the bound of memories (after calling smemBindTablesToMemories)
* @param[in] devObjPtr                - device object PTR.
*/
void smemXCat3TableInfoSetPart2
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemXCat3h */


