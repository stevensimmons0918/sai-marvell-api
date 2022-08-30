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
* @file prvCpssPipeDiag.h
*
* @brief Internal header with diag utilities.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssPipeDiagh
#define __prvCpssPipeDiagh

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssPxDiagBistCheckSkipOptionalClient function
* @endinternal
*
* @brief   Check skip of optional DFX client for BIST
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pipe                     - DFX pipe number
* @param[in] client                   - DFX client number
* @param[in] skipCpuMemory            - skip internal CPU related memories from output
*
* @retval GT_TRUE                  - skip BIST
* @retval GT_FALSE                 - does not skip BIST
*/
GT_BOOL prvCpssPxDiagBistCheckSkipOptionalClient
(
    IN  GT_U32              pipe,
    IN  GT_U32              client,
    IN  GT_BOOL             skipCpuMemory
);

/**
* @internal prvCpssPxDiagDataIntegrityDbPointerSet function
* @endinternal
*
* @brief   Function initializes current pointer to DB and size.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[out] dbArrayPtrPtr         - (pointer to pointer to) current DB array
*                                      NULL if DB doesn't exists
* @param[out] dbArrayEntryNumPtr       - (pointer to) current DB array size
*                                      0 if DB doesn't exists
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                   - on error
*/
GT_VOID prvCpssPxDiagDataIntegrityDbPointerSet
(
    OUT PRV_CPSS_DFX_DIAG_DATA_INTEGRITY_MAP_DB_STC    **dbArrayPtrPtr,
    OUT GT_U32                                          *dbArrayEntryNumPtr
);

/**
* @internal prvCpssPxDiagDataIntegrityTableLogicalToHwListGet function
* @endinternal
*
* @brief   Function returns the list of HW tables that relate to the logical table
*
* @note   APPLICABLE DEVICES:      PIPE.
* @note   NOT APPLICABLE DEVICES:  None
*
*
* @param[in] logicalTable             - the logical table
*
* @param[out] hwTableslistPtr          - (pointer to) the list of the HW tables
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if table not supported
*/
GT_STATUS prvCpssPxDiagDataIntegrityTableLogicalToHwListGet
(
    IN  CPSS_PX_LOGICAL_TABLE_ENT         logicalTable,
    OUT const CPSS_PX_TABLE_ENT           **hwTableslistPtr
);

/**
* @internal prvCpssPxDiagDataIntegrityTableHwToLogicalGet function
* @endinternal
*
* @brief  Function returns logical tables for specific hw table.
* @param[in] hwTable                  - the hw table name
* @param[in,out] startPtr             - (pointer to) a marker where from to start search.
*                                      0 on NULL means "search from beginning".
*                                      An out-value is used as input for next call
*                                      to continue the search.
*
* @param[out] logicalTablePtr          - (pointer to) the logical table name
*
* @retval GT_OK                    - logical table is found
* @retval GT_NOT_FOUND             - logical table is not found
*
*
*/
GT_VOID prvCpssPxDiagDataIntegrityTableHwToLogicalGet
(
    IN    CPSS_PX_TABLE_ENT              hwTable,
    OUT   CPSS_PX_LOGICAL_TABLE_ENT      *logicalTablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssPipeDiagh */
