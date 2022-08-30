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
* @file cpssPxDiagDataIntegrityTables.h
*
* @brief API definition for tables access
*
* @version   1
********************************************************************************
*/

#ifndef __cpssPxDiagDataIntegrityTablesh
#define __cpssPxDiagDataIntegrityTablesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/private/prvCpssMisc.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <cpss/px/cpssHwInit/cpssPxHwInit.h>


/*
* @struct CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC
*
* @brief This struct defines logical table shadow info entry
*/
typedef struct{

    /** @brief  logical tables */
    CPSS_PX_LOGICAL_TABLE_ENT      logicalTableName;
    /** @brief  table shadow type*/
    CPSS_PX_SHADOW_TYPE_ENT        shadowType;
    /** @brief  table size in Bytes*/
    GT_U32                         numOfBytes;
    /** @brief if the table has shadow*/
    GT_STATUS                      isSupported;

} CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC;

/**
* @struct CPSS_DXCH_LOGICAL_TABLES_SHADOW_INFO_STC
*
* @brief This struct defines logical table shadow entry
*/
typedef struct{

    /** @brief  number of tables that should have shadow*/
    GT_U32                                numOfDataIntegrityElements;
    /** @brief  list of logical tables*/
    CPSS_PX_LOGICAL_TABLE_SHADOW_INFO_STC *logicalTablesArr;

} CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC;


/**
* @internal cpssPxDiagDataIntegrityTableScan function
* @endinternal
*
* @brief   Data integrity SW daemon API
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
* @param[in] locationPtr              - (pointer to) location information,
*                                      ram location is not supported
* @param[in] numOfEntries             - number of entries to read (0xFFFFFFFF means till end of table)
*                                      NOTE: for 'logical table' this value is ignored and taken from :
* @param[in] locationPtr
*
* @param[out] nextEntryIndexPtr        - (pointer to) next index
* @param[out] wasWrapAroundPtr         - (pointer to) was wrap around indication
*                                      NOTE: ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagDataIntegrityTableScan
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC   *locationPtr,
    IN  GT_U32                                numOfEntries,
    OUT GT_U32                               *nextEntryIndexPtr,
    OUT GT_BOOL                              *wasWrapAroundPtr
);
/**
* @internal cpssPxDiagDataIntegrityTableEntryFix function
* @endinternal
*
* @brief   Data integrity : fix by writing data from the CPSS
*          shadow the specific table entry that was corrupted.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - the device number
* @param[in] locationPtr              - (pointer to) location information
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on bad param
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_EMPTY                 - the DFX location is not mapped to CPSS HW table.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagDataIntegrityTableEntryFix
(
    IN   GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_LOCATION_SPECIFIC_INFO_STC    *locationPtr
);

/**
* @internal cpssPxDiagDataIntegrityShadowTableSizeGet function
* @endinternal
*
* @brief   This function calculates size of memory required each
*          shadow table and total shadow memory size
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                 - device number
* @param[in,out] tablesInfoPtr      - in : pointer to table info
*                                     out: referenced table size
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxDiagDataIntegrityShadowTableSizeGet
(
    IN     GT_SW_DEV_NUM                            devNum,
    INOUT  CPSS_PX_LOGICAL_SHADOW_TABLES_INFO_STC   *tablesInfoPtr,
    OUT    GT_U32                                   *totalMemSizePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChDiagDataIntegrityTablesh */
