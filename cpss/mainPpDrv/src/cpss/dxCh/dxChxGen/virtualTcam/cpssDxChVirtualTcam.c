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
* @file cpssDxChVirtualTcam.c
*
* @brief The CPSS DXCH High Level Virtual TCAM Manager
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Map to convert vTCAM ruleSize to TCAM ruleSize */
static const CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT
tcamRuleSizeMap[CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E + 1] =
{
    CPSS_DXCH_TCAM_RULE_SIZE_10_B_E,
    CPSS_DXCH_TCAM_RULE_SIZE_20_B_E,
    CPSS_DXCH_TCAM_RULE_SIZE_30_B_E,
    CPSS_DXCH_TCAM_RULE_SIZE_40_B_E,
    CPSS_DXCH_TCAM_RULE_SIZE_50_B_E,
    CPSS_DXCH_TCAM_RULE_SIZE_60_B_E,
                                  0,
    CPSS_DXCH_TCAM_RULE_SIZE_80_B_E
};

/* Map to convert TCAM ruleSize to vTCAM ruleSize */
static const CPSS_DXCH_TCAM_RULE_SIZE_ENT
vtcamRuleSizeMap[CPSS_DXCH_TCAM_RULE_SIZE_80_B_E + 1] =
{
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E,
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E
};


/* indication to allow Rule To Be Absent From Tcam During Priority Update */
static GT_BOOL  const allowRuleToBeAbsentFromTcamDuringPriorityUpdate = GT_FALSE;

/* function to force (or not to force) the 'ruleWrite' API to use 'first' position.
   (by default it uses the 'last' position)
 debug mode to allow quick testing of the 'position first' that
   the 'rule Update' API allow to use. */
void  prvCpssDxChVirtualTcamDbVTcam_debug_writeRulePositionForceFirstSet(
    GT_BOOL forceFirstPosition
)
{
    VTCAM_GLOVAR(debug_writeRuleForceFirstPosition_enable) = forceFirstPosition;
}

static GT_STATUS perDevice_ruleInvalidate
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC         *vTcamMngPtr,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U32                                      ruleIndex,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT        devClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize
);
static GT_STATUS perDevice_ruleValidate
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U32                                      ruleIndex,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT        devClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_BOOL                                     valid
);
static GT_STATUS perDevice_ruleMove
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC         *vTcamMngPtr,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT        devClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                                      srcRuleId,
    IN  GT_U32                                      dstRuleId,
    IN  GT_BOOL                                     moveOrCopy
);
static GT_STATUS ruleMoveByLogicalIndex
(
    IN  GT_U32                                      vTcamMngId,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr,
    IN  GT_U32                                      srcLogicalIndex,
    IN  GT_U32                                      srcDeviceRuleIndex,
    IN  GT_U32                                      dstLogicalIndex,
    IN  GT_U32                                      dstDeviceRuleIndex,
    IN  GT_BOOL                                     calledFromResize
);
GT_STATUS priorityTableEntryRemoveLogicalIndexUpdateRange
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC           *vTcamMngDBPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *vtcamInfoPtr,
    IN   GT_U32                                       priority,
    IN   GT_U32                                       logicalIndex
);

static GT_STATUS copyAllRulesFromSrcDevToDevBmp(
    IN GT_U32                                            vTcamMngId,
    IN GT_U8                                             representativeDevNum,
    IN GT_PORT_GROUPS_BMP                                representativePortGroupsBmp,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC      devPortGroupsBmpArr[],
    IN GT_U32                                            numOfPortGroupBmps
);

static GT_STATUS invalidateAllRulesInDevsPortGroupsBmp(
    IN GT_U32                                            vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC      devPortGroupsBmpArr[],
    IN GT_U32                                            numOfPortGroupBmps
);

GT_STATUS prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN   GT_BOOL                                    checkExistance,
    OUT  GT_U32                                     *logicalIndexPtr,
    OUT  GT_U32                                     *physicalIndexPtr
);

GT_BOOL prvCpssDxChVirtualTcamHaBlockInfoIsNotMappedBlock
(
   IN  GT_U32                                          vTcamMngId,
   IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR outSegPtr
);

GT_STATUS prvCpssDxChVirtualTcamHaBlockInfoLookupIdVerify
(
   IN  GT_U32                                          vTcamMngId,
   IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR outSegPtr
);

static GT_STATUS rawEntryFormatOperationReadFromHw
(
    IN  GT_U8                                              devNum,
    IN  GT_PORT_GROUPS_BMP                                 portGroupsBmp,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT    devClass,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_ACCESS_BUFFERS_STC   *hwBuffersPtr,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT                       ruleSize,
    IN  GT_U32                                             ruleIndex
);

static GT_STATUS rawEntryFormatOperationWriteToHw
(
    IN  GT_U8                                              devNum,
    IN  GT_PORT_GROUPS_BMP                                 portGroupsBmp,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT    devClass,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_ACCESS_BUFFERS_STC   *hwBuffersPtr,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT                       ruleSize,
    IN  GT_U32                                             ruleIndex
);

extern GT_STATUS prvCpssDxChVirtualTcamSegmentTableAvlItemInsert
(
    IN  PRV_CPSS_AVL_TREE_ID                            treeId,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr
);

extern GT_VOID* prvCpssDxChVirtualTcamSegmentTableAvlItemRemove
(
    IN  PRV_CPSS_AVL_TREE_ID                            treeId,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr
);

extern GT_VOID wrPrintSegmentsTree
(
    IN PRV_CPSS_AVL_TREE_ID treeIdPtr,
    IN GT_BOOL              enable
);

extern GT_STATUS prvCpssDxChTtiConfigLogic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType,
    IN  CPSS_DXCH_TTI_RULE_UNT             *patternLogicFormatPtr,
    IN  CPSS_DXCH_TTI_RULE_UNT             *maskLogicFormatPtr,
    OUT GT_U32                             *patternHwFormatArray,
    OUT GT_U32                             *maskHwFormatArray
);

extern GT_STATUS prvCpssDxChTtiActionLogic2HwFormat
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_TTI_ACTION_STC           *logicFormatPtr,
    OUT GT_U32                             *hwFormatArray
);

static GT_STATUS priorityNewRuleIdAddedDbUpdate
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN  GT_U32                                      logicalIndex
);

static GT_STATUS perDevice_ruleClear
(
    IN  GT_U8                                              devNum,
    IN  GT_PORT_GROUPS_BMP                                 portGroupsBmp,
    IN  GT_U32                                             ruleIndex,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT    devClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT               ruleSize
);

/**
 * @internal prvCpssDxChVirtualTcamHaDbVTcamEntriesGet function
 * @endinternal
 *
 * @brief Function populates all the entries corresponding to the vTcamId in entriesPtrArr
 */
GT_STATUS prvCpssDxChVirtualTcamHaDbVTcamEntriesGet
(
   IN  GT_U32      vTcamMngId,
   IN  GT_U32      vTcamId,
   IN  GT_U32      numEntries,
   OUT PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR *entriesPtrArr
);

/**
 * @internal  prvCpssDxChVirtualTcamHaFreeSegmentSplit
 * @endinternal
 *
 * @brief  Fn. to split & maintain the Free Segment tree after a segment(inSegPtr) corresponding to rules added are found
 */
GT_STATUS prvCpssDxChVirtualTcamHaFreeSegmentSplit
(
   IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR inSegPtr,
   IN  GT_U32                                          vTcamMngId,
   OUT PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR *outSegPtr
);

/* HA Error history */

#ifdef WIN32
#define PRV_VTCAM_HA_LOG_ERR(_rc, _info)                                                      \
   do {                                                                                       \
     if (VTCAM_GLOVAR(vtcamHaErrDbCounter) < PRV_VTCAM_HA_MAX_ERR_CNS) {                      \
         VTCAM_GLOVAR(vtcamHaErrDb)[VTCAM_GLOVAR(vtcamHaErrDbCounter)].line = __LINE__;       \
         VTCAM_GLOVAR(vtcamHaErrDb)[VTCAM_GLOVAR(.vtcamHaErrDbCounter)].func = __FUNCTION__;  \
         VTCAM_GLOVAR(vtcamHaErrDb)[VTCAM_GLOVAR(vtcamHaErrDbCounter)].rc   = _rc;            \
         VTCAM_GLOVAR(vtcamHaErrDb)[VTCAM_GLOVAR(vtcamHaErrDbCounter)].info = _info;          \
     }                                                                                        \
     VTCAM_GLOVAR(vtcamHaErrDbCounter) ++;                                                    \
     cpssOsPrintf("%s", _info);                                                               \
   } while (0)
#else
#define PRV_VTCAM_HA_LOG_ERR(_rc, _info)                                           \
   do {                                                                                       \
     if (VTCAM_GLOVAR(vtcamHaErrDbCounter) < PRV_VTCAM_HA_MAX_ERR_CNS) {                      \
         VTCAM_GLOVAR(vtcamHaErrDb)[VTCAM_GLOVAR(vtcamHaErrDbCounter)].line = __LINE__;       \
         VTCAM_GLOVAR(vtcamHaErrDb)[VTCAM_GLOVAR(.vtcamHaErrDbCounter)].func = __func__;      \
         VTCAM_GLOVAR(vtcamHaErrDb)[VTCAM_GLOVAR(vtcamHaErrDbCounter)].rc   = _rc;            \
         VTCAM_GLOVAR(vtcamHaErrDb)[VTCAM_GLOVAR(vtcamHaErrDbCounter)].info = _info;          \
     }                                                                                        \
     VTCAM_GLOVAR(vtcamHaErrDbCounter) ++;                                                    \
     cpssOsPrintf("%s", _info);                                                               \
   } while (0)
#endif

/**
 * @internal prvCpssDxChVirtualTcamHaGetNumOfRulesSpaceAvailable function
 * @endinternal
 *
 * @brief   Calculate Number of rules that can be occupied in the supplied segment(free) based on ruleSize
 */
GT_STATUS prvCpssDxChVirtualTcamHaGetNumOfRulesSpaceAvailable
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT deviceClass,
    IN  GT_U32      ruleSize,
    IN  GT_VOID    *seg,
    OUT GT_U32     *numRules,
    OUT GT_U32     *rulesPerRowPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC  *segPtr = seg;
    GT_U32       rulesPerRow = 0;

    switch(ruleSize)
    {
        case 1: /* 10-byte rule */
            switch(segPtr->segmentColumnsMap)
            {

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_5_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E:
                    rulesPerRow = 1;
                    break;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_45_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_01_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E:
                    /* for sip6_10 devices - 1 rule, but this code dedicated to Falcon HA */
                    rulesPerRow =
                        (deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
                            ? 2 : 1;
                    break;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_2345_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_0123_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E:
                    rulesPerRow =
                        (deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
                            ? 4 : 2;
                    break;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_ALL_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    rulesPerRow =
                        (deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
                            ? 6 : 3;
                    break;

                default:
                    break;

            }
            break;
        case 2: /* 20-byte rule */
            switch(segPtr->segmentColumnsMap)
            {

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E:
                    rulesPerRow = 1;
                    break;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_2345_E:
                    rulesPerRow = 2;
                    break;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_ALL_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    rulesPerRow = 3;
                    break;

                default:
                    break;

            }

            break;
        case 3: /* 30-byte rule */
            switch(segPtr->segmentColumnsMap)
            {

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_ALL_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    rulesPerRow = 2;
                    break;
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_2345_E:
                    rulesPerRow = 1;
                    break;

                default:
                    break;

            }

            break;
        case 4: /* 40-byte rule */
            switch(segPtr->segmentColumnsMap)
            {
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_2345_E:
                    rulesPerRow =
                        (deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
                            ? 0 : 1;
                    break;
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_0123_E:
                    rulesPerRow =
                        (deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
                            ? 1 : 0;
                    break;
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E:
                    rulesPerRow = 1;
                    break;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_ALL_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    rulesPerRow = 1;
                    break;

            }
            break;
        case 5: /* 50-byte rule */
            switch(segPtr->segmentColumnsMap)
            {
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_01234_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E:
                    rulesPerRow = 1;
                    break;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_ALL_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    rulesPerRow = 1;
                    break;

            }
            break;
        case 6: /* 60-byte rule */
            switch(segPtr->segmentColumnsMap)
            {
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_SECOND_ALL_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    rulesPerRow = 1;
                    break;

                default:
                    break;

            }

            break;
        case 8: /* 80-byte rule */
            switch(segPtr->segmentColumnsMap)
            {
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    rulesPerRow = 1;
                    break;

                default:
                    break;

            }

            break;
        default:
            break;
    }


    *numRules = rulesPerRow * segPtr->rowsAmount;
    *rulesPerRowPtr = rulesPerRow;

    return GT_OK;
}

/**
 * @internal prvCpssDxChVirtualTcamHaGetUsableColumns function
 * @endinternal
 *
 * @brief   Get usable segment map of a free segment
 */
GT_U32 prvCpssDxChVirtualTcamHaGetUsableSegmentMap
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT deviceClass,
    IN  GT_U32      ruleSize,
    IN  GT_U32      segmentColumnsMap
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT columns0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT columns1;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT columns;
    GT_U32                                        horsBlockIdx;
    GT_U32                                        emptyMap;
    GT_U32                                        workMap;

    emptyMap = 0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        emptyMap, 0, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E);
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        emptyMap, 1, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E);

    /* free segment in DB splitted to blocks - never has colomns in more than one block */
    /* assumed, but not checked                                                         */
    columns0 = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
        segmentColumnsMap, 0);
    columns1 = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
        segmentColumnsMap, 1);

    if (columns0 != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
    {
        columns     = columns0;
        horsBlockIdx = 0;
    }
    else
    {
        columns     = columns1;
        horsBlockIdx = 1;
    }

    switch (ruleSize)
    {
        case 1: /* 10-byte rule */
            switch (columns)
            {
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E:
                    return segmentColumnsMap;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E:
                    return (horsBlockIdx == 0)
                        ? emptyMap : segmentColumnsMap;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E:
                    return emptyMap;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    return segmentColumnsMap;

                default:
                    return emptyMap;
            }
            break;
        case 2: /* 20-byte rule */
            switch(columns)
            {
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E:
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E:
                    return segmentColumnsMap;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E:
                    return (horsBlockIdx == 0)
                        ? emptyMap : segmentColumnsMap;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E:
                    return emptyMap;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    return segmentColumnsMap;

                default:
                    return emptyMap;
            }

            break;
        case 3: /* 30-byte rule */
            switch(columns)
            {

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    return segmentColumnsMap;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E:
                    return (horsBlockIdx == 0)
                        ? emptyMap : segmentColumnsMap;

                default:
                    return emptyMap;
            }

            break;
        case 4: /* 40-byte rule */
            switch(columns)
            {
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E:
                    if (deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
                    {
                        return emptyMap;
                    }
                    else
                    {
                        return (horsBlockIdx == 0) ? emptyMap : segmentColumnsMap;
                    }

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E:
                    if (deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
                    {
                        return segmentColumnsMap;
                    }
                    else
                    {
                        return (horsBlockIdx == 0) ? segmentColumnsMap : emptyMap;
                    }

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    if (deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
                    {
                        workMap = 0;
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            workMap, horsBlockIdx, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E);
                        return workMap;
                    }
                    else
                    {
                        workMap = 0;
                        if (horsBlockIdx == 0)
                        {
                            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                                workMap, 0, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E);
                        }
                        else
                        {
                            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                                workMap, 1, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E);
                        }
                        return workMap;
                    }

                default:
                    return emptyMap;
            }
            break;
        case 5: /* 50-byte rule */
            switch(columns)
            {
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E:
                    return segmentColumnsMap;

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    workMap = 0;
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                        workMap, horsBlockIdx, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E);
                    return workMap;

                default:
                    return emptyMap;
            }
            break;
        case 6: /* 60-byte rule */
            switch(columns)
            {
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    return segmentColumnsMap;

                default:
                    return emptyMap;
            }
            break;
        case 8: /* 80-byte rule */
            switch(columns)
            {
                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E:
                    if (horsBlockIdx == 0)
                    {
                        return segmentColumnsMap;
                    }
                    else
                    {
                        workMap = 0;
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            workMap, 1, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E);
                        return workMap;
                    }

                case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E:
                    return (horsBlockIdx == 0) ? emptyMap : segmentColumnsMap;

                default:
                    return emptyMap;
            }
            break;
        default:
            break;
    }

    return emptyMap;
}

/**
 * @internal prvCpssDxChVirtualTcamHaDbClear function
 * @endinternal
 *
 * @brief   clear/reset vTcamHaDb
 */
GT_VOID prvCpssDxChVirtualTcamHaDbClear
(
    IN GT_U32 vTcamMngId
)
{
    GT_U32 i = 0, j = 0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC        *vTcamMngPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_INFORMATION_MAC("vTcamMngId out of range\n");
        return;
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngPtr == NULL)
    {
        CPSS_LOG_INFORMATION_MAC("vTcamMngr not Initialized\n");
        return;
    }

    for(i=CPSS_DXCH_TCAM_RULE_SIZE_10_B_E; i<=CPSS_DXCH_TCAM_RULE_SIZE_80_B_E; ++i)
    {
        for(j=PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_0_E; j<PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_MAX_E; ++j)
        {
            vTcamMngPtr->vTcamHaDb[i][j].numRules = 0;
            if(vTcamMngPtr->vTcamHaDb[i][j].rulesArr)
            {
                cpssOsFree(vTcamMngPtr->vTcamHaDb[i][j].rulesArr);
                vTcamMngPtr->vTcamHaDb[i][j].rulesArr = NULL;
            }
        }
    }
}

/**
 * @internal prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs80bytes function
 * @endinternal
 *
 * @brief Used to fill holes in between 2 supplied segments(seg1, seg2)
 * this happens when a vTCAM is created with ruleAdds for non-contiguos ruleIds & some empty spaces(holes) remain in between
 * This function leaves wrong values in seg->baseLogicalIndex + seg->rulesAmount fields
 * the function prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts called later fixes them
 */
GT_STATUS prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs80bytes
(
   IN  GT_U32      vTcamMngId,
   IN  GT_U32      vTcamId,
   IN  GT_U32      numRules,
   IN  GT_VOID    *seg1,
   IN  GT_VOID    *seg2
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *vTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR        tcamCfgPtr;
    GT_STATUS                                          rc;
    GT_U32                                             lookupId;
    GT_U32                                             rule80ColumnMap;
    GT_U32                                             ruleMinColumnMap; /* has minimal rank in lookup order */
    GT_U32                                             neededRows;
    GT_U32                                             ruleSize = 8;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC   *segPtr1 = seg1, *segPtr2 = seg2;
    PRV_CPSS_AVL_TREE_ID                               treeId;
    GT_U32                                             luRowsRangeLow, fbRowsRangeLow;
    GT_U32                                             startRowsRangeLow, rowsRangeHigh;
    GT_U32                                             rightLow, rightHigh, workHigh;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC    patternSeg;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC    *addedSegPtr;
    GT_BOOL                                            found;
    GT_BOOL                                            rightFound;
    PRV_CPSS_AVL_TREE_PATH                             path;
    GT_VOID*                                           foundDataPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC    *freeLeftSegPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC    *freeRightSegPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT      columns;
    GT_BOOL                                            haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);
    rule80ColumnMap = 0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        rule80ColumnMap, 0, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        rule80ColumnMap, 1, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E);

    ruleMinColumnMap = 0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        ruleMinColumnMap, 0, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        ruleMinColumnMap, 1, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    tcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];

    treeId      = vTcamMngPtr->tcamSegCfg.segFreeLuTree;

    lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(tcamCfgPtr->tcamInfo.clientGroup, tcamCfgPtr->tcamInfo.hitNumber);

    if (haVtcamDebug)
    {
        cpssOsPrintf("prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs: seg1=0x%x, seg2=0x%x\r\n", seg1, seg2);
    }

    neededRows = numRules; /* one rule per row */
    startRowsRangeLow = 0;
    if (segPtr1)
    {
        startRowsRangeLow = segPtr1->rowsBase + segPtr1->rowsAmount;
    }
    luRowsRangeLow = startRowsRangeLow;
    fbRowsRangeLow = startRowsRangeLow;
    rowsRangeHigh = vTcamMngPtr->tcamSegCfg.tcamRowsAmount;
    if (segPtr2)
    {
        rowsRangeHigh = segPtr2->rowsBase;
    }

    /* avoid compiler warning */
    freeLeftSegPtr = NULL;
    freeRightSegPtr = NULL;

    while ((luRowsRangeLow < rowsRangeHigh) || (fbRowsRangeLow < rowsRangeHigh))
    {
        if (luRowsRangeLow < rowsRangeHigh)
        {
            /* search for left free segment in vTCAMs lookup */
            cpssOsMemSet(&patternSeg, 0, sizeof(patternSeg));
            patternSeg.lookupId          = lookupId;
            patternSeg.rowsBase          = luRowsRangeLow;
            patternSeg.segmentColumnsMap = ruleMinColumnMap; /* the least possible value */
            found = prvCpssAvlPathFind(
                treeId, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                &patternSeg, path, &foundDataPtr);
            if (found == GT_FALSE)
            {
                found = prvCpssAvlPathSeek(
                    treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                    path, &foundDataPtr);
            }
            else
            {
                freeLeftSegPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)foundDataPtr;
                if (freeLeftSegPtr->lookupId != lookupId)
                {
                    found = prvCpssAvlPathSeek(
                        treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                        path, &foundDataPtr);
                }
            }
            while (1)
            {
                freeLeftSegPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)foundDataPtr;
                if ((found == GT_FALSE)
                    || (freeLeftSegPtr->lookupId != lookupId)
                    || (freeLeftSegPtr->rowsBase >= rowsRangeHigh))
                {
                    luRowsRangeLow = rowsRangeHigh;
                    break;
                }
                columns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                    freeLeftSegPtr->segmentColumnsMap, 0 /*horzBlock*/);
                if (((freeLeftSegPtr->rowsBase + freeLeftSegPtr->rowsAmount) > luRowsRangeLow)
                    && (columns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E))
                {
                    if (luRowsRangeLow < freeLeftSegPtr->rowsBase)
                    {
                        luRowsRangeLow = freeLeftSegPtr->rowsBase;
                    }
                    break;
                }
                found = prvCpssAvlPathSeek(
                    treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    path, &foundDataPtr);
            }
        }

        if ((luRowsRangeLow >= rowsRangeHigh) && (fbRowsRangeLow < rowsRangeHigh))
        {
            /* if not found in vTCAMs lookup search for left free segment in free blocks */
            cpssOsMemSet(&patternSeg, 0, sizeof(patternSeg));
            patternSeg.lookupId          = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS;
            patternSeg.rowsBase          = fbRowsRangeLow;
            patternSeg.segmentColumnsMap = ruleMinColumnMap; /* the least possible value */
            found = prvCpssAvlPathFind(
                treeId, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                &patternSeg, path, &foundDataPtr);
            if (found == GT_FALSE)
            {
                found = prvCpssAvlPathSeek(
                    treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                    path, &foundDataPtr);
            }
            else
            {
                freeLeftSegPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)foundDataPtr;
                if (freeLeftSegPtr->lookupId != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                {
                    found = prvCpssAvlPathSeek(
                        treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                        path, &foundDataPtr);
                }
            }
            while (1)
            {
                freeLeftSegPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)foundDataPtr;
                if ((found == GT_FALSE)
                    || (freeLeftSegPtr->lookupId != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                    || (freeLeftSegPtr->rowsBase >= rowsRangeHigh))
                {
                    fbRowsRangeLow = rowsRangeHigh;
                    break;
                }
                columns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                    freeLeftSegPtr->segmentColumnsMap, 0 /*horzBlock*/);
                if (((freeLeftSegPtr->rowsBase + freeLeftSegPtr->rowsAmount) > fbRowsRangeLow)
                    && (columns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E))
                {
                    if (fbRowsRangeLow < freeLeftSegPtr->rowsBase)
                    {
                        fbRowsRangeLow = freeLeftSegPtr->rowsBase;
                    }
                    break;
                }
                found = prvCpssAvlPathSeek(
                    treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    path, &foundDataPtr);
            }
        }

        if ((luRowsRangeLow >= rowsRangeHigh) && (fbRowsRangeLow >= rowsRangeHigh))
        {
            break;
        }

        /* looking for the right part of 80-byte rule for freeLeftSegPtr */
        rightLow          = startRowsRangeLow;
        if (rightLow < freeLeftSegPtr->rowsBase)
        {
            rightLow = freeLeftSegPtr->rowsBase;
        }
        rightHigh = rowsRangeHigh;
        if (rightHigh > (freeLeftSegPtr->rowsBase + freeLeftSegPtr->rowsAmount))
        {
            rightHigh = (freeLeftSegPtr->rowsBase + freeLeftSegPtr->rowsAmount);
        }
        rightFound = GT_FALSE;

        /* search for Right free segment for the left in vTCAMs lookup */

        cpssOsMemSet(&patternSeg, 0, sizeof(patternSeg));
        patternSeg.lookupId          = lookupId;
        patternSeg.rowsBase          = rightLow;
        patternSeg.segmentColumnsMap = ruleMinColumnMap; /* the least possible value */
        found = prvCpssAvlPathFind(
            treeId, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
            &patternSeg, path, &foundDataPtr);
        if (found == GT_FALSE)
        {
            found = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                path, &foundDataPtr);
        }
        else
        {
            freeRightSegPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)foundDataPtr;
            if (freeRightSegPtr->lookupId != lookupId)
            {
                found = prvCpssAvlPathSeek(
                    treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    path, &foundDataPtr);
            }
        }
        while (1)
        {
            freeRightSegPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)foundDataPtr;
            if ((found == GT_FALSE)
                || (freeRightSegPtr->lookupId != lookupId)
                || (freeRightSegPtr->rowsBase >= rightHigh))
            {
                break;
            }
            columns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                freeRightSegPtr->segmentColumnsMap, 1 /*horzBlock*/);
            if (((freeRightSegPtr->rowsBase + freeRightSegPtr->rowsAmount) > rightLow)
                && ((columns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E)
                    || (columns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E)))
            {
                rightFound = GT_TRUE;
                break;
            }
            found = prvCpssAvlPathSeek(
                treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &foundDataPtr);
        }


        if (rightFound == GT_FALSE)
        {
            /* if not found search for Right free segment for the left in free blocks */
            cpssOsMemSet(&patternSeg, 0, sizeof(patternSeg));
            patternSeg.lookupId          = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS;
            patternSeg.rowsBase          = rightLow;
            patternSeg.segmentColumnsMap = ruleMinColumnMap; /* the least possible value */
            found = prvCpssAvlPathFind(
                treeId, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                &patternSeg, path, &foundDataPtr);
            if (found == GT_FALSE)
            {
                found = prvCpssAvlPathSeek(
                    treeId, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                    path, &foundDataPtr);
            }
            else
            {
                freeRightSegPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)foundDataPtr;
                if (freeRightSegPtr->lookupId != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                {
                    found = prvCpssAvlPathSeek(
                        treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                        path, &foundDataPtr);
                }
            }
            while (1)
            {
                freeRightSegPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)foundDataPtr;
                if ((found == GT_FALSE)
                    || (freeRightSegPtr->lookupId != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                    || (freeRightSegPtr->rowsBase >= rightHigh))
                {
                    break;
                }
                columns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                    freeRightSegPtr->segmentColumnsMap, 1 /*horzBlock*/);
                if (((freeRightSegPtr->rowsBase + freeRightSegPtr->rowsAmount) > rightLow)
                    && ((columns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E)
                        || (columns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E)))
                {
                    rightFound = GT_TRUE;
                    break;
                }
                found = prvCpssAvlPathSeek(
                    treeId, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    path, &foundDataPtr);
            }
        }

        /* not found right part for the left - search the next left */
        if (rightFound == GT_FALSE) continue;

        addedSegPtr = cpssBmPoolBufGet(vTcamMngPtr->segmentsPool);
        if (addedSegPtr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "cpssBmPoolBufGet failed");
        }
        workHigh = freeRightSegPtr->rowsBase + freeRightSegPtr->rowsAmount;
        if (workHigh > rightHigh)
        {
            workHigh = rightHigh;
        }
        cpssOsMemSet(addedSegPtr, 0, sizeof(*addedSegPtr));
        addedSegPtr->ruleSize  = ruleSize;
        addedSegPtr->vTcamId   = vTcamId;
        addedSegPtr->lookupId  = lookupId;
        addedSegPtr->rowsBase  = rightLow;
        if (addedSegPtr->rowsBase < freeRightSegPtr->rowsBase)
        {
            addedSegPtr->rowsBase = freeRightSegPtr->rowsBase;
        }
        addedSegPtr->segmentColumnsMap = rule80ColumnMap;
        addedSegPtr->rowsAmount = workHigh - addedSegPtr->rowsBase;
        if (addedSegPtr->rowsAmount > neededRows)
        {
            addedSegPtr->rowsAmount = neededRows;
        }
        /*addedSegPtr->baseLogicalIndex; - don't care */
        /*addedSegPtr->rulesAmount;      - don't care */

        rc = prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree(
                tcamCfgPtr->tcamSegCfgPtr, lookupId,
                addedSegPtr->rowsBase, addedSegPtr->rowsAmount, addedSegPtr->segmentColumnsMap);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree failed\n");
        }

        /* Add it to the vTcam's segmentsTree */
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tcamCfgPtr->segmentsTree, addedSegPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed for segments Tree\n");
        }

        neededRows -= addedSegPtr->rowsAmount;
        if (neededRows == 0)
        {
            return GT_OK;
        }
    }

    /* We haven't found the needed space */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
 * @internal prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs function
 * @endinternal
 *
 * @brief Used to fill holes in between 2 supplied segments(seg1, seg2)
 * this happens when a vTCAM is created with ruleAdds for non-contiguos ruleIds & some empty spaces(holes) remain in between
 */
GT_STATUS prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs
(
   IN  GT_U32      vTcamMngId,
   IN  GT_U32      vTcamId,
   IN  GT_U32      numRules,
   IN  GT_U32      ruleSize,
   IN  GT_VOID    *seg1,
   IN  GT_VOID    *seg2
)
{
    GT_STATUS                                          rc;
    GT_BOOL                                            secRun       = GT_FALSE;
    GT_U32                                             segRuleSpace = 0;
    GT_U32                                             rulesPerRow  = 0;
    GT_COMP_RES                                        r1 = 0, r2 = 0;
    PRV_CPSS_AVL_TREE_ITERATOR_ID                      iterPtr; /* avl tree iterator */
    GT_VOID                                           *dataPtr; /* pointer to avl node data */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC   *segPtr1 = seg1, *segPtr2 = seg2, *freeSegPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC    *workSegPtr;
    GT_U32                                             ruleSpaceAvailable = 0, startLogicalIndex=0, lookupId = 0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *vTcamMngPtr;
    PRV_CPSS_AVL_TREE_ID                               treeId;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR        tcamCfgPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC allocRequest;
    GT_COMP_RES (*lkpCmpFn) ( GT_VOID   *data_ptr1,  GT_VOID  *data_ptr2) = NULL;
    GT_U32                                             emptyMap;
    GT_U32                                             usableColumnMap;
    GT_U32                                             neededRows;
    GT_BOOL                                            haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);

    if (ruleSize == 8)
    {
        return prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs80bytes(
           vTcamMngId, vTcamId, numRules, seg1, seg2);
    }

    emptyMap = 0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        emptyMap, 0, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E);
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
        emptyMap, 1, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    tcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];

    treeId      = vTcamMngPtr->tcamSegCfg.segFreeLuTree;

    lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(tcamCfgPtr->tcamInfo.clientGroup, tcamCfgPtr->tcamInfo.hitNumber);

    if (haVtcamDebug)
    {
        cpssOsPrintf("prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs: seg1=0x%x, seg2=0x%x\r\n", seg1, seg2);
    }
    if (segPtr1)
    {
        startLogicalIndex = segPtr1->baseLogicalIndex + segPtr1->rulesAmount;
    }

get_free_seg_loop_start:
    /* Let us iterate the freeSegments tree, and find a free segment b/n Seg1 and Seg2 */
    /* We will use the same comparator function used to create the segmentsTree        */
    iterPtr = NULL;

    /* 2 options -> prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompare<FreeSeg>Func */
    if (secRun == GT_TRUE)
       lkpCmpFn = prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc;
    else
       lkpCmpFn = prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFreeSegFunc;

    do {
        dataPtr = prvCpssAvlGetNext(treeId, &iterPtr);
        freeSegPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)dataPtr;
        if (!freeSegPtr) break;

        if ((secRun == GT_FALSE) && (freeSegPtr->lookupId != lookupId))
        {
            continue;
        }
        if ((secRun != GT_FALSE) &&
            (freeSegPtr->lookupId != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS))
        {
            continue;
        }

        if (   ( !segPtr1 && segPtr2
                    && GT_SMALLER == lkpCmpFn(freeSegPtr, segPtr2)
               )
                || ( segPtr1 && segPtr2
                    &&  GT_SMALLER == (r1 = lkpCmpFn(segPtr1, freeSegPtr))
                    &&  GT_SMALLER == (r2 = lkpCmpFn(freeSegPtr, segPtr2))
                   )
                || ( !segPtr2 && segPtr1
                    && GT_SMALLER == (r2 = lkpCmpFn(segPtr1, freeSegPtr))
                   )
                || ( !segPtr1 && !segPtr2
                    && tcamCfgPtr->usedRulesAmount == 0
                    && ((freeSegPtr->lookupId == lookupId && secRun == GT_FALSE)
                        || (freeSegPtr->lookupId == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS && secRun == GT_TRUE))
                   )
           )
        {
            segRuleSpace = 0;
            rulesPerRow  = 0;
            rc = prvCpssDxChVirtualTcamHaGetNumOfRulesSpaceAvailable(
                vTcamMngPtr->tcamSegCfg.deviceClass,
                ruleSize, freeSegPtr, &segRuleSpace, &rulesPerRow);
            if (GT_OK != rc) CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaGetNumOfRulesSpaceAvailable failed\n");

            if (haVtcamDebug)
            {
                cpssOsPrintf ("freeSeg lookup ok r1=%d, r2=%d -- %d -- %d----[%d---%d---0x%x]\r\n",
                     r1, r2, segRuleSpace, rulesPerRow, freeSegPtr->rowsBase, freeSegPtr->rowsAmount, freeSegPtr->segmentColumnsMap);
            }
            if (segRuleSpace == 0) continue;

            usableColumnMap = prvCpssDxChVirtualTcamHaGetUsableSegmentMap(
                vTcamMngPtr->tcamSegCfg.deviceClass,
                ruleSize, freeSegPtr->segmentColumnsMap);

            if (usableColumnMap == emptyMap) continue;

            neededRows = (numRules - ruleSpaceAvailable + (rulesPerRow - 1)) / rulesPerRow;

            /* the new segment verified before removing an old one */
            workSegPtr = cpssBmPoolBufGet(vTcamMngPtr->segmentsPool);
            if (workSegPtr == NULL)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "cpssBmPoolBufGet failed");
            }
            cpssOsMemSet(workSegPtr, 0, sizeof(*workSegPtr));
            workSegPtr->rowsBase          = freeSegPtr->rowsBase;
            workSegPtr->rowsAmount        = freeSegPtr->rowsAmount;
            workSegPtr->vTcamId           = vTcamId;
            workSegPtr->lookupId          = lookupId;
            workSegPtr->baseLogicalIndex  = startLogicalIndex + ruleSpaceAvailable;
            workSegPtr->ruleSize          = ruleSize;
            workSegPtr->rulesAmount       = (neededRows * rulesPerRow);
            workSegPtr->segmentColumnsMap = usableColumnMap;
            if (workSegPtr->rowsAmount > neededRows)
            {
                workSegPtr->rowsAmount    = neededRows;
            }

            rc = prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree(
                    tcamCfgPtr->tcamSegCfgPtr, lookupId,
                    workSegPtr->rowsBase, workSegPtr->rowsAmount, workSegPtr->segmentColumnsMap);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree failed\n");
            }

            /* recalculate iterator - the free segments trees updated */
            {
                GT_BOOL found;
                PRV_CPSS_AVL_TREE_PATH     path;
                found = prvCpssAvlPathFind(
                    treeId, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                    workSegPtr, path, &dataPtr);
                if (found == GT_FALSE)
                {
                    iterPtr = NULL;
                }
                else
                {
                    dataPtr = prvCpssAvlIteratorForNodeGet(
                        treeId, dataPtr, &iterPtr);
                    if (dataPtr == NULL)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssAvlIteratorForNodeGet failed\n");
                    }
                }
            }

            /*check for consistency of segment lookupId with lookupIdHA*/
            rc = prvCpssDxChVirtualTcamHaBlockInfoLookupIdVerify(vTcamMngId, workSegPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamHaBlockInfoLookupIdVerify failed\n");
            }

            prvCpssDxChVirtualTcamHaDbgPrintSegment("whole free seg taken", workSegPtr);

            /* Add it to the vTcam's segmentsTree */
            rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tcamCfgPtr->segmentsTree, workSegPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed for segments Tree\n");
            }

            allocRequest.ruleSize        = workSegPtr->ruleSize;

            allocRequest.ruleStartColumnsBitmap =
                prvCpssDxChVirtualTcamDbSegmentTableRuleAlignmentBitmapGet(
                tcamCfgPtr->tcamSegCfgPtr->deviceClass,  allocRequest.ruleSize);

            ruleSpaceAvailable += segRuleSpace;

            if (ruleSpaceAvailable >= numRules)
            {
                return GT_OK;
            } else {
                /* Begin freeSegments tree traversal from head */
                iterPtr = NULL;
                if (haVtcamDebug)
                {
                    cpssOsPrintf("\nRESTART from HEAD\n");
                }
            }

        } else {
            if (haVtcamDebug)
            {
                cpssOsPrintf ("freeSeg lookup failed r1=%d, r2=%d\r\n", r1, r2);
                prvCpssDxChVirtualTcamHaDbgPrintSegment("free Seg", freeSegPtr);
            }
        }

    } while (dataPtr != NULL);

    if (secRun != GT_TRUE)
    {
        /* Don't quit yet. Go hunting again by ignoring the lookupId of freeSegments */
        secRun = GT_TRUE;
        goto get_free_seg_loop_start;
    }

    /* We haven't found the needed space */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
 * @internal prvCpssDxChVirtualTcamHaLogicalIndexVtcamUsedRulesMark function
 * @endinternal
 *
 * @brief Called only in case of Logical Index Mode - to mark used rules in bitmap.
 */
GT_STATUS prvCpssDxChVirtualTcamHaLogicalIndexVtcamUsedRulesMark
(
    IN  GT_U32      vTcamMngId,
    IN  GT_U32      vTcamId
)
{
    GT_STATUS                                          rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *vTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *vTcamInfoPtr;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT               ruleSize;
    GT_U32                                             numOfEntries = 0, count = 0;
    GT_U32                                             i, j;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR    *entriesArr = NULL;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    vTcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(!vTcamInfoPtr)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "vtcam %d not found", vTcamId);
    }

    /* Should be doing this for Logical index driven vTCAMs only */
    if (vTcamInfoPtr->tcamInfo.ruleAdditionMethod != CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E)
    {
        return GT_OK;
    }

    /* Get the entries corres. to vTCAM */
    rc = prvCpssDxChVirtualTcamHaDbVTcamEntriesNumAndSizeGet(vTcamMngId, vTcamId, &numOfEntries, &ruleSize);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc," prvCpssDxChVirtualTcamHaDbVTcamEntriesNumAndSizeGet failed\n");
    }

    if (numOfEntries == 0)
    {
        return GT_OK;
    }

    entriesArr = cpssOsMalloc(numOfEntries*sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR));
    if(!entriesArr) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "entriesArr memory allocation failed\n");

    rc = prvCpssDxChVirtualTcamHaDbVTcamEntriesGet(vTcamMngId, vTcamId, numOfEntries, entriesArr);
    if (rc != GT_OK)
    {
        cpssOsFree(entriesArr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamHaDbVTcamEntriesGet failed\n");
    }

    for (i=0; i<vTcamInfoPtr->rulesAmount; i++)
    {
        /* Locate the rule in the DB and update the field
           priority_LogicalIndex to 'segPtr->baseLogicalIndex + i' */
        for (j=0; j<numOfEntries; j++)
        {
            if(vTcamInfoPtr->rulePhysicalIndexArr[i] == entriesArr[j]->physicalIndex)
            {
                entriesArr[j]->priority_logicalIndex = i;

                /* Update the used rules bitmap over here */
                vTcamInfoPtr->usedRulesBitmapArr[i>>5] |= (1<<(i & 0x1f));
                count++;
            }
        }
    }

    cpssOsFree(entriesArr);
    if (count != vTcamInfoPtr->usedRulesAmount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "usedRulesAmount not matched for VTCAM %d",vTcamId);
    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChVirtualTcamHaPriorityVtcamTreesCreate function
 * @endinternal
 *
 * @brief Called only in case of Priority Mode - to create all the trees required to store priority info.
 */
GT_STATUS prvCpssDxChVirtualTcamHaPriorityVtcamTreesCreate
(
   IN  GT_U32      vTcamMngId,
   IN  GT_U32      vTcamId
)
{
    GT_STATUS                                          rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *vTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *vTcamInfoPtr;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT               ruleSize;
    GT_U32                                             numOfEntries = 0, count = 0;
    GT_U32                                             rangeStart, rangeEnd, prio, i, j;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR    *entriesArr = NULL;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   prioEntry;
    GT_BOOL                                            haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    vTcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(!vTcamInfoPtr)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "vtcam %d not found", vTcamId);
    }

    /* Should be doing this for Priority vTCAMs only*/
    if (vTcamInfoPtr->tcamInfo.ruleAdditionMethod != CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        if (haVtcamDebug)
        {
            cpssOsPrintf("prvCpssDxChVirtualTcamHaPriorityVtcamTreesCreate not needed for non-priority vTCAM. return GT_OK\n");
        }
        return GT_OK;
    }

    /* Get the entries corres. to vTCAM */
    rc = prvCpssDxChVirtualTcamHaDbVTcamEntriesNumAndSizeGet(vTcamMngId, vTcamId, &numOfEntries, &ruleSize);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc," prvCpssDxChVirtualTcamHaDbVTcamEntriesNumAndSizeGet failed\n");
    }

    if (numOfEntries == 0)
    {
        if (haVtcamDebug)
        {
            cpssOsPrintf("numOfEntries is 0. vTcamId=%d.\n", vTcamId);
        }
        return GT_OK;
    }

    entriesArr = cpssOsMalloc(numOfEntries*sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR));
    if(!entriesArr) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "entriesArr memory allocation failed\n");

    rc = prvCpssDxChVirtualTcamHaDbVTcamEntriesGet(vTcamMngId, vTcamId, numOfEntries, entriesArr);
    if (rc != GT_OK)
    {
        cpssOsFree(entriesArr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamHaDbVTcamEntriesGet failed\n");
    }

    for (i=0; i<vTcamInfoPtr->rulesAmount; i++)
    {
        /* Locate the rule in the DB and update the field
           priority_LogicalIndex to 'segPtr->baseLogicalIndex + i' */
        for (j=0; j<numOfEntries; j++)
        {
            if(vTcamInfoPtr->rulePhysicalIndexArr[i] == entriesArr[j]->physicalIndex)
            {
                entriesArr[j]->priority_logicalIndex = i;
                rc = priorityNewRuleIdAddedDbUpdate(vTcamMngId, vTcamId, entriesArr[j]->ruleId, i);
                if (rc != GT_OK)
                {
                    cpssOsFree(entriesArr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERR: priorityNewRuleIdAddedDbUpdate(vTCAM-%d, ruleId=%d, logicalIndex=%d) in HA DB\n",
                                                        vTcamId, entriesArr[j]->ruleId, i);
                }

                /* Update the used rules bitmap over here */
                vTcamInfoPtr->usedRulesBitmapArr[i>>5] |= (1<<(i & 0x1f));
                count++;
            }
        }
    }

    if(count != vTcamInfoPtr->usedRulesAmount)
    {
        cpssOsFree(entriesArr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "usedRulesAmount not matched for VTCAM %d",vTcamId);
    }

    /* Let us bubble sort the entriesArr based on priority_logicalIndex */
    for (i=0; i<numOfEntries-1; i++)
        for (j=0; j<numOfEntries-1-i; j++)
           if (entriesArr[j]->priority_logicalIndex > entriesArr[j+1]->priority_logicalIndex)
           {
               /* Swap */
               PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR tmp = entriesArr[j];
               entriesArr[j] = entriesArr[j+1];
               entriesArr[j+1] = tmp;
           }

    /* Now, iterate through the entries and create the priority ranges */
    rangeStart = 0xffffffff;
    rangeEnd   = 0xffffffff;
    prio       = 0xffffffff;
    prioEntry.baseLogIndex = 0xffffffff;

    for (i=0; i<numOfEntries; i++)
    {
        if (entriesArr[i]->priority != prio)
        {
            /* Start of new range */
            /* First populate the priority tree with already created range */
            if (prio != 0xffffffff)
            {
                prioEntry.priority = prio;
                prioEntry.baseLogIndex = rangeStart;
                prioEntry.rangeSize = rangeEnd - rangeStart + 1;

                rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(vTcamMngPtr, vTcamInfoPtr, &prioEntry);
                if (rc != GT_OK)
                {
                    cpssOsFree(entriesArr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamDbPriorityTableEntrySet failed\n");
                }
            }

            prio = entriesArr[i]->priority;
            rangeStart = rangeEnd = entriesArr[i]->priority_logicalIndex;
        }
        else
        {
            /* same priority not first entry */
            /* range can be not contiguous   */
            if (entriesArr[i]->priority_logicalIndex > rangeEnd)
            {
                rangeEnd = entriesArr[i]->priority_logicalIndex;
            }
        }
    }

    /* To handle the case last priority */
    if (0xffffffff != rangeStart)
    {
        prioEntry.priority = prio;
        prioEntry.baseLogIndex = rangeStart;
        prioEntry.rangeSize = rangeEnd - rangeStart + 1;

        rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(vTcamMngPtr, vTcamInfoPtr, &prioEntry);
        if (rc != GT_OK)
        {
            cpssOsFree(entriesArr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamDbPriorityTableEntrySet failed\n");
        }
    }

    cpssOsFree(entriesArr);
    return GT_OK;
}

/**
 * @internal prvCpssDxChVirtualTcamHaFillEmptyVtcams function
 * @endinternal
 *
 * @brief Function handles empty vTcams - resize to guaranteed#rules
 */
GT_STATUS prvCpssDxChVirtualTcamHaFillEmptyVtcams
(
   IN  GT_U32      vTcamMngId,
   IN  GT_U32      vTcamId
)
{
    GT_STATUS                                         rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC               *vTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR       vTcamInfoPtr;
    GT_U16                                           *oldRulePhysicalIndexArr;
    GT_BOOL                                           haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    vTcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(!vTcamInfoPtr)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "vtcam %d not found", vTcamId);
    }

    if (vTcamInfoPtr->usedRulesAmount != 0)
    {
        return GT_OK;
    }

    rc = prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs(vTcamMngId, vTcamId,
            vTcamInfoPtr->tcamInfo.guaranteedNumOfRules, vTcamInfoPtr->tcamInfo.ruleSize+1, NULL, NULL);
    if (GT_OK != rc)
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs failed \n");

    wrPrintSegmentsTree(vTcamInfoPtr->segmentsTree, haVtcamDebug);

    /* 1st Merge - Vertical */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(&vTcamMngPtr->tcamSegCfg, vTcamInfoPtr->segmentsTree);
    if (GT_OK != rc)
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge failed\n");

    /* 2nd Merge - Horizontal */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(&vTcamMngPtr->tcamSegCfg, vTcamInfoPtr->segmentsTree);
    if (GT_OK != rc)
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge failed\n");

    /* Update rule amounts */
    prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts(
            &vTcamMngPtr->tcamSegCfg, vTcamInfoPtr->segmentsTree);

    prvCpssDxChVirtualTcamDbBlocksReservedSpaceRecalculate(
        &vTcamMngPtr->tcamSegCfg, vTcamInfoPtr->tcamInfo.clientGroup, vTcamInfoPtr->tcamInfo.hitNumber);

    segmentTableVTcamBaseLogicalIndexUpdate(vTcamInfoPtr);

    if (vTcamInfoPtr->rulesAmount)
    {
        oldRulePhysicalIndexArr = vTcamInfoPtr->rulePhysicalIndexArr;
        /* logical to physical conversion array */
        vTcamInfoPtr->rulePhysicalIndexArr =
            (GT_U16*)cpssOsMalloc(sizeof(GT_U16) * vTcamInfoPtr->rulesAmount);
        if (vTcamInfoPtr->rulePhysicalIndexArr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "rulePhysicalIndexArr allocation failed\n");
        }
        prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate(
                vTcamInfoPtr->tcamSegCfgPtr,
                vTcamInfoPtr->segmentsTree,
                vTcamInfoPtr->rulePhysicalIndexArr);

        if (NULL != oldRulePhysicalIndexArr)
            cpssOsFree(oldRulePhysicalIndexArr);
    }

    if(vTcamInfoPtr->tcamInfo.guaranteedNumOfRules > vTcamInfoPtr->rulesAmount )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "cpssDxChVirtualTcam Resize is still needed! ret failure.\n");
    }

    if(haVtcamDebug)
    {
        cpssOsPrintf("After Merge segmentsTree- rc=%d\n", rc);
        wrPrintSegmentsTree(vTcamInfoPtr->segmentsTree, haVtcamDebug);
    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChVirtualTcamHaFillHolesAndMergeSegs function
 * @endinternal
 *
 * @brief Function handles 3 major tasks: filling holes in vTCAM, merging the segments(that can be merged) and
 * generates rulePhysicalIndexArr
 */
GT_STATUS prvCpssDxChVirtualTcamHaFillHolesAndMergeSegs
(
   IN  GT_U32      vTcamMngId,
   IN  GT_U32      vTcamId
)
{
    GT_STATUS                                         rc;
    PRV_CPSS_AVL_TREE_ID                              treeIdPtr;
    PRV_CPSS_AVL_TREE_ITERATOR_ID                     iterPtr; /* avl tree iterator */
    GT_VOID                                          *dataPtr; /* pointer to avl node data */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC  *segPtr1, *segPtr2; /* pointer to segmentsTree node data */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC               *vTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR       vTcamInfoPtr;
    GT_U16                                           *oldRulePhysicalIndexArr;
    GT_BOOL                                           haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    vTcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(!vTcamInfoPtr)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "vtcam %d not found", vTcamId);
    }

    treeIdPtr = vTcamInfoPtr->segmentsTree;
    if (!treeIdPtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "segments Tree not found\n");

    iterPtr = NULL;

    dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);

    /* empty vTCAM - will be handled later */
    if (dataPtr == NULL)
    {
        return GT_OK;
    }
    else
    {
        segPtr1     = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)dataPtr;
        /* Check for hole at start */
        if (segPtr1->baseLogicalIndex > 0)
        {
            /* We have a hole at the start
             * Properties of the hole segment?
             * No. of rules = segPtr1->baseLogicalIndex
             * Rules size   = known
             * Lookup order = >LO(NULL) && <LO(segPtr1)
             */
            rc = prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs (vTcamMngId
                    , vTcamId
                    , segPtr1->baseLogicalIndex, segPtr1->ruleSize, NULL, segPtr1);

            if (GT_OK != rc)
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs failed\n");
        }

        /* Iterating over all tree elements */
        do {
            dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
            if (dataPtr == NULL) break;

            segPtr2 = dataPtr;

            /* Got 2 segments */
            /* Check if we have a hole */
            if (segPtr1->baseLogicalIndex + segPtr1->rulesAmount < segPtr2->baseLogicalIndex)
            {
                /* We have a hole */
                rc = prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs (vTcamMngId
                        , vTcamId, segPtr2->baseLogicalIndex - (segPtr1->baseLogicalIndex + segPtr1->rulesAmount)
                        , segPtr1->ruleSize, segPtr1, segPtr2);

                if (GT_OK != rc)
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs failed\n");

            }

            segPtr1 = segPtr2;
            segPtr2 = NULL;

        } while (dataPtr != NULL);

        /* Do we have a hole at the end ? */
        if (segPtr1->baseLogicalIndex + segPtr1->rulesAmount < vTcamInfoPtr->tcamInfo.guaranteedNumOfRules)
        {
            rc = prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs (vTcamMngId
                    , vTcamId
                    , vTcamInfoPtr->tcamInfo.guaranteedNumOfRules - (segPtr1->baseLogicalIndex + segPtr1->rulesAmount)
                    , segPtr1->ruleSize, segPtr1, NULL);

            if (GT_OK != rc)
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaGetFreeSegBetweenTwoSegs failed\n");
        }
    }

    wrPrintSegmentsTree(vTcamInfoPtr->segmentsTree, haVtcamDebug);

    /* 1st Merge - Vertical */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(&vTcamMngPtr->tcamSegCfg, vTcamInfoPtr->segmentsTree);
    if (GT_OK != rc)
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge failed\n");

    /* 2nd Merge - Horizontal */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(&vTcamMngPtr->tcamSegCfg, vTcamInfoPtr->segmentsTree);
    if (GT_OK != rc)
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge failed\n");

    /* Update rule amounts */
    prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts(
            &vTcamMngPtr->tcamSegCfg, vTcamInfoPtr->segmentsTree);

    prvCpssDxChVirtualTcamDbBlocksReservedSpaceRecalculate(
        &vTcamMngPtr->tcamSegCfg, vTcamInfoPtr->tcamInfo.clientGroup, vTcamInfoPtr->tcamInfo.hitNumber);

    segmentTableVTcamBaseLogicalIndexUpdate(vTcamInfoPtr);

    if (vTcamInfoPtr->rulesAmount)
    {
        oldRulePhysicalIndexArr = vTcamInfoPtr->rulePhysicalIndexArr;
        /* logical to physical conversion array */
        vTcamInfoPtr->rulePhysicalIndexArr =
            (GT_U16*)cpssOsMalloc(sizeof(GT_U16) * vTcamInfoPtr->rulesAmount);
        if (vTcamInfoPtr->rulePhysicalIndexArr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "rulePhysicalIndexArr allocation failed\n");
        }
        prvCpssDxChVirtualTcamDbSegmentTableLogToPhyIndexTabGenerate(
                vTcamInfoPtr->tcamSegCfgPtr,
                vTcamInfoPtr->segmentsTree,
                vTcamInfoPtr->rulePhysicalIndexArr);

        if (NULL != oldRulePhysicalIndexArr)
            cpssOsFree(oldRulePhysicalIndexArr);
    }

    if(vTcamInfoPtr->tcamInfo.guaranteedNumOfRules > vTcamInfoPtr->rulesAmount )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "cpssDxChVirtualTcam Resize is still needed! ret failure.\n");
    }

    if(haVtcamDebug)
    {
        cpssOsPrintf("After Merge segmentsTree- rc=%d\n", rc);
        wrPrintSegmentsTree(vTcamInfoPtr->segmentsTree, haVtcamDebug);
    }

    /* allocate usedRulesBitmapArr */
    {
        GT_U32 bmpSize = (((vTcamInfoPtr->rulesAmount + 31) / 32) * sizeof(GT_U32));
        GT_U32 *old_usedRulesBitmapArr = vTcamInfoPtr->usedRulesBitmapArr;

        vTcamInfoPtr->usedRulesBitmapArr = (GT_U32*)cpssOsMalloc(bmpSize);
        if (vTcamInfoPtr->usedRulesBitmapArr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(vTcamInfoPtr->usedRulesBitmapArr,0, bmpSize);

        vTcamInfoPtr->usedRulesBitmapArrSizeHa = bmpSize;

        if (NULL != old_usedRulesBitmapArr)
        {
            cpssOsFree(old_usedRulesBitmapArr);
        }
    }

    return GT_OK;
}

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
   OUT GT_U32     *_numEntriesPtr,
   OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT *_ruleSizePtr
)
{
    GT_U32                                     i, j, count=0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC        *vTcamMngPtr;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT               tcamRuleSize;

    /* Get the rule size of the vTCAM from the DB */
    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    ruleSize = vTcamMngPtr->vTcamCfgPtrArr[vTcamId]->tcamInfo.ruleSize;
    tcamRuleSize = tcamRuleSizeMap[ruleSize];

    if (!_numEntriesPtr || !_ruleSizePtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "numEntries or ruleSize not found\n");

    for(j=PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_0_E; j<PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_MAX_E; ++j)
    {
        if (!vTcamMngPtr->vTcamHaDb[tcamRuleSize][j].rulesArr)
            continue;

        for(i=0; i< vTcamMngPtr->vTcamHaDb[tcamRuleSize][j].numRules; i++)
        {
            if( vTcamMngPtr->vTcamHaDb[tcamRuleSize][j].rulesArr[i].vTcamId == vTcamId
                    && vTcamMngPtr->vTcamHaDb[tcamRuleSize][j].rulesArr[i].found == 1)
                count++;
        }
    }

    *_numEntriesPtr = count;
    *_ruleSizePtr   = ruleSize;

    return GT_OK;
}

/**
 * @internal prvCpssDxChVirtualTcamHaDbVTcamEntriesGet function
 * @endinternal
 *
 * @brief Function populates all the entries corresponding to the vTcamId in entriesPtrArr
 */
GT_STATUS prvCpssDxChVirtualTcamHaDbVTcamEntriesGet
(
   IN  GT_U32      vTcamMngId,
   IN  GT_U32      vTcamId,
   IN  GT_U32      numEntries,
   OUT PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR *entriesPtrArr
)
{
    GT_U32                                     i, k, j=0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC        *vTcamMngPtr;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT       ruleSize;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT               tcamRuleSize;

    /* Get the rule size of the vTCAM from the DB */
    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    ruleSize = vTcamMngPtr->vTcamCfgPtrArr[vTcamId]->tcamInfo.ruleSize;
    tcamRuleSize = tcamRuleSizeMap[ruleSize];

    for(k=PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_0_E; k<PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_MAX_E; ++k)
    {
        if (!vTcamMngPtr->vTcamHaDb[tcamRuleSize][k].rulesArr)
          continue;

        for(i=0; i < vTcamMngPtr->vTcamHaDb[tcamRuleSize][k].numRules; i++)
        {
             if(    vTcamMngPtr->vTcamHaDb[tcamRuleSize][k].rulesArr[i].vTcamId == vTcamId
                 && vTcamMngPtr->vTcamHaDb[tcamRuleSize][k].rulesArr[i].found == 1)
             {
                entriesPtrArr[j]= &(vTcamMngPtr->vTcamHaDb[tcamRuleSize][k].rulesArr[i]);
                if(++j >= numEntries) return GT_OK;
             }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "prvCpssDxChVirtualTcamHaDbVTcamEntriesGet failed\n");
}

/*******************************************************/


/*************************TcamDumps*********************/

/* PATTERN used for debug purposes - to check how many bytes are changed etc. */
#define PATTERN {0xABCDEFAB, 0xABCDEFAB, 0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,\
                0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB}

/**
 * @internal prvCpssDxChVirtualTcamHaLookupIdGet function
 * @endinternal
 *
 * @brief Used to get the lookupId corresponding to a physical Index in TCAM(lookup set using ClientGroupHitNum Register)
 */
GT_STATUS prvCpssDxChVirtualTcamHaLookupIdGet
(
    IN GT_U8   devNum,
    IN  GT_U32 phyIdx,
    OUT GT_U32 *lookupId
)
{
    GT_STATUS                     rc;
    GT_U32                        floorIndex;
    GT_U32                        min, baseRow, baseColumn;
    GT_U32                        numBanksForHitNumGranularity = 0;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];

    cpssOsMemSet(floorInfoArr, 0, sizeof(floorInfoArr));

    baseRow     = phyIdx/12;
    baseColumn  = phyIdx%12;
    floorIndex  = baseRow / 256;
    numBanksForHitNumGranularity =  /* in BC2/BC3 = 2 , in BobK/Aldrin = 6 */
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;

    rc = cpssDxChTcamIndexRangeHitNumAndGroupGet(
         devNum, floorIndex, floorInfoArr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChTcamIndexRangeHitNumAndGroupGet failed\n");
    }

    if (baseColumn < 6)
    {
        /* lower half floor */
        min = 0;
    }
    else
    {
        /* higher half floor */
        min = numBanksForHitNumGranularity/2;
    }

    *lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(floorInfoArr[min].group, floorInfoArr[min].hitNum);
    PRV_CPSS_DXCH_VTCAM_LOOKUPID_IDX(*lookupId);

    return GT_OK;
}

/**
 * @internal prvCpssPatternMaskCompare function
 * @endinternal
 *
 * @brief compare two pairs of pattern and mask according to rule size.
 *
 * @param[in] parrernArr1   - array of pattern1 data
 * @param[in] maskArr1      - array of mask1 data
 * @param[in] parrernArr2   - array of pattern2 data
 * @param[in] maskArr2      - array of mask1 data
 * @param[in] vtcamRuleSize - rule size
 *
 * @retval 0 - if parrernArr1 == parrernArr2 AND maskArr1 == maskArr2
 * @retval 1 - otherwise.
 */
static GT_U32 prvCpssPatternMaskCompare
(
    IN const GT_U32                             parrernArr1[],
    IN const GT_U32                             maskArr1[],
    IN const GT_U32                             parrernArr2[],
    IN const GT_U32                             maskArr2[],
    IN CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT     vtcamRuleSize
)
{
    GT_U32 ruleCmpSizeInBytes; /* size of pattern/mask for comparison */
    GT_U32 numOfFullWords;    /* number of full words for comparison */
    GT_U32 ii;                /* iterator */

    ruleCmpSizeInBytes = (vtcamRuleSize + 1) * 10;

    numOfFullWords = ruleCmpSizeInBytes / 4;

    for (ii = 0; ii < numOfFullWords; ii++)
    {
        /* use XOR for comparison. XOR result is != 0 if operands are different. */
        if ((parrernArr1[ii] ^ parrernArr2[ii]) ||
            (maskArr1[ii] ^ maskArr2[ii]))
        {
            return 1;
        }
    }

    /* need to compare 2 LSBytes for rule sizes 10,30,50 bytes */
    if (ruleCmpSizeInBytes & 0x3)
    {
        /* use XOR for comparison. XOR result is != 0 if operands are different. */
        if (((parrernArr1[numOfFullWords] ^ parrernArr2[numOfFullWords]) & 0xFFFF) ||
            ((maskArr1[numOfFullWords] ^ maskArr2[numOfFullWords]) & 0xFFFF))
        {
            return 1;
        }
    }

    /* patterns and masks are equal */
    return 0;
}

/**
 * @internal prvCpssDxChVirtualTcamHaDbCreate function
 * @endinternal
 *
 * @brief Create vTcamHaDb - called as part of vTCAM Manager Init
 *
 * @param validOnly - 0 Populate HA DB with all configured rules from TCAM (i.e. diff. from default in TCAM).
 *                  - 1 Populate only valid rules
 */
GT_STATUS prvCpssDxChVirtualTcamHaDbCreate
(
    IN GT_U8  devNum,
    IN GT_U32 vTcamMngId,
    IN GT_U32 validOnly
)
{
    GT_STATUS                             rc = GT_OK;
    GT_U32                                portGroupId;
    GT_U32                                i, j, totalEntries, lookupId;
    GT_U32                                counter = 0;
    GT_PORT_GROUPS_BMP                    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT          tcamRuleSize;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT  vtcamRuleSize;
    GT_U32                                defHwMask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    GT_U32                                defHwPattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC   *vTcamMngPtr;
    GT_U32                                activatedfloorNumber;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_ENTRY_PTR
                                          haDbEntryPtr = NULL;
    GT_U32                                ruleCopySizeInBytes; /* size of pattern/mask for copy */

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if (!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    portGroupsBmp = vTcamMngPtr->portGroupBmpArr[devNum];

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum,portGroupsBmp,portGroupId);

    cpssOsMemSet (defHwMask, 0, sizeof(defHwMask));
    cpssOsMemSet (defHwPattern, 0, sizeof(defHwPattern));

    prvCpssDxChVirtualTcamHaDbClear(vTcamMngId);

    /* get number of powered UP TCAM floors */
    rc = cpssDxChTcamPortGroupActiveFloorsGet (devNum, portGroupsBmp, &activatedfloorNumber);
    if (rc != GT_OK)
    {
        return rc;
    }

    totalEntries = CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS * activatedfloorNumber;

    for (i=0; i<totalEntries; ++i)
    {
        GT_U32       mask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS] = PATTERN;   /* mask in HW format */
        GT_U32       pattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS] = PATTERN;
        GT_U32       action[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS] = {0,};
        GT_U32       entryNumber = 0;                                        /* Rule's index in TCAM */
        GT_U32       dummy;
        GT_BOOL      validFound = GT_FALSE;                                  /* is rule found valid */

        /* convert rule global index to TCAM entry number */
        rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum, i, &entryNumber, &dummy);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* read action */
        rc = prvCpssDxChPortGroupReadTableEntry (devNum,
                portGroupId,
                CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                (entryNumber/2),
                action);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChTcamPortGroupRuleRead (devNum, portGroupsBmp, i, &validFound, &tcamRuleSize, pattern, mask);

        /* validOnly if turned on populates only valid rules; currently called with 0, hence accepts both valid/invalid rules */
        if (rc != GT_OK || (!validFound && validOnly))
        {
            continue;
        }

        vtcamRuleSize = vtcamRuleSizeMap[tcamRuleSize];

        ruleCopySizeInBytes = (vtcamRuleSize + 1) * 10;

        /* need to compare/copy full words for rule sizes 10,30,50 bytes
           to avoid BE related bugs */
        if (ruleCopySizeInBytes & 0x3)
        {
            /* align size to full words */
            ruleCopySizeInBytes +=2;
        }

        if ( (prvCpssPatternMaskCompare(defHwPattern, defHwMask,
                                        pattern, mask,vtcamRuleSize) != 0)
             || validFound ) /* All valid rules accepted - including rule 0 and mask 0 */
        {
            rc = prvCpssDxChVirtualTcamHaLookupIdGet(devNum, i, &lookupId);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaLookupIdGet failed for Idx %d \n", i);
            }

            haDbEntryPtr = &vTcamMngPtr->vTcamHaDb[tcamRuleSize][lookupId];

            if (!haDbEntryPtr->rulesArr)
            {
                haDbEntryPtr->rulesArr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR)
                    cpssOsMalloc(sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_STC) * totalEntries);

                if (!haDbEntryPtr->rulesArr)
                {
                    for (tcamRuleSize=CPSS_DXCH_TCAM_RULE_SIZE_10_B_E; tcamRuleSize<=CPSS_DXCH_TCAM_RULE_SIZE_80_B_E; ++tcamRuleSize)
                        for (j=PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_0_E; j<PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_MAX_E; ++j)
                            if (vTcamMngPtr->vTcamHaDb[tcamRuleSize][j].rulesArr)
                            {
                                cpssOsFree(vTcamMngPtr->vTcamHaDb[tcamRuleSize][j].rulesArr);
                                vTcamMngPtr->vTcamHaDb[tcamRuleSize][j].rulesArr = NULL;
                            }

                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "malloc for TCAM Manager HA DB failed\n");
                }

                cpssOsMemSet (haDbEntryPtr->rulesArr, 0, (sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_STC) * totalEntries));

                haDbEntryPtr->numRules = 0;
            }

            counter = haDbEntryPtr->numRules;
            haDbEntryPtr->rulesArr[counter].physicalIndex = i;

            cpssOsMemCpy (haDbEntryPtr->rulesArr[counter].pattern, pattern, ruleCopySizeInBytes);
            cpssOsMemCpy (haDbEntryPtr->rulesArr[counter].mask,    mask,    ruleCopySizeInBytes);
            cpssOsMemCpy (haDbEntryPtr->rulesArr[counter].action,  action,  sizeof(action));
            haDbEntryPtr->rulesArr[counter].ruleId        = 0xFFFFFFFF;
            haDbEntryPtr->rulesArr[counter].vTcamId       = 0xFFFFFFFF;
            haDbEntryPtr->rulesArr[counter].found         = 0;

            /* uses segment DB for SIP5 and above or Legacy PCL segment DB only */
            /* other segments of legacy devices not supported                   */
            if (vTcamMngPtr->tcamSegCfg.hwDbReadFuncPtr != NULL)
            {
                GT_U32 ruleId, vTcamId;

                rc = vTcamMngPtr->tcamSegCfg.hwDbReadFuncPtr(
                    devNum, portGroupsBmp,
                    haDbEntryPtr->rulesArr[counter].physicalIndex,
                    &vTcamId, &ruleId);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "read from TCAM Manager HA HW DB failed\n");
                }
                if ((vTcamId != PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_DB_INVALID_VTCAM_ID) 
                    || (ruleId != PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_DB_INVALID_RULE_ID))
                {
                    haDbEntryPtr->rulesArr[counter].vTcamId = vTcamId;
                    haDbEntryPtr->rulesArr[counter].ruleId  = ruleId;
                    /* "found" field remains 0 up to matching contents on replay phase */
                }
            }

            haDbEntryPtr->numRules++;
        }
    }
    return GT_OK;
}

/**
 * @internal prvCpssDxChVirtualTcamHaDbVerifyEntry function
 * @endinternal
 *
 * @brief to compare rule stored in vTcamHaDb matches with the corres. rule retrieved using vTcamManager after catchup
 *
 * @param  dbEntryPtr - supplies the ruleId for data retrieval from vTCAM DB & the ruleData for comparison
 */
GT_STATUS prvCpssDxChVirtualTcamHaDbVerifyEntry
(
    IN  GT_U8                                             devNum,
    IN  GT_U32                                            vTcamMngId,
    IN  GT_U32                                            vTcamId,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR   dbEntryPtr
)
{
    GT_STATUS    rc;
    GT_U32       portGroupId;
    GT_U32       logicalIndex, deviceRuleIndex, dummy;
    GT_U32       mask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS]     = PATTERN; /* mask in HW format */
    GT_U32       pattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS]  = PATTERN;
    GT_U32       action[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS] = {0,};
    GT_U32       entryNumber = 0;            /* Rule's index in TCAM */
    GT_BOOL      validFound  = GT_FALSE;     /* is rule found valid  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR vtcamInfoPtr;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT                tcamRuleSize;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        vtcamRuleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E ;
    GT_PORT_GROUPS_BMP                          portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC    *vTcamMngPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "vtcam %d not found", vTcamId);
    }

    portGroupsBmp = vTcamMngPtr->portGroupBmpArr[devNum];

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum,portGroupsBmp,portGroupId);

    /* Convert the ruleId to logical index and corres. derived physical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
            vTcamMngId,vTcamId,dbEntryPtr->ruleId,
            GT_TRUE,/* check that the ruleId already exists in the DB */
            &logicalIndex, &deviceRuleIndex);

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert failed\n");
    }

    if (dbEntryPtr->physicalIndex != deviceRuleIndex)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_FAIL, "different dbEntryPtr->physicalIndex %d and converted deviceRuleIndex %d\n",
            dbEntryPtr->physicalIndex, deviceRuleIndex);
    }

    /* Read the pattern, mask and action from TCAM */

    /* Convert rule global index to TCAM entry number */
    rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum, deviceRuleIndex, &entryNumber,&dummy);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* read action */
    rc = prvCpssDxChPortGroupReadTableEntry(devNum,
            portGroupId,
            CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
            (entryNumber/2),
            action);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChTcamPortGroupRuleRead(devNum, portGroupsBmp, deviceRuleIndex
                                             , &validFound, &tcamRuleSize, pattern, mask);
    if (rc != GT_OK)
    {
        return rc;
    }

    vtcamRuleSize = vtcamRuleSizeMap[tcamRuleSize];

    if ((prvCpssPatternMaskCompare(pattern, mask,dbEntryPtr->pattern,
                                  dbEntryPtr->mask,vtcamRuleSize) == 0) &&
         (cpssOsMemCmp(action,  dbEntryPtr->action, sizeof(action)) == 0)
       )
    {
        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
 * @internal prvCpssDxChVirtualTcamHaClearUnMappedEntries function
 * @endinternal
 *
 * @brief Clear all unmapped entries in vTcamHaDb from TCAM
 *
 */
GT_STATUS prvCpssDxChVirtualTcamHaClearUnMappedEntries
(
    IN  GT_U8  devNum,
    IN  GT_U32 vTcamMngId
)
{
    GT_STATUS                                     rc = GT_OK, rc1 = GT_OK;
    GT_U32                                        i;
    GT_U32                                        lkp;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT                  tcamRuleSize;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC           *vTcamMngPtr;
    CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC
                                                 *replayRulePtr = NULL;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    for(tcamRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_10_B_E; tcamRuleSize <= CPSS_DXCH_TCAM_RULE_SIZE_80_B_E; tcamRuleSize++)
    {
        for(lkp=PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_0_E; lkp<PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_MAX_E; ++lkp)
        {

            if (!vTcamMngPtr->vTcamHaDb[tcamRuleSize][lkp].rulesArr)
                continue;

            for(i=0; i< vTcamMngPtr->vTcamHaDb[tcamRuleSize][lkp].numRules; i++)
            {
                if( vTcamMngPtr->vTcamHaDb[tcamRuleSize][lkp].rulesArr[i].found == 0 )
                {

                    if (vTcamMngPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_SAVE_ALL_RULES_E)
                    {
                        replayRulePtr = cpssOsMalloc(sizeof(CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC));

                        if (!replayRulePtr)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "Malloc for unmapped rule list entry creation failed. rc=%d\n", rc);
                        }

                        if (! VTCAM_GLOVAR(replayUnmappedRuleDb))
                        {
                            VTCAM_GLOVAR(replayUnmappedRuleDb) = replayRulePtr;
                        }

                        if (VTCAM_GLOVAR(replayUnmappedRuleDbTail))
                        {
                            VTCAM_GLOVAR(replayUnmappedRuleDbTail)->next = replayRulePtr;
                        }

                        VTCAM_GLOVAR(replayUnmappedRuleDbTail) = replayRulePtr;
                        replayRulePtr->next = NULL;

                        cpssOsMemSet(replayRulePtr, 0, sizeof(CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC));

                        cpssOsMemCpy (replayRulePtr->pattern, vTcamMngPtr->vTcamHaDb[tcamRuleSize][lkp].rulesArr[i].pattern, sizeof(replayRulePtr->pattern));
                        cpssOsMemCpy (replayRulePtr->mask, vTcamMngPtr->vTcamHaDb[tcamRuleSize][lkp].rulesArr[i].mask, sizeof(replayRulePtr->mask));
                        cpssOsMemCpy (replayRulePtr->action, vTcamMngPtr->vTcamHaDb[tcamRuleSize][lkp].rulesArr[i].action, sizeof(replayRulePtr->action));
                        replayRulePtr->physicalIndex = vTcamMngPtr->vTcamHaDb[tcamRuleSize][lkp].rulesArr[i].physicalIndex;
                    }

                    /* Clear the entry in TCAM */
                    rc1 = perDevice_ruleClear( devNum
                            , vTcamMngPtr->portGroupBmpArr[devNum]
                            , vTcamMngPtr->vTcamHaDb[tcamRuleSize][lkp].rulesArr[i].physicalIndex
                            , vTcamMngPtr->tcamSegCfg.deviceClass, vtcamRuleSizeMap[tcamRuleSize]);

                    if (rc1 != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("ruleClear of unmapped rule (@=%d, size=%d) failed. with rc = %d\n"
                                , vTcamMngPtr->vTcamHaDb[tcamRuleSize][lkp].rulesArr[i].physicalIndex, tcamRuleSize, rc1);
                        rc = GT_FAIL;
                    }
                }
            }
        }
    }
    return rc;
}

/**
 * @internal prvCpssDxChVirtualTcamHaDbMappedEntriesVerify function
 * @endinternal
 *
 * @brief Compare rulePhysicalIndexArr contents of vTcam Manager DB after HA with that of the vTcamHaDb
 *
 */
GT_STATUS prvCpssDxChVirtualTcamHaDbMappedEntriesVerify
(
    IN  GT_U8  devNum,
    IN  GT_U32 vTcamMngId
)
{
    GT_STATUS                                          rc            = GT_FAIL;
    GT_U32                                             numOfEntries;
    GT_U32                                             i, j;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT               ruleSize;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *vTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR   *entriesArr    = NULL;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC       *vtcamInfoPtr;
    GT_U32                                            *old_usedRulesBitmapArr,old_usedRulesBitmapArrSize;
    GT_U32                                             bmpSize;          /* size of used rules BMP */
    GT_BOOL                                            haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    for(i=0; i<vTcamMngPtr->vTcamCfgPtrArrSize; i++)
    {
        if(!vTcamMngPtr->vTcamCfgPtrArr[i]) continue;

        /* i = vtcamId */
        rc = prvCpssDxChVirtualTcamHaDbVTcamEntriesNumAndSizeGet(vTcamMngId, i, &numOfEntries, &ruleSize);
        if (rc != GT_OK) return rc;

        if(numOfEntries == 0)
        {
            if(haVtcamDebug)
            {
                cpssOsPrintf("\nNum of vTCAM entries = 0. vtcamId=%d.\n", i);
            }
            continue;
        }

        entriesArr = cpssOsMalloc(numOfEntries*sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR));
        if(!entriesArr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "entriesArr allocation failed\n");

        if(haVtcamDebug)
        {
            cpssOsPrintf("_M entriesArr - %d, %d\n", numOfEntries, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR));
        }

        rc = prvCpssDxChVirtualTcamHaDbVTcamEntriesGet(vTcamMngId, i, numOfEntries, entriesArr);
        if (rc != GT_OK)
        {
            cpssOsFree(entriesArr);
            return rc;
        }

        /* First check if No. of rules matches */
        if (numOfEntries != vTcamMngPtr->vTcamCfgPtrArr[i]->usedRulesAmount)
        {
            cpssOsFree(entriesArr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "HaCompare failed. No. of rules mismatch\n");
        }

        /* TODO Check if the num of bits set in the bitmap is same ? */
        for (j=0; j<numOfEntries; j++)
        {
            rc = prvCpssDxChVirtualTcamHaDbVerifyEntry(devNum, vTcamMngId, i, entriesArr[j]);
            if (rc != GT_OK)
            {
                cpssOsFree(entriesArr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "HaCompare failed. prvCpssDxChVirtualTcamHaDbVerifyEntry failed.\n");
            }
        }

        /* AutoResize VTCAMs - reallocate  usedRulesBitmapArr if needed - there is a chance of
            ruleAmount > usedRulesBitmapArrSizeHa because of alignment */
        vtcamInfoPtr = vTcamMngPtr->vTcamCfgPtrArr[i];
        if (vtcamInfoPtr->tcamInfo.autoResize == GT_TRUE)
        {
            bmpSize = (((vtcamInfoPtr->rulesAmount + 31) / 32) * sizeof(GT_U32));

            if (bmpSize > vtcamInfoPtr->usedRulesBitmapArrSizeHa)
            {
                old_usedRulesBitmapArrSize = vtcamInfoPtr->usedRulesBitmapArrSizeHa;
                old_usedRulesBitmapArr = vtcamInfoPtr->usedRulesBitmapArr;

                vtcamInfoPtr->usedRulesBitmapArr = (GT_U32*)cpssOsMalloc(bmpSize);/*increment for each rule add*/
                if (vtcamInfoPtr->usedRulesBitmapArr == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
                cpssOsMemSet(vtcamInfoPtr->usedRulesBitmapArr,0, bmpSize);

                vtcamInfoPtr->usedRulesBitmapArrSizeHa = bmpSize;

                /*copy the previous content*/
                cpssOsMemCpy(vtcamInfoPtr->usedRulesBitmapArr,old_usedRulesBitmapArr,old_usedRulesBitmapArrSize);

                cpssOsFree(old_usedRulesBitmapArr);
            }
        }

        if(entriesArr) cpssOsFree(entriesArr);
    }

    return GT_OK;
}

GT_STATUS prvCpssDxChVirtualTcamHaSw2HwRuleConvertPart2
(
    IN  GT_BOOL                                      valid,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT                 ruleSize,
    IN  GT_U32                                       *tcamPatternPtr,
    IN  GT_U32                                       *tcamMaskPtr,
    OUT GT_U32                                       *hwPattern,
    OUT GT_U32                                       *hwMask
)
{
    GT_U32  entryKey[3];        /* entry to write (key)         */
    GT_U32  entryMask[3];       /* entry to write (mask)        */
    GT_U32  entryDataX[3];      /* entry to write (dataX)       */
    GT_U32  entryDataY[3];      /* entry to write (dataY)       */
    GT_U32  bank;               /* current bank written         */
    GT_U32  ii;                 /* iteration index              */
    GT_U32  numOfBanks;         /* number of banks to write     */
    GT_U32  keySize;            /* key size values              */
    GT_U32  bankIdx;            /* bank iterator                */
    GT_U32  value;
    GT_U32  startBit;

    PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(keySize,ruleSize);
    PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_NUM_OF_BANKS_VAL_MAC(numOfBanks,ruleSize);

    /* bank#0 validates whole entry. Therefore write banks in reverse order to ensure that bank#0 will be last. */
    bank = numOfBanks - 1;
    for (bankIdx = 0; bankIdx < numOfBanks; bankIdx++, bank--)
    {
         /* clear data */
        cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
        cpssOsMemSet(&entryMask , 0, sizeof(entryMask));
        cpssOsMemSet(&entryDataX , 0, sizeof(entryDataX));
        cpssOsMemSet(&entryDataY , 0, sizeof(entryDataY));

        startBit = bank * 80;

        U32_SET_FIELD_IN_ENTRY_MAC(entryKey,0,3,keySize);
        U32_SET_FIELD_IN_ENTRY_MAC(entryMask,0,3,7); /* excet match */

        /* copy first 64 bits of pattern/mask */
        for(ii = 0 ; ii < 2 ; ii++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(tcamPatternPtr,startBit+(32*ii),32,value);
            U32_SET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),32,value);
            U32_GET_FIELD_IN_ENTRY_MAC(tcamMaskPtr,startBit+(32*ii),32,value);
            U32_SET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),32,value);
        }
        ii = 2;
        /* copy next 16 bits of pattern/mask (total of 80 bits)*/
        U32_GET_FIELD_IN_ENTRY_MAC(tcamPatternPtr,startBit+(32*ii),16,value);
        U32_SET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),16,value);
        U32_GET_FIELD_IN_ENTRY_MAC(tcamMaskPtr,startBit+(32*ii),16,value);
        U32_SET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),16,value);

        /* convert pattern/mask to X/Y format */
        for (ii = 0; ii <= 2; ii++)
        {
            entryDataX[ii] = PRV_BIT_PATTERN_MASK_TO_X_MAC(entryKey[ii], entryMask[ii]);
            entryDataY[ii] = PRV_BIT_PATTERN_MASK_TO_Y_MAC(entryKey[ii], entryMask[ii]);
        }

        /* first bank size is used as validity for the whole rule. when X=0 and Y=1 results--> Match 0, when X=1 and Y=1 results--> Always Miss. */
        if(bank == 0)
        {
            U32_SET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,(1-BOOL2BIT_MAC(valid)));
        }
        else
        {
            U32_SET_FIELD_IN_ENTRY_MAC(entryDataX,3,1,0); /* X=0*/
        }

        U32_SET_FIELD_IN_ENTRY_MAC(entryDataY,3,1,1); /* Y=1*/

        /* clear data */
        cpssOsMemSet(&entryKey , 0, sizeof(entryKey));
        cpssOsMemSet(&entryMask , 0, sizeof(entryMask));

        /* convert data from X/Y format to pattern/mask */
        for (ii = 0 ; ii <= 2 ; ii++)
        {
            entryKey[ii]  = PRV_BIT_X_Y_TO_PATTERN_MAC(entryDataX[ii],entryDataY[ii]);
            entryMask[ii] = PRV_BIT_X_Y_TO_MASK_MAC(entryDataX[ii],entryDataY[ii]);
        }

        startBit = bank * 80;

        /* copy first 64 bits of pattern/mask */
        for(ii = 0 ; ii < 2 ; ii++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),32,value);
            U32_SET_FIELD_IN_ENTRY_MAC(hwPattern,startBit+(32*ii),32,value);
            U32_GET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),32,value);
            U32_SET_FIELD_IN_ENTRY_MAC(hwMask,startBit+(32*ii),32,value);
        }
        ii = 2;
        /* copy next 16 bits of pattern/mask (total of 80 bits)*/
        U32_GET_FIELD_IN_ENTRY_MAC(entryKey,4+(32*ii),16,value);
        U32_SET_FIELD_IN_ENTRY_MAC(hwPattern,startBit+(32*ii),16,value);
        U32_GET_FIELD_IN_ENTRY_MAC(entryMask,4+(32*ii),16,value);
        U32_SET_FIELD_IN_ENTRY_MAC(hwMask,startBit+(32*ii),16,value);
    }
    return GT_OK;
}

GT_STATUS prvCpssDxChVirtualTcamHaSw2HwRuleConvertPart1
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT                *tcamRuleSize,
    OUT GT_U32                                      *hwAction,
    OUT GT_U32                                      *hwMask,
    OUT GT_U32                                      *hwPattern
)
{
    GT_STATUS rc = GT_OK;
    switch(ruleTypePtr->ruleType)
    {
      case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
          PRV_CPSS_DXCH_PCL_CONVERT_RULE_FORMAT_TO_TCAM_RULE_SIZE_VAL_MAC(*tcamRuleSize,ruleTypePtr->rule.pcl.ruleFormat); /*get the tcamRuleSize*/
          rc = prvCpssDxChPclRuleDataSw2HwConvert(
              devNum, ruleTypePtr->rule.pcl.ruleFormat,
              ruleDataPtr->rule.pcl.maskPtr, ruleDataPtr->rule.pcl.patternPtr, ruleDataPtr->rule.pcl.actionPtr,
              hwMask, hwPattern, hwAction);

          if (rc != GT_OK)
          {
              return rc;
          }
          break;
      case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
          PRV1_CPSS_DXCH_TTI_CONVERT_RULE_TYPE_TO_TCAM_RULE_SIZE_VAL_MAC(*tcamRuleSize,ruleTypePtr->rule.tti.ruleFormat); /*get the tcamRuleSize*/

          /* convert tti configuration pattern and mask to hardware format */
          rc = prvCpssDxChTtiConfigLogic2HwFormat(devNum,
                                       ruleTypePtr->rule.tti.ruleFormat,
                                       ruleDataPtr->rule.tti.patternPtr,
                                       ruleDataPtr->rule.tti.maskPtr,
                                       hwPattern,
                                       hwMask);

          /* convert tti action to hardware format */
          rc = prvCpssDxChTtiActionLogic2HwFormat(devNum,
                                       ruleDataPtr->rule.tti.actionPtr,
                                       hwAction);
          if (rc != GT_OK)
          {
              return rc;
          }
          break;
      default:
          rc = GT_BAD_PARAM;
          break;

    }
    return rc;
}

/**
 * @internal prvCpssDxChVirtualTcamHaSw2HwRuleConvert function
 * @endinternal
 *
 * @brief Converts Software rule format(PCL/TTI) to Hardware rule format
 *
 */
GT_STATUS prvCpssDxChVirtualTcamHaSw2HwRuleConvert
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT                *ruleSize,
    OUT GT_U32                                      *hwAction,
    OUT GT_U32                                      *hwMask,
    OUT GT_U32                                      *hwPattern
)
{
    GT_STATUS rc = GT_OK;
    GT_U32       hwMask1[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    GT_U32       hwPattern1[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];

    rc = prvCpssDxChVirtualTcamHaSw2HwRuleConvertPart1(devNum,ruleTypePtr,ruleDataPtr,ruleSize,hwAction,hwMask1,hwPattern1);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaSw2HwRuleConvert1 failed\n");
    }

    rc = prvCpssDxChVirtualTcamHaSw2HwRuleConvertPart2(ruleDataPtr->valid,*ruleSize,hwPattern1,hwMask1,hwPattern,hwMask);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaSw2HwRuleConvert2 failed\n");
    }

    return GT_OK;
}

/**
 * @internal prvCpssDxChVirtualTcamHaDbRulesUpdate function
 * @endinternal
 *
 * @brief Update vTcamHaDb for each rule with fields for ruleId, vTcamId, priority, found during config-replay
 *
 */
GT_STATUS prvCpssDxChVirtualTcamHaDbRulesUpdate
(
    IN CPSS_DXCH_TCAM_RULE_SIZE_ENT                tcamRuleSize,
    IN GT_U32                                      lookupId,
    IN GT_U32                                      priority,
    IN GT_U32                                     *hwAction,
    IN GT_U32                                     *hwMask,
    IN GT_U32                                     *hwPattern,
    IN GT_U32                                      vTcamMngId,
    IN GT_U32                                      vTcamId,
    IN CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId
)
{
    GT_STATUS                                        rc = GT_NOT_FOUND;
    GT_U32                                           i = 0;
    GT_U32                                           l = lookupId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT             vtcamRuleSize = vtcamRuleSizeMap[tcamRuleSize];
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC              *vTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC     *vTcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR  rulePtr = NULL;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    vTcamInfoPtr  = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];

    /* LegacyPCL/SIP5 and above TCAM only supported */
    if (vTcamMngPtr->tcamSegCfg.hwDbReadFuncPtr != NULL)
    {
        /* look for vTcamId and ruleId found in HA HW DB */
        for (i = 0; (i < vTcamMngPtr->vTcamHaDb[tcamRuleSize][l].numRules); i++)
        {
            rulePtr = &vTcamMngPtr->vTcamHaDb[tcamRuleSize][l].rulesArr[i];
            if ((rulePtr->vTcamId == vTcamId) && (rulePtr->ruleId == ruleId))
            {
                if ((prvCpssPatternMaskCompare(
                    hwPattern, hwMask, rulePtr->pattern,
                        rulePtr->mask, vtcamRuleSize) == 0) &&
                    (cpssOsMemCmp(hwAction,  rulePtr->action, sizeof(hwAction)) == 0))
                {
                    rulePtr->found = 1;
                    rulePtr->priority = priority;
                    rulePtr->priority_logicalIndex = 0xffffffff;
                    rc = GT_OK;
                    if (VTCAM_GLOVAR(haVtcamDebugDbFlag) != GT_FALSE)
                    {
                        cpssOsPrintf("Valid Rules DB updated: RuleId - %d\t vTcamId - %d\t\n", ruleId, vTcamId);
                    }
                    break;
                }
                else
                {
                    if (VTCAM_GLOVAR(haVtcamDebugDbFlag) != GT_FALSE)
                    {
                        cpssOsPrintf(
                            "Replayed rule contents not matches DB: RuleId - %d\t vTcamId - %d\t\n", ruleId, vTcamId);
                    }
                }
            }
        }
    }
    else
    {
        for(i=0; i < vTcamMngPtr->vTcamHaDb[tcamRuleSize][l].numRules; ++i)
        {
            rulePtr = &vTcamMngPtr->vTcamHaDb[tcamRuleSize][l].rulesArr[i];

            if(    rulePtr->found == 0
                || vTcamMngPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_MATCH_FIRST_OCCURRENCE_ONLY_E
              )
            {
                if ((prvCpssPatternMaskCompare(hwPattern, hwMask, rulePtr->pattern,
                                               rulePtr->mask, vtcamRuleSize) == 0) &&
                    (cpssOsMemCmp(hwAction,  rulePtr->action, sizeof(hwAction)) == 0))
                {
                    if (    rulePtr->found == 1
                         && vTcamId == rulePtr->vTcamId
                       )
                    {
                        break;
                    }

                    if (    (     (vTcamMngPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_HIGHER_PRIO_FIRST_E)
                               && (    vTcamInfoPtr->haLastConfigReplayPhysicalIndex <  rulePtr->physicalIndex
                                    || vTcamInfoPtr->haLastConfigReplayPhysicalIndex == 0xffffffff
                                  )
                            )
                         || !(vTcamMngPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_HIGHER_PRIO_FIRST_E)
                       )
                    {
                        rulePtr->ruleId  = ruleId;
                        rulePtr->vTcamId = vTcamId;
                        rulePtr->priority = priority;
                        rulePtr->priority_logicalIndex = 0xffffffff;
                        rulePtr->found = 1;

                        vTcamInfoPtr->haLastConfigReplayPhysicalIndex = rulePtr->physicalIndex;

                        rc = GT_OK;

                        if (VTCAM_GLOVAR(haVtcamDebugDbFlag) != GT_FALSE)
                        {
                            cpssOsPrintf("Valid Rules DB updated: RuleId - %d\t vTcamId - %d\t\n", ruleId, vTcamId);
                        }

                        break;
                    }
                }
            }
        }
    }

    return rc;
}

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
)
{
    /* Get vTCAM Manager corres. to devNum */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC              *vTcamMngPtr;
    GT_U32                                           i;
    GT_STATUS                                        rc;
#ifdef CPSS_LOG_ENABLE
    GT_U32    start_sec  = 0;
    GT_U32    start_nsec = 0;
    GT_U32    end_sec    = 0;
    GT_U32    end_nsec   = 0;
    GT_U32    diff_sec   = 0;
    GT_U32    diff_nsec  = 0;
#endif /*CPSS_LOG_ENABLE*/
    CPSS_SYSTEM_RECOVERY_INFO_STC   oldSystemRecoveryInfo,newSystemRecoveryInfo;  /* backup for h/w enable */
    GT_BOOL   haVtcamDebug;

    PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);

    if(! PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        return GT_OK; /* support only for Falcon - returning GT_OK */
    }

    CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

#ifdef CPSS_LOG_ENABLE
    if(haVtcamDebug)
    {
        rc = cpssOsTimeRT(&start_sec, &start_nsec);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssOsTimeRT failed for start\n");
        }
    }
#endif /*CPSS_LOG_ENABLE*/

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }
    for(i=0; i<vTcamMngPtr->vTcamCfgPtrArrSize; i++)
    {
        if(!vTcamMngPtr->vTcamCfgPtrArr[i])
            continue;

        rc = prvCpssDxChVirtualTcamHaFoundRuleSegmentsCreate(vTcamMngId, i);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaFoundRuleSegmentsCreate failed\n");
        }
    }

    /* After the recreate of all vtcams, for logical Index vTCAMs, fill the holes           */
    /* Note: below function will simply resize if vTCAM in question is a priority vTCAM */
    for(i=0; i<vTcamMngPtr->vTcamCfgPtrArrSize; i++)
    {
        if(!vTcamMngPtr->vTcamCfgPtrArr[i])
            continue;

        rc = prvCpssDxChVirtualTcamHaFillHolesAndMergeSegs(vTcamMngId, i);
        if (rc != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaFillHolesAndMergeSegs failed\n");
    }

    /* Fill Empty vTCAMs */
    for(i=0; i<vTcamMngPtr->vTcamCfgPtrArrSize; i++)
    {
        if(!vTcamMngPtr->vTcamCfgPtrArr[i])
            continue;

        rc = prvCpssDxChVirtualTcamHaFillEmptyVtcams(vTcamMngId, i);
        if (rc != GT_OK)
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaFillEmptyVtcams failed\n");
    }

    /* Loop for priority-driven vTCAMs only. Needed to populate the "4 Trees" */
    for(i=0; i<vTcamMngPtr->vTcamCfgPtrArrSize; i++)
    {
        if(!vTcamMngPtr->vTcamCfgPtrArr[i])
            continue;

        rc = prvCpssDxChVirtualTcamHaLogicalIndexVtcamUsedRulesMark(vTcamMngId, i);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaLogicalIndexVtcamUsedRulesMark failed\n");
        }
        rc = prvCpssDxChVirtualTcamHaPriorityVtcamTreesCreate(vTcamMngId, i);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaPriorityVtcamTreesCreate failed\n");
        }
    }

    if(haVtcamDebug)
    {
        cpssOsPrintf("\n\nSegmentsTree Layout after HA\n\n");
        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamSegCfg);
    }

    rc = prvCpssDxChVirtualTcamHaDbMappedEntriesVerify(devNum, vTcamMngId);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaDbMappedEntriesVerify failed\n");
    }

    rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    newSystemRecoveryInfo = oldSystemRecoveryInfo;
    newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);

    rc = prvCpssDxChVirtualTcamHaClearUnMappedEntries(devNum , vTcamMngId);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaClearUnMappedEntries failed\n");
    }

    /* restore sysRecovery to orig. state */
    prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);

    CPSS_LOG_INFORMATION_MAC("vTcam Manager (id=%d) HA H/w CatchUp Successful.\n", vTcamMngId);

#ifdef CPSS_LOG_ENABLE
    if(haVtcamDebug)
    {
        rc = cpssOsTimeRT(&end_sec, &end_nsec);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssOsTimeRT failed for end\n");
        }

        if(end_nsec < start_nsec)
        {
            end_nsec += 1000000000;
            end_sec  -= 1;
        }
        diff_sec  = end_sec  - start_sec;
        diff_nsec = end_nsec - start_nsec;

        CPSS_LOG_INFORMATION_MAC("system recovery time for H/w CatchUp  is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);
    }
#endif /*CPSS_LOG_ENABLE*/

    return GT_OK;
}

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
)
{
    GT_STATUS                              rc            = GT_OK;
    GT_STATUS                              catchupFailRc = GT_OK;
    GT_U8                                  devNum;
    GT_U32                                 vTcamMngId;
    GT_BOOL                                devFound;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC     *vTcamMngPtr;
    GT_BOOL                                haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);

    for(vTcamMngId = 0;vTcamMngId <= CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS; ++vTcamMngId)
    {
        if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
        {
            /* vTcamMngId out of range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
        }

        vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
        if(!vTcamMngPtr)
        {
            continue;
        }

        devFound = GT_FALSE;
        for (devNum = 0; devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
        {
            if (PRV_CPSS_VIRTUAL_TCAM_MNG_IS_DEV_BMP_SET_MAC (vTcamMngPtr->devsBitmap, devNum))
            {
                devFound = GT_TRUE;

                rc = prvCpssDxChVirtualTcamHaCatchUp(devNum, vTcamMngId);
                if (rc != GT_OK)
                {
                    catchupFailRc = rc;
                    CPSS_LOG_INFORMATION_MAC("\n Catchup Failed for vTCAM Manager vTcamMngId = %d\n", vTcamMngId);
                }
                break;
            }
        }

        if (!devFound && haVtcamDebug)
            cpssOsPrintf("\n No Devices Found for vTcamMngId : %d \n", vTcamMngId);
    }

    return catchupFailRc;
}

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
)
{
    GT_STATUS                              rc            = GT_OK;
    GT_STATUS                              rc1           = GT_OK;
    GT_STATUS                              replayFailRc  = GT_OK;
    GT_U32                                 vTcamMngId;
    CPSS_SYSTEM_RECOVERY_INFO_STC          oldSystemRecoveryInfo,newSystemRecoveryInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC    *vTcamMngPtr;
    CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC
                                          *replayRulePtr = NULL, *replayRulePtrTemp = NULL;

    if (VTCAM_GLOVAR(replayRuleDb))
    {
        /* Temporarily enable hw-writes. Set system recovery to complete */
        rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
        if (rc != GT_OK)
        {
            return rc;
        }
        newSystemRecoveryInfo = oldSystemRecoveryInfo;
        newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);

        for (vTcamMngId = 0;vTcamMngId <= CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS; ++vTcamMngId)
        {
            vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
            if (!vTcamMngPtr)
            {
                continue;
            }

            if (    vTcamMngPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_FAILED_RULES_ADD_E
                 || vTcamMngPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_SAVE_ALL_RULES_E )
            {

                replayRulePtr = VTCAM_GLOVAR(replayRuleDb);

                while (replayRulePtr)
                {
                    if (vTcamMngPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_FAILED_RULES_ADD_E)
                    {
                        if (replayRulePtr->vTcamMngId == vTcamMngId)
                        {
                            rc1 = cpssDxChVirtualTcamRuleWrite (
                                    replayRulePtr->vTcamMngId,
                                    replayRulePtr->vTcamId,
                                    replayRulePtr->ruleId,
                                    &replayRulePtr->ruleAttributes,
                                    &replayRulePtr->ruleType,
                                    &replayRulePtr->ruleData );

                            if (rc1 != GT_OK)
                            {
                                replayFailRc = rc1;
                                CPSS_LOG_ERROR_MAC(
                                        "\nHA config-replay failed rule's write NOK [rc1=%d] [vtcam mgr= %d, vtcam=%d, rule-id=%d]\n",
                                        rc1,
                                        replayRulePtr->vTcamMngId,
                                        replayRulePtr->vTcamId,
                                        replayRulePtr->ruleId);
                            }
                        }
                    }

                    replayRulePtrTemp = replayRulePtr;
                    replayRulePtr = replayRulePtr->next;

                    if (!(vTcamMngPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_SAVE_ALL_RULES_E))
                    {
                        cpssOsFree(replayRulePtrTemp);
                    }
                }

                if (    !(vTcamMngPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_SAVE_ALL_RULES_E)
                     && VTCAM_GLOVAR(replayRuleDb))
                {
                    VTCAM_GLOVAR(replayRuleDb) = NULL;
                    VTCAM_GLOVAR(replayRuleDbTail) = NULL;
                }
            }
        }

        prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
    }

    if (replayFailRc != GT_OK)
    {
        return replayFailRc;
    }

    return rc;
}

/**
 * @internal prvCpssDxChVirtualTcamCatchupForHa function
 * @endinternal
 *
 * @brief  CatchUp function for vTCAM Recovery in HA mode - called as part of completion handle
 */
GT_STATUS prvCpssDxChVirtualTcamCatchupForHa
(
    GT_VOID
)
{
    GT_STATUS                              catchupFailRc = GT_OK;
    GT_STATUS                              replayFailRc  = GT_OK;

    catchupFailRc = prvCpssDxChVirtualTcamCatchupForHaBuildDbAndClearUnmappedRules();
    replayFailRc  = prvCpssDxChVirtualTcamCatchupForHaWriteFailedReplyedRules();

    if (catchupFailRc != GT_OK)
    {
        return catchupFailRc;
    }
    else if (replayFailRc != GT_OK)
    {
        return replayFailRc;
    }

    return GT_OK;;
}

/**
 * @internal  prvCpssDxChVirtualTcamHaLookupIdRestore
 * @endinternal
 *
 * @brief  This Function takes a backup of the lookupId of a block by reading registers
 *         Also updates blockInfArr lookupId for blocks used by vTcamManager before hA
 *         by reading first rule of every block in TCAM
 *
 * @param  baseRow        - base row of segment to be mapped
 *         baseColumn     - base column of segment to be mapped
 */
GT_STATUS prvCpssDxChVirtualTcamHaLookupIdRestore
(
    IN      GT_U8                   devNum,
    IN      GT_U32                  vTcamMngId,
    IN      GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN      GT_U32                  blocksAmount,
    INOUT   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockInfArr
)
{
    GT_STATUS rc;                                /* return code           */
    GT_U32    j, min, baseRow, baseColumn;       /* loop bounds           */
    GT_U32    numBanksForHitNumGranularity = 0;
    GT_U32    floorIndex, physicalIndex;         /* floor Index           */
    CPSS_DXCH_TCAM_BLOCK_INFO_STC  floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    CPSS_DXCH_TCAM_RULE_SIZE_ENT    tcamRuleSize;
    GT_BOOL   validFound  = GT_FALSE;     /* is rule found valid  */
    GT_U32    mask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    GT_U32    pattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];
    GT_U32    activatedfloorIndex;                       /* Powered up floor Index*/
    GT_BOOL                                            seeked;
    GT_VOID_PTR                                        dbEntryPtr;
    PRV_CPSS_AVL_TREE_ID                               treeId;
    PRV_CPSS_AVL_TREE_ID                               treeId2;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR    segPtr = NULL;
    PRV_CPSS_AVL_TREE_SEEK_ENT                         mode;
    PRV_CPSS_AVL_TREE_PATH                             path;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *vTcamMngPtr;
    GT_U32                                             saveLookupId;
    GT_BOOL                                            haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }


    cpssOsMemSet(mask, 0, sizeof(mask));
    cpssOsMemSet(pattern, 0, sizeof(pattern));

    if(haVtcamDebug)
    {
        cpssOsPrintf("blocksAmount: %d\n", blocksAmount);
    }

    rc = cpssDxChTcamPortGroupActiveFloorsGet(
        devNum, vTcamMngPtr->portGroupBmpArr[devNum], &activatedfloorIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    for(j=0;j<blocksAmount;++j)
    {

        baseRow     = blockInfArr[j].rowsBase;
        baseColumn  = blockInfArr[j].columnsBase;
        if(haVtcamDebug)
        {
            cpssOsPrintf("baseRow%d,baseColumn%d\n",baseRow,baseColumn);
        }

        floorIndex = baseRow / 256;

        numBanksForHitNumGranularity =  /* in BC2/BC3 = 2 , in BobK/Aldrin = 6 */
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;

        rc = cpssDxChTcamIndexRangeHitNumAndGroupGet(
                devNum, floorIndex, floorInfoArr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "\n cpssDxChTcamIndexRangeHitNumAndGroupGet Failed\n");
        }

        if (baseColumn < 6)
        {
            /* lower half floor */
            min = 0;
        }
        else
        {
            /* higher half floor */
            min = numBanksForHitNumGranularity/2;
        }

        physicalIndex = baseRow * 12 + baseColumn;

        if (floorIndex < activatedfloorIndex)
        {
            rc = prvCpssDxChTcamHaRuleRead(devNum, portGroupsBmp, physicalIndex
                                            , &validFound, &tcamRuleSize, pattern, mask);
            if(haVtcamDebug)
            {
                cpssOsPrintf("\nruleSize%d \n\n", tcamRuleSize);
            }

            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "\ncpssDxChTcamPortGroupRuleRead failed.\n");
            }
        }
        else
        {
            /* floor is powered down and not used */
            tcamRuleSize = PRV_CPSS_DXCH_TCAM_HA_INVALID_RULE_SIZE_CNS;
        }

        if(tcamRuleSize != PRV_CPSS_DXCH_TCAM_HA_INVALID_RULE_SIZE_CNS && floorIndex < activatedfloorIndex)
            blockInfArr[j].lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(floorInfoArr[min].group, floorInfoArr[min].hitNum);
        else
            blockInfArr[j].lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS;
    }

    /* update free segment trees with lookupIds */
    treeId      = vTcamMngPtr->tcamSegCfg.segFreeLuTree;
    treeId2     = vTcamMngPtr->tcamSegCfg.segFreePrefTree;

    seeked = GT_TRUE;
    mode = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;

    cpssOsMemSet(path, 0, sizeof(PRV_CPSS_AVL_TREE_PATH));

    while (seeked != GT_FALSE)
    {
        seeked = prvCpssAvlPathSeek(
                treeId, mode,
                path, &dbEntryPtr);

        mode = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
        segPtr = dbEntryPtr;

        for(j=0;j<blocksAmount;++j)
        {
            baseRow     = blockInfArr[j].rowsBase;
            baseColumn  = blockInfArr[j].columnsBase;

            if (segPtr->rowsBase == baseRow)
            {
                if( (segPtr->segmentColumnsMap == 0x07 && baseColumn == 0) || (segPtr->segmentColumnsMap == 0x70 && baseColumn == 6) )
                {
                    if(blockInfArr[j].lookupId != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                    {
                        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtr);
                        prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId2, segPtr);

                        saveLookupId = segPtr->lookupId;
                        segPtr->lookupId = blockInfArr[j].lookupId;

                        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                        if (rc != GT_OK)
                        {
                            segPtr->lookupId = saveLookupId;
                            prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                            prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId2, segPtr);
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed to segFreeLuTree\n");
                        }

                        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId2, segPtr);
                        if (rc != GT_OK)
                        {
                            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtr);
                            segPtr->lookupId = saveLookupId;
                            prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                            prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId2, segPtr);
                            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed to segFreePrefTree\n");
                        }

                        break;
                    }
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal  prvCpssDxChVirtualTcamHaBlockInfoLookupIdVerify
* @endinternal
*
* @brief  Fn. to check consistency of segment lookupId with lookupIdHA
*         corresponding to the block where the given Segment(outSegPtr) is located
*
*/
GT_STATUS prvCpssDxChVirtualTcamHaBlockInfoLookupIdVerify
(
   IN  GT_U32                                          vTcamMngId,
   IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR outSegPtr
)
{
    GT_U32 blckIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC              *vTcamMngPtr;
    GT_U32 blocksInRow;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    blocksInRow =
        (vTcamMngPtr->tcamSegCfg.tcamColumnsAmount
            / vTcamMngPtr->tcamSegCfg.lookupColumnAlignment);

    /* two blocks per row */
    for (i = 0; (i < blocksInRow); i++)
    {
        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                outSegPtr->segmentColumnsMap, i);

        if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
            continue;

        blckIdx =
            (((outSegPtr->rowsBase / vTcamMngPtr->tcamSegCfg.lookupRowAlignment)
              * blocksInRow) + i);

        if(vTcamMngPtr->tcamSegCfg.blockInfArr[blckIdx].lookupId != outSegPtr->lookupId)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "segment lookupId not matching block lookupId\n");
    }
    return GT_OK;
}

/**
* @internal  prvCpssDxChVirtualTcamHaBlockInfoIsNotMappedBlock
* @endinternal
*
* @brief  To check is free segment resides in block not mapped to any lookup
*           It this case it just covers all of the single block
*
*/
GT_BOOL prvCpssDxChVirtualTcamHaBlockInfoIsNotMappedBlock
(
   IN  GT_U32                                          vTcamMngId,
   IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR outSegPtr
)
{
    GT_U32 blckIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC              *vTcamMngPtr;
    GT_U32 blocksInRow;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    blocksInRow =
        (vTcamMngPtr->tcamSegCfg.tcamColumnsAmount
            / vTcamMngPtr->tcamSegCfg.lookupColumnAlignment);

    /* two blocks per row */
    for (i = 0; (i < blocksInRow); i++)
    {
        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
                outSegPtr->segmentColumnsMap, i);

        if (segmentColumns == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E)
            continue;

        blckIdx =
            (((outSegPtr->rowsBase / vTcamMngPtr->tcamSegCfg.lookupRowAlignment)
              * blocksInRow) + i);

        if(vTcamMngPtr->tcamSegCfg.blockInfArr[blckIdx].lookupId
           != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
        {
            return GT_FALSE;
        }
    }
    return GT_TRUE;
}

/**
* @internal  prvCpssDxChVirtualTcamHaSegTreeCreate
* @endinternal
*
* @brief  Fn. to get a list of segments from the shadowDB, and verify that they can make a segment,
* or a list of segments ...
* Caution: For 10/20/30 byte rules, if we find a rule in the 1st column, the whole
*          row should be considered to belong to the same vTCAM.
*/
GT_STATUS prvCpssDxChVirtualTcamHaSegTreeCreate
(
   IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR inSegArr,
   IN  GT_U32                                          inSegArrSize,
   IN  GT_U32                                          vTcamMngId,
   IN  GT_U32                                          vTcamId
)
{
    GT_STATUS            rc,rc1;
    GT_U32               i;
    PRV_CPSS_AVL_TREE_ID segLuTree;
    CPSS_BM_POOL_ID      treePool;
    CPSS_BM_POOL_ID      nodesPool;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR  segPtr, outSegPtr,*segBuffArr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC              *vTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR     tcamCfgPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC allocRequest;

    GT_BOOL                                          seeked;                     /* node seeked   */
    PRV_CPSS_AVL_TREE_PATH                           path;                       /* tree iterator */
    GT_VOID_PTR                                      dbEntryPtr;                 /* pointer to entry in DB */
    GT_BOOL                                          haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    tcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];

    segBuffArr = cpssOsMalloc(inSegArrSize*sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR));
    if(!segBuffArr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "segBuffArr allocation failed\n");

    if(haVtcamDebug)
    {
            cpssOsPrintf("_M segBuffArr - %d, %d\n", inSegArrSize, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR));
    }

    rc = cpssBmPoolCreate(
            sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC), CPSS_BM_POOL_4_BYTE_ALIGNMENT_E,
            inSegArrSize, &nodesPool);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "nodesPool allocation failed\n");
    }

    rc = prvCpssAvlMemPoolCreate( inSegArrSize , &treePool);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Temporary tree nodes pool allocation failed\n");
    }

    rc = prvCpssAvlTreeCreate(
            (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
            treePool,&segLuTree);
    if (rc != GT_OK)
    {
        cpssBmPoolReCreate(treePool);
        prvCpssAvlMemPoolDelete(treePool);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Temporary tree creation failed\n");
    }

    for (i=0; i<inSegArrSize; i++)
    {
        /* Create Buffer */
        segPtr = NULL;
        segPtr = cpssBmPoolBufGet(nodesPool);
        if (!segPtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "segPtr not allocated\n");
        }

        *segPtr = *&(inSegArr[i]);
        segBuffArr[i] = segPtr;

        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(segLuTree, (GT_VOID_PTR) segPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "%d: Insert failed\n",i);
        }

    }

    if(haVtcamDebug)
    {
        cpssOsPrintf("PRINT for Temp. Segments Tree for vTcam %d\n", vTcamId);
        wrPrintSegmentsTree(segLuTree, haVtcamDebug);
    }

    /* Once for Horizontal merge */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(&vTcamMngPtr->tcamSegCfg,segLuTree);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Horizontal merge failed\n");
    }

    if(haVtcamDebug)
    {
        cpssOsPrintf("1st Merge temp segmentsTree - rc=%d\n", rc);
    }

    /* Once for Vertical merge ? */
    rc = prvCpssDxChVirtualTcamDbSegmentTableLuOrderedTreeMerge(&vTcamMngPtr->tcamSegCfg,segLuTree);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Vertical merge failed\n");
    }

    if(haVtcamDebug)
    {
        cpssOsPrintf("2nd Merge temp segmentsTree - rc=%d\n", rc);
    }

    wrPrintSegmentsTree(segLuTree, haVtcamDebug);

    /* Merge is over. Now, let us pop each element (i.e. merged segment) and first edit the free space.
     * Then add it to the segmentsTree of the corres. vTcam.
     */
    seeked = prvCpssAvlPathSeek(
            segLuTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr);

    while (seeked != GT_FALSE)
    {
        segPtr = dbEntryPtr;
        outSegPtr = cpssBmPoolBufGet(vTcamMngPtr->segmentsPool);
        if (outSegPtr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            goto ha_seg_tree_create_cleanup;
        }
        *outSegPtr = *segPtr;

        rc = prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree(
                tcamCfgPtr->tcamSegCfgPtr, outSegPtr->lookupId,
                outSegPtr->rowsBase, outSegPtr->rowsAmount, outSegPtr->segmentColumnsMap);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("prvCpssDxChVirtualTcamDbSegmentTableRemoveMemoryFromFree failed\n");
            goto ha_seg_tree_create_cleanup;
        }

        /*check for consistency of segment lookupId with lookupIdHA*/
        rc = prvCpssDxChVirtualTcamHaBlockInfoLookupIdVerify(vTcamMngId,outSegPtr);
        if (rc != GT_OK)
        {
            cpssBmPoolBufFree(vTcamMngPtr->segmentsPool, outSegPtr);
            CPSS_LOG_INFORMATION_MAC("prvCpssDxChVirtualTcamHaBlockInfoLookupIdVerify failed\n");
            goto ha_seg_tree_create_cleanup;
        }

        /* Add to vTCAM segments Tree */
        rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(tcamCfgPtr->segmentsTree,outSegPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Segments Treee Insert failed\n");
            goto ha_seg_tree_create_cleanup;
        }

        /* update rule amounts after splits and merges */
        prvCpssDxChVirtualTcamDbSegmentTableUpdateRuleAmounts(
            &vTcamMngPtr->tcamSegCfg, tcamCfgPtr->segmentsTree);

        prvCpssDxChVirtualTcamDbBlocksReservedSpaceRecalculate(
            &vTcamMngPtr->tcamSegCfg, tcamCfgPtr->tcamInfo.clientGroup, tcamCfgPtr->tcamInfo.hitNumber);

        allocRequest.ruleSize        = outSegPtr->ruleSize;

        /* Assign minimum columnType to 40 and 50B rules -
           Used for Horizontal Crop which is not handled in prvCpssDxChVirtualTcamHaFreeSegmentSplit */
        rc = prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeSplitUnusedColumns(
                tcamCfgPtr->tcamSegCfgPtr, &allocRequest, tcamCfgPtr->segmentsTree);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("prvCpssDxChVirtualTcamDbSegmentTablePrimaryTreeSplitUnusedColumns failed\n");
            goto ha_seg_tree_create_cleanup;
        }

        seeked = prvCpssAvlPathSeek(
                segLuTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr);
    }

    if (tcamCfgPtr->tcamInfo.ruleAdditionMethod 
        == CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        segmentTableVTcamBaseLogicalIndexUpdate(tcamCfgPtr);
    }

ha_seg_tree_create_cleanup:

    for (i=0; i<inSegArrSize; i++)
    {
        /* Create Buffer */
        rc1 = cpssBmPoolBufFree(nodesPool,segBuffArr[i]);
        if (rc1 != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc1, "buffer free failed rc = %d\n", rc);
        }
    }

    /* remove temporary tree of added segments */
    rc1 = prvCpssAvlTreeDelete(
            segLuTree,
            (GT_VOIDFUNCPTR)NULL,
            (GT_VOID*)NULL);
    if (rc1 != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc1, "Temporary tree delete failed rc = %d\n", rc);
    }

    rc1 = prvCpssAvlMemPoolDelete(treePool);
    if (rc1 != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc1, "Temporary tree nodes pool delete failed rc = %d\n", rc);
    }

    rc1 = cpssBmPoolDelete(nodesPool);
    if (rc1 != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc1, "nodesPool delete failed rc = %d\n", rc);
    }

    if(segBuffArr) cpssOsFree(segBuffArr);

    return rc;
}

/**
 * @internal  prvCpssDxChVirtualTcamHaFoundRuleSegmentsCreate
 * @endinternal
 *
 * @brief  Fn. to create a list of segments from the rules retrieved from vTcamhaDb corres. to vTcamId
 *         Which are the special rules to be followed for different rule sizes?
 *          -> 30-byte - will occupy whole block width.
 *          -> 10-byte - will ocuppy col 4-5, if first occuring at col 4; whole block width, if at col 0.
 *                       Will occupy col 5 if first occuring at col 5.
 *          -> 40-byte - Will occupy col 0-3, or 3-6(not handled in base code currently??) in case of reminder of 80B rule.
 */


GT_STATUS prvCpssDxChVirtualTcamHaFoundRuleSegmentsCreate
(
   IN  GT_U32      vTcamMngId,
   IN  GT_U32      vTcamId
)
{
    GT_STATUS                                         rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC               *vTcamMngPtr;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT              vtcamRuleSize;
    GT_U32                                            numOfEntries = 0;
    GT_U32                                            i, j, k, row, col, blockCol, ruleId, lookupId;
    GT_U32                                            colInBlockBmp;
    GT_U32                                            segmentIndex;
    GT_U32                                            numOfSegments;
    /* temporary DB of found rules physical indexes */
    /* to calculte segment types and logical starts */
    /* used for 10,20,30 byte rules only            */
    typedef struct {
        GT_U32 row;           /* TCAM row                           */
        GT_U16 segmentIdx;    /* index of segment in array          */
        GT_U8  blockCol;      /* column of blocks 0-1               */
        GT_U8  colInBlockBmp; /* bitmap of columns inside block 0-5 */
    } ROW_BLOCK_COL_STC;
    ROW_BLOCK_COL_STC                                 *usedRowsArr = NULL;
    GT_U32                                            usedRowsArrSize = 0;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR  *entriesArr = NULL;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR   segArr = NULL;
    GT_BOOL                                           haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    /* Get the rule size of the vTCAM from the DB */
    vtcamRuleSize = vTcamMngPtr->vTcamCfgPtrArr[vTcamId]->tcamInfo.ruleSize;
    /* TODO - Use Macro for lookup */
    lookupId = (vTcamMngPtr->vTcamCfgPtrArr[vTcamId]->tcamInfo.clientGroup << 8) |
                vTcamMngPtr->vTcamCfgPtrArr[vTcamId]->tcamInfo.hitNumber;

    rc = prvCpssDxChVirtualTcamHaDbVTcamEntriesNumAndSizeGet(vTcamMngId, vTcamId, &numOfEntries, &vtcamRuleSize);
    if (rc != GT_OK) return rc;

    if (numOfEntries == 0)
    {
        if(haVtcamDebug)
        {
            cpssOsPrintf("\n numOfEntries == 0. \n");
        }
        return GT_OK;
    }

    entriesArr = cpssOsMalloc(numOfEntries*sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR));
    if(!entriesArr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "entriesArr allocation failed\n");

    segArr = cpssOsMalloc(numOfEntries*sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));
    if(!segArr)
    {
         cpssOsFree(entriesArr);
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "segArr allocation failed\n");
    }

    cpssOsMemSet(entriesArr, 0, numOfEntries*sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR));
    cpssOsMemSet(segArr, 0, numOfEntries*sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC));

    if(haVtcamDebug)
    {
        cpssOsPrintf("_M entriesArr - %d, %d\n", numOfEntries, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_HA_DB_RULE_ENTRY_PTR));
        cpssOsPrintf("vtcamRuleSize - %d ", vtcamRuleSize);
        cpssOsPrintf("lookupId - %d\n", lookupId);
    }

    rc = prvCpssDxChVirtualTcamHaDbVTcamEntriesGet(vTcamMngId, vTcamId, numOfEntries, entriesArr);
    if (rc != GT_OK)
    {
        cpssOsFree(entriesArr);
        cpssOsFree(segArr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaDbVTcamEntriesGet failed\n");
    }

    switch(vtcamRuleSize)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E :
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E :
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E :
            if(!usedRowsArr)
            {
                usedRowsArr = cpssOsMalloc(sizeof(ROW_BLOCK_COL_STC) * numOfEntries);
                if (!usedRowsArr)
                {
                    rc = GT_NO_RESOURCE;
                    goto ha_rule_seg_cleanup;
                }
                cpssOsMemSet(usedRowsArr, 0xFF, sizeof(ROW_BLOCK_COL_STC) * numOfEntries);
            }

            break;

        default:
            break;
    }

    /* index i: for rules, k: for segments */
    for (i=0, k=0; i<numOfEntries; i++)
    {
        row = entriesArr[i]->physicalIndex/12;
        col = entriesArr[i]->physicalIndex%12;
        colInBlockBmp = (1 << (entriesArr[i]->physicalIndex % 6)); /* can be overriden */
        segmentIndex  = k;                                         /* can be overriden */
        blockCol = (col < 6) ? 0:1;
        ruleId = entriesArr[i]->ruleId;

        if (k >= numOfEntries)
        {
            CPSS_LOG_ERROR_MAC("\nFAIL: k=%d Vs %d\n", k, numOfEntries);
            rc = GT_FAIL;
            goto ha_rule_seg_cleanup;
        }

        switch(vtcamRuleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E :
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E :
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E :

                if (usedRowsArrSize >= numOfEntries)
                {
                   CPSS_LOG_ERROR_MAC("\nFAIL: usedRowsArrSize=%d Vs %d\n", usedRowsArrSize, numOfEntries);
                   rc = GT_FAIL;
                   goto ha_rule_seg_cleanup;
                }

                for(j=0; j<usedRowsArrSize; j++)
                {
                    if (row == usedRowsArr[j].row 
                        && blockCol == (GT_U32)usedRowsArr[j].blockCol)
                    {
                        colInBlockBmp |= usedRowsArr[j].colInBlockBmp;
                        usedRowsArr[j].colInBlockBmp = (GT_U8)colInBlockBmp;
                        segmentIndex = usedRowsArr[j].segmentIdx;
                        break;
                    }
                }

                if (j >= usedRowsArrSize)
                {
                    /* not found */
                    usedRowsArr[usedRowsArrSize].row           = row;
                    usedRowsArr[usedRowsArrSize].blockCol      = (GT_U8)blockCol;
                    usedRowsArr[usedRowsArrSize].segmentIdx    = (GT_U16)segmentIndex;
                    usedRowsArr[usedRowsArrSize].colInBlockBmp = (GT_U8)colInBlockBmp;
                    usedRowsArrSize ++;
                }

                GT_ATTR_FALLTHROUGH;

            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E :
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E :
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E :
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E :
                /* for priority driven VTCAM baseLogicalIndex will be recalculated */
                segArr[k].baseLogicalIndex = ruleId;
                segArr[k].vTcamId = vTcamId;
                /* ruleSize in segments refer to the column width corres. to each ruleSize */
                segArr[k].ruleSize = vtcamRuleSize + 1;
                segArr[k].lookupId = lookupId;
                segArr[k].rowsBase = row;
                segArr[k].rowsAmount = 1;
                segArr[k].rulesAmount = 1;
                segArr[k].segmentColumnsMap = 0;
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                        segArr[k].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);

        }

        switch(vtcamRuleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E :
                if (vTcamMngPtr->tcamSegCfg.deviceClass == 
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
                {
                    if (colInBlockBmp == (1 << 5)) /* column 5 only */
                    {
                        segArr[k].rulesAmount = 1;
                        segArr[segmentIndex].baseLogicalIndex = ruleId;
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E);
                    }
                    else if ((colInBlockBmp & 0x1F) == 0x10) /* column 4-5, 0-3 empty */
                    {
                        segArr[segmentIndex].rulesAmount       = 2;
                        segArr[segmentIndex].baseLogicalIndex = ruleId - ((col % 6) - 4);
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E);
                    }
                    else if (((colInBlockBmp & 0x03) == 0) && (colInBlockBmp & 0x0C) && (col >= 6))
                    {/* columns 2-3, 0-1 empty */
                        segArr[segmentIndex].rulesAmount       = 4;
                        segArr[segmentIndex].baseLogicalIndex = ruleId - ((col % 6) - 2);
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E);
                    }
                    else
                    {
                        segArr[segmentIndex].rulesAmount = 6;
                        segArr[segmentIndex].baseLogicalIndex = ruleId - (col % 6);
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                                segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);
                    }
                }
                else
                if (vTcamMngPtr->tcamSegCfg.deviceClass ==
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E)
                {
                    /* 10-bit rules placed in odd columns only*/
                    if (colInBlockBmp == (1 << 5)) /* column 5 only */
                    {
                        segArr[k].rulesAmount = 1;
                        segArr[segmentIndex].baseLogicalIndex = ruleId;
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_5_E);
                    }
                    else if (((colInBlockBmp & 0x07) == 0) && (colInBlockBmp & 0x08) && (col >= 6))
                    {/* column 3, 0-2 empty */
                        segArr[segmentIndex].rulesAmount       = 2;
                        segArr[segmentIndex].baseLogicalIndex = ruleId - (((col % 6) / 2) - 1);
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E);
                    }
                    else if (((colInBlockBmp & 0x3D) == 0) && (colInBlockBmp & 0x02) && (col >= 6))
                    {/* column 1, 0,2-5 empty */
                        segArr[segmentIndex].rulesAmount       = 1;
                        segArr[segmentIndex].baseLogicalIndex = ruleId;
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E);
                    }
                    else
                    {
                        segArr[segmentIndex].rulesAmount = 3;
                        segArr[segmentIndex].baseLogicalIndex = ruleId - ((col % 6) / 2);
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                                segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_MAC("\nFAIL: not applicable device class\n");
                    rc = GT_FAIL;
                    goto ha_rule_seg_cleanup;
                }

                if (! (segmentIndex < k))
                {
                    /* if updated not last segment - reuse the last */
                    /* inctement only if the last segment updated   */
                    k++;
                }

                break;

            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E :

                if (vTcamMngPtr->tcamSegCfg.deviceClass == 
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
                {
                    if (colInBlockBmp == (1 << 4)) /* column 4 only */
                    {
                        segArr[k].rulesAmount = 1;
                        segArr[segmentIndex].baseLogicalIndex = ruleId;
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E);
                    }
                    else if (((colInBlockBmp & 0x03) == 0) && (colInBlockBmp & 0x04) && (col >= 6))
                    {/* column 2, 01 empty */
                        segArr[segmentIndex].rulesAmount       = 2;
                        segArr[segmentIndex].baseLogicalIndex = ruleId - (((col % 6) / 2) - 1);
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E);
                    }
                    else
                    {
                        segArr[segmentIndex].rulesAmount = 3;
                        segArr[segmentIndex].baseLogicalIndex = ruleId - ((col % 6) / 2);
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                                segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);
                    }
                }
                else
                if (vTcamMngPtr->tcamSegCfg.deviceClass ==
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E)
                {
                    if ((colInBlockBmp == (1 << 4)) && (col < 6)) /* column 4 only */
                    {
                        segArr[k].rulesAmount = 1;
                        segArr[segmentIndex].baseLogicalIndex = ruleId;
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_45_E);
                    }
                    else if ((colInBlockBmp == 1) && (col >= 6)) /* column 0 only */
                    {
                        segArr[k].rulesAmount = 1;
                        segArr[segmentIndex].baseLogicalIndex = ruleId;
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E);
                    }
                    else if (((colInBlockBmp & 0x03) == 0) && (colInBlockBmp & 0x04) && (col >= 6))
                    {/* column 2, 01 empty */
                        segArr[segmentIndex].rulesAmount       = 2;
                        segArr[segmentIndex].baseLogicalIndex = ruleId - (((col % 6) / 2) - 1);
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E);
                    }
                    else
                    {
                        segArr[segmentIndex].rulesAmount = 3;
                        segArr[segmentIndex].baseLogicalIndex = ruleId - ((col % 6) / 2);
                        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                                segArr[segmentIndex].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ALL_E);
                    }
                }
                else
                {
                    CPSS_LOG_ERROR_MAC("\nFAIL: not applicable device class\n");
                    rc = GT_FAIL;
                    goto ha_rule_seg_cleanup;
                }

                if (! (segmentIndex < k))
                {
                    /* if updated not last segment - reuse the last */
                    /* inctement only if the last segment updated   */
                    k++;
                }

                break;

            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E :

                segArr[segmentIndex].rulesAmount = 2;
                segArr[segmentIndex].baseLogicalIndex = ruleId - ((col % 6) / 3);

                if (! (segmentIndex < k))
                {
                    /* if updated not last segment - reuse the last */
                    /* inctement only if the last segment updated   */
                    k++;
                }

                break;

            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E :
                if ((vTcamMngPtr->tcamSegCfg.deviceClass ==
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E)
                    && ((blockCol % 2) != 0))
                {
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[k].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_2345_E);
                }
                else
                {
                    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                            segArr[k].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_0123_E);
                }
                k++;
                break;

            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E :
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                        segArr[k].segmentColumnsMap, blockCol, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01234_E);

                k++;
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E :
                k++;
                break;

            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E :

                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_SET_MAC(
                        segArr[k].segmentColumnsMap, 1, PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_01_E);

                k++;
                break;

            default:
                rc = GT_FAIL;
                goto ha_rule_seg_cleanup;
                break;
        }

    }

    numOfSegments = k;

    if(haVtcamDebug)
    {
        cpssOsPrintf("vTcamId = %d, # rules = %d. # segs = %d, lookupId = %d\n", vTcamId, numOfEntries, numOfSegments, lookupId);
    }

    if (numOfSegments != 0)
    {
        rc = prvCpssDxChVirtualTcamHaSegTreeCreate(segArr, numOfSegments, vTcamMngId, vTcamId);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_MAC("\nrc = %d prvCpssDxChVirtualTcamHaSegTreeCreate failed.\n", rc);
            goto ha_rule_seg_cleanup;
        }
    }

ha_rule_seg_cleanup:

    if (usedRowsArr)
    {
        cpssOsFree(usedRowsArr);
    }

    if(segArr) cpssOsFree(segArr);

    if(entriesArr) cpssOsFree(entriesArr);

    return rc;
}

/*************************************************************

 *************************************************************/
extern GT_STATUS prvCpssDxChVirtualTcamSegmentTableAvlItemInsert
(
    IN  PRV_CPSS_AVL_TREE_ID                            treeId,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr
);

extern GT_VOID* prvCpssDxChVirtualTcamSegmentTableAvlItemRemove
(
    IN  PRV_CPSS_AVL_TREE_ID                            treeId,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr
);

/**
 * @internal  prvCpssDxChVirtualTcamHaFreeSegmentSplit
 * @endinternal
 *
 * @brief  Fn. to split & maintain the Free Segment tree after a segment(inSegPtr) corresponding to rules added are found
 */
GT_STATUS prvCpssDxChVirtualTcamHaFreeSegmentSplit
(
   IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR  inSegPtr,
   IN  GT_U32                                           vTcamMngId,
   OUT PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR *outSegPtrPtr
)
{
    GT_STATUS                                          rc            = GT_OK;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR    segPtrBkp     = NULL;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR    segPtr1       = NULL;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR    segPtr        = NULL;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_PTR    outSegPtr     = NULL;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC    saveSeg;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *vTcamMngPtr;
    PRV_CPSS_AVL_TREE_ID                               treeId;
    PRV_CPSS_AVL_TREE_ID                               treeId2;
    GT_BOOL                                            seeked;                     /* node seeked   */
    PRV_CPSS_AVL_TREE_SEEK_ENT                         mode;
    PRV_CPSS_AVL_TREE_PATH                             path;                       /* tree iterator */
    GT_VOID_PTR                                        dbEntryPtr;                 /* pointer to entry in DB */
    GT_BOOL                                            removeFreeSeg = GT_FALSE, reItr = GT_FALSE;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(!vTcamMngPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vTcamManager not initialized\n");
    }

    treeId      = vTcamMngPtr->tcamSegCfg.segFreeLuTree;
    treeId2     = vTcamMngPtr->tcamSegCfg.segFreePrefTree;

    seeked = GT_TRUE;
    mode = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;

    cpssOsMemSet(path, 0, sizeof(PRV_CPSS_AVL_TREE_PATH));

    prvCpssDxChVirtualTcamHaDbgPrintSegment("In Segment", inSegPtr);

    while (seeked != GT_FALSE)
    {

        if (removeFreeSeg == GT_TRUE)
        {
            segPtrBkp = segPtr;
        }

        if(reItr == GT_TRUE)
        {
            mode  = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
            cpssOsMemSet(path, 0, sizeof(PRV_CPSS_AVL_TREE_PATH));
            reItr = GT_FALSE;
        }

        seeked = prvCpssAvlPathSeek(
                treeId, mode,
                path, &dbEntryPtr);

        mode = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
        segPtr = dbEntryPtr;

        if (removeFreeSeg == GT_TRUE)
        {
            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtrBkp);
            prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId2, segPtrBkp);

            prvCpssDxChVirtualTcamHaDbgPrintSegment("Case x: free Seg is removed", segPtrBkp);
            cpssBmPoolBufFree(vTcamMngPtr->segmentsPool, segPtrBkp);

            segPtrBkp = NULL;
            removeFreeSeg = GT_FALSE;
        }

        prvCpssDxChVirtualTcamHaDbgPrintSegment("Base free Seg", segPtr);

        /* Check if inSegPtr is part of this segment */
        if (    segPtr->rowsBase <= inSegPtr->rowsBase
                && inSegPtr->rowsBase + inSegPtr->rowsAmount <= segPtr->rowsBase + segPtr->rowsAmount
                && segPtr->segmentColumnsMap == (inSegPtr->segmentColumnsMap & segPtr->segmentColumnsMap)
           )
        {
            if (segPtr->rowsBase == inSegPtr->rowsBase)
            {
                if (segPtr->rowsAmount > inSegPtr->rowsAmount)
                {
                    /* Only one free segment
                       ---------
                       | inSeg |
                       ---------
                       | Free  |
                       ---------
                       No need to allocate seg for free pool
                       */
                    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtr);
                    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId2, segPtr);

                    cpssOsMemCpy(&saveSeg, segPtr, sizeof(saveSeg));
                    segPtr->rowsBase    = inSegPtr->rowsBase + inSegPtr->rowsAmount;
                    segPtr->rowsAmount -= inSegPtr->rowsAmount;
                    segPtr->lookupId    = inSegPtr->lookupId;

                    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                    if (rc != GT_OK)
                    {
                        cpssOsMemCpy(segPtr, &saveSeg, sizeof(saveSeg));
                        prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed to SegFreeLuTree in prvCpssDxChVirtualTcamHaFreeSegmentSplit\n");
                    }

                    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId2, segPtr);
                    if (rc != GT_OK)
                    {
                        cpssOsMemCpy(segPtr, &saveSeg, sizeof(saveSeg));
                        prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                        prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId2, segPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed to SegFreePrefTree in prvCpssDxChVirtualTcamHaFreeSegmentSplit\n");
                    }
                    reItr = GT_TRUE;

                    prvCpssDxChVirtualTcamHaDbgPrintSegment("Case 1: free Seg", segPtr);
                } else {

                    if (segPtr->rowsAmount == inSegPtr->rowsAmount)
                    {

                        /* New segment is same size as free segment
                           ---------
                           | inSeg |
                           |       |
                           |       |
                           ---------
                           No need to allocate seg for free pool. Remove seg from free pool and
                           send it as outPtr*/

                        prvCpssDxChVirtualTcamHaDbgPrintSegment("Case 2: free Seg to be removed", segPtr);
                        removeFreeSeg = GT_TRUE;
                    }
                }

            } else {

                if (inSegPtr->rowsBase + inSegPtr->rowsAmount < segPtr->rowsBase + segPtr->rowsAmount)
                {
                    /* 2 free segments
                       ---------
                       | Free  |
                       ---------
                       | inSeg |
                       ---------
                       | Free1 |
                       ---------
                       */
                    segPtr1 = cpssBmPoolBufGet(vTcamMngPtr->segmentsPool);
                    if (segPtr1 == NULL)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "segPtr1 allocation failed\n");
                    }
                    *segPtr1 = *segPtr;

                    segPtr1->rowsBase = inSegPtr->rowsBase + inSegPtr->rowsAmount;
                    segPtr1->rowsAmount = segPtr->rowsBase + segPtr->rowsAmount -
                        (inSegPtr->rowsBase + inSegPtr->rowsAmount);
                    segPtr1->lookupId   = inSegPtr->lookupId;

                    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtr);
                    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId2, segPtr);

                    cpssOsMemCpy(&saveSeg, segPtr, sizeof(saveSeg));
                    segPtr->rowsAmount = inSegPtr->rowsBase - segPtr->rowsBase;
                    segPtr->lookupId   = inSegPtr->lookupId;

                    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                    if (rc != GT_OK)
                    {
                        cpssBmPoolBufFree(vTcamMngPtr->segmentsPool, segPtr1);
                        cpssOsMemCpy(segPtr, &saveSeg, sizeof(saveSeg));
                        prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed to SegFreeLuTree in prvCpssDxChVirtualTcamHaFreeSegmentSplit\n");
                    }

                    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId2, segPtr);
                    if (rc != GT_OK)
                    {
                        cpssBmPoolBufFree(vTcamMngPtr->segmentsPool, segPtr1);
                        cpssOsMemCpy(segPtr, &saveSeg, sizeof(saveSeg));
                        prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                        prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId2, segPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed to SegFreePrefTree in prvCpssDxChVirtualTcamHaFreeSegmentSplit\n");
                    }

                    reItr = GT_TRUE;

                    prvCpssDxChVirtualTcamHaDbgPrintSegment("Case 3: free Seg", segPtr);
                } else {

                    /* 1 free segments. inSeg occupies the second half.
                       ---------
                       | Free  |
                       ---------
                       | inSeg |
                       |       |
                       |       |
                       ---------
                       */
                    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtr);
                    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId2, segPtr);

                    cpssOsMemCpy(&saveSeg, segPtr, sizeof(saveSeg));
                    segPtr->rowsAmount = inSegPtr->rowsBase - segPtr->rowsBase;
                    segPtr->lookupId   = inSegPtr->lookupId;

                    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                    if (rc != GT_OK)
                    {
                        cpssOsMemCpy(segPtr, &saveSeg, sizeof(saveSeg));
                        prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed to SegFreeLuTree in prvCpssDxChVirtualTcamHaFreeSegmentSplit\n");
                    }

                    rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId2, segPtr);
                    if (rc != GT_OK)
                    {
                        cpssOsMemCpy(segPtr, &saveSeg, sizeof(saveSeg));
                        prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr);
                        prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId2, segPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed to SegFreePrefTree in prvCpssDxChVirtualTcamHaFreeSegmentSplit\n");
                    }

                    reItr = GT_TRUE;

                    prvCpssDxChVirtualTcamHaDbgPrintSegment("Case 4: free Seg", segPtr);
                }
            }

            if (segPtr1 != NULL)
            {
                rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId, segPtr1);
                if (rc != GT_OK)
                {
                    cpssBmPoolBufFree(vTcamMngPtr->segmentsPool, segPtr1);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed to SegFreeLuTree in prvCpssDxChVirtualTcamHaFreeSegmentSplit\n");
                }

                rc = prvCpssDxChVirtualTcamSegmentTableAvlItemInsert(treeId2, segPtr1);
                if (rc != GT_OK)
                {
                    prvCpssDxChVirtualTcamSegmentTableAvlItemRemove(treeId, segPtr1);
                    cpssBmPoolBufFree(vTcamMngPtr->segmentsPool, segPtr1);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Insert failed to SegFreePrefTree in prvCpssDxChVirtualTcamHaFreeSegmentSplit\n");
                }

                reItr = GT_TRUE;

                prvCpssDxChVirtualTcamHaDbgPrintSegment("Case : 2nd free Seg", segPtr1);
                segPtr1 = NULL;
            }

            prvCpssDxChVirtualTcamDbSegmentTableUpdateBlockFreeSpace(&vTcamMngPtr->tcamSegCfg, inSegPtr, GT_TRUE);
        }
    }

    if (outSegPtr == NULL)
    {
        outSegPtr = cpssBmPoolBufGet(vTcamMngPtr->segmentsPool);

        if (outSegPtr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "outSegPtr allocation failed\n");
        }

        *outSegPtr =  *inSegPtr;
    }

    *outSegPtrPtr = outSegPtr;

    return rc;
}

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
)
{
    GT_STATUS rc;
    GT_U32    pattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS];

    cpssOsMemSet(pattern, 0, sizeof(pattern));

    rc = cpssDxChTcamPortGroupRuleWrite(devNum,portGroupsBmp,index,
            GT_FALSE,
            ruleSize,
            pattern,
            pattern);

    return rc;
}

/**
 * @internal perDevice_ruleClear function
 * @endinternal
 *
 * @brief   Clear Rule in the TCAM of specific device.
 *
 * @note   APPLICABLE DEVICES:      xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 * @note   NOT APPLICABLE DEVICES:  None.
 *
 * @param[in] devNum                   - the device number.
 * @param[in] ruleIndex                - the rule index in the TCAM. (CPSS API physical index)
 * @param[in] devClass                 - the device cllass, used to access the correct API on legacy devices
 * @param[in] ruleSize                 - the rule size, used to compute actual HW rule index in certain devices
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong parameters
 * @retval GT_HW_ERROR              - on hardware error
 */
static GT_STATUS perDevice_ruleClear
(
    IN  GT_U8                                              devNum,
    IN  GT_PORT_GROUPS_BMP                                 portGroupsBmp,
    IN  GT_U32                                             ruleIndex,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT    devClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT               ruleSize
)
{
    switch(devClass)
    {
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E:
            return prvCpssDxChVirtualTcamHaRuleClear(devNum, portGroupsBmp, ruleIndex, tcamRuleSizeMap[ruleSize]);

        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E:
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "perDevice_ruleClear not supported\n");
    }

    return GT_OK;
}

GT_STATUS perDevice_tcamClear
(
    IN  GT_U8                                              devNum,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT    devClass
)
{
    GT_STATUS                     rc = GT_OK;
    GT_U32                        totalEntries, i;
    GT_PORT_GROUPS_BMP            portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

    totalEntries = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm;

    for(i=0; i < totalEntries; ++i)
    {
        rc = perDevice_ruleClear(devNum, portGroupsBmp, i, devClass, CPSS_DXCH_TCAM_RULE_SIZE_10_B_E);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"perDevice_ruleClear failed \n");
        }
    }

    return GT_OK;
}

/**
 * @internal  prvCpssDxChVirtualTcamHaMarkBlockFree
 * @endinternal
 *
 * @brief  Fn is used to mark single block as Free/Unused by vTcam Manager
           by setting first rule of the block to be of invalid ruleSize
 */
static GT_STATUS prvCpssDxChVirtualTcamHaMarkBlockFree
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              rowsBase,
    IN  GT_U32                              columnsBase
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 physicalIndex;

    physicalIndex   = rowsBase * 12 + columnsBase;

    rc = prvCpssDxChTcamHaRuleSizeSet(devNum, portGroupsBmp, physicalIndex, PRV_CPSS_DXCH_TCAM_HA_INVALID_RULE_SIZE_CNS);
    if (rc !=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "\nprvCpssDxChTcamHaRuleSizeSet failed.\n");
    }

    return GT_OK;
}

/**
 * @internal  prvCpssDxChVirtualTcamHaMarkBlockUsed
 * @endinternal
 *
 * @brief  Fn. to mark the blocks(in TCAM) when assigned to a lookup as used
 *         makes first rule of the block to valid ruleSize(0)
 */
static GT_STATUS prvCpssDxChVirtualTcamHaMarkBlockUsed
(
    IN  GT_U8                               devNum,
    IN  GT_PORT_GROUPS_BMP                  portGroupsBmp,
    IN  GT_U32                              rowsBase,
    IN  GT_U32                              columnsBase
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 physicalIndex;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT ruleSize;

    physicalIndex   = rowsBase * 12 + columnsBase;

     if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
         /*in sip6_10 devices 10-bit rules can start only on odd inexes */
         ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E;
     }
     else
     {
         ruleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E;
     }

    rc = prvCpssDxChVirtualTcamHaRuleClear(devNum, portGroupsBmp, physicalIndex, ruleSize);
    if (rc !=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "\nprvCpssDxChVirtualTcamHaRuleClear failed.\n");
    }

    return GT_OK;
}


/**
 * @internal  prvCpssDxChVirtualTcamHaAllBlocksMarkValid
 * @endinternal
 *
 * @brief  Fn is used to mark each block as Used by vTcam Manager during remove
           by setting first rule of each block to be of valid ruleSize(0)
 */
static GT_STATUS prvCpssDxChVirtualTcamHaAllBlocksMarkValid
(
    IN   GT_U8                  devNum,
    IN   GT_PORT_GROUPS_BMP     portGroupsBmp,
    IN   GT_U32                 vTcamMngId
)
{
    GT_STATUS rc;
    GT_U32    j, baseRow, baseColumn, blocksAmount;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockInfArr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vtcamMng not Initialized\n");
    }

    blocksAmount = vTcamMngDBPtr->tcamSegCfg.blocksAmount;
    blockInfArr  = vTcamMngDBPtr->tcamSegCfg.blockInfArr;

    for(j=0;j<blocksAmount;++j)
    {

        baseRow         = blockInfArr[j].rowsBase;
        baseColumn      = blockInfArr[j].columnsBase;

        rc = prvCpssDxChVirtualTcamHaMarkBlockUsed(devNum, portGroupsBmp, baseRow, baseColumn);
        if (rc !=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "\nprvCpssDxChVirtualTcamHaMarkBlockUsed failed.\n");
        }

    }

    return GT_OK;
}

/**
 * @internal  prvCpssDxChVirtualTcamHaAllBlocksMarkFree
 * @endinternal
 *
 * @brief  Fn is used to mark each block as Free/Unused by vTcam Manager during Init
           by setting first rule of each block to be of invalid ruleSize
 */
static GT_STATUS prvCpssDxChVirtualTcamHaAllBlocksMarkFree
(
    IN   GT_U8                  devNum,
    IN   GT_PORT_GROUPS_BMP     portGroupsBmp,
    IN   GT_U32                 vTcamMngId
)
{
    GT_STATUS rc;
    GT_U32    j, baseRow, baseColumn, blocksAmount;
    GT_U32    physicalIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockInfArr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vtcamMng not Initialized\n");
    }

    blocksAmount = vTcamMngDBPtr->tcamSegCfg.blocksAmount;
    blockInfArr  = vTcamMngDBPtr->tcamSegCfg.blockInfArr;

    for(j=0;j<blocksAmount;++j)
    {

        baseRow         = blockInfArr[j].rowsBase;
        baseColumn      = blockInfArr[j].columnsBase;

        physicalIndex   = baseRow * 12 +  baseColumn;

        rc = prvCpssDxChTcamHaRuleSizeSet(devNum, portGroupsBmp, physicalIndex, PRV_CPSS_DXCH_TCAM_HA_INVALID_RULE_SIZE_CNS);
        if (rc !=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "\nprvCpssDxChTcamHaRuleSizeSet failed.\n");
        }

    }

    return GT_OK;
}

/**
 * @internal  prvCpssDxChVirtualTcamHaVtcamMngClear
 * @endinternal
 *
 * @brief  deletes all vTCAM rules part of the Manger to be removed
 *         Also, validates(changes invalid ruleSize) the 1st rule of each TCAM block
 *
 */
static GT_STATUS prvCpssDxChVirtualTcamHaVtcamMngClear
(
    IN   GT_U8                                  devNum,
    IN   GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN   GT_U32                                 vTcamMngId
)
{
    GT_STATUS           rc;
    GT_U32              activatedfloorIndex;
    GT_U32              tcamFloorEntriesNum, tcamEntriesNum, tcamFloorsNum;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    if (VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]) == NULL)
    {
        /* can't find the vTcamMng DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vtcamMng not Initialized\n");
    }

    /* this function should not remove rules from TCAM it done by caller */

    tcamFloorEntriesNum =
        CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS;

    rc = cpssDxChCfgTableNumEntriesGet(
            devNum, CPSS_DXCH_CFG_TABLE_TTI_TCAM_E, &tcamEntriesNum);

    tcamFloorsNum = (tcamEntriesNum / tcamFloorEntriesNum);

    /* Power on the block before connecting to vTcam manager
     * Skip the power on if its more number of floors alredy up.
     * NOTE: All the the blocks are powered down at INIT stage.*/
    rc = cpssDxChTcamPortGroupActiveFloorsGet(
            devNum, portGroupsBmp, &activatedfloorIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Power on the whole TCAM temporarily to mark blocks valid */
    rc = cpssDxChTcamPortGroupActiveFloorsSet(devNum, portGroupsBmp, tcamFloorsNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChVirtualTcamHaAllBlocksMarkValid(devNum, portGroupsBmp, vTcamMngId);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamHaAllBlocksMarkValid failed\n");
    }

    /* Restore Power to the state of vTCAM Manager after marking blocks Free */
    rc = cpssDxChTcamPortGroupActiveFloorsSet(devNum, portGroupsBmp, activatedfloorIndex);

    return rc;
}

/*******************************************************************************
* prvCpssDxChVirtualTcamRuleFormatToRuleSize
*
* Description:
*     This Function maps PCL TCAM rule formats to virtual TCAM size enumeration.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       ruleFormat      - PCL rule format.
*
* OUTPUT:
*       ruleSizePtr   - [Pointer to] rule size corresponding to ruleFormat
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters (invalid rule format)
*
*  Comments:
*       None
*
*******************************************************************************/
static GT_STATUS prvCpssDxChVirtualTcamRuleFormatToRuleSize
(
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT *ruleSizePtr
)
{
    switch (ruleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E                     /*0*/   :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E                  /*1*/   :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E                    /*2*/   :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E                   /*3*/   :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E                      /*7*/   :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E                   /*8*/   :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E                     /*9*/   :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E                        /*13*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E        /*15*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E                         /*23*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E                          /*29*/  :

            *ruleSizePtr = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E                   /*4*/   :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E                    /*5*/   :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E                    /*6*/   :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E                    /*10*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E                     /*11*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E                     /*12*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E                        /*14*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E         /*16*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E              /*19*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E                         /*26*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E                          /*32*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E         /*33*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_NO_FIXED_FIELDS_E          /*35*/  :

            *ruleSizePtr= CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E                         /*21*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E                          /*27*/  :

            *ruleSizePtr = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E;
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E                         /*22*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E                          /*28*/  :

            *ruleSizePtr = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E;
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E                         /*24*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E                          /*30*/  :

            *ruleSizePtr = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E;
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E                         /*25*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E                          /*31*/  :

            *ruleSizePtr = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E;
            break;

        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E       /*17*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E      /*18*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E            /*20*/  :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_80_E                         /*34*/  :
            *ruleSizePtr = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E;
            break;
        default:

            /* Rule format is not supported at virtual tcam */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/*******************************************************************************
* prvCpssDxChVirtualTcamRuleIndexToRulePclIndex
*
* Description:
*     This Function maps virtual TCAM rule index to the index to be used on PCL TCAM API,
*      depending on device family.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       devnum      - Device number.
*       ruleSize       - Rule size corresponding to ruleFormat
*       ruleIndex      - Virtual TCAM rule index.
*
* OUTPUT:
*       rulePclIndexPtr   - [Pointer to] rule index to be used on PCL TCAM API
*
* RETURNS:
*       GT_OK                    - on success
*       GT_BAD_PARAM             - on wrong parameters (either wrong rule size or incorrect alignment)
*
*  Comments:
*       None
*
*******************************************************************************/
static GT_STATUS prvCpssDxChVirtualTcamRuleIndexToRulePclIndex
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                                      ruleIndex,
    OUT GT_U32                                     *rulePclIndexPtr
)
{
     /*Lock the access to per device data base in order to avoid corruption*/
    /* check that device exist */
    PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /* No Adjustment needed on BC2/3/BobK */
        *rulePclIndexPtr = ruleIndex;
    }
    else
    {
        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /* Adjust the rule index according to xCat3 PCL index rules */
        /* ruleIndex unit is a standard rule size */
        switch (ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:

                /* Std. rule  No Adjustment needed */
                *rulePclIndexPtr = ruleIndex;
                break;

            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:

                /* Ext. rule like 2 Std. rules */
                if ((ruleIndex % 2) != 0)
                {
                    /* Must be correctly aligned */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *rulePclIndexPtr = ruleIndex / 2;
                break;

            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:

                /* Ultra. rule aligned like 4 Std. rules */
                if ((ruleIndex % 4) != 0)
                {
                    /* Must be correctly aligned */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                *rulePclIndexPtr = ruleIndex / 4;
                break;

            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/* get tcam size */
#define GET_TCAM_SIZE_MAC(_devNum,_tcamType)  \
    PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize._tcamType

/**
* @enum RESIZE_MODE_ENT
 *
 * @brief Resize modes.
*/
typedef enum{

    /** Move rules with Rule pointed by index */
    RESIZE_MODE_WITH_INPUT_INDEX_E      ,

    /** Move rules without Rule pointed by index */
    RESIZE_MODE_WITHOUT_INPUT_INDEX_E

} RESIZE_MODE_ENT;

static GT_STATUS vtcamDbSegmentTableVTcamResize
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_U32                                      resizeValue,
    IN  RESIZE_MODE_ENT                             resizeMode,
    IN  GT_U32                                      resizeReferenceLogicalIndex
);

/*
 * prvCpssDxChVirtualTcamManagerBlockToLookupMap
 *
 * Description:
 *     This Function maps TCAM segment to lookup (i.e. pair of client_group and hit_number).
 *
 * INPUTS:
 *       cookiePtr      - device info passed to callback function.
 *       baseRow        - base row of mapped segment.
 *       baseColumn     - base column of mapped segment.
 *       lookupId       - id of lookup for mapping.
 *
 *  Comments:
 *
 */
static GT_STATUS prvCpssDxChVirtualTcamManagerBlockToLookupMap
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      baseRow,
    IN  GT_U32      baseColumn,
    IN  GT_U32      lookupId
)
{
    GT_STATUS rc;                                        /* return code           */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;  /* vTCAM Manager pointer */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *segTabCfgPtr;
    GT_U8     devNum = 0;                                /* device number         */
    GT_U32    i, min, max;                               /* loop bounds           */
    GT_U32    floorIndex;                                /* floor Index           */
    GT_U32    activatedfloorIndex;                       /* Powered up floor Index*/
    CPSS_DXCH_TCAM_BLOCK_INFO_STC  floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    GT_U32    numBanksForHitNumGranularity = 0;
    GT_BOOL   managerHwWriteBlock;    /*used to check if current manager is blocked for HW write ,used for parallel High Availability*/
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    segTabCfgPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC*)cookiePtr;
    vTcamMngDBPtr = segTabCfgPtr->pVTcamMngPtr;


    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr, &devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    floorIndex = baseRow / 256;

    /* Lock the access to per device database */
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    if (devNum < PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        numBanksForHitNumGranularity =  /* in BC2/BC3 = 2 , in BobK/Aldrin = 6 */
            PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;
    }

    /* Unlock the access to per device database */
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    /* iteration over the devices */
    do
    {
        rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet(
            devNum, vTcamMngDBPtr->portGroupBmpArr[devNum], floorIndex, floorInfoArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (baseColumn < 6)
        {
            /* lower half floor */
            min = 0;
            max = numBanksForHitNumGranularity/2;
        }
        else
        {
            /* higher half floor */
            min = numBanksForHitNumGranularity/2;
            max = numBanksForHitNumGranularity;
        }

        for (i = min; (i < max); i++)
        {
            floorInfoArr[i].group  =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_TO_CLIENT_GROUP_MAC(lookupId);
            floorInfoArr[i].hitNum =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_TO_HIT_NUMBER_MAC(lookupId);
        }

        /* Power on the block before connecting to vTcam manager
         * Skip the power on if its more number of floors alredy up.
         * NOTE: All the the blocks are powered down at INIT stage.*/
        rc = cpssDxChTcamPortGroupActiveFloorsGet(
            devNum, vTcamMngDBPtr->portGroupBmpArr[devNum], &activatedfloorIndex);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* floorIndex (0-11) -> numberOfFloors(1-12) to cpssDxChTcamActiveFloorsSet */
        if(activatedfloorIndex < floorIndex+1)
        {
            rc = cpssDxChTcamPortGroupActiveFloorsSet(
                devNum, vTcamMngDBPtr->portGroupBmpArr[devNum], floorIndex+1);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        for (i = min; (i < max); i++)
        {
            /* Set the block number bit in that floor to indicate the block is in use
             * This will help in identifying block level usage inturn to powering off the floor */
            vTcamMngDBPtr->poweredOnBlocksBitMap[floorIndex] |= (1 >> i);
        }

        rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
            devNum, vTcamMngDBPtr->portGroupBmpArr[devNum], floorIndex, floorInfoArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_TCAM_MANAGER_E);
        if (vTcamMngDBPtr->haSupported && tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E &&
            managerHwWriteBlock == GT_FALSE)
        {
            for(i=0;i < vTcamMngDBPtr->tcamSegCfg.blocksAmount;i++)
            {
                if(vTcamMngDBPtr->tcamSegCfg.blockInfArr[i].rowsBase == baseRow &&
                        vTcamMngDBPtr->tcamSegCfg.blockInfArr[i].columnsBase == baseColumn &&
                        vTcamMngDBPtr->tcamSegCfg.blockInfArr[i].lookupId == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                {
                    /* Mark Block to be used in H/w by vTcam Manager */
                    rc = prvCpssDxChVirtualTcamHaMarkBlockUsed(devNum, vTcamMngDBPtr->portGroupBmpArr[devNum], baseRow, baseColumn);
                    if (rc !=GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "\nprvCpssDxChVirtualTcamHaMarkBlockUsed failed.\n");
                    }
                    break;
                }
            }
        }

    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr, &devNum));

    return GT_OK;
}



/*
 * prvCpssDxChVirtualTcamManagerBlockFromLookupUnmap
 *
 * Description:
 *     This Function unmaps TCAM segment from any lookup (called at freing block).
 *
 * INPUTS:
 *       cookiePtr      - device info passed to callback function.
 *       baseRow        - base row of mapped segment.
 *       baseColumn     - base column of mapped segment.
 *
 *  Comments:
 *
 */
static GT_STATUS prvCpssDxChVirtualTcamManagerBlockFromLookupUnmap
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      baseRow,
    IN  GT_U32      baseColumn
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC              *vTcamMngDBPtr;  /* vTCAM Manager pointer */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC   *segTabCfgPtr;
    GT_U32      floorIndex;                         /* floor Index */
    GT_U32      i, min, max;                        /* loop bounds */
    GT_U32      floorNumWithRule = 0;               /* Number of floors with rule */
    GT_U32      numBanksForHitNumGranularity = 0;
    GT_U32      activatedfloorIndex;                /* Powered up floor Index*/
    GT_U8       devNum = 0;                         /* device number */
    GT_STATUS   rc;                                 /* return code */
    GT_BOOL     managerHwWriteBlock;    /*used to check if current manager is blocked for HW write ,used for parallel High Availability*/
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    floorIndex = baseRow / CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS;
    segTabCfgPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *) cookiePtr;
    vTcamMngDBPtr = segTabCfgPtr->pVTcamMngPtr;


    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr, &devNum);
    if(rc != GT_OK)
    {
        /* TCAM manager init will call this API before adding any device
         * return GT_OK, when no device added to vTCAM */
        return GT_OK;
    }

    do
    {
        /* Lock the access to per device database */
        CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if (devNum < PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            numBanksForHitNumGranularity =  /* in BC2/BC3 = 2 , in BobK/Aldrin = 6 */
                PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity;
        }

        /* Unlock the access to per device database */
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if (baseColumn < 6)
        {
            /* lower half floor */
            min = 0;
            max = numBanksForHitNumGranularity/2;
        }
        else
        {
            /* higher half floor */
            min = numBanksForHitNumGranularity/2;
            max = numBanksForHitNumGranularity;
        }

        for (i = min; (i < max); i++)
        {
            /* Set the block number bit in that floor to indicate the block is in use
             * This will help in identifying block level usage inturn to powering off the floor */
            vTcamMngDBPtr->poweredOnBlocksBitMap[floorIndex] &= ~(1 >> i);
        }

        /* Check the highest floor number having rule, need to power on till that floor */
        for(i = 0; i<CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS; i++)
        {
            if (vTcamMngDBPtr->poweredOnBlocksBitMap[i] != 0)
            {
                floorNumWithRule = i+1;     /* Number of floors is Index+1 */
            }
        }

        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_TCAM_MANAGER_E);
        if (vTcamMngDBPtr->haSupported && tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E &&
            managerHwWriteBlock == GT_FALSE)
        {
            for(i=0;i < vTcamMngDBPtr->tcamSegCfg.blocksAmount;i++)
            {
                if(vTcamMngDBPtr->tcamSegCfg.blockInfArr[i].rowsBase == baseRow &&
                        vTcamMngDBPtr->tcamSegCfg.blockInfArr[i].columnsBase == baseColumn &&
                        vTcamMngDBPtr->tcamSegCfg.blockInfArr[i].lookupId == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
                {
                    rc = prvCpssDxChVirtualTcamHaMarkBlockFree(devNum, vTcamMngDBPtr->portGroupBmpArr[devNum], baseRow, baseColumn);
                    if (rc !=GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "\nprvCpssDxChVirtualTcamHaMarkBlockFree failed.\n");
                    }
                    break;
                }
            }
        }

        rc = cpssDxChTcamPortGroupActiveFloorsGet(
            devNum, vTcamMngDBPtr->portGroupBmpArr[devNum], &activatedfloorIndex);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(activatedfloorIndex < floorNumWithRule)
        {
            rc = cpssDxChTcamPortGroupActiveFloorsSet(
                devNum, vTcamMngDBPtr->portGroupBmpArr[devNum], floorNumWithRule);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

    }

    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr, &devNum));
    return GT_OK;
}

/*
 * prvCpssDxChVirtualTcamManagerRuleInvalidate
 *
 * Description:
 *     This Function invalidates rule in TCAM.
 *
 * INPUTS:
 *       cookiePtr            - device info passed to callback function.
 *       rulePhysicalIndex    - rule physical index.
 *       ruleSize             - rule size in TCAM minimal rules (10 bytes)
 *
 *  Comments:
 *
 */
static GT_STATUS prvCpssDxChVirtualTcamManagerRuleInvalidate
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      rulePhysicalIndex,
    IN  GT_U32      ruleSize
)
{
    GT_STATUS                           rc;              /* return code           */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;  /* vTCAM Manager pointer */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *segTabCfgPtr;
    GT_U8                               devNum;          /* device number    */

    segTabCfgPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *) cookiePtr;
    vTcamMngDBPtr = segTabCfgPtr->pVTcamMngPtr;

    vTcamMngDBPtr->ruleInvalidateCounter ++ ;

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr, &devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* iteration over the devices */
    do
    {
        /* perDevice_ruleInvalidate not needed device DB locking */
        /* invaliddate the rule in the device */
        rc = perDevice_ruleInvalidate(
            devNum, vTcamMngDBPtr,
            vTcamMngDBPtr->portGroupBmpArr[devNum],
            rulePhysicalIndex,
            segTabCfgPtr->deviceClass,
            (CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT) (ruleSize - 1));
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr, &devNum));

    return GT_OK;
}

/*
 * prvCpssDxChVirtualTcamManagerRuleMove
 *
 * Description:
 *     This Function moves rule in TCAM.
 *
 * INPUTS:
 *       cookiePtr               - device info passed to callback function.
 *       srcRulePhysicalIndex    - source rule physical index.
 *       dstRulePhysicalIndex    - destination rule physical index.
 *       ruleSize                - rule size in TCAM minimal rules (10 bytes)
 *
 *  Comments:
 *
 */
static GT_STATUS prvCpssDxChVirtualTcamManagerRuleMove
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      srcRulePhysicalIndex,
    IN  GT_U32      dstRulePhysicalIndex,
    IN  GT_U32      ruleSize
)
{
    GT_STATUS                           rc;              /* return code           */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;  /* vTCAM Manager pointer */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC       *segTabCfgPtr;
    GT_U8                               devNum;          /* device number         */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT ruleSizeEnum;

    segTabCfgPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *) cookiePtr;
    vTcamMngDBPtr = segTabCfgPtr->pVTcamMngPtr;

    ruleSizeEnum =
        prvCpssDxChVirtualTcamSegmentTableRuleSizeToEnum(
            segTabCfgPtr, ruleSize);

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* iteration over the devices */
    do
    {
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /* move the rule in the device */
        rc = perDevice_ruleMove(
            devNum,
            vTcamMngDBPtr,
            vTcamMngDBPtr->portGroupBmpArr[devNum],
            segTabCfgPtr->deviceClass,
            ruleSizeEnum,
            srcRulePhysicalIndex,
            dstRulePhysicalIndex,
            GT_TRUE/*move*/);

        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamHaHwDbWrite function
* @endinternal
*
* @brief   Writes to HW DB pair of vTcamId and ruleId for HW rule index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] segmentDbCfgPtr         - (pointer to)segment configuration DB.
* @param[in] ruleHwIndex             - rule physical index.
* @param[in] vTcamId                 - vTCAM Id.
* @param[in] ruleId                  - rule Id
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvCpssDxChVirtualTcamHaHwDbWrite
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *segmentDbCfgPtr,
    IN  GT_U32                                       ruleHwIndex,
    IN  GT_U32                                       vTcamId,
    IN  GT_U32                                       ruleId
)
{
    GT_STATUS                           rc;              /* return code           */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;  /* vTCAM Manager pointer */
    GT_U8                               devNum;          /* device number         */

    if (segmentDbCfgPtr->hwDbWriteFuncPtr == NULL)
    {
        /* HW DB write function not bound */
        return GT_OK;
    }
    vTcamMngDBPtr = segmentDbCfgPtr->pVTcamMngPtr;
    if (vTcamMngDBPtr->haSupported == GT_FALSE)
    {
        /* no HA support */
        return GT_OK;
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* iteration over the devices */
    do
    {
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        rc = segmentDbCfgPtr->hwDbWriteFuncPtr(
            devNum, vTcamMngDBPtr->portGroupBmpArr[devNum],
            ruleHwIndex, vTcamId, ruleId);
        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamHaHwDbRead function
* @endinternal
*
* @brief   Reads to HW DB pair of vTcamId and ruleId by HW rule index.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] segmentDbCfgPtr         - (pointer to)segment configuration DB.
* @param[in] ruleHwIndex             - rule physical index.
* @param[out] vTcamIdPtr             - (pointer to)vTCAM Id.
* @param[out] ruleIdPtr              - (pointer to)rule Id
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvCpssDxChVirtualTcamHaHwDbRead
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *segmentDbCfgPtr,
    IN  GT_U32                                       ruleHwIndex,
    OUT GT_U32                                       *vTcamIdPtr,
    OUT GT_U32                                       *ruleIdPtr
)
{
    GT_STATUS                           rc;              /* return code           */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;  /* vTCAM Manager pointer */
    GT_U8                               devNum;          /* device number         */

    if (segmentDbCfgPtr->hwDbReadFuncPtr == NULL)
    {
        /* HW DB read function not bound */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }
    vTcamMngDBPtr = segmentDbCfgPtr->pVTcamMngPtr;
    if (vTcamMngDBPtr->haSupported == GT_FALSE)
    {
        /* no HA support */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* read from first device */
    PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    rc = segmentDbCfgPtr->hwDbReadFuncPtr(
        devNum, vTcamMngDBPtr->portGroupBmpArr[devNum],
        ruleHwIndex, vTcamIdPtr, ruleIdPtr);
    CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamHaHwDbClearAll function
* @endinternal
*
* @brief   Clear all HW DB of pairs of vTcamId and ruleId.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] segmentDbCfgPtr         - (pointer to)segment configuration DB.
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvCpssDxChVirtualTcamHaHwDbClearAll
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *segmentDbCfgPtr
)
{
    GT_STATUS                           rc;              /* return code           */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;  /* vTCAM Manager pointer */
    GT_U32                              ruleHwIndex;     /* rule hw index         */
    GT_U32                              numOfHwRules;    /* amount of possible rules in TCAM */

    if (segmentDbCfgPtr->hwDbWriteFuncPtr == NULL)
    {
        /* HW DB write function not bound */
        return GT_OK;
    }
    vTcamMngDBPtr = segmentDbCfgPtr->pVTcamMngPtr;
    if (vTcamMngDBPtr->haSupported == GT_FALSE)
    {
        /* no HA support */
        return GT_OK;
    }

    numOfHwRules = (segmentDbCfgPtr->tcamColumnsAmount * segmentDbCfgPtr->tcamRowsAmount);

    for (ruleHwIndex = 0; (ruleHwIndex < numOfHwRules); ruleHwIndex++)
    {
        rc = prvCpssDxChVirtualTcamHaHwDbWrite(
            segmentDbCfgPtr, ruleHwIndex, 
            PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_DB_INVALID_VTCAM_ID, 
            PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_DB_INVALID_RULE_ID);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamHaHwDbCopyAll function
* @endinternal
*
* @brief   Copy all HW DB of pairs of vTcamId and ruleId to a new device.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] segmentDbCfgPtr         - (pointer to)segment configuration DB.
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvCpssDxChVirtualTcamHaHwDbCopyAll
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *segmentDbCfgPtr,
    IN GT_U8                                         devNum,
    IN GT_U32                                        portGroupsBmp
)
{
    GT_STATUS                           rc;                 /* return code           */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;     /* vTCAM Manager pointer */
    GT_U32                              ruleHwIndex;        /* rule hw index         */
    GT_U32                              numOfHwRules;       /* amount of possible rules in TCAM */
    GT_U32                              vTcamId;            /* VTCAM Id */
    GT_U32                              ruleId;             /* Rule Id  */
    GT_U8                               srcDevNum;          /* source device for copy */
    GT_U32                              srcPortGroupsBmp;   /* source port group bitmap for copy */

    if (segmentDbCfgPtr->hwDbWriteFuncPtr == NULL)
    {
        /* HW DB write function not bound */
        return GT_OK;
    }
    if (segmentDbCfgPtr->hwDbReadFuncPtr == NULL)
    {
        /* HW DB write function not bound */
        return GT_OK;
    }
    vTcamMngDBPtr = segmentDbCfgPtr->pVTcamMngPtr;
    if (vTcamMngDBPtr->haSupported == GT_FALSE)
    {
        /* no HA support */
        return GT_OK;
    }

    if (vTcamMngDBPtr->numOfDevices == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr, &srcDevNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    srcPortGroupsBmp = vTcamMngDBPtr->portGroupBmpArr[srcDevNum];
    
    numOfHwRules = (segmentDbCfgPtr->tcamColumnsAmount * segmentDbCfgPtr->tcamRowsAmount);

    for (ruleHwIndex = 0; (ruleHwIndex < numOfHwRules); ruleHwIndex++)
    {
        rc = segmentDbCfgPtr->hwDbReadFuncPtr(
            srcDevNum, srcPortGroupsBmp, ruleHwIndex, &vTcamId, &ruleId);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = segmentDbCfgPtr->hwDbWriteFuncPtr(
            devNum, portGroupsBmp, ruleHwIndex, vTcamId, ruleId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamSegmentDbByDevClassGet function
* @endinternal
*
* @brief   Get Segment Cfg DB by device class.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngDBPtr         - (pointer to)VTCAM Manager structure.
* @param[in] deviceClass        - device TCAM class: SIP5(and above), LEGACY_PCL, LEGACY_TTI, AC5_PCL_TCAM1.
*
* @retval GT_OK                    - on success, others on fail.
*/
PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC* prvCpssDxChVirtualTcamSegmentDbByDevClassGet
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *vTcamMngDBPtr,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT   deviceClass
)
{
    switch (deviceClass)
    {
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E:
            return &(vTcamMngDBPtr->tcamSegCfg);
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
            return &(vTcamMngDBPtr->tcamTtiSegCfg);
        case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E:
            return &(vTcamMngDBPtr->pclTcam1SegCfg);
        default: return NULL;
    }
}

/**
* @internal prvCpssDxChVirtualTcamHaHwDbAccessBind function
* @endinternal
*
* @brief   Bind HW DB access functions to Segment Cfg DB.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId         - VTCAM Manager Id.
* @param[in] deviceClass        - device TCAM class: SIP5(and above), LEGACY_PCL, LEGACY_TTI, AC5_PCL_TCAM1.
* @param[in] hwDbWriteFuncPtr   - (pointer to)write function.
* @param[in] hwDbReadFuncPtr    - (pointer to)read function
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS prvCpssDxChVirtualTcamHaHwDbAccessBind
(
    IN GT_U32                                            vTcamMngId,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT   deviceClass,
    IN CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_WRITE_FUNC    *hwDbWriteFuncPtr,
    IN CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_READ_FUNC     *hwDbReadFuncPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC            *vTcamMngDBPtr;  /* vTCAM Manager pointer */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *segmentDbCfgPtr; /* segment DB configurati0n pointer */

    if (vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if (vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "vtcam manager not creared\n");
    }

    segmentDbCfgPtr = prvCpssDxChVirtualTcamSegmentDbByDevClassGet(
        vTcamMngDBPtr, deviceClass);
    if (segmentDbCfgPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong device class\n");
    }

    segmentDbCfgPtr->hwDbWriteFuncPtr = hwDbWriteFuncPtr;
    segmentDbCfgPtr->hwDbReadFuncPtr  = hwDbReadFuncPtr;
    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamHaHwDbAccessBind function
* @endinternal
*
* @brief   Bind HW DB access functions to VTCAM Manager.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] vTcamMngId         - VTCAM Manager Id.
* @param[in] hwDbWriteFuncPtr   - (pointer to)write function.
* @param[in] hwDbReadFuncPtr    - (pointer to)read function
*
* @retval GT_OK                    - on success, others on fail.
*/
GT_STATUS cpssDxChVirtualTcamHaHwDbAccessBind
(
    IN GT_U32                                        vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_WRITE_FUNC    *hwDbWriteFuncPtr,
    IN CPSS_DXCH_VIRTUAL_TCAM_HA_HW_DB_READ_FUNC     *hwDbReadFuncPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamHaHwDbAccessBind);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, hwDbWriteFuncPtr, hwDbReadFuncPtr));
    rc = prvCpssDxChVirtualTcamHaHwDbAccessBind(
        vTcamMngId,
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E,
        hwDbWriteFuncPtr, hwDbReadFuncPtr);
    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, hwDbWriteFuncPtr, hwDbReadFuncPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamManagerInit function
* @endinternal
*
* @brief   Initialize Virtual TCAM manager structure.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] vTcamMngId               - virtual TCAM Manager ID
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory alocation fail
*/
static GT_STATUS internal_cpssDxChVirtualTcamManagerInit
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              vTcamMngId
)
{
    GT_STATUS                                            rc; /* return code */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC        tcamSegCfg;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC        tcamTtiSegCfg;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC        pclTcam1SegCfg;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr;
    GT_U32        maxSegmentsInDb;  /* Total segments in one or both classes */

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    pVTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);

    /* pass HA HW DB access functions pointers if were bound */

    cpssOsMemSet(&tcamSegCfg, 0, sizeof(tcamSegCfg));
    tcamSegCfg.hwDbReadFuncPtr  = pVTcamMngPtr->tcamSegCfg.hwDbReadFuncPtr;
    tcamSegCfg.hwDbWriteFuncPtr = pVTcamMngPtr->tcamSegCfg.hwDbWriteFuncPtr;

    cpssOsMemSet(&tcamTtiSegCfg, 0, sizeof(tcamTtiSegCfg));
    tcamTtiSegCfg.hwDbReadFuncPtr  = pVTcamMngPtr->tcamTtiSegCfg.hwDbReadFuncPtr;
    tcamTtiSegCfg.hwDbWriteFuncPtr = pVTcamMngPtr->tcamTtiSegCfg.hwDbWriteFuncPtr;

    cpssOsMemSet(&pclTcam1SegCfg, 0, sizeof(pclTcam1SegCfg));
    pclTcam1SegCfg.hwDbReadFuncPtr  = pVTcamMngPtr->pclTcam1SegCfg.hwDbReadFuncPtr;
    pclTcam1SegCfg.hwDbWriteFuncPtr = pVTcamMngPtr->pclTcam1SegCfg.hwDbWriteFuncPtr;

    if(!PRV_CPSS_SIP_5_25_CHECK_MAC(devNum))
    {
        if(GT_TRUE == pVTcamMngPtr->haSupported)
        {
            CPSS_LOG_INFORMATION_MAC("TCAM Manager High Availability not supported on this device [%d]. - will run in normal mode", devNum);
            pVTcamMngPtr->haSupported = GT_FALSE;
        }
    }

    /* columns are units of size equal to minimal rule on the device           */
    /* for SIP5 devices it is 10 bytes, for Legacy devices it is standard rule */
    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        tcamSegCfg.tcamColumnsAmount        = 12;
        tcamSegCfg.tcamRowsAmount           =
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm
             / tcamSegCfg.tcamColumnsAmount;
        tcamSegCfg.lookupColumnAlignment    = 6;
        tcamSegCfg.lookupRowAlignment       = 256;
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            tcamSegCfg.deviceClass =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E;
        }
        else
        {
            tcamSegCfg.deviceClass =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E;
        }

        /* no TTI segment table */
        tcamTtiSegCfg.deviceClass =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_INVALID_E;

        /* no PCL TCAM1 segment table */
        pclTcam1SegCfg.deviceClass =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_INVALID_E;
    }
    else
    {
        /*
         * xCat3/Lion2 TTI settings:
         *   fineTuning.tunnelTerm contains number of TTI Tcam entries,
         *   At the virtual Tcam, 1 columns is a TTI Tcam row.
         *   tcamColumnsAmount must be 1 columns in order to
         *   insure contiguous allocation,
         */
        tcamTtiSegCfg.tcamColumnsAmount        = 1;
        tcamTtiSegCfg.tcamRowsAmount           =
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm;
        /* On xCat3/Lion2 TTI: Only one huge block */
        tcamTtiSegCfg.lookupColumnAlignment    = tcamTtiSegCfg.tcamColumnsAmount;
        tcamTtiSegCfg.lookupRowAlignment       = tcamTtiSegCfg.tcamRowsAmount;
        tcamTtiSegCfg.maxSegmentsInDb = (tcamTtiSegCfg.tcamColumnsAmount * tcamTtiSegCfg.tcamRowsAmount);
        tcamTtiSegCfg.deviceClass =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E;
        tcamTtiSegCfg.cookiePtr                = (GT_VOID*)&pVTcamMngPtr->tcamTtiSegCfg;
        tcamTtiSegCfg.ruleInvalidateFuncPtr    = prvCpssDxChVirtualTcamManagerRuleInvalidate;
        tcamTtiSegCfg.ruleMoveFuncPtr          = prvCpssDxChVirtualTcamManagerRuleMove;

        /*
         * xCat3/Lion2 PCL settings:
         *   fineTuning.policyTcamRaws contains number of PCL Tcam rows,
         *   and each row contains either 4 standard or 2 extended rules
         *   or one ultra rule and one standard rule.
         *   At the virtual Tcam, 4 columns are a PCL Tcam row.
         *   tcamColumnsAmount must be 4 columns in order to
         *   insure contiguous allocation,
         */
        tcamSegCfg.tcamColumnsAmount        = 4;
        tcamSegCfg.tcamRowsAmount           =
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policyTcamRaws;
        /* On xCat3/AC5/Lion2 PCL: Only one huge block */
        tcamSegCfg.lookupColumnAlignment    = tcamSegCfg.tcamColumnsAmount;
        tcamSegCfg.lookupRowAlignment       = tcamSegCfg.tcamRowsAmount;
        tcamSegCfg.deviceClass =
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E;

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
        {
            pclTcam1SegCfg.tcamColumnsAmount        = 4;
            pclTcam1SegCfg.tcamRowsAmount           =
                PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policyTcamRaws;
            /* On AC5 PCL: Only one huge block */
            pclTcam1SegCfg.lookupColumnAlignment    = pclTcam1SegCfg.tcamColumnsAmount;
            pclTcam1SegCfg.lookupRowAlignment       = pclTcam1SegCfg.tcamRowsAmount;
            pclTcam1SegCfg.maxSegmentsInDb          = (pclTcam1SegCfg.tcamColumnsAmount * pclTcam1SegCfg.tcamRowsAmount);
            pclTcam1SegCfg.deviceClass =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E;
            pclTcam1SegCfg.cookiePtr                = (GT_VOID*)&pVTcamMngPtr->pclTcam1SegCfg;
            pclTcam1SegCfg.ruleInvalidateFuncPtr    = prvCpssDxChVirtualTcamManagerRuleInvalidate;
            pclTcam1SegCfg.ruleMoveFuncPtr          = prvCpssDxChVirtualTcamManagerRuleMove;
        }
        else
        {
            /* no PCL TCAM1 segment table */
            pclTcam1SegCfg.deviceClass =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_INVALID_E;
        }
    }

    /* cookie for using prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst/Next */
    /* cookie for using  prvCpssDxChVirtualTcamManagerBlockToLookupMap      */
    /* cookie for using  prvCpssDxChVirtualTcamManagerRuleInvalidate        */
    /* cookie for using  prvCpssDxChVirtualTcamManagerRuleMove              */
    tcamSegCfg.cookiePtr                = (GT_VOID*)&pVTcamMngPtr->tcamSegCfg;
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        tcamSegCfg.lookupMapFuncPtr         = prvCpssDxChVirtualTcamManagerBlockToLookupMap;
        tcamSegCfg.lookupUnmapFuncPtr       = prvCpssDxChVirtualTcamManagerBlockFromLookupUnmap;
    }
    else
    {
        tcamSegCfg.lookupMapFuncPtr         = NULL;
        tcamSegCfg.lookupUnmapFuncPtr       = NULL;
    }
    tcamSegCfg.ruleInvalidateFuncPtr    = prvCpssDxChVirtualTcamManagerRuleInvalidate;
    tcamSegCfg.ruleMoveFuncPtr          = prvCpssDxChVirtualTcamManagerRuleMove;
    /* set maximal amount of segments as amount of 10-bytes rules in all of TCAM */
    tcamSegCfg.maxSegmentsInDb = (tcamSegCfg.tcamColumnsAmount * tcamSegCfg.tcamRowsAmount);

    /* currently this parameter not come from the application so set it here
       according to actual tcam size */
    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        pVTcamMngPtr->tcamNumEntriesInSmallestRuleSize = GET_TCAM_SIZE_MAC(devNum,tunnelTerm);
    }
    else
    {
        /*
         * xCat3/Lion2 TTI settings:
         *   fineTuning.tunnelTerm contains number of TTI Tcam entries,
         *   So that the number of entries in smallest rule size (i.e. standard
         *   rule size) is the number of TTI Tcam entries.
         */
        pVTcamMngPtr->tcamNumEntriesInSmallestRuleSize = GET_TCAM_SIZE_MAC(devNum,tunnelTerm);
        /*
         * xCat3/Lion2 PCL settings:
         *   fineTuning.policyTcamRaws contains number of PCL Tcam rows,
         *   and each row contains either 4 standard or 2 extended rules.
         *   So that the number of entries in smallest rule size (i.e. standard
         *   rule size) is four times the number of PCL Tcam rows.
         */
        pVTcamMngPtr->tcamNumEntriesInSmallestRuleSize += GET_TCAM_SIZE_MAC(devNum,policyTcamRaws) * 4;
    }

    pVTcamMngPtr->vTcamCfgPtrArrSize = CPSS_DXCH_VIRTUAL_TCAM_VTCAM_MAX_ID_CNS + 1;
    pVTcamMngPtr->vTcamCfgPtrArr =
        (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR*)cpssOsMalloc(
            sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR)
            * (CPSS_DXCH_VIRTUAL_TCAM_VTCAM_MAX_ID_CNS + 1));
    if (pVTcamMngPtr->vTcamCfgPtrArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(
        pVTcamMngPtr->vTcamCfgPtrArr, 0,
        (sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR)
            * (CPSS_DXCH_VIRTUAL_TCAM_VTCAM_MAX_ID_CNS + 1)));

    rc = prvCpssDxChVirtualTcamDbSegmentTableCreate(
        pVTcamMngPtr, &tcamSegCfg, &tcamTtiSegCfg, &pclTcam1SegCfg, GT_TRUE/*fillSegFreeTrees*/);
    if (rc != GT_OK)
    {
        cpssOsFree(pVTcamMngPtr->vTcamCfgPtrArr);
        return rc;
    }

    maxSegmentsInDb = tcamSegCfg.maxSegmentsInDb + tcamTtiSegCfg.maxSegmentsInDb + pclTcam1SegCfg.maxSegmentsInDb;
    rc = prvCpssDxChVirtualTcamDbRuleIdTableCreate(
        pVTcamMngPtr,
        maxSegmentsInDb /*maxEntriesInDb*/);
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
        cpssOsFree(pVTcamMngPtr->vTcamCfgPtrArr);
        return rc;
    }

    rc = prvCpssDxChVirtualTcamDbPriorityTableCreate(
        pVTcamMngPtr,
        maxSegmentsInDb /*maxEntriesInDb*/);
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbRuleIdTableDelete(pVTcamMngPtr);
        prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
        cpssOsFree(pVTcamMngPtr->vTcamCfgPtrArr);
        return rc;
    }

    return GT_OK;
}

/* forward declaration */
static GT_STATUS priorityDbRemoveRuleId(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *vTcamMngDBPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *vtcamInfoPtr,
    IN  GT_U32                                              logicalIndex
);

/**
* @internal prvCpssDxChVirtualTcamRuleDeleteFromDB function
* @endinternal
*
* @brief   Delete Rule From internal DB.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
static GT_STATUS prvCpssDxChVirtualTcamRuleDeleteFromDB
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId
)
{
    GT_STATUS   rc;
    GT_U32  deviceRuleIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U32  logicalIndex; /* logical index that converted from ruleId */

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* convert the ruleId to logical index and to physical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &logicalIndex, &deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (vtcamInfoPtr->usedRulesAmount == 0)
    {
        /* 'management' error */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* update the DB about the removed rule */
    vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] &= ~(1<<(logicalIndex & 0x1f));

    /* update the counter */
    vtcamInfoPtr->usedRulesAmount--;

    /* for priority mode - remove rule ID entry from DB */
    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
        {
            /* vTcamMngId out of range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
        }

        vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);

        rc = priorityDbRemoveRuleId(vTcamMngDBPtr, vtcamInfoPtr, logicalIndex);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamManagerDelete function
* @endinternal
*
* @brief   Delete Virtual TCAM manager structure contents.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - virtual TCAM Manager ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on TCAM manager not found
*/
static GT_STATUS prvCpssDxChVirtualTcamManagerDelete
(
    IN GT_U32   vTcamMngId
)
{
    GT_STATUS                                            rc; /* return code */
    GT_STATUS                                            totalRc; /* return code */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                   *pVTcamMngPtr;
    GT_U32                                               vTcamId;
    GT_U32                                               ruleId;

    totalRc = GT_OK;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    pVTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);

    if (pVTcamMngPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    if (pVTcamMngPtr->vTcamCfgPtrArr)
    {
        for (vTcamId = 0; (vTcamId < pVTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
        {
            if (pVTcamMngPtr->vTcamCfgPtrArr[vTcamId])
            {
                /* remove all rules */
                while (GT_OK == cpssDxChVirtualTcamNextRuleIdGet(
                    vTcamMngId, vTcamId, CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS, &ruleId))
                {
                    rc = prvCpssDxChVirtualTcamRuleDeleteFromDB(
                          vTcamMngId, vTcamId, ruleId);
                    if (rc != GT_OK)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        totalRc = rc;
                        break; /* to prevent endless loop */
                    }
                }
                /* remove vTcam from DB ignoring valid rules */
                /* all rules should be removed upper */
                rc = prvCpssDxChVirtualTcamRemove(vTcamMngId, vTcamId, GT_FALSE);
                if (rc != GT_OK)
                {
                    totalRc = rc;
                }
            }
        }
    }

    /* delete DB priority table */
    rc = prvCpssDxChVirtualTcamDbPriorityTableDelete(pVTcamMngPtr);
    if (rc != GT_OK)
    {
        totalRc = rc;
    }
    /* delete DB rule ID table */
    rc = prvCpssDxChVirtualTcamDbRuleIdTableDelete(pVTcamMngPtr);
    if (rc != GT_OK)
    {
        totalRc = rc;
    }
    /* delete DB segment table  */
    rc = prvCpssDxChVirtualTcamDbSegmentTableDelete(pVTcamMngPtr);
    if (rc != GT_OK)
    {
        totalRc = rc;
    }
    if (pVTcamMngPtr->vTcamCfgPtrArr)
    {
        /* free vTcam configuration structure */
        cpssOsFree(pVTcamMngPtr->vTcamCfgPtrArr);
        pVTcamMngPtr->vTcamCfgPtrArr = NULL;
    }

    return totalRc;
}

/**
* @internal internal_cpssDxChVirtualTcamManagerDelete function
* @endinternal
*
* @brief   Delete Virtual TCAM manager structure.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - virtual TCAM Manager ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_STATE             - on devices still attached to TCAM manager
* @retval GT_NOT_FOUND             - on TCAM manager not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamManagerDelete
(
    IN GT_U32   vTcamMngId
)
{
    GT_STATUS                                           rc; /* return code */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr;
    GT_U8                                               devNum;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    pVTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if (pVTcamMngPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(
        pVTcamMngPtr, &devNum);
    if (rc == GT_OK)
    {
        /* has yet not removed devices */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    cpssOsFree(pVTcamMngPtr);
    VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]) = NULL;

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamManagerDelete function
* @endinternal
*
* @brief   Delete Virtual TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on devices still attached to TCAM manager
* @retval GT_NOT_FOUND             - on TCAM manager not found
*/
GT_STATUS cpssDxChVirtualTcamManagerDelete
(
    IN  GT_U32                              vTcamMngId
)
{

    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamManagerDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId));

    rc = internal_cpssDxChVirtualTcamManagerDelete(vTcamMngId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamManagerCreate function
* @endinternal
*
* @brief   Create Virtual TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamMngCfgPtr           - Pointer to Configurable params of vTcam Mgr
*                                      (NULL pointer implies default config)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_ALREADY_EXIST         - if already exist TCAM Manager with given Id
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory allocation fail
*/
static GT_STATUS internal_cpssDxChVirtualTcamManagerCreate
(
    IN  GT_U32                                        vTcamMngId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC   *vTcamMngCfgPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *pVTcamMngPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    if (VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]) != NULL)
    {
        /* can't redefine an vTcamMng DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    pVTcamMngPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*)cpssOsMalloc(sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC));
    if (pVTcamMngPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]) = pVTcamMngPtr;

    cpssOsMemSet(pVTcamMngPtr, 0, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC));
    pVTcamMngPtr->vTcamMngId       = vTcamMngId;
    pVTcamMngPtr->autoResizeGranularity =
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DEFAULT_AUTO_RESIZE_GRANULARITY_CNS;

    if (vTcamMngCfgPtr)
    {
        pVTcamMngPtr->haSupported = vTcamMngCfgPtr->haSupportEnabled;
        pVTcamMngPtr->haFeaturesSupportedBmp = vTcamMngCfgPtr->haFeaturesEnabledBmp;
    }
    else
    {
        pVTcamMngPtr->haSupported = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamManagerCreate function
* @endinternal
*
* @brief   Create Virtual TCAM manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamMngCfgPtr           - Pointer to Configurable params of vTcam Mgr
*                                      (NULL pointer implies default config)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_ALREADY_EXIST         - if already exist TCAM Manager with given Id
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory allocation fail
*/
GT_STATUS cpssDxChVirtualTcamManagerCreate
(
    IN  GT_U32                                        vTcamMngId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC   *vTcamMngCfgPtr
)
{

    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamManagerCreate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamMngCfgPtr));

    rc = internal_cpssDxChVirtualTcamManagerCreate(vTcamMngId, vTcamMngCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamMngCfgPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamManagerDevPortGroupListAdd function
* @endinternal
*
* @brief  This function adds port groups of devices to an existing Virtual TCAM manager.
*         TCAM of one of port groups of old devices (if exist) copied
*         to TCAMs the port groups of newly added devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] devPortGroupsBmpArr[]    - the array device ids and port group bitmaps
*                                       added to the Virtual TCAM Manager.
* @param[in] numOfPortGroupBmps       - the number of device ids and port groups in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
*
* @note NONE
*
*/
static GT_STATUS prvCpssDxChVirtualTcamManagerDevPortGroupListAdd
(
    IN GT_U32                                        vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC  devPortGroupsBmpArr[],
    IN GT_U32                                        numOfPortGroupBmps
)
{
    GT_U8               i;
    GT_U8               devNum;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_PTR tcamMngDbPtr;
    GT_U32              tcamSize = 0;
    GT_STATUS           rc;
    GT_BOOL             noDevicesBeforAdding;
    GT_U8               representativeDevNum;
    GT_PORT_GROUPS_BMP  representativePortGroupsBmp;
    GT_U32              floorIndex;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    GT_BOOL             deviceAlreadyOwned;
    GT_U32              isSip5Dev, tcamFloorEntriesNum, tcamEntriesNum, tcamFloorsNum;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC    floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    GT_BOOL             managerHwWriteBlock;    /*used to check if current manager is blocked for HW write ,used for parallel High Availability*/
    GT_U32              start_sec  = 0;
    GT_U32              start_nsec = 0;
    GT_U32              end_sec    = 0;
    GT_U32              end_nsec   = 0;
    GT_U32              diff_sec   = 0;
    GT_U32              diff_nsec  = 0;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_BOOL             haVtcamDebug;
    GT_BOOL             haVtcamTimeDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);
    haVtcamTimeDebug = VTCAM_GLOVAR(haVtcamTimeDebug);

    /* prevent compiler warning */
    representativeDevNum = 0xFF;
    representativePortGroupsBmp = 0xFFFFFFFF;

    if (vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (numOfPortGroupBmps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CPSS_NULL_PTR_CHECK_MAC(devPortGroupsBmpArr);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    tcamMngDbPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if (tcamMngDbPtr == NULL)
    {
        /* can't find the vTcamMng DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* Check validity */
    for (i = 0; i < numOfPortGroupBmps; i++)
    {
        devNum        = devPortGroupsBmpArr[i].devNum;

        /* check device and port group bitmap */
        /* prvCpssDxChVirtualTcamFullTcamPortGroupBmpGet not needed device DB locking */
        rc =  prvCpssDxChVirtualTcamFullTcamPortGroupBmpGet(
            devNum, &portGroupsBmp);
        if (rc != GT_OK)
        {
            /* wrong devNum */
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* Single TCAM devices */
        if (portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
        {
            /* the device must be new */
            if (PRV_CPSS_VIRTUAL_TCAM_MNG_IS_DEV_BMP_SET_MAC(
                tcamMngDbPtr->devsBitmap, devNum))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* for Single TCAM devices - bitmap ignored */
            continue;
        }

        if (devPortGroupsBmpArr[i].portGroupsBmp == 0)
        {
            /* empty port group bitmap */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if ((devPortGroupsBmpArr[i].portGroupsBmp != CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
            && (devPortGroupsBmpArr[i].portGroupsBmp & (~ portGroupsBmp)))
        {
            /* adding not existing port group */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (PRV_CPSS_VIRTUAL_TCAM_MNG_IS_DEV_BMP_SET_MAC(
            tcamMngDbPtr->devsBitmap, devNum))
        {
            /* found device */
            if ((tcamMngDbPtr->portGroupBmpArr[devNum] == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
                || (devPortGroupsBmpArr[i].portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
                || (tcamMngDbPtr->portGroupBmpArr[devNum] & devPortGroupsBmpArr[i].portGroupsBmp))
            {
                /* adding already presenting port groups */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* check if the DB already hold existing device */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(tcamMngDbPtr,&representativeDevNum);
    if(rc == GT_OK)
    {
        /* there is at least one device in the TCAM manager prior to calling this function */
        /* so it may hold entries in the HW that need to copy to the new devices */

        /* representativeDevNum is valid device */
        noDevicesBeforAdding = GT_FALSE;
        representativePortGroupsBmp = tcamMngDbPtr->portGroupBmpArr[representativeDevNum];
    }
    else
    {
        /* there are no devices in the TCAM manager prior to calling this function */

        /* representativeDevNum is NOT valid device */
        noDevicesBeforAdding = GT_TRUE;
    }

    for (i = 0; i < numOfPortGroupBmps; i++)
    {
        devNum        = devPortGroupsBmpArr[i].devNum;

        /* already checked that GT_OK returned */
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        isSip5Dev = PRV_CPSS_SIP_5_CHECK_MAC(devNum);
        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /* prvCpssDxChVirtualTcamFullTcamPortGroupBmpGet not needed device DB locking */
        prvCpssDxChVirtualTcamFullTcamPortGroupBmpGet(
            devNum, &portGroupsBmp);
        if ((portGroupsBmp != CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
            && (devPortGroupsBmpArr[i].portGroupsBmp != CPSS_PORT_GROUP_UNAWARE_MODE_CNS))
        {
            /* for multiple TCAM devices - otherwise ignored */
            portGroupsBmp = devPortGroupsBmpArr[i].portGroupsBmp;
        }

        if (PRV_CPSS_VIRTUAL_TCAM_MNG_IS_DEV_BMP_SET_MAC(tcamMngDbPtr->devsBitmap, devNum))
        {
            deviceAlreadyOwned = GT_TRUE;
        }
        else
        {
            deviceAlreadyOwned = GT_FALSE;
        }

        if (deviceAlreadyOwned == GT_FALSE)
        {
            /* Support only xCat3 or SIP5 devices */
            CPSS_API_LOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            if((!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) &&
                (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
                (GT_FALSE == PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
            {
                CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
            }

            if ((noDevicesBeforAdding == GT_TRUE) && (i == 0))
            {
                tcamMngDbPtr->devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;
            }
            else
            {
                if(PRV_CPSS_PP_MAC(devNum)->devFamily != tcamMngDbPtr->devFamily)
                {   /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
                }
            }

            /* Check TCAM size */
            /* Get TCAM size for the current device */
            if(PRV_CPSS_SIP_5_CHECK_MAC(devNum))
            {
                tcamSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm;
            }
            else
            {
                /*
                 * xCat3/Lion2 TTI settings:
                 *   fineTuning.tunnelTerm contains number of TTI Tcam entries,
                 */
                tcamSize = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.tunnelTerm;
               /*
                * xCat3/Lion2 PCL settings:
                *   fineTuning.policyTcamRaws contains number of PCL Tcam rows,
                *   and each row contains either 4 standard or 2 extended rules.
                *   So that the number of entries in smallest rule size (i.e.
                *   standard rule size) is 4 times the number of PCL Tcam rows.
                */
                tcamSize += PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policyTcamRaws * 4;
            }
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

            if ((noDevicesBeforAdding == GT_TRUE) && (i == 0))
            {
                tcamMngDbPtr->tcamNumEntriesInSmallestRuleSize = tcamSize;
            }
            else
            {
                /* Check TCAM size of other devices */
                if (tcamSize < tcamMngDbPtr->tcamNumEntriesInSmallestRuleSize)
                {
                    /* the current added device hold TCAM with less entries than the
                        the size of the created one ... we can not support it */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(
                        GT_BAD_PARAM,
                        "the added device [%d] hold TCAM with less entries [%d] than the entries[%d] of the created one",
                        devNum, tcamSize, tcamMngDbPtr->tcamNumEntriesInSmallestRuleSize);
                }
            }
        }

        if ((noDevicesBeforAdding == GT_TRUE) && (i == 0))
        {
            /* Initialize TCAM manager structure for the first added device */
            CPSS_API_LOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            rc = internal_cpssDxChVirtualTcamManagerInit(devNum, vTcamMngId);
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_TCAM_MANAGER_E);
        if (isSip5Dev)
        {
            if (managerHwWriteBlock == GT_FALSE)
            {
                /* Power Down the whole TCAM at Init time of TCAM Manager
                 * Each block will be powered on, on-demand */
                rc = cpssDxChTcamPortGroupActiveFloorsSet(devNum, portGroupsBmp, 0);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

        if (deviceAlreadyOwned != GT_FALSE)
        {
            /* found device - validity already checked */
            tcamMngDbPtr->portGroupBmpArr[devNum] |= portGroupsBmp;
        }
        else
        {
            /* new device */
            /* update devices counter */
            tcamMngDbPtr->numOfDevices++;

            /* Add device to bitmap */
            PRV_CPSS_VIRTUAL_TCAM_MNG_DEV_BMP_SET_MAC(tcamMngDbPtr->devsBitmap, devNum);

            tcamMngDbPtr->portGroupBmpArr[devNum] = portGroupsBmp;

            /* use HW Shadow for read - ingnore return code - the feature can be not supported */
            prvCpssDxChTableReadFromShadowEnableSet(
                devNum, CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E, GT_TRUE);
            prvCpssDxChTableReadFromShadowEnableSet(
                devNum, CPSS_DXCH_SIP5_TABLE_TCAM_E, GT_TRUE);
        }

        if ((noDevicesBeforAdding == GT_TRUE)/*1st device*/ && (i == 0)/*1st portGrp*/)
        {
            /* Create Rules HA DB by reading the TCAM*/
            /* Created Once for 1 device & 1 portGrp - the same condition as when vTcamManger Init is called */

            rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
            if (rc != GT_OK)
            {
                return rc;
            }

            if (tcamMngDbPtr->haSupported 
                && (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
                && (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_PARALLEL_HA_E)
                && (managerHwWriteBlock == GT_FALSE))
            {
                if (tcamMngDbPtr->tcamSegCfg.hwDbWriteFuncPtr != NULL)
                {
                    /* wrong, needed just dev+group*/
                    rc = prvCpssDxChVirtualTcamHaHwDbClearAll(&(tcamMngDbPtr->tcamSegCfg));
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                if ((tcamMngDbPtr->haSupported != GT_FALSE) 
                    && (tcamMngDbPtr->tcamTtiSegCfg.hwDbWriteFuncPtr != NULL))
                {
                    /* wrong, needed just dev+group*/
                    rc = prvCpssDxChVirtualTcamHaHwDbClearAll(&(tcamMngDbPtr->tcamTtiSegCfg));
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
                if ((tcamMngDbPtr->haSupported != GT_FALSE) 
                    && (tcamMngDbPtr->pclTcam1SegCfg.hwDbWriteFuncPtr != NULL))
                {
                    /* wrong, needed just dev+group*/
                    rc = prvCpssDxChVirtualTcamHaHwDbClearAll(&(tcamMngDbPtr->pclTcam1SegCfg));
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }

            if (tcamMngDbPtr->haSupported && (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E ||
                managerHwWriteBlock == GT_TRUE))
            {
                CPSS_API_LOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                if (haVtcamTimeDebug)
                {
                    rc = cpssOsTimeRT(&start_sec, &start_nsec);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssOsTimeRT failed for start\n");
                    }
                }

                rc = prvCpssDxChVirtualTcamHaDbCreate(devNum, vTcamMngId, 0);
                if(rc != GT_OK)
                {
                    CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamHaDbCreate failed\n");
                }

                if (haVtcamDebug)
                {
                    cpssOsPrintf("Valid Rules DB populated from H/w(TCAM) - RuleData + Physical Index updated\n");
                }

                /* Update blockInfArr through Register read*/
                rc = prvCpssDxChVirtualTcamHaLookupIdRestore(devNum, vTcamMngId, tcamMngDbPtr->portGroupBmpArr[devNum],
                        tcamMngDbPtr->tcamSegCfg.blocksAmount, tcamMngDbPtr->tcamSegCfg.blockInfArr);
                CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "\nprvCpssDxChVirtualTcamHaLookupIdRestore failed.\n");
                }

                if (haVtcamTimeDebug)
                {
                    rc = cpssOsTimeRT(&end_sec, &end_nsec);
                    if(rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssOsTimeRT failed for end\n");
                    }

                    if(end_nsec < start_nsec)
                    {
                        end_nsec += 1000000000;
                        end_sec  -= 1;
                    }

                    diff_sec  = end_sec  - start_sec;
                    diff_nsec = end_nsec - start_nsec;

                    cpssOsPrintf("HA vtcam DB create time is [%d.%d] seconds\n" , diff_sec , diff_nsec);
                }
            }
        }

        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (tcamMngDbPtr->haSupported && tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E &&
            managerHwWriteBlock == GT_FALSE)
        {
            tcamFloorEntriesNum =
                CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS;

            rc = cpssDxChCfgTableNumEntriesGet(
                devNum, CPSS_DXCH_CFG_TABLE_TTI_TCAM_E, &tcamEntriesNum);

            tcamFloorsNum = (tcamEntriesNum / tcamFloorEntriesNum);

            /* Power on the whole TCAM temporarily to mark blocks Free */
            rc = cpssDxChTcamPortGroupActiveFloorsSet(devNum, portGroupsBmp, tcamFloorsNum);
            if (rc != GT_OK)
            {
                return rc;
            }

            CPSS_API_LOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            rc = prvCpssDxChVirtualTcamHaAllBlocksMarkFree(devNum, portGroupsBmp, vTcamMngId);
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamHaAllBlocksMarkFree failed\n");
            }

            /* Power down the whole TCAM after marking blocks Free */
            rc = cpssDxChTcamPortGroupActiveFloorsSet(devNum, portGroupsBmp, 0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if (noDevicesBeforAdding == GT_FALSE)
    {
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(
            representativeDevNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        isSip5Dev = PRV_CPSS_SIP_5_CHECK_MAC(representativeDevNum);
        CPSS_API_UNLOCK_NO_CHECKS_MAC(representativeDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        if (isSip5Dev)
        {
            /* Get the active floors for the existing device attached to vTCAM */
            rc = cpssDxChTcamPortGroupActiveFloorsGet(
                representativeDevNum, representativePortGroupsBmp, &floorIndex);
            if (rc != GT_OK)
            {
                return rc;
            }

            /* Set the number of active floors in all the device in devListArr */
            for (i = 0; (i < numOfPortGroupBmps); i++)
            {
                devNum        = devPortGroupsBmpArr[i].devNum;
                portGroupsBmp = devPortGroupsBmpArr[i].portGroupsBmp;
                rc = cpssDxChTcamPortGroupActiveFloorsSet(devNum, portGroupsBmp, floorIndex);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }

            /* Blocks to client groups/hits mapping                     */
            /* The index of the highest powered on floor retrieved upper*/
            tcamFloorsNum = floorIndex;
            for (floorIndex =  0; (floorIndex < tcamFloorsNum); floorIndex++)
            {
                rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupGet(
                    representativeDevNum, representativePortGroupsBmp,
                    floorIndex, floorInfoArr);
                if (rc != GT_OK)
                {
                    return rc;
                }
                for (i = 0; (i < numOfPortGroupBmps); i++)
                {
                    devNum        = devPortGroupsBmpArr[i].devNum;
                    portGroupsBmp = devPortGroupsBmpArr[i].portGroupsBmp;
                    rc = cpssDxChTcamPortGroupIndexRangeHitNumAndGroupSet(
                        devNum, portGroupsBmp, floorIndex, floorInfoArr);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }

        /* we hold devices before the call to this function , so no need
           to copy rules of vTcams from existing device */
        /* need to copy from representativeDevNum to all new device */
        rc = copyAllRulesFromSrcDevToDevBmp(
            vTcamMngId, representativeDevNum, representativePortGroupsBmp,
            devPortGroupsBmpArr, numOfPortGroupBmps);
        if (rc != GT_OK)
        {
            return rc;
        }

        for (i = 0; (i < numOfPortGroupBmps); i++)
        {
            devNum        = devPortGroupsBmpArr[i].devNum;
            portGroupsBmp = devPortGroupsBmpArr[i].portGroupsBmp;

            if ((tcamMngDbPtr->haSupported != GT_FALSE) 
                && (tcamMngDbPtr->tcamSegCfg.hwDbWriteFuncPtr != NULL))
            {
                /* wrong, needed just dev+group*/
                rc = prvCpssDxChVirtualTcamHaHwDbCopyAll(
                    &(tcamMngDbPtr->tcamSegCfg), devNum, portGroupsBmp);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            if ((tcamMngDbPtr->haSupported != GT_FALSE) 
                && (tcamMngDbPtr->tcamTtiSegCfg.hwDbWriteFuncPtr != NULL))
            {
                /* wrong, needed just dev+group*/
                rc = prvCpssDxChVirtualTcamHaHwDbCopyAll(
                    &(tcamMngDbPtr->tcamTtiSegCfg), devNum, portGroupsBmp);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            if ((tcamMngDbPtr->haSupported != GT_FALSE) 
                && (tcamMngDbPtr->pclTcam1SegCfg.hwDbWriteFuncPtr != NULL))
            {
                /* wrong, needed just dev+group*/
                rc = prvCpssDxChVirtualTcamHaHwDbCopyAll(
                    &(tcamMngDbPtr->pclTcam1SegCfg), devNum, portGroupsBmp);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChVirtualTcamManagerDevListAdd function
* @endinternal
*
* @brief   This function adds devices to an existing Virtual TCAM manager.
*         TCAM of one of old devices (if exist) copied to TCAMs the newly added devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] devListArr[]             - the array of device ids to add to the Virtual TCAM Manager.
* @param[in] numOfDevs               - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
*
* @note NONE
*
*/
static GT_STATUS internal_cpssDxChVirtualTcamManagerDevListAdd
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
)
{
    GT_U8               i;
    GT_STATUS           rc;
    CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC  devPortGroupsBmp;

    /* other parameters checked in called function */
    if(numOfDevs)
    {
        CPSS_NULL_PTR_CHECK_MAC(devListArr);
    }

    for (i = 0; (i < numOfDevs); i++)
    {
        devPortGroupsBmp.devNum = devListArr[i];
        devPortGroupsBmp.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        rc = prvCpssDxChVirtualTcamManagerDevPortGroupListAdd(
            vTcamMngId, &devPortGroupsBmp, 1 /*numOfPortGroupBmps*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamManagerDevListAdd function
* @endinternal
*
* @brief   This function adds devices to an existing Virtual TCAM manager.
*         TCAM of one of old devices (if exist) copied to TCAMs the newly added devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] devListArr[]             - the array of device ids to add to the Virtual TCAM Manager.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
*
* @note NONE
*
*/
GT_STATUS cpssDxChVirtualTcamManagerDevListAdd
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamManagerDevListAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, devListArr, numOfDevs));

    rc = internal_cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, devListArr, numOfDevs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, devListArr, numOfDevs));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamManagerDevPortGroupListRemove function
* @endinternal
*
* @brief  This function removes Port Group Bitmaps from an existing Virtual TCAM manager.
*         TCAMs of removed Port Group Bitmaps not updated.
*         Removing last device caused cleaning up all Tcam Manager configurations.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                       (APPLICABLE RANGES: 0..31)
* @param[in] portGroupBmpListArr[]    - the array of Port Group Bitmaps to remove from the
*                                       Virtual TCAM Manager.
* @param[in] numOfPortGroupBmps       - the number of Port Group Bitmaps in the array.
* @param[in] invalidateRulesInRemoved - GT_TRUE - invalidate rules in removed Port Groups,
*                                       GT_FALSE - leave as is.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_NOT_INITIALIZED       - if there are no devices in DB
*
* @note NONE
*
*/
/*static GT_STATUS internal_cpssDxChVirtualTcamManagerDevListRemove
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
)*/
static GT_STATUS prvCpssDxChVirtualTcamManagerDevPortGroupListRemove
(
    IN GT_U32                                          vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpListArr[], /*arrSizeVarName=numOfPortGroupBmps*/
    IN GT_U32                                          numOfPortGroupBmps,
    IN GT_BOOL                                         invalidateRulesInRemoved
)
{
    GT_STATUS           rc;
    GT_U8               i;
    GT_U8               devNum;
    GT_PORT_GROUPS_BMP  portGroupsBmp;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_PTR tcamMngDbPtr;

    if (vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(numOfPortGroupBmps >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(numOfPortGroupBmps)
    {
        CPSS_NULL_PTR_CHECK_MAC(portGroupBmpListArr);
    }

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    tcamMngDbPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if (tcamMngDbPtr == NULL)
    {
        /* can't find the vTcamMng DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* Check validity */
    for (i = 0; i < numOfPortGroupBmps; i++)
    {
        devNum        = portGroupBmpListArr[i].devNum;

        if (! PRV_CPSS_VIRTUAL_TCAM_MNG_IS_DEV_BMP_SET_MAC(
            tcamMngDbPtr->devsBitmap, devNum))
        {
            /* device not found in VTAM manager */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* check device and port group bitmap */
        /* prvCpssDxChVirtualTcamFullTcamPortGroupBmpGet not needed device DB locking */
        rc =  prvCpssDxChVirtualTcamFullTcamPortGroupBmpGet(
            devNum, &portGroupsBmp);
        if (rc != GT_OK)
        {
            /* wrong devNum */
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* Single TCAM devices */
        if (portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
        {
            /* for Single TCAM devices - bitmap ignored */
            continue;
        }

        if (portGroupBmpListArr[i].portGroupsBmp == 0)
        {
            /* empty port group bitmap */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (portGroupBmpListArr[i].portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
        {
            if (tcamMngDbPtr->portGroupBmpArr[devNum] != portGroupsBmp)
            {
                /* not all port groups of device in DB */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            if (portGroupBmpListArr[i].portGroupsBmp & (~ tcamMngDbPtr->portGroupBmpArr[devNum]))
            {
                /* removing not found port group */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    for (i = 0; (i < numOfPortGroupBmps); i++)
    {
        devNum = portGroupBmpListArr[i].devNum;

        /* Support only xCat3 or SIP5 devices */
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        if((!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) &&
            (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
            (GT_FALSE == PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
        }
        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /* removed port group bitmap validity already checked */
        if ((portGroupBmpListArr[i].portGroupsBmp == CPSS_PORT_GROUP_UNAWARE_MODE_CNS)
            || (portGroupBmpListArr[i].portGroupsBmp == tcamMngDbPtr->portGroupBmpArr[devNum]))
        {
            /* remove all port groups found in DB */
            tcamMngDbPtr->portGroupBmpArr[devNum] = 0;

            /* Remove device from bitmap */

            PRV_CPSS_VIRTUAL_TCAM_MNG_DEV_BMP_CLEAR_MAC(tcamMngDbPtr->devsBitmap, devNum);

            if(tcamMngDbPtr->numOfDevices == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: can't decrement numOfDevices with value 0");
            }

            /* don't use HW Shadow for read - ingnore return code - the feature can be not supported */
            prvCpssDxChTableReadFromShadowEnableSet(
                devNum, CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E, GT_FALSE);
            prvCpssDxChTableReadFromShadowEnableSet(
                devNum, CPSS_DXCH_SIP5_TABLE_TCAM_E, GT_FALSE);

            /* update devices counter */
            tcamMngDbPtr->numOfDevices--;
        }
        else
        {
            /* remove not all port groups found in DB */
            tcamMngDbPtr->portGroupBmpArr[devNum] &= (~ portGroupBmpListArr[i].portGroupsBmp);
        }
    }

    if (invalidateRulesInRemoved != GT_FALSE)
    {
        /* invalidate rules in removed port groups */
        rc = invalidateAllRulesInDevsPortGroupsBmp(
            vTcamMngId, portGroupBmpListArr, numOfPortGroupBmps);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (tcamMngDbPtr->haSupported)
        {
            for (i = 0; (i < numOfPortGroupBmps); i++)
            {
                devNum = portGroupBmpListArr[i].devNum;
                rc = prvCpssDxChVirtualTcamHaVtcamMngClear(
                    devNum, portGroupBmpListArr[i].portGroupsBmp, vTcamMngId);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamHaVtcamMngClear failed");
                }
            }
        }
    }

    if (tcamMngDbPtr->numOfDevices == 0)
    {
        /* Delete TCAM manager */
        rc = prvCpssDxChVirtualTcamManagerDelete(vTcamMngId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChVirtualTcamManagerDevListRemove function
* @endinternal
*
* @brief   This function removes devices from an existing Virtual TCAM manager.
*         TCAMs of removed devices not updated.
*         Removing last device caused cleaning up all Tcam Manager configurations.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] devListArr[]             - the array of device ids to remove from the
*                                      Virtual TCAM Manager.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_NOT_INITIALIZED       - if there are no devices in DB
*
* @note NONE
*
*/
static GT_STATUS internal_cpssDxChVirtualTcamManagerDevListRemove
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
)
{
    GT_U8               i;
    GT_STATUS           rc;
    CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC  devPortGroupsBmp;

    /* other parameters checked in called function */
    if (numOfDevs)
    {
        CPSS_NULL_PTR_CHECK_MAC(devListArr);
    }

    for (i = 0; (i < numOfDevs); i++)
    {
        devPortGroupsBmp.devNum = devListArr[i];
        devPortGroupsBmp.portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        rc = prvCpssDxChVirtualTcamManagerDevPortGroupListRemove(
            vTcamMngId, &devPortGroupsBmp, 1 /*numOfPortGroupBmps*/, GT_FALSE /*invalidateRulesInRemoved*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamManagerDevListRemove function
* @endinternal
*
* @brief   This function removes devices from an existing Virtual TCAM manager.
*         TCAMs of removed devices not updated.
*         Removing last device caused cleaning up all Tcam Manager configurations.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] devListArr[]             - the array of device ids to remove from the
*                                      Virtual TCAM Manager.
* @param[in] numOfDevs                - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_NOT_INITIALIZED       - if there are no devices in DB
*
* @note NONE
*
*/
GT_STATUS cpssDxChVirtualTcamManagerDevListRemove
(
    IN GT_U32   vTcamMngId,
    IN GT_U8    devListArr[],
    IN GT_U32   numOfDevs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamManagerDevListRemove);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, devListArr, numOfDevs));

    rc = internal_cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, numOfDevs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, devListArr, numOfDevs));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamManagerPortGroupListAdd function
* @endinternal
*
* @brief  This function adds Port Group Bitmaps to an existing Virtual TCAM manager.
*         Rules of one of old Port Group Bitmaps (if exist) copied to the newly added Port Group Bitmaps.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] portGroupBmpListArr[]    - the array of Port Group Bitmaps to add to the
*                                       Virtual TCAM Manager.
* @param[in] numOfPortGroupBmps       - the number of Port Group Bitmaps in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
*
* @note NONE
*
*/
static GT_STATUS internal_cpssDxChVirtualTcamManagerPortGroupListAdd
(
    IN GT_U32                                          vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpListArr[], /*arrSizeVarName=numOfPortGroupBmps*/
    IN GT_U32                                          numOfPortGroupBmps
)
{
    return prvCpssDxChVirtualTcamManagerDevPortGroupListAdd(
        vTcamMngId, portGroupBmpListArr, numOfPortGroupBmps);
}

/**
* @internal cpssDxChVirtualTcamManagerPortGroupListAdd function
* @endinternal
*
* @brief  This function adds Port Group Bitmaps to an existing Virtual TCAM manager.
*         Rules of one of old Port Group Bitmaps (if exist) copied to the newly added Port Group Bitmaps.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] portGroupBmpListArr[]    - the array of Port Group Bitmaps to add to the
*                                       Virtual TCAM Manager.
* @param[in] numOfPortGroupBmps       - the number of Port Group Bitmaps in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
*
* @note NONE
*
*/
GT_STATUS cpssDxChVirtualTcamManagerPortGroupListAdd
(
    IN GT_U32                                          vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpListArr[], /*arrSizeVarName=numOfPortGroupBmps*/
    IN GT_U32                                          numOfPortGroupBmps
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamManagerPortGroupListAdd);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, portGroupBmpListArr, numOfPortGroupBmps));

    rc = internal_cpssDxChVirtualTcamManagerPortGroupListAdd(
        vTcamMngId, portGroupBmpListArr, numOfPortGroupBmps);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, portGroupBmpListArr, numOfPortGroupBmps));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamManagerPortGroupListRemove function
* @endinternal
*
* @brief  This function removes Port Group Bitmaps from an existing Virtual TCAM manager.
*         TCAMs of removed Port Group Bitmaps not updated.
*         Removing last device caused cleaning up all Tcam Manager configurations.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                       (APPLICABLE RANGES: 0..31)
* @param[in] portGroupBmpListArr[]    - the array of Port Group Bitmaps to remove from the
*                                       Virtual TCAM Manager.
* @param[in] numOfPortGroupBmps       - the number of Port Group Bitmaps in the array.
* @param[in] invalidateRulesInRemoved - GT_TRUE - invalidate rules in removed Port Groups,
*                                       GT_FALSE - leave removed Port Groups as is,
*                                       mandatory when CPU has no access to removed device.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_NOT_INITIALIZED       - if there are no devices in DB
*
* @note NONE
*
*/
static GT_STATUS internal_cpssDxChVirtualTcamManagerPortGroupListRemove
(
    IN GT_U32                                          vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpListArr[], /*arrSizeVarName=numOfPortGroupBmps*/
    IN GT_U32                                          numOfPortGroupBmps,
    IN GT_BOOL                                         invalidateRulesInRemoved
)
{
    return prvCpssDxChVirtualTcamManagerDevPortGroupListRemove(
        vTcamMngId, portGroupBmpListArr,  numOfPortGroupBmps, invalidateRulesInRemoved);
}

/**
* @internal cpssDxChVirtualTcamManagerPortGroupListRemove function
* @endinternal
*
* @brief  This function removes Port Group Bitmaps from an existing Virtual TCAM manager.
*         TCAMs of removed Port Group Bitmaps not updated.
*         Removing last device caused cleaning up all Tcam Manager configurations.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                       (APPLICABLE RANGES: 0..31)
* @param[in] portGroupBmpListArr[]    - the array of Port Group Bitmaps to remove from the
*                                       Virtual TCAM Manager.
* @param[in] numOfPortGroupBmps       - the number of Port Group Bitmaps in the array.
* @param[in] invalidateRulesInRemoved - GT_TRUE - invalidate rules in removed Port Groups,
*                                       GT_FALSE - leave removed Port Groups as is,
*                                       mandatory when CPU has no access to removed device.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - if the Virtual TCAM Manager id is not found.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if devListArr is NULL pointer.
* @retval GT_NOT_INITIALIZED       - if there are no devices in DB
*
* @note NONE
*
*/
GT_STATUS cpssDxChVirtualTcamManagerPortGroupListRemove
(
    IN GT_U32                                          vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpListArr[], /*arrSizeVarName=numOfPortGroupBmps*/
    IN GT_U32                                          numOfPortGroupBmps,
    IN GT_BOOL                                         invalidateRulesInRemoved
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamManagerPortGroupListRemove);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, vTcamMngId, portGroupBmpListArr, numOfPortGroupBmps, invalidateRulesInRemoved));

    rc = internal_cpssDxChVirtualTcamManagerPortGroupListRemove(
        vTcamMngId, portGroupBmpListArr, numOfPortGroupBmps, invalidateRulesInRemoved);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, vTcamMngId, portGroupBmpListArr, numOfPortGroupBmps, invalidateRulesInRemoved));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamManagerPortGroupListGet function
* @endinternal
*
* @brief The function returns current Virtual TCAM manager registered devices and port group bitmaps.
*
* @param[in] vTcamMngId                 - the VTCAM Manager id.
*                                       (APPLICABLE RANGES : 0..31)
* @param[inout] numOfPortGroupsPtr      - (pointer to) the number of devices and port group bitmaps in
*                                       the array portGroupBmpListArr as 'input parameter',
*                                       and actual number of devices and port group bitmaps as 'output parameter'.
* @param[out] portGroupBmpListArr[]     - array of registered devices and port group bitmaps (no more than list length).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_FOUND             - Virtual TCAM Match Manager is not found
* @retval GT_BAD_PTR               - if portGroupBmpListArr or numOfPortGroupsPtr is NULL pointer.
*
*/
static GT_STATUS internal_cpssDxChVirtualTcamManagerPortGroupListGet
(
    IN GT_U32       vTcamMngId,
    INOUT GT_U32   *numOfPortGroupsPtr,
    OUT CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpListArr[]/*arrSizeVarName=*numOfPortGroupsPtr*/
)
{
    GT_U8   devNum;
    GT_U32  ii, jj; /*numOfDevs iterators*/
    GT_U32  numOfDevs; /*number of devices attached to the VTCAM*/
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_PTR  tcamMngDbPtr; /*pointer to specific Virtual TCAM manager DB*/

    if (vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    tcamMngDbPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if (tcamMngDbPtr == NULL)
    {
        /* can't find the vTcamMng DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }


    CPSS_NULL_PTR_CHECK_MAC(numOfPortGroupsPtr);
    CPSS_NULL_PTR_CHECK_MAC(portGroupBmpListArr);

    numOfDevs = *numOfPortGroupsPtr; /*number of devices attached to the VTCAM*/

    if(numOfDevs == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfDevs is ZERO ... no meaning calling the function");
    }

    jj = 0;

    /* the input number of devices should not exceed the actual number in DB */
    if (numOfDevs > tcamMngDbPtr->numOfDevices)
    {
        numOfDevs = tcamMngDbPtr->numOfDevices;
    }

    /* retrieve devices from VTCAM manager DB */
    for(ii = 0; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
        if(PRV_CPSS_VIRTUAL_TCAM_MNG_IS_DEV_BMP_SET_MAC(tcamMngDbPtr->devsBitmap, ii))
        {
            devNum = (GT_U8)ii;
            portGroupBmpListArr[jj].devNum = devNum;
            portGroupBmpListArr[jj++].portGroupsBmp =  tcamMngDbPtr->portGroupBmpArr[devNum];

            /*maximum number of devices exceeded*/
            if (jj == numOfDevs)
            {
                break;
            }
        }
    }

    /* actual number of retrieved devices */
    *numOfPortGroupsPtr = jj;

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamManagerPortGroupListGet function
* @endinternal
*
* @brief The function returns current Virtual TCAM manager registered devices and port group bitmaps.
*
* @param[in] vTcamMngId                 - the VTCAM Manager id.
*                                       (APPLICABLE RANGES : 0..31)
* @param[inout] numOfPortGroupsPtr      - (pointer to) the number of devices and port group bitmaps in
*                                       the array portGroupBmpListArr as 'input parameter',
*                                       and actual number of devices and port group bitmaps as 'output parameter'.
* @param[out] portGroupBmpListArr[]     - array of registered devices and port group bitmaps (no more than list length).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_NOT_FOUND             - Virtual TCAM Match Manager is not found
* @retval GT_BAD_PTR               - if portGroupBmpListArr or numOfPortGroupsPtr is NULL pointer.
*
*/
GT_STATUS cpssDxChVirtualTcamManagerPortGroupListGet
(
    IN GT_U32       vTcamMngId,
    INOUT GT_U32   *numOfPortGroupsPtr,
    OUT CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpListArr[]/*arrSizeVarName=*numOfPortGroupsPtr*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamManagerPortGroupListGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, vTcamMngId,numOfPortGroupsPtr, portGroupBmpListArr));

    rc = internal_cpssDxChVirtualTcamManagerPortGroupListGet(vTcamMngId, numOfPortGroupsPtr, portGroupBmpListArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, numOfPortGroupsPtr, portGroupBmpListArr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/*******************************************************************************/
/*******************************************************************************/
/****************** start DB operation *****************************************/
/*******************************************************************************/
/*******************************************************************************/

/*******************************************************************************
* prvCpssDxChVirtualTcamDbVTcamMngGet
*
* DESCRIPTION:
*       Get DB info about a vTcam manager.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       vTcamMngId        - Virtual TCAM Manager Id
*                           (APPLICABLE RANGES: 0..31)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the DB of the vTcam.
*       NULL on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC* prvCpssDxChVirtualTcamDbVTcamMngGet
(
    IN  GT_U32                                      vTcamMngId
)
{
    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        return NULL;
    }

    return VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
}

/*******************************************************************************
* prvCpssDxChVirtualTcamDbVTcamGet
*
* DESCRIPTION:
*       Get DB info about a vTcam in vTcam manager.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       vTcamMngId        - Virtual TCAM Manager Id
*                           (APPLICABLE RANGES: 0..31)
*       vTcamId           - unique Id of  Virtual TCAM
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       pointer to the DB of the vTcam.
*       NULL on error
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* prvCpssDxChVirtualTcamDbVTcamGet
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        return NULL;
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        return NULL;
    }

    if (vTcamMngDBPtr->vTcamCfgPtrArr == NULL)
    {
        return NULL;
    }

    if(vTcamId >= vTcamMngDBPtr->vTcamCfgPtrArrSize)
    {
        /* vTcamId out of range */
        return NULL;
    }

    return vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId];/* may be NULL ! */
}

/**
* @internal deviceIteratorGetNext function
* @endinternal
*
* @brief   The function returns the first/next devNum that is bound to the tcam manager.
*
* @param[in] vTcamMngDBPtr            - (pointer to)Virtual TCAM Manager structure
* @param[in] isFirst                  - is First or next
* @param[in,out] devNumPtr                - (pointer to) hold 'current' devNum . (relevant for 'next')
* @param[in,out] devNumPtr                - (pointer to) hold 'next' devNum .
*
* @retval GT_OK                    - iterator initialization OK
* @retval GT_NO_MORE               - no devices attached to the vTcamMngId
* @retval GT_FAIL                  - on error
*/
static GT_STATUS deviceIteratorGetNext
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr,
    IN  GT_BOOL isFirst,
    INOUT GT_U8   *devNumPtr
)
{
    GT_U32  ii;/* must not be GT_U8 */

    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(isFirst == GT_TRUE)
    {
        ii = 0;
        *devNumPtr = 0;
    }
    else
    {
        ii = *devNumPtr;

        ii++;
    }

    for(/* continue */; ii < PRV_CPSS_MAX_PP_DEVICES_CNS; ii++)
    {
        if(PRV_CPSS_VIRTUAL_TCAM_MNG_IS_DEV_BMP_SET_MAC(vTcamMngDBPtr->devsBitmap, ii))
        {
            *devNumPtr = (GT_U8)ii;
            return GT_OK;
        }
    }

    return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
}

/**
* @internal prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst function
* @endinternal
*
* @brief   The function returns the first devNum that is bound to the tcam manager.
*         it must be called before any iteration on the device list of the vTcam.
* @param[in] vTcamMngDBPtr            - (pointer to)Virtual TCAM Manager structure
*
* @param[out] devNumPtr                - (pointer to) hold 'first' devNum .
*
* @retval GT_OK                    - iterator initialization OK
* @retval GT_NO_MORE               - no devices attached to the vTcamMngId
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr,
    OUT GT_U8                               *devNumPtr
)
{
    return deviceIteratorGetNext(vTcamMngDBPtr,GT_TRUE,devNumPtr);
}

/**
* @internal prvCpssDxChVirtualTcamDbDeviceIteratorGetNext function
* @endinternal
*
* @brief   The function returns the next devNum that is bound to the tcam manager.
*
* @param[in] vTcamMngDBPtr            - (pointer to)Virtual TCAM Manager structure
* @param[in,out] devNumPtr                - (pointer to) hold 'current' devNum .
* @param[in,out] devNumPtr                - (pointer to) hold 'next' devNum .
*
* @retval 0                        - no more devices. ('current' device is the last one)
* @retval 1                        - devNumPtr hold 'next' devNum
*/
GT_U32  prvCpssDxChVirtualTcamDbDeviceIteratorGetNext
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr,
    INOUT GT_U8                             *devNumPtr
)
{
    GT_STATUS   rc;

    rc = deviceIteratorGetNext(vTcamMngDBPtr,GT_FALSE,devNumPtr);
    if(rc == GT_OK)
    {
        return 1; /*devPtr hold 'next' devNum */
    }

    return 0;/*no more devices. ('current' device is the last one) */
}

/**
* @internal prvCpssDxChVirtualTcamFullTcamPortGroupBmpGet function
* @endinternal
*
* @brief   The function gets bitmap of all port groups representing TCAMs.
*
* @param[in]  devNum            - device number
* @param[out] portGroupsBmpPtr  - (pointer to) bitmap of all port groups representing TCAMs.
*
* @retval GT_BAD_PARAM          - wrong device number.
* @retval GT_OK                 - correct result
*/
GT_STATUS  prvCpssDxChVirtualTcamFullTcamPortGroupBmpGet
(
    IN  GT_U8                             devNum,
    OUT GT_PORT_GROUPS_BMP                *portGroupsBmpPtr
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    if (PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.isMultiPortGroupDevice == GT_FALSE)
    {
        *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        return GT_OK;
    }
    if (! PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* Lion2 */
        *portGroupsBmpPtr = PRV_CPSS_PP_MAC(devNum)->portGroupsInfo.activePortGroupsBmp;
        CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        return GT_OK;
    }

    *portGroupsBmpPtr = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    rc = prvCpssMultiPortGroupsBmpCheckSpecificUnit(
        devNum, portGroupsBmpPtr, PRV_CPSS_DXCH_UNIT_TCAM_E);

    CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntryGet function
* @endinternal
*
* @brief   Get rule Id entry by rule Id.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] ruleId                   -  to find.
*
* @param[out] entryPtr                 - (pointer to) find rule entry.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - the ruleId was not found.
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntryGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC       *tcamCfgPtr,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                     ruleId,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC   *entryPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC    ruleIdEntry; /* key for search */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC   *dbEntryPtr;  /* pointer to entry in DB */

    ruleIdEntry.ruleId = ruleId;
    /* find rule ID entry in AVL tree */
    dbEntryPtr = prvCpssAvlSearch(tcamCfgPtr->ruleIdIdTree, &ruleIdEntry);
    if (dbEntryPtr == NULL)
    {
        return /* do not register as error , it is valid state !!! */ GT_NOT_FOUND;
    }
    *entryPtr =  *dbEntryPtr;
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntryRemove function
* @endinternal
*
* @brief   Remove rule Id entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] ruleId                   -  to remove.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on ruleId or/and logicalIndex already used
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntryRemove
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC       *tcamCfgPtr,
    IN   GT_U32                                             ruleId
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC ruleIdEntry, * ruleIdEntryPtr;


    ruleIdEntry.ruleId = ruleId;
    /* find rule ID entry in AVL tree */
    ruleIdEntryPtr = prvCpssAvlSearch(tcamCfgPtr->ruleIdIdTree, &ruleIdEntry);
    if (ruleIdEntryPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    /* remove rule ID entry from the tree */
    prvCpssAvlItemRemove(tcamCfgPtr->ruleIdIdTree, ruleIdEntryPtr);
    /* remove rule ID entry from the index tree */
    prvCpssAvlItemRemove(tcamCfgPtr->ruleIdIndexTree, ruleIdEntryPtr);

    /* free a buffer back to its pool.*/
    rc = cpssBmPoolBufFree(pVTcamMngPtr->ruleIdPool, ruleIdEntryPtr);

    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntryAdd function
* @endinternal
*
* @brief   Add rule Id entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] entryPtr                 - (pointer to) rule Id entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on ruleId or/and logicalIndex already used
* @retval GT_FAIL                  - on error
*
* @note treePathArr        - (pointer to) tree path array.
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntryAdd
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC               *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC      *tcamCfgPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC  *entryPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC * ruleIdEntryPtr;


    /* find rule ID entry in AVL tree */
    ruleIdEntryPtr = prvCpssAvlSearch(tcamCfgPtr->ruleIdIdTree, entryPtr);
    if (ruleIdEntryPtr)
    {
        /* rule ID entry already exist */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* get free buffer from pool.*/
    ruleIdEntryPtr = cpssBmPoolBufGet(pVTcamMngPtr->ruleIdPool);
    if (ruleIdEntryPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* copy rule ID entry to allocated buffer */
    cpssOsMemCpy(ruleIdEntryPtr, entryPtr, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC));

    /* add rule ID entry to the tree */
    rc = prvCpssAvlItemInsert(tcamCfgPtr->ruleIdIdTree, ruleIdEntryPtr);
    if (rc != GT_OK)
    {
        cpssBmPoolBufFree(pVTcamMngPtr->ruleIdPool, ruleIdEntryPtr);
        return rc;
    }
    /* add rule ID entry to the index tree */
    rc = prvCpssAvlItemInsert(tcamCfgPtr->ruleIdIndexTree, ruleIdEntryPtr);
    if (rc != GT_OK)
    {
        prvCpssAvlItemRemove(tcamCfgPtr->ruleIdIdTree, ruleIdEntryPtr);
        cpssBmPoolBufFree(pVTcamMngPtr->ruleIdPool, ruleIdEntryPtr);
        return rc;
    }

    return GT_OK;
}


/**
* @internal numHwMovesForHoleCalc function
* @endinternal
*
* @brief   calc number of HW moves needed to create hole at the start/end of the range
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] fromEndOrStart           - the type of direction
*                                      GT_TRUE  - hole from end   of range
*                                      GT_FALSE - hole from start of range
* @param[in] holeSize                 - the size of the needed hole
*
* @param[out] numMovesPtr              - (pointer to) number of moves needed
*                                      NOTE: when (numMovesPtr) == 0 meaning that the start/end
*                                      of the range is already empty.
*                                      so no need to move any index in the HW.
* @param[out] targetLogicalIndexPtr    - (pointer to) the logical index of the first
*                                      non-used , that should be used to move used indexes into it.
*                                      NOTE: relevant only when (numMovesPtr) != 0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad hole size
*/
static GT_STATUS numHwMovesForHoleCalc
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC               *vtcamInfoPtr,
    IN   GT_BOOL                                                    fromEndOrStart,
    IN   GT_U32                                                     holeSize,
    OUT  GT_U32                                                    *numMovesPtr,
    OUT  GT_U32                                                    *targetLogicalIndexPtr
)
{
    GT_U32  ii,iiStart,iiLast;
    GT_U32  *arrPtr = vtcamInfoPtr->usedRulesBitmapArr;
    GT_U32  value;
    GT_U32  holesCounter = 0;/* number of unused indexes in the range that expected to get the movements */
    GT_U32  numUsedInFinalHole = 0;/* number of used indexes with size <holeSize> from start/end of the range*/
    GT_U32  tmpLogicalIndex;

    *numMovesPtr = 0;
    *targetLogicalIndexPtr = 0;

    if(fromEndOrStart == GT_TRUE)
    {
        /* start from the end : direction of search is 'down' */
        iiStart = vtcamInfoPtr->rulesAmount - 1;
        iiLast  = iiStart - holeSize;

        /* calc number of used indexes with size <holeSize> from end of the range */
        for(ii = iiStart ; ii > iiLast ; ii--)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, ii,1,value);
            if(value)
            {
                numUsedInFinalHole++;
            }

            if(ii == 0)
            {
                break;
            }
        }

        if(numUsedInFinalHole == 0)
        {
            /* the end of the table is empty !!! no need to do any moves */
            *numMovesPtr = 0;
            *targetLogicalIndexPtr = 0;/* should not be used by the caller on this case */
            return GT_OK;
        }

        tmpLogicalIndex = ii;
        /* logicalIndexOfFirstHoleForTheMove:
           calc from current ii , the range that hold unused indexes in the
           amount of <numUsedInFinalHole>  */
        for(/*continue*/ ; /*no criteria*/ ; ii--)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, ii,1,value);
            if(value == 0)
            {
                holesCounter++;
            }

            if(holesCounter == numUsedInFinalHole)
            {
                /* this is the number of HW movements that needed , to create
                   hole at the end of the range */
                *numMovesPtr = tmpLogicalIndex - ii + 1;
                *targetLogicalIndexPtr = ii;
                return GT_OK;
            }

            if(ii == 0)
            {
                break;
            }
        }
    }
    else
    {
        /* start from the start : direction of search is 'up' */
        iiStart = 0;
        iiLast  = holeSize;

        /* calc number of used indexes with size <holeSize> from start of the range */
        for(ii = iiStart ; ii < iiLast ; ii++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, ii,1,value);
            if(value)
            {
                numUsedInFinalHole++;
            }
        }

        if(numUsedInFinalHole == 0)
        {
            /* the start of the table is empty !!! no need to do any moves */
            *numMovesPtr = 0;
            *targetLogicalIndexPtr = 0;/* should not be used by the caller on this case */
            return GT_OK;
        }

        tmpLogicalIndex = ii;
        /* logicalIndexOfFirstHoleForTheMove:
           calc from current ii , the range that hold unused indexes in the
           amount of <numUsedInFinalHole>  */
        for(/*continue*/ ; ii < vtcamInfoPtr->rulesAmount ; ii++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, ii,1,value);
            if(value == 0)
            {
                holesCounter++;
            }

            if(holesCounter == numUsedInFinalHole)
            {
                /* this is the number of HW movements that needed , to create
                   hole at the end of the range */
                *numMovesPtr = ii - tmpLogicalIndex + 1;
                *targetLogicalIndexPtr = ii;
                return GT_OK;
            }
        }
    }
    /* not found such hole size */
    /* but the caller made sure that we have enough holes !? */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
}

/**
* @internal findNearestLogicalIndexUsed function
* @endinternal
*
* @brief   find nearest to specific logical index that is used logical index
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] inLogicalIndex           - input logical index
* @param[in] lessOrMore               - the type of search
*                                      GT_TRUE  - find index valid that is less than current
*                                      GT_FALSE - find index valid that is more than current
* @param[in] lastLogicalIndexToCheck  - last logical index to check
*
* @param[out] outLogicalIndexPtr       - (pointer to) output logical index
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - no such used index
*/
static GT_STATUS findNearestLogicalIndexUsed
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC               *vtcamInfoPtr,
    IN   GT_U32                                                     inLogicalIndex,
    IN   GT_BOOL                                                    lessOrMore,
    IN   GT_U32                                                     lastLogicalIndexToCheck,
    OUT  GT_U32                                                    *outLogicalIndexPtr
)
{
    GT_U32  ii;
    GT_U32  *arrPtr = vtcamInfoPtr->usedRulesBitmapArr;
    GT_U32  value;

    if(lessOrMore == GT_TRUE)
    {
        /* direction of search is 'down' */
        if(inLogicalIndex == 0)
        {
            return /* do not register as error , it is valid state !!! */ GT_NOT_FOUND;
        }
        /* look for nearest used from specific index */
        for(ii = inLogicalIndex - 1 ; /*no criteria*/ ; ii--)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, ii,1,value);
            if(value)
            {
                *outLogicalIndexPtr = ii;
                /* found used index */
                return GT_OK;
            }

            if(ii == 0)
            {
                break;
            }

            if(ii == lastLogicalIndexToCheck)
            {
                break;
            }
        }
    }
    else
    {
        /* direction of search is 'up' */
        /* look for nearest used from specific index */
        for(ii = inLogicalIndex+1 ; ii < vtcamInfoPtr->rulesAmount ; ii++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(arrPtr,ii,1,value);
            if(value)
            {
                *outLogicalIndexPtr = ii;
                /* found used index */
                return GT_OK;
            }

            if(ii == lastLogicalIndexToCheck)
            {
                break;
            }
        }
    }

    return /* do not register as error , it is valid state !!! */ GT_NOT_FOUND;
}

/**
* @internal findNearestLogicalIndexHole function
* @endinternal
*
* @brief   find nearest to specific logical index an unused logical index
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] inLogicalIndex           - input logical index
*
* @param[out] outLogicalIndexPtr       - (pointer to) output logical index
*
* @retval GT_OK                    - on success
* @retval GT_FULL                  - on table full (no free logical index)
*/
static GT_STATUS findNearestLogicalIndexHole
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC               *vtcamInfoPtr,
    IN   GT_U32                                                     inLogicalIndex,
    OUT  GT_U32                                                    *outLogicalIndexPtr
)
{
    GT_U32  ii;
    GT_BOOL upWithMoreEntriesOrDown;
    GT_U32  maxSharedOffset;
    GT_U32  maxIterator;
    GT_U32  *arrPtr = vtcamInfoPtr->usedRulesBitmapArr;
    GT_U32  value;

    if (vtcamInfoPtr->usedRulesAmount >= vtcamInfoPtr->rulesAmount)
    {
        if ((vtcamInfoPtr->autoResizeDisable) ||
            (vtcamInfoPtr->tcamInfo.autoResize == GT_FALSE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
        }
        else
        {
            return /* do not register as error */ GT_FULL;
        }
    }

    if(inLogicalIndex >= (vtcamInfoPtr->rulesAmount / 2))
    {
        /* we are in the upper half of the array */
        /* meaning that we potentially have more indexes to go down than to go up */
        maxSharedOffset = vtcamInfoPtr->rulesAmount - inLogicalIndex;/* the end is closest */
        upWithMoreEntriesOrDown = GT_FALSE;
        maxIterator = inLogicalIndex + 1;

    }
    else
    {
        /* we are in the lower half of the array */
        /* meaning that we potentially have more indexes to go up than to go down */
        maxSharedOffset = inLogicalIndex + 1; /* the start is closest */
        upWithMoreEntriesOrDown = GT_TRUE;
        maxIterator = vtcamInfoPtr->rulesAmount - inLogicalIndex;
    }

    CPSS_LOG_INFO_PARAM_MAC(maxSharedOffset);
    CPSS_LOG_INFO_PARAM_MAC(upWithMoreEntriesOrDown);
    CPSS_LOG_INFO_PARAM_MAC(maxIterator);

    /* look for nearest hole from specific index */
    for(ii = 0 ; ii < maxSharedOffset ; ii++)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(arrPtr,inLogicalIndex + ii,1,value);
        if(value == 0)
        {
            *outLogicalIndexPtr = inLogicalIndex + ii;
            /* found free index */
            return GT_OK;
        }


        U32_GET_FIELD_IN_ENTRY_MAC(arrPtr,inLogicalIndex - ii,1,value);
        if(value == 0)
        {
            *outLogicalIndexPtr = inLogicalIndex - ii;
            /* found free index */
            return GT_OK;
        }
    }

    for(/* continue */; ii < maxIterator ; ii++)
    {
        if(upWithMoreEntriesOrDown == GT_TRUE)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(arrPtr,inLogicalIndex + ii,1,value);
            if(value == 0)
            {
                *outLogicalIndexPtr = inLogicalIndex + ii;
                /* found free index */
                return GT_OK;
            }
        }
        else
        {
            U32_GET_FIELD_IN_ENTRY_MAC(arrPtr,inLogicalIndex - ii,1,value);
            if(value == 0)
            {
                *outLogicalIndexPtr = inLogicalIndex - ii;
                /* found free index */
                return GT_OK;
            }
        }
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
}

/**
* @internal nearestLogicalIndexEnvironmentGet function
* @endinternal
*
* @brief   find nearest contigous rules ranges and holes
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] inLogicalIndex           - input logical index
* @param[in] logicalIndexInLeft       - GT_TRUE : input logical index belongs to the left part,
*                                       GT_FALSE: the right part
*
* @param[out] rightRulesSizePtr       - (pointer to) size of range of contigous rules from the right
* @param[out] rightHoleSizePtr        - (pointer to) size of range of the hole after contigous rules from the right
* @param[out] leftRulesSizePtr        - (pointer to) size of range of contigous rules from the left
* @param[out] leftHoleSizePtr         - (pointer to) size of range of the hole after contigous rules from the left
*
* @retval GT_OK                    - on success
* @retval GT_FULL                  - on table full (no free logical index)
*/
static GT_STATUS nearestLogicalIndexEnvironmentGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC               *vtcamInfoPtr,
    IN   GT_U32                                                     inLogicalIndex,
    IN   GT_BOOL                                                    logicalIndexInLeft,
    OUT  GT_U32                                                    *rightRulesSizePtr,
    OUT  GT_U32                                                    *rightHoleSizePtr,
    OUT  GT_U32                                                    *leftRulesSizePtr,
    OUT  GT_U32                                                    *leftHoleSizePtr
)
{
    GT_U32   rightRulesSize;    /* amount of rules from right to the neares hole */
    GT_U32   rightHoleSize;     /* size of the nearest hole from right */
    GT_U32   leftRulesSize;     /* amount of rules from left to the neares hole */
    GT_U32   leftHoleSize;      /* size of the nearest hole from left */
    GT_U32   *arrPtr;           /* work poiner */
    GT_U32   maxLogIndex;       /* maximal logical index */
    GT_U32   maxWordIndex;      /* maximal word index in logical rule indexex bitmap */
    GT_U32   logIndex;          /* rule logical index */
    GT_U32   wordIndex;         /* word Index */
    GT_U32   bitIndex;          /* bit Index */
    GT_U32   word;              /* work variable */

    *rightRulesSizePtr   =  0;
    *rightHoleSizePtr    =  0;
    *leftRulesSizePtr    =  0;
    *leftHoleSizePtr     =  0;

    if (vtcamInfoPtr->usedRulesAmount >= vtcamInfoPtr->rulesAmount)
    {
        return /* do not register as error */ GT_FULL;
    }
    if (inLogicalIndex >= vtcamInfoPtr->rulesAmount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    arrPtr = vtcamInfoPtr->usedRulesBitmapArr;
    maxLogIndex = vtcamInfoPtr->rulesAmount;
    maxWordIndex = maxLogIndex / 32;

    word      = arrPtr[inLogicalIndex / 32];
    if (((word >> (inLogicalIndex % 32)) & 1) == 0)
    {
        /* no move needed, the pointed place free */
        *rightRulesSizePtr   =  0;
        *leftRulesSizePtr    =  0;
        *rightHoleSizePtr    =  1;
        *leftHoleSizePtr     =  1;
        return GT_OK;
    }

    /* searcing the nearest hole from the right */
    rightRulesSize = (logicalIndexInLeft == GT_FALSE) ? 1 : 0;
    rightHoleSize  = 0;
    logIndex = (inLogicalIndex + 1);
    /* count rules (ones in bitmap) */
    while (logIndex < maxLogIndex)
    {
        wordIndex = logIndex / 32;
        word      = arrPtr[wordIndex];
        if (((logIndex % 32) == 0)
            && (word == 0xFFFFFFFF)
            && (wordIndex < maxWordIndex))
        {
            logIndex += 32;
            rightRulesSize += 32;
            continue;
        }
        bitIndex  = (logIndex % 32);
        if (((word >> bitIndex) & 1) == 1)
        {
            logIndex += 1;
            rightRulesSize += 1;
            continue;
        }
        /* first hole */
        break;
    }
    /* count holes (zeros in bitmap) */
    while (logIndex < maxLogIndex)
    {
        wordIndex = logIndex / 32;
        word      = arrPtr[wordIndex];
        if (((logIndex % 32) == 0)
            && (word == 0)
            && (wordIndex < maxWordIndex))
        {
            logIndex += 32;
            rightHoleSize += 32;
            continue;
        }
        bitIndex  = (logIndex % 32);
        if (((word >> bitIndex) & 1) == 0)
        {
            logIndex += 1;
            rightHoleSize += 1;
            continue;
        }
        /* first rule */
        break;
    }

    /* searcing the nearest hole from the left */
    leftRulesSize = (logicalIndexInLeft != GT_FALSE) ? 1 : 0;
    leftHoleSize   = 0;
    logIndex = (inLogicalIndex - 1);
    /* count rules (ones in bitmap) */
    while ((logIndex + 1) > 0)
    {
        wordIndex = logIndex / 32;
        word      = arrPtr[wordIndex];
        if (((logIndex % 32) == 31)
            && (word == 0xFFFFFFFF)
            && (wordIndex > 0))
        {
            logIndex -= 32;
            leftRulesSize += 32;
            continue;
        }
        bitIndex  = (logIndex % 32);
        if (((word >> bitIndex) & 1) == 1)
        {
            logIndex -= 1;
            leftRulesSize += 1;
            continue;
        }
        /* first hole */
        break;
    }
    /* count holes (zeros in bitmap) */
    while ((logIndex + 1) > 0)
    {
        wordIndex = logIndex / 32;
        word      = arrPtr[wordIndex];
        if (((logIndex % 32) == 31)
            && (word == 0)
            && (wordIndex > 0))
        {
            logIndex -= 32;
            leftHoleSize += 32;
            continue;
        }
        bitIndex  = (logIndex % 32);
        if (((word >> bitIndex) & 1) == 0)
        {
            logIndex -= 1;
            leftHoleSize += 1;
            continue;
        }
        /* first rule */
        break;
    }

   *rightRulesSizePtr    =  rightRulesSize;
   *rightHoleSizePtr     =  rightHoleSize;
   *leftRulesSizePtr     =  leftRulesSize;
   *leftHoleSizePtr      =  leftHoleSize;
   if ((rightHoleSize == 0) && (leftHoleSize == 0))
   {
       /* no free space found */
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
   }
   return GT_OK;
}

/*******************************************************************************
* prioGetNodeByIndex
*
* DESCRIPTION:
*       get priority entry node by logical index
*       return NULL if that logical index not in range of any priority
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       vtcamInfoPtr            - (pointer to)virtual TCAM structure
*       logicalIndex            - input logical index
*
* OUTPUTS:
*       None
*
* RETURNS:
*       pointer to the priority entry node that logical index is in it's range
*       NULL if that logical index not in range of any priority
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC*
    prioGetNodeByIndex(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr,
    IN GT_U32 logicalIndex
)
{
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    needToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *foundPriorityEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL found1;

    needToFindInfo.priority = 0;/* don't care */
    needToFindInfo.baseLogIndex = logicalIndex;
    needToFindInfo.rangeSize = 0;/* don't care */
    /* check if this logical index is occupied */
    found1 = prvCpssAvlPathFind(vtcamInfoPtr->priorityIndexTree,
            PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
            &needToFindInfo,/*pData*/
            avlTreePath,/*path*/
            &dbEntryPtr /* use dedicated var to avoid warnings */);
    foundPriorityEntryPtr = dbEntryPtr;

    if(found1 == GT_FALSE)
    {
        /* no node that hold it */
        return NULL;
    }

    if(logicalIndex >= (foundPriorityEntryPtr->baseLogIndex +
                        foundPriorityEntryPtr->rangeSize))
    {
        /* no node that hold it */
        return NULL;
    }

    return foundPriorityEntryPtr;
}

/*******************************************************************************
* prioGetNodeByPriority
*
* DESCRIPTION:
*       get priority entry node by priority
*       return NULL if that priority not in DB
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       vtcamInfoPtr            - (pointer to)virtual TCAM structure
*       priority                - input priority
*
* OUTPUTS:
*       None
*
* RETURNS:
*       pointer to the priority entry node of the priority
*       NULL if that priority not in DB
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC*
    prioGetNodeByPriority(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr,
    IN GT_U32 priority
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    needToFindInfo;

    needToFindInfo.priority = priority;
    needToFindInfo.baseLogIndex = 0;/* don't care */
    needToFindInfo.rangeSize = 0;/* don't care */
    /* check if this logical index is occupied */
    return prvCpssAvlSearch(vtcamInfoPtr->priorityPriTree,
            &needToFindInfo/*pData*/);
}

/**
* @internal priorityDbLogicalIndexRemoveAndMovePriority_sameIndex function
* @endinternal
*
* @brief   newLogicalIndex == removedLogicalIndex :
*         update the trees of 'priority' that mapped to 'logical indexes'.
*         find the priority that hold the 'new' logical index, update it's
*         entry about the 'number of used' and maybe it's base index.
*         all other priorities between 'new' index to 'removed' index need to
*         modify 'base logical index'
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] priority                 - the  that will get the new index
* @param[in] removedLogicalIndex      - the removed logical index (equal to the new logical index)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on DB bad state.
*/
static GT_STATUS priorityDbLogicalIndexRemoveAndMovePriority_sameIndex
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *vTcamMngDBPtr,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr,
    IN GT_U32 priority,
    IN GT_U32 removedLogicalIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *foundPriorityEntryPtr;
    GT_U32  currPriority;

    /* handle priority that hold the removed index */
    foundPriorityEntryPtr = prioGetNodeByIndex(vtcamInfoPtr,removedLogicalIndex);
    if(foundPriorityEntryPtr)
    {
        currPriority = foundPriorityEntryPtr->priority;

        /* notify the priority DB that the logical index is removed from the range */
        /* if this is the last logical index in the range it will update the range */
        rc = priorityTableEntryRemoveLogicalIndexUpdateRange(vTcamMngDBPtr,vtcamInfoPtr,
            foundPriorityEntryPtr->priority,
            removedLogicalIndex);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }

        /* get again the node (that may changed or not exists any more) */
        foundPriorityEntryPtr = prioGetNodeByPriority(vtcamInfoPtr,currPriority);
        if(foundPriorityEntryPtr &&
           foundPriorityEntryPtr->baseLogIndex == removedLogicalIndex)
        {
            foundPriorityEntryPtr->baseLogIndex++;
            foundPriorityEntryPtr->rangeSize--;
        }
    }

    /* handle priority that get the new index */
    foundPriorityEntryPtr = prioGetNodeByPriority(vtcamInfoPtr,priority);
    if(foundPriorityEntryPtr)
    {
        foundPriorityEntryPtr->rangeSize++;
        if(foundPriorityEntryPtr->baseLogIndex == (removedLogicalIndex+1))
        {
            foundPriorityEntryPtr->baseLogIndex--;
        }
    }

    return GT_OK;
}


/**
* @internal priorityDbLogicalIndexRemoveAndMovePriority_newIndexLower function
* @endinternal
*
* @brief   newLogicalIndex < removedLogicalIndex :
*         update the trees of 'priority' that mapped to 'logical indexes'.
*         find the priority that hold the 'new' logical index, update it's
*         entry about the 'number of used' and maybe it's base index.
*         all other priorities between 'new' index to 'removed' index need to
*         modify 'base logical index'
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] priority                 - the  that will get the new index
* @param[in] newLogicalIndex          - the new logical index
* @param[in] removedLogicalIndex      - the removed logical index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on DB bad state.
*/
static GT_STATUS priorityDbLogicalIndexRemoveAndMovePriority_newIndexLower
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *vTcamMngDBPtr,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr,
    IN GT_U32 priority,
    IN GT_U32 newLogicalIndex, /* newLogicalIndex < removedLogicalIndex*/
    IN GT_U32 removedLogicalIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *foundPriorityEntryPtr;
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    needToFindInfo;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL found1;

    needToFindInfo.priority = 0;/* don't care */
    needToFindInfo.baseLogIndex = removedLogicalIndex;
    needToFindInfo.rangeSize = 0;/* don't care */

    found1 = prvCpssAvlPathFind(vtcamInfoPtr->priorityIndexTree,
            PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
            &needToFindInfo,/*pData*/
            avlTreePath,/*path*/
            &dbEntryPtr /* use dedicated var to avoid warnings */);

    /* handle all the others in the middle */
    while (found1)
    {
        foundPriorityEntryPtr = dbEntryPtr;
        if(foundPriorityEntryPtr->baseLogIndex > removedLogicalIndex)
        {
            /* Range starts after the removed index. */
            /* Should not happen since we asked for <= */
        }
        else
        if(foundPriorityEntryPtr->baseLogIndex == removedLogicalIndex)
        {
            /* Range starts at the removed index. */
            /* Should not happen since removed index should have been empty */
            foundPriorityEntryPtr->rangeSize--;
            if (foundPriorityEntryPtr->rangeSize > 0)
            {
                foundPriorityEntryPtr->baseLogIndex++;
            }
            else
            {
                /* remove the node from the tree */
                /* update the trees of the priority */
                rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
                    vTcamMngDBPtr,vtcamInfoPtr,foundPriorityEntryPtr);
                if(rc != GT_OK)
                {
                    return rc;
                }
                found1 = prvCpssAvlPathFind(vtcamInfoPtr->priorityIndexTree,
                        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                        &needToFindInfo,/*pData*/
                        avlTreePath,/*path*/
                        &dbEntryPtr /* use dedicated var to avoid warnings */);
                continue;
            }
        }
        else
        if(foundPriorityEntryPtr->baseLogIndex > newLogicalIndex)
        {
            if((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) >
                    removedLogicalIndex)
            {
                /* Range contains the removed not the new index. */
                foundPriorityEntryPtr->rangeSize--;
            }
            else
            {
                /* Range does not contain the removed or the new index. */
            }
            foundPriorityEntryPtr->baseLogIndex++;
        }
        else
        if(foundPriorityEntryPtr->baseLogIndex == newLogicalIndex)
        {
            if((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) >
                    removedLogicalIndex)
            {
                /* Range starts at the new and contains the removed index. */
                if (foundPriorityEntryPtr->priority != priority)
                {
                    /* Skip the new index */
                    foundPriorityEntryPtr->rangeSize--;
                    foundPriorityEntryPtr->baseLogIndex++;
                }
            }
            else
            {
                /* Range starts at the new and does not contain the removed index. */
                if (foundPriorityEntryPtr->priority == priority)
                {
                    /* add the new index to the range */
                    foundPriorityEntryPtr->rangeSize++;
                }
                else
                {
                    /* Skip the new index */
                    foundPriorityEntryPtr->baseLogIndex++;
                }
            }
        }
        else
        {
            /* Cases for:(foundPriorityEntryPtr->baseLogIndex < newLogicalIndex)*/
            if((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) >
                    removedLogicalIndex)
            {
                /* Range contains both the new and the removed index. */
                /* Should not happen - new is either at the start or end of some range. */
            }
            else
            if((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) >
                    newLogicalIndex)
            {
                /* Range contains the new not the removed index. */
                /* Should not happen - new is either at the start or end of some range. */
                foundPriorityEntryPtr->rangeSize++;
            }
            else
            if((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) ==
                    newLogicalIndex)
            {
                /* Range starts before and ends just before the new index. */
                if (foundPriorityEntryPtr->priority == priority)
                {
                    /* add the added index to the range */
                    foundPriorityEntryPtr->rangeSize++;
                }
                else
                {
                    /* Unimportant range. */
                }
                break;
            }
            else
            {
                /* Range lower than new index. */
                break;
            }
        }

        found1 = prvCpssAvlPathSeek(vtcamInfoPtr->priorityIndexTree,
                PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E,
                avlTreePath,/*path*/
                &dbEntryPtr /* use dedicated var to avoid warnings */);
    };

    return GT_OK;
}

/**
* @internal priorityDbLogicalIndexRemoveAndMovePriority_newIndexHigher function
* @endinternal
*
* @brief   newLogicalIndex > removedLogicalIndex :
*         update the trees of 'priority' that mapped to 'logical indexes'.
*         find the priority that hold the 'new' logical index, update it's
*         entry about the 'number of used' and maybe it's base index.
*         all other priorities between 'new' index to 'removed' index need to
*         modify 'base logical index'
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] priority                 - the  that will get the new index
* @param[in] newLogicalIndex          - the new logical index
* @param[in] removedLogicalIndex      - the removed logical index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on DB bad state.
*/
static GT_STATUS priorityDbLogicalIndexRemoveAndMovePriority_newIndexHigher
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *vTcamMngDBPtr,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr,
    IN GT_U32 priority,
    IN GT_U32 newLogicalIndex, /* newLogicalIndex > removedLogicalIndex*/
    IN GT_U32 removedLogicalIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *foundPriorityEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *toBeRemovedPriorityEntryPtr = NULL;
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    needToFindInfo;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL found1;

    needToFindInfo.priority = 0;/* don't care */
    needToFindInfo.baseLogIndex = removedLogicalIndex;
    needToFindInfo.rangeSize = 0;/* don't care */

    found1 = prvCpssAvlPathFind(vtcamInfoPtr->priorityIndexTree,
            PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
            &needToFindInfo,/*pData*/
            avlTreePath,/*path*/
            &dbEntryPtr /* use dedicated var to avoid warnings */);

    if (!found1)
    {
        found1 = prvCpssAvlPathSeek(vtcamInfoPtr->priorityIndexTree,
                PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                avlTreePath,/*path*/
                &dbEntryPtr /* use dedicated var to avoid warnings */);
    }

    /* handle all the others in the middle */
    while (found1)
    {
        foundPriorityEntryPtr = dbEntryPtr;
        if((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) <= removedLogicalIndex)
        {
            /* Range ends before the removed index. */
            /* Unimportant range. */
        }
        else
        if((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) <= newLogicalIndex)
        {
            /* Cases for:((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) <= newLogicalIndex)*/
            /* Range ends before the new index. */
            if(foundPriorityEntryPtr->baseLogIndex < removedLogicalIndex)
            {
                /* Range contains the removed and does not contain the new index. */
                foundPriorityEntryPtr->rangeSize--;
            }
            else
            if(foundPriorityEntryPtr->baseLogIndex == removedLogicalIndex)
            {
                /* Range starts at the removed and does not contain the new index. */
                foundPriorityEntryPtr->rangeSize--;
                if (foundPriorityEntryPtr->rangeSize == 0)
                {
                    /* Mark this entry to be removed, and go on */
                    toBeRemovedPriorityEntryPtr = foundPriorityEntryPtr;
                }
            }
            else
            {
                /* Range does not contain the removed or the new index. */
                foundPriorityEntryPtr->baseLogIndex--;
            }
        }
        else
        {
            /* Cases for:((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) > newLogicalIndex)*/
            /* Range ends after the new index. */
            if(foundPriorityEntryPtr->baseLogIndex <= removedLogicalIndex)
            {
                /* Range contains both the removed and the new index. */
                if (foundPriorityEntryPtr->priority != priority)
                {
                    /* remove the new index from the range */
                    foundPriorityEntryPtr->rangeSize--;
                }
            }
            else
            if(foundPriorityEntryPtr->baseLogIndex <= newLogicalIndex)
            {
                /* Range contains the new not the removed index. */
                if (foundPriorityEntryPtr->priority == priority)
                {
                    /* retain the new index in the range */
                    foundPriorityEntryPtr->rangeSize++;
                }
                foundPriorityEntryPtr->baseLogIndex--;
            }
            else
            {
                /* Range higher than new index. */
                break;
            }
        }

        found1 = prvCpssAvlPathSeek(vtcamInfoPtr->priorityIndexTree,
                PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                avlTreePath,/*path*/
                &dbEntryPtr /* use dedicated var to avoid warnings */);
    };

    if (toBeRemovedPriorityEntryPtr != NULL)
    {
        /* remove the node from the tree */
        /* update the trees of the priority */
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
            vTcamMngDBPtr,vtcamInfoPtr,toBeRemovedPriorityEntryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal priorityDbLogicalIndexRemoveAndMovePriority function
* @endinternal
*
* @brief   update the trees of 'priority' that mapped to 'logical indexes'.
*         find the priority that hold the 'new' logical index, update it's
*         entry about the 'number of used' and maybe it's base index.
*         all other priorities between 'new' index to 'removed' index need to
*         modify 'base logical index'
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] priority                 - the  that will get the new index
* @param[in] newLogicalIndex          - the new logical index
* @param[in] removedLogicalIndex      - the removed logical index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on DB bad state.
*/
static GT_STATUS priorityDbLogicalIndexRemoveAndMovePriority
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *vTcamMngDBPtr,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr,
    IN GT_U32 priority,
    IN GT_U32 newLogicalIndex,
    IN GT_U32 removedLogicalIndex
)
{
    if(newLogicalIndex == removedLogicalIndex)
    {
        return priorityDbLogicalIndexRemoveAndMovePriority_sameIndex(
            vTcamMngDBPtr,vtcamInfoPtr,priority,removedLogicalIndex);
    }

    if(newLogicalIndex < removedLogicalIndex)
    {
        return priorityDbLogicalIndexRemoveAndMovePriority_newIndexLower(
            vTcamMngDBPtr,vtcamInfoPtr,priority,
            newLogicalIndex,removedLogicalIndex);
    }

    return priorityDbLogicalIndexRemoveAndMovePriority_newIndexHigher(
        vTcamMngDBPtr,vtcamInfoPtr,priority,
        newLogicalIndex,removedLogicalIndex);
}

/**
* @internal priorityDbLogicalIndexRangeMove function
* @endinternal
*
* @brief   updates priority ranges DB after moving (shifting) range of rules
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] numOfRules               - amount of moved rules
* @param[in] oldBaseLogicalIndex      - old base logical index of moved rules
* @param[in] newBaseLogicalIndex      - new base logical index of moved rules
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on DB bad state.
*
* @note   this function updates nodes indexed by two trees - one by base logical index, other by priority
* @note   the data updated changed, but nodes remain in the same order in both trees and not needed to be
* @note   removed before update and inserted back after update
* @note   the range of numOfRules moved overriding empty range - supposed but not checked
*
*/
static GT_STATUS priorityDbLogicalIndexRangeMove
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *pVTcamMngPtr,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr,
    IN GT_U32                                      numOfRules,
    IN GT_U32                                      oldBaseLogicalIndex,
    IN GT_U32                                      newBaseLogicalIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *foundPriorityEntryPtr;
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    needToFindInfo;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL found1;
    GT_BOOL pullToHigh;
    GT_U32  oldLogIndexStart;
    GT_U32  currentRulesNum;
    GT_U32  currentOldBase;
    GT_U32  currentOldIndexStart;
    GT_U32  maxPositions;
    PRV_CPSS_AVL_TREE_SEEK_ENT seekType;
    GT_U32  shiftSize;
    GT_U32  rangeLow;
    GT_U32  rangeHigh;
    GT_U32  rulesNumInRange;
    GT_U32  rangeHighUnmovedRules;
    GT_U32  rangeLowUnmovedRules;
    GT_U32  holeSizeInRange;

    if ((numOfRules == 0) || (oldBaseLogicalIndex == newBaseLogicalIndex))
    {
        return GT_OK;
    }

    /* calculate the overriden range: base - rangeLow, size - rulesNumInRange                       */
    /* that is a part of destination range: base - newBaseLogicalIndex, size - numOfRules           */
    /* and does not contain rules of a source range: base - oldBaseLogicalIndex, size - numOfRules  */
    /* the calculated range assumed not containig rules - will be overriden                         */
    if (oldBaseLogicalIndex < newBaseLogicalIndex)
    {
        rulesNumInRange = newBaseLogicalIndex - oldBaseLogicalIndex;
        if (rulesNumInRange > numOfRules)
        {
            rulesNumInRange = numOfRules;
        }
        rangeLow = newBaseLogicalIndex + numOfRules - rulesNumInRange;
    }
    else
    {
        rulesNumInRange = oldBaseLogicalIndex - newBaseLogicalIndex;
        if (rulesNumInRange > numOfRules)
        {
            rulesNumInRange = numOfRules;
        }
        rangeLow = newBaseLogicalIndex;
    }

    /* remove overridden range from priorities DB */
    rc = prvCpssDxChVirtualTcamDbPriorityTableIndexRangeClear(
        pVTcamMngPtr, vtcamInfoPtr, rangeLow, rulesNumInRange);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* update logical indexes of all records of priority DB        */
    /* for logical indexes belonging to source range               */
    /* the indexes will match the new positions of the moved rules */
    if (oldBaseLogicalIndex < newBaseLogicalIndex)
    {
        pullToHigh       = GT_TRUE;
        oldLogIndexStart = (oldBaseLogicalIndex + numOfRules - 1);
        seekType         = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;
        shiftSize        = (newBaseLogicalIndex - oldBaseLogicalIndex);
    }
    else
    {
        pullToHigh       = GT_FALSE;
        oldLogIndexStart = oldBaseLogicalIndex;
        seekType         = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
        shiftSize        = (oldBaseLogicalIndex - newBaseLogicalIndex);
    }

    needToFindInfo.priority = 0;/* don't care */
    needToFindInfo.baseLogIndex = oldLogIndexStart;
    needToFindInfo.rangeSize = 0;/* don't care */
    found1 = prvCpssAvlPathFind(
        vtcamInfoPtr->priorityIndexTree,
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
        &needToFindInfo,/*pData*/
        avlTreePath,/*path*/
        &dbEntryPtr /* use dedicated var to avoid warnings */);
    foundPriorityEntryPtr = dbEntryPtr;
    if (found1 == GT_FALSE)
    {
        CPSS_LOG_INFORMATION_MAC(
            "priorityDbLogicalIndexRangeMove base[%d] size[%d] target[%d] - base not found\n",
            numOfRules, oldBaseLogicalIndex, newBaseLogicalIndex);
    }
    if ((found1 == GT_FALSE) && (pullToHigh != GT_FALSE))
    {
        /* no entries containing oldLogIndexStart and less */
        CPSS_LOG_INFORMATION_MAC(
            "priorityDbLogicalIndexRangeMove base[%d] size[%d] target[%d] - nothing updated\n",
            numOfRules, oldBaseLogicalIndex, newBaseLogicalIndex);
        return GT_OK;
    }

    if ((found1 == GT_FALSE) && (pullToHigh == GT_FALSE))
    {
        found1 = prvCpssAvlPathSeek(
            vtcamInfoPtr->priorityIndexTree,
            PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            avlTreePath,/*path*/
            &dbEntryPtr /* use dedicated var to avoid warnings */);
        foundPriorityEntryPtr = dbEntryPtr;
        if ((found1 == GT_FALSE) ||
            (foundPriorityEntryPtr->baseLogIndex >= (oldBaseLogicalIndex + numOfRules - 1)))
        {
            /* nothing to update */
            CPSS_LOG_INFORMATION_MAC(
                "priorityDbLogicalIndexRangeMove base[%d] size[%d] target[%d] - nothing updated\n",
                numOfRules, oldBaseLogicalIndex, newBaseLogicalIndex);
            return GT_OK;
        }
    }

    currentRulesNum = numOfRules;
    currentOldBase  = oldBaseLogicalIndex;
    while (1)
    {
        foundPriorityEntryPtr = dbEntryPtr;
        rangeLow   = foundPriorityEntryPtr->baseLogIndex;
        rangeHigh  = foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize - 1;

        if (pullToHigh != GT_FALSE)
        {
            /* adjust current values */
            if (rangeHigh < currentOldBase)
            {
                return GT_OK;
            }
            maxPositions = rangeHigh - currentOldBase + 1;
            if (currentRulesNum > maxPositions)
            {
                currentRulesNum = maxPositions;
            }
            currentOldIndexStart = currentOldBase + currentRulesNum - 1;
            if (rangeLow > currentOldIndexStart)
            {
                /* should never occur */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* pull to high */
            rangeLowUnmovedRules = 0;
            rulesNumInRange = (currentOldIndexStart - rangeLow + 1);
            if (rulesNumInRange > currentRulesNum)
            {
                rangeLowUnmovedRules = rulesNumInRange - currentRulesNum;
                rulesNumInRange = currentRulesNum;
            }
            holeSizeInRange = (rangeHigh - currentOldIndexStart);
            if (holeSizeInRange > shiftSize)
            {
                holeSizeInRange = shiftSize;
            }
            rangeHighUnmovedRules = (rangeHigh - currentOldIndexStart) - holeSizeInRange;

            /* update */
            if ((rangeLowUnmovedRules == 0) && (rangeHighUnmovedRules == 0))
            {
                foundPriorityEntryPtr->baseLogIndex += shiftSize;
            }
            else if ((rangeLowUnmovedRules != 0) && (rangeHighUnmovedRules == 0))
            {
                foundPriorityEntryPtr->rangeSize += shiftSize;
            }
            else if ((rangeLowUnmovedRules == 0) && (rangeHighUnmovedRules != 0))
            {
                if (foundPriorityEntryPtr->rangeSize <= shiftSize)
                {
                    /* should never occur */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                foundPriorityEntryPtr->baseLogIndex += shiftSize;
                foundPriorityEntryPtr->rangeSize -= shiftSize;
            }
            else
            {
                /* no update needed */
            }
        }
        else
        {
            /* adjust current values */
            if (rangeLow > (currentOldBase + currentRulesNum - 1))
            {
                return GT_OK;
            }

            maxPositions = ((currentOldBase + currentRulesNum - 1) - rangeLow + 1);
            if (currentRulesNum > maxPositions)
            {
                currentOldBase += (currentRulesNum - maxPositions);
                currentRulesNum = maxPositions;
            }
            currentOldIndexStart = currentOldBase;

            /* pull to low */
            rangeHighUnmovedRules = 0;
            rulesNumInRange = (rangeHigh - currentOldIndexStart + 1);
            if (rulesNumInRange > currentRulesNum)
            {
                rangeHighUnmovedRules = rulesNumInRange - currentRulesNum;
                rulesNumInRange       = currentRulesNum;
            }
            holeSizeInRange = (currentOldIndexStart - rangeLow);
            if (holeSizeInRange > shiftSize)
            {
                holeSizeInRange = shiftSize;
            }
            rangeLowUnmovedRules = (currentOldIndexStart - rangeLow) - holeSizeInRange;


            /* update */
            if ((rangeLowUnmovedRules == 0) && (rangeHighUnmovedRules == 0))
            {
                foundPriorityEntryPtr->baseLogIndex -= shiftSize;
            }
            else if ((rangeLowUnmovedRules != 0) && (rangeHighUnmovedRules == 0))
            {
                if (foundPriorityEntryPtr->rangeSize <= shiftSize)
                {
                    /* should never occur */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                foundPriorityEntryPtr->rangeSize -= shiftSize;
            }
            else if ((rangeLowUnmovedRules == 0) && (rangeHighUnmovedRules != 0))
            {
                if (foundPriorityEntryPtr->rangeSize <= shiftSize)
                {
                    /* should never occur */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                foundPriorityEntryPtr->baseLogIndex -= shiftSize;
                foundPriorityEntryPtr->rangeSize += shiftSize;
            }
            else
            {
                /* no update needed */
            }
        }

        found1 = prvCpssAvlPathSeek(
            vtcamInfoPtr->priorityIndexTree,
            seekType,
            avlTreePath,/*path*/
            &dbEntryPtr /* use dedicated var to avoid warnings */);
        if (found1 == GT_FALSE) break;
    }

    return GT_OK;
}

/**
* @internal priorityDbEntryAdjustByLogicalIndexBitmap function
* @endinternal
*
* @brief   truncate priority DB entry empty Low and High logical indexes, If all the space without rules - free the entry
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to)virtual TCAM Manger structure
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] vtcamInfoPtr             - (pointer to)priority DB entry
*
* @retval GT_OK                    - on success
*/
static GT_STATUS priorityDbEntryAdjustByLogicalIndexBitmap
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                    *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC         *vtcamInfoPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *priorityEntryPtr
)
{
    GT_U32 rangeLow;
    GT_U32 rangeHigh;
    GT_U32 bitVal;
    GT_U32 rangeIsEmpty;

    rangeIsEmpty = 0;
    rangeLow  = priorityEntryPtr->baseLogIndex;
    rangeHigh = rangeLow + priorityEntryPtr->rangeSize - 1;
    if (priorityEntryPtr->rangeSize > 0)
    {
        for (/*no init*/; (rangeLow <= rangeHigh); rangeLow++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(vtcamInfoPtr->usedRulesBitmapArr, rangeLow, 1, bitVal);
            if (bitVal) break;
        }
        for (/*no init*/; ((rangeLow <= rangeHigh) && (rangeHigh > 0)); rangeHigh--)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(vtcamInfoPtr->usedRulesBitmapArr, rangeHigh, 1, bitVal);
            if (bitVal) break;
        }
        if (rangeLow == rangeHigh)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(vtcamInfoPtr->usedRulesBitmapArr, rangeHigh, 1, bitVal);
            if (bitVal == 0)
            {
                rangeIsEmpty = 1;
            }
        }
        if (rangeLow > rangeHigh)
        {
            rangeIsEmpty = 1;
        }
    }
    else
    {
        rangeIsEmpty = 1;
    }

    if (rangeIsEmpty == 0)
    {
        /* inplace update of the entry */
        priorityEntryPtr->baseLogIndex = rangeLow;
        priorityEntryPtr->rangeSize    = rangeHigh - rangeLow + 1;
    }
    else
    {
        /* delete entry - no rules in it's range */
        /* delete entry from priority tree */
        prvCpssAvlItemRemove(vtcamInfoPtr->priorityPriTree, priorityEntryPtr);
        /* delete entry from priority index tree */
        prvCpssAvlItemRemove(vtcamInfoPtr->priorityIndexTree, priorityEntryPtr);
        /* free a buffer back to its pool */
        cpssBmPoolBufFree(pVTcamMngPtr->priorityPool, priorityEntryPtr);
    }

    return GT_OK;
}

/**
* @internal priorityDbEntryAdjustByLogicalIndexBitmap function
* @endinternal
*
* @brief   truncate priority DB entry empty Low and High logical indexes, If all the space without rules - free the entry
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to)virtual TCAM Manger structure
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] searchLogicalIndex       - Logical Index to search priority DB entry (typicall of deleted rule)
*
* @retval GT_OK                    - on success
*/
static GT_STATUS prvDxChVirtualTcamPriorityDbEntryAdjustByLogicalIndexBitmap
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                    *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC         *vtcamInfoPtr,
    IN   GT_U32                                              searchLogicalIndex
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *foundPriorityEntryPtr;
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    needToFindInfo;
    GT_VOID_PTR                 dbEntryPtr;
    GT_BOOL found1;

    needToFindInfo.priority = 0; /* don't care */
    needToFindInfo.baseLogIndex = searchLogicalIndex;
    needToFindInfo.rangeSize = 0;/* don't care */
    found1 = prvCpssAvlPathFind(
        vtcamInfoPtr->priorityIndexTree,
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
        &needToFindInfo,/*pData*/
        avlTreePath,/*path*/
        &dbEntryPtr /* use dedicated var to avoid warnings */);
    foundPriorityEntryPtr = dbEntryPtr;
    if ((found1 == GT_FALSE) ||
        ((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) <= searchLogicalIndex))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return priorityDbEntryAdjustByLogicalIndexBitmap(
        pVTcamMngPtr, vtcamInfoPtr, foundPriorityEntryPtr);
}

/**
* @internal rulesDbLogicalIndexRemove function
* @endinternal
*
* @brief   update the trees of 'rules' that mapped to 'logical indexes'
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] newLogicalIndex          - the new logical index
* @param[in] removedLogicalIndex      - the removed logical index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on DB bad state.
*/
static GT_STATUS rulesDbLogicalIndexRemove
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr,
    IN GT_U32 newLogicalIndex,
    IN GT_U32 removedLogicalIndex
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC needToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC *ruleIdEntryPtr;
    GT_U32  startLogicalIndex;
    GT_U32  endLogicalIndex;
    GT_U32  operatorPlusPlus;
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    PRV_CPSS_AVL_TREE_SEEK_ENT  seekMode;
    PRV_CPSS_AVL_TREE_FIND_ENT  findMode;
    GT_BOOL found1,found2;

    if(newLogicalIndex == removedLogicalIndex)
    {
        return GT_OK;
    }

    if(removedLogicalIndex > newLogicalIndex)
    {
        /* move indexes up to close the hole */
        operatorPlusPlus = 1;
    }
    else
    {
        /* move indexes down to close the hole */
        operatorPlusPlus = 0;
    }

    startLogicalIndex = removedLogicalIndex;
    endLogicalIndex = newLogicalIndex;

    needToFindInfo.ruleId = 0;/*dont care */
    needToFindInfo.logicalIndex = startLogicalIndex;
    needToFindInfo.applicationDataPtr = 0;/*dont care */

    findMode = operatorPlusPlus == 0 ?
        PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E :
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E;

    /* check if this logical index is occupied */
    found1 = prvCpssAvlPathFind(vtcamInfoPtr->ruleIdIndexTree,
            findMode,
            &needToFindInfo,/*pData*/
            avlTreePath,/*path*/
            &dbEntryPtr /* use dedicated var to avoid warnings */);
    ruleIdEntryPtr = dbEntryPtr;

    if(found1 == GT_FALSE)
    {
        findMode = operatorPlusPlus == 0 ?
            PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E :
            PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E;

        found2 = prvCpssAvlPathFind(vtcamInfoPtr->ruleIdIndexTree,
                findMode,
                &needToFindInfo,/*pData*/
                avlTreePath,/*path*/
                &dbEntryPtr /* use dedicated var to avoid warnings */);
         ruleIdEntryPtr = dbEntryPtr;
    }
    else
    {
        found2 = GT_FALSE;
    }

    if(found1 == GT_FALSE && found2 == GT_FALSE)
    {
        /* empty DB ... no need to update anything */
        return GT_OK;
    }

    seekMode = operatorPlusPlus ?
                PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E:
                PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
    /* modify content of the elements in the tree but without changing the
        'tree path' because modification in ascending/descending order */
    do
    {
        if(operatorPlusPlus)
        {
            if((ruleIdEntryPtr->logicalIndex + 1) >= vtcamInfoPtr->rulesAmount)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: can't iterate to next logical index , since out of range");
            }

            ruleIdEntryPtr->logicalIndex++;
        }
        else
        {
            if(ruleIdEntryPtr->logicalIndex == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: can't iterate to previous logical index , since out of range");
            }

            ruleIdEntryPtr->logicalIndex--;
        }

        if(operatorPlusPlus && ((ruleIdEntryPtr->logicalIndex-1) <= endLogicalIndex))
        {
            /* no more updates */
            break;
        }
        else
        if((operatorPlusPlus == 0) && ((ruleIdEntryPtr->logicalIndex+1) >= endLogicalIndex))
        {
            /* no more updates */
            break;
        }

        found1 = prvCpssAvlPathSeek(vtcamInfoPtr->ruleIdIndexTree,
            seekMode,
            avlTreePath,
            &dbEntryPtr /* use dedicated var to avoid warnings */);
         ruleIdEntryPtr = dbEntryPtr;

        if(GT_FALSE == found1)
        {
            /* no more ?! */
            break;
        }

    }
    while(1);

    return GT_OK;
}

/**
* @internal rulesDbLogicalIndexRangeShift function
* @endinternal
*
* @brief   update the trees of 'rules' that mapped to 'logical indexes' of shifted rules
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] startLogicalIndex        - the new logical index
* @param[in] numOfRules               - amount of rules
* @param[in] lowToHighDirection       - if GT_TRUE range [(startLogicalIndex - numOfRules + 1)..startLogicalIndex]
*                                          moved to [(startLogicalIndex - numOfRules + 1 + shiftValue) .. (startLogicalIndex + shiftValue)]
*                                       if GT_FALSE range [startLogicalIndex .. (startLogicalIndex + numOfRules - 1)]
*                                          moved to [(startLogicalIndex - shiftValue) .. (startLogicalIndex + numOfRules - 1 - shiftValue)]
* @param[in] shiftValue               - shift value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on DB bad state.
*/
static GT_STATUS rulesDbLogicalIndexRangeShift
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr,
    IN GT_U32   startLogicalIndex,
    IN GT_U32   numOfRules,
    IN GT_BOOL  lowToHighDirection,
    IN GT_U32   shiftValue
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC needToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC *ruleIdEntryPtr;
    GT_U32  endLogicalIndex;
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    PRV_CPSS_AVL_TREE_SEEK_ENT  seekMode;
    PRV_CPSS_AVL_TREE_FIND_ENT  findMode;
    GT_BOOL found1;

    if ((numOfRules == 0) || (shiftValue == 0))
    {
        return GT_OK;
    }

    if (lowToHighDirection == GT_FALSE)
    {
        endLogicalIndex = startLogicalIndex + numOfRules - 1;
        findMode = PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E;
        seekMode = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
    }
    else
    {
        endLogicalIndex = startLogicalIndex - numOfRules + 1;
        findMode = PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E;
        seekMode = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;
    }

    needToFindInfo.ruleId = 0;/*dont care */
    needToFindInfo.logicalIndex = startLogicalIndex;
    needToFindInfo.applicationDataPtr = 0;/*dont care */


    /* check if this logical index is occupied */
    found1 = prvCpssAvlPathFind(
        vtcamInfoPtr->ruleIdIndexTree,
        findMode,
        &needToFindInfo,/*pData*/
        avlTreePath,/*path*/
        &dbEntryPtr /* use dedicated var to avoid warnings */);
    ruleIdEntryPtr = dbEntryPtr;


    if (found1 == GT_FALSE)
    {
        /* empty DB ... no need to update anything */
        return GT_OK;
    }

    /* modify content of the elements in the tree but without changing the
        'tree path' because modification in ascending/descending order */
    do
    {
        if ((lowToHighDirection != GT_FALSE)
            && (ruleIdEntryPtr->logicalIndex < endLogicalIndex))
        {
            break;
        }

        if ((lowToHighDirection != GT_FALSE)
            && (ruleIdEntryPtr->logicalIndex <= startLogicalIndex))
        {
            ruleIdEntryPtr->logicalIndex += shiftValue;
        }

        if ((lowToHighDirection == GT_FALSE)
            && (ruleIdEntryPtr->logicalIndex > endLogicalIndex))
        {
            break;
        }

        if ((lowToHighDirection == GT_FALSE)
            && (ruleIdEntryPtr->logicalIndex >= startLogicalIndex))
        {
            ruleIdEntryPtr->logicalIndex -= shiftValue;
        }

        found1 = prvCpssAvlPathSeek(vtcamInfoPtr->ruleIdIndexTree,
            seekMode,
            avlTreePath,
            &dbEntryPtr /* use dedicated var to avoid warnings */);
         ruleIdEntryPtr = dbEntryPtr;

        if(GT_FALSE == found1)
        {
            /* no more ?! */
            break;
        }

    }
    while(1);

    return GT_OK;
}

/* original index is not valid */
#define ORIG_INDEX_NOT_USED_CNS   (0xFFFFFFFF - 1)

/**
* @internal prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexGet function
* @endinternal
*
* @brief   Get place for new rule.
*         If needed move existing rules.
*         If needed resize vTCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - vTCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - virtual TCAM Id
*                                      (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] origIndexForPriorityUpdate - the original index of the entry that doing
* @param[in] priority                 update'
*                                      value ORIG_INDEX_NOT_USED_CNS meaning that it is
*                                      not relevant.
* @param[in] vTcamMaxSize             - maximal size of vTCAM. (no resize to greater).
*                                      The current size retrieved from Segment Table.
* @param[in] hwContextPtr             - (pointer to)HW context for moving existing rules.
* @param[in] priority                 - new rule priority
* @param[in] position                 -  in range of rule of the
*                                      specified priority (see enum)
* @param[in] sizeIncrement            - amount of rules to be added if resize will needed.
* @param[in] recalled                 -  due to autoresize
*
* @param[out] logIndexPtr              - (pointer to) Logical Index of new rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_FULL                  - on table full (no free logical index)
*/
GT_STATUS prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexGet
(
    IN   GT_U32                                                    vTcamMngId,
    IN   GT_U32                                                    vTcamId,
    IN   GT_U32                                                    origIndexForPriorityUpdate ,
    IN   GT_U32                                                    vTcamMaxSize,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_HW_CONTEXT_STC  *hwContextPtr,
    IN   GT_U32                                                    priority,
    IN   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT   position,
    IN   GT_U32                                                    sizeIncrement,
    IN   GT_BOOL                                                   recalled,
    OUT  GT_U32                                                    *logIndexPtr
)
{
    GT_STATUS   rc;
    GT_BOOL     found;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC         *vtcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     needToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *foundPriorityEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_U32      logicalIndex = 0;
    GT_U32      freeLogicalIndex = 0;
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;
    GT_U32      ii;
    GT_BOOL     lookForUnusedEntry;
    GT_U32      higherValidIndex;
    GT_U32      saveGuaranteedSize;
    GT_BOOL     indexIsOk = GT_TRUE;
    RESIZE_MODE_ENT  resizeMode;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (vTcamMngDBPtr->vTcamCfgPtrArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    vTcamMaxSize = vTcamMaxSize;

    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod !=
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        /* the function is not for this mode */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: the function is only for priority mode");
    }

    /* find the priority in the DB */
    rc = prvCpssDxChVirtualTcamDbPriorityTableEntryGet(
        vtcamInfoPtr,priority,&priorityEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    CPSS_LOG_INFORMATION_MAC("the priority[%d] hold : baseLogIndex[%d] rangeSize[%d] \n",
        priorityEntry.priority,
        priorityEntry.baseLogIndex,
        priorityEntry.rangeSize);

    if(priorityEntry.priority != priority)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: the DB hold different priority[%d] than expected[%d]",
            priorityEntry.priority,priority);
    }

    if((priorityEntry.baseLogIndex + priorityEntry.rangeSize) >
        vtcamInfoPtr->rulesAmount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: the DB hold mismatch information regarding baseLogIndex[%d] , rangeSize[%d] and rulesAmount[%d]",
            priorityEntry.baseLogIndex,
            priorityEntry.rangeSize,
            vtcamInfoPtr->rulesAmount);
    }

    logicalIndex = 0;/*dummy for compiler warnings*/
    resizeMode   = RESIZE_MODE_WITH_INPUT_INDEX_E;
    lookForUnusedEntry = GT_TRUE;
    if (position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E)
    {
        if((priorityEntry.baseLogIndex + priorityEntry.rangeSize) ==
            vtcamInfoPtr->rulesAmount)
        {
            indexIsOk = GT_FALSE;/* the index is not OK because it not match the {base,range} */
            resizeMode = RESIZE_MODE_WITHOUT_INPUT_INDEX_E;
            if( priorityEntry.rangeSize == 0)
            {
                /* new priority is added to the vtcam as worst priority and got indication that
                   end of range is used */
                if(priorityEntry.baseLogIndex == 0)
                {
                    logicalIndex = 0;
                    CPSS_LOG_INFORMATION_MAC("use logical index[%d] (empty vtcam) as last index and check if can auto-resize in the new priority \n",
                        logicalIndex);
                }
                else
                {
                    logicalIndex = priorityEntry.baseLogIndex - 1;
                    CPSS_LOG_INFORMATION_MAC("use logical index[%d] (last index in the vtcam) as last index and push others to make room for index in the new priority \n",
                        logicalIndex);
                }
            }
            else
            {
                /* this priority uses last indexes in the vtcam , so can't use 'next index' */
                /* we need to calculate where is the nearest hole. */
                logicalIndex = priorityEntry.baseLogIndex + priorityEntry.rangeSize - 1;
                CPSS_LOG_INFORMATION_MAC("use logical index[%d] (last index in the vtcam) as last index and push others to make room for last index in the priority \n",
                    logicalIndex);
            }

            lookForUnusedEntry = GT_FALSE;
        }
    }
    else /* position is 'FIRST' */
    {
        if(priorityEntry.baseLogIndex == vtcamInfoPtr->rulesAmount)
        {
            /* rangeSize must be 0 at this point ! so this is new priority */
            /* so use index inside the valid rules amount ... like for 'LAST' */

            indexIsOk = GT_FALSE;/* the index is not OK because it not match the {base,range} */
            resizeMode = RESIZE_MODE_WITHOUT_INPUT_INDEX_E;
            if(priorityEntry.baseLogIndex == 0)
            {
                logicalIndex = 0;
                CPSS_LOG_INFORMATION_MAC("use logical index[%d] (empty vtcam) as first index and check if can auto-resize in the new priority \n",
                    logicalIndex);
            }
            else
            {
                logicalIndex = priorityEntry.baseLogIndex - 1;
                CPSS_LOG_INFORMATION_MAC("use logical index[%d] (last index in the vtcam) as first index and push others to make room for index in the new priority \n",
                    logicalIndex);
            }
            lookForUnusedEntry = GT_FALSE;
        }
    }

    if(lookForUnusedEntry == GT_TRUE)
    {
        if(position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E)
        {
            /* we need new logical index .
                start with the obvious one --> the 'next index'
            */
            logicalIndex = priorityEntry.baseLogIndex + priorityEntry.rangeSize;
            CPSS_LOG_INFORMATION_MAC("check the logical index[%d] as next after last index in the priority \n",
                logicalIndex);
        }
        else
        {
            /* we need new logical index .
                start with the obvious one --> the 'first index'
            */
            logicalIndex = priorityEntry.baseLogIndex;
            CPSS_LOG_INFORMATION_MAC("check the logical index[%d] as lowest index in the priority \n",
                logicalIndex);
        }
    }
    else
    {
        /* already got logicalIndex */
    }

    /* check if can keep the same index without HW moves */
    if(logicalIndex < vtcamInfoPtr->rulesAmount)/* index within the range */
    {
        if(position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E)
        {
            if((logicalIndex == (origIndexForPriorityUpdate + 1) && (lookForUnusedEntry == GT_TRUE)) || /*last in current priority
                            (if rangeSize == 0 than it is last in previous priority) */
               logicalIndex == origIndexForPriorityUpdate)  /*first in next priority */
            {
                CPSS_LOG_INFORMATION_MAC("no HW moves needed (to set rule as last)\n");
                logicalIndex = origIndexForPriorityUpdate;
                freeLogicalIndex = logicalIndex;

                goto foundSameIndexToUse_lbl;
            }
        }
        else
        {
            if((logicalIndex == (origIndexForPriorityUpdate + 1) && (lookForUnusedEntry == GT_TRUE)) ||/*last in previous priority */
               logicalIndex == origIndexForPriorityUpdate)/* first in current priority
               (if rangeSize == 0 than it is first in next priority) */
            {
                /*    this 'first' is attached to last rule in previous priority */
                /* or this 'first' is already first rule in current or next priority */
                CPSS_LOG_INFORMATION_MAC("no HW moves needed (to set rule as first)\n");
                logicalIndex = origIndexForPriorityUpdate;
                freeLogicalIndex = logicalIndex;

                goto foundSameIndexToUse_lbl;
            }
        }
    }

    if(lookForUnusedEntry == GT_TRUE)
    {
        cpssOsMemSet(&needToFindInfo,0,sizeof(needToFindInfo));
        needToFindInfo.logicalIndex = logicalIndex;
        /* check if this logical index is occupied */
        found = prvCpssAvlPathFind(vtcamInfoPtr->priorityIndexTree,
                PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                &needToFindInfo,/*pData*/
                avlTreePath,/*path*/
                &dbEntryPtr /* use dedicated var to avoid warnings */);
         foundPriorityEntryPtr = dbEntryPtr;

        if((found == GT_FALSE) || /* no equal or higher index */
           (foundPriorityEntryPtr->baseLogIndex > logicalIndex)) /* no equal index */
        {
            if(position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E)
            {
                /* the logical index is not used .. but look for better higher index that is not used */
                CPSS_LOG_INFORMATION_MAC("the logical Index[%d] is free  .. but look for better higher index that is not used \n",
                                            logicalIndex);

                if(priorityEntry.rangeSize != 0)
                {
                    if(found == GT_FALSE)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: the DB not found priority[%d] in one DB but found in another",
                            priorityEntry.priority);
                    }

                    rc = findNearestLogicalIndexUsed(vtcamInfoPtr,
                        logicalIndex,
                        GT_FALSE, /*more : lessOrMore*/
                        priorityEntry.baseLogIndex + priorityEntry.rangeSize - 1,/*lastLogicalIndexToCheck*/
                        &higherValidIndex
                    );

                    if(rc != GT_OK)
                    {
                        /* we don't have better index than logicalIndex */
                        CPSS_LOG_INFORMATION_MAC("not found better logical Index , so [%d] will be used \n",
                                                    logicalIndex);
                    }
                    else
                    {
                        /* we got higher index of 'used index' ... so need to
                           use the one before it */
                        logicalIndex = higherValidIndex - 1;
                        CPSS_LOG_INFORMATION_MAC("found better logical Index[%d] and will be used \n",
                                                    logicalIndex);
                    }
                }
                else  /*priorityEntry.rangeSize == 0*/
                {
                    /* this is new range */
                    CPSS_LOG_INFORMATION_MAC("priority[%d] considered new and will be added with first logical index[%d] \n",
                        priorityEntry.priority ,
                        logicalIndex);
                    /*
                        the code below will do for this case:
                       priorityEntry.rangeSize++;

                       meaning that priorityEntry.rangeSize will be 1 ,
                       indicating that we add the first entry to the range
                    */
                }

            }
            else /*position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E*/
            {
                /* the logical index is not used */
                CPSS_LOG_INFORMATION_MAC("the logical Index[%d] is free and will be used \n",
                        logicalIndex);
            }

            /* we can use this new logical index */
            *logIndexPtr = logicalIndex;

            if(position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E ||
              (priorityEntry.baseLogIndex + priorityEntry.rangeSize) <= logicalIndex) /* POSITION_FIRST and add new priority entry into the DB */
            {
                priorityEntry.rangeSize++;

                /* updated the DB about the new range value */
                rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
                    vTcamMngDBPtr,vtcamInfoPtr,&priorityEntry);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
                }
            }

            return GT_OK;
        }

        /* the logical index is used */
        CPSS_LOG_INFORMATION_MAC("the logical Index[%d] is used by priority[%d] baseLogIndex[%d] rangeSize[%d] \n",
            logicalIndex,
            foundPriorityEntryPtr->priority,
            foundPriorityEntryPtr->baseLogIndex,
            foundPriorityEntryPtr->rangeSize);
    }

    /* find nearest to specific logical index an unused logical index */
    rc = findNearestLogicalIndexHole(vtcamInfoPtr,
        logicalIndex,
        &freeLogicalIndex);
    if(rc != GT_OK)
    {
        /* No Hole - needed resize */
        if ((vtcamInfoPtr->autoResizeDisable) ||
            (vtcamInfoPtr->tcamInfo.autoResize == GT_FALSE))
        {
            return rc;
        }

        if (recalled != GT_FALSE)
        {
            /* should not occur */
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }

        /* Auto resize */
        /* resize memory for Virtual TCAM in Segment Table for TCAM.*/

        /* save guaranteedNumOfRules updated by resize function */
        saveGuaranteedSize = vtcamInfoPtr->tcamInfo.guaranteedNumOfRules;
        rc = vtcamDbSegmentTableVTcamResize(
            vTcamMngId ,vTcamId,
            sizeIncrement,
            resizeMode,
            logicalIndex);

        if (VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr) != NULL)
        {
            cpssOsFree(VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr));
            VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr) = NULL;
            VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr) = 0;
        }
        /* restore guaranteedNumOfRules - should remain as was */
        vtcamInfoPtr->tcamInfo.guaranteedNumOfRules = saveGuaranteedSize;
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
        }

        /* recursive call after resize */
        return prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexGet(
            vTcamMngId, vTcamId,
            origIndexForPriorityUpdate,
            vTcamMaxSize, hwContextPtr,
            priority, position, GT_TRUE /*recalled*/,
            sizeIncrement, logIndexPtr);
    }

    if(freeLogicalIndex >= vtcamInfoPtr->rulesAmount)
    {
        /* wrong calculations */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: wrong calculations : freeLogicalIndex[%d] >= vtcamInfoPtr->rulesAmount[%d]",
            freeLogicalIndex,vtcamInfoPtr->rulesAmount);
    }

    /* so we got a hole in the logical index range */

    /* we need to move all logical indexes from current logical index to the
       free logical index */
    /* the move procedure  must invalidate source rule to guarantee consistency
       of following copied one. because write/copy updates trio action, pattern
       and mask sequentially but not in atomic manner.*/
    if(freeLogicalIndex < logicalIndex)
    {
        if(indexIsOk == GT_TRUE)
        {
            /* the logicalIndex is kept in the 'upper' priority */
            /* so decrease out index */
            logicalIndex--;
        }

        for(ii = freeLogicalIndex ; ii < logicalIndex; ii++)
        {
            /* call the CB function to move each entry at a time */
            rc = hwContextPtr->ruleMoveFuncPtr(hwContextPtr->cookiePtr,
                ii+1,/*srcRuleLogicalIndex*/
                ii,/*dstRuleLogicalIndex*/
                vtcamInfoPtr->tcamInfo.ruleSize,
                GT_TRUE/*move*/);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
            }
        }

        /* copy the bits in the DB of vtcamInfoPtr->usedRulesBitmapArr */
        copyBitsInMemory(vtcamInfoPtr->usedRulesBitmapArr,/*memPtr*/
                 freeLogicalIndex,/*targetStartBit*/
                 freeLogicalIndex + 1, /*sourceStartBit*/
                 logicalIndex - freeLogicalIndex);/*numBits*/
    }
    else
    if(freeLogicalIndex > logicalIndex)
    {
        for(ii = freeLogicalIndex-1 ; ii >= logicalIndex; ii--)
        {
            /* call the CB function to move each entry at a time */
            rc = hwContextPtr->ruleMoveFuncPtr(hwContextPtr->cookiePtr,
                ii,/*srcRuleLogicalIndex*/
                ii+1,/*dstRuleLogicalIndex*/
                vtcamInfoPtr->tcamInfo.ruleSize,
                GT_TRUE/*move*/);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
            }

            if(ii == 0)/* logicalIndex is 0 */
            {
                break;
            }
        }

        /* copy the bits in the DB of vtcamInfoPtr->usedRulesBitmapArr */
        copyBitsInMemory(vtcamInfoPtr->usedRulesBitmapArr,/*memPtr*/
                 logicalIndex + 1,/*targetStartBit*/
                 logicalIndex, /*sourceStartBit*/
                 freeLogicalIndex - logicalIndex);/*numBits*/
    }
    else /* freeLogicalIndex == logicalIndex */
    {
        foundSameIndexToUse_lbl:
        /*do nothing more*/
        ;
    }

    /* update the priority DB about the removed index , that will move the ranges of them */
    rc = priorityDbLogicalIndexRemoveAndMovePriority(vTcamMngDBPtr,vtcamInfoPtr,
        priority,
        logicalIndex,freeLogicalIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    /* get the priority again from the DB ... because it may have changed */
    rc = prvCpssDxChVirtualTcamDbPriorityTableEntryGet(
        vtcamInfoPtr,priority,&priorityEntry);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    /* update the rules DB about the removed index , that will change logical indexes*/
    rc = rulesDbLogicalIndexRemove(vtcamInfoPtr,logicalIndex,freeLogicalIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    if(priorityEntry.baseLogIndex == vtcamInfoPtr->rulesAmount)
    {
        if(priorityEntry.rangeSize != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: expected new priority but already hold rangeSize[%d]",
                priorityEntry.rangeSize);
        }
        priorityEntry.baseLogIndex--;
        priorityEntry.rangeSize = 1;

        /* updated the DB about the new range value */
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
            vTcamMngDBPtr,vtcamInfoPtr,&priorityEntry);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }
    else
    if(priorityEntry.rangeSize == 0)
    {
        priorityEntry.rangeSize++;
        /* updated the DB about the new range value */
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
            vTcamMngDBPtr,vtcamInfoPtr,&priorityEntry);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    /* the freeLogicalIndex got the 'shift' so we actually state to caller that
       'logicalIndex' is actually free now !!! */
    *logIndexPtr = logicalIndex;


    return GT_OK;
}

void prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1GetMoveTraceEnableSet(GT_U32 enable)
{
    VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1GetMoveTraceEnable) = enable;
}

/**
* @internal prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1Get function
* @endinternal
*
* @brief   Get place for new rule.
*         If needed move existing rules.
*         If needed resize vTCAM.
*         Optimized version of prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexGet.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - vTCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - virtual TCAM Id
*                                      (APPLICABLE RANGES: 0..0xFFFF)
* @param[in] origIndexForPriorityUpdate - the original index of the entry that doing
* @param[in] priority                 update'
*                                      value ORIG_INDEX_NOT_USED_CNS meaning that it is
*                                      not relevant.
* @param[in] vTcamMaxSize             - maximal size of vTCAM. (no resize to greater).
*                                      The current size retrieved from Segment Table.
* @param[in] hwContextPtr             - (pointer to)HW context for moving existing rules.
* @param[in] priority                 - new rule priority
* @param[in] position                 -  in range of rule of the
*                                      specified priority (see enum)
* @param[in] sizeIncrement            - amount of rules to be added if resize will needed.
* @param[in] recalled                 -  due to autoresize
*
* @param[out] logIndexPtr              - (pointer to) Logical Index of new rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_FULL                  - on table full (no free logical index)
*/
GT_STATUS prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1Get
(
    IN   GT_U32                                                    vTcamMngId,
    IN   GT_U32                                                    vTcamId,
    IN   GT_U32                                                    origIndexForPriorityUpdate ,
    IN   GT_U32                                                    vTcamMaxSize,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_HW_CONTEXT_STC  *hwContextPtr,
    IN   GT_U32                                                    priority,
    IN   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT   position,
    IN   GT_U32                                                    sizeIncrement,
    IN   GT_BOOL                                                   recalled,
    OUT  GT_U32                                                    *logIndexPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                  *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC         *vtcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityNeighbourEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityMaxRange;
    GT_U32      logicalIndex = 0;
    GT_U32      freeLogicalIndex = 0;
    GT_U32      ii;
    GT_U32      saveGuaranteedSize;
    RESIZE_MODE_ENT  resizeMode;
    GT_U32      rightRulesSize;
    GT_U32      rightHoleSize;
    GT_U32      leftRulesSize;
    GT_U32      leftHoleSize;
    GT_U32      shiftSize;
    GT_U32      numOfMovedRules;
    GT_BOOL     logicalIndexInLeft;
    GT_U32      preparedLogicalIndex;
    GT_U32      newRuleLogicalIndex;
    GT_U32      ruleOldPriority;
    GT_BOOL     priorityUpdateWithoutMove;
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;
    GT_U32      priorityUpdateRuleId;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     ruleNeedToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     *ruleIdEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL    retVal;
    GT_U32     holeBase;
    GT_U32     holeSize;
    GT_U32     holeLowPartSize;
    GT_U32     closeToHoleLow; /*0 - low side, 100 - high side, 50 - middle*/

    priorityUpdateRuleId = 0; /* avoid compiler warning */

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (vTcamMngDBPtr->vTcamCfgPtrArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    vTcamMaxSize = vTcamMaxSize;

    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod !=
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        /* the function is not for this mode */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: the function is only for priority mode");
    }

    if (origIndexForPriorityUpdate != ORIG_INDEX_NOT_USED_CNS)
    {
        /* Called from RulePriorityUpdate - update priority of existing rule       */
        /* this function must prepare the new free position for the rule if needed */
        /* and return the new position to caller                                   */
        /* the caller will move the rule and update the Rule Id DB                 */
        /* but the priority DB updated by this function.                           */

        priorityUpdateWithoutMove = GT_TRUE; /* initial to be updated */

        /* get the priority of the orig ruleId */
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntryFind(
            vtcamInfoPtr,
            origIndexForPriorityUpdate,
            PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,/*findType*/
            avlTreePath,
            &priorityEntry);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        ruleOldPriority = priorityEntry.priority;

        rc = prvCpssDxChVirtualTcamDbPriorityTableMinRangeGet(
            vtcamInfoPtr, PRV_CPSS_AVL_TREE_FIND_EQUAL_E,
            ruleOldPriority, &priorityEntry);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        /* cases when no change needed at all */
        if (ruleOldPriority == priority)
        {
            /* update without move inside the same priority */
            if ((priorityEntry.baseLogIndex == origIndexForPriorityUpdate)
                && (position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E))
            {
                /* no update needed */
                *logIndexPtr = origIndexForPriorityUpdate;
                return GT_OK;
            }
            if (((priorityEntry.baseLogIndex + priorityEntry.rangeSize - 1) == origIndexForPriorityUpdate)
                && (position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E))
            {
                /* no update needed */
                *logIndexPtr = origIndexForPriorityUpdate;
                return GT_OK;
            }

            /* the rule in the range of the same priority at the middle */
            priorityUpdateWithoutMove = GT_FALSE;
        }

        /* case when decreasing the rule priority and the neighbour rules have */
        /* such priorities tha enough to update priority DB only               */
        if ((priorityUpdateWithoutMove == GT_TRUE) && (ruleOldPriority > priority))
        {
            rc = prvCpssDxChVirtualTcamDbPriorityTableMinRangeGet(
                vtcamInfoPtr, PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                (ruleOldPriority - 1), &priorityNeighbourEntry);
            if ((rc != GT_OK) || (priorityNeighbourEntry.priority < priority))
            {
                priorityNeighbourEntry.rangeSize = 0;
                priorityNeighbourEntry.baseLogIndex = origIndexForPriorityUpdate;
                priorityNeighbourEntry.priority = priority;
            }
            if (priorityEntry.baseLogIndex != origIndexForPriorityUpdate)
            {
                priorityUpdateWithoutMove = GT_FALSE;
            }
            if (priorityNeighbourEntry.priority != priority)
            {
                priorityUpdateWithoutMove = GT_FALSE;
            }
            if ((priorityNeighbourEntry.rangeSize > 0) &&
                (position != CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E))
            {
                priorityUpdateWithoutMove = GT_FALSE;
            }
            if (priorityUpdateWithoutMove == GT_TRUE)
            {
                /* no rule move need, DB update needed */
                rc = prvCpssDxChVirtualTcamDbPriorityTableMoveNeighbourEntriesBound(
                    vTcamMngDBPtr, vtcamInfoPtr,
                    &priorityNeighbourEntry/*lowEntryPtr*/,
                    &priorityEntry/*highEntryPtr*/,
                    GT_FALSE/*moveLowToHigh*/);
                if (rc != GT_OK)
                {
                    return rc;
                }
                *logIndexPtr = origIndexForPriorityUpdate;
                return GT_OK;
            }
        }

        /* case when increasing the rule priority and the neighbour rules have */
        /* such priorities tha enough to update priority DB only               */
        if ((priorityUpdateWithoutMove == GT_TRUE) && (ruleOldPriority < priority))
        {
            rc = prvCpssDxChVirtualTcamDbPriorityTableMinRangeGet(
                vtcamInfoPtr, PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                (ruleOldPriority + 1), &priorityNeighbourEntry);
            if ((rc != GT_OK) || (priorityNeighbourEntry.priority > priority))
            {
                priorityNeighbourEntry.rangeSize = 0;
                priorityNeighbourEntry.baseLogIndex = origIndexForPriorityUpdate + 1;
                priorityNeighbourEntry.priority = priority;
            }
            if ((priorityEntry.baseLogIndex + priorityEntry.rangeSize - 1) != origIndexForPriorityUpdate)
            {
                priorityUpdateWithoutMove = GT_FALSE;
            }
            if (priorityNeighbourEntry.priority != priority)
            {
                priorityUpdateWithoutMove = GT_FALSE;
            }
            if ((priorityNeighbourEntry.rangeSize > 0) &&
                (position != CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E))
            {
                priorityUpdateWithoutMove = GT_FALSE;
            }
            if (priorityUpdateWithoutMove == GT_TRUE)
            {
                /* no rule move need, DB update needed */
                rc = prvCpssDxChVirtualTcamDbPriorityTableMoveNeighbourEntriesBound(
                    vTcamMngDBPtr, vtcamInfoPtr,
                    &priorityEntry/*lowEntryPtr*/,
                    &priorityNeighbourEntry/*highEntryPtr*/,
                    GT_TRUE/*moveLowToHigh*/);
                if (rc != GT_OK)
                {
                    return rc;
                }
                *logIndexPtr = origIndexForPriorityUpdate;
                return GT_OK;
            }
        }
        /* save Id of rule for updated priority */
        ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
        ruleNeedToFindInfo.ruleId = 0; /*don't care*/
        ruleNeedToFindInfo.logicalIndex = origIndexForPriorityUpdate;
        retVal = prvCpssAvlPathFind(
            vtcamInfoPtr->ruleIdIndexTree,
            PRV_CPSS_AVL_TREE_FIND_EQUAL_E,
            &ruleNeedToFindInfo, /* pData */
            avlTreePath,    /* path */
            &dbEntryPtr     /* use dedicated var to avoid warnings */);
        ruleIdEntryPtr = dbEntryPtr;
        if (retVal == GT_FALSE)
        {
            return rc;
        }
        priorityUpdateRuleId = ruleIdEntryPtr->ruleId;
    }

    /* find the priority in the DB */
    rc = prvCpssDxChVirtualTcamDbPriorityTableEntryBallancedGet(
        vtcamInfoPtr, priority, &priorityEntry);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_LOG_INFORMATION_MAC("BallancedGet returned : priority[%d] baseLogIndex[%d] rangeSize[%d] \n",
        priorityEntry.priority,
        priorityEntry.baseLogIndex,
        priorityEntry.rangeSize);

    if ((priorityEntry.baseLogIndex + priorityEntry.rangeSize) >
        vtcamInfoPtr->rulesAmount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: the DB hold mismatch information regarding baseLogIndex[%d] , rangeSize[%d] and rulesAmount[%d]",
            priorityEntry.baseLogIndex,
            priorityEntry.rangeSize,
            vtcamInfoPtr->rulesAmount);
    }

    if (priorityEntry.priority < priority)
    {
        /* nearest low priority found */
        logicalIndex = priorityEntry.baseLogIndex + priorityEntry.rangeSize;
        logicalIndexInLeft = GT_FALSE;
        resizeMode = RESIZE_MODE_WITH_INPUT_INDEX_E;
    }
    else if (priorityEntry.priority > priority)
    {
        /* nearest high priority found */
        logicalIndex = priorityEntry.baseLogIndex;
        logicalIndexInLeft = GT_FALSE;
        resizeMode = RESIZE_MODE_WITH_INPUT_INDEX_E;
    }
    else if (position == CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E)
    {
        /* exact priority found */
        logicalIndex = priorityEntry.baseLogIndex + priorityEntry.rangeSize;
        logicalIndexInLeft = GT_FALSE;
        resizeMode = RESIZE_MODE_WITH_INPUT_INDEX_E;
    }
    else
    {
        /* exact priority found */
        logicalIndex = priorityEntry.baseLogIndex;
        logicalIndexInLeft = GT_FALSE;
        resizeMode = RESIZE_MODE_WITH_INPUT_INDEX_E;
    }

    if ((logicalIndex >= vtcamInfoPtr->rulesAmount) && (vtcamInfoPtr->rulesAmount > 0))
    {
        /* fix if calculated out of VTCAM range */
        logicalIndex = vtcamInfoPtr->rulesAmount - 1;
        logicalIndexInLeft = GT_TRUE;
        resizeMode = RESIZE_MODE_WITHOUT_INPUT_INDEX_E;
    }

    /* find nearest to specific logical index an unused logical index */
    rc = nearestLogicalIndexEnvironmentGet(
        vtcamInfoPtr, logicalIndex, logicalIndexInLeft,
        &rightRulesSize, &rightHoleSize,
        &leftRulesSize, &leftHoleSize);
    if (rc == GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC(
            "nearestLogicalIndexEnvironmentGet call logicalIndex[%d] logicalIndexInLeft[%d] \n",
            logicalIndex, logicalIndexInLeft);
        CPSS_LOG_INFORMATION_MAC(
            "nearestLogicalIndexEnvironmentGet return rightRulesSize[%d] rightHoleSize[%d] leftRulesSize[%d] leftHoleSize[%d]\n",
            rightRulesSize, rightHoleSize, leftRulesSize, leftHoleSize);

    }
    if (rc != GT_OK)
    {
        /* No Hole - needed resize */
        if ((vtcamInfoPtr->autoResizeDisable) ||
            (vtcamInfoPtr->tcamInfo.autoResize == GT_FALSE))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
        }

        if (recalled != GT_FALSE)
        {
            /* should not occur */
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }

        /* Auto resize */
        /* resize memory for Virtual TCAM in Segment Table for TCAM.*/

        /* save guaranteedNumOfRules updated by resize function */
        saveGuaranteedSize = vtcamInfoPtr->tcamInfo.guaranteedNumOfRules;
        rc = vtcamDbSegmentTableVTcamResize(
            vTcamMngId ,vTcamId,
            sizeIncrement,
            resizeMode,
            logicalIndex);
        if (VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr) != NULL)
        {
            cpssOsFree(VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr));
            VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr) = NULL;
            VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr) = 0;
        }
        if (rc != GT_OK)
        {
            /* retry with minimal size increment == 1 */
            rc = vtcamDbSegmentTableVTcamResize(
                vTcamMngId ,vTcamId,
                1/*sizeIncrement*/,
                resizeMode,
                logicalIndex);
            if (VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr) != NULL)
            {
                cpssOsFree(VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr));
                VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr) = NULL;
                VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr) = 0;
            }
        }
        /* restore guaranteedNumOfRules - should remain as was */
        vtcamInfoPtr->tcamInfo.guaranteedNumOfRules = saveGuaranteedSize;
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
        }

        /* recursive call after resize */
        return prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1Get(
            vTcamMngId, vTcamId,
            origIndexForPriorityUpdate,
            vTcamMaxSize, hwContextPtr,
            priority, position, GT_TRUE /*recalled*/,
            sizeIncrement, logIndexPtr);
    }

    /* assumed rc == GT_OK */

    /* cases when rules move not needed */
    preparedLogicalIndex = 0xFFFFFFFF; /* recalculated */
    if ((leftRulesSize == 0) && (rightRulesSize == 0))
    {
        preparedLogicalIndex = logicalIndex;
    }
    else if ((leftRulesSize == 0) && (leftHoleSize != 0) && (logicalIndexInLeft == GT_FALSE))
    {
        preparedLogicalIndex = logicalIndex - 1;
    }
    else if ((rightRulesSize == 0) && (rightHoleSize != 0) && (logicalIndexInLeft != GT_FALSE))
    {
        preparedLogicalIndex = logicalIndex + 1;
    }
    if (preparedLogicalIndex != 0xFFFFFFFF)
    {
        GT_BOOL updated;
        GT_U32  prioHighBound;
        GT_BOOL placeFromHigh;

        updated       = GT_FALSE;

        if (priorityEntry.priority != priority)
        {
            /* the found priority entry was neighbour */
            /* prepare entry to be updated below      */
            priorityEntry.priority     = priority;
            priorityEntry.baseLogIndex = preparedLogicalIndex;
            priorityEntry.rangeSize    = 1;
            updated                    = GT_TRUE;
            placeFromHigh              = GT_TRUE;
        }

        if (priorityEntry.baseLogIndex > preparedLogicalIndex)
        {
            priorityEntry.rangeSize    += (priorityEntry.baseLogIndex - preparedLogicalIndex);
            priorityEntry.baseLogIndex  = preparedLogicalIndex;
            updated                     = GT_TRUE;
            placeFromHigh               = GT_FALSE;
        }
        prioHighBound = priorityEntry.baseLogIndex + priorityEntry.rangeSize;
        if (prioHighBound <= preparedLogicalIndex)
        {
            priorityEntry.rangeSize    += (preparedLogicalIndex - prioHighBound + 1);
            updated                     = GT_TRUE;
            placeFromHigh               = GT_TRUE;
        }
        if (updated != GT_FALSE)
        {
            rc = prvCpssDxChVirtualTcamDbPriorityTableEntryNeighbourAdjust(
                vTcamMngDBPtr, vtcamInfoPtr, priority, placeFromHigh);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDxChVirtualTcamDbPriorityTableMaxRangeGet(
                vtcamInfoPtr, priority, &priorityMaxRange);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            CPSS_LOG_INFORMATION_MAC(
                "prvCpssDxChVirtualTcamDbPriorityTableMaxRangeGet returned base[%d] size[%d] priority[%d]\n",
                priorityMaxRange.baseLogIndex, priorityMaxRange.rangeSize, priorityMaxRange.priority);
            CPSS_LOG_INFORMATION_MAC(
                "updated priority entry base[%d] size[%d] priority[%d]\n",
                priorityEntry.baseLogIndex, priorityEntry.rangeSize, priorityEntry.priority);

            if (priorityMaxRange.baseLogIndex > priorityEntry.baseLogIndex)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            if ((priorityMaxRange.baseLogIndex + priorityMaxRange.rangeSize)
                 < (priorityEntry.baseLogIndex + priorityEntry.rangeSize))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* updated the DB about the new range value */
            rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
                vTcamMngDBPtr,vtcamInfoPtr,&priorityEntry);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        /* the start place found free - no shift needed */
        *logIndexPtr = preparedLogicalIndex;

        if (origIndexForPriorityUpdate != ORIG_INDEX_NOT_USED_CNS)
        {
            /* Called from RulePriorityUpdate */
            prvCpssDxChVirtualTcamDbPriorityRemoveRuleFromRangeById(
                vTcamMngDBPtr, vtcamInfoPtr, priorityUpdateRuleId, preparedLogicalIndex);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
        if (VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1GetMoveTraceEnable))
        {
            prvCpssDxChVirtualTcamDbVTcamPrintRulesLogIndexesBitmap(vtcamInfoPtr);
            cpssOsPrintf("New rule place [%d] priority %d\n", preparedLogicalIndex, priority);
        }
        return GT_OK;
    }

    /* so we got a hole in the logical index range */

    /* (leftHoleSize != 0) || (rightHoleSize != 0) */
    if ((leftHoleSize != 0) &&
         ((rightHoleSize == 0) || (rightRulesSize > leftRulesSize)))
    {
        /* left shift */
        preparedLogicalIndex = logicalIndex - ((logicalIndexInLeft == GT_FALSE) ? 1 : 0);
        holeBase = (preparedLogicalIndex - leftRulesSize - leftHoleSize + 1);
        holeSize = leftHoleSize;
        rc = prvCpssDxChVirtualTcamSplitHoleLowPartSizeGet(
            vtcamInfoPtr, holeBase, holeSize, priority, preparedLogicalIndex,
            &holeLowPartSize, &closeToHoleLow);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        shiftSize        = (leftHoleSize - holeLowPartSize);
        if (shiftSize == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        freeLogicalIndex = preparedLogicalIndex - leftRulesSize + 1 - shiftSize;
        newRuleLogicalIndex =
            preparedLogicalIndex - ((shiftSize - 1) * (100 - closeToHoleLow) / 100);
    }
    else
    {
        /* right shift */
        preparedLogicalIndex = logicalIndex + ((logicalIndexInLeft == GT_FALSE) ? 0 : 1);
        holeBase = (preparedLogicalIndex + rightRulesSize);
        holeSize = rightHoleSize;
        rc = prvCpssDxChVirtualTcamSplitHoleLowPartSizeGet(
            vtcamInfoPtr, holeBase, holeSize, priority, preparedLogicalIndex,
            &holeLowPartSize, &closeToHoleLow);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        shiftSize        = holeLowPartSize;
        if (shiftSize == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        freeLogicalIndex = preparedLogicalIndex + rightRulesSize - 1 + shiftSize;
        newRuleLogicalIndex =
            preparedLogicalIndex + ((shiftSize - 1) * closeToHoleLow / 100);
    }

    numOfMovedRules = ((freeLogicalIndex < preparedLogicalIndex) ? leftRulesSize : rightRulesSize);

    CPSS_LOG_INFORMATION_MAC(
        "preparedLogicalIndex[%d] shiftSize[%d] newRuleLogicalIndex[%d] freeLogicalIndex[%d] numOfMovedRules[%d]\n",
        preparedLogicalIndex, shiftSize, newRuleLogicalIndex, freeLogicalIndex, numOfMovedRules);

    if (VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1GetMoveTraceEnable))
    {
        prvCpssDxChVirtualTcamDbVTcamPrintRulesLogIndexesBitmap(vtcamInfoPtr);
        cpssOsPrintf("holeBase %d holeSize %d holeLowPartSize %d\n", holeBase, holeSize, holeLowPartSize);
        if (freeLogicalIndex < preparedLogicalIndex)
        {
            cpssOsPrintf(
                "rules %d-%d ==> %d, ",
                (freeLogicalIndex + shiftSize), preparedLogicalIndex, freeLogicalIndex);
        }
        else
        {
            cpssOsPrintf(
                "rules %d-%d ==> %d, ",
                preparedLogicalIndex, (freeLogicalIndex - shiftSize), (preparedLogicalIndex + shiftSize));
        }
        cpssOsPrintf("new rule %d priority %d\n", newRuleLogicalIndex, priority);
    }

    if (freeLogicalIndex >= vtcamInfoPtr->rulesAmount)
    {
        /* wrong calculations */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: wrong calculations : freeLogicalIndex[%d] >= vtcamInfoPtr->rulesAmount[%d]",
            freeLogicalIndex,vtcamInfoPtr->rulesAmount);
    }

    /* we need to move all logical indexes from current logical index to the
       free logical index */
    if (freeLogicalIndex < preparedLogicalIndex)
    {
        for (ii = freeLogicalIndex; ((ii + shiftSize) <= preparedLogicalIndex); ii++)
        {
            /* call the CB function to move each entry at a time */
            rc = hwContextPtr->ruleMoveFuncPtr(
                hwContextPtr->cookiePtr,
                (ii + shiftSize),/*srcRuleLogicalIndex*/
                ii,/*dstRuleLogicalIndex*/
                vtcamInfoPtr->tcamInfo.ruleSize,
                GT_TRUE/*move*/);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
            }
        }

        /* copy the bits in the DB of vtcamInfoPtr->usedRulesBitmapArr */
        copyBitsInMemory(
            vtcamInfoPtr->usedRulesBitmapArr,/*memPtr*/
            freeLogicalIndex,/*targetStartBit*/
            (freeLogicalIndex + shiftSize), /*sourceStartBit*/
            (preparedLogicalIndex - (freeLogicalIndex + shiftSize) + 1));/*numBits*/
        resetBitsInMemory(
            vtcamInfoPtr->usedRulesBitmapArr,/*memPtr*/
            (preparedLogicalIndex - shiftSize + 1)/*startBit*/,
            shiftSize /*numBits*/);
    }
    else if (freeLogicalIndex > preparedLogicalIndex)
    {
        for (ii = freeLogicalIndex; (ii >= (preparedLogicalIndex + shiftSize)); ii--)
        {
            /* call the CB function to move each entry at a time */
            rc = hwContextPtr->ruleMoveFuncPtr(
                hwContextPtr->cookiePtr,
                (ii - shiftSize),/*srcRuleLogicalIndex*/
                ii,/*dstRuleLogicalIndex*/
                vtcamInfoPtr->tcamInfo.ruleSize,
                GT_TRUE/*move*/);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
            }
        }

        /* copy the bits in the DB of vtcamInfoPtr->usedRulesBitmapArr */
        copyBitsInMemory(
            vtcamInfoPtr->usedRulesBitmapArr,/*memPtr*/
            (preparedLogicalIndex + shiftSize),/*targetStartBit*/
            preparedLogicalIndex, /*sourceStartBit*/
            (freeLogicalIndex - shiftSize + 1 - preparedLogicalIndex));/*numBits*/
        resetBitsInMemory(
            vtcamInfoPtr->usedRulesBitmapArr,/*memPtr*/
            preparedLogicalIndex/*startBit*/,
            shiftSize /*numBits*/);
    }
    else /* freeLogicalIndex == logicalIndex */
    {
        /*do nothing more*/
    }

    /* update the priority DB about the removed index , that will move the ranges of them */
    {
        GT_U32 oldBaseLogicalIndex;
        GT_U32 newBaseLogicalIndex;

        if (freeLogicalIndex < preparedLogicalIndex)
        {
            oldBaseLogicalIndex = preparedLogicalIndex - numOfMovedRules + 1;
            newBaseLogicalIndex = freeLogicalIndex;
        }
        else
        {
            oldBaseLogicalIndex = preparedLogicalIndex;
            newBaseLogicalIndex = freeLogicalIndex - numOfMovedRules + 1;
        }

        CPSS_LOG_INFORMATION_MAC(
            "priorityDbLogicalIndexRangeMove call oldBaseLogicalIndex[%d] newBaseLogicalIndex[%d] numOfMovedRules[%d]\n",
            oldBaseLogicalIndex, newBaseLogicalIndex, numOfMovedRules);

        rc = priorityDbLogicalIndexRangeMove(
            vTcamMngDBPtr, vtcamInfoPtr, numOfMovedRules /*numOfRules*/,
            oldBaseLogicalIndex, newBaseLogicalIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    /* update the rules DB about the removed index , that will change logical indexes*/
    {
        GT_BOOL lowToHighDirection;
        GT_U32  startLogicalIndex;

        if (preparedLogicalIndex < freeLogicalIndex)
        {
           lowToHighDirection = GT_TRUE;
           startLogicalIndex  = (preparedLogicalIndex + numOfMovedRules - 1);
        }
        else
        {
            lowToHighDirection = GT_FALSE;
            startLogicalIndex  = (preparedLogicalIndex - numOfMovedRules + 1);
        }

        CPSS_LOG_INFORMATION_MAC(
            "rulesDbLogicalIndexRangeShift call startLogicalIndex[%d] numOfMovedRules[%d] lowToHighDirection[%d] shiftSize[%d]\n",
            startLogicalIndex, numOfMovedRules, lowToHighDirection, shiftSize);

        rc = rulesDbLogicalIndexRangeShift(
            vtcamInfoPtr,
            startLogicalIndex,
            numOfMovedRules  /*numOfRules*/,
            lowToHighDirection,
            shiftSize  /*shiftValue*/);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    /* get the priority again from the DB ... because it may have changed */
    rc = prvCpssDxChVirtualTcamDbPriorityTableEntryGet(
        vtcamInfoPtr,priority,&priorityEntry);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }
    if (priorityEntry.priority != priority)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (priorityEntry.rangeSize == 0)
    {
        priorityEntry.rangeSize = 1;
        priorityEntry.baseLogIndex = newRuleLogicalIndex;
    }
    else if (newRuleLogicalIndex < priorityEntry.baseLogIndex)
    {
        priorityEntry.rangeSize += (priorityEntry.baseLogIndex - newRuleLogicalIndex);
        priorityEntry.baseLogIndex = newRuleLogicalIndex;
    }
    else if (newRuleLogicalIndex >= (priorityEntry.baseLogIndex + priorityEntry.rangeSize))
    {
        priorityEntry.rangeSize += (newRuleLogicalIndex - (priorityEntry.baseLogIndex + priorityEntry.rangeSize) + 1);
    }
    else
    {
        /* no change needed */
        rc = GT_NO_CHANGE;
    }

    if (rc == GT_OK)
    {
        rc = prvCpssDxChVirtualTcamDbPriorityTableMaxRangeGet(
            vtcamInfoPtr, priority, &priorityMaxRange);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        if (priorityMaxRange.baseLogIndex > priorityEntry.baseLogIndex)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if ((priorityMaxRange.baseLogIndex + priorityMaxRange.rangeSize)
             < (priorityEntry.baseLogIndex + priorityEntry.rangeSize))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        CPSS_LOG_INFORMATION_MAC(
            "prvCpssDxChVirtualTcamDbPriorityTableEntrySet call base[%d] size[%d] priority[%d]\n",
            priorityEntry.baseLogIndex, priorityEntry.rangeSize, priorityEntry.priority);
        /* updated the DB about the new range value */
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
            vTcamMngDBPtr,vtcamInfoPtr,&priorityEntry);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    /* the freeLogicalIndex got the 'shift' so we actually state to caller that
       'logicalIndex' is actually free now !!! */
    *logIndexPtr = newRuleLogicalIndex;

    if (origIndexForPriorityUpdate != ORIG_INDEX_NOT_USED_CNS)
    {
        /* Called from RulePriorityUpdate */
        prvCpssDxChVirtualTcamDbPriorityRemoveRuleFromRangeById(
            vTcamMngDBPtr, vtcamInfoPtr, priorityUpdateRuleId, newRuleLogicalIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert function
* @endinternal
*
* @brief   convert rule Id to logical index and to physical index.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   -  to find.
* @param[in] checkExistance           - indication if need to check that the ruleId already
*                                      exists in the DB.
*
* @param[out] logicalIndexPtr          - (pointer to) the logical index.  (can be NULL)
* @param[out] physicalIndexPtr         - (pointer to) the physical index. (can be NULL)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN   GT_BOOL                                    checkExistance,
    OUT  GT_U32                                     *logicalIndexPtr,
    OUT  GT_U32                                     *physicalIndexPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC ruleIdEntry;
    GT_U32 logicalIndex;

    /* wrong values to be overriden below */
    if (logicalIndexPtr)  *logicalIndexPtr  = 0xFFFFFFFF;
    if (physicalIndexPtr) *physicalIndexPtr = 0xFFFFFFFF;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"vTcamMngId[%d] it out of valid range[0..%d] \n",
            vTcamMngId,
            CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS);
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"vTcamMngId[%d] was not created \n",
            vTcamMngId);
    }

    if (vTcamMngDBPtr->vTcamCfgPtrArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"vTcamId[%d] was not created \n",
            vTcamId);
    }

    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E)
    {
        logicalIndex = ruleId;

        if(logicalIndex >= vtcamInfoPtr->rulesAmount)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"ruleId[%d] it out of valid range[0..%d] \n",
                ruleId,
                vtcamInfoPtr->rulesAmount-1);
        }
    }
    else /*CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E*/
    {
        /* convert the ruleId to logical index */
        rc = prvCpssDxChVirtualTcamDbRuleIdTableEntryGet(vtcamInfoPtr, ruleId, &ruleIdEntry);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"The ruleId[%d] was not created",
                ruleId);
        }

        logicalIndex = ruleIdEntry.logicalIndex;

        if(logicalIndex >= vtcamInfoPtr->rulesAmount)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            /* error in DB management */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error:logicalIndex[%d] >= vtcamInfoPtr->rulesAmount[%d] ",
                logicalIndex ,
                vtcamInfoPtr->rulesAmount);
        }
    }

    if (vtcamInfoPtr->rulePhysicalIndexArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        /* error in DB management */
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_FAIL, "DB error: vtcamInfoPtr->rulePhysicalIndexArr == NULL");
    }

    CPSS_LOG_INFORMATION_MAC("converted rule[%d] to logical index[%d] and HW index[%d] \n",
        ruleId,
        logicalIndex,
        vtcamInfoPtr->rulePhysicalIndexArr[logicalIndex]);

    if(checkExistance == GT_TRUE)
    {
        /* check if the rule exists in the DB  */
        if(0 == (vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] & (1<<(logicalIndex & 0x1f))))
        {
            /* the rule not exists in the DB */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,"the logicalIndex[%d] of rule[%d] not exists in the DB",
                logicalIndex,
                ruleId);
        }
    }

    /* convert the logical index to physical index in the HW */
    if(physicalIndexPtr)
    {
        *physicalIndexPtr = vtcamInfoPtr->rulePhysicalIndexArr[logicalIndex];
    }

    if(logicalIndexPtr)
    {
        *logicalIndexPtr = logicalIndex;
    }

    return GT_OK;
}

/* cookie for callback_virtualTcamDbRuleIdInvalidate(...) */
/* cookie for callback_virtualTcamDbRuleIdMove(...)       */
typedef struct{
    GT_U32  vTcamMngId;
    GT_U32  vTcamId;
}CALLBACK_COOKIE_INFO_STC;

/*
 * Typedef: function PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_RULE_INVALIDATE_FUNC
 *
 * Description:
 *     This Function invalidates rule in TCAM.
 *
 * INPUTS:
 *       cookiePtr            - device info passed to callback function.
 *       ruleLogicalIndex     - rule logial index.
 *       ruleSize             - size of rules used in virtual TCAM.
 *
 *  Comments:
 *
 */
static GT_STATUS callback_virtualTcamDbRuleIdInvalidate
(
    IN  GT_VOID                                   *cookiePtr,
    IN  GT_U32                                    ruleLogicalIndex,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT      ruleSize
)
{
    GT_STATUS                           rc;
    GT_U8                               devNum;
    CALLBACK_COOKIE_INFO_STC*           myCookiePtr = cookiePtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *vTcamDBPtr;

    if(myCookiePtr->vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[myCookiePtr->vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"vTcamMng  was not created \n");
    }

    vTcamMngDBPtr->ruleInvalidateCounter ++;

    vTcamDBPtr = prvCpssDxChVirtualTcamDbVTcamGet(
                    myCookiePtr->vTcamMngId,
                    myCookiePtr->vTcamId);
    if(vTcamDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"vTcam  was not created \n");
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* iteration over the devices */
    do
    {
        /* perDevice_ruleInvalidate not needed device DB locking */
        /* invalidate the rule in the device */
        rc = perDevice_ruleInvalidate(
            devNum, vTcamMngDBPtr,
            vTcamMngDBPtr->portGroupBmpArr[devNum],
            ruleLogicalIndex,
            vTcamDBPtr->tcamSegCfgPtr->deviceClass,
            ruleSize);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));

    return GT_OK;
}

/*
 * Typedef: function PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_RULE_MOVE_FUNC
 *
 * Description:
 *     This Function move/copy rule in TCAM.
 *
 * INPUTS:
 *       cookiePtr               - device info passed to callback function.
 *       srcRuleLogicalIndex     - source rule logical index.
 *       dstRuleLogicalIndex     - destination rule logical index.
 *       ruleSize                - size of rules used in virtual TCAM.
 *
 *  Comments:
 *
 */
static GT_STATUS callback_virtualTcamDbRuleIdMove
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      srcRuleLogicalIndex,
    IN  GT_U32      dstRuleLogicalIndex,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT      ruleSize,
    IN  GT_BOOL     moveOrCopy
)
{
    GT_STATUS   rc;
    GT_U8      devNum;
    CALLBACK_COOKIE_INFO_STC*  myCookiePtr = cookiePtr;
    GT_U32  srcDeviceRuleIndex,trgDeviceRuleIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;

    CPSS_LOG_INFORMATION_MAC("Move Rule from logical index[%d] to [%d] size[%d] \n",
        srcRuleLogicalIndex ,  dstRuleLogicalIndex,  ruleSize);

    if(myCookiePtr->vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[myCookiePtr->vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(
        myCookiePtr->vTcamMngId,myCookiePtr->vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* convert the logical index to physical index in the HW */
    srcDeviceRuleIndex = vtcamInfoPtr->rulePhysicalIndexArr[srcRuleLogicalIndex];
    trgDeviceRuleIndex = vtcamInfoPtr->rulePhysicalIndexArr[dstRuleLogicalIndex];

    /* iteration over the devices */
    do
    {
        if (devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
        {
            /* should never occur */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /* move the rule in the device */
        rc = perDevice_ruleMove(
            devNum,
            vTcamMngDBPtr,
            vTcamMngDBPtr->portGroupBmpArr[devNum],
            vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
            ruleSize,
            srcDeviceRuleIndex,
            trgDeviceRuleIndex,
            moveOrCopy);
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));

    return GT_OK;
}

void prvCpssDxChVirtualTcamPriorityAddNewRuleNeedNewLogicalIndexFuncionVersionSet(GT_U32 version)
{
    VTCAM_GLOVAR(priorityAddNewRuleNeedNewLogicalIndexFuncionVersion) = version;
}

/**
* @internal priorityAddNewRuleNeedNewLogicalIndex function
* @endinternal
*
* @brief   Get place for new rule.
*         If needed move existing rules.
*         If needed resize vTCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] origIndexForPriorityUpdate - the original index of the entry that doing
* @param[in] priority                 update'
*                                      value ORIG_INDEX_NOT_USED_CNS meaning that it is
*                                      not relevant.
* @param[in] priority                 - new rule priority
* @param[in] position                 -  in range of rule of the
*                                      specified priority (see enum)
*
* @param[out] logIndexPtr              - (pointer to) Logical Index of new rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_FULL                  - on table full (no free logical index)
*/
static GT_STATUS priorityAddNewRuleNeedNewLogicalIndex
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN   GT_U32                                                    origIndexForPriorityUpdate ,
    IN   GT_U32                                                    priority,
    IN   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT   position,
    OUT  GT_U32                                                    *logIndexPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_TAB_HW_CONTEXT_STC hwContext;
    CALLBACK_COOKIE_INFO_STC cookieInfo;
    GT_U32 logicalIndex;
    GT_U32 autoResizeAddition;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    autoResizeAddition = 0;
    if ((vtcamInfoPtr->autoResizeDisable == GT_FALSE) &&
        (vtcamInfoPtr->tcamInfo.autoResize != GT_FALSE))
    {
        autoResizeAddition =
            prvCpssDxChVirtualTcamDbSegmentTableResizeGranularityGet(
                vtcamInfoPtr->tcamSegCfgPtr, vtcamInfoPtr->tcamInfo.ruleSize);
    }

    cookieInfo.vTcamMngId = vTcamMngId;
    cookieInfo.vTcamId = vTcamId;

    /* build the call back info */
    hwContext.cookiePtr = &cookieInfo;
    hwContext.ruleInvalidateFuncPtr = callback_virtualTcamDbRuleIdInvalidate;
    hwContext.ruleMoveFuncPtr = callback_virtualTcamDbRuleIdMove;

    /* Get logical index for the new rule .
       Get place for new rule.
       If needed move existing rules.
       If needed resize vTCAM.
    */
    if (VTCAM_GLOVAR(priorityAddNewRuleNeedNewLogicalIndexFuncionVersion) == 0)
    {
        rc = prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexGet(
            vTcamMngId, vTcamId,
            origIndexForPriorityUpdate,
            (vtcamInfoPtr->rulesAmount + autoResizeAddition),
            &hwContext,
            priority,
            position,
            autoResizeAddition,/*sizeIncrement,*/
            GT_FALSE /*recalled*/,
            &logicalIndex
        );
    }
    else
    {
        rc = prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexVer1Get(
            vTcamMngId, vTcamId,
            origIndexForPriorityUpdate,
            (vtcamInfoPtr->rulesAmount + autoResizeAddition),
            &hwContext,
            priority,
            position,
            autoResizeAddition,/*sizeIncrement,*/
            GT_FALSE /*recalled*/,
            &logicalIndex
        );
    }

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    *logIndexPtr = logicalIndex;

    return GT_OK;
}

/**
* @internal priorityRuleWritePreparations function
* @endinternal
*
* @brief   make preparations before adding new rule in priority mode.
*         the place for new rule may require to :
*         1. move existing rules.
*         2. resize vTCAM.
*         3. close holes in the tcam.
*         NOTE: the actual 'new rule' addition is not done in this API
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] vtcamInfoPtr             - (pointer to) virtual TCAM entry
* @param[in] ruleId                   -  to find.
* @param[in] priority                 - priority.
*
* @param[out] logicalIndexPtr          - (pointer to) the logical index of the rule
* @param[out] isNotExistPtr            - (pointer to) indication that the ruleId does not exist in DB
*                                      prior to this action.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS priorityRuleWritePreparations
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN  GT_U32                                      priority,
    OUT  GT_U32                                     *logicalIndexPtr,
    OUT GT_BOOL                                     *isNotExistPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC ruleIdEntry;
    GT_U32 logicalIndex;
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT position;

    *isNotExistPtr = GT_TRUE;

    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E)
    {
        /* the function is not for this mode */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    /* convert the ruleId to logical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdTableEntryGet(vtcamInfoPtr,ruleId, &ruleIdEntry);
    if(rc == GT_OK)/* already exists ... has logical index */
    {
        CPSS_LOG_INFORMATION_MAC("ruleId[%d] already exists with logical index[%d] \n",
            ruleId,
            ruleIdEntry.logicalIndex);

        *isNotExistPtr = GT_FALSE;

        logicalIndex = ruleIdEntry.logicalIndex;
        if(logicalIndex >= vtcamInfoPtr->rulesAmount)
        {
            /* error in DB management */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error:logicalIndex[%d] >= vtcamInfoPtr->rulesAmount[%d] ",
                logicalIndex , vtcamInfoPtr->rulesAmount);
        }

        /* check if the rule was already in the DB  */
        if(0 == (vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] & (1<<(logicalIndex & 0x1f))))
        {
            /* error in DB management */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error:logicalIndex[%d] not found in usedRulesBitmapArr[]",
                logicalIndex);
        }

        /* need to check if the priority in the DB is the same as the given one */
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntryFind(
            vtcamInfoPtr,
            logicalIndex,
            PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,/*findType*/
            avlTreePath,
            &priorityEntry
        );
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }

        if(priorityEntry.priority != priority)
        {
            /* mismatch */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"the priority[%d] must be the same as the DB priority[%d]",
                priority,priorityEntry.priority);
        }

        if(priorityEntry.baseLogIndex > logicalIndex ||
           (priorityEntry.baseLogIndex + priorityEntry.rangeSize) <= logicalIndex )
        {

            /* 'management' error */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: the DB hold mismatch information regarding baseLogIndex[%d] , rangeSize[%d] and logicalIndex[%d]",
                priorityEntry.baseLogIndex,
                priorityEntry.rangeSize,
                logicalIndex);
        }

    }
    else
    if(rc == GT_NOT_FOUND) /* not found .. need new logical index */
    {
        CPSS_LOG_INFORMATION_MAC("ruleId[%d] not found .. need new logical index \n",
            ruleId);

         *isNotExistPtr = GT_TRUE;

         if (vtcamInfoPtr->usedRulesAmount > vtcamInfoPtr->rulesAmount)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: vtcam [%d] : usedRulesAmount[%d] > rulesAmount[%d]",
                 vTcamId,
                 vtcamInfoPtr->usedRulesAmount,
                 vtcamInfoPtr->rulesAmount);
         }

         if (vtcamInfoPtr->usedRulesAmount == vtcamInfoPtr->rulesAmount)
         {
             if ((vtcamInfoPtr->autoResizeDisable) ||
                 (vtcamInfoPtr->tcamInfo.autoResize == GT_FALSE))
             {
                 /* the vtcam already full without auto-resize option */
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL,"vtcam [%d] is full and without auto-resize option for new rule in priority[%d]",
                     vTcamId,priority);
             }
         }

         if (GT_TRUE == VTCAM_GLOVAR(debug_writeRuleForceFirstPosition_enable))
         {
            /* debug mode to allow quick testing of the 'position first' that
               the 'rule Update' API allow to use. */
            position = CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E;
         }
         else
         {
            position = CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E;
         }

        /* Get logical index for the new rule .
           Get place for new rule.
           If needed move existing rules.
           If needed resize vTCAM.
        */
        rc = priorityAddNewRuleNeedNewLogicalIndex(
            vTcamMngId,
            vTcamId,
            ORIG_INDEX_NOT_USED_CNS,/*origIndexForPriorityUpdate*/
            priority,
            position,/*position*/
            &logicalIndex
        );
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }

        CPSS_LOG_INFORMATION_MAC("ruleId[%d] got new logical index[%d] \n",
            ruleId,
            logicalIndex);

    }
    else
    {
        /* error */
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    *logicalIndexPtr = logicalIndex;
    return GT_OK;
}

/**
* @internal priorityNewRuleIdAddedDbUpdate function
* @endinternal
*
* @brief   priority mode : update the DB about new rule ID that was added.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   -  to find.
* @param[in] logicalIndex             - logical index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on ruleId or/and logicalIndex already used
* @retval GT_FAIL                  - on error
*/
static GT_STATUS priorityNewRuleIdAddedDbUpdate
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN  GT_U32                                      logicalIndex
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC ruleIdEntry;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&ruleIdEntry,0,sizeof(ruleIdEntry));

    ruleIdEntry.ruleId = ruleId;
    ruleIdEntry.logicalIndex = logicalIndex;

    CPSS_LOG_INFORMATION_MAC("Add Rule to DB : ruleId[%d] , logicalIndex[%d] \n", ruleId, logicalIndex);
    rc =  prvCpssDxChVirtualTcamDbRuleIdTableEntryAdd(vTcamMngDBPtr,
        vtcamInfoPtr,&ruleIdEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* NOTE: the priority DB was updated during
     prvCpssDxChVirtualTcamDbUtilNewRuleLogIndexGet(...) */


    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntrySet function
* @endinternal
*
* @brief   Add or Update Logical Index Range to Priority Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*
* @note 1. The given range should not overlap any existing range.
*       2. if priority0 >= priority1 then baseLogIndex0 >= baseLogIndex1
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntrySet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC           *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *tcamCfgPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *entryPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    * priorityEntryPtr;

    /* find priority entry in AVL tree */
    priorityEntryPtr = prvCpssAvlSearch(tcamCfgPtr->priorityPriTree, entryPtr);
    /* priority entry not found */
    if (priorityEntryPtr == NULL)
    {
        if (entryPtr->rangeSize == 0)
        {
            /* not found and not needed */
            return GT_OK;
        }
        /* get free buffer from priority pool.*/
        priorityEntryPtr = cpssBmPoolBufGet(pVTcamMngPtr->priorityPool);
        if (priorityEntryPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"cpssBmPoolBufGet: error getting free buffer from pool");
        }
    }
    else
    if ((entryPtr->rangeSize != 0) &&
        (entryPtr->priority == priorityEntryPtr->priority) &&
        (entryPtr->baseLogIndex == priorityEntryPtr->baseLogIndex))
    {
        /* None of the key fields have changed, and there is no need to remove:
         *  - update entry in place in order to save tree manipulations */
        priorityEntryPtr->rangeSize = entryPtr->rangeSize;
        /* modified and a new not needed */
        return GT_OK;
    }
    else
    {
        /* delete entry from priority tree */
        prvCpssAvlItemRemove(tcamCfgPtr->priorityPriTree, priorityEntryPtr);
        /* delete entry from priority index tree */
        prvCpssAvlItemRemove(tcamCfgPtr->priorityIndexTree, priorityEntryPtr);
        if (entryPtr->rangeSize == 0)
        {
            /* free a buffer back to its pool */
            rc = cpssBmPoolBufFree(pVTcamMngPtr->priorityPool, priorityEntryPtr);
            if (rc != GT_OK)
            {
                return rc;
            }
            /* freed and a new not needed */
            return GT_OK;
        }
    }

    cpssOsMemCpy(priorityEntryPtr, entryPtr,
                 sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC));

    /* insert new logical index range into priority tree */
    rc = prvCpssAvlItemInsert(tcamCfgPtr->priorityPriTree, priorityEntryPtr);
    if (rc != GT_OK)
    {
        cpssBmPoolBufFree(pVTcamMngPtr->priorityPool, priorityEntryPtr);
        return rc;
    }

    /* insert new logical index range into priority index tree */
    rc = prvCpssAvlItemInsert(tcamCfgPtr->priorityIndexTree, priorityEntryPtr);
    if (rc != GT_OK)
    {
        prvCpssAvlItemRemove(tcamCfgPtr->priorityPriTree, priorityEntryPtr);
        cpssBmPoolBufFree(pVTcamMngPtr->priorityPool, priorityEntryPtr);

        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntryGet function
* @endinternal
*
* @brief   Get Logical Index Range from Priority Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
*
* @note For not existing range returned rangeSizePtr == 0 and
*       baseLogIndexPtr points to the end of the last range
*       that should be before the required range, i.e. the range with
*       minimal priority greater the given (if not exist - baseLogIndexPtr == 0)
*       This value can be used as base of this not existing range adding it.
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntryGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *tcamCfgPtr,
    IN   GT_U32                                       priority,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *entryPtr
)
{
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry, * priorityEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL retVal;

    /* set priority to start find in tree */
    priorityEntry.priority = priority;
    priorityEntry.baseLogIndex = 0; /* don't care */
    priorityEntry.rangeSize = 0;    /* don't care */

    retVal = prvCpssAvlPathFind(tcamCfgPtr->priorityPriTree,
                                PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                                &priorityEntry, /* pData */
                                avlTreePath,    /* path */
                                &dbEntryPtr     /* use dedicated var to avoid warnings */);
    if (retVal == GT_TRUE)
    {
        priorityEntryPtr = dbEntryPtr;
        /* exact match */
        if (priorityEntryPtr->priority == priority)
        {
            /* copy to output entry  */
            cpssOsMemCpy(entryPtr, priorityEntryPtr,
                         sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC));
        }
        else
        {
            /* maximal less */
            entryPtr->priority = priority;
            /* calculate base logical index for new priority */
            entryPtr->baseLogIndex = priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize;
            entryPtr->rangeSize = 0;
        }
    }
    else
    {
        /* not found - required less than all present */
        entryPtr->priority = priority;
        entryPtr->baseLogIndex = 0;
        entryPtr->rangeSize = 0;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityRemoveRuleFromRangeById function
* @endinternal
*
* @brief   Removes rule from priority entry if first or last.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in]  tcamInfoPtr              - (pointer to)virtual TCAM structure
* @param[in]  ruleId                   - rile id.
* @param[in]  reservedLogIndex         - reserved Logical Index for yet not copied rule - do not remove.
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityRemoveRuleFromRangeById
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC           *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *tcamCfgPtr,
    IN   GT_U32                                       ruleId,
    IN   GT_U32                                       reservedLogIndex
)
{
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *priorityEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL retVal;
    GT_U32  ruleLogicalIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     ruleNeedToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     *ruleIdEntryPtr;
    GT_U32  lowBound, highBound;

    ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
    ruleNeedToFindInfo.ruleId = ruleId;
    ruleNeedToFindInfo.logicalIndex = 0; /*don't care*/
    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->ruleIdIdTree,
        PRV_CPSS_AVL_TREE_FIND_EQUAL_E,
        &ruleNeedToFindInfo, /* pData */
        avlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    ruleIdEntryPtr = dbEntryPtr;
    if (retVal == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    ruleLogicalIndex = ruleIdEntryPtr->logicalIndex;

    priorityEntry.priority = 0; /* don't care */
    priorityEntry.baseLogIndex = ruleLogicalIndex;
    priorityEntry.rangeSize = 0;    /* don't care */
    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->priorityIndexTree,
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
        &priorityEntry, /* pData */
        avlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    if (retVal == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    priorityEntryPtr = dbEntryPtr;

    /* assumed priorityEntryPtr->baseLogIndex <= ruleLogicalIndex as search condition */
    if ((priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize) <= ruleLogicalIndex)
    {
        /* found priority entry does not contain the rule position */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    lowBound = priorityEntryPtr->baseLogIndex;

    /* adjust low bound to the nearest existing rule greater or equal */
    if (lowBound < ruleLogicalIndex)
    {
        ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
        ruleNeedToFindInfo.ruleId = 0; /*don't care*/
        ruleNeedToFindInfo.logicalIndex = lowBound;
        retVal = prvCpssAvlPathFind(
            tcamCfgPtr->ruleIdIndexTree,
            PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            &ruleNeedToFindInfo, /* pData */
            avlTreePath,    /* path */
            &dbEntryPtr     /* use dedicated var to avoid warnings */);
        ruleIdEntryPtr = dbEntryPtr;
        if ((retVal == GT_FALSE) ||
            (ruleIdEntryPtr->logicalIndex >= (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize)))
        {
            /* should never occur */
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        lowBound = ruleIdEntryPtr->logicalIndex;
    }

    highBound = (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize - 1);

    /* adjust high bound to the nearest existing rule less or equal */
    if (highBound > ruleLogicalIndex)
    {
        ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
        ruleNeedToFindInfo.ruleId = 0; /*don't care*/
        ruleNeedToFindInfo.logicalIndex = highBound;
        retVal = prvCpssAvlPathFind(
            tcamCfgPtr->ruleIdIndexTree,
            PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
            &ruleNeedToFindInfo, /* pData */
            avlTreePath,    /* path */
            &dbEntryPtr     /* use dedicated var to avoid warnings */);
        ruleIdEntryPtr = dbEntryPtr;
        /* assumed found inside priority entry range after previous check */
        highBound = ruleIdEntryPtr->logicalIndex;
    }

    /* if reserved rule position was dedicated to rule priority found in DB expand the range to include it again */
    if ((reservedLogIndex >= priorityEntryPtr->baseLogIndex) &&
        (reservedLogIndex < (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize)))
    {
        if (lowBound > reservedLogIndex)
        {
            lowBound = reservedLogIndex;
        }
        if (highBound < reservedLogIndex)
        {
            highBound = reservedLogIndex;
        }
    }

    if (highBound == lowBound)
    {
        /* priority range contains one rule - the rule being removed */
        /* remove priority entry */
        /* delete entry from priority tree */
        prvCpssAvlItemRemove(tcamCfgPtr->priorityPriTree, priorityEntryPtr);
        /* delete entry from priority index tree */
        prvCpssAvlItemRemove(tcamCfgPtr->priorityIndexTree, priorityEntryPtr);
        /* put back to pool */
        cpssBmPoolBufFree(pVTcamMngPtr->priorityPool, priorityEntryPtr);
        return GT_OK;
    }

    /* if removed rule is the least in the range move lowBound to the next rule*/
    if (lowBound == ruleLogicalIndex)
    {
        ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
        ruleNeedToFindInfo.ruleId = 0; /*don't care*/
        ruleNeedToFindInfo.logicalIndex = (lowBound + 1);
        retVal = prvCpssAvlPathFind(
            tcamCfgPtr->ruleIdIndexTree,
            PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            &ruleNeedToFindInfo, /* pData */
            avlTreePath,    /* path */
            &dbEntryPtr     /* use dedicated var to avoid warnings */);
        ruleIdEntryPtr = dbEntryPtr;
        /* assumed found inside priority entry range after previous check */
        lowBound = ruleIdEntryPtr->logicalIndex;
    }

    /* if removed rule is the greatest in the range move highBound to the previous rule*/
    if (highBound == ruleLogicalIndex)
    {
        ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
        ruleNeedToFindInfo.ruleId = 0; /*don't care*/
        ruleNeedToFindInfo.logicalIndex = (highBound - 1);
        retVal = prvCpssAvlPathFind(
            tcamCfgPtr->ruleIdIndexTree,
            PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
            &ruleNeedToFindInfo, /* pData */
            avlTreePath,    /* path */
            &dbEntryPtr     /* use dedicated var to avoid warnings */);
        ruleIdEntryPtr = dbEntryPtr;
        /* assumed found inside priority entry range after previous check */
        highBound = ruleIdEntryPtr->logicalIndex;
    }

    /* if reserved rule position was dedicated to rule priority found in DB expand the range to include it again */
    if ((reservedLogIndex >= priorityEntryPtr->baseLogIndex) &&
        (reservedLogIndex < (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize)))
    {
        if (lowBound > reservedLogIndex)
        {
            lowBound = reservedLogIndex;
        }
        if (highBound < reservedLogIndex)
        {
            highBound = reservedLogIndex;
        }
    }

    /* inplace priority entry update */

    /*priorityEntryPtr->priority unchanged */
    priorityEntryPtr->baseLogIndex = lowBound;
    priorityEntryPtr->rangeSize = (highBound - lowBound + 1);

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableMaxRangeGet function
* @endinternal
*
* @brief   Get Maximal Logical Index Range for the given Priority (range between neighbour priorityes).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  tcamInfoPtr              - (pointer to)virtual TCAM structure
* @param[in]  priority                 - priority as key to find entry.
*
* @param[out] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableMaxRangeGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *tcamCfgPtr,
    IN   GT_U32                                       priority,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *entryPtr
)
{
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *priorityEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL retVal;
    GT_U32  lowBound, highBound;


    if (priority == 0)
    {
        lowBound = 0;
    }
    else
    {
        /* set priority to start find in tree */
        priorityEntry.priority = (priority > 0) ? (priority - 1) : 0;
        priorityEntry.baseLogIndex = 0; /* don't care */
        priorityEntry.rangeSize = 0;    /* don't care */
        retVal = prvCpssAvlPathFind(
            tcamCfgPtr->priorityPriTree,
            PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
            &priorityEntry, /* pData */
            avlTreePath,    /* path */
            &dbEntryPtr     /* use dedicated var to avoid warnings */);
        if (retVal == GT_FALSE)
        {
            lowBound = 0;
        }
        else
        {
            priorityEntryPtr = dbEntryPtr;
            lowBound = (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize);
        }
    }

    /* set priority to start find in tree */
    priorityEntry.priority = (priority + 1);
    priorityEntry.baseLogIndex = 0; /* don't care */
    priorityEntry.rangeSize = 0;    /* don't care */
    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->priorityPriTree,
        PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
        &priorityEntry, /* pData */
        avlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    if (retVal == GT_FALSE)
    {
        highBound = tcamCfgPtr->rulesAmount;
    }
    else
    {
        priorityEntryPtr = dbEntryPtr;
        highBound = priorityEntryPtr->baseLogIndex;
    }


    entryPtr->priority = priority;
    entryPtr->baseLogIndex = lowBound;
    entryPtr->rangeSize = (highBound - lowBound);

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableMinRangeGet function
* @endinternal
*
* @brief   Get Minimal Logical Index Range for the given Priority.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  tcamInfoPtr              - (pointer to)virtual TCAM structure
* @param[in]  priority                 - priority as key to find entry.
* @param[in]  findType                 - type of find entry by priority (EQ, LE, GE)
*
* @param[out] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableMinRangeGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *tcamCfgPtr,
    IN   PRV_CPSS_AVL_TREE_FIND_ENT                   findType,
    IN   GT_U32                                       priority,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *entryPtr
)
{
    PRV_CPSS_AVL_TREE_PATH      prioAvlTreePath;
    PRV_CPSS_AVL_TREE_PATH      ruleAvlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *priorityEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     ruleNeedToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     *ruleIdEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL retVal;
    GT_U32  lowBound, highBound;

    /* set priority to start find in tree */
    priorityEntry.priority = priority;
    priorityEntry.baseLogIndex = 0; /* don't care */
    priorityEntry.rangeSize = 0;    /* don't care */
    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->priorityPriTree,
        findType,
        &priorityEntry, /* pData */
        prioAvlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    priorityEntryPtr = dbEntryPtr;
    if (retVal == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
    ruleNeedToFindInfo.ruleId = 0; /*don't care*/
    ruleNeedToFindInfo.logicalIndex = priorityEntryPtr->baseLogIndex;
    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->ruleIdIndexTree,
        PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
        &ruleNeedToFindInfo, /* pData */
        ruleAvlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    ruleIdEntryPtr = dbEntryPtr;

    lowBound = (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize - 1);
    if ((retVal != GT_FALSE) &&
        (ruleIdEntryPtr->logicalIndex < (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize)))
    {
        lowBound = ruleIdEntryPtr->logicalIndex;
    }

    ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
    ruleNeedToFindInfo.ruleId = 0; /*don't care*/
    ruleNeedToFindInfo.logicalIndex = (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize - 1);
    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->ruleIdIndexTree,
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
        &ruleNeedToFindInfo, /* pData */
        ruleAvlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    ruleIdEntryPtr = dbEntryPtr;

    highBound = (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize - 1);
    if ((retVal != GT_FALSE) && (ruleIdEntryPtr->logicalIndex >= priorityEntryPtr->baseLogIndex))
    {
        highBound = ruleIdEntryPtr->logicalIndex;
    }

    entryPtr->priority = priorityEntryPtr->priority;
    entryPtr->baseLogIndex = lowBound;
    entryPtr->rangeSize = (highBound - lowBound + 1);

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableMoveNeighbourEntriesBound function
* @endinternal
*
* @brief   Update neighbour priority entries moving the low or High bound of one of them to the neigbour.
* @note    the function does not check validity of the given pair of neigbour entries - can corrupt the DB.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in]  tcamInfoPtr              - (pointer to)virtual TCAM structure
* @param[in]  lowEntryPtr              - (pointer to) entry with info
* @param[in]  highEntryPtr             - (pointer to) entry with info
* @param[in]  moveLowToHigh            - priority GT_TRUE - low to high, GT_FALSE high to low.
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableMoveNeighbourEntriesBound
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *tcamCfgPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *lowEntryPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *highEntryPtr,
    IN   GT_BOOL                                            moveLowToHigh
)
{
    GT_STATUS rc;
    PRV_CPSS_AVL_TREE_PATH      ruleAvlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     ruleNeedToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     *ruleIdEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL retVal;
    GT_U32  movedLogIndex;

    if (moveLowToHigh != GT_FALSE)
    {
        /* Low to High */
        movedLogIndex = (lowEntryPtr->baseLogIndex + lowEntryPtr->rangeSize - 1);
        priorityEntry.baseLogIndex = lowEntryPtr->baseLogIndex;
        priorityEntry.priority    = lowEntryPtr->priority;
        if (lowEntryPtr->rangeSize <= 1)
        {
            priorityEntry.rangeSize    = 0;
        }
        else
        {
            ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
            ruleNeedToFindInfo.ruleId = 0; /*don't care*/
            ruleNeedToFindInfo.logicalIndex = (movedLogIndex - 1);
            retVal = prvCpssAvlPathFind(
                tcamCfgPtr->ruleIdIndexTree,
                PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
                &ruleNeedToFindInfo, /* pData */
                ruleAvlTreePath,    /* path */
                &dbEntryPtr     /* use dedicated var to avoid warnings */);
            ruleIdEntryPtr = dbEntryPtr;
            if ((retVal == GT_FALSE) || (ruleIdEntryPtr->logicalIndex < lowEntryPtr->baseLogIndex))
            {
                priorityEntry.rangeSize    = 0;
            }
            else
            {
                priorityEntry.rangeSize    = (ruleIdEntryPtr->logicalIndex - lowEntryPtr->baseLogIndex + 1);
            }
        }
        /* update low entry */
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
            pVTcamMngPtr,tcamCfgPtr,&priorityEntry);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* update low entry */
        priorityEntry.baseLogIndex = movedLogIndex;
        priorityEntry.priority     = highEntryPtr->priority;
        priorityEntry.rangeSize    =
            (highEntryPtr->baseLogIndex - movedLogIndex) + highEntryPtr->rangeSize;
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
            pVTcamMngPtr,tcamCfgPtr,&priorityEntry);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* High to Low */
        movedLogIndex = highEntryPtr->baseLogIndex;
        priorityEntry.baseLogIndex = highEntryPtr->baseLogIndex;
        priorityEntry.priority    = highEntryPtr->priority;
        if (highEntryPtr->rangeSize <= 1)
        {
            priorityEntry.rangeSize    = 0;
        }
        else
        {
            ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
            ruleNeedToFindInfo.ruleId = 0; /*don't care*/
            ruleNeedToFindInfo.logicalIndex = (movedLogIndex + 1);
            retVal = prvCpssAvlPathFind(
                tcamCfgPtr->ruleIdIndexTree,
                PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
                &ruleNeedToFindInfo, /* pData */
                ruleAvlTreePath,    /* path */
                &dbEntryPtr     /* use dedicated var to avoid warnings */);
            ruleIdEntryPtr = dbEntryPtr;
            if ((retVal == GT_FALSE) ||
                (ruleIdEntryPtr->logicalIndex >= (highEntryPtr->baseLogIndex + highEntryPtr->rangeSize)))
            {
                priorityEntry.rangeSize    = 0;
            }
            else
            {
                priorityEntry.baseLogIndex = ruleIdEntryPtr->logicalIndex;
                priorityEntry.rangeSize    =
                    highEntryPtr->rangeSize - (ruleIdEntryPtr->logicalIndex - highEntryPtr->baseLogIndex);
            }
        }
        /* update low entry */
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
            pVTcamMngPtr,tcamCfgPtr,&priorityEntry);
        if (rc != GT_OK)
        {
            return rc;
        }
        /* update low entry */
        priorityEntry.baseLogIndex = lowEntryPtr->baseLogIndex;
        priorityEntry.priority     = lowEntryPtr->priority;
        priorityEntry.rangeSize    =
            (movedLogIndex - lowEntryPtr->baseLogIndex + 1);
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
            pVTcamMngPtr,tcamCfgPtr,&priorityEntry);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableIndexRangeClear function
* @endinternal
*
* @brief   Clear priority entries covering the given index range not containing rules.
*          The range can remain covered if all of it at the middle of one priority entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] priority                 - priority
* @param[in] placeFromHigh            - GT_TRUE for high neighbour, GT_FALSE for low neighbour
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableIndexRangeClear
(
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *pVTcamMngPtr,
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *tcamCfgPtr,
    IN       GT_U32                                             baseLogIndex,
    IN       GT_U32                                             numOfRules
)
{
    PRV_CPSS_AVL_TREE_PATH      ruleAvlTreePath; /* rule tree path */
    PRV_CPSS_AVL_TREE_PATH      prioAvlTreePath; /* priority tree path */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *priorityEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     ruleNeedToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     *ruleIdEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL retVal;

    if ((baseLogIndex + numOfRules) > tcamCfgPtr->rulesAmount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (numOfRules == 0)
    {
        return GT_OK;
    }

    /* check that the range does not contain rules */
    ruleNeedToFindInfo.ruleId = 0; /*don't care*/
    ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
    ruleNeedToFindInfo.logicalIndex = baseLogIndex;
    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->ruleIdIndexTree,
        PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
        &ruleNeedToFindInfo, /* pData */
        ruleAvlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    ruleIdEntryPtr = dbEntryPtr;
    if ((retVal != GT_FALSE) &&
        (ruleIdEntryPtr->logicalIndex < (baseLogIndex + numOfRules)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* set priority to start find in tree */
    priorityEntry.baseLogIndex = (baseLogIndex + numOfRules - 1);
    priorityEntry.rangeSize = 0;    /* don't care */
    priorityEntry.priority = 0; /* don't care */

    while (1)
    {
        retVal = prvCpssAvlPathFind(
            tcamCfgPtr->priorityIndexTree,
            PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
            &priorityEntry, /* pData */
            prioAvlTreePath,    /* path */
            &dbEntryPtr     /* use dedicated var to avoid warnings */);
        if (retVal == GT_FALSE)
        {
            return GT_OK;
        }
        priorityEntryPtr = dbEntryPtr;

        if ((priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize) <= baseLogIndex)
        {
            /* out of range */
            return GT_OK;
        }
        if ((priorityEntryPtr->baseLogIndex < baseLogIndex) &&
            ((priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize) > (baseLogIndex + numOfRules)))
        {
            /* includes all range and semething from both cides */
            return GT_OK;
        }
        if ((priorityEntryPtr->baseLogIndex >= baseLogIndex) &&
            ((priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize) <= (baseLogIndex + numOfRules)))
        {
            /* delete entry - no rules in it's range */
            /* delete entry from priority tree */
            prvCpssAvlItemRemove(tcamCfgPtr->priorityPriTree, priorityEntryPtr);
            /* delete entry from priority index tree */
            prvCpssAvlItemRemove(tcamCfgPtr->priorityIndexTree, priorityEntryPtr);
            /* free a buffer back to its pool */
            cpssBmPoolBufFree(pVTcamMngPtr->priorityPool, priorityEntryPtr);
            continue;
        }
        if ((priorityEntryPtr->baseLogIndex >= baseLogIndex) &&
            ((priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize) > (baseLogIndex + numOfRules)))
        {
            /* truncate from the low cide */
            priorityEntryPtr->rangeSize =
                (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize) - (baseLogIndex + numOfRules);
            priorityEntryPtr->baseLogIndex = (baseLogIndex + numOfRules);
            continue;
        }
        if ((priorityEntryPtr->baseLogIndex < baseLogIndex) &&
            ((priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize) <= (baseLogIndex + numOfRules)))
        {
            /* truncate from the high cide */
            priorityEntryPtr->rangeSize = baseLogIndex - priorityEntryPtr->baseLogIndex;
            return GT_OK;
        }
        /* should never occur */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntryNeighbourAdjust function
* @endinternal
*
* @brief   Adjust bound of neighbour priority entry before adding rule.
*          Decreases space covered by priorty entry not containing rules.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] priority                 - priority
* @param[in] placeFromHigh            - GT_TRUE for high neighbour, GT_FALSE for low neighbour
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntryNeighbourAdjust
(
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *pVTcamMngPtr,
    IN       PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *tcamCfgPtr,
    IN       GT_U32                                             priority,
    IN       GT_BOOL                                            placeFromHigh
)
{
    PRV_CPSS_AVL_TREE_PATH      ruleAvlTreePath;
    PRV_CPSS_AVL_TREE_PATH      prioAvlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *priorityEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     ruleNeedToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC     *ruleIdEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL retVal;
    PRV_CPSS_AVL_TREE_FIND_ENT  priorityFindType;
    PRV_CPSS_AVL_TREE_FIND_ENT  ruleFindType;
    GT_U32  boundRuleLogIndex;
    GT_U32  findPriority;

    if ((priority == 0) && (placeFromHigh == GT_FALSE))
    {
        /* no neghbour */
        return GT_OK;
    }

    if (placeFromHigh != GT_FALSE)
    {
        priorityFindType = PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E;
        ruleFindType     = PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E;
        findPriority     = priority + 1;
    }
    else
    {
        priorityFindType = PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E;
        ruleFindType     = PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E;
        findPriority     = priority - 1;
    }
    /* set priority to start find in tree */
    priorityEntry.baseLogIndex = 0; /* don't care */
    priorityEntry.rangeSize = 0;    /* don't care */
    priorityEntry.priority = findPriority;

    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->priorityPriTree,
        priorityFindType,
        &priorityEntry, /* pData */
        prioAvlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    if (retVal == GT_FALSE)
    {
        /* nothing to adjust */
        return GT_OK;
    }

    priorityEntryPtr = dbEntryPtr;
    ruleNeedToFindInfo.applicationDataPtr = NULL; /*don't care*/
    ruleNeedToFindInfo.ruleId             = 0;    /*don't care*/
    if (placeFromHigh != GT_FALSE)
    {
        ruleNeedToFindInfo.logicalIndex = priorityEntryPtr->baseLogIndex;
    }
    else
    {
        ruleNeedToFindInfo.logicalIndex = (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize - 1);
    }

    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->ruleIdIndexTree,
        ruleFindType,
        &ruleNeedToFindInfo, /* pData */
        ruleAvlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    ruleIdEntryPtr = dbEntryPtr;
    boundRuleLogIndex =
        (retVal == GT_FALSE) ? 0xFFFFFFFF : ruleIdEntryPtr->logicalIndex;

    while (1)
    {
        if (placeFromHigh != GT_FALSE)
        {
            if ((boundRuleLogIndex == 0xFFFFFFFF) ||
                ((priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize) <= boundRuleLogIndex))
            {
                /* delete entry - no rules in it's range */
                /* delete entry from priority tree */
                prvCpssAvlItemRemove(tcamCfgPtr->priorityPriTree, priorityEntryPtr);
                /* delete entry from priority index tree */
                prvCpssAvlItemRemove(tcamCfgPtr->priorityIndexTree, priorityEntryPtr);
                /* free a buffer back to its pool */
                cpssBmPoolBufFree(pVTcamMngPtr->priorityPool, priorityEntryPtr);
            }
            else
            {
                /* inplace update entry and return */
                priorityEntryPtr->rangeSize    =
                    (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize) - boundRuleLogIndex;
                priorityEntryPtr->baseLogIndex = boundRuleLogIndex;
                return GT_OK;
            }
        }
        else
        {
            if ((boundRuleLogIndex == 0xFFFFFFFF) ||
                (priorityEntryPtr->baseLogIndex > boundRuleLogIndex))
            {
                /* delete entry - no rules in it's range */
                /* delete entry from priority tree */
                prvCpssAvlItemRemove(tcamCfgPtr->priorityPriTree, priorityEntryPtr);
                /* delete entry from priority index tree */
                prvCpssAvlItemRemove(tcamCfgPtr->priorityIndexTree, priorityEntryPtr);
                /* free a buffer back to its pool */
                cpssBmPoolBufFree(pVTcamMngPtr->priorityPool, priorityEntryPtr);
            }
            else
            {
                /* inplace update entry and return */
                if ((priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize) > boundRuleLogIndex)
                {
                    priorityEntryPtr->rangeSize = boundRuleLogIndex - priorityEntryPtr->baseLogIndex + 1;
                }
                return GT_OK;
            }
        }
        /* set the same priority to start GE or LE find in tree - the previous found entry deleted */
        priorityEntry.baseLogIndex = 0; /* don't care */
        priorityEntry.rangeSize = 0;    /* don't care */
        priorityEntry.priority = findPriority;
        retVal = prvCpssAvlPathFind(
            tcamCfgPtr->priorityPriTree,
            priorityFindType,
            &priorityEntry, /* pData */
            prioAvlTreePath,    /* path */
            &dbEntryPtr     /* use dedicated var to avoid warnings */);
        priorityEntryPtr = dbEntryPtr;
        if (retVal == GT_FALSE)
        {
            return GT_OK;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamSplitHoleLowPartSizeGet function
* @endinternal
*
* @brief   Get size of left part of the splitted hole.
* @note    Two cases for splitting hole:
* @note    1. write new rule into the hole, 2. shift range of rules from the hole side into it.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in]  holeBaseLogIndex         - Hole Base Logical Index
* @param[in]  holeSize                 - Hole Size in rules
* @param[in]  requiredPriority         - priority of the new rule
* @param[in]  anchorLogicalIndex       - logical index of busy position precalculated to insert the new rule
* @param[out] lowPartSizePtr           - (pointer to) size of low part of the hole
* @param[out] newRuleCloseToLowPtr     - (pointer to) position of the new rule in a new hole:
*                                        0 - low side, 100 - high side, 50 - middle
*
* @retval                              - GT_OK - on success, GT_FAIL - on wrong parameters
*
*/
GT_STATUS prvCpssDxChVirtualTcamSplitHoleLowPartSizeGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *tcamCfgPtr,
    IN   GT_U32                                             holeBaseLogIndex,
    IN   GT_U32                                             holeSize,
    IN   GT_U32                                             requiredPriority,
    IN   GT_U32                                             anchorLogicalIndex,
    OUT  GT_U32                                             *lowPartSizePtr,
    OUT  GT_U32                                            *newRuleCloseToLowPtr
)
{
    GT_U32                                              priorityGranularity; /* differnce of priorities placing contiguous */
    PRV_CPSS_AVL_TREE_PATH                              avlTreePath;         /* path in AVL tree used for seek */
    PRV_CPSS_AVL_TREE_SEEK_ENT                          seekType;            /* type of seek */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;       /* priority entry pattern to search */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    anchorPriorityEntry; /* found priority entry by anchor */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    nextPriorityEntry;   /* found next priority entry by anchor */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *priorityEntryPtr;   /* pointer to the found priority record */
    GT_VOID_PTR                                         dbEntryPtr;          /* pointer to entry in DB             */
    GT_BOOL                                             retVal;              /* return value */

    priorityGranularity = 1;

    if (holeSize < 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if ((holeBaseLogIndex <= anchorLogicalIndex)
        && ((holeBaseLogIndex + holeSize) > anchorLogicalIndex))
    {
        /* anchor should not be inside the hole, but if is - not needed to continue */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (holeBaseLogIndex <= anchorLogicalIndex)
    {
        /* move rules to high part of hole - to use all hole as high */
        if (holeBaseLogIndex == 0)
        {
            *lowPartSizePtr = 0;
            *newRuleCloseToLowPtr = 0;
            return GT_OK;
        }
    }
    else
    {
        /* move rules to low part of hole - to use all hole low - no */
        if ((holeBaseLogIndex + holeSize) >= tcamCfgPtr->rulesAmount)
        {
            *lowPartSizePtr = holeSize;
            *newRuleCloseToLowPtr = 100;
            return GT_OK;
        }
    }

    anchorPriorityEntry.priority = 0xFFFFFFFF;
    /* set priority to start find in tree */
    priorityEntry.priority = 0; /* don't care */
    priorityEntry.baseLogIndex = anchorLogicalIndex;
    priorityEntry.rangeSize = 0;    /* don't care */
    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->priorityIndexTree,
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
        &priorityEntry, /* pData */
        avlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    if (retVal != GT_FALSE)
    {
        priorityEntryPtr   = dbEntryPtr;
        anchorPriorityEntry = *priorityEntryPtr;
    }

    if (anchorPriorityEntry.priority == 0xFFFFFFFF)
    {
        /* should not occur */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if ((requiredPriority > anchorPriorityEntry.priority)
        && (holeBaseLogIndex >= anchorLogicalIndex))
    {
        /* should not occur */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if ((requiredPriority < anchorPriorityEntry.priority)
        && (holeBaseLogIndex <= anchorLogicalIndex))
    {
        /* should not occur */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (requiredPriority > anchorPriorityEntry.priority)
    {
        seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
    }
    else
    {
        seekType = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;
    }
    nextPriorityEntry.priority = 0xFFFFFFFF;
    retVal = prvCpssAvlPathSeek(
        tcamCfgPtr->priorityIndexTree, seekType, avlTreePath,
        &dbEntryPtr     /* use dedicated var to avoid warnings */);
    if (retVal != GT_FALSE)
    {
        priorityEntryPtr = dbEntryPtr;
        nextPriorityEntry = *priorityEntryPtr;
    }

    if (nextPriorityEntry.priority == 0xFFFFFFFF)
    {
        if (holeBaseLogIndex <= anchorLogicalIndex)
        {
            *newRuleCloseToLowPtr = 0;
            *lowPartSizePtr       = 0;
            return GT_OK;
        }
        else
        {
            *newRuleCloseToLowPtr = 100;
            *lowPartSizePtr       = holeSize;
            return GT_OK;
        }
    }

    if (holeBaseLogIndex <= anchorLogicalIndex)
    {
        /* move rules to high part of hole - low can be 0, high - no */
        *lowPartSizePtr       = ((holeSize - 1) / 2);
    }
    else
    {
        /* move rules to low part of hole - high can be 0, low - no */
        *lowPartSizePtr       = ((holeSize + 1) / 2);
    }

    if (requiredPriority == anchorPriorityEntry.priority)
    {
        if (holeBaseLogIndex <= anchorLogicalIndex)
        {
            *newRuleCloseToLowPtr = 0;
            return GT_OK;
        }
        else
        {
            *newRuleCloseToLowPtr = 100;
            return GT_OK;
        }
    }

    if (requiredPriority == nextPriorityEntry.priority)
    {
        if (holeBaseLogIndex <= anchorLogicalIndex)
        {
            *newRuleCloseToLowPtr = 100;
            return GT_OK;
        }
        else
        {
            *newRuleCloseToLowPtr = 0;
            return GT_OK;
        }
    }

    if ((requiredPriority <= (anchorPriorityEntry.priority + priorityGranularity))
        && ((requiredPriority + priorityGranularity) >= anchorPriorityEntry.priority))
    {
        if (holeBaseLogIndex <= anchorLogicalIndex)
        {
            *newRuleCloseToLowPtr = 0;
            return GT_OK;
        }
        else
        {
            *newRuleCloseToLowPtr = 100;
            return GT_OK;
        }
    }

    if ((requiredPriority <= (nextPriorityEntry.priority + priorityGranularity))
        && ((requiredPriority + priorityGranularity) >= nextPriorityEntry.priority))
    {
        if (holeBaseLogIndex <= anchorLogicalIndex)
        {
            *newRuleCloseToLowPtr = 100;
            return GT_OK;
        }
        else
        {
            *newRuleCloseToLowPtr = 0;
            return GT_OK;
        }
    }

    *newRuleCloseToLowPtr = 50;
    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntryBallancedGet function
* @endinternal
*
* @brief   Get Logical Index Range from Priority Table with memory ballance for a new priority.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] priority                 - priority
* @param[out] entryPtr                - (pointer to) entry with info
*
* @retval GT_OK                    - on success
*
* @note For not existing range returned rangeSizePtr == 0 and
*       baseLogIndexPtr points to the middle of the nearest less priority range (or 0 if not)
*       and nearest more priority range (or tcamCfgPtr->rulesAmount if not).
*
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntryBallancedGet
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *tcamCfgPtr,
    IN   GT_U32                                             priority,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC   *entryPtr
)
{
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *priorityEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL retVal;
    GT_U32  lowBound, highBound;

    /* set priority to start find in tree */
    priorityEntry.priority = priority;
    priorityEntry.baseLogIndex = 0; /* don't care */
    priorityEntry.rangeSize = 0;    /* don't care */

    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->priorityPriTree,
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,
        &priorityEntry, /* pData */
        avlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);

    priorityEntryPtr = dbEntryPtr;
    if ((retVal == GT_TRUE) &&
        ((priorityEntryPtr->priority == priority) ||
         ((priorityEntryPtr->priority + 1) == priority)))
    {
        /* found exact or neighbour priority entry - copy to output entry  */
        cpssOsMemCpy(
            entryPtr, priorityEntryPtr,
            sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC));
        return GT_OK;
    }

    lowBound = (retVal == GT_TRUE)
        ? (priorityEntryPtr->baseLogIndex + priorityEntryPtr->rangeSize)
        : 0xFFFFFFFF;

    retVal = prvCpssAvlPathFind(
        tcamCfgPtr->priorityPriTree,
        PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
        &priorityEntry, /* pData */
        avlTreePath,    /* path */
        &dbEntryPtr     /* use dedicated var to avoid warnings */);

    priorityEntryPtr = dbEntryPtr;
    if ((retVal == GT_TRUE) && (priorityEntryPtr->priority == (priority + 1)))
    {
        /* found neighbour priority entry - copy to output entry  */
        cpssOsMemCpy(
            entryPtr, priorityEntryPtr,
            sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC));
        return GT_OK;
    }

    highBound = (retVal == GT_TRUE)
        ? priorityEntryPtr->baseLogIndex
        : 0xFFFFFFFF;

    /*tcamCfgPtr->rulesAmount;*/

    entryPtr->priority = priority;
    entryPtr->rangeSize = 0;
    if ((lowBound != 0xFFFFFFFF) && (highBound != 0xFFFFFFFF))
    {
        entryPtr->baseLogIndex = ((lowBound + highBound) / 2);
    }
    else if (lowBound != 0xFFFFFFFF)
    {
        /* assumed highBound == 0xFFFFFFFF */
        /* can be tcamCfgPtr->rulesAmount i.e. out of space */
        entryPtr->baseLogIndex = lowBound;
    }
    else if (highBound != 0xFFFFFFFF)
    {
        /* assumed lowBound == 0xFFFFFFFF */
        entryPtr->baseLogIndex =
            (highBound > 0) ? (highBound - 1) : 0;
    }
    else
    {
        /* assumed lowBound == 0xFFFFFFFF and highBound == 0xFFFFFFFF */
        entryPtr->baseLogIndex = 0;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbPriorityTableEntryFind function
* @endinternal
*
* @brief   Find entry Id rule by logical index.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @param[out] treePathArr              - (pointer to) tree path array.
* @param[out] entryPtr                 - (pointer to) entry with info
*
* @retval GT_OK                    - on success
* @retval GT_NOT_FOUND             - entry not found
*/
GT_STATUS prvCpssDxChVirtualTcamDbPriorityTableEntryFind
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC            *tcamCfgPtr,
    IN   GT_U32                                                  logicalIndex,
    IN   PRV_CPSS_AVL_TREE_FIND_ENT                              findType,
    OUT  PRV_CPSS_AVL_TREE_PATH                                  treePathArr,
    OUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC       *entryPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry, * priorityEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL retVal;

    /* set logical index to start find in tree */
    priorityEntry.priority = 0;     /* don't care */
    priorityEntry.baseLogIndex = logicalIndex;
    priorityEntry.rangeSize = 0;    /* don't care */

    retVal = prvCpssAvlPathFind(tcamCfgPtr->priorityIndexTree,
                                findType,
                                &priorityEntry, /* pData */
                                treePathArr,    /* path */
                                &dbEntryPtr     /* use dedicated var to avoid warnings */);
    /* entry not found */
    if (retVal == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,"prvCpssAvlPathFind: entry with logical index %d not found", logicalIndex);
    }

    priorityEntryPtr = dbEntryPtr;
    /* copy to output entry  */
    cpssOsMemCpy(entryPtr, priorityEntryPtr,
                 sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC));

    return GT_OK;
}

/**
* @internal priorityTableEntryRemoveLogicalIndexUpdateRange function
* @endinternal
*
* @brief   Remove Logical Index from the Range of Priority.
*         ONLY if the removed logical index is the 'last' in the range we need to update
*         the range.
*         otherwise we not update the range.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS priorityTableEntryRemoveLogicalIndexUpdateRange
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC           *vTcamMngDBPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *vtcamInfoPtr,
    IN   GT_U32                                       priority,
    IN   GT_U32                                       logicalIndex
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    GT_U32  prevValidIndex;

    /* find the priority in the DB */
    rc = prvCpssDxChVirtualTcamDbPriorityTableEntryGet(
        vtcamInfoPtr,priority,&priorityEntry);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(priorityEntry.rangeSize == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"rangeSize is 0 for priority[%d]",
            priority);
    }

    if((priorityEntry.baseLogIndex + priorityEntry.rangeSize - 1) != logicalIndex)
    {
        /* nothing to do .. not the last index in the range */
        return GT_OK;
    }

    CPSS_LOG_INFORMATION_MAC("deleting the last logical index[%d] for priority[%d] , need to calculate new rangeSize \n",
        logicalIndex,priority);

    if(priorityEntry.baseLogIndex == logicalIndex)
    {
        /* the current priority should be removed */
        priorityEntry.rangeSize = 0;
        /* we remove the single 'first' and 'last' index */
        CPSS_LOG_INFORMATION_MAC("The priority[%d] is now without logical indexes .. will be removed \n",
            priorityEntry.rangeSize);
    }
    else
    {
        /* removing the last logical index of the priority ..
           so check for collapse until first valid logical index */
        rc = findNearestLogicalIndexUsed(vtcamInfoPtr,
            logicalIndex,
            GT_TRUE, /*less : lessOrMore*/
            priorityEntry.baseLogIndex,/*lastLogicalIndexToCheck*/
            &prevValidIndex
        );

        if(rc == GT_OK)
        {
            if(prevValidIndex < priorityEntry.baseLogIndex)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"prevValidIndex[%d] can't be less than priorityEntry.baseLogIndex[%d]",
                    prevValidIndex,
                    priorityEntry.baseLogIndex);
            }

            /* found next valid index previous to the removed one ...
                update the rangeSize */
            priorityEntry.rangeSize = 1 + (prevValidIndex - priorityEntry.baseLogIndex);

            CPSS_LOG_INFORMATION_MAC("new priorityEntry.rangeSize[%d] for priority[%d] \n",
                priorityEntry.rangeSize);
        }
        else
        {
            /* the current priority should be removed */
            priorityEntry.rangeSize = 0;

            CPSS_LOG_INFORMATION_MAC("The priority[%d] is now without logical indexes .. will be removed \n",
                priorityEntry.rangeSize);
        }
    }

    /* update the trees of the priority */
    rc = prvCpssDxChVirtualTcamDbPriorityTableEntrySet(
        vTcamMngDBPtr,vtcamInfoPtr,&priorityEntry);

    return rc;

}

/**
* @internal prvCpssDxChVirtualTcamDbRuleIdTableEntryRemoveByLogicalIndex function
* @endinternal
*
* @brief   Remove rule Id entry by logical index from tree : ruleIdIndexTree
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pVTcamMngPtr             - (pointer to) VTCAM Manager structure
* @param[in] tcamCfgPtr               - (pointer to)virtual TCAM structure
* @param[in] logicalIndex             - logical index of ruleId to remove.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on ruleId or/and logicalIndex already used
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChVirtualTcamDbRuleIdTableEntryRemoveByLogicalIndex
(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                *pVTcamMngPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC       *tcamCfgPtr,
    IN  GT_U32                                              logicalIndex
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC ruleIdEntry, * ruleIdEntryPtr;

    ruleIdEntry.logicalIndex = logicalIndex;
    /* find logicalIndex in entry in AVL tree */
    ruleIdEntryPtr = prvCpssAvlSearch(tcamCfgPtr->ruleIdIndexTree, &ruleIdEntry);
    if (ruleIdEntryPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* call to delete by ruleId */
    rc = prvCpssDxChVirtualTcamDbRuleIdTableEntryRemove(
            pVTcamMngPtr,
            tcamCfgPtr,
            ruleIdEntryPtr->ruleId);

    return rc;
}
/**
* @internal priorityDbRemoveRuleId function
* @endinternal
*
* @brief   Remove rule Id entry and update priority entry if needed when the ruleId
*         is deleted.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on DB mismatch
* @retval GT_FAIL                  - on error
*/
static GT_STATUS priorityDbRemoveRuleId(
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC                 *vTcamMngDBPtr,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC        *vtcamInfoPtr,
    IN  GT_U32                                              logicalIndex
)
{
    GT_STATUS rc;

    /* remove logical index from bitmap */
    vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] &= ~(1<<(logicalIndex & 0x1f));

    /* Remove rule Id entry by logical index from tree : ruleIdIndexTree */
    rc = prvCpssDxChVirtualTcamDbRuleIdTableEntryRemoveByLogicalIndex(
            vTcamMngDBPtr,
            vtcamInfoPtr,
            logicalIndex);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = prvDxChVirtualTcamPriorityDbEntryAdjustByLogicalIndexBitmap(
        vTcamMngDBPtr, vtcamInfoPtr, logicalIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }


    return GT_OK;
}

/**
* @internal resizeMapHwIndexesToLogicalIndexes function
* @endinternal
*
* @brief   (for resize purposes) map HW index to logical indexes of
*         the 'old'/'new' logical index.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on Cpu memory allocation failed.
* @retval GT_BAD_STATE             - on DB management error
*/
static GT_STATUS resizeMapHwIndexesToLogicalIndexes(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr,
    IN GT_BOOL                                      oldOrNew
)
{
    GT_U32  logicalIndex;
    GT_U32  hwIndex;
    GT_U32  maxSegmentsInDb = vTcamInfoPtr->tcamSegCfgPtr->maxSegmentsInDb;
    GT_U32  memSize = sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RESIZE_HW_INDEX_MAP_INFO_STC) * maxSegmentsInDb;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RESIZE_HW_INDEX_MAP_INFO_STC    *prvCpssDxChVirtualTcamDbResizeHwIndexDbArr;
    GT_U32 prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr;

    prvCpssDxChVirtualTcamDbResizeHwIndexDbArr =
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr);
    prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr =
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr);
    if((prvCpssDxChVirtualTcamDbResizeHwIndexDbArr != NULL) &&
        (prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr < maxSegmentsInDb))
    {
        /* There is not enough space at the array - reallocate it */
        cpssOsFree(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr);
        prvCpssDxChVirtualTcamDbResizeHwIndexDbArr = NULL;
        prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr = 0;
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr) = NULL;
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr) = 0;
    }

    if (prvCpssDxChVirtualTcamDbResizeHwIndexDbArr == NULL)
    {
        if(oldOrNew == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        prvCpssDxChVirtualTcamDbResizeHwIndexDbArr = cpssOsMalloc(memSize);
        if(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr = maxSegmentsInDb;

        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr) =
            prvCpssDxChVirtualTcamDbResizeHwIndexDbArr;
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr) =
            prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr;
    }

    if(oldOrNew == GT_TRUE)
    {
        for(hwIndex = 0; hwIndex < maxSegmentsInDb; hwIndex++)
        {
            prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[hwIndex].oldLogicalIndex = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS;
            prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[hwIndex].newLogicalIndex = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS;
        }
    }

    for(logicalIndex = 0; logicalIndex < vTcamInfoPtr->rulesAmount; logicalIndex++)
    {
        hwIndex = vTcamInfoPtr->rulePhysicalIndexArr[logicalIndex];

        if(hwIndex >= maxSegmentsInDb)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB error : rulesAmount[%d] logicalIndex[%d], hwIndex[%d] >= maxSegmentsInDb[%d]",
                vTcamInfoPtr->rulesAmount,logicalIndex,hwIndex,maxSegmentsInDb);
        }

        if(oldOrNew == GT_TRUE)
        {
            prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[hwIndex].oldLogicalIndex = logicalIndex;
        }
        else
        {
            prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[hwIndex].newLogicalIndex = logicalIndex;
        }
    }

    return GT_OK;
}

/**
* @internal resizeMoveLogicalIndexes function
* @endinternal
*
* @brief   (for resize purposes) move the 'old' logical index rules to 'new'
*         logical index.
*         NOTE: using prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[]
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamInfoPtr             - (pointer to) virtual TCAM structure
* @param[in] resizeValue              -
*                                      value > 0 --> resize to bigger size
* @param[in] resizeMode               - resize mode (with/without index)
* @param[in] resizeReferenceLogicalIndex - resize reference logical index
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on Cpu memory allocation failed.
* @retval GT_BAD_STATE             - on DB management error
*/
static GT_STATUS resizeMoveLogicalIndexes(
    IN  GT_U32                                      vTcamMngId,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC  *vTcamInfoPtr,
    IN  GT_U32                                      resizeValue,
    IN  RESIZE_MODE_ENT                             resizeMode,
    IN  GT_U32                                      resizeReferenceLogicalIndex
)
{
    GT_STATUS   rc;
    GT_U32  oldLogicalIndex;
    GT_U32  newSrc_LogicalIndex;
    GT_U32  newTrg_LogicalIndex;
    GT_U32  oldHwIndex,newHwIndex;
    GT_U32  maxSegmentsInDb = vTcamInfoPtr->tcamSegCfgPtr->maxSegmentsInDb;
    GT_BOOL needBreak = GT_FALSE;
    GT_U32  startIndex = 0;
    GT_U32  endIndex = maxSegmentsInDb;
    GT_BOOL needStraightDirection = GT_FALSE;
    GT_U32  ruleAlignBmp;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RESIZE_HW_INDEX_MAP_INFO_STC    *prvCpssDxChVirtualTcamDbResizeHwIndexDbArr;

    prvCpssDxChVirtualTcamDbResizeHwIndexDbArr =
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr);

    if(resizeValue == 0)
    {
        /* the function is not for such cases */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    ruleAlignBmp = prvCpssDxChVirtualTcamDbSegmentTableRuleEnumAlignmentBitmapGet(
        vTcamInfoPtr->tcamSegCfgPtr->deviceClass,
        vTcamInfoPtr->tcamInfo.ruleSize);
    if (ruleAlignBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* do revers iterator , to keep lookup order and not to override any existing rule */

    /* loop on the HW indexes provide assurance that the lookup order will not be
       changed because the added segments are */

    for (oldHwIndex = maxSegmentsInDb - 1; (needBreak == GT_FALSE); oldHwIndex--)
    {
        if(oldHwIndex == 0)
        {
            /* break will come after we will finish handling index 0 */
            needBreak = GT_TRUE;
        }
        if ((ruleAlignBmp & (1 << (oldHwIndex % vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount))) == 0) continue;

        /* the 'old' index is now at this index ... so this is SRC */
        newSrc_LogicalIndex = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[oldHwIndex].newLogicalIndex;

        if(newSrc_LogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS)
        {
            /* prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[oldHwIndex].newLogicalIndex was not mapped */
            continue;
        }

        /* do not move non-valid rule */
        if(0 == (vTcamInfoPtr->usedRulesBitmapArr[newSrc_LogicalIndex>>5] & (1<<(newSrc_LogicalIndex&0x1f))))
        {
            continue;
        }

        oldLogicalIndex = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[oldHwIndex].oldLogicalIndex;

        if(oldLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS)
        {
            /* prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[oldHwIndex].oldLogicalIndex was not mapped */
            continue;
        }

        newTrg_LogicalIndex = oldLogicalIndex;

        if((oldLogicalIndex > resizeReferenceLogicalIndex) ||
           ((oldLogicalIndex == resizeReferenceLogicalIndex) && resizeMode == RESIZE_MODE_WITH_INPUT_INDEX_E))
        {
            /* the TRG index need to be according to the resizeValue with offset from 'old' index*/
            newTrg_LogicalIndex = oldLogicalIndex + resizeValue;
        }
        else
        {
            newTrg_LogicalIndex = oldLogicalIndex;
        }

        if(newSrc_LogicalIndex == newTrg_LogicalIndex)
        {
            /* no move needed */
            continue;
        }

        if(newSrc_LogicalIndex > newTrg_LogicalIndex)
        {
            /* all next moves need straight iterator (not revers iterator) */
            /* the nearest next entry with alignment allowing the rule of the VTCAMs size */
            for (endIndex = oldHwIndex + 1;
                  ((ruleAlignBmp & (1 << (endIndex % vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount))) == 0);
                  endIndex++) {}
            needStraightDirection = GT_TRUE;
            break;
        }

        newHwIndex = vTcamInfoPtr->rulePhysicalIndexArr[newTrg_LogicalIndex];

        /* move the index in HW and in DB */
        rc = ruleMoveByLogicalIndex(vTcamMngId,vTcamInfoPtr,
            newSrc_LogicalIndex,oldHwIndex,
            newTrg_LogicalIndex,newHwIndex,
            GT_TRUE/*calledFromResize*/);

        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(needStraightDirection == GT_FALSE)
    {
        /* no more needed */
        return GT_OK;
    }

    for(oldHwIndex = startIndex; oldHwIndex < endIndex ; oldHwIndex += 1)
    {
        if ((ruleAlignBmp & (1 << (oldHwIndex % vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount))) == 0) continue;

        /* the 'old' index is now at this index ... so this is SRC */
        newSrc_LogicalIndex = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[oldHwIndex].newLogicalIndex;

        if(newSrc_LogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS)
        {
            /* prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[oldHwIndex].newLogicalIndex was not mapped */
            continue;
        }

        /* do not move non-valid rule */
        if(0 == (vTcamInfoPtr->usedRulesBitmapArr[newSrc_LogicalIndex>>5] & (1<<(newSrc_LogicalIndex&0x1f))))
        {
            continue;
        }

        oldLogicalIndex = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[oldHwIndex].oldLogicalIndex;

        if(oldLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS)
        {
            /* prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[oldHwIndex].oldLogicalIndex was not mapped */
            continue;
        }

        newTrg_LogicalIndex = oldLogicalIndex;

        if((oldLogicalIndex > resizeReferenceLogicalIndex) ||
           ((oldLogicalIndex == resizeReferenceLogicalIndex) && resizeMode == RESIZE_MODE_WITH_INPUT_INDEX_E))
        {
            /* the TRG index need to be according to the resizeValue with offset from 'old' index*/
            newTrg_LogicalIndex = oldLogicalIndex + resizeValue;
        }
        else
        {
            newTrg_LogicalIndex = oldLogicalIndex;
        }

        if(newSrc_LogicalIndex == newTrg_LogicalIndex)
        {
            /* no move needed */
            continue;
        }

        if(newSrc_LogicalIndex < newTrg_LogicalIndex)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "DB error: algorithm error : unexpected 'opposite' direction.newSrc_LogicalIndex[%d] < newTrg_LogicalIndex[%d] \n",
                newSrc_LogicalIndex , newTrg_LogicalIndex);
        }

        newHwIndex = vTcamInfoPtr->rulePhysicalIndexArr[newTrg_LogicalIndex];

        /* move the index in HW and in DB */
        rc = ruleMoveByLogicalIndex(vTcamMngId,vTcamInfoPtr,
            newSrc_LogicalIndex,oldHwIndex,
            newTrg_LogicalIndex,newHwIndex,
            GT_TRUE/*calledFromResize*/);

        if(rc != GT_OK)
        {
            return rc;
        }
    }


    return GT_OK;
}

/**
* @internal priorityResizeUpdateTrees function
* @endinternal
*
* @brief   (for resize purposes) for priority mode :
*         update the content of next trees that hold logical indexes values:
*         vTcamInfoPtr->ruleIdIndexTree,vTcamInfoPtr->priorityIndexTree
*         it will use the DB of prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[]
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamInfoPtr             - (pointer to) virtual TCAM structure
* @param[in] resizeValue              -
*                                      value > 0 --> resize to bigger size
*                                      value < 0 --> resize to smaller size
* @param[in] resizeMode               - resize mode (with/without index)
* @param[in] resizeReferenceLogicalIndex - resize reference logical index
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on DB management error
*/
static GT_STATUS priorityResizeUpdateTrees(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr,
    IN  GT_32                                       resizeValue,
    IN  RESIZE_MODE_ENT                             resizeMode,
    IN  GT_U32                                      resizeReferenceLogicalIndex
)
{
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;
    PRV_CPSS_AVL_TREE_SEEK_ENT seekMode;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC *ruleIdEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *priorityEntryPtr;
    GT_U32  hwIndex;
    GT_BOOL needBreak;
    GT_U32  oldLogicalIndex,newLogicalIndex,treeLogicalIndex;
    GT_BOOL foundMatch;
    GT_U32  maxSegmentsInDb = vTcamInfoPtr->tcamSegCfgPtr->maxSegmentsInDb;
    GT_U32  startIndex,endIndex;
    GT_U32  ruleAlignBmp;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RESIZE_HW_INDEX_MAP_INFO_STC    *prvCpssDxChVirtualTcamDbResizeHwIndexDbArr;

    prvCpssDxChVirtualTcamDbResizeHwIndexDbArr =
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr);

    ruleAlignBmp = prvCpssDxChVirtualTcamDbSegmentTableRuleEnumAlignmentBitmapGet(
        vTcamInfoPtr->tcamSegCfgPtr->deviceClass,
        vTcamInfoPtr->tcamInfo.ruleSize);
    if (ruleAlignBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if(resizeValue > 0)
    {
        /* do reverse scan on the vTcamInfoPtr->ruleIdIndexTree */
        seekMode = PRV_CPSS_AVL_TREE_SEEK_LAST_E;
        for (startIndex = maxSegmentsInDb - 1;
              ((ruleAlignBmp & (1 << (startIndex % vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount))) == 0);
              startIndex--) {}
        endIndex = 0;
    }
    else
    {
        seekMode = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        startIndex = 0;
        for (endIndex = maxSegmentsInDb - 1;
              ((ruleAlignBmp & (1 << (endIndex % vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount))) == 0);
              endIndex--) {}
    }

    hwIndex = startIndex;
    needBreak = GT_FALSE;

    while(GT_TRUE ==
        prvCpssAvlPathSeek(vTcamInfoPtr->ruleIdIndexTree,
        seekMode,
        avlTreePath,
        &dbEntryPtr /* use dedicated var to avoid warnings */))
    {
        if(resizeValue > 0)
        {
            seekMode = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;
        }
        else
        {
            seekMode = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
        }
        ruleIdEntryPtr = dbEntryPtr;

        treeLogicalIndex = ruleIdEntryPtr->logicalIndex;

        foundMatch = GT_FALSE;
        /* update the logicalIndex */
        for(/* already initialized*/; needBreak == GT_FALSE ; /*no iteration change here */)
        {
            if(hwIndex == endIndex)
            {
                /* break will come after we will finish handling index 0 */
                needBreak = GT_TRUE;
            }

            oldLogicalIndex = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[hwIndex].oldLogicalIndex;
            newLogicalIndex = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[hwIndex].newLogicalIndex;

            if(oldLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS ||
               newLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS )
            {
                goto ruleTree_modifyIterator_lbl;/* instead of continue */
            }

            if(oldLogicalIndex == treeLogicalIndex)
            {
                foundMatch = GT_TRUE;
                /* found match ... update the tree with newLogicalIndex */
                ruleIdEntryPtr->logicalIndex = newLogicalIndex;
                break;
            }
ruleTree_modifyIterator_lbl:
            if(resizeValue > 0)
            {
                for (hwIndex--;
                      ((ruleAlignBmp & (1 << (hwIndex % vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount))) == 0);
                      hwIndex--) {}
            }
            else
            {
                for (hwIndex++;
                      ((ruleAlignBmp & (1 << (hwIndex % vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount))) == 0);
                      hwIndex++) {}
            }
        }

        if(foundMatch == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: mismatch between prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[] and vTcamInfoPtr->ruleIdIndexTree[] \n");
        }
    }

    if(resizeValue < 0)
    {
        /* can not handle here the modifications in vTcamInfoPtr->priorityIndexTree */
        return GT_OK;
    }

    /* do reverse scan on the vTcamInfoPtr->ruleIdIndexTree */
    seekMode = PRV_CPSS_AVL_TREE_SEEK_LAST_E;
    for (startIndex = maxSegmentsInDb - 1;
          ((ruleAlignBmp & (1 << (startIndex % vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount))) == 0);
          startIndex--) {}

    hwIndex = startIndex;
    needBreak = GT_FALSE;

    while(GT_TRUE ==
        prvCpssAvlPathSeek(vTcamInfoPtr->priorityIndexTree,
        seekMode,
        avlTreePath,
        &dbEntryPtr /* use dedicated var to avoid warnings */))
    {
        seekMode = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;

        priorityEntryPtr = dbEntryPtr;

        treeLogicalIndex = priorityEntryPtr->baseLogIndex;
        foundMatch = GT_FALSE;
        /* update the logicalIndex */
        for(/* already initialized*/; needBreak == GT_FALSE ; hwIndex--)
        {
            if(hwIndex == 0)
            {
                /* break will come after we will finish handling index 0 */
                needBreak = GT_TRUE;
            }
            if ((ruleAlignBmp & (1 << (hwIndex % vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount))) == 0) continue;

            oldLogicalIndex = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[hwIndex].oldLogicalIndex;
            newLogicalIndex = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[hwIndex].newLogicalIndex;

            if(oldLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS ||
               newLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS )
            {
                continue;
            }

            if(oldLogicalIndex == treeLogicalIndex)
            {
                foundMatch = GT_TRUE;

                if((oldLogicalIndex > resizeReferenceLogicalIndex) ||
                   ((oldLogicalIndex == resizeReferenceLogicalIndex) && resizeMode == RESIZE_MODE_WITH_INPUT_INDEX_E))
                {
                    /* the TRG index need to be according to the resizeValue with offset from 'old' index*/
                    newLogicalIndex = oldLogicalIndex + resizeValue;
                }
                else
                {
                    newLogicalIndex = oldLogicalIndex;
                }

                /* found match ... update the tree with newLogicalIndex */
                priorityEntryPtr->baseLogIndex = newLogicalIndex;
                break;
            }
        }

        if(foundMatch == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: mismatch between prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[] and vTcamInfoPtr->priorityIndexTree[] \n");
        }

    }

    return GT_OK;
}

/**
* @internal priorityResizeUpdateTrees_FromRemove function
* @endinternal
*
* @brief   (for downsize purposes - called from RemoveEmptyRangeOfLogicalIndexes) for priority mode :
*         update the content of next trees that hold logical indexes values:
*         vTcamInfoPtr->ruleIdIndexTree
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamInfoPtr             - (pointer to) virtual TCAM structure
* @param[in] downSizeValue            - value to downsize
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on DB management error
*/
static GT_STATUS priorityResizeUpdateTrees_FromRemove(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr,
    IN GT_U32                                       downSizeValue
)
{
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;
    PRV_CPSS_AVL_TREE_SEEK_ENT seekMode;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC *ruleIdEntryPtr;

    seekMode = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
    while(prvCpssAvlPathSeek(vTcamInfoPtr->ruleIdIndexTree,
                seekMode,
                avlTreePath,
                &dbEntryPtr /* use dedicated var to avoid warnings */))
    {
        seekMode = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
        ruleIdEntryPtr = dbEntryPtr;

        if (ruleIdEntryPtr->logicalIndex < downSizeValue)
        {
            /* Start range should be empty at this point */
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        else
        {
            ruleIdEntryPtr->logicalIndex -= downSizeValue;
        }
    }

    return GT_OK;
}

/**
* @internal resizeUpdateLogicalIndexBmp function
* @endinternal
*
* @brief   (for resize purposes)
*         update the content of next BMP that hold logical indexes values :
*         vTcamInfoPtr->usedRulesBitmapArr
*         it will use the DB of prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[]
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamInfoPtr             - (pointer to) virtual TCAM structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on DB management error
*/
static GT_STATUS resizeUpdateLogicalIndexBmp(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr
)
{
    GT_U32  hwIndex;
    GT_U32  maxSegmentsInDb = vTcamInfoPtr->tcamSegCfgPtr->maxSegmentsInDb;
    GT_BOOL needBreak = GT_FALSE;
    GT_U32  bmpSize ;
    GT_U32  *old_usedRulesBitmapArr = vTcamInfoPtr->usedRulesBitmapArr;
    GT_U32  oldLogicalIndex,newLogicalIndex;
    GT_U32  ruleAlignBmp;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RESIZE_HW_INDEX_MAP_INFO_STC    *prvCpssDxChVirtualTcamDbResizeHwIndexDbArr;

    prvCpssDxChVirtualTcamDbResizeHwIndexDbArr =
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr);

    ruleAlignBmp = prvCpssDxChVirtualTcamDbSegmentTableRuleEnumAlignmentBitmapGet(
        vTcamInfoPtr->tcamSegCfgPtr->deviceClass,
        vTcamInfoPtr->tcamInfo.ruleSize);
    if (ruleAlignBmp == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    bmpSize =
        (((vTcamInfoPtr->rulesAmount + 31) / 32)
          * sizeof(GT_U32));

    if (bmpSize == 0)
    {
        if (vTcamInfoPtr->usedRulesBitmapArr != NULL)
        {
            /* the new state is null-sized */
            cpssOsFree(vTcamInfoPtr->usedRulesBitmapArr);
            vTcamInfoPtr->usedRulesBitmapArr = NULL;
        }
        return GT_OK;
    }

    /* malloc new usedRulesBitmapArr[] */
    vTcamInfoPtr->usedRulesBitmapArr = (GT_U32*)cpssOsMalloc(bmpSize);
    if (vTcamInfoPtr->usedRulesBitmapArr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    /* memset to 0 the BMP */
    cpssOsMemSet(vTcamInfoPtr->usedRulesBitmapArr,0,bmpSize);

    if (old_usedRulesBitmapArr == NULL)
    {
        /* an old state was null-sized */
        return GT_OK;
    }

    for(hwIndex = maxSegmentsInDb - 1; (needBreak == GT_FALSE); hwIndex--)
    {
        if(hwIndex == 0)
        {
            /* break will come after we will finish handling index 0 */
            needBreak = GT_TRUE;
        }
        if ((ruleAlignBmp & (1 << (hwIndex % vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount))) == 0) continue;

        oldLogicalIndex = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[hwIndex].oldLogicalIndex;
        newLogicalIndex = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[hwIndex].newLogicalIndex;

        if(oldLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS ||
           newLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS )
        {
            continue;
        }

        if(old_usedRulesBitmapArr[oldLogicalIndex>>5]&(1<<(oldLogicalIndex&0x1f)))
        {
            /* the oldLogicalIndex was set in old_usedRulesBitmapArr[] */
            /* so need set bit newLogicalIndex in usedRulesBitmapArr */

            vTcamInfoPtr->usedRulesBitmapArr[newLogicalIndex>>5] |= (1<<(newLogicalIndex&0x1f));
        }
    }

    /* free the old array ... as it is not relevant any more */
    cpssOsFree(old_usedRulesBitmapArr);

    return GT_OK;
}


/**
* @internal vtcamDbSegmentTableVTcamAllocOrResize function
* @endinternal
*
* @brief   Allocate/resize memory for Virtual TCAM in Segment Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngDBPtr            - (pointer to) TCAM Manager info
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] resizeValue              - value == 0 --> no resize (initial alloc)
*                                      value > 0 --> resize to bigger size
* @param[in,out] alloc_vTcamInfoPtr       - pointer to vtcam that temporary gets segments for the resize operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
static GT_STATUS vtcamDbSegmentTableVTcamAllocOrResize
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *vTcamMngDBPtr,
    IN  GT_U32                                      vTcamId,
    IN  GT_U32                                      resizeValue,
    INOUT  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *alloc_vTcamInfoPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ALLOC_REQUEST_STC    allocRequest; /* request*/
    GT_U32      actualUsedSize; /* minimal of guaranteedNumOfRules ang maximal present rule index     */
                                /* when inserting rules the empty end of VTCAM should not be enlarged */
                                /* initial size and added space more than required due to alignment   */
                                /* This value is the estimation of a low bound of empty end of VTCAM  */
    GT_U32      lastRulePlace;

    if (vTcamMngDBPtr->vTcamCfgPtrArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vTcamInfoPtr  = vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId];

    allocRequest.lookupId        =
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
            vTcamInfoPtr->tcamInfo.clientGroup, vTcamInfoPtr->tcamInfo.hitNumber);
    allocRequest.vTcamId         = vTcamId;
    allocRequest.rulesAmount     = vTcamInfoPtr->tcamInfo.guaranteedNumOfRules;
    allocRequest.ruleSize =
        prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
            vTcamInfoPtr->tcamSegCfgPtr, vTcamInfoPtr->tcamInfo.ruleSize);
    allocRequest.ruleStartColumnsBitmap =
        prvCpssDxChVirtualTcamDbSegmentTableRuleAlignmentBitmapGet(
        vTcamInfoPtr->tcamSegCfgPtr->deviceClass,  allocRequest.ruleSize);
    if ((allocRequest.ruleSize == 0) || (allocRequest.ruleStartColumnsBitmap == 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    actualUsedSize = vTcamInfoPtr->rulesAmount;
    /* AvailableCheck function calls this code with usedRulesBitmapArr == NULL */
    if ((vTcamInfoPtr->rulesAmount > 0) && vTcamInfoPtr->usedRulesBitmapArr)
    {

        lastRulePlace = prvCpssBitmapLastOneBitIndexInRangeFind(
            vTcamInfoPtr->usedRulesBitmapArr,
            vTcamInfoPtr->tcamInfo.guaranteedNumOfRules,
            (vTcamInfoPtr->rulesAmount - 1),
            0xFFFFFFFF/*notFoundReturnValue*/);
        actualUsedSize =
            (lastRulePlace != 0xFFFFFFFF)
                ? (lastRulePlace + 1)
                : vTcamInfoPtr->tcamInfo.guaranteedNumOfRules;
    }

    if(resizeValue == 0)
    {
        /* initial space allocation */
        alloc_vTcamInfoPtr = vTcamInfoPtr;
    }
    else /*if(resizeValue > 0)*/
    {
        /* expanding VTCAM space */
        if (resizeValue
            <= (vTcamInfoPtr->rulesAmount - actualUsedSize))
        {
            /* VTAM already has needed space             */
            /* set stamp that no space reallocation done */
            alloc_vTcamInfoPtr->rulesAmount = 0;
            return GT_OK;
        }
        /* set proper needed size */
        allocRequest.rulesAmount = resizeValue
            - (vTcamInfoPtr->rulesAmount - actualUsedSize);
    }

    /*Allocate memory for Virtual TCAM in Segment Table for TCAM.*/
    rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate(
        vTcamInfoPtr->tcamSegCfgPtr,  alloc_vTcamInfoPtr, &allocRequest);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal vtcamDbSegmentTableVTcamDownSize__LogicalIndexMode function
* @endinternal
*
* @brief   logical Index Mode :
*         down size memory for Virtual TCAM in Segment Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] downSizeValue            -
*                                      value > 0 --> down size to smaller size
* @param[in] downSizeReferenceLogicalIndex - down size reference logical index
*                                      relevant only when downSizeValue != 0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
static GT_STATUS vtcamDbSegmentTableVTcamDownSize__LogicalIndexMode
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_U32                                      downSizeValue,
    IN  GT_U32                                      downSizeReferenceLogicalIndex
)
{
    GT_STATUS   rc;
    GT_U32  ii,iiMax;
    GT_U32  src_logicalIndex,srcDeviceRuleIndex;
    GT_U32  dst_logicalIndex,trgDeviceRuleIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *vTcamMngDBPtr;
    GT_U32   srcExists;
    GT_U32   dstExists;

    vTcamMngDBPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    vTcamInfoPtr  = vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId];

    /* the vTcamInfoPtr->usedRulesAmount should not be changed during the operation */

    if((downSizeReferenceLogicalIndex + downSizeValue) > vTcamInfoPtr->rulesAmount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (vTcamInfoPtr->tcamInfo.tcamSpaceUnmovable != GT_FALSE)
    {
        /* unmovable vTCAM cannot be resized */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"vTcamId[%d] unmovable \n", vTcamId);
    }

    iiMax = vTcamInfoPtr->rulesAmount - downSizeValue;
    ii = downSizeReferenceLogicalIndex;

    /* invalidate existing rules at the decreased area,
     *   while moving higher numbered rules to fill the gap */
    for(/*already initialized*/ ; ii < iiMax ;ii++)
    {
        src_logicalIndex = ii + downSizeValue;
        dst_logicalIndex = ii;

        srcDeviceRuleIndex = vTcamInfoPtr->rulePhysicalIndexArr[src_logicalIndex];
        trgDeviceRuleIndex = vTcamInfoPtr->rulePhysicalIndexArr[dst_logicalIndex];

        /* check if 'src' was used index */
        srcExists = (vTcamInfoPtr->usedRulesBitmapArr[src_logicalIndex>>5] & (1<<(src_logicalIndex & 0x1f)));
        /* check if destination exists in DB */
        dstExists = (vTcamInfoPtr->usedRulesBitmapArr[dst_logicalIndex>>5] & (1<<(dst_logicalIndex & 0x1f)));

        if (dstExists)
        {
            if(vTcamInfoPtr->usedRulesAmount == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB error: usedRulesAmount : can not decrement from ZERO ");
            }
            if (srcExists)
            {
                /* the destination rule will be overriden by source */
                /* not need to delete by cpssDxChVirtualTcamRuleDelete */
                vTcamInfoPtr->usedRulesAmount --;
                vTcamInfoPtr->usedRulesBitmapArr[dst_logicalIndex>>5] &= (~ (1<<(dst_logicalIndex & 0x1f)));
            }
            else
            {
                /* remove the index in HW and in DB */
                /* Note: this not done in case source exists, because move
                   operation below fixed to be atomically */
                rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,vTcamId, dst_logicalIndex);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        if (srcExists)
        {
            /* move the index in HW and in DB */
            rc = ruleMoveByLogicalIndex(vTcamMngId,vTcamInfoPtr,
                src_logicalIndex,srcDeviceRuleIndex,
                dst_logicalIndex,trgDeviceRuleIndex,
                GT_TRUE/*calledFromResize*/);

            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* continue invalidating existing rules until the end of the decreased area,
     *   in case moving rules  didn't cover all of the decreased area */
    iiMax = downSizeReferenceLogicalIndex + downSizeValue;
    for(/*already initialized*/ ; ii < iiMax ;ii++)
    {
        dst_logicalIndex = ii;

        trgDeviceRuleIndex = vTcamInfoPtr->rulePhysicalIndexArr[dst_logicalIndex];

        /* check is destination exists in DB */
        dstExists = (vTcamInfoPtr->usedRulesBitmapArr[dst_logicalIndex>>5] & (1<<(dst_logicalIndex & 0x1f)));

        if(dstExists)
        {
            /* remove the index in HW and in DB */
            rc = cpssDxChVirtualTcamRuleDelete(vTcamMngId,vTcamId, dst_logicalIndex);

            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* after the entries not occupy the last indices any more , we need to
       restore them into the 'free' segments

       the function will update vTcamInfoPtr->rulesAmount
    */
    rc = prvCpssDxChVirtualTcamDbSegmentTableDetachSpace(vTcamInfoPtr->tcamSegCfgPtr,
        vTcamInfoPtr, GT_TRUE,/*from the end*/
        (vTcamInfoPtr->rulesAmount - downSizeValue), NULL);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    if(vTcamInfoPtr->rulesAmount < vTcamInfoPtr->tcamInfo.guaranteedNumOfRules)
    {
        vTcamInfoPtr->tcamInfo.guaranteedNumOfRules = vTcamInfoPtr->rulesAmount;
    }

    return GT_OK;
}

/**
* @internal priorityTreeDownSizeRangeUpdate function
* @endinternal
*
* @brief   (for resize purposes of down size)
*         move the 'old' logical index rules to 'new' logical index.
*         according to needed amount of moves
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamInfoPtr             - (pointer to) virtual TCAM structure
* @param[in] fromEndOrStart           - the type of direction
*                                      GT_TRUE  - hole from end   of range
*                                      GT_FALSE - hole from start of range
* @param[in] firstTargetLogicalIndex  - the first logical index to move used indexes to it
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on Cpu memory allocation failed.
* @retval GT_BAD_STATE             - on DB management error
*/
static GT_STATUS priorityTreeDownSizeRangeUpdate(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr,
    IN   GT_BOOL                                    fromEndOrStart,
    IN  GT_U32                                      firstTargetLogicalIndex
)
{
    GT_U32  *arrPtr = vTcamInfoPtr->usedRulesBitmapArr;
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *currentPriorityEntryPtr = NULL;
    PRV_CPSS_AVL_TREE_FIND_ENT avlFoundStartType;
    PRV_CPSS_AVL_TREE_SEEK_ENT avlSeekStartType;
    PRV_CPSS_AVL_TREE_SEEK_ENT avlNextType;
    GT_BOOL found;
    GT_U32  currentLogIndex;
    GT_U32  boundLogIndex;
    GT_U32  value;
    GT_U32  numOfHolesTotal;
    GT_U32  numOfHolesPrevPos;

    if (fromEndOrStart != GT_FALSE)
    {
        avlFoundStartType = PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E;
        avlSeekStartType  = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        avlNextType       = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
    }
    else
    {
        avlFoundStartType = PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E;
        avlSeekStartType  = PRV_CPSS_AVL_TREE_SEEK_LAST_E;
        avlNextType       = PRV_CPSS_AVL_TREE_SEEK_PREVIOUS_E;
    }

    priorityEntry.priority = 0; /*dummy*/
    priorityEntry.rangeSize = 0; /*dummy*/
    priorityEntry.baseLogIndex = firstTargetLogicalIndex;
    found = prvCpssAvlPathFind(
        vTcamInfoPtr->priorityIndexTree, avlFoundStartType,
        &priorityEntry, avlTreePath, &dbEntryPtr);
    if (found == GT_FALSE)
    {
        found = prvCpssAvlPathSeek(
            vTcamInfoPtr->priorityIndexTree, avlSeekStartType,
            avlTreePath, &dbEntryPtr);
    }
    if (found == GT_FALSE)
    {
        /* should not occur, but nothing to do */
        return GT_OK;
    }
    currentPriorityEntryPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *)dbEntryPtr;

    if (((fromEndOrStart != GT_FALSE) 
        && ((currentPriorityEntryPtr->baseLogIndex + currentPriorityEntryPtr->rangeSize) <= firstTargetLogicalIndex))
        || ((fromEndOrStart == GT_FALSE) && (currentPriorityEntryPtr->baseLogIndex > firstTargetLogicalIndex)))
    {
        found = prvCpssAvlPathSeek(
            vTcamInfoPtr->priorityIndexTree, avlNextType,
            avlTreePath, &dbEntryPtr);
    }
    if (found == GT_FALSE)
    {
        /* should not occur, but nothing to do */
        return GT_OK;
    }

    currentPriorityEntryPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *)dbEntryPtr;
    currentLogIndex  = firstTargetLogicalIndex;
    numOfHolesTotal = 0;
    if (fromEndOrStart != GT_FALSE)
    {
        /* firstTargetLogicalIndex inside first found range */
        if (currentLogIndex > currentPriorityEntryPtr->baseLogIndex)
        {
            boundLogIndex = (currentPriorityEntryPtr->baseLogIndex + currentPriorityEntryPtr->rangeSize);
            for (/*no init*/; (currentLogIndex < boundLogIndex); currentLogIndex++)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, currentLogIndex, 1, value);
                if (value == 0)
                {
                    numOfHolesTotal ++;
                }
            }
            currentPriorityEntryPtr->rangeSize -= numOfHolesTotal;
            found = prvCpssAvlPathSeek(
                vTcamInfoPtr->priorityIndexTree, avlNextType,
                avlTreePath, &dbEntryPtr);
            if (found == GT_FALSE)
            {
                return GT_OK;
            }
        }
    }
    else
    {
        /* firstTargetLogicalIndex inside first found range */
        if (currentLogIndex < (currentPriorityEntryPtr->baseLogIndex + currentPriorityEntryPtr->rangeSize))
        {
            boundLogIndex = currentPriorityEntryPtr->baseLogIndex;
            for (/*no init*/; (currentLogIndex >= boundLogIndex); currentLogIndex--)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, currentLogIndex, 1, value);
                if (value == 0)
                {
                    numOfHolesTotal ++;
                }
                if (currentLogIndex == 0) break; /* to prtevent using negative values */
            }
            currentPriorityEntryPtr->rangeSize    -= numOfHolesTotal;
            currentPriorityEntryPtr->baseLogIndex += numOfHolesTotal;
            found = prvCpssAvlPathSeek(
                vTcamInfoPtr->priorityIndexTree, avlNextType,
                avlTreePath, &dbEntryPtr);
            if (found == GT_FALSE)
            {
                return GT_OK;
            }
        }
    }

    while (1)
    {
        currentPriorityEntryPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *)dbEntryPtr;

        if (fromEndOrStart != GT_FALSE)
        {
            boundLogIndex = currentPriorityEntryPtr->baseLogIndex;
            for (/*no init*/; (currentLogIndex < boundLogIndex); currentLogIndex++)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, currentLogIndex, 1, value);
                if (value == 0)
                {
                    numOfHolesTotal ++;
                }
            }
            numOfHolesPrevPos = numOfHolesTotal;
            boundLogIndex = (currentPriorityEntryPtr->baseLogIndex + currentPriorityEntryPtr->rangeSize);
            for (/*no init*/; (currentLogIndex < boundLogIndex); currentLogIndex++)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, currentLogIndex, 1, value);
                if (value == 0)
                {
                    numOfHolesTotal ++;
                }
            }
            currentPriorityEntryPtr->baseLogIndex -= numOfHolesTotal;
            currentPriorityEntryPtr->rangeSize -= (numOfHolesTotal - numOfHolesPrevPos);
        }
        else
        {
            boundLogIndex = (currentPriorityEntryPtr->baseLogIndex + currentPriorityEntryPtr->rangeSize);
            for (/*no init*/; (currentLogIndex >= boundLogIndex); currentLogIndex--)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, currentLogIndex, 1, value);
                if (value == 0)
                {
                    numOfHolesTotal ++;
                }
                if (currentLogIndex == 0) break; /* to prtevent using negative values */
            }
            numOfHolesPrevPos = numOfHolesTotal;
            boundLogIndex = currentPriorityEntryPtr->baseLogIndex;
            for (/*no init*/; (currentLogIndex >= boundLogIndex); currentLogIndex--)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, currentLogIndex, 1, value);
                if (value == 0)
                {
                    numOfHolesTotal ++;
                }
                if (currentLogIndex == 0) break; /* to prtevent using negative values */
            }
            currentPriorityEntryPtr->rangeSize    -= (numOfHolesTotal - numOfHolesPrevPos);
            currentPriorityEntryPtr->baseLogIndex += numOfHolesTotal;
        }
        found = prvCpssAvlPathSeek(
            vTcamInfoPtr->priorityIndexTree, avlNextType,
            avlTreePath, &dbEntryPtr);
        if (found == GT_FALSE)
        {
            return GT_OK;
        }
    }
    /* return from loop upper - here not needed */
}

/**
* @internal priorityTreeDownSizeRangeUpdate_InPlace function
* @endinternal
*
* @brief   (for resize purposes of down size - and only for the special case of no
*         HW moves need, only logical index moves needed)
*         move the 'old' logical index rules to 'new' logical index.
*         according to needed amount of moves
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamInfoPtr             - (pointer to) virtual TCAM structure
* @param[in] downSizeValue            -
*                                      value > 0 --> down size to smaller size
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on Cpu memory allocation failed.
* @retval GT_BAD_STATE             - on DB management error
*
* @note In special case handled by this function, priority mapping trees can be
*       updated in place instead of removing item and inserting again.
*
*/
static GT_STATUS priorityTreeDownSizeRangeUpdate_InPlace(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr,
    IN  GT_U32                                      downSizeValue
)
{
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL                 found;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *currentPriorityEntryPtr = NULL;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *previousPriorityEntryPtr = NULL;

    /* keep loop of priorities and update their base */

    found = prvCpssAvlPathSeek(vTcamInfoPtr->priorityPriTree,
                PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                avlTreePath,
                &dbEntryPtr /* use dedicated var to avoid warnings */);
    while(found)
    {
        currentPriorityEntryPtr = dbEntryPtr;

        /* Check existing correct ordering - should be same order by log. index than by prio. */
        if (previousPriorityEntryPtr != NULL)
        {
            if ((previousPriorityEntryPtr->baseLogIndex +
                    previousPriorityEntryPtr->rangeSize) >
                (currentPriorityEntryPtr->baseLogIndex -
                    downSizeValue))
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

        /* Update the tree entry - it should still preserve correct tree ordering */
        if(downSizeValue < currentPriorityEntryPtr->baseLogIndex)
        {
            currentPriorityEntryPtr->baseLogIndex -= downSizeValue;
        }
        else
        if(downSizeValue < (currentPriorityEntryPtr->baseLogIndex +
                            currentPriorityEntryPtr->rangeSize))
        {
            currentPriorityEntryPtr->rangeSize -= (downSizeValue -
                                            currentPriorityEntryPtr->baseLogIndex);
            currentPriorityEntryPtr->baseLogIndex = 0;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        previousPriorityEntryPtr = currentPriorityEntryPtr;

        found = prvCpssAvlPathSeek(vTcamInfoPtr->priorityPriTree,
                    PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    avlTreePath,
                    &dbEntryPtr /* use dedicated var to avoid warnings */);
    }

    return GT_OK;
}

/**
* @internal priorityDownSizeMoveLogicalIndexes function
* @endinternal
*
* @brief   (for resize purposes of down size)
*         move the 'old' logical index rules to 'new' logical index.
*         according to needed amount of moves
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamInfoPtr             - (pointer to) virtual TCAM structure
* @param[in] fromEndOrStart           - the type of direction
*                                      GT_TRUE  - hole from end   of range
*                                      GT_FALSE - hole from start of range
* @param[in] numOfHwMoves             - number of valid rules to move
* @param[in] firstTargetLogicalIndex  - the first logical index to move used indexes to it
*
* @param[out] lastIndexMovedPtr        - (pointer to) the last index that was used.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on Cpu memory allocation failed.
* @retval GT_BAD_STATE             - on DB management error
*/
static GT_STATUS priorityDownSizeMoveLogicalIndexes(
    IN GT_U32                                       vTcamMngId,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr,
    IN   GT_BOOL                                    fromEndOrStart,
    IN  GT_U32                                      numOfHwMoves,
    IN  GT_U32                                      firstTargetLogicalIndex,
    OUT GT_U32                                      *lastIndexMovedPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii,iiStart,iiLast;
    GT_U32  jj;
    GT_U32  *arrPtr = vTcamInfoPtr->usedRulesBitmapArr;
    GT_U32  value;
    GT_U32  numOfMovedEntries = 0;/* number of entries that we moved in HW */
    GT_U32  src_logicalIndex,srcDeviceRuleIndex;
    GT_U32  dst_logicalIndex,trgDeviceRuleIndex;
    GT_BOOL didMove;

    if(fromEndOrStart == GT_TRUE)
    {
        /* start from the end : direction of move is 'down' (from high to low index) */
        iiStart = firstTargetLogicalIndex;
        iiLast  = vTcamInfoPtr->rulesAmount;
        jj = iiStart + 1;

        /* find rules that need the move */
        for(ii = iiStart ; ii < iiLast ; ii++)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, ii,1,value);
            if(value)
            {
                /* when ii == iiStart the bit was found as empty by the caller */
                /* for other ii our call to ruleMoveByLogicalIndex(...)
                   should have evacuated this bit already ! */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            didMove = GT_FALSE;
            /* index ii is empty , we need index to occupy it */
            for(/*continue*/ ; (jj < iiLast) && (didMove == GT_FALSE) ; jj++)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, jj,1,value);
                if(value == 0)
                {
                    continue;
                }
                /* index jj is used ... need to move the entry from jj to index ii */
                src_logicalIndex = jj;
                dst_logicalIndex = ii;
                /* convert the logical index to physical index in the HW */
                srcDeviceRuleIndex = vTcamInfoPtr->rulePhysicalIndexArr[src_logicalIndex];
                trgDeviceRuleIndex = vTcamInfoPtr->rulePhysicalIndexArr[dst_logicalIndex];

                rc = ruleMoveByLogicalIndex(vTcamMngId,vTcamInfoPtr,
                    src_logicalIndex,srcDeviceRuleIndex,
                    dst_logicalIndex,trgDeviceRuleIndex,
                    GT_TRUE/*calledFromResize*/);

                if(rc != GT_OK)
                {
                    return rc;
                }

                didMove = GT_TRUE;/* indication to update jj
                                     and break the loop */
            }

            if(didMove == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            numOfMovedEntries++;

            if(numOfMovedEntries == numOfHwMoves)
            {
                /* we are done with the moves */
                *lastIndexMovedPtr = ii;/* the last move was into this index */
                return GT_OK;
            }
        }
    }
    else
    {
        if(firstTargetLogicalIndex == 0)
        {
            /* can't search indexes to move */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* start from the start : direction of move is 'up' (from low to high index) */
        iiStart = firstTargetLogicalIndex;
        jj = iiStart - 1;

        /* find rules that need the move */
        for(ii = iiStart ; /*no criteria*/ ; ii--)
        {
            U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, ii,1,value);
            if(value)
            {
                /* when ii == iiStart the bit was found as empty by the caller */
                /* for other ii our call to ruleMoveByLogicalIndex(...)
                   should have evacuated this bit already ! */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            didMove = GT_FALSE;
            /* index ii is empty , we need index to occupy it */
            for(/*continue*/ ; (didMove == GT_FALSE) ; jj--)
            {
                U32_GET_FIELD_IN_ENTRY_MAC(arrPtr, jj,1,value);
                if(value == 0)
                {
                    if(jj == 0)
                    {
                        break;
                    }

                    continue;
                }
                /* index jj is used ... need to move the entry from jj to index ii */
                src_logicalIndex = jj;
                dst_logicalIndex = ii;
                /* convert the logical index to physical index in the HW */
                srcDeviceRuleIndex = vTcamInfoPtr->rulePhysicalIndexArr[src_logicalIndex];
                trgDeviceRuleIndex = vTcamInfoPtr->rulePhysicalIndexArr[dst_logicalIndex];

                rc = ruleMoveByLogicalIndex(vTcamMngId,vTcamInfoPtr,
                    src_logicalIndex,srcDeviceRuleIndex,
                    dst_logicalIndex,trgDeviceRuleIndex,
                    GT_TRUE/*calledFromResize*/);

                if(rc != GT_OK)
                {
                    return rc;
                }

                didMove = GT_TRUE;/* indication to update jj
                                     and break the loop */

                if(jj == 0)
                {
                    break;
                }
            }

            if(didMove == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            numOfMovedEntries++;

            if(numOfMovedEntries == numOfHwMoves)
            {
                /* we are done with the moves */
                *lastIndexMovedPtr = ii;/* the last move was into this index */
                return GT_OK;
            }

            if(ii == 0)
            {
                break;
            }

        }
    }

    /* not moved number of needed entries */

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
}


/**
* @internal vtcamDbSegmentTableVTcamDownSize__PriorityMode function
* @endinternal
*
* @brief   priority Mode :
*         down size memory for Virtual TCAM in Segment Table for TCAM.
*         NOTE: the operation start by trying to reduce rules from the specified
*         place , but if not enough empty space there ... it will look for
*         empty space at other points for removal.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] downSizeValue            -
*                                      value > 0 --> down size to smaller size
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
static GT_STATUS vtcamDbSegmentTableVTcamDownSize__PriorityMode
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_U32                                      downSizeValue
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *vTcamMngDBPtr;
    GT_U32  numHwMovesForHoleAtEnd;  /* number of HW moves needed to create hole at the end   of the range */
    GT_U32  numHwMovesForHoleAtStart;/* number of HW moves needed to create hole at the start of the range*/
    GT_U32  targetIndexEndSide,targetIndexStartSide;/* the first index to get moved HW rule */
    GT_U32  targetIndex;
    GT_U32  lastIndexMoved = 0;/* the last logical index that was moved */
    GT_BOOL fromEndOrStart;/* indication that the hole is at the start/end */
    GT_U32  numOfHwMoves;
    GT_U32  rulesAmountGranularity;
    GT_U32  numOfDetachedRules;

    /* the vTcamInfoPtr->usedRulesAmount should not be changed during the operation */

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    vTcamInfoPtr  = vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId];

    rulesAmountGranularity =
        prvCpssDxChVirtualTcamDbSegmentTableVTcamRuleAmountGranularity(
            vTcamInfoPtr->tcamSegCfgPtr, vTcamInfoPtr->tcamInfo.ruleSize);
    downSizeValue -= (downSizeValue % rulesAmountGranularity);
    if (downSizeValue ==0)
    {
        return GT_OK;
    }

    if(downSizeValue > vTcamInfoPtr->rulesAmount)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (vTcamInfoPtr->tcamInfo.tcamSpaceUnmovable != GT_FALSE)
    {
        /* unmovable vTCAM cannot be resized */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"vTcamId[%d] unmovable \n", vTcamId);
    }

    if(vTcamInfoPtr->rulesAmount < (vTcamInfoPtr->usedRulesAmount + downSizeValue))
    {
        /* not enough rules to remove (totally ... not only from specific please) */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* make decision : compress 'up' or compress 'down'
       compress 'up'   means that we need to leave first downSizeValue indexes as free.
       compress 'down' means that we need to leave last  downSizeValue indexes as free.
    */
    /* calc 'cost' from the end */
    rc = numHwMovesForHoleCalc(vTcamInfoPtr,
            GT_TRUE/* from the end */,
            downSizeValue, /* size of hole */
            &numHwMovesForHoleAtEnd,
            &targetIndexEndSide);
    if(rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        return rc;
    }

    /* initial value more than numHwMovesForHoleAtEnd */
    numHwMovesForHoleAtStart = 0xFFFFFFFF;
    if (numHwMovesForHoleAtEnd != 0)
    {
        /* calc 'cost' from the start */
        rc = numHwMovesForHoleCalc(vTcamInfoPtr,
                GT_FALSE/* from the start */,
                downSizeValue, /* size of hole */
                &numHwMovesForHoleAtStart,
                &targetIndexStartSide);
        if(rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            return rc;
        }
    }

    if (numHwMovesForHoleAtEnd < numHwMovesForHoleAtStart)
    {
        /* making the hole at the end of the range is better (than at the start)*/
        fromEndOrStart = GT_TRUE;/*from end*/
        numOfHwMoves = numHwMovesForHoleAtEnd;
        targetIndex = targetIndexEndSide;
    }
    else
    {
        /* making the hole at the start of the range is better (than at the end)*/
        fromEndOrStart = GT_FALSE;/*from start*/
        numOfHwMoves = numHwMovesForHoleAtStart;
        targetIndex = targetIndexStartSide;
    }

    if (numOfHwMoves != 0)
    {
        /* update the priority ranges */
        rc = priorityTreeDownSizeRangeUpdate(
            vTcamInfoPtr,
            fromEndOrStart,
            targetIndex);
        if(rc != GT_OK)
        {
            return rc;
        }

        /*
           move rules from the 'old' logical index rules to 'new' logical index.
           according to needed amount of moves, Update rule ID DB.
           Priority DB already was updated.
        */
        rc = priorityDownSizeMoveLogicalIndexes(vTcamMngId,
            vTcamInfoPtr,
            fromEndOrStart,
            numOfHwMoves,
            targetIndex,
            &lastIndexMoved);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(fromEndOrStart == GT_TRUE  &&/*from end */
           lastIndexMoved > (vTcamInfoPtr->rulesAmount - downSizeValue))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
        else
        if(fromEndOrStart == GT_FALSE  &&/*from start */
           lastIndexMoved > downSizeValue)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    /* after the entries not occupy the last indexes any more , we need to
       restore them into the 'free' segments
       the function will update vTcamInfoPtr->rulesAmount
       The real amount of detached rule can be less than required.
    */
    rc = prvCpssDxChVirtualTcamDbSegmentTableDetachSpace(vTcamInfoPtr->tcamSegCfgPtr,
        vTcamInfoPtr, fromEndOrStart,
        (vTcamInfoPtr->rulesAmount - downSizeValue), &numOfDetachedRules);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    if (vTcamInfoPtr->rulesAmount < vTcamInfoPtr->tcamInfo.guaranteedNumOfRules)
    {
        vTcamInfoPtr->tcamInfo.guaranteedNumOfRules = vTcamInfoPtr->rulesAmount;
    }

    if (fromEndOrStart == GT_FALSE)
    {
        /* update the priority ranges */
        rc = priorityTreeDownSizeRangeUpdate_InPlace(
            vTcamInfoPtr,
            numOfDetachedRules);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* update vTcamInfoPtr->usedRulesBitmapArr[] */
        copyBitsInMemory(vTcamInfoPtr->usedRulesBitmapArr,
            0,/*targetStartBit*/
            numOfDetachedRules,/*sourceStartBit*/
            vTcamInfoPtr->rulesAmount); /*numBits*/

        /* update the content of next trees that hold logical index values:
            vTcamInfoPtr->ruleIdIndexTree
        */
        rc = priorityResizeUpdateTrees_FromRemove(
            vTcamInfoPtr, numOfDetachedRules);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal vtcamDbSegmentTableVTcamResize function
* @endinternal
*
* @brief   Allocate/resize memory for Virtual TCAM in Segment Table for TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] resizeValue              - value == 0 --> no resize (initial alloc)
*                                      value > 0 --> resize to bigger size
* @param[in] resizeMode               - resize mode (with/without index)
*                                      relevant only when resizeValue != 0
* @param[in] resizeReferenceLogicalIndex - resize reference logical index
*                                      relevant only when resizeValue != 0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*/
static GT_STATUS vtcamDbSegmentTableVTcamResize
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_U32                                      resizeValue,
    IN  RESIZE_MODE_ENT                             resizeMode,
    IN  GT_U32                                      resizeReferenceLogicalIndex
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vTcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC tmp_vTcamInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *alloc_vTcamInfoPtr = &tmp_vTcamInfo;
    GT_U32  actualResizeValue;/*actual resize value (due to granularity issues)*/
    GT_U32  moveResizeValue;/*resize value for rules move*/

    if (resizeValue == 0) return GT_OK;

    /* get vtcam DB */
    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    vTcamInfoPtr  = vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId];

    /* allocate to temp info */
    cpssOsMemSet(&tmp_vTcamInfo,0,sizeof(tmp_vTcamInfo));

    if (vTcamInfoPtr->tcamInfo.tcamSpaceUnmovable != GT_FALSE)
    {
        /* unmovable vTCAM cannot be resized */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"vTcamId[%d] unmovable \n", vTcamId);
    }

    rc =  vtcamDbSegmentTableVTcamAllocOrResize( vTcamMngDBPtr, vTcamId,
        resizeValue,
        &tmp_vTcamInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* update the number of rules needed to be 'guaranteed' */
    vTcamInfoPtr->tcamInfo.guaranteedNumOfRules += resizeValue;

    actualResizeValue = tmp_vTcamInfo.rulesAmount;
    /* function prvCpssDxChVirtualTcamDbSegmentTableVTcamAllocate Filled next in the vtcam :
        alloc_vTcamInfoPtr->segmentsTree
        alloc_vTcamInfoPtr->rulesAmount
        alloc_vTcamInfoPtr->rulePhysicalIndexArr[]

        all those only for the 'added size'
    */

    if (tmp_vTcamInfo.rulesAmount != 0)
    {
        /* free this memory as it MUST not be used */
        cpssOsFree(alloc_vTcamInfoPtr->rulePhysicalIndexArr);
        alloc_vTcamInfoPtr->rulePhysicalIndexArr = NULL;
    }

    /* map HW index to logical indexes of the 'old' logical index */
    rc = resizeMapHwIndexesToLogicalIndexes(vTcamInfoPtr,GT_TRUE/*old*/);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    if (tmp_vTcamInfo.rulesAmount != 0)
    {
        /* function prvCpssDxChVirtualTcamDbSegmentTableVTcamCombine will update
           vTcamInfoPtr->rulesAmount  */

        /* bind the new segmentsTree from alloc_vTcamInfoPtr into vTcamInfoPtr */
        /* and align the info in the tree of vTcamInfoPtr->segmentsTree */
        rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamCombine(
            vTcamInfoPtr,alloc_vTcamInfoPtr);

        /* remove temporary tree of added segments */
        prvCpssAvlTreeDelete(
            alloc_vTcamInfoPtr->segmentsTree,
            (GT_VOIDFUNCPTR)NULL,
            (GT_VOID*)NULL);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    /* map HW index to logical indexes of the 'new' logical index */
    rc = resizeMapHwIndexesToLogicalIndexes(vTcamInfoPtr,GT_FALSE/*new*/);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    if (tmp_vTcamInfo.rulesAmount != 0)
    {
        if(vTcamInfoPtr->tcamInfo.ruleAdditionMethod ==
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
        {
            /* update the content of next trees that hold logical indexes values:
                vTcamInfoPtr->ruleIdIndexTree,vTcamInfoPtr->priorityIndexTree
                it will use the DB of prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[]
            */
            rc = priorityResizeUpdateTrees(vTcamInfoPtr,
                actualResizeValue,resizeMode,resizeReferenceLogicalIndex);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
            }
        }

        rc = resizeUpdateLogicalIndexBmp(vTcamInfoPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    /* update the content of next BMP that hold logical indexes values :
        vTcamInfoPtr->usedRulesBitmapArr
        it will use the DB of prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[]
    */
    if(vTcamInfoPtr->tcamInfo.ruleAdditionMethod ==
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        moveResizeValue = actualResizeValue;
    }
    else
    {
        moveResizeValue = resizeValue;
    }

    if (moveResizeValue != 0)
    {
        /* move logical indexes to their final position in the DB and in the HW
           with the assurance of the resizeValue
           it will use the DB of prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[]
        */
        rc = resizeMoveLogicalIndexes(vTcamMngId,vTcamInfoPtr,
            moveResizeValue,resizeMode,resizeReferenceLogicalIndex);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    return rc;
}

/*******************************************************************************/
/*******************************************************************************/
/******************** end DB operation *****************************************/
/*******************************************************************************/
/*******************************************************************************/

/*******************************************************************************/
/*******************************************************************************/
/****************** start 'per device' functions *******************************/
/*******************************************************************************/
/*******************************************************************************/


/**
* @internal perDevice_ruleWrite function
* @endinternal
*
* @brief   Write Rule into the TCAM of specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       Read only from first active port group of the bitmap.
* @param[in] tcamIndex                - the TCAM index (APPLICABLE DEVICES: AC5)
* @param[in] ruleIndex                - the rule index in the TCAM. (CPSS API physical index)
* @param[in] ruleTypePtr              -     (pointer to)type of contents of rule
* @param[in] ruleDataPtr              -     (pointer to)data of contents of rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS perDevice_ruleWrite
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U32                                      tcamIndex,
    IN  GT_U32                                      ruleIndex,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PCL_RULE_OPTION_ENT ruleOptionsBmp;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT ruleSize;  /* Rule size derived from PCL rule format. Used only on xCat3 PCL */
    GT_U32                        rulePclIndex;     /* Rule index, adjusted according to underlying API and rule size */

    switch(ruleTypePtr->ruleType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            /* The PCL API supports the creation of rule as 'not valid' */
            if(ruleDataPtr->valid == GT_FALSE)
            {
                ruleOptionsBmp = CPSS_DXCH_PCL_RULE_OPTION_WRITE_INVALID_E;
            }
            else
            {
                ruleOptionsBmp = 0;
            }

            rc = prvCpssDxChVirtualTcamRuleFormatToRuleSize(ruleTypePtr->rule.pcl.ruleFormat,
                                                                &ruleSize);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssDxChVirtualTcamRuleIndexToRulePclIndex(devNum,
                                                                ruleSize,
                                                                ruleIndex,
                                                                &rulePclIndex);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChPclPortGroupRuleSet(
                devNum, portGroupsBmp, tcamIndex,
                ruleTypePtr->rule.pcl.ruleFormat,
                rulePclIndex,
                ruleOptionsBmp,
                ruleDataPtr->rule.pcl.maskPtr,
                ruleDataPtr->rule.pcl.patternPtr,
                ruleDataPtr->rule.pcl.actionPtr
                );
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            rc = cpssDxChTtiPortGroupRuleSet(
                devNum, portGroupsBmp,
                ruleIndex,
                ruleTypePtr->rule.tti.ruleFormat,
                ruleDataPtr->rule.tti.patternPtr,
                ruleDataPtr->rule.tti.maskPtr,
                ruleDataPtr->rule.tti.actionPtr
                );
            if(rc != GT_OK)
            {
                return rc;
            }

            /* The TTI API NOT supports the creation of rule as 'not valid' */
            /* so we invalidate the rule after it's creation */
            if(ruleDataPtr->valid == GT_FALSE)
            {
                rc = cpssDxChTtiPortGroupRuleValidStatusSet(
                    devNum, portGroupsBmp,
                    ruleIndex,
                    GT_FALSE);
            }

            break;
        default:
            rc = GT_BAD_PARAM;
            break;
    }

    return rc;

}

/**
* @internal perDevice_ruleRead function
* @endinternal
*
* @brief   Read Rule from the TCAM of specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       Read only from first active port group of the bitmap.
* @param[in] tcamIndex                - TCAM index (APPLICABLE DEVICES: AC5)
* @param[in] ruleIndex                - the rule index in the TCAM. (CPSS API physical index)
* @param[in] ruleTypePtr              -     (pointer to)type of contents of rule
*
* @param[out] ruleDataPtr              -     (pointer to)data of contents of rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS perDevice_ruleRead
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U32                                      tcamIndex,
    IN  GT_U32                                      ruleIndex,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
)
{
    GT_STATUS rc;
    GT_U32                        rulePclIndex;     /* Rule index, adjusted according to underlying API and rule size */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT ruleSize;  /* Rule size derived from PCL rule format. Used only on xCat3 PCL */

    switch(ruleTypePtr->ruleType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:

            rc = prvCpssDxChVirtualTcamRuleFormatToRuleSize(ruleTypePtr->rule.pcl.ruleFormat,
                                                                &ruleSize);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = prvCpssDxChVirtualTcamRuleIndexToRulePclIndex(devNum,
                                                                ruleSize,
                                                                ruleIndex,
                                                                &rulePclIndex);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChPclPortGroupRuleParsedGet(
                devNum, portGroupsBmp, tcamIndex,
                ruleTypePtr->rule.pcl.ruleFormat,
                rulePclIndex,
                0,/*ruleOptionsBmp -- unused parameter */
                &ruleDataPtr->valid,
                ruleDataPtr->rule.pcl.maskPtr,
                ruleDataPtr->rule.pcl.patternPtr,
                ruleDataPtr->rule.pcl.actionPtr
                );
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            rc = cpssDxChTtiPortGroupRuleGet(
                devNum, portGroupsBmp,
                ruleIndex,
                ruleTypePtr->rule.tti.ruleFormat,
                ruleDataPtr->rule.tti.patternPtr,
                ruleDataPtr->rule.tti.maskPtr,
                ruleDataPtr->rule.tti.actionPtr
                );
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChTtiPortGroupRuleValidStatusGet(
                devNum, portGroupsBmp,
                ruleIndex,
                &ruleDataPtr->valid);

            break;
        default:
            rc = GT_BAD_PARAM;
            break;
    }

    return rc;

}

/**
* @internal perDevice_ruleActionUpdate function
* @endinternal
*
* @brief   Update the action of a Rule in the TCAM of specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       Read only from first active port group of the bitmap.
* @param[in] tcamIndex                - the TCAM index for AC5 devices
* @param[in] ruleIndex                - the rule index in the TCAM. (CPSS API physical index)
* @param[in] ruleSize                 - the rule size, used to compute actual HW rule index in certain devices
* @param[in] actionTypePtr            - (pointer to)type of contents of action
* @param[in] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS perDevice_ruleActionUpdate
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U32                                      tcamIndex,
    IN  GT_U32                                      ruleIndex,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
)
{
    GT_STATUS rc;
    CPSS_PCL_RULE_SIZE_ENT        pclRuleSize;      /* Rule size derived from vTcam rule size. Used only on xCat3 PCL */
    GT_U32                        rulePclIndex;     /* Rule index, adjusted according to underlying API and rule size */

    switch(actionTypePtr->ruleType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            CPSS_NULL_PTR_CHECK_MAC(actionDataPtr->action.pcl.actionPtr);

            if(actionTypePtr->action.pcl.direction != CPSS_PCL_DIRECTION_INGRESS_E &&
               actionTypePtr->action.pcl.direction != CPSS_PCL_DIRECTION_EGRESS_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if(actionTypePtr->action.pcl.direction == CPSS_PCL_DIRECTION_INGRESS_E &&
               actionDataPtr->action.pcl.actionPtr->egressPolicy != GT_FALSE)
            {
                /* the 'direction' state 'ingress' , but the 'egressPolicy' not state it */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if(actionTypePtr->action.pcl.direction == CPSS_PCL_DIRECTION_EGRESS_E &&
               actionDataPtr->action.pcl.actionPtr->egressPolicy != GT_TRUE)
            {
                /* the 'direction' state 'egress' , but the 'egressPolicy' not state it */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            switch(ruleSize)
            {
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_10_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_20_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_30_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_40_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_50_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_60_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_80_BYTES_E; break;

                default:
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDxChVirtualTcamRuleIndexToRulePclIndex(devNum,
                                                                ruleSize,
                                                                ruleIndex,
                                                                &rulePclIndex);
            if (rc != GT_OK)
            {
                return rc;
            }


            rc = cpssDxChPclPortGroupRuleActionUpdate(
                devNum, portGroupsBmp, tcamIndex,
                pclRuleSize,/* - not used in bc2 and above */
                rulePclIndex,
                actionDataPtr->action.pcl.actionPtr
                );
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            rc = cpssDxChTtiPortGroupRuleActionUpdate(
                devNum, portGroupsBmp,
                ruleIndex,
                actionDataPtr->action.tti.actionPtr
                );
            break;
        default:
            rc = GT_BAD_PARAM;
            break;
    }

    return rc;

}

/* the size of tti action type 2 in words */
#define TTI_ACTION_TYPE_2_SIZE_CNS      8
extern GT_STATUS prvCpssDxChTtiActionHw2LogicFormat
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              *hwFormatArray,
    OUT CPSS_DXCH_TTI_ACTION_STC            *logicFormatPtr
);

/* logic taken from function internal_cpssDxChTtiPortGroupRuleGet */
static GT_STATUS ttiRuleActionGet
(
    IN  GT_U8                              devNum,
    IN  GT_PORT_GROUPS_BMP                 portGroupsBmp,
    IN  GT_U32                             index,
    OUT CPSS_DXCH_TTI_ACTION_STC           *actionPtr
)
{
    GT_STATUS   rc;
    GT_U32      portGroupId;
    GT_U32      hwTtiActionArray[TTI_ACTION_TYPE_2_SIZE_CNS];                 /* TTI action in hw format */
    GT_U32      entryNumber;                                                  /* Rule's index in TCAM    */
    GT_U32      dummy;

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum,portGroupsBmp,portGroupId);
     /* zero out hw format */
    cpssOsMemSet(hwTtiActionArray,0,sizeof(hwTtiActionArray));

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        /* convert rule global index to TCAM entry number */
        rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,index,&entryNumber,&dummy);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* read action */
        rc = prvCpssDxChPortGroupReadTableEntry(
            devNum, portGroupId,
            CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
            (entryNumber/2),
            hwTtiActionArray);
    }
    else
    {
        /* now read router / tunnel termination TCAM action */
        rc = prvCpssDxChPortGroupReadTableEntry(
            devNum, portGroupId,
            CPSS_DXCH3_LTT_TT_ACTION_E,
            index,
            hwTtiActionArray);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

    /* convert tti action from hw format to logic format */
    rc = prvCpssDxChTtiActionHw2LogicFormat(devNum,hwTtiActionArray,actionPtr);

    return rc;
}

/**
* @internal perDevice_ruleActionGet function
* @endinternal
*
* @brief   Get the action of a Rule from the TCAM of specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       Read only from first active port group of the bitmap.
* @param[in] tcamIndex                - the TCAM index for AC5 devices
* @param[in] ruleIndex                - the rule index in the TCAM. (CPSS API physical index)
* @param[in] ruleSize                 - the rule size, used to compute actual HW rule index in certain devices
* @param[in] actionTypePtr            - (pointer to)type of contents of action
* @param[in] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS perDevice_ruleActionGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U32                                      tcamIndex,
    IN  GT_U32                                      ruleIndex,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
)
{
    GT_STATUS rc;
    CPSS_PCL_RULE_SIZE_ENT        pclRuleSize;      /* Rule size derived from vTcam rule size. Used only on xCat3 PCL */
    GT_U32                        rulePclIndex;     /* Rule index, adjusted according to underlying API and rule size */

    switch(actionTypePtr->ruleType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:

            switch(ruleSize)
            {
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_10_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_20_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_30_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_40_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_50_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_60_BYTES_E; break;
                case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_80_BYTES_E; break;
                    break;
                default:
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            rc = prvCpssDxChVirtualTcamRuleIndexToRulePclIndex(devNum,
                                                                ruleSize,
                                                                ruleIndex,
                                                                &rulePclIndex);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChPclPortGroupRuleActionGet(
                devNum, portGroupsBmp, tcamIndex,
                pclRuleSize,/* - not used in bc2 and above */
                rulePclIndex,
                actionTypePtr->action.pcl.direction,
                actionDataPtr->action.pcl.actionPtr
                );
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            rc = ttiRuleActionGet(
                devNum, portGroupsBmp,
                ruleIndex,
                actionDataPtr->action.tti.actionPtr
                );
            break;
        default:
            rc = GT_BAD_PARAM;
            break;
    }

    return rc;

}

/**
* @internal perDevice_ruleInvalidate function
* @endinternal
*
* @brief   Invalidate Rule in the TCAM of specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] vTcamMngPtr              - (pointer to) VTCAM Manager structure.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       Read only from first active port group of the bitmap.
* @param[in] ruleIndex                - the rule index in the TCAM. (CPSS API physical index)
* @param[in] devClass                 - the device cllass, used to access the correct API on legacy devices
* @param[in] ruleSize                 - the rule size, used to compute actual HW rule index in certain devices
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS perDevice_ruleInvalidate
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC         *vTcamMngPtr,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U32                                      ruleIndex,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT        devClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize
)
{
    GT_STATUS rc;

    if ((devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E)
        || (devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E))
    {
        CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        rc = prvCpssDxChTcamPortGroupRuleInvalidateEntry(
            devNum, portGroupsBmp, ruleIndex);
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }
    else
    {
        rc = perDevice_ruleValidate(
            devNum, portGroupsBmp, ruleIndex, devClass, ruleSize, GT_FALSE);
    }
    if (rc != GT_OK)
    {
        return rc;
    }

    if (vTcamMngPtr->haSupported != GT_FALSE)
    {
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC* segDbPtr;

        segDbPtr = prvCpssDxChVirtualTcamSegmentDbByDevClassGet(vTcamMngPtr, devClass);
        if ((segDbPtr != NULL) && (segDbPtr->hwDbWriteFuncPtr != NULL))
        {
            CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            rc = segDbPtr->hwDbWriteFuncPtr(
                devNum, portGroupsBmp, ruleIndex,
                PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_DB_INVALID_VTCAM_ID,
                PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_DB_INVALID_RULE_ID);
            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal perDevice_ruleValidate function
* @endinternal
*
* @brief   Validate/invalidate Rule in the TCAM of specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       Read only from first active port group of the bitmap.
* @param[in] ruleIndex                - the rule index in the TCAM. (CPSS API physical index)
* @param[in] devClass                 - the device cllass, used to access the correct API on legacy devices
* @param[in] ruleSize                 - the rule size, used to compute actual HW rule index in certain devices
* @param[in] valid                    - set the rule as on invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS perDevice_ruleValidate
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U32                                      ruleIndex,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT        devClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_BOOL                                     valid
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PCL_RULE_SIZE_ENT pclRuleSize;      /* Rule size derived from vTcam rule size. Used only on xCat3 PCL */
    GT_U32                 rulePclIndex;     /* Rule index, adjusted according to underlying API and rule size */
    GT_U32                 tcamIndex;        /* TCAM index */

    switch (devClass)
    {
    default:
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:

        /* USE TTI API regardless to 'TTI/PCL' actual format */
        rc = cpssDxChTtiPortGroupRuleValidStatusSet(
            devNum, portGroupsBmp,
            ruleIndex,
            valid
            );
        break;

    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E:

        tcamIndex = (devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E) ? 1 : 0;

        switch(ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_10_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_20_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_30_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_40_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_50_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_60_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_80_BYTES_E; break;
                break;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChVirtualTcamRuleIndexToRulePclIndex(devNum,
                                                            ruleSize,
                                                            ruleIndex,
                                                            &rulePclIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPclPortGroupRuleValidStatusSet(
            devNum, portGroupsBmp, tcamIndex,
            pclRuleSize,
            rulePclIndex,
            valid);
        break;

    }

    return rc;

}

/**
* @internal perDevice_ruleValidStatusGet function
* @endinternal
*
* @brief   Gat Valid state of Rule in the TCAM of specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       Read only from first active port group of the bitmap.
* @param[in] ruleIndex                - the rule index in the TCAM. (CPSS API physical index)
* @param[in] devClass                 - the device cllass, used to access the correct API on legacy devices
* @param[in] ruleSize                 - the rule size, used to compute actual HW rule index in certain devices
*
* @param[out] validPtr                 - (pointer to) the rule as valid on invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS perDevice_ruleValidStatusGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U32                                      ruleIndex,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT        devClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_BOOL                                     *validPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32                        rulePclIndex;     /* Rule index, adjusted according to underlying API and rule size */
    CPSS_PCL_RULE_SIZE_ENT        rulePclSize;      /* PCL Rule size translated from TCAM rule size */
    GT_U32                        tcamIndex;        /* TCAM index */

    switch(devClass)
    {
    default:
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        /* USE TTI API regardless to 'TTI/PCL' actual format */
        rc = cpssDxChTtiPortGroupRuleValidStatusGet(
            devNum, portGroupsBmp,
            ruleIndex, validPtr);
        break;

    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E:

        tcamIndex = (devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E) ? 1 : 0;

        /* translate rule size enum to VTCAM and PCL rule size enums */
        switch(ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                    rulePclSize = CPSS_PCL_RULE_SIZE_30_BYTES_E;
                    break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                    rulePclSize = CPSS_PCL_RULE_SIZE_60_BYTES_E;
                    break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                    rulePclSize = CPSS_PCL_RULE_SIZE_80_BYTES_E;
                    break;

            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChVirtualTcamRuleIndexToRulePclIndex(devNum,
                                                            ruleSize,
                                                            ruleIndex,
                                                            &rulePclIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChPclPortGroupRuleAnyStateGet(
            devNum, portGroupsBmp, tcamIndex,
            rulePclSize,
            rulePclIndex,
            validPtr,
            &rulePclSize);
    }

    return rc;

}
/**
* @internal perDevice_ruleMove function
* @endinternal
*
* @brief   Move or Copy Rule in the TCAM of specific device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number.
* @param[in] vTcamMngPtr              - (pointer to) VTCAM Manager structure.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                       NOTEs:
*                                       1. for non multi-port groups device this parameter is IGNORED.
*                                       2. for multi-port groups device :
*                                       (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                       bitmap must be set with at least one bit representing
*                                       valid port group(s). If a bit of non valid port group
*                                       is set then function returns GT_BAD_PARAM.
*                                       value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                       Read only from first active port group of the bitmap.
* @param[in] devClass                 - the device cllass, used to access the correct API on legacy devices
* @param[in] ruleSize                 - the rule size to move.
* @param[in] srcRuleId                - source rule id - the source rule index in the TCAM. (CPSS API physical index)
* @param[in] dstRuleId                - destination rule id - the destination rule index in the TCAM. (CPSS API physical index)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS perDevice_ruleMove
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC         *vTcamMngPtr,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT        devClass,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT        ruleSize,
    IN  GT_U32                                      srcRuleId,
    IN  GT_U32                                      dstRuleId,
    IN  GT_BOOL                                     moveOrCopy
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PCL_RULE_SIZE_ENT pclRuleSize;      /* Rule size derived from vTcam rule size. Used only on xCat3 PCL */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ttiRuleSize;  /* Rule size derived from vTcam rule size. Used only on xCat3 TTI */
    GT_U32                 srcPclRuleId;     /* Source Rule index, adjusted according to underlying API and rule size */
    GT_U32                 dstPclRuleId;     /* Destination Rule index, adjusted according to underlying API and rule size */
    /* support for HA - need to check for invalid rulesize rule in case of block unmapped*/
    GT_U32                 blocksAmount, j, blockStartIndex;
    GT_BOOL                blockFree = GT_FALSE;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockPtr;
    GT_U32                 tcamIndex;        /* TCAM index */

    vTcamMngPtr->ruleMoveCounter ++;

    if(vTcamMngPtr->haSupported)
    {
        blocksAmount = vTcamMngPtr->tcamSegCfg.blocksAmount;
        blockPtr     = vTcamMngPtr->tcamSegCfg.blockInfArr;

        for(j=0;j<blocksAmount;++j)
        {
            blockStartIndex = blockPtr[j].rowsBase*12 + blockPtr[j].columnsBase;
            /* block is already unmapped - rule has been set with invalid ruleSize(7) - will change to valid temporarily
                and invalidate again after move */
            if(blockStartIndex == srcRuleId && blockPtr[j].lookupId == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_LOOKUP_FREE_BLOCK_CNS)
            {
                rc = prvCpssDxChTcamHaRuleSizeSet(devNum, portGroupsBmp, srcRuleId, tcamRuleSizeMap[ruleSize]);
                if(rc != GT_OK)
                {
                    return rc;
                }

                blockFree = GT_TRUE;
            }
        }
    }

    CPSS_LOG_INFORMATION_MAC("[%s] Rule from HW index[%d] to [%d] size[%d] \n",
        (moveOrCopy == GT_TRUE ? "Move" : "Copy") ,
        srcRuleId ,  dstRuleId,  ruleSize);

    switch(devClass)
    {
    default:
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E:

        tcamIndex = (devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E) ? 1 : 0;

        switch(ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_10_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_20_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_30_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_40_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_50_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_60_BYTES_E; break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E: pclRuleSize = CPSS_PCL_RULE_SIZE_80_BYTES_E; break;
                break;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssDxChVirtualTcamRuleIndexToRulePclIndex(devNum,
                                                            ruleSize,
                                                            srcRuleId,
                                                            &srcPclRuleId);
        if (rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssDxChVirtualTcamRuleIndexToRulePclIndex(devNum,
                                                            ruleSize,
                                                            dstRuleId,
                                                            &dstPclRuleId);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* USE PCL API regardless to 'TTI/PCL' actual format */

        /* copy the rule */
        rc = cpssDxChPclPortGroupRuleCopy(
            devNum, portGroupsBmp, tcamIndex,
            pclRuleSize,
            srcPclRuleId,
            dstPclRuleId
            );
        if(rc != GT_OK)
        {
            return rc;
        }
        break;

    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:

        switch(ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E: ttiRuleSize = CPSS_DXCH_TCAM_RULE_SIZE_30_B_E;
                break;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* USE TTI API */

        /* copy the rule */
        rc = rawEntryFormatOperationReadFromHw(
                devNum, portGroupsBmp,
                devClass,
                &(vTcamMngPtr->hwBuffers),
                ttiRuleSize,
                srcRuleId);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = rawEntryFormatOperationWriteToHw(
                devNum, portGroupsBmp,
                devClass,
                &(vTcamMngPtr->hwBuffers),
                ttiRuleSize,
                dstRuleId);
        if(rc != GT_OK)
        {
            return rc;
        }
        break;
    }

    if (vTcamMngPtr->haSupported != GT_FALSE)
    {
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC* segDbPtr;
        GT_U32                                         vTcamId;
        GT_U32                                         ruleId;

        segDbPtr = prvCpssDxChVirtualTcamSegmentDbByDevClassGet(vTcamMngPtr, devClass);
        if ((segDbPtr != NULL) && (segDbPtr->hwDbReadFuncPtr != NULL) && (segDbPtr->hwDbWriteFuncPtr != NULL))
        {
            CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            rc = segDbPtr->hwDbReadFuncPtr(
                devNum, portGroupsBmp, srcRuleId, &vTcamId, &ruleId);
            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            if (rc != GT_OK)
            {
                return rc;
            }
            CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            rc = segDbPtr->hwDbWriteFuncPtr(
                devNum, portGroupsBmp, dstRuleId, vTcamId, ruleId);
            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    if(moveOrCopy == GT_TRUE)/* the move requires also invalidation of the source */
    {
        /* Caller supposes aleady locked device DB */
        /* invalidate the src index */
        rc = perDevice_ruleInvalidate(
            devNum, vTcamMngPtr, portGroupsBmp,
            srcRuleId,
            devClass,
            ruleSize);

        /* should perDevice_ruleValidate be removed?? */
        if(rc == GT_OK && vTcamMngPtr->haSupported)
        {
            rc = perDevice_ruleClear(
                    devNum, portGroupsBmp,
                    srcRuleId,
                    devClass,
                    ruleSize);

            if(rc == GT_OK && blockFree)
            {
                rc = prvCpssDxChTcamHaRuleSizeSet(devNum, portGroupsBmp, srcRuleId, PRV_CPSS_DXCH_TCAM_HA_INVALID_RULE_SIZE_CNS);
            }
        }
    }

    return rc;

}

/**
* @internal ruleMoveByLogicalIndex function
* @endinternal
*
* @brief   Validate/invalidate Rule in the TCAM of specific device, by logical indexes.
*         and update the DB about it.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vtcamInfoPtr             - (pointer to) virtual TCAM structure
* @param[in] srcLogicalIndex          - source logical index.
* @param[in] srcDeviceRuleIndex       - source rule id - the source rule index in the TCAM. (CPSS API physical index)
* @param[in] dstLogicalIndex          - destination logical index.
* @param[in] dstDeviceRuleIndex       - destination rule id - the destination rule index in the TCAM. (CPSS API physical index)
* @param[in] calledFromResize         - indication that called from resize operation
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS ruleMoveByLogicalIndex
(
    IN  GT_U32                                      vTcamMngId,
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr,
    IN  GT_U32                                      srcLogicalIndex,
    IN  GT_U32                                      srcDeviceRuleIndex,
    IN  GT_U32                                      dstLogicalIndex,
    IN  GT_U32                                      dstDeviceRuleIndex,
    IN  GT_BOOL                                     calledFromResize
)
{
    GT_STATUS   rc;
    GT_U8     devNum;     /* device number    */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(srcDeviceRuleIndex != dstDeviceRuleIndex)/*check HW movement*/
    {
        CPSS_LOG_INFORMATION_MAC("move HW index[%d] to index[%d] (shift[%d]) \n",
            srcDeviceRuleIndex,dstDeviceRuleIndex,((int)dstDeviceRuleIndex-(int)srcDeviceRuleIndex));

        /* get first devNum iterator */
        rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* iteration over the devices */
        do
        {
            PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

            /* move the rule in the device */
            rc = perDevice_ruleMove(
                devNum,
                vTcamMngDBPtr,
                vTcamMngDBPtr->portGroupBmpArr[devNum],
                vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
                vtcamInfoPtr->tcamInfo.ruleSize,
                srcDeviceRuleIndex,
                dstDeviceRuleIndex,
                GT_TRUE/*move*/);

            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

            if(rc != GT_OK)
            {
                return rc;
            }
        }
        /* get next devNum iterator */
        while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));
    }

    if(srcLogicalIndex != dstLogicalIndex)/*check DB movement*/
    {
        CPSS_LOG_INFORMATION_MAC("move logical index[%d] to index[%d] (shift[%d]) \n",
            srcLogicalIndex,dstLogicalIndex,((int)dstLogicalIndex-(int)srcLogicalIndex));

        /*Remove from the DB the 'src' rule (and it's logical index) */
        /* update the DB about the 'src' rule */
        vtcamInfoPtr->usedRulesBitmapArr[srcLogicalIndex>>5] &= ~(1<<(srcLogicalIndex & 0x1f));

        if(calledFromResize == GT_FALSE)
        {
            /* update the counter */
            vtcamInfoPtr->usedRulesAmount--;
        }

        /* for priority mode - remove 'src' rule ID entry from DB */
        if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
        {
            if(calledFromResize == GT_FALSE)
            {
                rc = priorityDbRemoveRuleId(
                    VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]), vtcamInfoPtr, srcLogicalIndex);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
                }
            }
            else  /* update the src logical Index entry to trg logical index */
            {
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC ruleIdEntry, * ruleIdEntryPtr;

                ruleIdEntry.logicalIndex = srcLogicalIndex;
                /* find logicalIndex in entry in AVL tree */
                ruleIdEntryPtr = prvCpssAvlSearch(vtcamInfoPtr->ruleIdIndexTree, &ruleIdEntry);
                if (ruleIdEntryPtr == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB error: srcLogicalIndex[%d] was not found in ruleIdIndexTree",
                        srcLogicalIndex);
                }

                /* remove rule ID entry from the index tree */
                ruleIdEntryPtr = prvCpssAvlItemRemove(vtcamInfoPtr->ruleIdIndexTree, ruleIdEntryPtr);
                if(ruleIdEntryPtr == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB error: tree ruleIdIndexTree : failed to remove srcLogicalIndex[%d] (needed temporary removal before adding it again)",
                        srcLogicalIndex);
                }
                /* update the logical index and add it to the tree */
                ruleIdEntryPtr->logicalIndex = dstLogicalIndex;
                /* add updated rule ID entry to the index tree */
                rc = prvCpssAvlItemInsert(vtcamInfoPtr->ruleIdIndexTree, ruleIdEntryPtr);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "DB error: tree ruleIdIndexTree : failed to re-insert dstLogicalIndex[%d] (after temporary removed)",
                        dstLogicalIndex);
                }
            }
        }

        /* NOTE: next is never valid for 'priority mode' because the dstRuleId
           already checked for existence */
        if(0 == (vtcamInfoPtr->usedRulesBitmapArr[dstLogicalIndex>>5] & (1<<(dstLogicalIndex & 0x1f))))
        {
            /* update the DB about the 'trg' rule */

            /* dst was not valid */

            if(calledFromResize == GT_FALSE)
            {
                /* update the counter */
                vtcamInfoPtr->usedRulesAmount++;
            }

            /* update the DB about the moved rule */
            vtcamInfoPtr->usedRulesBitmapArr[dstLogicalIndex>>5] |= (1<<(dstLogicalIndex & 0x1f));
        }
    }


    return  GT_OK;
}

/*******************************************************************************/
/*******************************************************************************/
/******************** end 'per device' functions *******************************/
/*******************************************************************************/
/*******************************************************************************/
/**
* @internal copySingleRuleFromSrcDevToDevBmp function
* @endinternal
*
* @brief   Copy single rule from a representative device to all needed devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devClass                    - the device cllass, used to access the correct API on legacy devices
* @param[in] representativeDevNum        - the device to copy from it
* @param[in] representativePortGroupsBmp - the port group bitmap to copy from it
* @param[in] devPortGroupsBmpArr[]       - the array device ids and port group bitmaps
*                                          added to the Virtual TCAM Manager.
* @param[in] numOfDevs                   - the number of device ids in the array.
* @param[in] hwIndex                     - the  to read from the representativeDevNum
*                                          and to copy to newDevsBmp device(s)
* @param[in] hwBuffersPtr                - (pointer to) read/write buffers structure
* @param[in] ruleSize                    - the rule size of the read and copy entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS copySingleRuleFromSrcDevToDevBmp(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT  devClass,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_ACCESS_BUFFERS_STC *hwBuffersPtr,
    IN GT_U8                                             representativeDevNum,
    IN GT_PORT_GROUPS_BMP                                representativePortGroupsBmp,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC      devPortGroupsBmpArr[],
    IN GT_U32                                            numOfPortGroupBmps,
    IN GT_U32                                            hwIndex,
    IN CPSS_DXCH_TCAM_RULE_SIZE_ENT                      ruleSize
)
{
    GT_STATUS   rc;
    GT_U32      i;

    CPSS_API_LOCK_MAC(representativeDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    /* read from HW */
    rc = rawEntryFormatOperationReadFromHw(
        representativeDevNum, representativePortGroupsBmp,
        devClass, hwBuffersPtr, ruleSize, hwIndex);
    CPSS_API_UNLOCK_MAC(representativeDevNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    for (i = 0; (i < numOfPortGroupBmps); i++)
    {
        CPSS_API_LOCK_MAC(devPortGroupsBmpArr[i].devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        /* write to HW */
        rc = rawEntryFormatOperationWriteToHw(
            devPortGroupsBmpArr[i].devNum, devPortGroupsBmpArr[i].portGroupsBmp,
            devClass, hwBuffersPtr, ruleSize, hwIndex);
        CPSS_API_UNLOCK_MAC(devPortGroupsBmpArr[i].devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal invalidateSingleRuleInDevsPortGroupsBmp function
* @endinternal
*
* @brief   Invalidate single rule In gives devices port group bitmaps.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devClass                    - the device cllass, used to access the correct API on legacy devices
* @param[in] vTcamMngPtr                 - (pointer to) VTCAM Manager structure.
* @param[in] devPortGroupsBmpArr[]       - the array device ids and port group bitmaps
*                                          added to the Virtual TCAM Manager.
* @param[in] numOfDevs                   - the number of device ids in the array.
* @param[in] hwIndex                     - the  to read from the representativeDevNum
*                                          and to copy to newDevsBmp device(s)
* @param[in] ruleSize                    - the rule size of the read and copy entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS invalidateSingleRuleInDevsPortGroupsBmp(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC               *vTcamMngPtr,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT  devClass,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC      devPortGroupsBmpArr[],
    IN GT_U32                                            numOfPortGroupBmps,
    IN GT_U32                                            hwIndex,
    IN CPSS_DXCH_TCAM_RULE_SIZE_ENT                      ruleSize
)
{
    GT_STATUS   rc;
    GT_U32      i;

    for (i = 0; (i < numOfPortGroupBmps); i++)
    {
        /* perDevice_ruleInvalidate not needed device DB locking */
        /* invalidate in HW */
        rc = perDevice_ruleInvalidate(
            devPortGroupsBmpArr[i].devNum, vTcamMngPtr, devPortGroupsBmpArr[i].portGroupsBmp,
            hwIndex, devClass, ruleSize);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal copyAllRulesFromSrcDevToDevBmp_perVtcam function
* @endinternal
*
* @brief   Copy all rules from vtcam of the TCAM , from srcDevice to all needed devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr                - (pointer to)  Virtual TCAM
* @param[in] devClass                    - the device cllass, used to access the correct API on legacy devices
* @param[in] representativeDevNum        - the device to copy from it
* @param[in] representativePortGroupsBmp - the port group bitmap to copy from it
* @param[in] devPortGroupsBmpArr[]       - the array device ids and port group bitmaps
*                                          added to the Virtual TCAM Manager.
* @param[in] numOfPortGroupBmps          - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
static GT_STATUS copyAllRulesFromSrcDevToDevBmp_perVtcam(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*      vtcamInfoPtr,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT  devClass,
    IN GT_U8                                             representativeDevNum,
    IN GT_PORT_GROUPS_BMP                                representativePortGroupsBmp,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC      devPortGroupsBmpArr[],
    IN GT_U32                                            numOfPortGroupBmps
)
{
    GT_STATUS   rc;
    GT_U32  logicalIndex;
    GT_U32  hwIndex;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSize = (GT_U32)vtcamInfoPtr->tcamInfo.ruleSize;

    for(logicalIndex = 0 ; logicalIndex < vtcamInfoPtr->rulesAmount ; logicalIndex++)
    {
        if(0 == (vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] & (1<<(logicalIndex & 0x1f))))
        {
            /* not used logical index */
            continue;
        }

        /* convert logical index to HW index */
        hwIndex = vtcamInfoPtr->rulePhysicalIndexArr[logicalIndex];

        /* copy single rule from device to bmp of devices */
        rc = copySingleRuleFromSrcDevToDevBmp(
            devClass, &(vtcamInfoPtr->tcamSegCfgPtr->pVTcamMngPtr->hwBuffers),
            representativeDevNum, representativePortGroupsBmp,
            devPortGroupsBmpArr, numOfPortGroupBmps,
            hwIndex, ruleSize);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal invalidateAllRulesInDevsPortGroupsBmp_perVtcam function
* @endinternal
*
* @brief   Invalidate all rules from vtcam of the TCAM in all ginen portgroups bitmaps.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vtcamInfoPtr                - (pointer to)  Virtual TCAM
* @param[in] devClass                    - the device cllass, used to access the correct API on legacy devices
* @param[in] devPortGroupsBmpArr[]       - the array device ids and port group bitmaps
*                                          added to the Virtual TCAM Manager.
* @param[in] numOfPortGroupBmps          - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
static GT_STATUS invalidateAllRulesInDevsPortGroupsBmp_perVtcam(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*      vtcamInfoPtr,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT  devClass,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC      devPortGroupsBmpArr[],
    IN GT_U32                                            numOfPortGroupBmps
)
{
    GT_STATUS   rc;
    GT_U32  logicalIndex;
    GT_U32  hwIndex;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSize = (GT_U32)vtcamInfoPtr->tcamInfo.ruleSize;

    for(logicalIndex = 0 ; logicalIndex < vtcamInfoPtr->rulesAmount ; logicalIndex++)
    {
        if(0 == (vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] & (1<<(logicalIndex & 0x1f))))
        {
            /* not used logical index */
            continue;
        }

        /* convert logical index to HW index */
        hwIndex = vtcamInfoPtr->rulePhysicalIndexArr[logicalIndex];
        /* invalidate single rule in bmp of devices portg groups */
        rc = invalidateSingleRuleInDevsPortGroupsBmp(
            vtcamInfoPtr->tcamSegCfgPtr->pVTcamMngPtr,
            devClass, devPortGroupsBmpArr, numOfPortGroupBmps,
            hwIndex, ruleSize);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal copyAllRulesFromSrcDevToDevBmp function
* @endinternal
*
* @brief   Copy all rules from all vtcams of the TCAM , from srcDevice to all needed devices.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId                  - Virtual TCAM Manager Id
*                                         (APPLICABLE RANGES: 0..31)
* @param[in] representativeDevNum        - the device to copy from it
* @param[in] representativePortGroupsBmp - the port group bitmap to copy from it
* @param[in] devPortGroupsBmpArr[]       - the array device ids and port group bitmaps
*                                          added to the Virtual TCAM Manager.
* @param[in] numOfPortGroupBmps          - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
static GT_STATUS copyAllRulesFromSrcDevToDevBmp(
    IN GT_U32                                            vTcamMngId,
    IN GT_U8                                             representativeDevNum,
    IN GT_PORT_GROUPS_BMP                                representativePortGroupsBmp,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC      devPortGroupsBmpArr[],
    IN GT_U32                                            numOfPortGroupBmps
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *tcamMngDbPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr;
    GT_U32  vTcamId;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    tcamMngDbPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);

    for(vTcamId = 0; vTcamId < tcamMngDbPtr->vTcamCfgPtrArrSize ; vTcamId++)
    {
        vtcamInfoPtr = tcamMngDbPtr->vTcamCfgPtrArr[vTcamId];
        if(vtcamInfoPtr == NULL)
        {
            continue;
        }

        /* do copy for this vtcam */
        rc = copyAllRulesFromSrcDevToDevBmp_perVtcam(
            vtcamInfoPtr, vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
            representativeDevNum, representativePortGroupsBmp,
            devPortGroupsBmpArr, numOfPortGroupBmps);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal invalidateAllRulesInDevsPortGroupsBmp function
* @endinternal
*
* @brief   Invalidate all rules from all vtcams of the TCAM in given list of devices port group bitmaps.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId                  - Virtual TCAM Manager Id
*                                         (APPLICABLE RANGES: 0..31)
* @param[in] devPortGroupsBmpArr[]       - the array device ids and port group bitmaps
*                                          added to the Virtual TCAM Manager.
* @param[in] numOfPortGroupBmps          - the number of device ids in the array.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
static GT_STATUS invalidateAllRulesInDevsPortGroupsBmp(
    IN GT_U32                                            vTcamMngId,
    IN CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC      devPortGroupsBmpArr[],
    IN GT_U32                                            numOfPortGroupBmps
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *tcamMngDbPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr;
    GT_U32  vTcamId;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    tcamMngDbPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);

    for(vTcamId = 0; vTcamId < tcamMngDbPtr->vTcamCfgPtrArrSize ; vTcamId++)
    {
        vtcamInfoPtr = tcamMngDbPtr->vTcamCfgPtrArr[vTcamId];
        if(vtcamInfoPtr == NULL)
        {
            continue;
        }

        /* do copy for this vtcam */
        rc = invalidateAllRulesInDevsPortGroupsBmp_perVtcam(
            vtcamInfoPtr, vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
            devPortGroupsBmpArr, numOfPortGroupBmps);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamPclRuleFormatToVtcamRuleSize function
* @endinternal
*
* @brief   Convert PCL Rule Format to Virtual TCAM Rule Size.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] pclRuleFormat               - PCL Rule Format
*
* @retval  - Virtual TCAM Rule Size
*/
static CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT prvCpssDxChVirtualTcamPclRuleFormatToVtcamRuleSize
(
    IN  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT   pclRuleFormat
)
{
    switch (pclRuleFormat)
    {
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E                       :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E                    :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_L4_E                      :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV6_DIP_E                     :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E                        :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E                     :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_STD_IPV4_L4_E                       :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_UDB_E                          :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_IPV4_ROUTED_ACL_QOS_E          :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_NOT_IPV6_E                     :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L2_E                      :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV6_L4_E                      :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_NOT_IPV6_E                      :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L2_E                       :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV6_L4_E                       :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_UDB_E                          :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_EXT_IPV4_PORT_VLAN_QOS_E           :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_EXT_IPV4_RACL_VACL_E                :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_EXT_E;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E         :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_ROUTED_ACL_QOS_E        :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_ULTRA_IPV6_RACL_VACL_E              :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_80_E                           :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ULTRA_E;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E                           :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E                            :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E                           :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E                            :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E                           :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E                            :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E                           :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E                            :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E                           :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E                            :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_50_B_E;
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E                           :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E                            :
        case CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_NO_FIXED_FIELDS_E           :
        case CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_NO_FIXED_FIELDS_E            :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
        default:
        case CPSS_DXCH_PCL_RULE_FORMAT_LAST_E                                     :
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal prvCpssDxChVirtualTcamTtiRuleFormatToVtcamRuleSize function
* @endinternal
*
* @brief   Convert TTI Rule Format to Virtual TCAM Rule Size.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] ttiRuleFormat               - TTI Rule Format
*
* @retval  - Virtual TCAM Rule Size
*/
static CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT prvCpssDxChVirtualTcamTtiRuleFormatToVtcamRuleSize
(
    IN  CPSS_DXCH_TTI_RULE_TYPE_ENT   ttiRuleFormat
)
{
    switch (ttiRuleFormat)
    {
        case CPSS_DXCH_TTI_RULE_IPV4_E   :
        case CPSS_DXCH_TTI_RULE_MPLS_E   :
        case CPSS_DXCH_TTI_RULE_ETH_E    :
        case CPSS_DXCH_TTI_RULE_MIM_E    :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_STD_E;
        case CPSS_DXCH_TTI_RULE_UDB_10_E :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_10_B_E;
        case CPSS_DXCH_TTI_RULE_UDB_20_E :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E;
        case CPSS_DXCH_TTI_RULE_UDB_30_E :
            return CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
        default:
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E, LOG_ERROR_NO_MSG);
    }
}

/*******************************************************************************/
/*******************************************************************************/
/****************** start APIs *************************************************/
/*******************************************************************************/
/*******************************************************************************/

/**
* @internal internal_cpssDxChVirtualTcamRuleWrite function
* @endinternal
*
* @brief   Write Rule to Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*                                      if rule with given rule Id already exists it overridden only
*                                      when the same priotity specified, otherwize GT_BAD_PARAM returned.
* @param[in] ruleAttributesPtr        - (pointer to)rule attributes (for priority driven vTCAM - priority)
* @param[in] ruleTypePtr              -     (pointer to)type of contents of rule
* @param[in] ruleDataPtr              -     (pointer to)data of contents of rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamRuleWrite
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  *ruleAttributesPtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
)
{
    GT_STATUS   rc;
    GT_U32      deviceRuleIndex, lookupId;
    GT_U8       devNum;
    GT_BOOL     newRule = GT_FALSE;
    GT_U32      logicalIndex = 0; /* logical index that converted from ruleId */
    GT_U32      hwAction[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS];
    GT_U32      hwMask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS] = PATTERN;
    GT_U32      hwPattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS] = PATTERN;
    GT_U32     *old_usedRulesBitmapArr,old_usedRulesBitmapArrSize;
    GT_U32      bmpSize;          /* size of used rules BMP */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *vtcamInfoPtr;
    GT_U32                                       tcamIndex;        /* TCAM index */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT                 tcamRuleSize;
    GT_BOOL                                      managerHwWriteBlock;  /*used to check if current manager is blocked for HW write ,used for parallel High Availability*/
    CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC
                                                *replayRulePtr = NULL;
    CPSS_SYSTEM_RECOVERY_INFO_STC                tempSystemRecovery_Info;

    CPSS_NULL_PTR_CHECK_MAC(ruleAttributesPtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleDataPtr);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    /* default, updated below */
    tcamIndex = 0;

    /* check that rule type is consistent with vTcam manager type */
    switch (vtcamInfoPtr->tcamSegCfgPtr->deviceClass)
    {
    default:
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E:
        /* Both PCL/TTI are OK */
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E:
        /* PCL Only */
        tcamIndex =
            (vtcamInfoPtr->tcamSegCfgPtr->deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E)
                ? 1 : 0;
        if(ruleTypePtr->ruleType != CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        /* TTI Only */
        if(ruleTypePtr->ruleType != CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    }

    switch (ruleTypePtr->ruleType)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E:
            if (vtcamInfoPtr->tcamInfo.ruleSize !=
                prvCpssDxChVirtualTcamPclRuleFormatToVtcamRuleSize(ruleTypePtr->rule.pcl.ruleFormat))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E:
            if (vtcamInfoPtr->tcamInfo.ruleSize !=
                prvCpssDxChVirtualTcamTtiRuleFormatToVtcamRuleSize(ruleTypePtr->rule.tti.ruleFormat))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_TCAM_MANAGER_E);
    if (    vTcamMngDBPtr->haSupported
         && (    (    tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E
                   && tempSystemRecovery_Info.systemRecoveryState != CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E
                 )
              || managerHwWriteBlock == GT_TRUE
            )
       )
    {
        /* get first devNum iterator */
        rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("no valid first devNum \n");
            return rc;
        }

        rc = prvCpssDxChVirtualTcamHaSw2HwRuleConvert(devNum,ruleTypePtr,ruleDataPtr,&tcamRuleSize,hwAction,hwMask,hwPattern);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Rule Conversion Failed\n");
        }

        lookupId = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(vtcamInfoPtr->tcamInfo.clientGroup, vtcamInfoPtr->tcamInfo.hitNumber);
        PRV_CPSS_DXCH_VTCAM_LOOKUPID_IDX(lookupId);

        /* TODO - check if validity can be done before DB manipulation */
        /* Check whether rule is valid & populate shadow with vtcamId,ruleId,found */
        rc = prvCpssDxChVirtualTcamHaDbRulesUpdate(tcamRuleSize, lookupId, ruleAttributesPtr->priority,
                hwAction,hwMask,hwPattern,vTcamMngId,vTcamId,ruleId);

        if (rc != GT_OK)
        {

            if ( ! (    vTcamMngDBPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_REPLAY_FAILED_RULES_ADD_E
                     || vTcamMngDBPtr->haFeaturesSupportedBmp & CPSS_DXCH_VIRTUAL_TCAM_HA_FEATURE_SAVE_ALL_RULES_E
                   )
               )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Rule Not Found in HA DB - Failed Rule Replay / Save all rules feature not enabled\n");
            }

            replayRulePtr = cpssOsMalloc(sizeof(CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC));

            if (!replayRulePtr)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "Malloc for replay db entry creation failed. rc=%d\n", rc);
            }

            replayRulePtr->vTcamMngId      = vTcamMngId;
            replayRulePtr->vTcamId         = vTcamId;
            replayRulePtr->ruleId          = ruleId;
            replayRulePtr->ruleAttributes  = *ruleAttributesPtr;
            replayRulePtr->ruleType        = *ruleTypePtr;
            replayRulePtr->ruleData        = *ruleDataPtr;

            if (ruleTypePtr->ruleType == CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
            {
                replayRulePtr->pattern.pcl  = *(ruleDataPtr->rule.pcl.patternPtr);
                replayRulePtr->mask.pcl     = *(ruleDataPtr->rule.pcl.maskPtr);
                replayRulePtr->action.pcl   = *(ruleDataPtr->rule.pcl.actionPtr);

                replayRulePtr->ruleData.rule.pcl.patternPtr = &replayRulePtr->pattern.pcl;
                replayRulePtr->ruleData.rule.pcl.maskPtr    = &replayRulePtr->mask.pcl;
                replayRulePtr->ruleData.rule.pcl.actionPtr  = &replayRulePtr->action.pcl;
            }
            else
            {
                replayRulePtr->pattern.tti  = *(ruleDataPtr->rule.tti.patternPtr);
                replayRulePtr->mask.tti     = *(ruleDataPtr->rule.tti.maskPtr);
                replayRulePtr->action.tti   = *(ruleDataPtr->rule.tti.actionPtr);

                replayRulePtr->ruleData.rule.tti.patternPtr = &replayRulePtr->pattern.tti;
                replayRulePtr->ruleData.rule.tti.maskPtr    = &replayRulePtr->mask.tti;
                replayRulePtr->ruleData.rule.tti.actionPtr  = &replayRulePtr->action.tti;
            }

            if (! VTCAM_GLOVAR(replayRuleDb))
            {
                VTCAM_GLOVAR(replayRuleDb) = replayRulePtr;
            }

            if (VTCAM_GLOVAR(replayRuleDbTail))
            {
                VTCAM_GLOVAR(replayRuleDbTail)->next = replayRulePtr;
            }

            VTCAM_GLOVAR(replayRuleDbTail) = replayRulePtr;
            replayRulePtr->next = NULL;

            CPSS_LOG_INFORMATION_MAC("\nRule not found in TCAM Manager HA Shadow DB. rc=%d\n", rc);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        /* AutoResize VTCAMs - reallocate  usedRulesBitmapArr if needed - since user supplies original size, not actual */
        if (vtcamInfoPtr->tcamInfo.autoResize == GT_TRUE)
        {
            /* At this point in HA, segment creation is yet to happen. Hence, maximal size will be usedRulesAmount at this point.
               RulesAmount will be updated after segment creation, and check will be done at the end of catchup for rulesAmount(maximal size) again */
            bmpSize = ((((vtcamInfoPtr->usedRulesAmount + 1) + 31) / 32) * sizeof(GT_U32));

            if (bmpSize > vtcamInfoPtr->usedRulesBitmapArrSizeHa)
            {
                old_usedRulesBitmapArrSize = vtcamInfoPtr->usedRulesBitmapArrSizeHa;
                old_usedRulesBitmapArr = vtcamInfoPtr->usedRulesBitmapArr;

                vtcamInfoPtr->usedRulesBitmapArr = (GT_U32*)cpssOsMalloc(bmpSize);
                if (vtcamInfoPtr->usedRulesBitmapArr == NULL)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
                cpssOsMemSet(vtcamInfoPtr->usedRulesBitmapArr,0, bmpSize);

                vtcamInfoPtr->usedRulesBitmapArrSizeHa = bmpSize;

                /*copy the previous content*/
                cpssOsMemCpy(vtcamInfoPtr->usedRulesBitmapArr,old_usedRulesBitmapArr, old_usedRulesBitmapArrSize);

                cpssOsFree(old_usedRulesBitmapArr);
            }
        }


        if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
                CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E)
        {
            CPSS_LOG_INFORMATION_MAC("mode : LOGICAL_INDEX \n");
            logicalIndex = ruleId;

            CPSS_LOG_INFORMATION_MAC("logicalIndex[%d] \n" , logicalIndex);

            /* check if the rule was already in the DB  */
            newRule = (vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] & (1<<(logicalIndex & 0x1f))) ?
                GT_FALSE :
                GT_TRUE;

            /* check if the rule was already in the DB  */
            if(newRule == GT_TRUE)
            {
                CPSS_LOG_INFORMATION_MAC("update the DB about the new rule \n");
                /* update the DB about the new rule */
                vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] |= (1<<(logicalIndex & 0x1f));
                /* update the counter */
                vtcamInfoPtr->usedRulesAmount++;

                CPSS_LOG_INFORMATION_MAC("updated the DB counter : usedRulesAmount[%d] \n" ,
                        vtcamInfoPtr->usedRulesAmount);
            } else {
                CPSS_LOG_ERROR_MAC("ERR: Trying to add already Existing rule during HA config replay. Please check\n");
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

        } else if (    vtcamInfoPtr->tcamInfo.ruleAdditionMethod
                == CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E) {

            CPSS_LOG_INFORMATION_MAC("mode : PRIORITY \n");
            /* We cannot update usedRulesBitmapArr as we don't know the logical index */
            vtcamInfoPtr->usedRulesAmount++;
        }
        return rc;
    }

    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E)
    {
        CPSS_LOG_INFORMATION_MAC("mode : LOGICAL_INDEX \n");
        logicalIndex = ruleId;

        CPSS_LOG_INFORMATION_MAC("logicalIndex[%d] \n" , logicalIndex);

        if(logicalIndex >= vtcamInfoPtr->rulesAmount)
        {
            CPSS_LOG_INFORMATION_MAC("error : logicalIndex[%d] >= vtcamInfoPtr->rulesAmount[%d] \n",
                logicalIndex ,vtcamInfoPtr->rulesAmount);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* check if the rule was already in the DB  */
        newRule = (vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] & (1<<(logicalIndex & 0x1f))) ?
            GT_FALSE :
            GT_TRUE;
        if(newRule == GT_TRUE)
        {
            CPSS_LOG_INFORMATION_MAC("new rule \n");

            if(vtcamInfoPtr->usedRulesAmount >= vtcamInfoPtr->rulesAmount)
            {
                CPSS_LOG_INFORMATION_MAC("error : vtcamInfoPtr->usedRulesAmount[%d] >= vtcamInfoPtr->rulesAmount[%d] \n",
                    vtcamInfoPtr->usedRulesAmount , vtcamInfoPtr->rulesAmount);
                /* 'management' error */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            CPSS_LOG_INFORMATION_MAC("existing rule \n");
        }
    }
    else
    {
        CPSS_LOG_INFORMATION_MAC("mode : PRIORITY \n");

        /*
        *       Get place for new rule.
        *       If rule exists it returns it's logical index (and no changes needed).
        *       If needed move existing rules.
        *       If needed resize vTCAM.
        */
        rc = priorityRuleWritePreparations(vTcamMngId,vTcamId,vtcamInfoPtr,
            ruleId,
            ruleAttributesPtr->priority,
            &logicalIndex,&newRule);
        if(rc != GT_OK)
        {
            return rc;
        }

        CPSS_LOG_INFORMATION_MAC("logicalIndex[%d] \n" , logicalIndex);
    }

    /* convert the logical index to physical index in the HW */
    deviceRuleIndex = vtcamInfoPtr->rulePhysicalIndexArr[logicalIndex];

    CPSS_LOG_INFORMATION_MAC("convert the logical index[%d] to physical index in the HW[%d] \n" ,
         logicalIndex, vtcamInfoPtr->rulePhysicalIndexArr[logicalIndex]);

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("no valid first devNum \n");
        return rc;
    }

    /* iteration over the devices */
    do
    {
        CPSS_LOG_INFORMATION_MAC("devNum iterator [%d] \n" ,
            devNum);

        /* perDevice_ruleWrite not needed device DB locking */
        /* write the rule to the device */
        rc = perDevice_ruleWrite(
            devNum,
            vTcamMngDBPtr->portGroupBmpArr[devNum],
            tcamIndex,
            deviceRuleIndex,
            ruleTypePtr,ruleDataPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));

    /* check if the rule was already in the DB  */
    if(newRule == GT_TRUE)
    {
        CPSS_LOG_INFORMATION_MAC("update the DB about the new rule \n");

        if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
        {
            /* update the DB about the new rule */
            rc = priorityNewRuleIdAddedDbUpdate(vTcamMngId,vTcamId,
                ruleId,
                logicalIndex);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* NOTE: even if logicalIndex bit is already set ... we need to update
           the the counter.
           because the bit in this case was preserved 'high' during the shifting
           of valid ranges into 'hole' that was found to allow the new rule to
           fit into it's priority */
        /* update the DB about the new rule */
        vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] |= (1<<(logicalIndex & 0x1f));
        /* update the counter */
        vtcamInfoPtr->usedRulesAmount++;

        CPSS_LOG_INFORMATION_MAC("updated the DB counter : usedRulesAmount[%d] \n" ,
            vtcamInfoPtr->usedRulesAmount);

        rc = prvCpssDxChVirtualTcamHaHwDbWrite(
            vtcamInfoPtr->tcamSegCfgPtr, deviceRuleIndex,
            vTcamId, ruleId);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* already in the DB */
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamRuleWrite function
* @endinternal
*
* @brief   Write Rule to Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*                                      if rule with given rule Id already exists it overridden only
*                                      when the same priotity specified, otherwize GT_BAD_PARAM returned.
* @param[in] ruleAttributesPtr        - (pointer to)rule attributes (for priority driven vTCAM - priority)
* @param[in] ruleTypePtr              -     (pointer to)type of contents of rule
* @param[in] ruleDataPtr              -     (pointer to)data of contents of rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamRuleWrite
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  *ruleAttributesPtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRuleWrite);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId, ruleAttributesPtr, ruleTypePtr, ruleDataPtr));

    rc = internal_cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, ruleId, ruleAttributesPtr, ruleTypePtr, ruleDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId, ruleAttributesPtr, ruleTypePtr, ruleDataPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamRuleRead function
* @endinternal
*
* @brief   Read Rule from Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] ruleTypePtr              -     (pointer to)type of contents of rule
*
* @param[out] ruleDataPtr              -      (pointer to)data of contents of rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamRuleRead
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
)
{
    GT_STATUS   rc;
    GT_U32  deviceRuleIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U8   devNum;
    GT_U32  logicalIndex; /* logical index that converted from ruleId */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    GT_U32                              tcamIndex;        /* TCAM index */

    CPSS_NULL_PTR_CHECK_MAC(ruleTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(ruleDataPtr);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    /* default, updated below */
    tcamIndex = 0;

    /* check that rule type is consistent with vTcam manager type */
    switch (vtcamInfoPtr->tcamSegCfgPtr->deviceClass)
    {
    default:
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E:
        /* Both PCL/TTI are OK */
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E:
        /* PCL Only */
        tcamIndex =
            (vtcamInfoPtr->tcamSegCfgPtr->deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E)
                ? 1 : 0;
        if(ruleTypePtr->ruleType != CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
        /* TTI Only */
        if(ruleTypePtr->ruleType != CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_TTI_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        break;
    }

    /* convert the ruleId to logical index and to physical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &logicalIndex,&deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* perDevice_ruleRead not needed device DB locking */
    /* read the rule from the first device */
    rc = perDevice_ruleRead(
        devNum,
        vTcamMngDBPtr->portGroupBmpArr[devNum],
        tcamIndex,
        deviceRuleIndex, ruleTypePtr, ruleDataPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamRuleRead function
* @endinternal
*
* @brief   Read Rule from Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] ruleTypePtr              -     (pointer to)type of contents of rule
*
* @param[out] ruleDataPtr              -      (pointer to)data of contents of rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamRuleRead
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        *ruleTypePtr,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        *ruleDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRuleRead);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId, ruleTypePtr, ruleDataPtr));

    rc = internal_cpssDxChVirtualTcamRuleRead(vTcamMngId, vTcamId, ruleId, ruleTypePtr, ruleDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId, ruleTypePtr, ruleDataPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChVirtualTcamRuleActionUpdate function
* @endinternal
*
* @brief   Update Rule Action in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] actionTypePtr            - (pointer to)type of contents of action
* @param[in] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamRuleActionUpdate
(
    IN  GT_U32                                        vTcamMngId,
    IN  GT_U32                                        vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
)
{
    GT_STATUS   rc;
    GT_U32  deviceRuleIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U8   devNum;
    GT_U32  logicalIndex; /* logical index that converted from ruleId */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    GT_U32  tcamIndex;

    CPSS_NULL_PTR_CHECK_MAC(actionTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(actionDataPtr);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    tcamIndex = 
        (vtcamInfoPtr->tcamSegCfgPtr->deviceClass 
         == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E) ? 1 : 0;

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    /* convert the ruleId to logical index and to physical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &logicalIndex,&deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* iteration over the devices */
    do
    {
        /* perDevice_ruleActionUpdate not needs for device DB locking */
        /* update the action in the device */
        rc = perDevice_ruleActionUpdate(
            devNum,
            vTcamMngDBPtr->portGroupBmpArr[devNum], tcamIndex,
            deviceRuleIndex,vtcamInfoPtr->tcamInfo.ruleSize,
            actionTypePtr, actionDataPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamRuleActionUpdate function
* @endinternal
*
* @brief   Update Rule Action in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] actionTypePtr            - (pointer to)type of contents of action
* @param[in] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssDxChVirtualTcamRuleActionUpdate
(
    IN  GT_U32                                        vTcamMngId,
    IN  GT_U32                                        vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRuleActionUpdate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId, actionTypePtr, actionDataPtr));

    rc = internal_cpssDxChVirtualTcamRuleActionUpdate(vTcamMngId, vTcamId, ruleId, actionTypePtr, actionDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId, actionTypePtr, actionDataPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamRuleActionGet function
* @endinternal
*
* @brief   Get Rule Action in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] actionTypePtr            - (pointer to)type of contents of action
*
* @param[out] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamRuleActionGet
(
    IN  GT_U32                                        vTcamMngId,
    IN  GT_U32                                        vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
)
{
    GT_STATUS   rc;
    GT_U32  deviceRuleIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U8   devNum;
    GT_U32  logicalIndex; /* logical index that converted from ruleId */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    GT_U32  tcamIndex;

    CPSS_NULL_PTR_CHECK_MAC(actionTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(actionDataPtr);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    tcamIndex = 
        (vtcamInfoPtr->tcamSegCfgPtr->deviceClass 
         == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E) ? 1 : 0;

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    /* convert the ruleId to logical index and to physical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &logicalIndex,&deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

     CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    /* read the action from the first device */
    rc = perDevice_ruleActionGet(
        devNum,
        vTcamMngDBPtr->portGroupBmpArr[devNum], tcamIndex,
        deviceRuleIndex,
        vtcamInfoPtr->tcamInfo.ruleSize,
        actionTypePtr, actionDataPtr);

     CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamRuleActionGet function
* @endinternal
*
* @brief   Get Rule Action in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] actionTypePtr            - (pointer to)type of contents of action
*
* @param[out] actionDataPtr            - (pointer to)data of contents of action
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssDxChVirtualTcamRuleActionGet
(
    IN  GT_U32                                        vTcamMngId,
    IN  GT_U32                                        vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                ruleId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_TYPE_STC   *actionTypePtr,
    OUT CPSS_DXCH_VIRTUAL_TCAM_RULE_ACTION_DATA_STC   *actionDataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRuleActionGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId, actionTypePtr, actionDataPtr));

    rc = internal_cpssDxChVirtualTcamRuleActionGet(vTcamMngId, vTcamId, ruleId, actionTypePtr, actionDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId, actionTypePtr, actionDataPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChVirtualTcamRuleValidStatusSet function
* @endinternal
*
* @brief   Set Rule Valid Status in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] valid                    - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamRuleValidStatusSet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId,
    IN   GT_BOOL                            valid
)
{
    GT_STATUS   rc;
    GT_U32  deviceRuleIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U8   devNum;
    GT_U32  logicalIndex; /* logical index that converted from ruleId */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    /* convert the ruleId to logical index and to physical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &logicalIndex,&deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* iteration over the devices */
    do
    {
        /* perDevice_ruleValidate not needed device DB locking */
        /* invalidate the rule in the device */
        rc = perDevice_ruleValidate(
            devNum,
            vTcamMngDBPtr->portGroupBmpArr[devNum],
            deviceRuleIndex,
            vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
            vtcamInfoPtr->tcamInfo.ruleSize,
            valid);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));

    return GT_OK;
}


/**
* @internal cpssDxChVirtualTcamRuleValidStatusSet function
* @endinternal
*
* @brief   Set Rule Valid Status in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] valid                    - GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssDxChVirtualTcamRuleValidStatusSet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId,
    IN   GT_BOOL                            valid
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRuleValidStatusSet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId, valid));

    rc = internal_cpssDxChVirtualTcamRuleValidStatusSet(vTcamMngId, vTcamId, ruleId, valid);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId, valid));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChVirtualTcamRuleValidStatusGet function
* @endinternal
*
* @brief   Get Rule Valid Status in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @param[out] validPtr                 - (pointer to)GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamRuleValidStatusGet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId,
    OUT  GT_BOOL                            *validPtr
)
{
    GT_STATUS   rc;
    GT_U32  deviceRuleIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U8   devNum;
    GT_U32  logicalIndex; /* logical index that converted from ruleId */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;

    CPSS_NULL_PTR_CHECK_MAC(validPtr);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    /* convert the ruleId to logical index and to physical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &logicalIndex,&deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* perDevice_ruleValidStatusGet not needed for device DB locking */
    /* invalidate the rule in the device */
    rc = perDevice_ruleValidStatusGet(
        devNum,
        vTcamMngDBPtr->portGroupBmpArr[devNum],
        deviceRuleIndex,
        vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
        vtcamInfoPtr->tcamInfo.ruleSize,
        validPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamRuleValidStatusGet function
* @endinternal
*
* @brief   Get Rule Valid Status in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @param[out] validPtr                 - (pointer to)GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssDxChVirtualTcamRuleValidStatusGet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId,
    OUT  GT_BOOL                            *validPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRuleValidStatusGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId, validPtr));

    rc = internal_cpssDxChVirtualTcamRuleValidStatusGet(vTcamMngId, vTcamId, ruleId, validPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId, validPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamRuleDelete function
* @endinternal
*
* @brief   Delete Rule From internal DB and Invalidate it in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamRuleDelete
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId
)
{
    GT_STATUS   rc;
    GT_U32  deviceRuleIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    GT_U8   devNum;
    GT_U32  logicalIndex; /* logical index that converted from ruleId */
    GT_U32  saveGuaranteedNumOfRules;
    GT_BOOL doAutoResize;
    GT_U32  autoResizeRemoval; /* downsize granularity measured in rules          */
    GT_U32  ruleAlignment;     /* TCAM column alignment of rules                  */
    GT_U32  rulesNumAlignment; /* vTCAM rules amount alignment                    */
    GT_U32  minRulesAmount;    /* aligned guarantied vTCAM size                   */
    GT_U32  freeRulesAmount;   /* amount of free rule places in vTCAM space       */
    GT_U32  overSizeRulesAmount; /* amount of overSize rule places in vTCAM space */
    GT_U32  spaceToFree;        /* amount of rule places to free from vTCAM space */

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    vTcamMngDBPtr->ruleInvalidateCounter ++ ;

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    /* convert the ruleId to logical index and to physical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &logicalIndex,&deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(vtcamInfoPtr->usedRulesAmount == 0)
    {
        /* 'management' error */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* iteration over the devices */
    do
    {
        /* perDevice_ruleValidate not needed device DB locking */
        /* invalidate the rule in the device */
        rc = perDevice_ruleInvalidate(
            devNum, vTcamMngDBPtr,
            vTcamMngDBPtr->portGroupBmpArr[devNum],
            deviceRuleIndex,
            vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
            vtcamInfoPtr->tcamInfo.ruleSize);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(vTcamMngDBPtr->haSupported)
        {
            rc = perDevice_ruleClear(devNum,
                    vTcamMngDBPtr->portGroupBmpArr[devNum],
                    deviceRuleIndex,
                    vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
                    vtcamInfoPtr->tcamInfo.ruleSize);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));

    /* update the DB about the removed rule */
    vtcamInfoPtr->usedRulesBitmapArr[logicalIndex>>5] &= ~(1<<(logicalIndex & 0x1f));

    /* update the counter */
    vtcamInfoPtr->usedRulesAmount--;

    /* for priority mode - remove rule ID entry from DB */
    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        rc = priorityDbRemoveRuleId(vTcamMngDBPtr, vtcamInfoPtr, logicalIndex);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    /* for priority mode - autoresize */

    ruleAlignment = prvCpssDxChVirtualTcamSegmentTableRuleSizeToAlignment(
        vtcamInfoPtr->tcamSegCfgPtr, vtcamInfoPtr->tcamInfo.ruleSize);
    rulesNumAlignment =
        (vtcamInfoPtr->tcamSegCfgPtr->lookupColumnAlignment >= ruleAlignment)
            ? (vtcamInfoPtr->tcamSegCfgPtr->lookupColumnAlignment / ruleAlignment)
            : 1;

    minRulesAmount      = vtcamInfoPtr->tcamInfo.guaranteedNumOfRules + rulesNumAlignment - 1;
    minRulesAmount     -= (minRulesAmount % rulesNumAlignment);
    freeRulesAmount     = vtcamInfoPtr->rulesAmount - vtcamInfoPtr->usedRulesAmount;
    overSizeRulesAmount = vtcamInfoPtr->rulesAmount - minRulesAmount;
    spaceToFree =
        (freeRulesAmount < overSizeRulesAmount)
            ? freeRulesAmount : overSizeRulesAmount;

    autoResizeRemoval =
        prvCpssDxChVirtualTcamDbSegmentTableResizeGranularityGet(
            vtcamInfoPtr->tcamSegCfgPtr, vtcamInfoPtr->tcamInfo.ruleSize);

    doAutoResize = (vtcamInfoPtr->autoResizeDisable == GT_FALSE) &&
                    (vtcamInfoPtr->tcamInfo.autoResize);
    if (vtcamInfoPtr->tcamInfo.ruleAdditionMethod !=
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        doAutoResize = GT_FALSE;
    }
    if (spaceToFree == 0)
    {
        doAutoResize = GT_FALSE;
    }
    /* granularity related only to vTCAM having decreasable size */
    if ((vtcamInfoPtr->usedRulesAmount > minRulesAmount) && (spaceToFree < autoResizeRemoval))
    {
        doAutoResize = GT_FALSE;
    }

    if (doAutoResize != GT_FALSE)
    {
        /* decrease vTCAM size */
        saveGuaranteedNumOfRules = vtcamInfoPtr->tcamInfo.guaranteedNumOfRules;
        rc = vtcamDbSegmentTableVTcamDownSize__PriorityMode(
            vTcamMngId, vTcamId, spaceToFree/*downSizeValue*/);
        vtcamInfoPtr->tcamInfo.guaranteedNumOfRules = saveGuaranteedNumOfRules;
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamRuleDelete function
* @endinternal
*
* @brief   Delete Rule From internal DB and Invalidate it in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssDxChVirtualTcamRuleDelete
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     ruleId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRuleDelete);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId));

    rc = internal_cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, ruleId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChVirtualTcamRuleMove function
* @endinternal
*
* @brief   Move Rule in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] srcRuleId                - source rule id: for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] dstRuleId                - destination rule id: for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamRuleMove
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     srcRuleId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     dstRuleId
)
{
    GT_STATUS   rc;
    GT_U32  srcDeviceRuleIndex,trgDeviceRuleIndex;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U32  src_logicalIndex; /* source logical index that converted from srcRuleId */
    GT_U32  dst_logicalIndex; /* destination logical index that converted from dstRuleId */
    GT_BOOL dst_checkExistance; /*indication if need to check that the dstRuleId already
                          exists in the DB. */

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    /* convert the srcRuleId to logical index and to physical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,srcRuleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &src_logicalIndex,&srcDeviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(srcRuleId == dstRuleId)
    {
        CPSS_LOG_INFORMATION_MAC("No action done when srcRuleId[%d] == dstRuleId \n",
            srcRuleId);

        /* nothing to update */
        return GT_OK;
    }

    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E)
    {
        /* the trgRule is not checked for existence */
        dst_checkExistance = GT_FALSE;
    }
    else
    {
        /* the trgRule is checked for existence */
        dst_checkExistance = GT_TRUE;
    }

    /* convert the dstRuleId to logical index and to physical index */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,dstRuleId,
        dst_checkExistance,
        &dst_logicalIndex,&trgDeviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* move the index in HW and in DB */
    rc = ruleMoveByLogicalIndex(vTcamMngId,vtcamInfoPtr,
        src_logicalIndex,srcDeviceRuleIndex,
        dst_logicalIndex,trgDeviceRuleIndex,
        GT_FALSE/*calledFromResize*/);

    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamRuleMove function
* @endinternal
*
* @brief   Move Rule in Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] srcRuleId                - source rule id: for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
* @param[in] dstRuleId                - destination rule id: for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
*/
GT_STATUS cpssDxChVirtualTcamRuleMove
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     srcRuleId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID     dstRuleId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRuleMove);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, srcRuleId, dstRuleId));

    rc = internal_cpssDxChVirtualTcamRuleMove(vTcamMngId, vTcamId, srcRuleId, dstRuleId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, srcRuleId, dstRuleId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal rawEntryFormatOperationReadFromHw function
* @endinternal
*
* @brief   1. copy raw entry (pattern,mask,action) from HW to SW buffer.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                  - device number.
* @param[in] portGroupsBmp           - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                     - read only from first active port group of the bitmap.
* @param[in] devClass                 - device class.
* @param[in] hwBuffersPtr             - (pointer to) read/write buffers structure
* @param[in] ruleSize                 - rule size
* @param[in] ruleIndex                - HW index of the rule in the TCAM from which pattern,
*                                      mask and action are taken.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*
* @note function based on : prvCpssDxChPclSip5RuleCopy
*
*/
static GT_STATUS rawEntryFormatOperationReadFromHw
(
    IN  GT_U8                                              devNum,
    IN  GT_PORT_GROUPS_BMP                                 portGroupsBmp,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT    devClass,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_ACCESS_BUFFERS_STC   *hwBuffersPtr,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT                       ruleSize,
    IN  GT_U32                                             ruleIndex
)
{
    GT_STATUS    rc;                                             /* return code */
    GT_BOOL      is_eArch;                                       /* whether device is eArch */
    CPSS_PCL_RULE_SIZE_ENT       pclRuleSize;                   /* PCL Rule size translated from TCAM rule size */
    CPSS_PCL_RULE_SIZE_ENT       pclRuleSizeFound;              /* previously configured PCL rule size by given rule index */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT vTcamRuleSize;         /* Virtual TCAM rule size translated from TCAM rule size */
    GT_U32                       pclRuleIndex;                  /* Rule index, adjusted according to underlying API and rule size */
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSizeFound;                 /* previously configured rule size by given rule index */
    GT_U32       entryNumber;                                    /* Rule's index in TCAM */
    GT_U32       dummy;
    GT_U32      validArr[5];                                    /* TTI TCAM line valid bits    */
    GT_U32      compareModeArr[5];                              /* TTI TCAM line compare mode  */
    GT_U32      i;                                                            /* index */
    GT_U32      portGroupId=0;
    GT_U32      tcamIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* Support only xCat3 or SIP5 devices */
    is_eArch = PRV_CPSS_SIP_5_CHECK_MAC(devNum);
    if((!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) &&
        (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
        (GT_FALSE == is_eArch))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (is_eArch)
    {
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);
        /* get the first port group in the BMP */
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(devNum,portGroupsBmp,portGroupId);

        /* get Validity and size of source rule */
        rc = cpssDxChTcamPortGroupRuleValidStatusGet(
            devNum,portGroupsBmp,ruleIndex,&(hwBuffersPtr->rawEntryFormatOperation_valid),&ruleSizeFound);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"failed to get validity and size of source rule for ruleIndex[%d] \n",
                ruleIndex);
        }

        if (ruleSize != ruleSizeFound)
        {
            /* source rule has another size */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"mismatch : for ruleIndex[%d] : expected rule size[%d] but actual is[%d] \n",
                ruleIndex,ruleSize,ruleSizeFound);
        }

        /* Converts rule global index to TCAM entry number */
        rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,ruleIndex,&entryNumber,&dummy);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"failed to convert rule global index to TCAM entry number : for ruleIndex[%d] \n",
                ruleIndex);
        }

        /* get action of source rule */
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                                portGroupId,
                                                CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                                (entryNumber/2),
                                                hwBuffersPtr->rawEntryFormatOperation_action);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"failed to get action of source rule : for ruleIndex[%d] \n",
                ruleIndex);
        }

        /* get mask and pattern */
        rc =  cpssDxChTcamPortGroupRuleRead(
            devNum,portGroupsBmp,ruleIndex,
            &(hwBuffersPtr->rawEntryFormatOperation_valid),
            &ruleSizeFound,
            hwBuffersPtr->rawEntryFormatOperation_pattern,
            hwBuffersPtr->rawEntryFormatOperation_mask);
    }
    else
    if ((devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E)
        ||(devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E))
    {
        tcamIndex = (devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E) ? 1 : 0;
        /* translate rule size enum to VTCAM and PCL rule size enums */
        switch(ruleSize)
        {
            case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
                    pclRuleSize = CPSS_PCL_RULE_SIZE_30_BYTES_E;
                    vTcamRuleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
                    break;
            case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
                    pclRuleSize = CPSS_PCL_RULE_SIZE_60_BYTES_E;
                    vTcamRuleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
                    break;
            case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
                    pclRuleSize = CPSS_PCL_RULE_SIZE_80_BYTES_E;
                    vTcamRuleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E;
                    break;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChVirtualTcamRuleIndexToRulePclIndex(devNum,
                                                            vTcamRuleSize,
                                                            ruleIndex,
                                                            &pclRuleIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* get Validity and size of source rule */
        rc = cpssDxChPclPortGroupRuleAnyStateGet(devNum,
                                                    portGroupsBmp, tcamIndex,
                                                    pclRuleSize,
                                                    pclRuleIndex,
                                                    &(hwBuffersPtr->rawEntryFormatOperation_valid),
                                                    &pclRuleSizeFound);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"failed to get validity and size of source rule for ruleIndex[%d] \n",
                ruleIndex);
        }

        if ( pclRuleSize !=  pclRuleSizeFound)
        {
            /* source rule has another size */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"mismatch : for ruleIndex[%d] : expected rule size[%d] but actual is[%d] \n",
                ruleIndex,pclRuleSize,pclRuleSizeFound);
        }

        /* get action, mask and pattern */
        rc = cpssDxChPclPortGroupRuleGet(
            devNum, portGroupsBmp, tcamIndex, pclRuleSize, pclRuleIndex,
            hwBuffersPtr->rawEntryFormatOperation_mask,
            hwBuffersPtr->rawEntryFormatOperation_pattern,
            hwBuffersPtr->rawEntryFormatOperation_action);
    }
    else
    {
        if (PRV_CPSS_IS_LION_STYLE_MULTI_PORT_GROUPS_DEVICE_MAC(devNum))
        {
            rc = prvCpssDxChRouterTunnelTermTcamMultiPortGroupsGetLine(devNum,
                                                                       portGroupsBmp,
                                                                       ruleIndex,
                                                                       &validArr[0],
                                                                       &compareModeArr[0],
                                                                       hwBuffersPtr->rawEntryFormatOperation_pattern,
                                                                       hwBuffersPtr->rawEntryFormatOperation_mask,
                                                                       hwBuffersPtr->rawEntryFormatOperation_action);
        }
        else
        {
            /* read tunnel termination configuration and action from hardware */
            rc = prvCpssDxChRouterTunnelTermTcamGetLine(devNum,
                                                        portGroupId,
                                                        ruleIndex,
                                                        &validArr[0],
                                                        &compareModeArr[0],
                                                        hwBuffersPtr->rawEntryFormatOperation_pattern,
                                                        hwBuffersPtr->rawEntryFormatOperation_mask,
                                                        hwBuffersPtr->rawEntryFormatOperation_action);
        }

        if (rc != GT_OK)
        {
            return rc;
        }

        /* line holds valid TTI entry if the following applies:
             - all entries are valid
             - the compare mode or all entries is row compare
             - keyArr[5] bit 31 must be 1 (to indicate TTI entry and not IPv6 entry) */
        hwBuffersPtr->rawEntryFormatOperation_valid = GT_TRUE;
        for (i = 0 ; i < 4; i++)
        {
            /* if entry is not valid or is single compare mode, whole line is not valid */
            if ((validArr[i] == 0) || (compareModeArr[i] == 0))
            {
                hwBuffersPtr->rawEntryFormatOperation_valid = GT_FALSE;
                break;
            }
        }
        /* if whole line is valid, verify it is indeed TTI entry and not IPv6 entry */
        if ((hwBuffersPtr->rawEntryFormatOperation_valid == GT_TRUE)
            && (((hwBuffersPtr->rawEntryFormatOperation_pattern[5] >> 31) & 0x1) != 1))
        {
            hwBuffersPtr->rawEntryFormatOperation_valid = GT_FALSE;
        }

    }

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"failed to get mask and pattern : for ruleIndex[%d] \n",
            ruleIndex);
    }

    return GT_OK;
}

/**
* @internal rawEntryFormatOperationWriteToHw function
* @endinternal
*
* @brief   1. copy raw entry (pattern,mask,action) from HW to SW buffer.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                  - device number.
* @param[in] portGroupsBmp           - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Lion2; Bobcat3; Falcon)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                     - read only from first active port group of the bitmap.
* @param[in] devClass                 - device class.
* @param[in] hwBuffersPtr             - (pointer to) read/write buffers structure
* @param[in] ruleSize                 - rule size
* @param[in] ruleIndex                - HW index of the rule in the TCAM to which pattern,
*                                      mask and action are written.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*
* @note function based on : prvCpssDxChPclSip5RuleCopy
*
*/
static GT_STATUS rawEntryFormatOperationWriteToHw
(
    IN  GT_U8                                              devNum,
    IN  GT_PORT_GROUPS_BMP                                 portGroupsBmp,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_ENT    devClass,
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_HW_ACCESS_BUFFERS_STC   *hwBuffersPtr,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT                       ruleSize,
    IN  GT_U32                                             ruleIndex
)
{
    GT_STATUS    rc;                                             /* return code */
    GT_BOOL      is_eArch;                                       /* whether device is eArch */
    GT_U32       ruleSizeVal=0;                                  /* rule size (in std rules)      */
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT vTcamRuleSize;          /* Virtual TCAM rule size translated from TCAM rule size */
    GT_U32                       pclRuleIndex;                   /* Rule index, adjusted according to underlying API and rule size */
    GT_U32       entryNumber;                                    /* Rule's index in TCAM */
    GT_U32       dummy;
    GT_U32       portGroupId=0;
    GT_U32       tcamIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* Support only xCat3 or SIP5 devices */
    is_eArch = PRV_CPSS_SIP_5_CHECK_MAC(devNum);
    if((!PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)) &&
        (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E) &&
        (GT_FALSE == is_eArch))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    if (is_eArch)
    {
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum,portGroupsBmp,PRV_CPSS_DXCH_UNIT_TCAM_E);

        /* Converts rule global index to TCAM entry number */
        rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum,ruleIndex,&entryNumber,&dummy);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"failed to convert rule global index to TCAM entry number : for ruleIndex[%d] \n",
                ruleIndex);
        }

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            /* write action to destination rule */
            rc = prvCpssDxChPortGroupWriteTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                                     (entryNumber/2),
                                                     hwBuffersPtr->rawEntryFormatOperation_action);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"failed to write action to destination rule : for ruleIndex[%d] \n",
                    ruleIndex);
            }

            /* write destination rule */
            rc = cpssDxChTcamPortGroupRuleWrite(devNum,1<<portGroupId,ruleIndex,
                hwBuffersPtr->rawEntryFormatOperation_valid,
                ruleSize,
                hwBuffersPtr->rawEntryFormatOperation_pattern,
                hwBuffersPtr->rawEntryFormatOperation_mask);

            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"failed to write destination mask and pattern : for ruleIndex[%d] \n",
                    ruleIndex);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
    }
    else
    if ((devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E)
        ||(devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E))
    {
        tcamIndex = (devClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E) ? 1 : 0;
        /* translate rule size enum to VTCAM and PCL rule size enums */
        switch(ruleSize)
        {
            case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
                    vTcamRuleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E;
                    ruleSizeVal = 1;
                    break;
            case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
                    vTcamRuleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E;
                    ruleSizeVal = 2;
                    break;
            case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
                    vTcamRuleSize = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E;
                    ruleSizeVal = 3;
                    break;
            default:
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChVirtualTcamRuleIndexToRulePclIndex(devNum,
                                                            vTcamRuleSize,
                                                            ruleIndex,
                                                            &pclRuleIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            rc = prvCpssDxChPclTcamRuleWrite(
                devNum, portGroupId, tcamIndex, ruleSizeVal, pclRuleIndex,
                hwBuffersPtr->rawEntryFormatOperation_valid,
                hwBuffersPtr->rawEntryFormatOperation_action,
                hwBuffersPtr->rawEntryFormatOperation_mask,
                hwBuffersPtr->rawEntryFormatOperation_pattern);

            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"failed to write destination mask and pattern : for portGroupId[%d], ruleIndex[%d] \n",
                    portGroupId,
                    ruleIndex);
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
    }
    else
    {/*PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E*/
        
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(
            devNum, portGroupsBmp, portGroupId)
        {
            if (hwBuffersPtr->rawEntryFormatOperation_valid)
            {
                /* now write entry to the router / tunnel termination TCAM */
                rc = prvCpssDxChRouterTunnelTermTcamSetLine(
                    devNum,portGroupId,ruleIndex,
                    hwBuffersPtr->rawEntryFormatOperation_pattern,
                    hwBuffersPtr->rawEntryFormatOperation_mask,
                    hwBuffersPtr->rawEntryFormatOperation_action);
            }
            else
            {
                /* just invalidate the line */
                rc = prvCpssDxChRouterTunnelTermTcamInvalidateLine(
                    devNum,portGroupId,ruleIndex);
            }

            if (rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(
               devNum, portGroupsBmp, portGroupId)

    }

    return GT_OK;
}
/* operation type for rawEntryFormatOperation */
typedef enum{
    RAW_ENTRY_FORMAT_OPERATION_UPLOAD_HW_TO_SW_E,   /*read from HW to SW */
    RAW_ENTRY_FORMAT_OPERATION_DOWNLOAD_SW_TO_HW_E  /*write from SW to HW */
}RAW_ENTRY_FORMAT_OPERATION_ENT;

/**
* @internal rawEntryFormatOperation function
* @endinternal
*
* @brief   This function supports 2 operations :
*         1. copy raw entry (pattern,mask,action) from HW to SW buffer.
*         2. copy raw entry (pattern,mask,action) from SW buffer to HW.
*         This function copies the Rule's mask, pattern and action to new TCAM position.
*         The source Rule is not invalidated by the function. To implement move TCAM
*         Rule from old position to new one at first prvCpssDxChTcamRuleCopy
*         should be called. And after this cpssDxChTcamGortGroupRuleValidStatusSet should
*         be used to invalidate rule in old position.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vtcamInfoPtr             - (pointer to)virtual TCAM structure
* @param[in] operation                -  type
* @param[in] ruleHwIndex              - HW index of the rule in the TCAM . (to read from or to write to)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter, and when the rule cannot start in a given index.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
*
* @note function based on : prvCpssDxChPclSip5RuleCopy
*
*/
static GT_STATUS rawEntryFormatOperation
(
    IN   GT_U32                                        vTcamMngId,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC   *vtcamInfoPtr,
    IN    RAW_ENTRY_FORMAT_OPERATION_ENT                operation,
    IN  GT_U32                                          ruleHwIndex
)
{
    GT_STATUS   rc;
    GT_U8   devNum;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT ruleSize = (GT_U32)vtcamInfoPtr->tcamInfo.ruleSize;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* get first devNum iterator */
    rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* iteration over the devices */
    do
    {
        if(operation == RAW_ENTRY_FORMAT_OPERATION_DOWNLOAD_SW_TO_HW_E)
        {
            PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            /* write to HW */
            rc = rawEntryFormatOperationWriteToHw(
                devNum,
                vTcamMngDBPtr->portGroupBmpArr[devNum],
                vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
                &(vtcamInfoPtr->tcamSegCfgPtr->pVTcamMngPtr->hwBuffers),
                ruleSize,
                ruleHwIndex);
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            /* read from HW */
            rc = rawEntryFormatOperationReadFromHw(
                devNum,
                vTcamMngDBPtr->portGroupBmpArr[devNum],
                vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
                &(vTcamMngDBPtr->hwBuffers),
                ruleSize,
                ruleHwIndex);
            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
            }
            break;/* from single device */
        }
    }
    /* get next devNum iterator */
    while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));

    return GT_OK;
}

/**
* @internal vtcamFullMoveRuleByRawFormat function
* @endinternal
*
* @brief   we need to move HW entry from old place to new but the table is full and
*         FORCED resize didn't helped.
*         the rule will be absent from the HW for some time during the function
*         operation.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - rule id: that used to refer existing rule
* @param[in] priority                 - the new  of the rule
* @param[in] position                 -  in range of rule of the
*                                      specified priority (see enum)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
static GT_STATUS vtcamFullMoveRuleByRawFormat
(
    IN   GT_U32                                                    vTcamMngId,
    IN   GT_U32                                                    vTcamId,
    IN   PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC               *vtcamInfoPtr,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                            ruleId,
    IN   GT_U32                                                    priority,
    IN   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT   position,
    IN   GT_U32  orig_deviceRuleIndex,
    IN   GT_U32  orig_logicalIndex
)
{
    GT_STATUS rc;
    GT_U32  new_deviceRuleIndex;/* new HW index that will be for the ruleId */
    GT_U32  new_logicalIndex; /* new logical index that will be for the ruleId */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    GT_U32  orig_usedRulesAmount;

    /* the table is full and FORCED resize didn't helped */
    CPSS_LOG_INFORMATION_MAC("the Table is FULL and operation will remove the "
        "entry from the HW (for short time...) and then reposition it to new place \n");

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);

    /* 1. copy raw entry (pattern,mask,action) from HW to SW buffer.*/
    rc = rawEntryFormatOperation(vTcamMngId,vtcamInfoPtr,
        RAW_ENTRY_FORMAT_OPERATION_UPLOAD_HW_TO_SW_E,
        orig_deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    /* 2. remove the entry from the DB */
    {
        /* remove the old rule according to logical index */
        rc = priorityDbRemoveRuleId(vTcamMngDBPtr, vtcamInfoPtr, orig_logicalIndex);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    /* update the counter */
    orig_usedRulesAmount = vtcamInfoPtr->usedRulesAmount;
    vtcamInfoPtr->usedRulesAmount--;

    /* 3. re-calculate the new logical index ... now it must succeed
        (we have empty place according to DB) */
    /* Get logical index for the moved rule .
       Get place for new rule.
       If needed move existing rules.
       If needed resize vTCAM.
    */
    rc = priorityAddNewRuleNeedNewLogicalIndex(
        vTcamMngId,
        vTcamId,
        ORIG_INDEX_NOT_USED_CNS,/*origIndexForPriorityUpdate*/
        priority,
        position,
        &new_logicalIndex
    );
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    if(orig_usedRulesAmount != (vtcamInfoPtr->usedRulesAmount + 1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    new_deviceRuleIndex = vtcamInfoPtr->rulePhysicalIndexArr[new_logicalIndex];
    CPSS_LOG_INFORMATION_MAC("ruleId[%d] got new logical index[%d] and new HW index[%d] \n",
        ruleId,
        new_logicalIndex,
        new_deviceRuleIndex);

    /* 4. copy raw entry (pattern,mask,action) from SW buffer to HW.*/
    rc = rawEntryFormatOperation(vTcamMngId,vtcamInfoPtr,
        RAW_ENTRY_FORMAT_OPERATION_DOWNLOAD_SW_TO_HW_E,
        new_deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    /* update the DB about the new logical index */
    vtcamInfoPtr->usedRulesBitmapArr[new_logicalIndex>>5] |= (1<<(new_logicalIndex & 0x1f));

    /* update the DB about the 'new' rule */
    rc = priorityNewRuleIdAddedDbUpdate(vTcamMngId,vTcamId,
        ruleId,
        new_logicalIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }


    /*restore*/
    vtcamInfoPtr->usedRulesAmount = orig_usedRulesAmount;

    return GT_OK;
}

/**
* @internal internal_cpssDxChVirtualTcamRulePriorityGet function
* @endinternal
*
* @brief   Get Rule Priority (for priority driven Virtual TCAMs only).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   -  that used to refer existing rule.
*
* @param[out] rulePriorityPtr          - (pointer to)rule priority
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters and for logical index driven Virtual TCAMs.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamRulePriorityGet
(
    IN   GT_U32                                      vTcamMngId,
    IN   GT_U32                                      vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    OUT  GT_U32                                      *rulePriorityPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U32  orig_logicalIndex; /* logical index that converted from ruleId */

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod !=
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"the function relevant only to 'priority' mode");
    }

    /* check validity of the ruleId */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId, vTcamId, ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &orig_logicalIndex, NULL /*deviceRuleIndexPtr*/);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* get the priority of the orig ruleId */
    rc = prvCpssDxChVirtualTcamDbPriorityTableEntryFind(
        vtcamInfoPtr,
        orig_logicalIndex,
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,/*findType*/
        avlTreePath,
        &priorityEntry
    );
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    CPSS_LOG_INFORMATION_MAC("In the DB : the ruleId[%d] hold priority[%d] \n",
        ruleId,
        priorityEntry.priority);

    *rulePriorityPtr = priorityEntry.priority;
    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamRulePriorityGet function
* @endinternal
*
* @brief   Get Rule Priority (for priority driven Virtual TCAMs only).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   -  that used to refer existing rule.
*
* @param[out] rulePriorityPtr          - (pointer to)rule priority
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters and for logical index driven Virtual TCAMs.
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamRulePriorityGet
(
    IN   GT_U32                                      vTcamMngId,
    IN   GT_U32                                      vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    OUT  GT_U32                                      *rulePriorityPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRulePriorityGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId, rulePriorityPtr));

    rc = internal_cpssDxChVirtualTcamRulePriorityGet(vTcamMngId, vTcamId, ruleId, rulePriorityPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId, rulePriorityPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamRulePriorityUpdate function
* @endinternal
*
* @brief   Update rule priority (for priority driven vTCAM).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - rule id: that used to refer existing rule
* @param[in] priority                 - the new  of the rule
* @param[in] position                 -  in range of rule of the
*                                      specified priority (see enum)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FULL                  - when the TCAM (not only the vTcam) is FULL
*                                       and moving the rule will require the entry to
*                                       be absent from the HW for some time.
*                                       (the function can not guarantee loss less hits
*                                       on that rule)
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
static GT_STATUS internal_cpssDxChVirtualTcamRulePriorityUpdate
(
    IN   GT_U32                                                    vTcamMngId,
    IN   GT_U32                                                    vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                            ruleId,
    IN   GT_U32                                                    priority,
    IN   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT   position
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U8   devNum;
    GT_U32  orig_deviceRuleIndex;
    GT_U32  orig_logicalIndex; /* logical index that converted from ruleId */

    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_AVL_TREE_PATH  avlTreePath;
    GT_U32  new_deviceRuleIndex;/* new HW index that will be for the ruleId */
    GT_U32  new_logicalIndex; /* new logical index that will be for the ruleId */
    GT_BOOL isAutoResize;
    GT_BOOL tableIsFull = GT_FALSE;
    GT_U32  actualResizeNumRules = 0;/* relevant when tableIsFull == GT_TRUE */

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod !=
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"the function relevant only to 'priority' mode");
    }

    /* check input param */
    switch(position)
    {
        case CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_FIRST_E:
        case CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_LAST_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Unknown position[%d] parameter",
                position);
    }

    /* check validity of the ruleId */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &orig_logicalIndex,&orig_deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* get the priority of the orig ruleId */
    rc = prvCpssDxChVirtualTcamDbPriorityTableEntryFind(
        vtcamInfoPtr,
        orig_logicalIndex,
        PRV_CPSS_AVL_TREE_FIND_MAX_LESS_OR_EQUAL_E,/*findType*/
        avlTreePath,
        &priorityEntry
    );
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    CPSS_LOG_INFORMATION_MAC("In the DB : the ruleId[%d] hold priority[%d] \n",
        ruleId,
        priorityEntry.priority);

    if(priorityEntry.baseLogIndex > orig_logicalIndex ||
       ((priorityEntry.baseLogIndex + priorityEntry.rangeSize) <= orig_logicalIndex))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"DB error: the orig logical index [%d] is out of the 'orig' priority range[%d..%d]",
            orig_logicalIndex,
            priorityEntry.baseLogIndex,
            priorityEntry.baseLogIndex + priorityEntry.rangeSize - 1);
    }

    if(vtcamInfoPtr->rulesAmount == vtcamInfoPtr->usedRulesAmount)
    {
        tableIsFull = GT_TRUE;
    }
    else
    {
        tableIsFull = GT_FALSE;
    }

    if(tableIsFull == GT_TRUE)
    {
        /* increase the table by 1 for the new priority*/
        rc = cpssDxChVirtualTcamResize(vTcamMngId,vTcamId,
            priority,GT_TRUE/*toInsert*/,1/*sizeInRules*/);

        /* don't care about rc !
            even if it failed ... the values of
                vtcamInfoPtr->rulesAmount , vtcamInfoPtr->usedRulesAmount
            are ok
        */
        actualResizeNumRules = vtcamInfoPtr->rulesAmount -
                               vtcamInfoPtr->usedRulesAmount;

        /* recalculate after resize */
        rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
            vTcamMngId,vTcamId,ruleId,
            GT_TRUE,/* check that the ruleId already exists in the DB */
            &orig_logicalIndex,&orig_deviceRuleIndex);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* save the value */
    isAutoResize = vtcamInfoPtr->tcamInfo.autoResize;
    /* FORCE the operation to not do any auto resize */
    vtcamInfoPtr->tcamInfo.autoResize = GT_FALSE;

    /* Get logical index for the moved rule .
       Get place for new rule.
       If needed move existing rules.
       If needed resize vTCAM.
    */
    rc = priorityAddNewRuleNeedNewLogicalIndex(
        vTcamMngId,
        vTcamId,
        orig_logicalIndex,/*origIndexForPriorityUpdate*/
        priority,
        position,
        &new_logicalIndex
    );
    /*NOTE: the 'rc' will be checked later !!!
      do not return from here !!!!*/

    if((rc == GT_FULL) && (tableIsFull == GT_TRUE) && (actualResizeNumRules == 0))
    {
        /* the table is full and FORCED resize didn't helped */

        /*
        * when the TCAM (not only the vTcam) is FULL
        * and moving the rule will require the entry to
        * be absent from the HW for some time.
        * (the function can not guarantee loss less hits
        *  on that rule)
        */

        if(allowRuleToBeAbsentFromTcamDuringPriorityUpdate == GT_TRUE)
        {
            rc = vtcamFullMoveRuleByRawFormat(vTcamMngId,vTcamId,
                vtcamInfoPtr,ruleId,priority,position,
                orig_deviceRuleIndex,orig_logicalIndex);
        }
        else
        {
            rc = GT_FULL;
        }

        /* restore value */
        vtcamInfoPtr->tcamInfo.autoResize = isAutoResize;

        return /* this is NOT error ! it valid case that we can't support */ rc;
    }

    /* restore value */
    vtcamInfoPtr->tcamInfo.autoResize = isAutoResize;

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    /* convert AGAIN the ruleId to logical index and to physical index
       because the logic may have moved it (when trying to find place for the new priority)
    */
    rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        &orig_logicalIndex,&orig_deviceRuleIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    new_deviceRuleIndex = vtcamInfoPtr->rulePhysicalIndexArr[new_logicalIndex];
    CPSS_LOG_INFORMATION_MAC("ruleId[%d] got new logical index[%d] and new HW index[%d] \n",
        ruleId,
        new_logicalIndex,
        new_deviceRuleIndex);

    CPSS_LOG_INFORMATION_MAC("start removing the original logical index [%d] for the rule \n",
        orig_logicalIndex);

    if(orig_logicalIndex != new_logicalIndex)
    {
        if (VTCAM_GLOVAR(priorityAddNewRuleNeedNewLogicalIndexFuncionVersion) == 0)
        {
            /* remove the old rule according to logical index */
            rc = priorityDbRemoveRuleId(vTcamMngDBPtr, vtcamInfoPtr, orig_logicalIndex);
        }
        else
        {
           /* only Remove rule Id entry by logical index from tree : ruleIdIndexTree */
            rc = prvCpssDxChVirtualTcamDbRuleIdTableEntryRemoveByLogicalIndex(
                    vTcamMngDBPtr,
                    vtcamInfoPtr,
                    orig_logicalIndex);
        }
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }
    else
    {
        /* only Remove rule Id entry by logical index from tree : ruleIdIndexTree */
        rc = prvCpssDxChVirtualTcamDbRuleIdTableEntryRemoveByLogicalIndex(
                vTcamMngDBPtr,
                vtcamInfoPtr,
                orig_logicalIndex);
        if(rc != GT_OK)
        {
            return rc;
        }
    }


    /* remove the original logical index from the DB */
    vtcamInfoPtr->usedRulesBitmapArr[orig_logicalIndex>>5] &= ~(1<<(orig_logicalIndex & 0x1f));

    CPSS_LOG_INFORMATION_MAC("start adding the new logical index [%d] for the rule \n",
        new_logicalIndex);

    if(orig_deviceRuleIndex != new_deviceRuleIndex)
    {
        /* get first devNum iterator */
        rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMngDBPtr,&devNum);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* iteration over the devices */
        do
        {
            PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

            /* move the rule in the device */
            rc = perDevice_ruleMove(
                devNum,
                vTcamMngDBPtr,
                vTcamMngDBPtr->portGroupBmpArr[devNum],
                vtcamInfoPtr->tcamSegCfgPtr->deviceClass,
                vtcamInfoPtr->tcamInfo.ruleSize,
                orig_deviceRuleIndex,
                new_deviceRuleIndex,
                GT_TRUE/*move*/);

            CPSS_API_UNLOCK_NO_CHECKS_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

            if(rc != GT_OK)
            {
                return rc;
            }
        }
        /* get next devNum iterator */
        while(prvCpssDxChVirtualTcamDbDeviceIteratorGetNext(vTcamMngDBPtr,&devNum));
    }

    /* update the DB about the new logical index */
    vtcamInfoPtr->usedRulesBitmapArr[new_logicalIndex>>5] |= (1<<(new_logicalIndex & 0x1f));

    /* update the DB about the 'new' rule */
    rc = priorityNewRuleIdAddedDbUpdate(vTcamMngId,vTcamId,
        ruleId,
        new_logicalIndex);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    if(actualResizeNumRules && (tableIsFull == GT_TRUE))
    {
        /* decrease the table by the value that we caused to add */
        rc = cpssDxChVirtualTcamResize(vTcamMngId,vTcamId,
            priority,GT_FALSE/*toInsert*/,actualResizeNumRules/*sizeInRules*/);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamRulePriorityUpdate function
* @endinternal
*
* @brief   Update rule priority (for priority driven vTCAM).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - rule id: that used to refer existing rule
* @param[in] priority                 - the new  of the rule
* @param[in] position                 -  in range of rule of the
*                                      specified priority (see enum)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_FULL                  - when the TCAM (not only the vTcam) is FULL
*                                       and moving the rule will require the entry to
*                                       be absent from the HW for some time.
*                                       (the function can not guarantee loss less hits
*                                       on that rule)
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRulePriorityUpdate
(
    IN   GT_U32                                                    vTcamMngId,
    IN   GT_U32                                                    vTcamId,
    IN   CPSS_DXCH_VIRTUAL_TCAM_RULE_ID                            ruleId,
    IN   GT_U32                                                    priority,
    IN   CPSS_DXCH_VIRTUAL_TCAM_EQUAL_PRIORITY_RULE_POSITION_ENT   position
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRulePriorityUpdate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId, priority, position));

    rc = internal_cpssDxChVirtualTcamRulePriorityUpdate(vTcamMngId, vTcamId, ruleId, priority, position);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId, priority, position));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamDbRuleIdToHwIndexConvert function
* @endinternal
*
* @brief   debug function - convert rule Id to logical index and to hardware index.
*         purpose is to allow application to know where the CPSS inserted the rule
*         in to the hardware.
*         this is 'DB' (DataBase) operation that not access the hardware.
*         NOTE: the function will return failure if the ruleId was not inserted.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   -  to find.
*
* @param[out] logicalIndexPtr          - (pointer to) the logical index.  (can be NULL)
* @param[out] hwIndexPtr               - (pointer to) the hardware index. (can be NULL)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - if both logicalIndexPtr and hwIndexPtr are NULL
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS internal_cpssDxChVirtualTcamDbRuleIdToHwIndexConvert
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              ruleId,
    OUT  GT_U32                                     *logicalIndexPtr,
    OUT  GT_U32                                     *hwIndexPtr
)
{
    if(logicalIndexPtr == NULL && hwIndexPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    /* convert the ruleId to logical index and to physical index */
    return prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
        vTcamMngId,vTcamId,ruleId,
        GT_TRUE,/* check that the ruleId already exists in the DB */
        logicalIndexPtr,hwIndexPtr);
}

/**
* @internal cpssDxChVirtualTcamDbRuleIdToHwIndexConvert function
* @endinternal
*
* @brief   debug function - convert rule Id to logical index and to hardware index.
*         purpose is to allow application to know where the CPSS inserted the rule
*         in to the hardware.
*         this is 'DB' (DataBase) operation that not access the hardware.
*         NOTE: the function will return failure if the ruleId was not inserted.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   -  to find.
*
* @param[out] logicalIndexPtr          - (pointer to) the logical index.  (can be NULL)
* @param[out] hwIndexPtr               - (pointer to) the hardware index. (can be NULL)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - if both logicalIndexPtr and hwIndexPtr are NULL
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssDxChVirtualTcamDbRuleIdToHwIndexConvert
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN   GT_U32                                     ruleId,
    OUT  GT_U32                                     *logicalIndexPtr,
    OUT  GT_U32                                     *hwIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamDbRuleIdToHwIndexConvert);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId, logicalIndexPtr, hwIndexPtr));

    rc = internal_cpssDxChVirtualTcamDbRuleIdToHwIndexConvert(vTcamMngId, vTcamId, ruleId, logicalIndexPtr, hwIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId, logicalIndexPtr, hwIndexPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamUsageGet function
* @endinternal
*
* @brief   Get Virtual TCAM usage.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*
* @param[out] vTcamUsagePtr            - (pointer to) Virtual TCAM Usage structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamUsageGet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    OUT  CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC   *vTcamUsagePtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;

    CPSS_NULL_PTR_CHECK_MAC(vTcamUsagePtr);

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    vTcamUsagePtr->rulesUsed = vtcamInfoPtr->usedRulesAmount;
    vTcamUsagePtr->rulesFree = vtcamInfoPtr->rulesAmount - vtcamInfoPtr->usedRulesAmount;

    return GT_OK;
}
/**
* @internal cpssDxChVirtualTcamUsageGet function
* @endinternal
*
* @brief   Get Virtual TCAM usage.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*
* @param[out] vTcamUsagePtr            - (pointer to) Virtual TCAM Usage structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamUsageGet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    OUT  CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC   *vTcamUsagePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamUsageGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, vTcamUsagePtr));

    rc = internal_cpssDxChVirtualTcamUsageGet(vTcamMngId, vTcamId, vTcamUsagePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, vTcamUsagePtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamInfoGet function
* @endinternal
*
* @brief   Get Virtual TCAM info.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*
* @param[out] vTcamInfoPtr             - (pointer to) Virtual TCAM Info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamInfoGet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    OUT  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC    *vTcamInfoPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;

    CPSS_NULL_PTR_CHECK_MAC(vTcamInfoPtr);

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "vTcamMngId[%d] : vTcamId[%d] not exists",
            vTcamMngId,vTcamId);
    }

    *vTcamInfoPtr = vtcamInfoPtr->tcamInfo;

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamInfoGet function
* @endinternal
*
* @brief   Get Virtual TCAM info.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*
* @param[out] vTcamInfoPtr             - (pointer to) Virtual TCAM Info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamInfoGet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    OUT  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC    *vTcamInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamInfoGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, vTcamInfoPtr));

    rc = internal_cpssDxChVirtualTcamInfoGet(vTcamMngId, vTcamId, vTcamInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, vTcamInfoPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);


    return rc;
}


/**
* @internal internal_cpssDxChVirtualTcamCreate function
* @endinternal
*
* @brief   Create Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] vTcamInfoPtr             - (pointer to) Virtual TCAM info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_ALREADY_EXIST         - vTcam with given Id exists
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
static GT_STATUS internal_cpssDxChVirtualTcamCreate
(
    IN  GT_U32                            vTcamMngId,
    IN  GT_U32                            vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC   *vTcamInfoPtr
)
{
    GT_STATUS                                   rc;               /* return code */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *vTcamMngDBPtr;   /* TCAM manager */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *tcamCfgPtr;      /* vTCAM*/
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC   *tcamSegCfgPtr;
    GT_U32                                      bmpSize;          /* size of used rules BMP */
    GT_U32                                      maxNumOfRules;    /* maximal number of rules */
    GT_BOOL                                     managerHwWriteBlock;    /*used to check if current manager is blocked for HW write ,used for parallel High Availability*/
    CPSS_SYSTEM_RECOVERY_INFO_STC               tempSystemRecovery_Info; /*holds system recovery info */

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if (vTcamMngDBPtr == NULL)
    {
        /* can't find the vTcamMng DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    if (vTcamMngDBPtr->vTcamCfgPtrArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (vTcamId >= vTcamMngDBPtr->vTcamCfgPtrArrSize)
    {
        /* vTcamId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (vTcamInfoPtr->tcamSpaceUnmovable != GT_FALSE)
    {
        /* unmovable vTCAM cannot be resized both auto both by resize API call */
        if (vTcamInfoPtr->autoResize != GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    tcamSegCfgPtr = &vTcamMngDBPtr->tcamSegCfg;     /* Default */
    if ((vTcamMngDBPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
        (vTcamMngDBPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E) ||
        (vTcamMngDBPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
    {
        if (vTcamInfoPtr->clientGroup == 0)
        {
            /* group #0 (PCL) in this device family */
            tcamSegCfgPtr = &vTcamMngDBPtr->tcamSegCfg;
        }
        else
        if (vTcamInfoPtr->clientGroup == 1)
        {
            /* group #1 (TTI) in this device family */
            tcamSegCfgPtr = &vTcamMngDBPtr->tcamTtiSegCfg;
        }
        else if ((vTcamMngDBPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E) && (vTcamInfoPtr->clientGroup == 2))
        {
            /* group #2 (PCL TCAM1) in this device family */
            tcamSegCfgPtr = &vTcamMngDBPtr->pclTcam1SegCfg;
        }
        else
        {
            /* Only groups #0/1 (PCL/TTI) are allowed on this device family */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (vTcamInfoPtr->hitNumber != 0)
        {
            /* Parallel lookups not allowed on this device family */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        switch (vTcamInfoPtr->ruleSize)
        {
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                /* OK */
                break;
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
            case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                if (tcamSegCfgPtr->deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E)
                {
                    /* Invalid size on this device class */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
                /* OK */
                break;
            default:
                /* Invalid size on this device family */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                break;
        }
    }

    if (vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId] != NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }

    vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId] =
        (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*)
        cpssOsMalloc(sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC));

    if (vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    tcamCfgPtr = vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId];
    cpssOsMemSet(tcamCfgPtr, 0, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC));

    /* Initialise */
    tcamCfgPtr->haLastConfigReplayPhysicalIndex = 0xffffffff;

    /* save the caller info */
    tcamCfgPtr->tcamInfo = *vTcamInfoPtr;
    tcamCfgPtr->tcamSegCfgPtr = tcamSegCfgPtr;

    /* allocation of trees */
    /*Create Local per-vTcam Access Trees for Rule Id Table.*/
    rc = prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesCreate(
        vTcamMngDBPtr,  tcamCfgPtr);
    if (rc != GT_OK)
    {
        cpssOsFree(tcamCfgPtr);
        vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId] = NULL;
        return rc;
    }
    /*Create Local per-vTcam Access Trees for Priority Table.*/
    rc = prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesCreate(
        vTcamMngDBPtr,  tcamCfgPtr);
    if (rc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesDelete(
            vTcamMngDBPtr,  tcamCfgPtr);
        cpssOsFree(tcamCfgPtr);
        vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId] = NULL;
        return rc;
    }

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_TCAM_MANAGER_E);
    if (vTcamMngDBPtr->haSupported && (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E ||
        managerHwWriteBlock == GT_TRUE))
    {
        rc = prvCpssAvlTreeCreate(
                (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
                tcamSegCfgPtr->pVTcamMngPtr->segNodesPool, &vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId]->segmentsTree);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssAvlTreeCreate failed");
        }

    }
    else
    {
        /*Allocate memory for Virtual TCAM in Segment Table for TCAM.*/
        rc = vtcamDbSegmentTableVTcamAllocOrResize(
                vTcamMngDBPtr, vTcamId,
                0/* no realloc*/,NULL/*NA*/);
        if (rc != GT_OK)
        {
            prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete(
                    vTcamMngDBPtr,  tcamCfgPtr);
            prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesDelete(
                    vTcamMngDBPtr,  tcamCfgPtr);
            cpssOsFree(tcamCfgPtr);
            vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId] = NULL;
            return rc;
        }
    }

    if (vTcamInfoPtr->guaranteedNumOfRules)
    {
        if (tcamCfgPtr->rulesAmount >= vTcamInfoPtr->guaranteedNumOfRules)
        {
            maxNumOfRules = tcamCfgPtr->rulesAmount;
        }
        else
        {
            /* HA case, rulesAmount is not assigned yet.
               use maximal possible value after alignments. */
            maxNumOfRules = vTcamInfoPtr->guaranteedNumOfRules +
                            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_DEFAULT_AUTO_RESIZE_GRANULARITY_CNS;
        }

        bmpSize = (((maxNumOfRules + 31) / 32) * sizeof(GT_U32));
        tcamCfgPtr->usedRulesBitmapArr = (GT_U32*)cpssOsMalloc(bmpSize);
        if (tcamCfgPtr->usedRulesBitmapArr == NULL)
        {
            prvCpssDxChVirtualTcamDbSegmentTableVTcamFree(tcamCfgPtr);
            prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete(
                vTcamMngDBPtr,  tcamCfgPtr);
            prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesDelete(
                vTcamMngDBPtr,  tcamCfgPtr);
            cpssOsFree(tcamCfgPtr);
            vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId] = NULL;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }
        /*reset the allocated memory */
        cpssOsMemSet(tcamCfgPtr->usedRulesBitmapArr,0,bmpSize);

        tcamCfgPtr->usedRulesBitmapArrSizeHa = bmpSize;
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamCreate function
* @endinternal
*
* @brief   Create Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] vTcamInfoPtr             - (pointer to) Virtual TCAM info structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_ALREADY_EXIST         - vTcam with given Id exists
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamCreate
(
    IN  GT_U32                           vTcamMngId,
    IN  GT_U32                           vTcamId,
    IN  CPSS_DXCH_VIRTUAL_TCAM_INFO_STC   *vTcamInfoPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamCreate);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, vTcamInfoPtr));

    rc = internal_cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, vTcamInfoPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, vTcamInfoPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamRemove function
* @endinternal
*
* @brief   Remove Virtual TCAM - with option to remove not empty vTCAM from DB
*         without cleaning HW.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] failOnHavingRules        - GT_TRUE  - to fail if vTcam contains valid rules
*                                      - GT_FALSE - to remove vTcam anyway
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - vTcam contains rules (either valid or invalid rules)
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS prvCpssDxChVirtualTcamRemove
(
    IN  GT_U32                             vTcamMngId,
    IN  GT_U32                             vTcamId,
    IN  GT_BOOL                            failOnHavingRules
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC          *vTcamMngDBPtr;   /* TCAM manager */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *tcamCfgPtr;      /* vTCAM*/

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"vTcamMngId %d out of range", vTcamMngId);
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if (vTcamMngDBPtr == NULL)
    {
        /* can't find the vTcamMng DB */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,"Can't find the vTcamMng DB");
    }

    if (vTcamId >= vTcamMngDBPtr->vTcamCfgPtrArrSize)
    {
        /* vTcamId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"vTcamId %d out of range", vTcamId);
    }

    if (vTcamMngDBPtr->vTcamCfgPtrArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId] == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND,"Can't find the vTcam [%d] in DB",vTcamId);
    }

    tcamCfgPtr = vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId];
    if ((failOnHavingRules != GT_FALSE) && (tcamCfgPtr->usedRulesAmount))
    {
        /* vTcam contains valid rules */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"vTcam[%d] contains [%d]valid rules",
            vTcamId,tcamCfgPtr->usedRulesAmount);
    }

    /* free rule ID tree */
    rc = prvCpssDxChVirtualTcamDbRuleIdTableVTcamTreesDelete(
        vTcamMngDBPtr,  tcamCfgPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete failed");
    }
    /* free priority trees */
    rc = prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete(
        vTcamMngDBPtr,  tcamCfgPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamDbPriorityTableVTcamTreesDelete failed");
    }
    /* free segment table tree */
    rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamFree(tcamCfgPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChVirtualTcamDbSegmentTableVTcamFree failed");
    }

    /* free rule state bitmap */
    if (tcamCfgPtr->usedRulesBitmapArr != NULL)
    {
        cpssOsFree(tcamCfgPtr->usedRulesBitmapArr);
    }

    /* free TCAM manager */
    cpssOsFree(vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId]);
    vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId] = 0;

    return GT_OK;
}

/**
* @internal internal_cpssDxChVirtualTcamRemove function
* @endinternal
*
* @brief   Remove Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - vTcam contains rules (either valid or invalid rules)
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
static GT_STATUS internal_cpssDxChVirtualTcamRemove
(
    IN  GT_U32                             vTcamMngId,
    IN  GT_U32                             vTcamId
)
{
    return prvCpssDxChVirtualTcamRemove(vTcamMngId, vTcamId, GT_TRUE);
}

/**
* @internal cpssDxChVirtualTcamRemove function
* @endinternal
*
* @brief   Remove Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - vTcam contains rules (either valid or invalid rules)
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamRemove
(
    IN  GT_U32                             vTcamMngId,
    IN  GT_U32                             vTcamId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamRemove);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId));

    rc = internal_cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamResize function
* @endinternal
*
* @brief   Resize Virtual TCAM. Guaranteed amount of rules updated.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] rulePlace                - place to insert/remove space after it.
*                                      for logical-index driven vTCAMs - logical-index,
*                                      for priority driven vTCAMs - priority value.
*                                      ----------------------------------------
*                                      For logical-index driven vTCAMs:
*                                      a) toInsert == GT_TRUE
*                                      Function adds free space with size <sizeInRules> rules
*                                      after logical-index == rulePlace.
*                                      Logical indexes of existing rules higher than inserted space
*                                      (i.e. rules with old-logical-index >= rulePlace)
*                                      are increased by <sizeInRules>.
*                                      b) toInsert == GT_FALSE
*                                      Function invalidates existing rules with logical indexes
*                                      between <rulePlace> and <rulePlace + sizeInRules - 1>.
*                                      This space became free. Function decreases logical indexes of
*                                      existing rules higher than freed space
*                                      (i.e. rules with old-logical-index >= rulePlace) by <sizeInRules>.
*                                      ----------------------------------------
*                                      For priority driven vTCAMs:
*                                      a) toInsert == GT_TRUE
*                                      Function adds free space with size <sizeInRules> rules
*                                      after all rules with priority == rulePlace.
*                                      b) toInsert == GT_FALSE
*                                      Function removes and frees space of <sizeInRules> rules
*                                      after all rules with priority == rulePlace.
*                                      No rules are removed, but rules can be moved in TCAM
*                                      to gather free spaces to one chunk .
*                                      Rules with given and lower priorities can be compressed
*                                      using free spaces between them.
*                                      ----------------------------------------
*                                      CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS - used
*                                      for logical-index driven vTCAMs only
*                                      to append free space (i.e toInsert == GT_TRUE)
* @param[in] toInsert                 - GT_TRUE - insert rule-places, GT_FALSE - remove rule-places
*                                      logical index of rules up to inserted space increased,
*                                      logical index of rules up to removed space - decreased,
*                                      rules inside removed space - removed.
* @param[in] sizeInRules              - amount of rule-space to insert/remove
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_SUPPORTED         - on unmovable vTCAM
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*
* @note Example1 (logical-index driven vTCAM):
*       Old vTcam size = 200, rulePlace = 50, toInsert = GT_TRUE, sizeInRules = 100
*       New vTam size is 300.
*       Rules 0-49 logical index unchanged.
*       Rules 50-199 logical index increased by 100 to be 150-299.
*       Place for rules 50-149 added, contains no valid rules.
*       Example2 (logical-index driven vTCAM)
*       Old vTcam size = 300, rulePlace = 50, toInsert = GT_FALSE, sizeInRules = 100
*       New vTam size is 200.
*       Rules 0-49 logical index unchanged.
*       Rules 150-299 logical index decreased by 100 to be 50-199.
*       Place of rules 50-149 removed with all rules that contained.
*
*/
static GT_STATUS internal_cpssDxChVirtualTcamResize
(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  GT_U32                                rulePlace,
    IN  GT_BOOL                               toInsert,
    IN  GT_U32                                sizeInRules
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    priorityEntry;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U32      baseRuleIndex;
    GT_U32      priority;
    RESIZE_MODE_ENT resizeMode;

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"vTcamId[%d] was not created \n",
            vTcamId);
    }

    if (vtcamInfoPtr->tcamInfo.tcamSpaceUnmovable != GT_FALSE)
    {
        /* unmovable vTCAM cannot be resized */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"vTcamId[%d] unmovable \n", vTcamId);
    }

    /* check that at least one device exists , otherwise return error */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_DEVS_BITMAP_IS_EMPTY_MAC(vTcamMngId);

    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E)
    {
        if(rulePlace == CPSS_DXCH_VIRTUAL_TCAM_BEFORE_START_CNS && (toInsert == GT_TRUE))
        {
            /* 'before' first logical index */
            baseRuleIndex = 0;
            resizeMode = RESIZE_MODE_WITH_INPUT_INDEX_E;/*include move of baseRuleIndex */
        }
        else
        if(rulePlace == CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS && (toInsert == GT_TRUE))
        {
            /* 'after' last logical index */
            baseRuleIndex = vtcamInfoPtr->rulesAmount;
            resizeMode = RESIZE_MODE_WITHOUT_INPUT_INDEX_E;/*add from baseRuleIndex */
        }
        else
        {
            if(rulePlace >= vtcamInfoPtr->rulesAmount)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            /* 'after' current logical index */
            baseRuleIndex = rulePlace;
            resizeMode = RESIZE_MODE_WITH_INPUT_INDEX_E;/*include move of baseRuleIndex */
        }

        if(toInsert == GT_FALSE)
        {
            if((baseRuleIndex + sizeInRules) > vtcamInfoPtr->rulesAmount)
            {
                /* try to remove rules beyond the range of logical indexes */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

    }
    else /* priority mode */
    {
        priority = rulePlace;

        /* find the info for this priority */
        rc = prvCpssDxChVirtualTcamDbPriorityTableEntryGet(
            vtcamInfoPtr,priority,&priorityEntry);
        if(rc != GT_OK)
        {
            return rc;
        }

        baseRuleIndex = priorityEntry.baseLogIndex + priorityEntry.rangeSize;
        resizeMode = RESIZE_MODE_WITH_INPUT_INDEX_E;/*include move of baseRuleIndex */
    }

    if(sizeInRules == 0)
    {
        return GT_OK;
    }

    if(toInsert == GT_FALSE)
    {
        if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E)
        {
            rc = vtcamDbSegmentTableVTcamDownSize__LogicalIndexMode(vTcamMngId ,vTcamId,
                sizeInRules, baseRuleIndex);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            rc = vtcamDbSegmentTableVTcamDownSize__PriorityMode(vTcamMngId ,vTcamId,
                sizeInRules);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        /* resize memory for Virtual TCAM in Segment Table for TCAM.*/
        rc = vtcamDbSegmentTableVTcamResize(vTcamMngId ,vTcamId,
            sizeInRules, resizeMode , baseRuleIndex);

        if (VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr) != NULL)
        {
            cpssOsFree(VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr));
            VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr) = NULL;
            VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr) = 0;
        }

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamResize function
* @endinternal
*
* @brief   Resize Virtual TCAM. Guaranteed amount of rules updated.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] rulePlace                - place to insert/remove space after it.
*                                      for logical-index driven vTCAMs - logical-index,
*                                      for priority driven vTCAMs - priority value.
*                                      ----------------------------------------
*                                      For logical-index driven vTCAMs:
*                                      a) toInsert == GT_TRUE
*                                      Function adds free space with size <sizeInRules> rules
*                                      after logical-index == rulePlace.
*                                      Logical indexes of existing rules higher than inserted space
*                                      (i.e. rules with old-logical-index >= rulePlace)
*                                      are increased by <sizeInRules>.
*                                      b) toInsert == GT_FALSE
*                                      Function invalidates existing rules with logical indexes
*                                      between <rulePlace> and <rulePlace + sizeInRules - 1>.
*                                      This space became free. Function decreases logical indexes of
*                                      existing rules higher than freed space
*                                      (i.e. rules with old-logical-index >= rulePlace) by <sizeInRules>.
*                                      ----------------------------------------
*                                      For priority driven vTCAMs:
*                                      a) toInsert == GT_TRUE
*                                      Function adds free space with size <sizeInRules> rules
*                                      after all rules with priority == rulePlace.
*                                      b) toInsert == GT_FALSE
*                                      Function removes and frees space of <sizeInRules> rules
*                                      after all rules with priority == rulePlace.
*                                      No rules are removed, but rules can be moved in TCAM
*                                      to gather free spaces to one chunk .
*                                      Rules with given and lower priorities can be compressed
*                                      using free spaces between them.
*                                      ----------------------------------------
*                                      CPSS_DXCH_VIRTUAL_TCAM_APPEND_CNS - used
*                                      for logical-index driven vTCAMs only
*                                      to append free space (i.e toInsert == GT_TRUE)
* @param[in] toInsert                 - GT_TRUE - insert rule-places, GT_FALSE - remove rule-places
*                                      logical index of rules up to inserted space increased,
*                                      logical index of rules up to removed space - decreased,
*                                      rules inside removed space - removed.
* @param[in] sizeInRules              - amount of rule-space to insert/remove
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NO_RESOURCE           - on tree/buffers resource errors
* @retval GT_FULL                  - the TCAM space is full for current allocation
*                                       request
*
* @note Example1 (logical-index driven vTCAM):
*       Old vTcam size = 200, rulePlace = 50, toInsert = GT_TRUE, sizeInRules = 100
*       New vTam size is 300.
*       Rules 0-49 logical index unchanged.
*       Rules 50-199 logical index increased by 100 to be 150-299.
*       Place for rules 50-149 added, contains no valid rules.
*       Example2 (logical-index driven vTCAM)
*       Old vTcam size = 300, rulePlace = 50, toInsert = GT_FALSE, sizeInRules = 100
*       New vTam size is 200.
*       Rules 0-49 logical index unchanged.
*       Rules 150-299 logical index decreased by 100 to be 50-199.
*       Place of rules 50-149 removed with all rules that contained.
*
*/
GT_STATUS cpssDxChVirtualTcamResize
(
    IN  GT_U32                                vTcamMngId,
    IN  GT_U32                                vTcamId,
    IN  GT_U32                                rulePlace,
    IN  GT_BOOL                               toInsert,
    IN  GT_U32                                sizeInRules
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamResize);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, rulePlace, toInsert, sizeInRules));

    rc = internal_cpssDxChVirtualTcamResize(vTcamMngId, vTcamId, rulePlace, toInsert, sizeInRules);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, rulePlace, toInsert, sizeInRules));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChVirtualTcamNextRuleIdGet function
* @endinternal
*
* @brief   Function gets next existing rule ID.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*                                      CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS used to start iteration of rules
*
* @param[out] nextRuleIdPtr            -      (pointer to)id of next rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_EMPTY                 - on 'START_RULE_ID' indication and no rules exists.
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NO_MORE               - all rule already passed by iteration
*/
static GT_STATUS internal_cpssDxChVirtualTcamNextRuleIdGet
(
    IN  GT_U32        vTcamMngId,
    IN  GT_U32        vTcamId,
    IN  GT_U32        ruleId,
    OUT GT_U32        *nextRuleIdPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;
    GT_U32  logicalIndex; /* logical index that converted from ruleId */
    GT_U32  nextLogicalIndex;/* next logical index*/
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    GT_BOOL    found;/* indication that the current ruleId found */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC    needToFindInfo;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC    *foundRuleIdEntryPtr;

    CPSS_NULL_PTR_CHECK_MAC(nextRuleIdPtr);

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    if (vtcamInfoPtr->rulesAmount == 0)
    {
        /* empty table */
        return /* NOT to register as error to LOG !!! */ GT_EMPTY;
    }

    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E)
    {
        if(ruleId == CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS)
        {
            logicalIndex = 0;
            if((vtcamInfoPtr->usedRulesBitmapArr != NULL) &&
                (vtcamInfoPtr->usedRulesBitmapArr[0] & BIT_0))
            {
                *nextRuleIdPtr = 0;
                return GT_OK;
            }
        }
        else
        {
            /* convert the ruleId to logical index */
            rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
                vTcamMngId,vTcamId,ruleId,
                GT_FALSE,/* don't check that the ruleId already exists in the DB */
                &logicalIndex,NULL);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        /* find nearest to specific logical index that is used logical index */
        rc = findNearestLogicalIndexUsed(vtcamInfoPtr,
            logicalIndex,
            GT_FALSE, /*more : lessOrMore*/
            0xFFFFFFFF,/*lastLogicalIndexToCheck*/
            &nextLogicalIndex
        );

        if(rc != GT_OK)
        {
            if(ruleId == CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS)
            {
                /* empty table */
                return /* NOT to register as error to LOG !!! */ GT_EMPTY;
            }
            else
            {
                return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
            }
        }

        /* return the 'next' ruleId */
        *nextRuleIdPtr = nextLogicalIndex;

        return GT_OK;
    }

    /* vtcamInfoPtr->tcamInfo.ruleAdditionMethod ==
        CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E*/


    if(ruleId == CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS)
    {
        found = prvCpssAvlPathSeek(vtcamInfoPtr->ruleIdIndexTree,
            PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            avlTreePath,
            &dbEntryPtr /* use dedicated var to avoid warnings */);

        if(found == GT_FALSE)
        {
            /* empty table */
            return /* NOT to register as error to LOG !!! */ GT_EMPTY;
        }

        foundRuleIdEntryPtr = dbEntryPtr;
    }
    else
    {
        /* convert the ruleId to logical index */
        rc = prvCpssDxChVirtualTcamDbRuleIdToLogicalIndexConvert(
            vTcamMngId,vTcamId,ruleId,
            GT_TRUE,/* check that the ruleId already exists in the DB */
            &logicalIndex,NULL);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        cpssOsMemSet(&needToFindInfo,0,sizeof(needToFindInfo));
        needToFindInfo.logicalIndex = logicalIndex;

        found = prvCpssAvlPathFind(vtcamInfoPtr->ruleIdIndexTree,
            PRV_CPSS_AVL_TREE_FIND_MIN_GREATER_OR_EQUAL_E,
            &needToFindInfo,/*pData*/
            avlTreePath,/*path*/
            &dbEntryPtr /* use dedicated var to avoid warnings */);

        foundRuleIdEntryPtr = dbEntryPtr;

        if(found == GT_TRUE &&
           foundRuleIdEntryPtr->logicalIndex == logicalIndex)
        {
            /* the DB found 'equal' .. but we need 'next' ,
               use the found node to get the next one ! */
            found = prvCpssAvlPathSeek(vtcamInfoPtr->ruleIdIndexTree,
                PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                avlTreePath,
                &dbEntryPtr /* use dedicated var to avoid warnings */);

            foundRuleIdEntryPtr = dbEntryPtr;

            if(found == GT_TRUE &&
               foundRuleIdEntryPtr->logicalIndex == logicalIndex)
            {
                /* the 'next' hold info of 'current' ?! */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "DB error: the 'next' hold info of 'current' logicalIndex[%d] (ruleId) ?! ",
                    logicalIndex,ruleId);
            }
        }

        if(found == GT_FALSE)
        {
            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }
    }

    /* return the 'next' ruleId */
    *nextRuleIdPtr = foundRuleIdEntryPtr->ruleId;

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamNextRuleIdGet function
* @endinternal
*
* @brief   Function gets next existing rule ID.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] ruleId                   - for logical-index driven vTCAM - logical-index
*                                      for priority driven vTCAM - ruleId that used to refer existing rule
*                                      CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS used to start iteration of rules
*
* @param[out] nextRuleIdPtr            -      (pointer to)id of next rule
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_EMPTY                 - on 'START_RULE_ID' indication and no rules exists.
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NO_MORE               - all rule already passed by iteration
*/
GT_STATUS cpssDxChVirtualTcamNextRuleIdGet
(
    IN  GT_U32        vTcamMngId,
    IN  GT_U32        vTcamId,
    IN  GT_U32        ruleId,
    OUT GT_U32        *nextRuleIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamNextRuleIdGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, ruleId, nextRuleIdPtr));

    rc = internal_cpssDxChVirtualTcamNextRuleIdGet(vTcamMngId, vTcamId, ruleId, nextRuleIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, ruleId, nextRuleIdPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamAutoResizeEnable function
* @endinternal
*
* @brief   Set the auto resize enable in Priority mode Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] enable                   - for logical-index driven vTCAM - NOT APPLICABLE
*                                      for priority driven vTCAM without auto-resize - NOT APPLICABLE
*                                      for priority driven vTCAM with auto-resize -
*                                      FALSE - Auto-resize forced OFF
*                                      TRUE - Auto-resize restored to ON (default)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id or rule with rileId not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
static GT_STATUS internal_cpssDxChVirtualTcamAutoResizeEnable
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   GT_BOOL                            enable
)
{
    GT_STATUS                                       rc;
    GT_BOOL                                         doAutoResize;
    GT_U32                                          rulesAmountGranularity;
    GT_U32                                          saveGuaranteedNumOfRules;
    GT_U32                                          roundedUpGuaranteedNumOfRules;
    GT_U32                                          roundedUpUsedNumOfRules;
    GT_U32                                          downSizeValue;
    GT_U32                                          autoResizeRemoval;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC              *vTcamMngDBPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*    vtcamInfoPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* Check vTcam is priority mode with auto-resize */
    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod !=
            CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(vtcamInfoPtr->tcamInfo.autoResize == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Check if need to enable or disable */
    if(vtcamInfoPtr->autoResizeDisable != enable)
    {
        /* Already in the right state */
        return GT_OK;
    }

    /* Update auto-resize disabled state, perform actions for enable case */
    vtcamInfoPtr->autoResizeDisable = !enable;

    if (enable)
    {
        doAutoResize = GT_TRUE;

        rulesAmountGranularity =
            prvCpssDxChVirtualTcamDbSegmentTableVTcamRuleAmountGranularity(
                vtcamInfoPtr->tcamSegCfgPtr, vtcamInfoPtr->tcamInfo.ruleSize);
        roundedUpGuaranteedNumOfRules = vtcamInfoPtr->tcamInfo.guaranteedNumOfRules +
                                                        rulesAmountGranularity - 1;
        roundedUpGuaranteedNumOfRules -= (roundedUpGuaranteedNumOfRules %
                                                        rulesAmountGranularity);
        roundedUpUsedNumOfRules = vtcamInfoPtr->usedRulesAmount +
                                                        rulesAmountGranularity - 1;
        roundedUpUsedNumOfRules -= (roundedUpUsedNumOfRules %
                                                        rulesAmountGranularity);
        /* really can be == only */
        if (vtcamInfoPtr->rulesAmount <= roundedUpGuaranteedNumOfRules)
        {
            doAutoResize = GT_FALSE;
        }
        /* really can be == only */
        if (vtcamInfoPtr->rulesAmount <= roundedUpUsedNumOfRules)
        {
            doAutoResize = GT_FALSE;
        }
        /* Guaranteed and used amount of rules (rounded up to a row) must remain */
        if (roundedUpUsedNumOfRules > roundedUpGuaranteedNumOfRules)
        {
            downSizeValue = vtcamInfoPtr->rulesAmount - roundedUpUsedNumOfRules;
        }
        else
        {
            downSizeValue = vtcamInfoPtr->rulesAmount - roundedUpGuaranteedNumOfRules;
        }
        if (roundedUpUsedNumOfRules > roundedUpGuaranteedNumOfRules)
        {
            /* granularity applicable only when used rules more than guaranteed */
            autoResizeRemoval =
                prvCpssDxChVirtualTcamDbSegmentTableResizeGranularityGet(
                    vtcamInfoPtr->tcamSegCfgPtr, vtcamInfoPtr->tcamInfo.ruleSize);
            downSizeValue -= (downSizeValue % autoResizeRemoval);
        }

        if (doAutoResize != GT_FALSE)
        {
            /* decrease vTCAM size */
            saveGuaranteedNumOfRules = vtcamInfoPtr->tcamInfo.guaranteedNumOfRules;
            rc = vtcamDbSegmentTableVTcamDownSize__PriorityMode(
                vTcamMngId, vTcamId,
                downSizeValue);
            vtcamInfoPtr->tcamInfo.guaranteedNumOfRules = saveGuaranteedNumOfRules;
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
            }
        }

    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamAutoResizeEnable function
* @endinternal
*
* @brief   Set the auto resize enable in Priority mode Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] enable                   - for logical-index driven vTCAM - NOT APPLICABLE
*                                      for priority driven vTCAM without auto-resize - NOT APPLICABLE
*                                      for priority driven vTCAM with auto-resize -
*                                      FALSE - Auto-resize forced OFF
*                                      TRUE - Auto-resize restored to ON (default)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*/
GT_STATUS cpssDxChVirtualTcamAutoResizeEnable
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             vTcamId,
    IN   GT_BOOL                            enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamAutoResizeEnable);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamId, enable));

    rc = internal_cpssDxChVirtualTcamAutoResizeEnable(vTcamMngId, vTcamId, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamId, enable));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChVirtualTcamAutoResizeGranularitySet function
* @endinternal
*
* @brief   Set the auto resize granularity in Priority mode Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] granularity              - Granularity (in rules) to be applied at auto-resize events
*                                      on vTCAMs belonging to this vTCAM manager.
*                                      (Rounded up to a whole TCAM row)
*                                      (APPLICABLE RANGES: 1..240)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note To be called only while vTCAM manager has no devices attached
*
*/
static GT_STATUS internal_cpssDxChVirtualTcamAutoResizeGranularitySet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             granularity
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC              *vTcamMngDBPtr;

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngDBPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngDBPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* Check vTcam manager is not yet attached to a device */
    if(vTcamMngDBPtr->numOfDevices > 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* Update auto-resize granularity (to be rounded up to a whole row on usage) */
    if ((granularity < 1) ||
        (granularity > 240))
    {
        /* out of applicable range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    vTcamMngDBPtr->autoResizeGranularity = granularity;

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamAutoResizeGranularitySet function
* @endinternal
*
* @brief   Set the auto resize granularity in Priority mode Virtual TCAM.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] granularity              - Granularity (in rules) to be applied at auto-resize events
*                                      on vTCAMs belonging to this vTCAM manager.
*                                      (Rounded up to a whole TCAM row)
*                                      (APPLICABLE RANGES: 1..240)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_FOUND             - vTcam with given Id not found
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device in devListArr
*
* @note To be called only while vTCAM manager has no devices attached
*
*/
GT_STATUS cpssDxChVirtualTcamAutoResizeGranularitySet
(
    IN   GT_U32                             vTcamMngId,
    IN   GT_U32                             granularity
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamAutoResizeGranularitySet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, granularity));

    rc = internal_cpssDxChVirtualTcamAutoResizeGranularitySet(vTcamMngId, granularity);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, granularity));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal cpssDxChVirtualTcamDefrag function
* @endinternal
*
* @brief   Defragment TCAM manager memory.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssDxChVirtualTcamDefrag
(
    IN  GT_U32                              vTcamMngId
)
{
    vTcamMngId = vTcamMngId;

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}

/*
 * typedef: enum VTCAM_BASIC_INFO_STC
 *
 * Description:
 *      info needed for comparing vtcams to decide optimal insert to tcam when start
   from scratch
 *
 * Enumerations:
 *
 *     vTcamId     - vtcamId
 *     ruleSize    - ruleSize
 *     newOrAddedNumRules - new or added number of rules
 *     newVtcam    - new/existing vtcam indication
 *     newTcamInfo - new vtcam info
 */
typedef struct
{
    GT_U32                               vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT ruleSize;
    GT_U32                               newOrAddedNumRules;
    GT_BOOL                              newVtcam;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC      newTcamInfo;
} VTCAM_BASIC_INFO_STC;

/*******************************************************************************
* prvCpssDxChVirtualTcamDbAllocWithOnlySegmentTable
*
* DESCRIPTION:
*       allocate vtcam with segments tree
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       vTcamMngPtr  - (pointer to) the TCAM manager
*       vTcamId      - the vtcam ID to allocate for
*       tcamInfoPtr  - (pointer to) the vtcam info
*       allocSkeletonOnly   - indication to allocate only skeleton
* OUTPUTS:
*       vTcamMngPtr  - (pointer to) the TCAM manager , with the added vtcam
*                       allocation at index vTcamId.
*
* RETURNS :
*       pointer to allocated vtcam memory (on NULL)
*
* COMMENTS:
*
*******************************************************************************/
PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* prvCpssDxChVirtualTcamDbAllocWithOnlySegmentTable
(
    INOUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC      *vTcamMngPtr,
    IN GT_U32                                   vTcamId,
    IN CPSS_DXCH_VIRTUAL_TCAM_INFO_STC         *tcamInfoPtr,
    IN    GT_BOOL                               allocSkeletonOnly
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vtcamInfoPtr;

    vtcamInfoPtr =
        (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*)
        cpssOsMalloc(sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC));

    if (vtcamInfoPtr == NULL)
    {
        return NULL;
    }

    cpssOsMemSet(vtcamInfoPtr, 0, sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC));

    vtcamInfoPtr->tcamInfo = *tcamInfoPtr;
    if ((vTcamMngPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
        (vTcamMngPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
    {
        if (tcamInfoPtr->clientGroup == 0)
        {
            vtcamInfoPtr->tcamSegCfgPtr = &vTcamMngPtr->tcamSegCfg;
        }
        else
        {
            vtcamInfoPtr->tcamSegCfgPtr = &vTcamMngPtr->tcamTtiSegCfg;
        }
    }
    else if (vTcamMngPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
    {
        if (tcamInfoPtr->clientGroup == 0)
        {
            vtcamInfoPtr->tcamSegCfgPtr = &vTcamMngPtr->tcamSegCfg;
        }
        else if (tcamInfoPtr->clientGroup == 1)
        {
            vtcamInfoPtr->tcamSegCfgPtr = &vTcamMngPtr->tcamTtiSegCfg;
        }
        else /*if (tcamInfoPtr->clientGroup == 2)*/
        {
            vtcamInfoPtr->tcamSegCfgPtr = &vTcamMngPtr->pclTcam1SegCfg;
        }
    }
    else
    {
        vtcamInfoPtr->tcamSegCfgPtr = &vTcamMngPtr->tcamSegCfg;
    }
    if(allocSkeletonOnly == GT_FALSE)
    {
        rc = prvCpssAvlTreeCreate(
            (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
            vTcamMngPtr->segNodesPool, &vtcamInfoPtr->segmentsTree);
        if (rc != GT_OK)
        {
            cpssOsFree(vtcamInfoPtr);
            return NULL;
        }
    }

    /* bind it to proper place */
    vTcamMngPtr->vTcamCfgPtrArr[vTcamId] = vtcamInfoPtr;

    return vtcamInfoPtr;
}

/**
* @internal virtualTcamMemoryAvailabilityCheck_fromCurrentState function
* @endinternal
*
* @brief   Check availability of TCAM manager memory for list of new and existing
*         Virtual TCAMs ... if we will try to add it NOW (from current state)
*         NOTE: existing vtcams comes from origVTcamMngPtr
*         new + updated vtcams comes from the vTcamCheckInfoArr[]
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] newVTcamMngPtr           - ((pointer to) to new TCAM manager.
* @param[in] vTcamAmount              - amount of Virtual TCAMs in the array.
* @param[in] vTcamCheckInfoArr[]      - array of infos for Virtual TCAMs.
*                                      OUTPUTS:
*
* @retval GT_OK                    - For successful operation.
* @retval GT_FULL                  - the TCAM is full and can not support all those vtcams
* @retval others                   - other errors
*/
static GT_STATUS virtualTcamMemoryAvailabilityCheck_fromCurrentState
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC      *newVTcamMngPtr,
    IN   GT_U32                                 vTcamAmount,
    IN   CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC  vTcamCheckInfoArr[]
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam         */
    CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC *currCheckInfoPtr;
    GT_U32  vTcamId = 0;
    GT_U32  ii, oldRulesAmount;
    GT_BOOL prvCpssDxChVirtualTcamDebugTraceEnable;

    prvCpssDxChVirtualTcamDebugTraceEnable =
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDebugTraceEnable);

    /* add updated/new vtcams to the tree */
    CPSS_LOG_INFORMATION_MAC("treat elements from the vTcamCheckInfoArr[] to the tcam \n");
    currCheckInfoPtr = &vTcamCheckInfoArr[0];
    for(ii = 0 ; ii < vTcamAmount; ii++ , currCheckInfoPtr++)
    {
        vTcamId = currCheckInfoPtr->vTcamId;

        /* from 'new' */
        vTcamCfgPtr = newVTcamMngPtr->vTcamCfgPtrArr[vTcamId];

        if(vTcamCfgPtr)
        {
            oldRulesAmount = vTcamCfgPtr->rulesAmount;

            if(currCheckInfoPtr->vTcamInfo.guaranteedNumOfRules < vTcamCfgPtr->rulesAmount)
            {/* decreased vtcam */
                #ifdef CPSS_LOG_ENABLE
                {
                    GT_U32  neededResizeValue;/*needed resize value */

                    neededResizeValue = vTcamCfgPtr->rulesAmount - currCheckInfoPtr->vTcamInfo.guaranteedNumOfRules;

                    CPSS_LOG_INFORMATION_MAC("request to decrease existing vTcamId[%d] : client[%d] hitNum[%d] ruleSize[%d B] removing memory of rulesAmount[%d] \n",
                        vTcamId ,
                        currCheckInfoPtr->vTcamInfo.clientGroup ,
                        currCheckInfoPtr->vTcamInfo.hitNumber ,
                        10*(currCheckInfoPtr->vTcamInfo.ruleSize+1) ,
                        neededResizeValue);
                }
                #endif /*CPSS_LOG_ENABLE*/
                /* we assumes that the application will do ALL necessary action to
                   decrease this vtcam */

                /* the function will update vTcamInfoPtr->rulesAmount */
                rc = prvCpssDxChVirtualTcamDbSegmentTableDetachSpace(vTcamCfgPtr->tcamSegCfgPtr/* from 'new' */,
                    vTcamCfgPtr, GT_TRUE,/*from the end*/
                    currCheckInfoPtr->vTcamInfo.guaranteedNumOfRules/*numOfRulesToKeep*/, NULL);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
                }

                if (prvCpssDxChVirtualTcamDebugTraceEnable != GT_FALSE)
                {
                    cpssOsPrintf(
                        "After decrease vtcam[%d] old size[%d] new size[%d] \n",
                        vTcamId, oldRulesAmount, vTcamCfgPtr->rulesAmount);
                    cpssOsPrintf(
                        "Rule size in min rules[%d] \n",
                            prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
                                vTcamCfgPtr->tcamSegCfgPtr, vTcamCfgPtr->tcamInfo.ruleSize));
                    prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
                        vTcamCfgPtr->tcamSegCfgPtr, GT_TRUE /*perLookupOnly*/);
                }

                CPSS_LOG_INFORMATION_MAC("Actual decrease vtcam[%d] to new size[%d] \n",
                    vTcamId,
                    vTcamCfgPtr->rulesAmount);

                if(vTcamCfgPtr->rulesAmount < vTcamCfgPtr->tcamInfo.guaranteedNumOfRules)
                {
                    vTcamCfgPtr->tcamInfo.guaranteedNumOfRules = vTcamCfgPtr->rulesAmount;
                }
            }
            else /* keep 'as is' vtcam */
            if(currCheckInfoPtr->vTcamInfo.guaranteedNumOfRules == vTcamCfgPtr->rulesAmount)
            {
                CPSS_LOG_INFORMATION_MAC("request to keep size 'as is' in existing vTcamId[%d] : client[%d] hitNum[%d] ruleSize[%d B] keep rulesAmount[%d] \n",
                    vTcamId ,
                    currCheckInfoPtr->vTcamInfo.clientGroup ,
                    currCheckInfoPtr->vTcamInfo.hitNumber ,
                    10*(currCheckInfoPtr->vTcamInfo.ruleSize+1) ,
                    vTcamCfgPtr->rulesAmount);
            }
            else/* increased vtcam */
            {
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC tmp_vTcamInfo;
                PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC *alloc_vTcamInfoPtr = &tmp_vTcamInfo;
                GT_U32  neededResizeValue;/*needed resize value */

                neededResizeValue = currCheckInfoPtr->vTcamInfo.guaranteedNumOfRules -
                                        vTcamCfgPtr->rulesAmount;

                CPSS_LOG_INFORMATION_MAC("request to increase resize existing vTcamId[%d] : client[%d] hitNum[%d] ruleSize[%d B] ADDING memory for rulesAmount[%d] \n",
                    vTcamId ,
                    currCheckInfoPtr->vTcamInfo.clientGroup ,
                    currCheckInfoPtr->vTcamInfo.hitNumber ,
                    10*(currCheckInfoPtr->vTcamInfo.ruleSize+1) ,
                    neededResizeValue);

                /* allocate to temp info */
                cpssOsMemSet(&tmp_vTcamInfo,0,sizeof(tmp_vTcamInfo));

                rc =  vtcamDbSegmentTableVTcamAllocOrResize( newVTcamMngPtr /* from 'new' */,
                    vTcamId,
                    neededResizeValue,
                    &tmp_vTcamInfo);
                if(rc != GT_OK)
                {
                    CPSS_LOG_INFORMATION_MAC("failed on vtcamDbSegmentTableVTcamAllocOrResize : on vTcamId[%d] \n",
                        vTcamId);
                    goto cleanExit_lbl;
                }

                if (tmp_vTcamInfo.rulesAmount != 0)
                {
                    /* free this memory as it MUST not be used */
                    cpssOsFree(alloc_vTcamInfoPtr->rulePhysicalIndexArr);
                    alloc_vTcamInfoPtr->rulePhysicalIndexArr = NULL;

                    /* function prvCpssDxChVirtualTcamDbSegmentTableVTcamCombine will update
                       vTcamInfoPtr->rulesAmount  */

                    /* bind the new segmentsTree from alloc_vTcamInfoPtr into vTcamInfoPtr */
                    /* and align the info in the tree of vTcamInfoPtr->segmentsTree */
                    rc = prvCpssDxChVirtualTcamDbSegmentTableVTcamCombine(
                        vTcamCfgPtr,alloc_vTcamInfoPtr);

                    prvCpssAvlTreeDelete(
                        alloc_vTcamInfoPtr->segmentsTree,
                        (GT_VOIDFUNCPTR)NULL,
                        (GT_VOID*)NULL);

                    if(rc != GT_OK)
                    {
                        CPSS_LOG_INFORMATION_MAC("failed on prvCpssDxChVirtualTcamDbSegmentTableVTcamCombine : on vTcamId[%d] \n",
                            vTcamId);
                        goto cleanExit_lbl;
                    }

                    if (prvCpssDxChVirtualTcamDebugTraceEnable != GT_FALSE)
                    {
                        cpssOsPrintf(
                            "After increase vtcam[%d] old size[%d] new size[%d] \n",
                            vTcamId, oldRulesAmount, vTcamCfgPtr->rulesAmount);
                        cpssOsPrintf(
                            "Rule size[%d] \n",
                                prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
                                    vTcamCfgPtr->tcamSegCfgPtr, vTcamCfgPtr->tcamInfo.ruleSize));
                        prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
                            &newVTcamMngPtr->tcamSegCfg, GT_TRUE /*perLookupOnly*/);
                        prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
                            &newVTcamMngPtr->tcamTtiSegCfg, GT_TRUE /*perLookupOnly*/);
                        prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
                            &newVTcamMngPtr->pclTcam1SegCfg, GT_TRUE /*perLookupOnly*/);
                    }
                }

                CPSS_LOG_INFORMATION_MAC("Actual increase vtcam[%d] to new size[%d] \n",
                    vTcamId,
                    vTcamCfgPtr->rulesAmount);
            }
        }
        else /* new vtcam */
        {
            CPSS_LOG_INFORMATION_MAC( "add new vTcamId[%d] : client[%d] hitNum[%d] ruleSize[%d B] rulesAmount[%d] \n",
                vTcamId ,
                currCheckInfoPtr->vTcamInfo.clientGroup ,
                currCheckInfoPtr->vTcamInfo.hitNumber ,
                10*(currCheckInfoPtr->vTcamInfo.ruleSize+1) ,
                currCheckInfoPtr->vTcamInfo.guaranteedNumOfRules);

            /* allocate vtcam with segments tree support .. in the new tcam manager */
            vTcamCfgPtr =
                prvCpssDxChVirtualTcamDbAllocWithOnlySegmentTable(newVTcamMngPtr,
                    vTcamId,
                    &currCheckInfoPtr->vTcamInfo,
                    GT_TRUE/* only skeleton*/);
            if (vTcamCfgPtr == NULL)
            {
                CPSS_LOG_INFORMATION_MAC("fail to allocate memory for vTCAM [%d] \n",
                    ii);
                rc = GT_OUT_OF_CPU_MEM;
                goto cleanExit_lbl;
            }

            /* Allocate memory for Virtual TCAM in Segment Table for TCAM. */
            rc = vtcamDbSegmentTableVTcamAllocOrResize(
                newVTcamMngPtr /* from 'new' */, vTcamId,
                0/* no realloc*/,NULL/*NA*/);
            if(rc != GT_OK)
            {
                CPSS_LOG_INFORMATION_MAC("failed on vtcamDbSegmentTableVTcamAllocOrResize : on vTcamId[%d] \n",
                    vTcamId);
                goto cleanExit_lbl;
            }
            if (prvCpssDxChVirtualTcamDebugTraceEnable != GT_FALSE)
            {
                cpssOsPrintf(
                    "After create vtcam[%d] size[%d] \n",
                    vTcamId, vTcamCfgPtr->rulesAmount);
                cpssOsPrintf(
                    "Rule size[%d] \n",
                        prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
                            vTcamCfgPtr->tcamSegCfgPtr, vTcamCfgPtr->tcamInfo.ruleSize));
                prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
                    &newVTcamMngPtr->tcamSegCfg, GT_TRUE /*perLookupOnly*/);
                prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
                    &newVTcamMngPtr->tcamTtiSegCfg, GT_TRUE /*perLookupOnly*/);
                prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
                    &newVTcamMngPtr->pclTcam1SegCfg, GT_TRUE /*perLookupOnly*/);
            }

        }
    }

    rc = GT_OK;

    CPSS_LOG_INFORMATION_MAC("done creating/updating the vtcams in vTcamCheckInfoArr[] successfully \n");
cleanExit_lbl:
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("failed to creating/updating all vtcams , stopped at vTcamCheckInfoArr[%d] vTcamId[%d] \n",
            ii,vTcamId);
    }

    return rc;
}

/**
* @internal deleteTcamManagerWithOnlySegmentTable function
* @endinternal
*
* @brief   delete tcam manager that was created with createTcamManagerWithOnlySegmentTable(...)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS deleteTcamManagerWithOnlySegmentTable
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC     *pVTcamMngPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii;

    rc = prvCpssDxChVirtualTcamDbSegmentTableDeleteWithVtcamsSegments(pVTcamMngPtr);

    /************************************/
    /* do memory allocation clean up before returning the ERROR code to the caller */
    /************************************/
    if(pVTcamMngPtr->vTcamCfgPtrArr)
    {
        for(ii = 0 ; ii < pVTcamMngPtr->vTcamCfgPtrArrSize ; ii++)
        {
            if(pVTcamMngPtr->vTcamCfgPtrArr[ii])
            {
                if(pVTcamMngPtr->vTcamCfgPtrArr[ii]->rulePhysicalIndexArr)
                {
                    cpssOsFree(pVTcamMngPtr->vTcamCfgPtrArr[ii]->rulePhysicalIndexArr);
                }

                cpssOsFree(pVTcamMngPtr->vTcamCfgPtrArr[ii]);
            }
        }

        cpssOsFree(pVTcamMngPtr->vTcamCfgPtrArr);
    }
    cpssOsFree(pVTcamMngPtr);

    return rc;
}

/* dummy HW access functions for Tcam Manager With Only Segment Table */

GT_STATUS dummyBlockToLookupMapFunction
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      baseRow,
    IN  GT_U32      baseColumn,
    IN  GT_U32      lookupId
)
{
    cookiePtr    = cookiePtr;
    baseRow      = baseRow;
    baseColumn   = baseColumn;
    lookupId     = lookupId;
    return GT_OK;
}

GT_STATUS dummyBlockFromLookupUnmapFunction
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      baseRow,
    IN  GT_U32      baseColumn
)
{
    cookiePtr    = cookiePtr;
    baseRow      = baseRow;
    baseColumn   = baseColumn;
    return GT_OK;
}

GT_STATUS dummyRuleInvalidateFunction
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      rulePhysicalIndex,
    IN  GT_U32      ruleSize
)
{
    cookiePtr         = cookiePtr;
    rulePhysicalIndex = rulePhysicalIndex;
    ruleSize          = ruleSize;
    return GT_OK;
}

GT_STATUS dummyRuleMoveFunction
(
    IN  GT_VOID     *cookiePtr,
    IN  GT_U32      srcRulePhysicalIndex,
    IN  GT_U32      dstRulePhysicalIndex,
    IN  GT_U32      ruleSize
)
{
    cookiePtr              = cookiePtr;
    srcRulePhysicalIndex   = srcRulePhysicalIndex;
    dstRulePhysicalIndex   = dstRulePhysicalIndex;
    ruleSize               = ruleSize;
    return GT_OK;
}

/**
* @internal createTcamManagerWithOnlySegmentTable function
* @endinternal
*
* @brief   create new TCAM manager with support only for 'segments table' and it is
*         based on the same parameters of the 'orig' TCAM manager.
*         with option to also copy the full segments layout of the orig TCAM manager.
*         on such option it copies : TCAM : free segments (trees , buffers)
*         per vtcam : used segments (tree) and rulesAmount
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] origVTcamMngPtr          - (pointer to) the original TCAM manager
* @param[in] copyOrigLayout           - indication if to copy the layout from the orig TCAM or not.
*                                      GT_TRUE - new layout will be copy the orig layout from orig TCAM.
*                                      GT_FALSE - new layout is empty.
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - on CPU memory allocation fail
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - on error
* @retval GT_NO_RESOURCE           - not enough free place in TCAM
*/
static GT_STATUS createTcamManagerWithOnlySegmentTable
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC     *origVTcamMngPtr,
    IN GT_BOOL                                 copyOrigLayout,
    OUT PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*   *newVTcamMngPtrPtr
)
{
    GT_STATUS   rc = GT_OK;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC *pVTcamMngPtr;
    GT_BOOL  fillSegFreeTrees;
    GT_BOOL  allocSkeletonOnly;

    *newVTcamMngPtrPtr = NULL;

    CPSS_LOG_INFORMATION_MAC("allocate new TCAM manager \n");
    pVTcamMngPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*)
        cpssOsMalloc(sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC));
    if(pVTcamMngPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(pVTcamMngPtr, 0,sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC));

    CPSS_LOG_INFORMATION_MAC("allocate array of vTcams for new TCAM manager \n");
    pVTcamMngPtr->vTcamCfgPtrArrSize = CPSS_DXCH_VIRTUAL_TCAM_VTCAM_MAX_ID_CNS + 1;
    pVTcamMngPtr->vTcamCfgPtrArr =
        (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR*)cpssOsMalloc(
            sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR)
            * (CPSS_DXCH_VIRTUAL_TCAM_VTCAM_MAX_ID_CNS + 1));
    if (pVTcamMngPtr->vTcamCfgPtrArr == NULL)
    {
        CPSS_LOG_INFORMATION_MAC("failed to alloc vTcamCfgPtrArr \n");
        rc = GT_OUT_OF_CPU_MEM;
        goto cleanExit_lbl;
    }
    cpssOsMemSet(
        pVTcamMngPtr->vTcamCfgPtrArr, 0,
        (sizeof(PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR)
            * (CPSS_DXCH_VIRTUAL_TCAM_VTCAM_MAX_ID_CNS + 1)));

    CPSS_LOG_INFORMATION_MAC("fill new TCAM manager base on size parameters from orig TCAM manager \n");

    fillSegFreeTrees = (copyOrigLayout == GT_TRUE) ? GT_FALSE : GT_TRUE;

    rc = prvCpssDxChVirtualTcamDbSegmentTableCreate(
        pVTcamMngPtr,
        &origVTcamMngPtr->tcamSegCfg,
        &origVTcamMngPtr->tcamTtiSegCfg,
        &origVTcamMngPtr->pclTcam1SegCfg,
        fillSegFreeTrees);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("failed on prvCpssDxChVirtualTcamDbSegmentTableCreate \n");
        goto cleanExit_lbl;
    }

    /* rebind HW-access function to dummy */
    pVTcamMngPtr->tcamSegCfg.lookupMapFuncPtr          = dummyBlockToLookupMapFunction;
    pVTcamMngPtr->tcamSegCfg.lookupUnmapFuncPtr        = dummyBlockFromLookupUnmapFunction;
    pVTcamMngPtr->tcamSegCfg.ruleInvalidateFuncPtr     = dummyRuleInvalidateFunction;
    pVTcamMngPtr->tcamSegCfg.ruleMoveFuncPtr           = dummyRuleMoveFunction;
    pVTcamMngPtr->tcamTtiSegCfg.lookupMapFuncPtr       = dummyBlockToLookupMapFunction;
    pVTcamMngPtr->tcamTtiSegCfg.lookupUnmapFuncPtr     = dummyBlockFromLookupUnmapFunction;
    pVTcamMngPtr->tcamTtiSegCfg.ruleInvalidateFuncPtr  = dummyRuleInvalidateFunction;
    pVTcamMngPtr->tcamTtiSegCfg.ruleMoveFuncPtr        = dummyRuleMoveFunction;
    pVTcamMngPtr->pclTcam1SegCfg.lookupMapFuncPtr      = dummyBlockToLookupMapFunction;
    pVTcamMngPtr->pclTcam1SegCfg.lookupUnmapFuncPtr    = dummyBlockFromLookupUnmapFunction;
    pVTcamMngPtr->pclTcam1SegCfg.ruleInvalidateFuncPtr = dummyRuleInvalidateFunction;
    pVTcamMngPtr->pclTcam1SegCfg.ruleMoveFuncPtr       = dummyRuleMoveFunction;
    pVTcamMngPtr->devFamily                            = origVTcamMngPtr->devFamily;

    if(copyOrigLayout == GT_TRUE)
    {
        CPSS_LOG_INFORMATION_MAC("copy to the new TCAM manager the used segments of the orig TCAM manager \n");
    }

    allocSkeletonOnly = (copyOrigLayout == GT_TRUE) ? GT_FALSE : GT_TRUE;

    rc = prvCpssDxChVirtualTcamDbSegmentTableCopy(origVTcamMngPtr,pVTcamMngPtr,allocSkeletonOnly);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("failed on prvCpssDxChVirtualTcamDbSegmentTableCopy \n");
        goto cleanExit_lbl;
    }

    if(copyOrigLayout == GT_TRUE)
    {
        CPSS_LOG_INFORMATION_MAC("ended copy to the new TCAM manager the vTCAMs used segments \n");
    }

cleanExit_lbl:
    if(rc != GT_OK)
    {
        /************************************/
        /* do memory allocation clean up before returning the ERROR code to the caller */
        /************************************/
        deleteTcamManagerWithOnlySegmentTable(pVTcamMngPtr);

        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *newVTcamMngPtrPtr = pVTcamMngPtr;

    return GT_OK;
}

/* macro to check (and return 'bad param' if check fails) of field in tcamInfo info */
#define CHECK_VTCAM_INFO_SAME_VALUE_MAC(existVtcamInfoPtr,updatedVtcamInfoPtr,field)    \
    if(updatedVtcamInfoPtr->field != existVtcamInfoPtr->field)    \
    {                                                             \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,               \
            "the vTcamId[%d] in vTcamCheckInfoArr index [%d] : hold field [%s] value[%d] mismatch from exists value[%d]", \
            vTcamId,ii,                                           \
            #field,                                               \
            updatedVtcamInfoPtr->field,                           \
            existVtcamInfoPtr->field                              \
            );                                                    \
    }

/**
* @internal memoryAvailabilityCheckParams function
* @endinternal
*
* @brief   Check parameters for cpssDxChVirtualTcamMemoryAvailabilityCheck(...)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamAmount              - amount of Virtual TCAMs in the array.
* @param[in] vTcamCheckInfoArr[]      - array of infos for Virtual TCAMs.
*
* @param[out] tcamAvailabilityPtr      - (pointer to)TCAM Availability (see enum)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if pointer or array-pointer is NULL.
* @retval GT_NOT_FOUND             - TCAM manager with given Id not found
*/
static GT_STATUS memoryAvailabilityCheckParams
(
    IN   GT_U32                                 vTcamMngId,
    IN   GT_U32                                 vTcamAmount,
    IN   CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC  vTcamCheckInfoArr[],
    OUT  CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT *tcamAvailabilityPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC* vTcamMngPtr; /* pointer to vTcam Manager */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam         */
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC *existVtcamInfoPtr;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC *updatedVtcamInfoPtr;
    CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC *currCheckInfoPtr;
    GT_U32 vTcamId;
    GT_U32  ii;

    CPSS_NULL_PTR_CHECK_MAC(vTcamCheckInfoArr);
    CPSS_NULL_PTR_CHECK_MAC(tcamAvailabilityPtr);

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "vTcamMngId out of range\n");
    }

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "TCAM manager id [%d] was not found",
            vTcamMngId);
    }

    if(vTcamAmount == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "no meaning calling function with amount 'ZERO'");
    }

    currCheckInfoPtr = &vTcamCheckInfoArr[0];
    for(ii = 0 ; ii < vTcamAmount; ii++ , currCheckInfoPtr++)
    {
        vTcamId = currCheckInfoPtr->vTcamId;

        /*******************/
        /* validity checks */
        /*******************/
        if(vTcamId >= vTcamMngPtr->vTcamCfgPtrArrSize)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "the vTcamId[%d] in vTcamCheckInfoArr index [%d] is over max size [%d]",
                vTcamId,ii,vTcamMngPtr->vTcamCfgPtrArrSize);
        }

        if (vTcamMngPtr->vTcamCfgPtrArr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "vTcamMngPtr->vTcamCfgPtrArr is NULL\n");
        }

        vTcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];

        if(currCheckInfoPtr->isExist != GT_FALSE && vTcamCfgPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "the vTcamId[%d] in vTcamCheckInfoArr index [%d] expected to exists but is not",
                vTcamId,ii);
        }
        else
        if(currCheckInfoPtr->isExist == GT_FALSE && vTcamCfgPtr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "the vTcamId[%d] in vTcamCheckInfoArr index [%d] exist although not expected to be",
                vTcamId,ii);
        }

        if(currCheckInfoPtr->isExist == GT_TRUE && vTcamCfgPtr)
        {
            existVtcamInfoPtr   = &vTcamCfgPtr->tcamInfo;
            updatedVtcamInfoPtr = &currCheckInfoPtr->vTcamInfo;

            /* the updated info must be with same values as exists
               except for 'guaranteedNumOfRules' that can be different */
            CHECK_VTCAM_INFO_SAME_VALUE_MAC(existVtcamInfoPtr,updatedVtcamInfoPtr,clientGroup);
            CHECK_VTCAM_INFO_SAME_VALUE_MAC(existVtcamInfoPtr,updatedVtcamInfoPtr,hitNumber);
            CHECK_VTCAM_INFO_SAME_VALUE_MAC(existVtcamInfoPtr,updatedVtcamInfoPtr,ruleSize);
            CHECK_VTCAM_INFO_SAME_VALUE_MAC(existVtcamInfoPtr,updatedVtcamInfoPtr,autoResize);
            CHECK_VTCAM_INFO_SAME_VALUE_MAC(existVtcamInfoPtr,updatedVtcamInfoPtr,ruleAdditionMethod);

        }

        if(currCheckInfoPtr->isExist == GT_FALSE && (vTcamCfgPtr == NULL))
        {
            if ((vTcamMngPtr->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E) ||
                (vTcamMngPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E) ||
                (vTcamMngPtr->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
            {
                /* Extra check for parameters in xCat3 */
                updatedVtcamInfoPtr = &currCheckInfoPtr->vTcamInfo;

                if (updatedVtcamInfoPtr->clientGroup > 1) /* 0 - pcl, 1 - tti, 2 - pcl_tcam1(AC5) */
                {
                    if ((updatedVtcamInfoPtr->clientGroup > 2) ||
                        (vTcamMngPtr->devFamily != CPSS_PP_FAMILY_DXCH_AC5_E))
                    {
                        /* Only groups #0/1 (PCL/TTI) are allowed on this device family */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                    }
                }

                if (updatedVtcamInfoPtr->hitNumber != 0)
                {
                    /* Parallel lookups not allowed on this device family */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                switch (updatedVtcamInfoPtr->ruleSize)
                {
                    case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_30_B_E:
                        /* OK */
                        break;
                    case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_60_B_E:
                    case CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_80_B_E:
                        if ((updatedVtcamInfoPtr->clientGroup != 0)
                            && (updatedVtcamInfoPtr->clientGroup != 2))
                        {
                            /* Invalid size on this device family */
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                        }
                        /* OK */
                        break;
                    default:
                        /* Invalid size on this device family */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                        break;
                }
            }

        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChVirtualTcamMemoryAvailabilityCheck function
* @endinternal
*
* @brief   Check availability of TCAM manager memory for list of new and existing Virtual TCAMs.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamAmount              - amount of Virtual TCAMs in the array.
* @param[in] vTcamCheckInfoArr[]      - array of infos for Virtual TCAMs.
*
* @param[out] tcamAvailabilityPtr      - (pointer to)TCAM Availability (see enum)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if pointer or array-pointer is NULL.
* @retval GT_NOT_FOUND             - TCAM manager with given Id not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamMemoryAvailabilityCheck
(
    IN   GT_U32                                 vTcamMngId,
    IN   GT_U32                                 vTcamAmount,
    IN   CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC  vTcamCheckInfoArr[],
    OUT  CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT *tcamAvailabilityPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC* feasibilityCheck_vTcamMngPtr; /* pointer to temporary
                    vTcam Manager that is used for feasibility checks*/
    IN GT_BOOL                                 copyOrigLayout;/* indication if to copy
                the layout from the orig TCAM or not.
                GT_TRUE - new layout will be copy the orig layout from orig TCAM.
                GT_FALSE - new layout is empty.*/
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC* vTcamMngPtr; /* pointer to vTcam Manager */

    /* check parameters */
    rc = memoryAvailabilityCheckParams(vTcamMngId,vTcamAmount,vTcamCheckInfoArr,tcamAvailabilityPtr);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Failed on parameters check");
    }

    *tcamAvailabilityPtr = CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_NOT_AVAILABLE_E;

    vTcamMngPtr = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]);
    if(vTcamMngPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "TCAM manager id [%d] was not found",
            vTcamMngId);
    }

    /* check if can be supported as 'from current state'*/
    copyOrigLayout = GT_TRUE;
    /* get vTcam Manager that is used for feasibility checks */
    rc = createTcamManagerWithOnlySegmentTable(vTcamMngPtr,
        copyOrigLayout,
        &feasibilityCheck_vTcamMngPtr);
    if(rc != GT_OK)
    {
        /* all allocations free by the function itself */
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    CPSS_LOG_INFORMATION_MAC("check ability to insert from current state \n");
    /* check ability to insert from current state */
    rc = virtualTcamMemoryAvailabilityCheck_fromCurrentState(
        feasibilityCheck_vTcamMngPtr,
        vTcamAmount,vTcamCheckInfoArr);
    if (rc == GT_OK)
    {
        *tcamAvailabilityPtr = CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_AVAILABLE_E;
    }
    else if (rc == GT_FULL)
    {
        *tcamAvailabilityPtr = CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_NOT_AVAILABLE_E;
    }

    /* release resources */
    deleteTcamManagerWithOnlySegmentTable(feasibilityCheck_vTcamMngPtr);

    if ((rc != GT_OK) && (rc != GT_FULL))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamMemoryAvailabilityCheck function
* @endinternal
*
* @brief   Check availability of TCAM manager memory for list of new and existing Virtual TCAMs.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamAmount              - amount of Virtual TCAMs in the array.
* @param[in] vTcamCheckInfoArr[]      - array of infos for Virtual TCAMs.
*
* @param[out] tcamAvailabilityPtr      - (pointer to)TCAM Availability (see enum)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - if pointer or array-pointer is NULL.
* @retval GT_NOT_FOUND             - TCAM manager with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamMemoryAvailabilityCheck
(
    IN   GT_U32                                 vTcamMngId,
    IN   GT_U32                                 vTcamAmount,
    IN   CPSS_DXCH_VIRTUAL_TCAM_CHECK_INFO_STC  vTcamCheckInfoArr[],
    OUT  CPSS_DXCH_VIRTUAL_TCAM_AVAILABILTY_ENT *tcamAvailabilityPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamMemoryAvailabilityCheck);

   CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, vTcamMngId, vTcamAmount, vTcamCheckInfoArr, tcamAvailabilityPtr));

    rc = internal_cpssDxChVirtualTcamMemoryAvailabilityCheck(vTcamMngId, vTcamAmount, vTcamCheckInfoArr, tcamAvailabilityPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, vTcamMngId, vTcamAmount, vTcamCheckInfoArr, tcamAvailabilityPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssDxChVirtualTcamLibraryShutdown function
* @endinternal
*
* @brief   Shutdown Virtual TCAM Library.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on TCAM manager not found
*/
GT_STATUS prvCpssDxChVirtualTcamLibraryShutdown
(
    GT_VOID
)
{
    GT_STATUS                           rc=GT_OK;
    GT_STATUS                           rc1;
    GT_U32                              vTcamMngId;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMng;
    GT_U8                               vTcamDev;

    if (cpssSharedGlobalVarsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (vTcamMngId = 0; (vTcamMngId < CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS); vTcamMngId++)
    {
        while ((vTcamMng = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId])) != NULL)
        {
            rc1 = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMng, &vTcamDev);
            if (rc1 == GT_NO_MORE)
            {
                rc1 = prvCpssDxChVirtualTcamManagerDelete(vTcamMngId);
                if (rc1 == GT_OK)
                {
                    /* prvCpssDxChVirtualTcamManagerDelete now does not delete */
                    /* up-level structure pointed by vTcamMngDB[vTcamMngId]    */
                    if ((vTcamMng = VTCAM_GLOVAR(vTcamMngDB[vTcamMngId])) != NULL)
                    {
                        cpssOsFree(vTcamMng);
                        VTCAM_GLOVAR(vTcamMngDB[vTcamMngId]) = NULL;
                    }
                }
                else
                {
                    rc = rc1;
                }
                break;
            }
            else if (rc1 != GT_OK)
            {
                rc = rc1;
                break;
            }
            /* Remove device from vTcam manager - it will be removed with it's last device */
            rc1 = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, &vTcamDev, 1);
            if (rc1 != GT_OK)
            {
                rc = rc1;
                break;
            }
        }
    }
    return rc;
}

/*******************************************************************************
* prvCpssDxChVirtualTcamDbVTcamIsUnmovable
*
* DESCRIPTION:
*       Checks whether VTCAM is unmovable.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       vTcamMngDBPtr        - (pointer to) VTCAM Manager DB structure
*       vTcamId              - unique Id of  Virtual TCAM
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_TRUE for unmovable VTCAM, GT_FALSE for all other cases including wrong parameters
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_BOOL prvCpssDxChVirtualTcamDbVTcamIsUnmovable
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC  *vTcamMngDBPtr,
    IN  GT_U32                              vTcamId
)
{
    if (vTcamMngDBPtr == NULL)
    {
        return GT_FALSE;
    }

    if (vTcamMngDBPtr->vTcamCfgPtrArr == NULL)
    {
        return GT_FALSE;
    }

    if(vTcamId >= vTcamMngDBPtr->vTcamCfgPtrArrSize)
    {
        /* vTcamId out of range */
        return GT_FALSE;
    }

    return vTcamMngDBPtr->vTcamCfgPtrArr[vTcamId]->tcamInfo.tcamSpaceUnmovable;
}

/**
* @internal internal_cpssDxChVirtualTcamSpaceLayoutGet function
* @endinternal
*
* @brief   Get vTCAM Space Layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] maxNumOfSegments         - maximal number of serments in output array
*                                       0 means that only numOfSegments required
*
* @param[out] numOfSegmentsPtr        - (pointer to)actual number of serments
* @param[out] segmentArrayPtr         - (pointer to)array of Space segments
*                                       NULL can be specified only when maxNumOfSegments == 0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
static GT_STATUS internal_cpssDxChVirtualTcamSpaceLayoutGet
(
    IN   GT_U32                                     vTcamMngId,
    IN   GT_U32                                     vTcamId,
    IN   GT_U32                                     maxNumOfSegments,
    OUT  GT_U32                                     *numOfSegmentsPtr,
    OUT  CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC   segmentArrayPtr[]
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC* vTcamInfoPtr; /* vTCAM info               */
    GT_U32                                   tcamWidth;        /* tcam Width               */
    GT_U32                                   blockWidth;       /* Block Width              */
    GT_U32                                   horzBlockWidth;   /* horz Block Width         */
    PRV_CPSS_AVL_TREE_SEEK_ENT               seekMode;         /* seek Mode                */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC lastSeg;   /* last segment            */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segPtr;   /* memory segment           */
    GT_U32                                   spaceSegmentsNum; /* amount of space segments */
    GT_BOOL                                  seeked;           /* node seeked              */
    PRV_CPSS_AVL_TREE_PATH                   path;             /* tree iterator            */
    GT_VOID_PTR                              dbEntryPtr;       /* pointer to entry in DB   */
    CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC sumSpaceSegment;  /* summary space Segment    */
    GT_U32                                   maxColumnsBitmap; /* all columns that can be covered by the rule of the given size */
    GT_U32                                   startColumnsBitmap; /* all columns that rule of the given size can start from */
    GT_U32                                   ruleColumnsNum;      /* amount of columns used by rule of the given size can start from */
    /* same row segments merging data */
    GT_U32                                   numOfRowsArr[2];  /* horz merged segments height  */
    GT_U32                                   columnsBmpArr[2]; /* horz merged segments columns */
    GT_U32                                   horzSegmentsNum;  /* num of horz merged segments  */
    GT_U32                                   horzSegmentsRowBase; /* horz merged segs row base */
    GT_U32                                   horzSegmentsRowsNum; /* horz merged segs rows num */
    GT_U32                                   horzSegmentsColMap;  /* horz merged segs col map  */
    GT_U32                                   i;                /* loop index                 */
    GT_U32                                   j;                /* loop index                 */
    GT_BOOL                                  lastLoop;         /* last Loop                  */

    CPSS_NULL_PTR_CHECK_MAC(numOfSegmentsPtr);
    if (maxNumOfSegments != 0)
    {
        CPSS_NULL_PTR_CHECK_MAC(segmentArrayPtr);
    }

    /* get vtcam DB */
    vTcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId, vTcamId);
    if(vTcamInfoPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_NOT_FOUND, "vTcamMngId[%d] : vTcamId[%d] not exists",
            vTcamMngId, vTcamId);
    }

    tcamWidth = vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount;
    blockWidth = vTcamInfoPtr->tcamSegCfgPtr->lookupColumnAlignment;
    horzBlockWidth = (tcamWidth / blockWidth);

    if (horzBlockWidth > (sizeof(numOfRowsArr) / sizeof(numOfRowsArr[0])))
    {
        /* All TCAMs supported now have 2 blocks per row                */
        /* This check prevents errors supporting yet not existing TCAMs */
        prvCpssDxChVirtualTcamDbSegmentTableException();
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    startColumnsBitmap =
        prvCpssDxChVirtualTcamDbSegmentTableRuleEnumAlignmentBitmapGet(
            vTcamInfoPtr->tcamSegCfgPtr->deviceClass,
            vTcamInfoPtr->tcamInfo.ruleSize);
    if (startColumnsBitmap == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_FAIL, "vTcamMngId[%d] : vTcamId[%d] ruleSizeEnum [%d] no alignment bitmap",
            vTcamMngId, vTcamId, vTcamInfoPtr->tcamInfo.ruleSize);
    }

    ruleColumnsNum = prvCpssDxChVirtualTcamSegmentTableRuleSizeToUnits(
        vTcamInfoPtr->tcamSegCfgPtr, vTcamInfoPtr->tcamInfo.ruleSize);
    if (ruleColumnsNum == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_FAIL, "vTcamMngId[%d] : vTcamId[%d] ruleSizeEnum [%d] no rule size in units",
            vTcamMngId, vTcamId, vTcamInfoPtr->tcamInfo.ruleSize);
    }

    j = ((1 << ruleColumnsNum) - 1);
    maxColumnsBitmap = 0;
    for (i = 0; (i < vTcamInfoPtr->tcamSegCfgPtr->tcamColumnsAmount); i++)
    {
        if (startColumnsBitmap & (1 << i))
        {
            maxColumnsBitmap |= (j << i);
        }
    }

    horzSegmentsRowBase = 0; /* to avoid compiler warning */
    spaceSegmentsNum = 0;
    sumSpaceSegment.numOfRows = 0;
    horzSegmentsNum  = 0;
    seekMode = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
    seeked = GT_TRUE;
    lastLoop = GT_FALSE;
    while (seeked != GT_FALSE)
    {
        seeked = prvCpssAvlPathSeek(
                vTcamInfoPtr->segmentsTree, seekMode,
                path, &dbEntryPtr /* use dedicated var to avoid warnings */);
        segPtr = dbEntryPtr;
        seekMode = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;

        if (seeked == GT_FALSE)
        {
            /* last loop with dummy segment to output rest of data */
            cpssOsMemSet(&lastSeg, 0, sizeof(lastSeg));
            segPtr                    = &lastSeg;
            segPtr->rowsBase          = 0xFFFFFFFF;
            segPtr->rowsAmount        = 0;
            segPtr->segmentColumnsMap = 0;
            lastLoop                  = GT_TRUE;
        }

        if ((horzSegmentsNum == 0) && (lastLoop == GT_FALSE))
        {
            horzSegmentsRowBase = segPtr->rowsBase;
            numOfRowsArr[0] = segPtr->rowsAmount;
            columnsBmpArr[0] =
                maxColumnsBitmap &
                prvCpssDxChVirtualTcamDbSegmentTableSegColMapToMinRuleColumnsBitmap(
                    tcamWidth, blockWidth, segPtr->segmentColumnsMap);
            horzSegmentsNum ++;
            continue;
        }

        /* assumed (horzSegmentsNum != 0)*/
        if ((horzSegmentsRowBase == segPtr->rowsBase) && (lastLoop == GT_FALSE))
        {
            numOfRowsArr[horzSegmentsNum] = segPtr->rowsAmount;
            columnsBmpArr[horzSegmentsNum] =
                maxColumnsBitmap &
                prvCpssDxChVirtualTcamDbSegmentTableSegColMapToMinRuleColumnsBitmap(
                    tcamWidth, blockWidth, segPtr->segmentColumnsMap);
            horzSegmentsNum ++;
            if (horzSegmentsNum >= horzBlockWidth)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            continue;
        }

        /* assumed (horzSegmentsRowBase != segPtr->rowsBase) */

        if (horzSegmentsRowBase > segPtr->rowsBase)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        while ((horzSegmentsNum > 0) && (horzSegmentsRowBase < segPtr->rowsBase))
        {
            /* accomulated horizontal segments merging */
            horzSegmentsRowsNum = (segPtr->rowsBase - horzSegmentsRowBase);
            horzSegmentsColMap  = 0;
            for (i = 0; (i < horzSegmentsNum); i++)
            {
                if (horzSegmentsRowsNum > numOfRowsArr[i])
                {
                    horzSegmentsRowsNum = numOfRowsArr[i];
                }
                horzSegmentsColMap |= columnsBmpArr[i];
            }

            /* split already merged part */
            if ((sumSpaceSegment.numOfRows != 0)
                 && (sumSpaceSegment.columnsBitmap == horzSegmentsColMap)
                 && (horzSegmentsRowBase
                     == (sumSpaceSegment.startRowIndex + sumSpaceSegment.numOfRows)))
            {
                sumSpaceSegment.numOfRows     += horzSegmentsRowsNum;
            }
            else
            {
                if (sumSpaceSegment.numOfRows != 0)
                {
                    if (spaceSegmentsNum < maxNumOfSegments)
                    {
                        segmentArrayPtr[spaceSegmentsNum].startRowIndex = sumSpaceSegment.startRowIndex;
                        segmentArrayPtr[spaceSegmentsNum].columnsBitmap = sumSpaceSegment.columnsBitmap;
                        segmentArrayPtr[spaceSegmentsNum].numOfRows     = sumSpaceSegment.numOfRows;
                    }
                    spaceSegmentsNum ++;
                }
                sumSpaceSegment.startRowIndex = horzSegmentsRowBase;
                sumSpaceSegment.numOfRows     = horzSegmentsRowsNum;
                sumSpaceSegment.columnsBitmap = horzSegmentsColMap;
            }

            /* remove merged from array */
            for (i = 0; (i < horzSegmentsNum); i++)
            {
                numOfRowsArr[i] -= horzSegmentsRowsNum;
            }
            horzSegmentsRowBase += horzSegmentsRowsNum;
            j = 0;
            for (i = 0; (i < horzSegmentsNum); i++)
            {
                if (numOfRowsArr[i] == 0)
                {
                    j++;
                    continue;
                }
                if (j == 0) continue;
                numOfRowsArr[i - j]   = numOfRowsArr[i];
                columnsBmpArr[i - j]  = columnsBmpArr[i];
            }
            horzSegmentsNum -= j;
        }
        /* add new segment to arrays, see previous while loop condition */
        horzSegmentsRowBase = segPtr->rowsBase;
        numOfRowsArr[horzSegmentsNum] = segPtr->rowsAmount;
        columnsBmpArr[horzSegmentsNum] =
            maxColumnsBitmap &
            prvCpssDxChVirtualTcamDbSegmentTableSegColMapToMinRuleColumnsBitmap(
                tcamWidth, blockWidth, segPtr->segmentColumnsMap);
        horzSegmentsNum ++;
    }

    if (sumSpaceSegment.numOfRows > 0)
    {
        if (spaceSegmentsNum < maxNumOfSegments)
        {
            segmentArrayPtr[spaceSegmentsNum] = sumSpaceSegment;
        }
        spaceSegmentsNum ++;
    }
    *numOfSegmentsPtr = spaceSegmentsNum;

    return GT_OK;
}

/**
* @internal cpssDxChVirtualTcamSpaceLayoutGet function
* @endinternal
*
* @brief   Get vTCAM Space Layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] maxNumOfSegments         - maximal number of serments in output array
*                                       0 means that only numOfSegments required
*
* @param[out] numOfSegmentsPtr        - (pointer to)actual number of serments
* @param[out] segmentArrayPtr         - (pointer to)array of Space segments
*                                       NULL can be specified only when maxNumOfSegments == 0
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_FOUND             - vTcam with given Id not found
*/
GT_STATUS cpssDxChVirtualTcamSpaceLayoutGet
(
    IN   GT_U32                                     vTcamMngId,
    IN   GT_U32                                     vTcamId,
    IN   GT_U32                                     maxNumOfSegments,
    OUT  GT_U32                                     *numOfSegmentsPtr,
    OUT  CPSS_DXCH_VIRTUAL_TCAM_SPACE_SEGMENT_STC   segmentArrayPtr[]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamSpaceLayoutGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC(
        (funcId, vTcamMngId, vTcamId, maxNumOfSegments, numOfSegmentsPtr, segmentArrayPtr));

    rc = internal_cpssDxChVirtualTcamSpaceLayoutGet(
        vTcamMngId, vTcamId, maxNumOfSegments, numOfSegmentsPtr, segmentArrayPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC(
        (funcId, rc, vTcamMngId, vTcamId, maxNumOfSegments, numOfSegmentsPtr, segmentArrayPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}
/**
 * @internal internal_cpssDxChVirtualTcamManagerHaConfigReplayRuleListsGet function
 * @endinternal
 *
 * @brief  Get the head of two lists of rules to store failed and unmapped rules.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 *
 *
 * @param[out] gtNokRulesListPtr     - (pointer to) head of list of rules which failed during HA config replay.
 * @param[out] unmappedRulesListPtr  - (pointer to) head of list of rules which were removed from TCAM as it was unmapped during config replay.
 *
 * @retval GT_OK                     - on success
 */
static GT_STATUS internal_cpssDxChVirtualTcamManagerHaConfigReplayRuleListsGet
(
   OUT   CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC  **gtNokRulesListPtr,
   OUT   CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC   **unmappedRulesListPtr
)
{
    if (!gtNokRulesListPtr || !unmappedRulesListPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *gtNokRulesListPtr = VTCAM_GLOVAR(replayRuleDb);
    *unmappedRulesListPtr = VTCAM_GLOVAR(replayUnmappedRuleDb);

    return GT_OK;
}

/**
 * @internal cpssDxChVirtualTcamManagerHaConfigReplayRuleListsGet function
 * @endinternal
 *
 * @brief  Get the head of two lists of rules to store failed and unmapped rules.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 *
 *
 * @param[out] gtNokRulesListPtr     - (pointer to) head of list of rules which failed during HA config replay.
 * @param[out] unmappedRulesListPtr  - (pointer to) head of list of rules which were removed from TCAM as it was unmapped during config replay.
 *
 * @retval GT_OK                     - on success
 */
GT_STATUS cpssDxChVirtualTcamManagerHaConfigReplayRuleListsGet
(
   OUT   CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC  **gtNokRulesListPtr,
   OUT   CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC   **unmappedRulesListPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamManagerHaConfigReplayRuleListsGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    rc = internal_cpssDxChVirtualTcamManagerHaConfigReplayRuleListsGet(
           gtNokRulesListPtr, unmappedRulesListPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChVirtualTcamManagerHaConfigReplayRuleListsFree function
 * @endinternal
 *
 * @brief  Free the two lists of rules created during HA to store failed and unmapped rules.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 *
 *
 * @retval GT_OK                     - on success
 */
static GT_STATUS internal_cpssDxChVirtualTcamManagerHaConfigReplayRuleListsFree
(
   GT_VOID
)
{
    CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC *replayRulePtr;
    CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC  *replayUnmappedRulePtr;
    CPSS_DXCH_VIRTUAL_TCAM_HA_DB_REPLAY_RULE_ENTRY_STC *replayRulePtrTemp;
    CPSS_DXCH_VIRTUAL_TCAM_HA_UNMAPPED_RULE_ENTRY_STC  *replayUnmappedRulePtrTemp;

    replayRulePtr         = VTCAM_GLOVAR(replayRuleDb);
    replayUnmappedRulePtr = VTCAM_GLOVAR(replayUnmappedRuleDb);

    while (replayRulePtr)
    {
        replayRulePtrTemp = replayRulePtr;
        replayRulePtr = replayRulePtr->next;

        cpssOsFree(replayRulePtrTemp);
    }

    VTCAM_GLOVAR(replayRuleDb) = NULL;
    VTCAM_GLOVAR(replayRuleDbTail) = NULL;

    while (replayUnmappedRulePtr)
    {
        replayUnmappedRulePtrTemp = replayUnmappedRulePtr;
        replayUnmappedRulePtr = replayUnmappedRulePtr->next;

        cpssOsFree(replayUnmappedRulePtrTemp);
    }

    VTCAM_GLOVAR(replayUnmappedRuleDb) = NULL;
    VTCAM_GLOVAR(replayUnmappedRuleDbTail) = NULL;

    return GT_OK;
}

/**
 * @internal cpssDxChVirtualTcamManagerHaConfigReplayRuleListsFree function
 * @endinternal
 *
 * @brief  Free the two lists of rules created during HA to store failed and unmapped rules.
 *
 * @note   APPLICABLE DEVICES:      Aldrin2; Falcon.
 *
 *
 * @retval GT_OK                     - on success
 */
GT_STATUS cpssDxChVirtualTcamManagerHaConfigReplayRuleListsFree
(
   GT_VOID
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChVirtualTcamManagerHaConfigReplayRuleListsFree);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    rc = internal_cpssDxChVirtualTcamManagerHaConfigReplayRuleListsFree();

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_TCAM_MANAGER_CNS);

    return rc;
}
