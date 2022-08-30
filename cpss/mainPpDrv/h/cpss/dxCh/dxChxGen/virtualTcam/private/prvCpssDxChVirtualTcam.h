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
* @file prvCpssDxChVirtualTcam.h
*
* @brief The CPSS DXCH High Level Virtual TCAM Manager private functions
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChVirtualTcam_h
#define __prvCpssDxChVirtualTcam_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>

/*
 * Typedef: function CPSS_DXCH_VIRTUAL_TCAM_HA_POST_CREATE_MGR_FUNC
 *
 * Description:
 *     This Callback Function called after Create TCAM Manager begore adding devices to it.
 *     Used to bind HW DB access callbacks.
 *
 * INPUTS:
 * @param[in]  vtcamMgrId              - VTCAM Manager Id
 *
 * @retval GT_OK                      - on success, other at fail.
 *
 */
typedef GT_STATUS CPSS_DXCH_VIRTUAL_TCAM_HA_POST_CREATE_MGR_FUNC
(
    IN   GT_U32             vtcamMgrId
);


/**
* @internal prvCpssDxChVirtualTcamHaRuleClear function
* @endinternal
*
* @brief   Clears (Sets to zeros) the TCAM mask and key. Writing operation is preformed on all
*         bits in the line.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Falcon)
*                                      Bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      3. see general note about portGroupsBmp in TCAM APIs for Falcon
* @param[in] index                    - global line  in TCAM to write to.
* @param[in] ruleSize                 - size of rule
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChVirtualTcamHaRuleClear
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize
);

/**
 * @internal prvCpssDxChVirtualTcamHaDbClear function
 * @endinternal
 *
 * @brief   clear/reset vTcamHaDb
 */
GT_VOID prvCpssDxChVirtualTcamHaDbClear
(
    IN GT_U32 vTcamMngId
);

/**
 * @internal prvCpssDxChVirtualTcamHaCatchUp function
 * @endinternal
 *
 * @brief HA Catchup - called during systemRecovery
 *
 */
GT_STATUS prvCpssDxChVirtualTcamHaCatchUp
(
    IN  GT_U8    devNum,
    IN  GT_U32   vTcamMngId
);

/**
 * @internal prvCpssDxChVirtualTcamCatchupForHaBuildDbAndClearUnmappedRules function
 * @endinternal
 *
 * @brief  CatchUp function for vTCAM Recovery in HA mode - called as part of completion handle
 *         Build VTCAM DB by replay info about found rules.
 *         Cleanup from TCAM not replayed rules.
 */
GT_STATUS prvCpssDxChVirtualTcamCatchupForHaBuildDbAndClearUnmappedRules
(
    GT_VOID
);

/**
 * @internal prvCpssDxChVirtualTcamCatchupForHaWriteFailedReplyedRules function
 * @endinternal
 *
 * @brief  CatchUp function for vTCAM Recovery in HA mode - called as part of completion handle
 *         Write rules that were replayed, but not found in TCAM.
 */
GT_STATUS prvCpssDxChVirtualTcamCatchupForHaWriteFailedReplyedRules
(
    GT_VOID
);

/**
 * @internal prvCpssDxChVirtualTcamCatchupForHa function
 * @endinternal
 *
 * @brief  CatchUp function for vTCAM Recovery in HA mode - called as part of completion handle
 */
GT_STATUS prvCpssDxChVirtualTcamCatchupForHa
(
    GT_VOID
);

/**
 * @internal prvCpssDxChVirtualTcamDbVTcamDumpFullTcamPriorityModeData function
 * @endinternal
 *
 * @brief   Dump all Trees Created in Priority Mode vTCAMs
 *
 */
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpFullTcamPriorityModeData ();

/**
 * @internal  prvCpssDxChVirtualTcamHaFoundRuleSegmentsCreate
 * @endinternal
 *
 * @brief  Fn. to create a list of segments from the rules retrieved from vTcamhaDb corresponding to the vTcamId supplied
 *
 */
GT_STATUS prvCpssDxChVirtualTcamHaFoundRuleSegmentsCreate
(
    IN GT_U32 vTcamMngId,
    IN GT_U32 vTcamId
);

/**
 * @internal prvCpssDxChVirtualTcamHaDbVTcamEntriesNumAndSizeGet function
 * @endinternal
 *
 * @brief Function finds the number of entries and the ruleSize corresponding to the vTCAM with supplied vTcamId
 */
GT_STATUS prvCpssDxChVirtualTcamHaDbVTcamEntriesNumAndSizeGet
(
   IN  GT_U32      vTcamMngId,
   IN  GT_U32      vTcamId,
   OUT GT_U32     *_numEntries,
   OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT *_ruleSize
);

extern GT_COMP_RES prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFreeSegFunc
(
    IN  GT_VOID    *data_ptr1,
    IN  GT_VOID    *data_ptr2
);

extern GT_VOID prvCpssDxChVirtualTcamHaDbgPrintSegment
(
    GT_CHAR      *info,
    GT_VOID_PTR   dataPtr
);

/**
* @internal prvCpssDxChVirtualTcamHsuSizeGet function
* @endinternal
*
* @brief   Gets the memory size needed for Virtual TCAM feature internal DB HSU export information.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
*
* @param[out] sizePtr                  - points to memory size needed for export data,
*                                      including HSU data block header (calculated in bytes)
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamHsuSizeGet
(
    OUT  GT_U32      *sizePtr
);

/**
* @internal prvCpssDxChVirtualTcamHsuExport function
* @endinternal
*
* @brief   Exports Virtual TCAM feature internal DB HSU data.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the Virtual TCAM feature internal DB
*                                      to track the export progress.
*                                      At first iteration set (iteratorPtr) = 0
*                                      At next iterations supply value returned by
*                                      previous iteration.
*                                      Relevant only when single operation export is not used
* @param[in,out] hsuBlockMemSizePtr       != CPSS_HSU_SINGLE_ITERATION_CNS)
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block supplied
*                                      for the current iteration (in bytes);
*                                      - Size must be at least 1k bytes.
*                                      - Use the value CPSS_HSU_SINGLE_ITERATION_CNS value
*                                      to indicate single iteration export. In such case
*                                      it is assumed that hsuBlockMemPtr is large enough
*                                      for the entire HSU data.
* @param[in] hsuBlockMemPtr           - points to memory block to hold HSU data in
*                                      current iteration
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the Virtual TCAM feature internal DB
*                                      to track the export progress.
*                                      Relevant only if exportCompletePtr == GT_FALSE
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block that was
*                                      not used in the current iteration (in bytes)
*                                      Relevant only when single iteration export is not used
*                                      (input valus was not CPSS_HSU_SINGLE_ITERATION_CNS)
*
* @param[out] exportCompletePtr        - GT_TRUE:  HSU data export is completed.
*                                      GT_FALSE: HSU data export is not completed.
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*
* @note 1. Several export processes can be performed simultaneously.
*       2. It is assumed that in case of multiple iterations export operation,
*       there are no calls to other Virtual TCAM feature internal DB APIs
*       until export operation is completed. Such calls may modify the data
*       stored within the Virtual TCAM feature internal DB and
*       cause unexpected behavior.
*
*/
GT_STATUS prvCpssDxChVirtualTcamHsuExport
(
    INOUT GT_UINTPTR    *iteratorPtr,
    INOUT GT_U32        *hsuBlockMemSizePtr,
    IN    GT_U8         *hsuBlockMemPtr,
    OUT   GT_BOOL       *exportCompletePtr
);

/**
* @internal prvCpssDxChVirtualTcamHsuImport function
* @endinternal
*
* @brief   Imports Virtual TCAM feature internal DB HSU data.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the Virtual TCAM feature internal DB
*                                      to track the import progress.
*                                      At first iteration set (iteratorPtr) = 0
*                                      At next iterations supply value returned by
*                                      previous iteration.
*                                      Relevant only when single operation import is not used
* @param[in,out] hsuBlockMemSizePtr       != CPSS_HSU_SINGLE_ITERATION_CNS)
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block supplied
*                                      for the current iteration (in bytes);
*                                      - Size must be at least 1k bytes.
*                                      - Use the value CPSS_HSU_SINGLE_ITERATION_CNS value
*                                      to indicate single iteration import. In such case
*                                      it is assumed that hsuBlockMemPtr contains all HSU data.
* @param[in] hsuBlockMemPtr           - points to memory block that holds HSU data for
*                                      current iteration
* @param[in,out] iteratorPtr              - points to iteration value; The iteration value
*                                      is used by the Virtual TCAM feature internal DB
*                                      to track the import progress.
*                                      Relevant only if importCompletePtr == GT_FALSE
* @param[in,out] hsuBlockMemSizePtr       - points to the size of the memory block that was
*                                      not used in the current iteration (in bytes)
*                                      Relevant only when single iteration import is not used
*                                      (input valus was not CPSS_HSU_SINGLE_ITERATION_CNS)
*
* @param[out] importCompletePtr        - GT_TRUE:  HSU data import is completed.
*                                      GT_FALSE: HSU data import is not completed.
*
* @retval GT_OK                    - if allocation succedded
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*
* @note 1. The import function will override all the Virtual TCAM feature internal DB
*       data and set it to the data of the Virtual TCAM feature internal DB that
*       performed the export at the time of the export. All changes made between
*       the export and the import will be discarded.
*       2. It is assumed that in case of multiple iterations import operation,
*       there are no calls to other Virtual TCAM APIs until import is
*       completed. Such calls may modify the data stored within the
*       Virtual TCAM feature internal DB and cause unexpected behavior.
*       3. It is assumed that no different import processes are performed
*       simultaneously.
*
*/
GT_STATUS prvCpssDxChVirtualTcamHsuImport
(
    INOUT GT_UINTPTR    *iteratorPtr,
    INOUT GT_U32        *hsuBlockMemSizePtr,
    IN    GT_U8         *hsuBlockMemPtr,
    OUT   GT_BOOL       *importCompletePtr
);

/***********************************************************************************************/
/* Using LPM memory as HW DB for VTCAM manager */

/* LPM Memory line 32 bits */
#define PRV_CPSS_DXCH_LPM_LINE_BITS_CNS 32
/* HW DB - Rule Id 32 bits, vTcamId 12 bits */
#define PRV_CPSS_DXCH_VTCAM_HA_HW_DB_ENTRY_BITS_CNS 44

/**
 * @internal prvCpssDxChVirtualTcamHaHwDbLpmWrite function
 * @endinternal
 * @brief This Function writes to HW DB pair of vTcamId and ruleId for HW rule index.
 *
 * @param[in] devNum                  - the device number
 * @param[in] portGroupsBmp           - bitmap of Port Groups.
 * @param[in] ruleHwIndex             - rule physical index.
 * @param[in] vTcamId                 - vTCAM Id.
 *                                      (APPLICABLE RANGES: 0..0xFFF)
 * @param[in] ruleId                  - rule Id
 *
 *  Comments:
 *
 */
GT_STATUS prvCpssDxChVirtualTcamHaHwDbLpmWrite
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              ruleHwIndex,
    IN  GT_U32              vTcamId,
    IN  GT_U32              ruleId
);

/**
 * @internal prvCpssDxChVirtualTcamHaHwDbLpmRead function
 * @endinternal
 * @brief This Function reads to HW DB pair of vTcamId and ruleId for HW rule index.
 *
 * @param[in] devNum                  - the device number
 * @param[in] portGroupsBmp           - bitmap of Port Groups.
 * @param[in] ruleHwIndex             - rule physical index.
 * @param[out] vTcamIdPtr             - (pointer to)vTCAM Id.
 * @param[out] ruleIdPtr              - (pointer to)rule Id
 *
 *  Comments:
 *
 */
GT_STATUS prvCpssDxChVirtualTcamHaHwDbLpmRead
(
    IN   GT_U8               devNum,
    IN   GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN   GT_U32              ruleHwIndex,
    OUT  GT_U32             *vTcamIdPtr,
    OUT  GT_U32             *ruleIdPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChVirtualTcam_h */


