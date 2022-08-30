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
* @file prvCpssPxDiagDataIntegrityMainMappingDb.h
*
* @brief Internal header with DFX Data Integrity module main mapping batabase.
*
* @version   6
********************************************************************************
*/
#ifndef __prvCpssPxDiagDataIntegrityMainMappingDbh
#define __prvCpssxPDiagDataIntegrityMainMappingDbh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/px/diag/cpssPxDiagDataIntegrity.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxHwTables.h>
#include <cpss/common/diag/private/prvCpssCommonDiag.h>
#include <cpss/px/diag/cpssPxDiagDataIntegrityTables.h>

#define LAST_VALID_PX_TABLE_CNS   0xFFFFFFFF

/* Maximal DFX cause index  */
#define MAX_DFX_INT_CAUSE_NUM_CNS                       27
/* indicator of unused DB entry */
#define DATA_INTEGRITY_ENTRY_NOT_USED_CNS               0xCAFECAFE

extern PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC pipeDataIntegrityDbArray[];
extern GT_U32 pipeDataIntegrityDbArrayEntryNum;
extern PRV_CPSS_DIAG_DATA_INTEGRITY_DFX_FIRST_STAGE_MAPPING_DATA_STC pipeDfxIntToPipeClientMapArr[];

/**
* @internal prvCpssPxDiagDataIntegrityMemoryIndexesGet function
* @endinternal
*
* @brief   Function gets all memory location coordinates for given memory type.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in] memType                  - memory type
* @param[in,out] arraySizePtr        - in : size of input array
*                                      out: actual size of array
*
* @param[out] memLocationArr           - array of memory locations
* @param[out] protectionTypePtr        - (pointer to) protection memory type
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_SIZE              - if size of input array is not enough to store
*                                       coordinates of all found items
* @retval GT_NOT_FOUND             - if memory or HW table is not found in
*                                       Data Integrity DB
*/
GT_STATUS prvCpssPxDiagDataIntegrityMemoryIndexesGet
(
    IN  GT_SW_DEV_NUM                                           devNum,
    IN  CPSS_PX_DIAG_DATA_INTEGRITY_MEM_TYPE_ENT                memType,
    INOUT GT_U32                                                *arraySizePtr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEMORY_LOCATION_STC            *memLocationArr,
    OUT CPSS_DIAG_DATA_INTEGRITY_MEM_ERROR_PROTECTION_TYPE_ENT  *protectionTypePtr
);

/**
* @internal prvCpssPxDiagDataIntegrityFillHwLogicalFromRam
*           function
* @endinternal
*
* @brief   fill HW and logical locations in
*          CPSS_PX_LOCATION_FULL_INFO_STC struct according to
*          RAM location.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - PP device number
* @param[in,out] locationPtr          - (pointer to) location. HW- and logical-
*                                       related parts will be modified.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device, portGroup
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssPxDiagDataIntegrityFillHwLogicalFromRam
(
    IN    GT_U8 devNum,
    INOUT CPSS_PX_LOCATION_FULL_INFO_STC *locationPtr
);

/**
* @internal prvCpssPxDiagDataIntegrityDbPointerSet function
* @endinternal
*
* @brief   Function initializes current pointer to DB and size.
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[out] dbArrayPtrPtr            - (pointer to pointer to) current DB array
*                                      NULL if DB doesn't exists
* @param[out] dbArrayEntryNumPtr       - (pointer to) current DB array size
*                                      0 if DB doesn't exists
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_VOID prvCpssPxDiagDataIntegrityDbPointerSet
(
    OUT PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    **dbArrayPtrPtr,
    OUT GT_U32                                          *dbArrayEntryNumPtr
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPxDiagDataIntegrityMainMappingDbh */
