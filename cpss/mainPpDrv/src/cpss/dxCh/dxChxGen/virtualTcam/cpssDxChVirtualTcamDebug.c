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
* @file cpssDxChVirtualTcamDebug.c
*
* @brief The CPSS DXCH Debug/verification tools for Virtual TCAM Manager
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
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/* condition to check that the last index in pritority range used by rule */
static const GT_BOOL prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify_priorityRangeLastUsed = GT_TRUE;

GT_VOID flipHaDbgPrints (GT_U32 item)
{

    GT_BOOL w;

    switch (item)
    {
        case 1:
            w = VTCAM_GLOVAR(haVtcamDebugSegmentsFlag);
            VTCAM_GLOVAR(haVtcamDebugSegmentsFlag) = (! w);
            break;
        case 2:
            w = VTCAM_GLOVAR(haVtcamDebugDbFlag);
            VTCAM_GLOVAR(haVtcamDebugDbFlag) = (! w);
            break;
        case 3:
            w = VTCAM_GLOVAR(haVtcamDebug);
            VTCAM_GLOVAR(haVtcamDebug) = (! w);
            break;
        case 4:
            w = VTCAM_GLOVAR(haVtcamTimeDebug);
            VTCAM_GLOVAR(haVtcamTimeDebug) = (! w);
            break;

        default:
            break;
    }
}

GT_STATUS tcamdump
(
)
{
    GT_U32 i, j, l, vTcamMngId = 0;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT  k;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC *vTcamMngPtr;

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if(vTcamMngPtr == NULL)
    {
        return GT_OK;
    }

    for(k=CPSS_DXCH_TCAM_RULE_SIZE_10_B_E;k<=CPSS_DXCH_TCAM_RULE_SIZE_80_B_E;++k)
    {
        for(l=PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_0_0_E; l<PRV_CPSS_DXCH_VTCAM_HA_DB_LOOKUPID_MAX_E; ++l)
        {
            for(i=0;i<vTcamMngPtr->vTcamHaDb[k][l].numRules;++i){
                cpssOsPrintf("print rule details, Physical Index %d,  RuleSize %d, RuleId %d, vTcamId %d, Found %d\nP",
                        vTcamMngPtr->vTcamHaDb[k][l].rulesArr[i].physicalIndex, k, vTcamMngPtr->vTcamHaDb[k][l].rulesArr[i].ruleId,
                        vTcamMngPtr->vTcamHaDb[k][l].rulesArr[i].vTcamId, vTcamMngPtr->vTcamHaDb[k][l].rulesArr[i].found);

                for(j=0;j<CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS;++j)
                    cpssOsPrintf("%08x", vTcamMngPtr->vTcamHaDb[k][l].rulesArr[i].pattern[j]);

                cpssOsPrintf("\nM");

                for(j=0;j<CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS;++j)
                    cpssOsPrintf("%08x", vTcamMngPtr->vTcamHaDb[k][l].rulesArr[i].mask[j]);

                cpssOsPrintf("\nA");
                for(j=0;j<CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS;++j)
                    cpssOsPrintf("%08x", vTcamMngPtr->vTcamHaDb[k][l].rulesArr[i].action[j]);

                cpssOsPrintf("\n");
            }
        }
    }

    return GT_OK;
}

#define PATTERN {0xABCDEFAB, 0xABCDEFAB, 0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,\
                0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB, 0xABCDEFAB,0xABCDEFAB}

GT_STATUS tcamrread
(
    IN GT_U8  devNum,
    IN GT_U32 index
)
{
        GT_STATUS    rc;
        GT_U32       mask[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS] = PATTERN; /* mask in HW format           */
        GT_U32       pattern[CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS] = PATTERN;
        GT_U32       action[CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS] = {0};
        GT_U32       entryNumber = 0;/* Rule's index in TCAM    */
        GT_U32       dummy,j;
        GT_BOOL      validFound = GT_FALSE; /* is rule found valid     */
        CPSS_DXCH_TCAM_RULE_SIZE_ENT
                     tcamRuleSize;
        GT_PORT_GROUPS_BMP
                     portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* convert rule global index to TCAM entry number */
        rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum, index, &entryNumber,&dummy);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* read action */
        rc = prvCpssDxChPortGroupReadTableEntry(devNum,
                                                0,
                                                CPSS_DXCH_SIP5_TABLE_TCAM_PCL_TTI_ACTION_E,
                                                (entryNumber/2),
                                                action);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = cpssDxChTcamPortGroupRuleRead(devNum, portGroupsBmp,index, &validFound,&tcamRuleSize,pattern,mask);
        cpssOsPrintf("\nRULE: idx=%d {valid=%d, ruleSize=%d,", index, validFound, tcamRuleSize);

        cpssOsPrintf("\nP");
        for(j=0;j<CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS;++j) cpssOsPrintf("%08x", pattern[j]);

        cpssOsPrintf("\nM");
        for(j=0;j<CPSS_DXCH_PCL_RULE_SIZE_IN_WORDS_CNS;++j) cpssOsPrintf("%08x", mask[j]);

        cpssOsPrintf("\nA");
        for(j=0;j<CPSS_DXCH_PCL_ACTION_SIZE_IN_WORDS_CNS;++j) cpssOsPrintf("%08x", action[j]);

        cpssOsPrintf("\n");


     return GT_OK;
}

GT_STATUS tcamrclr
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_TCAM_RULE_SIZE_ENT        ruleSize
)
{
    GT_PORT_GROUPS_BMP  portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_SPECIFIC_UNIT_MAC(devNum, portGroupsBmp, PRV_CPSS_DXCH_UNIT_TCAM_E);

    return prvCpssDxChVirtualTcamHaRuleClear(devNum, portGroupsBmp, index, ruleSize);
}

GT_VOID prvCpssDxChVirtualTcamHaDbgPrintSegment
(
    GT_CHAR      *info,
    GT_VOID_PTR   dataPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC
        *segmentsNodePtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)dataPtr;

    if (VTCAM_GLOVAR(haVtcamDebugSegmentsFlag))
    {
        cpssOsPrintf("\n<%25s> ", info);
        cpssOsPrintf("%3.3d\t ", segmentsNodePtr->rowsBase);
        cpssOsPrintf("%3.3d\t ", segmentsNodePtr->rowsAmount);
        cpssOsPrintf("COL_MAP 0x%2x\t ", segmentsNodePtr->segmentColumnsMap);
        cpssOsPrintf("%4.4d\t ", segmentsNodePtr->vTcamId);
        cpssOsPrintf("%3.3d\t ", segmentsNodePtr->lookupId);
        cpssOsPrintf("%4.4d\t ", segmentsNodePtr->rulesAmount);
        cpssOsPrintf("%d\t ", segmentsNodePtr->ruleSize);
        cpssOsPrintf("%d\n", segmentsNodePtr->baseLogicalIndex);
    }
}

/**
* @internal vtcamHaAppValidityWithCb function
* @endinternal
*
* @brief   API to create,init vTcamManager, create vTcam, add valid/invalid rules with zero pattern and mask
*
* @param[in] createMngr     - creates Manager if enabled
* @param[in] createVtcam    - creates vtcam(vtcamId)
* @param[in] postCreateMgrFuncPtr    - (pointer to) VTCAM Manager create callback (NULL - ignored)
* @param[in] vtcamId        - vtcam Id of the vtcam to be created
* @param[in] ruleSize       - ruleSize for the vtcam created/ruleAdd
* @param[in] vtcamSize      - guaranteed no. of rules in vtcam
* @param[in] numRules       - number of rule adds(vtcamruleWrite)
* @param[in] ruleIdOffset   - offset ruleIds to start from some index
* @param[in] vtcamMode      - 0- logicalIndex, 1- Priority
* @param[in] priority       - priority in case of Priority mode
* @param[in] valid          - specify valid/invalid rule
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS vtcamHaAppValidityWithCb
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           createMngr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_HA_POST_CREATE_MGR_FUNC *postCreateMgrFuncPtr,
    IN  GT_U8                           createVtcam,
    IN  GT_U32                          vtcamId,
    IN  GT_U32                          ruleSize,
    IN  GT_U32                          vtcamSize,
    IN  GT_U32                          numRules,
    IN  GT_U32                          ruleIdOffset,
    IN  GT_U8                           vtcamMode,
    IN  GT_U32                          priority,
    IN  GT_BOOL                         valid
)
{
    GT_STATUS                                   rc = GT_OK;
    GT_U32                                      vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC             vTcamInfo;
    CPSS_DXCH_PCL_ACTION_STC                    action;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               pattern;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              vTcamRuleId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC            vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  ruleAttributes;
    CPSS_PACKET_CMD_ENT                         defaultRulePktCmd    = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    CPSS_NET_RX_CPU_CODE_ENT                    defaultRuleCpuCode   = CPSS_NET_FIRST_USER_DEFINED_E + 2;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT          ipV4Key              = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT          ipV6Key              = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
    CPSS_SYSTEM_RECOVERY_INFO_STC               tempSystemRecovery_Info;
    CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC *haAppVtcamMngCfgParamPtr;

    haAppVtcamMngCfgParamPtr = &(VTCAM_GLOVAR(haAppVtcamMngCfgParam));

    vTcamMngId = devNum % CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS;

    cpssOsBzero((GT_CHAR*) &action, sizeof(action));
    cpssOsBzero((GT_CHAR*) &mask, sizeof(mask));
    cpssOsBzero((GT_CHAR*) &pattern, sizeof(pattern));
    cpssOsBzero((GT_CHAR*) &vTcamRuleId, sizeof(vTcamRuleId));
    cpssOsBzero((GT_CHAR*) &vTcamUsage, sizeof(vTcamUsage));
    cpssOsBzero((GT_CHAR*) &ruleData, sizeof(ruleData));
    cpssOsBzero((GT_CHAR*) &tcamRuleType, sizeof(tcamRuleType));
    cpssOsBzero((GT_CHAR*) &ruleAttributes, sizeof(ruleAttributes));
    cpssOsBzero((GT_CHAR*) &vTcamInfo, sizeof(vTcamInfo));

    if(vtcamMode == 1)
        ruleAttributes.priority = priority;

    if(createMngr)
    {
        /* attemt to delete VTCAM Manager - it may not be deleted after fail of the previous test */
        cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId, haAppVtcamMngCfgParamPtr);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamManagerCreate FAILED");
            return rc;
        }

        if (postCreateMgrFuncPtr)
        {
            rc = postCreateMgrFuncPtr(vTcamMngId);
            if (GT_OK != rc)
            {
                cpssOsPrintf("VTCAM Maneger Create Callback FAILED");
                return rc;
            }
        }

        rc = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, &devNum, 1);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamManagerDevListAdd FAILED");
            return rc;
        }
    }

    if(!ruleIdOffset){
        ruleIdOffset = 0;
    }

    if(createVtcam)
    {
        if (vtcamId >= 1024)
        {
            /* test supports TCAM ids < 1024, update test if need more values. */
            cpssOsPrintf("Failure: TCAM ID %d must be < 1024\n", vtcamId);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        vTcamInfo.clientGroup        = 0;
        vTcamInfo.hitNumber          = 0;
        vTcamInfo.ruleAdditionMethod = vtcamMode ?   CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E
            : CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* Create vTCAM(1) - short default rules (30B)*/
        vTcamInfo.ruleSize             = ruleSize;
        if (vtcamSize < numRules)
            vTcamInfo.autoResize           = GT_TRUE;
        else
            vTcamInfo.autoResize           = GT_FALSE;
        vTcamInfo.guaranteedNumOfRules = vtcamSize;
        /* 3 are default rules for IPV4 TCP, IPV4 UDP, Other */

        rc = cpssDxChVirtualTcamCreate(vTcamMngId, vtcamId, &vTcamInfo);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamCreate FAILED for vTcam : %d", vtcamId);
            return rc;
        }
    }

    /* Installing port-based default rules */
    for(vTcamRuleId=ruleIdOffset; vTcamRuleId < ruleIdOffset+numRules; vTcamRuleId++)
    {
        if(ruleSize != 7)/*not 80B*/
        {
            action.pktCmd                    = defaultRulePktCmd;
            action.mirror.cpuCode            = defaultRuleCpuCode;
            ruleData.valid                   = valid;
            ruleData.rule.pcl.actionPtr      = &action;
            ruleData.rule.pcl.maskPtr        = &mask;
            ruleData.rule.pcl.patternPtr     = &pattern;
            tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        }
        else
        {
            action.pktCmd                    = defaultRulePktCmd;
            action.mirror.cpuCode            = defaultRuleCpuCode;
            action.bypassBridge = GT_TRUE;
            ruleData.valid                   = valid;
            ruleData.rule.pcl.actionPtr      = &action;
            ruleData.rule.pcl.maskPtr        = &mask;
            ruleData.rule.pcl.patternPtr     = &pattern;
            tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        }

        switch (ruleSize)
        {
            case 0:
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
                break;

            case 1:
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
                break;

            case 2:
                tcamRuleType.rule.pcl.ruleFormat = ipV4Key;
                break;

            case 3:
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E;
                break;

            case 4:
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
                break;

            case 5:
                tcamRuleType.rule.pcl.ruleFormat = ipV6Key;
                break;

            case 7:
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Invalid rule size - HA App\n");
        }

        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            cpssDxChVirtualTcamUsageGet(vTcamMngId, vtcamId, &vTcamUsage);
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChVirtualTcamUsageGet FAILED :"
                        "devNum[%d], vTcamId[%d], rc[%d]\n",
                        devNum, vtcamId, rc);
                return rc;
            }

            if (vTcamInfo.autoResize == GT_FALSE && vTcamUsage.rulesFree == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"No Free rules left in TCAM FAILED :"
                        "devNum[%d], vTcamId[%d], rc[%d]\n",
                        devNum, vtcamId, rc);
            }
        }

        rc = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vtcamId,
                vTcamRuleId, &ruleAttributes,
                &tcamRuleType, &ruleData);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamRuleWrite FAILED :"
                    "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                    devNum, vtcamId, vTcamRuleId, rc);
            return rc;
        }

    }

    return GT_OK;
}

/**
* @internal vtcamHaAppWithCb function
* @endinternal
*
* @brief   API to create,init vTcamManager, create vTcam, perform ruleAdd
*          Used for UTs/Testing
*
* @param[in] createMngr     - creates Manager if enabled
* @param[in] createVtcam    - creates vtcam(vtcamId)
* @param[in] postCreateMgrFuncPtr    - (pointer to) VTCAM Manager create callback (NULL - ignored)
* @param[in] vtcamId        - vtcam Id of the vtcam to be created
* @param[in] ruleSize       - ruleSize for the vtcam created/ruleAdd
* @param[in] vtcamSize      - guaranteed no. of rules in vtcam
* @param[in] numRules       - number of rule adds(vtcamruleWrite)
* @param[in] vtcamIdOffset  - offset specified to make unique rules in vTcam
* @param[in] ruleIdOffset   - offset ruleIds to start from some index
* @param[in] vtcamMode      - 0- logicalIndex, 1- Priority
* @param[in] priority       - priority in case of Priority mode
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS vtcamHaAppWithCb
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           createMngr,
    IN  CPSS_DXCH_VIRTUAL_TCAM_HA_POST_CREATE_MGR_FUNC *postCreateMgrFuncPtr,
    IN  GT_U8                           createVtcam,
    IN  GT_U32                          vtcamId,
    IN  GT_U32                          ruleSize,
    IN  GT_U32                          vtcamSize,
    IN  GT_U32                          numRules,
    IN  GT_U8                           vtcamIdOffset,
    IN  GT_U32                          ruleIdOffset,
    IN  GT_U8                           vtcamMode,
    IN  GT_U32                          priority
)
{
    GT_STATUS                                   rc = GT_OK;
    GT_U32                                      vTcamMngId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC             vTcamInfo;
    CPSS_DXCH_PCL_ACTION_STC                    action;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT               pattern;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ID              vTcamRuleId;
    CPSS_DXCH_VIRTUAL_TCAM_USAGE_STC            vTcamUsage;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC        ruleData;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC        tcamRuleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC  ruleAttributes;
    CPSS_PACKET_CMD_ENT                         defaultRulePktCmd    = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    CPSS_NET_RX_CPU_CODE_ENT                    defaultRuleCpuCode   = CPSS_NET_FIRST_USER_DEFINED_E + 2;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT          ipV4Key              = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT          ipV6Key              = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
    GT_BOOL                                     managerHwWriteBlock;    /*used to check if current manager is blocked for HW write ,used for parallel High Availability*/
    CPSS_SYSTEM_RECOVERY_INFO_STC               tempSystemRecovery_Info;
    CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC *haAppVtcamMngCfgParamPtr;

    haAppVtcamMngCfgParamPtr = &(VTCAM_GLOVAR(haAppVtcamMngCfgParam));

    vTcamMngId = devNum % CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS;

    cpssOsBzero((GT_CHAR*) &action, sizeof(action));
    cpssOsBzero((GT_CHAR*) &mask, sizeof(mask));
    cpssOsBzero((GT_CHAR*) &pattern, sizeof(pattern));
    cpssOsBzero((GT_CHAR*) &vTcamRuleId, sizeof(vTcamRuleId));
    cpssOsBzero((GT_CHAR*) &vTcamUsage, sizeof(vTcamUsage));
    cpssOsBzero((GT_CHAR*) &ruleData, sizeof(ruleData));
    cpssOsBzero((GT_CHAR*) &tcamRuleType, sizeof(tcamRuleType));
    cpssOsBzero((GT_CHAR*) &ruleAttributes, sizeof(ruleAttributes));
    cpssOsBzero((GT_CHAR*) &vTcamInfo, sizeof(vTcamInfo));

    if(vtcamMode == 1)
        ruleAttributes.priority = priority;

    if(createMngr)
    {
        /* attemt to delete VTCAM Manager - it may not be deleted after fail of the previous test */
        cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        rc = cpssDxChVirtualTcamManagerCreate(vTcamMngId, haAppVtcamMngCfgParamPtr);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamManagerCreate FAILED");
            return rc;
        }

        if (postCreateMgrFuncPtr)
        {
            rc = postCreateMgrFuncPtr(vTcamMngId);
            if (GT_OK != rc)
            {
                cpssOsPrintf("VTCAM Maneger Create Callback FAILED");
                return rc;
            }
        }

        rc = cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, &devNum, 1);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamManagerDevListAdd FAILED");
            return rc;
        }
    }

    if(!vtcamIdOffset){
        vtcamIdOffset = (GT_U8)((numRules/10)>10?(numRules/10):10);
    }

    if(!ruleIdOffset){
        ruleIdOffset = 0;
    }

    if(createVtcam)
    {
        if (vtcamId >= 1024)
        {
            /* test supports TCAM ids < 1024, update test if need more values. */
            cpssOsPrintf("Failure: TCAM ID %d must be < 1024\n", vtcamId);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        vTcamInfo.clientGroup        = 0;
        vTcamInfo.hitNumber          = 0;
        vTcamInfo.ruleAdditionMethod = vtcamMode ?   CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E
            : CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;

        /* Create vTCAM(1) - short default rules (30B)*/
        vTcamInfo.ruleSize             = ruleSize;
        if (vtcamSize < numRules)
            vTcamInfo.autoResize           = GT_TRUE;
        else
            vTcamInfo.autoResize           = GT_FALSE;
        vTcamInfo.guaranteedNumOfRules = vtcamSize;
        /* 3 are default rules for IPV4 TCP, IPV4 UDP, Other */

        rc = cpssDxChVirtualTcamCreate(vTcamMngId, vtcamId, &vTcamInfo);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamCreate FAILED for vTcam : %d", vtcamId);
            return rc;
        }
    }

    /* Installing port-based default rules */
    for(vTcamRuleId=ruleIdOffset; vTcamRuleId < ruleIdOffset+numRules; vTcamRuleId++)
    {
        if(ruleSize != 7)/*not 80B*/
        {
            pattern.ruleIngrUdbOnly.replacedFld.srcPort = (vtcamIdOffset * vtcamId) + vTcamRuleId;
            mask.ruleIngrUdbOnly.replacedFld.srcPort = 0xFFF;

            action.pktCmd                    = defaultRulePktCmd;
            action.mirror.cpuCode            = defaultRuleCpuCode;
            ruleData.valid                   = GT_TRUE;
            ruleData.rule.pcl.actionPtr      = &action;
            ruleData.rule.pcl.maskPtr        = &mask;
            ruleData.rule.pcl.patternPtr     = &pattern;
            tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        }
        else
        {
            mask.ruleUltraIpv6PortVlanQos.vid1 = 0xFFF;
            action.pktCmd                    = defaultRulePktCmd;
            action.mirror.cpuCode            = defaultRuleCpuCode;
            action.bypassBridge = GT_TRUE;
            pattern.ruleUltraIpv6PortVlanQos.vid1 = (vtcamIdOffset * vtcamId) + vTcamRuleId;
            ruleData.valid                   = GT_TRUE;
            ruleData.rule.pcl.actionPtr      = &action;
            ruleData.rule.pcl.maskPtr        = &mask;
            ruleData.rule.pcl.patternPtr     = &pattern;
            tcamRuleType.ruleType            = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;
        }

        switch (ruleSize)
        {
            case 0:
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
                break;

            case 1:
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
                break;

            case 2:
                tcamRuleType.rule.pcl.ruleFormat = ipV4Key;
                break;

            case 3:
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E;
                break;

            case 4:
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
                break;

            case 5:
                tcamRuleType.rule.pcl.ruleFormat = ipV6Key;
                break;

            case 7:
                tcamRuleType.rule.pcl.ruleFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_ULTRA_IPV6_PORT_VLAN_QOS_E;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Invalid rule size - HA App\n");
        }
        rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (rc != GT_OK)
        {
            return rc;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_TCAM_MANAGER_E);
        if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E &&
            managerHwWriteBlock == GT_FALSE)
        {
            rc = cpssDxChVirtualTcamUsageGet(vTcamMngId, vtcamId, &vTcamUsage);
            if (GT_OK != rc)
            {
                cpssOsPrintf("cpssDxChVirtualTcamUsageGet FAILED :"
                        "devNum[%d], vTcamId[%d], rc[%d]\n",
                        devNum, vtcamId, rc);
                return rc;
            }

            if (vTcamInfo.autoResize == GT_FALSE && vTcamUsage.rulesFree == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"No Free rules left in TCAM FAILED :"
                        "devNum[%d], vTcamId[%d], rc[%d]\n",
                        devNum, vtcamId, rc);
            }

        }

        rc = cpssDxChVirtualTcamRuleWrite(vTcamMngId, vtcamId,
                vTcamRuleId, &ruleAttributes,
                &tcamRuleType, &ruleData);
        if (GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChVirtualTcamRuleWrite FAILED :"
                    "devNum[%d],vTcamId [%d] vTcamRuleId[%d], rc[%d]\n",
                    devNum, vtcamId, vTcamRuleId, rc);

            if (VTCAM_GLOVAR(haapp_ignore_write_ret) == GT_FALSE)
            {
                return rc;
            }
        }

    }

    return GT_OK;
}

static GT_STATUS vTcamHaDeleteAllVtcamRules
(
    IN GT_U32   vTcamMngId
)
{
    GT_STATUS   st;
    GT_STATUS   stNext;
    GT_U32  RuleId;
    GT_U32  RuleIdNext;
    GT_U32  vTcamId;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC    vTcamInfo;

    for (vTcamId = 0; vTcamId < 1024; vTcamId++)
    {
        if (GT_OK != cpssDxChVirtualTcamInfoGet(vTcamMngId,vTcamId,&vTcamInfo))
        {
            continue;
        }
        stNext = cpssDxChVirtualTcamNextRuleIdGet(
            vTcamMngId, vTcamId, CPSS_DXCH_VIRTUAL_TCAM_START_RULE_ID_CNS, &RuleId);
        while (GT_OK == stNext)
        {
            stNext = cpssDxChVirtualTcamNextRuleIdGet(
                vTcamMngId, vTcamId, RuleId, &RuleIdNext);

            st = cpssDxChVirtualTcamRuleDelete(
                vTcamMngId, vTcamId, RuleId);

            if (st != GT_OK)
            {
                cpssOsPrintf("cpssDxChVirtualTcamRuleDelete failed on ruleId[%d:%d]\n",vTcamId,RuleId);
                return st;
            }
            RuleId = RuleIdNext;
        }
    }
    return GT_OK;
}


/**
 * @internal prvCpssDxChVirtualTcamHaSystemRecoverySet function
 * @endinternal
 *
 * @brief used to set system recovery for HA, with vTCAM specific actions involved(clearing vTCAM DB etc.)
 * Usage only in UT/Testing
 *
 */
GT_STATUS prvCpssDxChVirtualTcamHaSystemRecoverySet
(
    IN GT_U8                                devNum,
    IN GT_U32                               vTcamMngId,
    IN CPSS_SYSTEM_RECOVERY_STATE_ENT       systemRecoveryState,
    IN CPSS_SYSTEM_RECOVERY_PROCESS_ENT     systemRecoveryProcess,
    IN GT_U32                               tcamReset
)
{
    GT_STATUS rc = GT_OK, rc1;
#ifdef CPSS_LOG_ENABLE
    GT_U32    start_sec  = 0;
    GT_U32    start_nsec = 0;
    GT_U32    end_sec    = 0;
    GT_U32    end_nsec   = 0;
    GT_U32    diff_sec   = 0;
    GT_U32    diff_nsec  = 0;
#endif
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC            *vTcamMngDBPtr;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;
    CPSS_DXCH_VIRTUAL_TCAM_PORT_GROUP_BITMAP_STC    portGroupBmpList;
    CPSS_SYSTEM_RECOVERY_INFO_STC newSystemRecoveryInfo;
#ifdef CPSS_LOG_ENABLE
    rc = cpssOsTimeRT(&start_sec, &start_nsec);
    if (GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssOsTimeRT FAILED");
    }
#endif

    vTcamMngId = vTcamMngId % CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS;

    if (systemRecoveryState == 1 && systemRecoveryProcess == 2)
    {
        vTcamMngDBPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
        if(vTcamMngDBPtr == NULL)
        {
            /* vTcam manager is NULL */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "vTcam manager not initialized\n");
        }

        tcamSegCfgPtr = &(vTcamMngDBPtr->tcamSegCfg);

        if(tcamSegCfgPtr)
        {
            cpssOsPrintf("\n\nSEGMENTSTREE LAYOUT before HA\n\n");
            prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(tcamSegCfgPtr);
        }

        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamPriorityModeData();

        cpssOsPrintf("HA for vTcam Manager started - INIT STATE\n");

        vTcamMngDBPtr->haSupported = GT_FALSE;/*rules not cleared - for HA reconstruction */

        portGroupBmpList.devNum = devNum;
        portGroupBmpList.portGroupsBmp = vTcamMngDBPtr->portGroupBmpArr[devNum];
        rc = cpssDxChVirtualTcamManagerPortGroupListRemove(vTcamMngId, &portGroupBmpList, 1, GT_FALSE);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChVirtualTcamManagerPortGroupListRemove FAILED systemRecovery(1,2)\n");
        }

        rc = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
        if (rc != GT_OK)
        {
            cpssOsPrintf("cpssDxChVirtualTcamManagerDelete FAILED");
            return rc;
        }

    }

    rc = cpssSystemRecoveryStateGet(&newSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* setting systemRecovery state and process */
    newSystemRecoveryInfo.systemRecoveryState     = systemRecoveryState;
    newSystemRecoveryInfo.systemRecoveryProcess   = systemRecoveryProcess;
    prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);

    if (systemRecoveryState == 3 && systemRecoveryProcess == 2)
    {
        cpssOsPrintf("HA for vTcam Manager - in H/w CatchUp STATE\n");
        rc = prvCpssDxChVirtualTcamCatchupForHa();
        if(rc != GT_OK)
        {
            cpssOsPrintf("H/w CatchUp failed\n");

            prvCpssDxChVirtualTcamHaDbClear(vTcamMngId);
            cpssOsPrintf("vTCAM HA DB cleared\n");

            vTcamMngDBPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
            if(vTcamMngDBPtr == NULL)
            {
                /* vTcam manager is NULL */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "vTcam manager is NULL\n");
            }
            tcamSegCfgPtr = &(vTcamMngDBPtr->tcamSegCfg);

            if(tcamSegCfgPtr)
            {
                cpssOsPrintf("\n\nSEGMENTSTREE LAYOUT after HA HwCatchup failed\n\n");
                prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(tcamSegCfgPtr);
            }

            portGroupBmpList.devNum = devNum;
            portGroupBmpList.portGroupsBmp = vTcamMngDBPtr->portGroupBmpArr[devNum];
            rc1 = cpssDxChVirtualTcamManagerPortGroupListRemove(vTcamMngId, &portGroupBmpList, 1, GT_FALSE);
            if (rc1 != GT_OK)
            {
                cpssOsPrintf("cpssDxChVirtualTcamManagerPortGroupListRemove FAILED  systemRecovery(3,2)\n");
            }

            rc1 = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
            if (rc1 != GT_OK)
            {
                cpssOsPrintf("cpssDxChVirtualTcamManagerDelete FAILED");
                return rc1;
            }

            cpssOsPrintf("vTCAM Manager destroyed with H/w reset\n");

            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDxChVirtualTcamHaCatchUp failed for dev: %d, vTcamMng: %d\n", devNum, vTcamMngId) ;
        }

        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamPriorityModeData();
    }

    if (systemRecoveryState == 2 && systemRecoveryProcess == 2)
    {
        newSystemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
        prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
        if(tcamReset){

            rc1 = vTcamHaDeleteAllVtcamRules(vTcamMngId);
            if (rc1 != GT_OK)
            {
                cpssOsPrintf("vTcamHaDeleteAllVtcamRules FAILED systemRecovery(2,2)\n ");
                 return rc1;
            }

            prvCpssDxChVirtualTcamHaDbClear(vTcamMngId);
            cpssOsPrintf("vTCAM HA DB cleared\n");

            vTcamMngDBPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
            if(vTcamMngDBPtr == NULL)
            {
                /* vTcam manager is NULL */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "vTcam manager not initialized\n");
            }
            portGroupBmpList.devNum = devNum;
            portGroupBmpList.portGroupsBmp = vTcamMngDBPtr->portGroupBmpArr[devNum];
            rc1 = cpssDxChVirtualTcamManagerPortGroupListRemove(vTcamMngId, &portGroupBmpList, 1, GT_FALSE);
            if (rc1 != GT_OK)
            {
                cpssOsPrintf("cpssDxChVirtualTcamManagerPortGroupListRemove FAILED systemRecovery(2,2)\n ");
            }

            rc1 = cpssDxChVirtualTcamManagerDelete(vTcamMngId);
            if (rc1 != GT_OK)
            {
                cpssOsPrintf("cpssDxChVirtualTcamManagerDelete FAILED");
                return rc1;
            }

            cpssOsPrintf("vTCAM Manager destroyed with H/w reset\n");

            /* Restore TCAM Active number of floors */
            rc = cpssDxChTcamActiveFloorsSet(devNum,
                    fineTuningPtr->tableSize.policyTcamRaws/CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"cpssDxChTcamActiveFloorsSet failed");
            }
        }

        CPSS_LOG_INFORMATION_MAC("HA for vTcam Manager Complete\n");
    }

#ifdef CPSS_LOG_ENABLE
    rc = cpssOsTimeRT(&end_sec, &end_nsec);
    if (GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssOsTimeRT FAILED");
    }

    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;

    CPSS_LOG_INFORMATION_MAC("system recovery time for State: [%d] & Process: [%d]  is [%d] seconds + [%d] nanoseconds \n" ,
            systemRecoveryState, systemRecoveryProcess, diff_sec , diff_nsec);
#endif

    return GT_OK;
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify function
* @endinternal
*
* @brief   Verify DB info about Virtual TCAM Rules layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_BOOL                                     stopAfterFirstError,
    IN  GT_BOOL                                     printErrorMessage
)
{
    GT_STATUS  totalRc;      /* last not OK returned code */
    PRV_CPSS_AVL_TREE_PATH      avlTreePath;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*     vtcamInfoPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC ruleIdEntry, * ruleIdEntryPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC    *foundPriorityEntryPtr;
    GT_VOID_PTR                 dbEntryPtr;  /* pointer to entry in DB             */
    PRV_CPSS_AVL_TREE_SEEK_ENT  seekMode;
    GT_U32  ii;
    GT_U32  actualUsed = 0;
    GT_U32  lastIndexFromPriority = 0xFFFFFFFF;
    GT_U32  savedIndexAfterPriority;
    GT_U32  emptyPriorityRangeLow;
    GT_U32  emptyPriorityRangeHigh;
    GT_U32  prevPrio = 0;
    GT_U32  errorCounter;

    totalRc = GT_OK;
    errorCounter = 0;

    /* get vtcam DB */
    vtcamInfoPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if(vtcamInfoPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify vTcamMngId [%d] vTcamId [%d] BAD PARAM\n",
                vTcamMngId ,vTcamId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    ruleIdEntry.ruleId = 0;
    ruleIdEntry.applicationDataPtr = NULL;

#define CHECK_TREE_ORDER_IS_CORRECT_MAC(treeId)                           \
    if(vtcamInfoPtr->treeId)                                              \
    {                                                                     \
        GT_BOOL treeIsOk = prvCpssAvlTreeIsOrderCorrect(vtcamInfoPtr->treeId); \
        if(treeIsOk == GT_FALSE)                                          \
        {                                                                 \
            prvCpssDxChVirtualTcamDbSegmentTableException();              \
            errorCounter ++;                                              \
            totalRc = GT_BAD_STATE;                                       \
            if (printErrorMessage != GT_FALSE)                            \
            {                                                             \
                cpssOsPrintf(                                             \
                    "prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify vTcamMngId [%d] vTcamId [%d] [%s] corrupted\n", \
                    vTcamMngId ,vTcamId,                                  \
                    #treeId);                                             \
            }                                                             \
        }                                                                 \
    }

    /**************************************************
        check trees
    ***************************************************/
    CHECK_TREE_ORDER_IS_CORRECT_MAC(segmentsTree);
    CHECK_TREE_ORDER_IS_CORRECT_MAC(ruleIdIdTree);
    CHECK_TREE_ORDER_IS_CORRECT_MAC(ruleIdIndexTree);
    CHECK_TREE_ORDER_IS_CORRECT_MAC(priorityPriTree);
    CHECK_TREE_ORDER_IS_CORRECT_MAC(priorityIndexTree);

    if(totalRc != GT_OK)
    {
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    /**************************************************
        check that logical index and ruleId coherent in the DB.
    ***************************************************/
    for (ii = 0; ii < vtcamInfoPtr->rulesAmount; ii++)
    {
        if (vtcamInfoPtr->usedRulesBitmapArr[ii>>5] & (1<<(ii & 0x1f)))
        {
            actualUsed++;

            if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod == CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E &&
               vtcamInfoPtr->ruleIdIndexTree )
            {
                /**************************************************
                    check that valid logical index also valid in the tree : ruleIdIndexTree
                ***************************************************/

                ruleIdEntry.logicalIndex = ii;
                ruleIdEntryPtr = prvCpssAvlSearch(vtcamInfoPtr->ruleIdIndexTree,&ruleIdEntry);
                if(ruleIdEntryPtr == NULL)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    errorCounter ++;
                    totalRc = GT_NOT_FOUND;
                    if (printErrorMessage != GT_FALSE)
                    {
                        cpssOsPrintf(
                            "VTcamRulesLayoutVerify: logical index [%d] missing in vtcamInfoPtr->ruleIdIndexTree\n", ii);
                    }
                    if (stopAfterFirstError != GT_FALSE)
                    {
                        return totalRc;
                    }
                }
                else
                {
                    /**************************************************
                        check that ruleId that was derived from tree ruleIdIndexTree
                        also exists in tree : ruleIdIdTree
                    ***************************************************/
                    ruleIdEntry.ruleId = ruleIdEntryPtr->ruleId;
                    ruleIdEntryPtr = prvCpssAvlSearch(vtcamInfoPtr->ruleIdIdTree,&ruleIdEntry);
                    if(ruleIdEntryPtr == NULL)
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        errorCounter ++;
                        totalRc = GT_NOT_FOUND;
                        if (printErrorMessage != GT_FALSE)
                        {
                            cpssOsPrintf(
                                "VTcamRulesLayoutVerify: RuleId [%d] missing in vtcamInfoPtr->ruleIdIdTree\n",
                                ruleIdEntry.ruleId);
                        }
                        if (stopAfterFirstError != GT_FALSE)
                        {
                            return totalRc;
                        }
                    }
                }
            }
        }
        else
        {
            /**************************************************
                check that non valid logical index also not exist in the tree : ruleIdIndexTree
            ***************************************************/
            if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod == CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E)
            {
                ruleIdEntry.logicalIndex = ii;
                ruleIdEntryPtr = prvCpssAvlSearch(vtcamInfoPtr->ruleIdIndexTree,&ruleIdEntry);
                if(ruleIdEntryPtr != NULL)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    errorCounter ++;
                    totalRc = GT_NOT_FOUND;
                    if (printErrorMessage != GT_FALSE)
                    {
                        cpssOsPrintf(
                            "VTcamRulesLayoutVerify: logical index [%d] found (with ruleId[%d])in vtcamInfoPtr->ruleIdIndexTree (but not in usedRulesBitmapArr[])\n",
                            ii,ruleIdEntryPtr->ruleId);
                    }
                    if (stopAfterFirstError != GT_FALSE)
                    {
                        return totalRc;
                    }
                }
            }
        }
    }

    /**************************************************
        check that actual number of logical indexes set in the usedRulesBitmapArr[]
        match the number of usedRulesAmount
    ***************************************************/
    if (vtcamInfoPtr->usedRulesAmount != actualUsed)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VTcamRulesLayoutVerify: Different vtcamInfoPtr->usedRulesAmount [%d] and actualUsed [%d] from vtcamInfoPtr->usedRulesBitmapArr \n",
                vtcamInfoPtr->usedRulesAmount, actualUsed);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    /**************************************************
        check priority table
    ***************************************************/
    if(vtcamInfoPtr->tcamInfo.ruleAdditionMethod == CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_PRIORITY_E &&
        vtcamInfoPtr->priorityPriTree)
    {
        savedIndexAfterPriority = 0;

        seekMode = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
        while(GT_TRUE == prvCpssAvlPathSeek(vtcamInfoPtr->priorityPriTree,
                seekMode,
                avlTreePath,
                &dbEntryPtr /* use dedicated var to avoid warnings */))
        {
            seekMode = PRV_CPSS_AVL_TREE_SEEK_NEXT_E;
            foundPriorityEntryPtr = dbEntryPtr;

            /**************************************************
                check that the tree priorityPriTree not hold ZERO size ranges.
            ***************************************************/
            if (foundPriorityEntryPtr->rangeSize == 0)
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                errorCounter ++;
                totalRc = GT_ALREADY_EXIST;
                if (printErrorMessage != GT_FALSE)
                {
                    cpssOsPrintf(
                        "VTcamRulesLayoutVerify: priority [%d] rangeSize == 0 (DB should not hold those) \n",
                        foundPriorityEntryPtr->priority);
                }
                if (stopAfterFirstError != GT_FALSE)
                {
                    return totalRc;
                }
            }

            /**************************************************
                check that priority info not violate rulesAmount.
            ***************************************************/
            if ((foundPriorityEntryPtr->baseLogIndex + foundPriorityEntryPtr->rangeSize) >
                vtcamInfoPtr->rulesAmount)
            {
                errorCounter ++;
                totalRc = GT_ALREADY_EXIST;
                if (printErrorMessage != GT_FALSE)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    cpssOsPrintf(
                        "VTcamRulesLayoutVerify: priority [%d] baseLogIndex [%d] + rangeSize[%d] violate rulesAmount[%d] \n",
                        foundPriorityEntryPtr->baseLogIndex,
                        foundPriorityEntryPtr->rangeSize,
                        vtcamInfoPtr->rulesAmount);
                }
                if (stopAfterFirstError != GT_FALSE)
                {
                    return totalRc;
                }
            }

            if(lastIndexFromPriority == 0xFFFFFFFF)
            {
                /* first time nothing to compare */
            }
            else
            {
                /**************************************************
                    check that previous range not violate into current priority
                    range.
                ***************************************************/
                if(lastIndexFromPriority >= foundPriorityEntryPtr->baseLogIndex)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    errorCounter ++;
                    totalRc = GT_ALREADY_EXIST;
                    if (printErrorMessage != GT_FALSE)
                    {
                        cpssOsPrintf(
                            "VTcamRulesLayoutVerify: priority [%d] starts at[%d] collide with end of previous priority [%] that ends at[%d]\n",
                            foundPriorityEntryPtr->priority,
                            foundPriorityEntryPtr->baseLogIndex,
                            prevPrio,
                            lastIndexFromPriority);
                    }
                    if (stopAfterFirstError != GT_FALSE)
                    {
                        return totalRc;
                    }
                }
            }
            /* save the end of range */
            lastIndexFromPriority = foundPriorityEntryPtr->baseLogIndex +
                                    foundPriorityEntryPtr->rangeSize - 1;


            prevPrio = foundPriorityEntryPtr->priority;

            /**************************************************
                check that no logical index is set between current priority range
               and the end of the previous range
            ***************************************************/
            emptyPriorityRangeLow = savedIndexAfterPriority;
            emptyPriorityRangeHigh = foundPriorityEntryPtr->baseLogIndex;
            if (emptyPriorityRangeLow < emptyPriorityRangeHigh)
            {
                for (ii = emptyPriorityRangeLow; (ii < emptyPriorityRangeHigh); ii++)
                {
                    if (vtcamInfoPtr->usedRulesBitmapArr[ii>>5] & (1<<(ii & 0x1f)))
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        errorCounter ++;
                        totalRc = GT_BAD_STATE;
                        if (printErrorMessage != GT_FALSE)
                        {
                            cpssOsPrintf(
                                "VTcamRulesLayoutVerify: logical index [%d] out of priority ranges\n", ii);
                        }
                        if (stopAfterFirstError != GT_FALSE)
                        {
                            return totalRc;
                        }
                    }
                }
            }
            savedIndexAfterPriority = foundPriorityEntryPtr->baseLogIndex
                + foundPriorityEntryPtr->rangeSize;

            if (prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify_priorityRangeLastUsed == GT_TRUE)
            {
                /**************************************************
                    check that the last index in the range of priority is set
                    (the 'delete' rule API should make sure of it)
                ***************************************************/
                ii = foundPriorityEntryPtr->baseLogIndex
                    + foundPriorityEntryPtr->rangeSize - 1;
                if ((vtcamInfoPtr->usedRulesBitmapArr[ii>>5] & (1<<(ii & 0x1f))) == 0)
                {
                    prvCpssDxChVirtualTcamDbSegmentTableException();
                    errorCounter ++;
                    totalRc = GT_ALREADY_EXIST;
                    if (printErrorMessage != GT_FALSE)
                    {
                        cpssOsPrintf(
                            "VTcamRulesLayoutVerify: priority [%d] rangeSize [%d] last index unused (but expected to be)\n",
                            foundPriorityEntryPtr->priority, foundPriorityEntryPtr->rangeSize);
                    }
                    if (stopAfterFirstError != GT_FALSE)
                    {
                        return totalRc;
                    }
                }
            }
        }

        /**************************************************
            check that no logical index is set after last priority range
        ***************************************************/
        if(vtcamInfoPtr->rulesAmount)
        {
            emptyPriorityRangeLow = savedIndexAfterPriority;
            emptyPriorityRangeHigh = vtcamInfoPtr->rulesAmount - 1;
            if (emptyPriorityRangeLow < emptyPriorityRangeHigh)
            {
                for (ii = emptyPriorityRangeLow; (ii < emptyPriorityRangeHigh); ii++)
                {
                    if (vtcamInfoPtr->usedRulesBitmapArr[ii>>5] & (1<<(ii & 0x1f)))
                    {
                        prvCpssDxChVirtualTcamDbSegmentTableException();
                        errorCounter ++;
                        totalRc = GT_BAD_STATE;
                        if (printErrorMessage != GT_FALSE)
                        {
                            cpssOsPrintf(
                                "VTcamRulesLayoutVerify: logical index [%d] out of priority ranges (after last priority range) \n", ii);
                        }
                        if (stopAfterFirstError != GT_FALSE)
                        {
                            return totalRc;
                        }
                    }
                }
            }
        }
    }

    /**************************************************
        check that the HW indexes for the logical indexes are in ascending order
    ***************************************************/
    if (vtcamInfoPtr->rulesAmount >= 2)
    {
        for (ii = 0; (ii < (vtcamInfoPtr->rulesAmount - 1)); ii++)
        {
            if (vtcamInfoPtr->rulePhysicalIndexArr[ii]
                >= vtcamInfoPtr->rulePhysicalIndexArr[ii + 1])
            {
                prvCpssDxChVirtualTcamDbSegmentTableException();
                errorCounter ++;
                totalRc = GT_BAD_STATE;
                if (printErrorMessage != GT_FALSE)
                {
                    cpssOsPrintf(
                        "VTcamRulesLayoutVerify: logical index [%d] hold HW index [%d] that is not less than the HW index [%d] of next logical index \n",
                            ii,
                            vtcamInfoPtr->rulePhysicalIndexArr[ii],
                            vtcamInfoPtr->rulePhysicalIndexArr[ii+1]);
                }
                if (stopAfterFirstError != GT_FALSE)
                {
                    return totalRc;
                }
            }
        }
    }

    if ((printErrorMessage != GT_FALSE) && (errorCounter != 0))
    {
        cpssOsPrintf(
            "prvCpssDxChVirtualTcamDbVTcamRulesLayoutVerify vTcamMngId [%d] vTcamId [%d] total errors [%d] \n",
            vTcamMngId ,vTcamId, errorCounter);
    }
    return totalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbVerifySegment function
* @endinternal
*
* @brief   Verify TCAM segment layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] segmentPtr               - (pointer to) segment DB entry
* @param[in] rowsAmount               - amount of rows in TCAM
* @param[in] blockColumnsNum          - amount of columns in block
* @param[in] tcamColumnsNum           - amount of  columns in TCAM
* @param[in,out] columnBmpArr[]           - array of column bitmaps per row (checked)
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
* @param[in,out] errorCounterPtr          - (pointer to) error counter
* @param[in,out] columnBmpArr[]           - array of column bitmaps per row (updated)
* @param[in,out] errorCounterPtr          - (pointer to) error counter
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
static GT_STATUS prvCpssDxChVirtualTcamDbVerifySegment
(
    IN     PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segmentPtr,
    IN     GT_U32                                          rowsAmount,
    IN     GT_U32                                          tcamColumnsNum,
    IN     GT_U32                                          blockColumnsNum,
    INOUT  GT_U16                                          columnBmpArr[],
    IN     GT_BOOL                                         stopAfterFirstError,
    IN     GT_BOOL                                         printErrorMessage,
    INOUT  GT_U32                                          *errorCounterPtr
)
{
    GT_STATUS totalRc; /* last not GT_OK returned code       */
    GT_U32    row;     /* TCAM row                           */
    GT_U32    maxRow;  /* TCAM row upper bound of segment    */
    GT_U32    segColumnsBitmap; /* bitmap of 10-byte columns */

    segColumnsBitmap =
        prvCpssDxChVirtualTcamDbSegmentTableSegColMapToMinRuleColumnsBitmap(
            tcamColumnsNum, blockColumnsNum, segmentPtr->segmentColumnsMap);

    totalRc = GT_OK;
    maxRow  = (segmentPtr->rowsBase + segmentPtr->rowsAmount);
    if (maxRow > rowsAmount)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        (*errorCounterPtr) ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VirtualTcamDbVerifySegment: segment out of TCAM rowsBase [%d] rowsAmount [%d]\n",
                segmentPtr->rowsBase, segmentPtr->rowsAmount);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    for (row = segmentPtr->rowsBase; (row < maxRow); row++)
    {
        if ((segColumnsBitmap & columnBmpArr[row]) != 0)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            (*errorCounterPtr) ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifySegment: overlap: row [%d] columns [0x%X] vTcamId [%d]\n",
                    row, (segColumnsBitmap & columnBmpArr[row]), segmentPtr->vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }
        columnBmpArr[row] |= (GT_U16)segColumnsBitmap;
    }

    return totalRc;
}

/*******************************************************************************
* prvCpssDxChVirtualTcamDbVerifySegmentTree
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* NOT APPLICABLE DEVICES: None.
*
* INPUTS:
*       segmentsPtr         - (pointer to) segment DB entry
*       rowsAmount          - amount of rows in TCAM
*       tcamColumnsNum      - amount of columns in TCAM
*       blockColumnsNum     - amount of columns in block
*       columnBmpArr        - array of column bitmaps per row (checked)
*       stopAfterFirstError - GT_TRUE  - return after first error
*                           - GT_FALSE - continue checking
*       printErrorMessage   - GT_TRUE  - print error messages
*                             GT_FALSE - count errors only
*       errorCounterPtr     - (pointer to) error counter
*
* OUTPUTS:
*       columnBmpArr        - array of column bitmaps per row (updated)
*       errorCounterPtr     - (pointer to) error counter
*
* RETURNS:
*       GT_OK               - no errors found,
*       other               - the code of the last error.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_STATUS prvCpssDxChVirtualTcamDbVerifySegmentTree
(
    IN     PRV_CPSS_AVL_TREE_ID                            segmentsTree,
    IN     GT_U32                                          rowsAmount,
    IN     GT_U32                                          tcamColumnsNum,
    IN     GT_U32                                          blockColumnsNum,
    INOUT  GT_U16                                          columnBmpArr[],
    IN     GT_BOOL                                         stopAfterFirstError,
    IN     GT_BOOL                                         printErrorMessage,
    INOUT  GT_U32                                          *errorCounterPtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *segmentPtr; /* pointer to segnent DB entry   */
    GT_STATUS                                       totalRc;     /* last not GT_OK returned code  */
    GT_STATUS                                       rc;          /* returned code                 */
    PRV_CPSS_AVL_TREE_SEEK_ENT                      seekType;    /* seek type                     */
    GT_VOID                                         *dbEntryPtr; /* DB Entry pointer              */
    PRV_CPSS_AVL_TREE_PATH                          path;        /* tree iterator                 */

    totalRc = GT_OK;

    /* pass segments to segments of the tree */
    for (seekType = PRV_CPSS_AVL_TREE_SEEK_FIRST_E;
         (GT_FALSE != prvCpssAvlPathSeek(
          segmentsTree, seekType, path, &dbEntryPtr));
         seekType = PRV_CPSS_AVL_TREE_SEEK_NEXT_E)
    {
        segmentPtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC*)dbEntryPtr;
        rc = prvCpssDxChVirtualTcamDbVerifySegment(
            segmentPtr, rowsAmount, tcamColumnsNum, blockColumnsNum, columnBmpArr,
            stopAfterFirstError, printErrorMessage, errorCounterPtr);
        if (rc != GT_OK)
        {
            totalRc = rc;
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }
    }
    return totalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbVerifyFullTcamLayout function
* @endinternal
*
* @brief   Verify DB info about full TCAM layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS prvCpssDxChVirtualTcamDbVerifyFullTcamLayout
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_BOOL                                     stopAfterFirstError,
    IN  GT_BOOL                                     printErrorMessage
)
{
    GT_STATUS                                     rc;           /* return code               */
    GT_STATUS                                     totalRc;      /* last not OK returned code */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr;  /* pointer to vTcam Manager  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr;  /* pointer to vTcam          */
    GT_U32                                        vTcamId;      /* vTcamId                   */
    GT_U32                                        errorCounter; /* error counter             */
    GT_U32                                        rowsAmount;   /* rows amount               */
    GT_U16                                        *columnBmpArr; /* column Bitmap Array      */
    GT_U16                                        *columnTtiBmpArr; /* column Bitmap Array      */
    GT_U16                                        *columnPclTcam1BmpArr; /* column Bitmap Array      */
    GT_U16                                        *columnCurrentBmpArr; /* column Bitmap Array      */
    GT_U16                                        fullColBmp;   /* full columns bitmap       */
    GT_U32                                        row;          /* rows index                */
    GT_U32                                        pclRowsStart;       /* PCL rows start in column array        */
    GT_U32                                        pclTtiStart;        /* TTI rows start in column array        */
    GT_U32                                        pclTcam1RowsStart;  /* PCL TCAM1 rows start in column array  */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if (vTcamMngPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VirtualTcamDbVerifyFullTcamLayout vTcamMngId [%d] BAD PARAM\n",
                vTcamMngId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (vTcamMngPtr->vTcamCfgPtrArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VirtualTcamDbVerifyFullTcamLayout vTcamMngId [%d] vTcamCfgPtrArr == NULL\n",
                vTcamMngId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if ((vTcamMngPtr->tcamSegCfg.tcamColumnsAmount > 16) ||
        (vTcamMngPtr->tcamTtiSegCfg.tcamColumnsAmount > 16 ||
         (vTcamMngPtr->pclTcam1SegCfg.tcamColumnsAmount > 16)))
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VirtualTcamDbVerifyFullTcamLayout vTcamMngId [%d] tcamColumnsAmount > 16 not supported\n",
                vTcamMngId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    totalRc = GT_OK;
    errorCounter = 0;
    rowsAmount = vTcamMngPtr->tcamSegCfg.tcamRowsAmount +
                    vTcamMngPtr->tcamTtiSegCfg.tcamRowsAmount +
                    vTcamMngPtr->pclTcam1SegCfg.tcamRowsAmount;
    columnBmpArr = (GT_U16*)cpssOsMalloc(rowsAmount * sizeof(GT_U16));
    if (columnBmpArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "VirtualTcamDbVerifyFullTcamLayout no memory fo work data\n");
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(columnBmpArr, 0, (rowsAmount * sizeof(GT_U16)));
    columnTtiBmpArr = &columnBmpArr[vTcamMngPtr->tcamSegCfg.tcamRowsAmount];
    columnPclTcam1BmpArr = &columnBmpArr[vTcamMngPtr->tcamSegCfg.tcamRowsAmount + vTcamMngPtr->tcamTtiSegCfg.tcamRowsAmount];

    rc = prvCpssDxChVirtualTcamDbVerifySegmentTree(
        vTcamMngPtr->tcamSegCfg.segFreeLuTree,
        vTcamMngPtr->tcamSegCfg.tcamRowsAmount,
        vTcamMngPtr->tcamSegCfg.tcamColumnsAmount,
        vTcamMngPtr->tcamSegCfg.lookupColumnAlignment,
        columnBmpArr,
        stopAfterFirstError, printErrorMessage, &errorCounter);
    if (rc != GT_OK)
    {
        totalRc = rc;
        if (stopAfterFirstError != GT_FALSE)
        {
            cpssOsFree(columnBmpArr);
            prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamSegCfg);
            return totalRc;
        }
    }
    rc = prvCpssDxChVirtualTcamDbVerifySegmentTree(
        vTcamMngPtr->tcamTtiSegCfg.segFreeLuTree,
        vTcamMngPtr->tcamTtiSegCfg.tcamRowsAmount,
        vTcamMngPtr->tcamTtiSegCfg.tcamColumnsAmount,
        vTcamMngPtr->tcamTtiSegCfg.lookupColumnAlignment,
        columnTtiBmpArr,
        stopAfterFirstError, printErrorMessage, &errorCounter);
    if (rc != GT_OK)
    {
        totalRc = rc;
        if (stopAfterFirstError != GT_FALSE)
        {
            cpssOsFree(columnBmpArr);
            prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamTtiSegCfg);
            return totalRc;
        }
    }

    rc = prvCpssDxChVirtualTcamDbVerifySegmentTree(
        vTcamMngPtr->pclTcam1SegCfg.segFreeLuTree,
        vTcamMngPtr->pclTcam1SegCfg.tcamRowsAmount,
        vTcamMngPtr->pclTcam1SegCfg.tcamColumnsAmount,
        vTcamMngPtr->pclTcam1SegCfg.lookupColumnAlignment,
        columnPclTcam1BmpArr,
        stopAfterFirstError, printErrorMessage, &errorCounter);
    if (rc != GT_OK)
    {
        totalRc = rc;
        if (stopAfterFirstError != GT_FALSE)
        {
            cpssOsFree(columnBmpArr);
            prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->pclTcam1SegCfg);
            return totalRc;
        }
    }

    for (vTcamId = 0; (vTcamId < vTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
    {
        vTcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];
        if (vTcamCfgPtr == NULL) continue;

        switch (vTcamCfgPtr->tcamSegCfgPtr->deviceClass)
        {
            case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_E:
            case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP5_E:
            case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E:
                columnCurrentBmpArr = columnBmpArr;
                break;
            case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_TTI_E:
                columnCurrentBmpArr = columnTtiBmpArr;
                break;
            case PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_LEGACY_PCL_TCAM1_E:
                columnCurrentBmpArr = columnPclTcam1BmpArr;
                break;
            default:
                cpssOsFree(columnBmpArr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssDxChVirtualTcamDbVerifySegmentTree(
            vTcamCfgPtr->segmentsTree, rowsAmount,
            vTcamCfgPtr->tcamSegCfgPtr->tcamColumnsAmount,
            vTcamCfgPtr->tcamSegCfgPtr->lookupColumnAlignment,
            columnCurrentBmpArr,
            stopAfterFirstError, printErrorMessage, &errorCounter);
        if (rc != GT_OK)
        {
            totalRc = rc;
            if (stopAfterFirstError != GT_FALSE)
            {
                cpssOsFree(columnBmpArr);
                prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(vTcamCfgPtr->tcamSegCfgPtr);
                return totalRc;
            }
        }
    }
    /* Check for lost segments */

   pclRowsStart      = 0;
   pclTtiStart       = pclRowsStart + vTcamMngPtr->tcamSegCfg.tcamRowsAmount;
   pclTcam1RowsStart = pclTtiStart + vTcamMngPtr->tcamTtiSegCfg.tcamRowsAmount;

    fullColBmp = (GT_U16)((1 << vTcamMngPtr->tcamSegCfg.tcamColumnsAmount) - 1);
    for (row = pclRowsStart; (row < pclTtiStart); row++)
    {
        if (fullColBmp != columnBmpArr[row])
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyFullTcamLayout found lost memory in row [%d]\n", row);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                cpssOsFree(columnBmpArr);
                prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamSegCfg);
                return totalRc;
            }
        }
    }

    fullColBmp = (GT_U16)((1 << vTcamMngPtr->tcamTtiSegCfg.tcamColumnsAmount) - 1);
    for (row = pclTtiStart; (row < pclTcam1RowsStart); row++)
    {
        if (fullColBmp != columnBmpArr[row])
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyFullTcamLayout found lost memory in row [%d]\n", row);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                cpssOsFree(columnBmpArr);
                prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamTtiSegCfg);
                return totalRc;
            }
        }
    }

    fullColBmp = (GT_U16)((1 << vTcamMngPtr->pclTcam1SegCfg.tcamColumnsAmount) - 1);
    for (row = pclTcam1RowsStart; (row < rowsAmount); row++)
    {
        if (fullColBmp != columnBmpArr[row])
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyFullTcamLayout found lost memory in row [%d]\n", row);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                cpssOsFree(columnBmpArr);
                prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->pclTcam1SegCfg);
                return totalRc;
            }
        }
    }

    /* cleanup */
    cpssOsFree(columnBmpArr);
    if ((printErrorMessage != GT_FALSE) && (errorCounter != 0))
    {
        cpssOsPrintf(
            "VirtualTcamDbVerifyFullTcamLayout vTcamMngId [%d] total errors [%d] \n",
            vTcamMngId , errorCounter);
    }
    if (totalRc != GT_OK)
    {
        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamSegCfg);
        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamTtiSegCfg);
        prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->pclTcam1SegCfg);
    }
    return totalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbVerifyAllTrees function
* @endinternal
*
* @brief   Verify trees in DB - matching data to compaire functions.
*         Should detect corruption caused by data updated without removing and inserting
*         related tree items.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
* @param[in] stopAfterFirstError      - GT_TRUE  - return after first error
*                                      - GT_FALSE - continue checking
* @param[in] printErrorMessage        - GT_TRUE  - print error messages
*                                      GT_FALSE - count errors only
*
* @retval GT_OK                    - no errors found,
* @retval other                    - the code of the last error.
*/
GT_STATUS prvCpssDxChVirtualTcamDbVerifyAllTrees
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_BOOL                                     stopAfterFirstError,
    IN  GT_BOOL                                     printErrorMessage
)
{
    GT_BOOL                                       rc;           /* return code               */
    GT_STATUS                                     totalRc;      /* last not OK returned code */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr;  /* pointer to vTcam Manager  */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr;  /* pointer to vTcam          */
    GT_U32                                        vTcamId;      /* vTcamId                   */
    GT_U32                                        errorCounter; /* error counter             */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if (vTcamMngPtr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] BAD PARAM\n",
                vTcamMngId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (vTcamMngPtr->vTcamCfgPtrArr == NULL)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamCfgPtrArr == NULL\n",
                vTcamMngId);
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    totalRc = GT_OK;
    errorCounter = 0;
    rc = prvCpssAvlTreeIsOrderCorrect(vTcamMngPtr->tcamSegCfg.segFreeLuTree);
    if (rc == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] segFreeLuTree corrupted\n",
                vTcamMngId);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }
    rc = prvCpssAvlTreeIsOrderCorrect(vTcamMngPtr->tcamTtiSegCfg.segFreeLuTree);
    if (rc == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] segFreeLuTree corrupted\n",
                vTcamMngId);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }
    rc = prvCpssAvlTreeIsOrderCorrect(vTcamMngPtr->pclTcam1SegCfg.segFreeLuTree);
    if (rc == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] segFreeLuTree corrupted\n",
                vTcamMngId);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    rc = prvCpssAvlTreeIsOrderCorrect(vTcamMngPtr->tcamSegCfg.segFreePrefTree);
    if (rc == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] segFreePrefTree corrupted\n",
                vTcamMngId);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    rc = prvCpssAvlTreeIsOrderCorrect(vTcamMngPtr->tcamTtiSegCfg.segFreePrefTree);
    if (rc == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] segFreePrefTree corrupted\n",
                vTcamMngId);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    rc = prvCpssAvlTreeIsOrderCorrect(vTcamMngPtr->pclTcam1SegCfg.segFreePrefTree);
    if (rc == GT_FALSE)
    {
        prvCpssDxChVirtualTcamDbSegmentTableException();
        errorCounter ++;
        totalRc = GT_BAD_STATE;
        if (printErrorMessage != GT_FALSE)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamDbVerifyAllTrees vTcamMngId [%d] segFreePrefTree corrupted\n",
                vTcamMngId);
        }
        if (stopAfterFirstError != GT_FALSE)
        {
            return totalRc;
        }
    }

    for (vTcamId = 0; (vTcamId < vTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
    {
        vTcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];
        if (vTcamCfgPtr == NULL) continue;

        rc = prvCpssAvlTreeIsOrderCorrect(vTcamCfgPtr->priorityIndexTree);
        if (rc == GT_FALSE)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            errorCounter ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamId  [%d] priorityIndexTree corrupted\n",
                    vTcamMngId, vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }

        rc = prvCpssAvlTreeIsOrderCorrect(vTcamCfgPtr->priorityPriTree);
        if (rc == GT_FALSE)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            errorCounter ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamId  [%d] priorityPriTree corrupted\n",
                    vTcamMngId, vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }

        rc = prvCpssAvlTreeIsOrderCorrect(vTcamCfgPtr->ruleIdIdTree);
        if (rc == GT_FALSE)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            errorCounter ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamId  [%d] ruleIdIdTree corrupted\n",
                    vTcamMngId, vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }

        rc = prvCpssAvlTreeIsOrderCorrect(vTcamCfgPtr->ruleIdIndexTree);
        if (rc == GT_FALSE)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            errorCounter ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamId ruleIdIndexTree [%d]  corrupted\n",
                    vTcamMngId, vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }

        rc = prvCpssAvlTreeIsOrderCorrect(vTcamCfgPtr->segmentsTree);
        if (rc == GT_FALSE)
        {
            prvCpssDxChVirtualTcamDbSegmentTableException();
            errorCounter ++;
            totalRc = GT_BAD_STATE;
            if (printErrorMessage != GT_FALSE)
            {
                cpssOsPrintf(
                    "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] vTcamId  [%d] segmentsTree corrupted\n",
                    vTcamMngId, vTcamId);
            }
            if (stopAfterFirstError != GT_FALSE)
            {
                return totalRc;
            }
        }
    }

    if ((printErrorMessage != GT_FALSE) && (errorCounter != 0))
    {
        cpssOsPrintf(
            "VirtualTcamDbVerifyAllTrees vTcamMngId [%d] total errors [%d] \n",
            vTcamMngId , errorCounter);
    }
    return totalRc;
}

/**
* @internal prvCpssDxChVirtualTcamDbManagerDump function
* @endinternal
*
* @brief   Dump tcam info without ‘vtcam info’
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbManagerDump
(
    IN  GT_U32                                      vTcamMngId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC *vTcamMngDBPtr; /* poinetr to vTcam manager */
    GT_U32 *devsBitmap; /* pointer to the devices bitmap */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC *tcamSegCfgPtr; /* pointer to DB segments table configuration */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_BLOCK_INFO_STC *blockInfArr; /* array of DB segments block state */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_PTR *vTcamCfgPtrArr; /* array of vTCAMs */
    GT_BOOL inRange; /* helper variable for printing range */
    GT_U32 i, beginOfRange, arrSize; /* index, helper variable for printing range, array size */

    cpssOsPrintf("\n"); /* make sure the printings will start in new line */

    if(vTcamMngId > CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS)
    {
        /* vTcamMngId out of range */
        cpssOsPrintf("vTcamMngId out of range\n");
        return;
    }

    vTcamMngDBPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if(vTcamMngDBPtr == NULL)
    {
        /* vTcam manager is NULL */
        cpssOsPrintf("vTcam manager is NULL\n");
        return;
    }

    devsBitmap = vTcamMngDBPtr->devsBitmap; /* get's devsBitmap */
    cpssOsPrintf("devsBitmap: ");
    if (devsBitmap == NULL)
    {
        cpssOsPrintf("NULL\n");
    }
    else
    {
        inRange = GT_FALSE;
        beginOfRange = 0;
        arrSize = vTcamMngDBPtr->numOfDevices; /* get's devsBitmap size */
        /* iterating over the bitmap */
        for (i = 0; i < arrSize; i++)
        {
            if (0 == (devsBitmap[i>>5] & (1<<(i & 0x1f))))
            {
                /* end of range */
                if (inRange == GT_TRUE)
                {
                    /* printing the range that just ended*/
                    if ( i != 0 && i-1 != beginOfRange)
                    {
                        cpssOsPrintf("..%d", i - 1);
                    }
                    if (i != arrSize-1)
                    {
                        cpssOsPrintf(", ");
                    }
                    inRange = GT_FALSE;
                }
            }
            /* devsBitmap[i] is on */
            else
            {
                /* beginning of range */
                if (inRange == GT_FALSE)
                {
                    cpssOsPrintf("%d",i);
                    inRange = GT_TRUE;
                    beginOfRange = i;
                }
            }
        }
        /* daling with the last range */
        if (inRange == GT_TRUE)
        {
            /* printing the range that just ended*/
            if (i != 0 && i-1 != beginOfRange)
            {
                cpssOsPrintf("..%d", i - 1);
            }
        }
    }
    cpssOsPrintf("\n");

    arrSize = vTcamMngDBPtr->numOfDevices;
    cpssOsPrintf("numOfDevices = %d\n", arrSize); /* printing num of devices*/

    tcamSegCfgPtr = &vTcamMngDBPtr->tcamSegCfg;
    cpssOsPrintf("tcamSegCfg{\n");/* printing tcamSegCfg structure*/
    cpssOsPrintf("\t");
    cpssOsPrintf("tcamColumnsAmount = %d\n", tcamSegCfgPtr->tcamColumnsAmount);
    cpssOsPrintf("\t");
    cpssOsPrintf("tcamRowsAmount = %d\n", tcamSegCfgPtr->tcamRowsAmount);
    cpssOsPrintf("\t");
    cpssOsPrintf("lookupColumnAlignment = %d\n", tcamSegCfgPtr->lookupColumnAlignment);
    cpssOsPrintf("\t");
    cpssOsPrintf("lookupRowAlignment = %d\n", tcamSegCfgPtr->lookupRowAlignment);
    cpssOsPrintf("\t");
    cpssOsPrintf("maxSegmentsInDb = %d\n", tcamSegCfgPtr->maxSegmentsInDb);

    arrSize = tcamSegCfgPtr->blocksAmount;
    cpssOsPrintf("blocksAmount = %d\n", arrSize); /* printing blocks amount*/

    blockInfArr = tcamSegCfgPtr->blockInfArr;
    if (blockInfArr == NULL)
    {
        cpssOsPrintf("blockInfArr is NULL\n");
    }
    else
    {
        cpssOsPrintf("blockInfArr table:\n");
        cpssOsPrintf("index\t");
        cpssOsPrintf("columnsBase\t");
        cpssOsPrintf("rowsBase\t");
        cpssOsPrintf("columnsAmount\t");
        cpssOsPrintf("rowsAmount\t");
        cpssOsPrintf("lookupId\n");
        /* iterating over blockInfArr elements */
        for (i = 0; i < arrSize; i++)
        {
            /* printing blockInfArr element*/
            cpssOsPrintf("%d\t    ",i);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].columnsBase);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].rowsBase);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].columnsAmount);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].rowsAmount);
            cpssOsPrintf("%d\n",       blockInfArr[i].lookupId);
        }
    }
    cpssOsPrintf("}\n");

    tcamSegCfgPtr = &vTcamMngDBPtr->tcamTtiSegCfg;
    cpssOsPrintf("tcamTtiSegCfg{\n");/* printing tcamTtiSegCfg structure*/
    cpssOsPrintf("\t");
    cpssOsPrintf("tcamColumnsAmount = %d\n", tcamSegCfgPtr->tcamColumnsAmount);
    cpssOsPrintf("\t");
    cpssOsPrintf("tcamRowsAmount = %d\n", tcamSegCfgPtr->tcamRowsAmount);
    cpssOsPrintf("\t");
    cpssOsPrintf("lookupColumnAlignment = %d\n", tcamSegCfgPtr->lookupColumnAlignment);
    cpssOsPrintf("\t");
    cpssOsPrintf("lookupRowAlignment = %d\n", tcamSegCfgPtr->lookupRowAlignment);
    cpssOsPrintf("\t");
    cpssOsPrintf("maxSegmentsInDb = %d\n", tcamSegCfgPtr->maxSegmentsInDb);

    arrSize = tcamSegCfgPtr->blocksAmount;
    cpssOsPrintf("blocksAmount = %d\n", arrSize); /* printing blocks amount*/

    blockInfArr = tcamSegCfgPtr->blockInfArr;
    if (blockInfArr == NULL)
    {
        cpssOsPrintf("blockInfArr is NULL\n");
    }
    else
    {
        cpssOsPrintf("blockInfArr table:\n");
        cpssOsPrintf("index\t");
        cpssOsPrintf("columnsBase\t");
        cpssOsPrintf("rowsBase\t");
        cpssOsPrintf("columnsAmount\t");
        cpssOsPrintf("rowsAmount\t");
        cpssOsPrintf("lookupId\n");
        /* iterating over blockInfArr elements */
        for (i = 0; i < arrSize; i++)
        {
            /* printing blockInfArr element*/
            cpssOsPrintf("%d\t    ",i);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].columnsBase);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].rowsBase);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].columnsAmount);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].rowsAmount);
            cpssOsPrintf("%d\n",       blockInfArr[i].lookupId);
        }
    }
    cpssOsPrintf("}\n");

    tcamSegCfgPtr = &vTcamMngDBPtr->pclTcam1SegCfg;
    cpssOsPrintf("pclTcam1SegCfg{\n");/* printing tcamTtiSegCfg structure*/
    cpssOsPrintf("\t");
    cpssOsPrintf("tcamColumnsAmount = %d\n", tcamSegCfgPtr->tcamColumnsAmount);
    cpssOsPrintf("\t");
    cpssOsPrintf("tcamRowsAmount = %d\n", tcamSegCfgPtr->tcamRowsAmount);
    cpssOsPrintf("\t");
    cpssOsPrintf("lookupColumnAlignment = %d\n", tcamSegCfgPtr->lookupColumnAlignment);
    cpssOsPrintf("\t");
    cpssOsPrintf("lookupRowAlignment = %d\n", tcamSegCfgPtr->lookupRowAlignment);
    cpssOsPrintf("\t");
    cpssOsPrintf("maxSegmentsInDb = %d\n", tcamSegCfgPtr->maxSegmentsInDb);

    arrSize = tcamSegCfgPtr->blocksAmount;
    cpssOsPrintf("blocksAmount = %d\n", arrSize); /* printing blocks amount*/

    blockInfArr = tcamSegCfgPtr->blockInfArr;
    if (blockInfArr == NULL)
    {
        cpssOsPrintf("blockInfArr is NULL\n");
    }
    else
    {
        cpssOsPrintf("blockInfArr table:\n");
        cpssOsPrintf("index\t");
        cpssOsPrintf("columnsBase\t");
        cpssOsPrintf("rowsBase\t");
        cpssOsPrintf("columnsAmount\t");
        cpssOsPrintf("rowsAmount\t");
        cpssOsPrintf("lookupId\n");
        /* iterating over blockInfArr elements */
        for (i = 0; i < arrSize; i++)
        {
            /* printing blockInfArr element*/
            cpssOsPrintf("%d\t    ",i);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].columnsBase);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].rowsBase);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].columnsAmount);
            cpssOsPrintf("%d\t\t    ", blockInfArr[i].rowsAmount);
            cpssOsPrintf("%d\n",       blockInfArr[i].lookupId);
        }
    }
    cpssOsPrintf("}\n");

    arrSize = vTcamMngDBPtr->vTcamCfgPtrArrSize;
    vTcamCfgPtrArr = vTcamMngDBPtr->vTcamCfgPtrArr;
    if (vTcamCfgPtrArr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtrArr is NULL\n");
    }
    else
    {
        cpssOsPrintf("vTcamCfgPtrArr: ");
        beginOfRange = GT_FALSE;
        /* iterating over vTcamCfgPtrArr elements */
        for (i = 0; i < arrSize; i++)
        {
            /* printing all indexes of elements which are not NULL */
            if (vTcamCfgPtrArr[i] != NULL)
            {
                if (beginOfRange == GT_FALSE)
                {
                    cpssOsPrintf("%d",i);
                    beginOfRange = GT_TRUE;
                }
                else
                {
                    cpssOsPrintf(", %d",i);
                }
            }
        }
        cpssOsPrintf("\n");
    }
}

/**
* @internal printTcamInfo function

* @endinternal
*
* @brief   print tcam info about a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamInfo                 - structure of tcamInfo
*                                       None
*/
static GT_VOID printTcamInfo
(
    IN CPSS_DXCH_VIRTUAL_TCAM_INFO_STC tcamInfo
)
{
    /* the vTcam Information */
    cpssOsPrintf("tcamInfo{\n");
    cpssOsPrintf("\t");
    cpssOsPrintf("clientGroup = %d\n", tcamInfo.clientGroup);
    cpssOsPrintf("\t");
    cpssOsPrintf("hitNumber = %d\n", tcamInfo.hitNumber);
    cpssOsPrintf("\t");
    cpssOsPrintf("ruleSize = %d\n", tcamInfo.ruleSize);
    cpssOsPrintf("\t");
    cpssOsPrintf("autoResize = %d\n", tcamInfo.autoResize);
    cpssOsPrintf("\t");
    cpssOsPrintf("guaranteedNumOfRules = %d\n", tcamInfo.guaranteedNumOfRules);
    cpssOsPrintf("\t");
    cpssOsPrintf("ruleAdditionMethod = %d\n", tcamInfo.ruleAdditionMethod);
    cpssOsPrintf("}\n");
}


/**
* @internal printUsedRulesBitmapArr function
* @endinternal
*
* @brief   print a used rules bitmap array of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] usedRulesBitmapArr       - bitmap of used rules
* @param[in] rulesAmount              - amount of rules
* @param[in] expected_usedRulesAmount - expected amount of used rules
*                                       None
*/
static GT_VOID printUsedRulesBitmapArr
(
    IN GT_U32   *usedRulesBitmapArr,
    IN GT_U32   rulesAmount,
    IN GT_U32   expected_usedRulesAmount
)
{
    if (usedRulesBitmapArr == NULL)
    {
        cpssOsPrintf("usedRulesBitmapArr:not-exist \n");
        return;
    }
    cpssOsPrintf("usedRulesBitmapArr: ");
    printBitmapArr("usedRulesBitmapArr",
                    usedRulesBitmapArr,
                    rulesAmount,
                    expected_usedRulesAmount);
}

/**
* @internal print_segmentColumnsMap function
* @endinternal
*
* @brief   print a used rules bitmap array of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       None
*/
static GT_VOID print_segmentColumnsMap
(
    IN PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP segmentColumnsMap
)
{
    GT_U32 i;
    char* name;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT segmentColumns;

    for (i = 0; (i < 8); i++)
    {
        segmentColumns = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ROW_MAP_GET_MAC(
            segmentColumnsMap, i);
        if (segmentColumns ==
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E) continue;
        name =
            (segmentColumns < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E)
                ? prvCpssDxChVirtualTcamDbSegmentColumnsEnumNames[segmentColumns]
                : "WRONG";
        cpssOsPrintf("%d/%s ", i, name);
    }
    cpssOsPrintf("\t");
}

/**
* @internal printRulePhysicalIndexArr function
* @endinternal
*
* @brief   print a rule physical index array of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] rulesAmount              - amount of rules
* @param[in] rulePhysicalIndexArr     - array of physical indexes
*                                       None
*/
static GT_VOID printRulePhysicalIndexArr
(
    IN GT_U16   *rulePhysicalIndexArr,
    IN GT_U32   rulesAmount
)
{
    GT_U32 i,j,rowNum; /* indexes */

    if (rulePhysicalIndexArr == NULL)
    {
        cpssOsPrintf("rulePhysicalIndexArr is NULL\n");
    }
    else
    {
        cpssOsPrintf("Logical index to HW index mapping:\n\n");
        cpssOsPrintf("Row/column\t");
        for (i = 0; i < 10 ; i++)
        {
            cpssOsPrintf("%d\t", i);
        }
        cpssOsPrintf("\n");
        rowNum = 0;
        /* iterating over rulePhysicalIndexArr elements and printing*/
        for (i = 0; i < rulesAmount; i++)
        {
            cpssOsPrintf("%d\t\t",rowNum);
            /* printing a row */
            for (j = 0; j < 10 && i < rulesAmount; j++, i++ )
            {
                cpssOsPrintf("%d\t",rulePhysicalIndexArr[i]);
            }
            cpssOsPrintf("\n");
            rowNum++;
            i--;
        }
        cpssOsPrintf("\n");
    }
}

/**
* @internal printSegmentsTree function
* @endinternal
*
* @brief   print the avl segments tree of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] treeIdPtr                - a pointer to the id of the segments tree
*                                       None
*/
static GT_VOID printSegmentsTree
(
    IN PRV_CPSS_AVL_TREE_ID treeIdPtr
)
{
    GT_U32 i; /* index */
    PRV_CPSS_AVL_TREE_ITERATOR_ID iterPtr; /* avl tree iterator */
    GT_VOID *dataPtr; /* pointer to avl node data */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC  *segmentsNodePtr; /* pointer to segmentsTree node data */

    /* checks if the tree itslef is not NULL */
    if (treeIdPtr == NULL)
    {
        cpssOsPrintf("segmentsTree is NULL\n");
    }
    else
    {
        i = 0;
        /* get's the first avl tree node */
        iterPtr = NULL;
        dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
        if (dataPtr == NULL)
        {
            cpssOsPrintf("segmentsTree is empty\n");
        }
        else
        {
            cpssOsPrintf("segmentsTree table:\n\n");
            cpssOsPrintf("index\t");
            cpssOsPrintf("rowsBase    ");
            cpssOsPrintf("rowsAmount\t");
            cpssOsPrintf("segmentColumnsMap\t");
            cpssOsPrintf("vTcamId\t   ");
            cpssOsPrintf("lookupId\t");
            cpssOsPrintf("rulesAmount\t");
            cpssOsPrintf("ruleSize    ");
            cpssOsPrintf("baseLogicalIndex\n");
            /* iterating over all tree elements */
            while (dataPtr != NULL)
            {
                segmentsNodePtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_ENTRY_STC *)dataPtr;
                cpssOsPrintf("%d\t ",i);
                cpssOsPrintf("%3.3d\t      ", segmentsNodePtr->rowsBase);
                cpssOsPrintf("%3.3d\t    ", segmentsNodePtr->rowsAmount);
                print_segmentColumnsMap(segmentsNodePtr->segmentColumnsMap);
                cpssOsPrintf("%4.4d\t      ", segmentsNodePtr->vTcamId);
                cpssOsPrintf("%3.3d\t   ", segmentsNodePtr->lookupId);
                cpssOsPrintf("%4.4d\t\t    ", segmentsNodePtr->rulesAmount);
                cpssOsPrintf("%d\t\t  ", segmentsNodePtr->ruleSize);
                cpssOsPrintf("%d\n", segmentsNodePtr->baseLogicalIndex);
                dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
                i++;
            }
            cpssOsPrintf("\n");
        }
    }
}

GT_VOID wrPrintSegmentsTree
(
    IN PRV_CPSS_AVL_TREE_ID treeIdPtr,
    IN GT_BOOL              enable
)
{
    if(enable)
        printSegmentsTree(treeIdPtr);
}

/**
* @internal printRuleIdTree function
* @endinternal
*
* @brief   print the avl rule ID type tree of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] treeIdPtr                - a pointer to the id of the rule ID type tree
* @param[in] namePtr                  - the name of the tree
*                                      can be "ruleIdIdTree" or "ruleIdIndexTree"
*                                       None
*/
static GT_VOID printRuleIdTree
(
    IN PRV_CPSS_AVL_TREE_ID        treeIdPtr,
    IN GT_CHAR_PTR                 namePtr
)
{
    GT_U32 i; /* index */
    PRV_CPSS_AVL_TREE_ITERATOR_ID iterPtr; /* avl tree iterator */
    GT_VOID *dataPtr; /* pointer to avl node data */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC  *ruleIdIdNodePtr; /* pointer to ruleIdTree node data */

    /* checks if the tree itslef is not NULL */
    if (treeIdPtr == NULL)
    {
        cpssOsPrintf("%s is NULL\n",namePtr);
    }
    else
    {
        i = 0;
        /* get's the first avl tree node */
        iterPtr = NULL;
        dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
        if (dataPtr == NULL)
        {
            cpssOsPrintf("%s is empty\n",namePtr);
        }
        else
        {
            cpssOsPrintf("%s table:\n\n",namePtr);
            cpssOsPrintf("index\t");
            cpssOsPrintf("ruleId\t");
            cpssOsPrintf("logicalIndex\n");
            /* iterating over all tree elements */
            while (dataPtr != NULL)
            {
                ruleIdIdNodePtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RULE_ID_ENTRY_STC *)dataPtr;
                cpssOsPrintf("%d\t ",i);
                cpssOsPrintf("%d\t", ruleIdIdNodePtr->ruleId);
                cpssOsPrintf("%d\n", ruleIdIdNodePtr->logicalIndex);
                dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
                i++;
            }
            cpssOsPrintf("\n");
        }
    }
}

/**
* @internal printPriorityTree function
* @endinternal
*
* @brief   print the avl priority type tree of a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] treeIdPtr                - a pointer to the id of the priority type tree
* @param[in] namePtr                  - the name of the tree
*                                      can be "priorityPriTree" or "priorityIndexTree"
*                                       None
*/
static GT_VOID printPriorityTree
(
    IN PRV_CPSS_AVL_TREE_ID        treeIdPtr,
    IN GT_CHAR_PTR                 namePtr
)
{
    GT_U32 i; /* index */
    PRV_CPSS_AVL_TREE_ITERATOR_ID iterPtr; /* avl tree iterator */
    GT_VOID *dataPtr; /* pointer to avl node data */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *priorityPriNodePtr; /* pointer to priorityTree node data */

    if (treeIdPtr == NULL)
    {
        cpssOsPrintf("%s is NULL\n", namePtr);
    }
    else
    {
        i = 0;
        /* get's the first avl tree node */
        iterPtr = NULL;
        dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
        if (dataPtr == NULL)
        {
            cpssOsPrintf("%s is empty\n", namePtr);
        }
        else
        {
            cpssOsPrintf("%s table:\n\n",namePtr);
            cpssOsPrintf("index\t");
            cpssOsPrintf("priority\t");
            cpssOsPrintf("baseLogIndex\t");
            cpssOsPrintf("rangeSize\n");
            /* iterating over all tree elements */
            while (dataPtr != NULL)
            {
                priorityPriNodePtr = (PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_PRIORITY_ENTRY_STC *)dataPtr;
                cpssOsPrintf("%d\t ",i);
                cpssOsPrintf("%d\t", priorityPriNodePtr->priority);
                cpssOsPrintf("%d\t", priorityPriNodePtr->baseLogIndex);
                cpssOsPrintf("%d\n", priorityPriNodePtr->rangeSize);
                dataPtr = prvCpssAvlGetNext(treeIdPtr, &iterPtr);
                i++;
            }
            cpssOsPrintf("\n");
        }
    }
}

/**
* @enum HW_INDEX_SEGMENT_TYPE_ENT
 *
 * @brief enumeration for typed of segments for HW indexes.
*/
typedef enum{

    /** @brief indication that the current HW index is in 'existing' segment
     *  'old' = valid index , old = valid index
     */
    HW_INDEX_SEGMENT_TYPE_EXISTING_E,

    /** @brief indication that the current HW index is in 'added' segment
     *  'old' = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS , new = valid index
     */
    HW_INDEX_SEGMENT_TYPE_ADDED_E,

    /** @brief indication that the current HW index is in 'removed' segment
     *  'new' = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS , old = valid index
     */
    HW_INDEX_SEGMENT_TYPE_REMOVED_E

} HW_INDEX_SEGMENT_TYPE_ENT;


/* indication to not valid diff of GT_32 variable */
#define NOT_VALID_DIFF_CNS  ((GT_32)(BIT_30))


/**
* @internal pringSegmentTypeRangeInfo function
* @endinternal
*
* @brief   helper function to print range in prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[]
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] segmentType              - the type of segment
* @param[in] startRangeLogicalIndex   - the logical index that starts the range
* @param[in] lastInRangeLogicalIndex  - the logical index that is last in the range (inclusive)
* @param[in] diffToOldLogicalIndex    - the diff between new logical indexes to old logical indexes
*                                      (zero or negative or positive)
*                                       None
*/
static void pringSegmentTypeRangeInfo(
    IN HW_INDEX_SEGMENT_TYPE_ENT segmentType,
    IN GT_U32                    startRangeLogicalIndex,
    IN GT_U32                    lastInRangeLogicalIndex,
    IN GT_32                     diffToOldLogicalIndex
)
{
    GT_U32  rangeSize = (lastInRangeLogicalIndex + 1) - startRangeLogicalIndex;
    if(segmentType == HW_INDEX_SEGMENT_TYPE_EXISTING_E)
    {
        /* the segment hold different values for old and new with specific offset between them */
        cpssOsPrintf("existing segment: new logical index[%4.4d..%4.4d] with diff[%d] (to old logical index) ([%d] indexes)\n",
            startRangeLogicalIndex,
            lastInRangeLogicalIndex,
            diffToOldLogicalIndex,
            rangeSize);
    }
    else
    if(segmentType == HW_INDEX_SEGMENT_TYPE_ADDED_E)
    {
        /* the segment was 'added' (no 'old') */
        cpssOsPrintf("added segment:    new logical index[%4.4d..%4.4d] (no old logical index)([%d] indexes)\n",
            startRangeLogicalIndex,
            lastInRangeLogicalIndex,
            rangeSize);
    }
    else  /*HW_INDEX_SEGMENT_TYPE_REMOVED_E*/
    {
        /* the segment was 'removed' (no 'new') */
        cpssOsPrintf("removed segment:  old logical index[%4.4d..%4.4d] (no new logical index)([%d] indexes)\n",
            startRangeLogicalIndex,
            lastInRangeLogicalIndex,
            rangeSize);
    }
}

/**
* @internal print_resizeHwIndexDbArr function
* @endinternal
*
* @brief   print the prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[] for specific vtcam
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
* @param[in] ruleSize                 - the rule size of the entries in the vtcam.
*                                       None
*/
void print_resizeHwIndexDbArr(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC* tcamSegCfgPtr,
    IN CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_ENT         ruleSize
)
{
    GT_U32  ii;
    GT_32 currDiffOldAndNew = NOT_VALID_DIFF_CNS;/* current diff between old and new */
    GT_32 prevDiffOldAndNew = NOT_VALID_DIFF_CNS;/* previous diff between old and new */
    HW_INDEX_SEGMENT_TYPE_ENT currSegmentType;
    HW_INDEX_SEGMENT_TYPE_ENT prevSegmentType = HW_INDEX_SEGMENT_TYPE_EXISTING_E;
    GT_U32  stepSize;
    GT_U32  startRangeLogicalIndex=0;
    GT_U32  inRangeLogicalIndex=0;
    GT_BOOL startRange = GT_TRUE;
    GT_U32  indexInRange;
    GT_BOOL currentInfoPrinted = GT_TRUE;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_RESIZE_HW_INDEX_MAP_INFO_STC    *prvCpssDxChVirtualTcamDbResizeHwIndexDbArr;
    GT_U32  prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr;

    prvCpssDxChVirtualTcamDbResizeHwIndexDbArr =
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr);
    prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr =
        VTCAM_GLOVAR(prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr);

    stepSize = prvCpssDxChVirtualTcamSegmentTableRuleSizeToAlignment(
        tcamSegCfgPtr, ruleSize);
    if (stepSize == 0)
    {
        cpssOsPrintf("prvCpssDxChVirtualTcamDbResizeHwIndexDbArr wrong rule size \n");
        return;
    }

    if(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr == NULL)
    {
        cpssOsPrintf("prvCpssDxChVirtualTcamDbResizeHwIndexDbArr not exists \n");
        return;
    }

    cpssOsPrintf("prvCpssDxChVirtualTcamDbResizeHwIndexDbArr: \n");
    for(ii = 0 ; ii < prvCpssDxChVirtualTcamDbNumEntries_resizeHwIndexDbArr ; ii += stepSize)
    {
        if ((tcamSegCfgPtr->deviceClass == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_DEV_CLASS_SIP6_10_E)
            && (ruleSize == CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_40_B_E)
            && (((ii / stepSize) % 3) == 1))
        {
            /* 40 byte rule aligned to 0 in even blocks and to 8 in odd blocks                 */
            /* 40 byte rule aligned to 4 supported by CPSS, but not suppoerted by VTCAM library*/
            continue;
        }

        if((prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].oldLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS) &&
           (prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].newLogicalIndex == PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS))
        {
            /* this is not indication that the range (of logical indexes) ended !
              (because we probably got HW indexes in non full floor  */
            continue;
        }

        if((prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].oldLogicalIndex != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS) &&
           (prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].newLogicalIndex != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS))
        {
            currSegmentType = HW_INDEX_SEGMENT_TYPE_EXISTING_E;

            currDiffOldAndNew = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].newLogicalIndex -
                                prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].oldLogicalIndex;

            indexInRange = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].newLogicalIndex;
        }
        else
        if(prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].oldLogicalIndex != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS)
        {
            currSegmentType = HW_INDEX_SEGMENT_TYPE_REMOVED_E;
            currDiffOldAndNew = 0;
            indexInRange = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].oldLogicalIndex;
        }
        else  /* newLogicalIndex != PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_HW_INDEX_NOT_USED_CNS */
        {
            currSegmentType = HW_INDEX_SEGMENT_TYPE_ADDED_E;
            currDiffOldAndNew = 0;
            indexInRange = prvCpssDxChVirtualTcamDbResizeHwIndexDbArr[ii].newLogicalIndex;
        }

        if(startRange == GT_TRUE)
        {
            /* save the start of range info */
            startRange = GT_FALSE;

            if(currentInfoPrinted == GT_FALSE)
            {
                /* save the start of range info */
                startRangeLogicalIndex = indexInRange;
            }
            else
            {
                currentInfoPrinted = GT_FALSE;
            }
        }
        else
        if(prevSegmentType   != currSegmentType ||
           prevDiffOldAndNew != currDiffOldAndNew )
        {
            /* end of range */
            /* print the range info */
            pringSegmentTypeRangeInfo(prevSegmentType,
                startRangeLogicalIndex,
                inRangeLogicalIndex,
                prevDiffOldAndNew);
            /* indicate that the range ended */
            startRange = GT_TRUE;
            currentInfoPrinted = GT_TRUE;
            /* save the start of range info */
            startRangeLogicalIndex = indexInRange;
        }

        /* update 'last' index in the range */
        inRangeLogicalIndex = indexInRange;

        prevDiffOldAndNew = currDiffOldAndNew;
        prevSegmentType = currSegmentType;
    }

    if(currentInfoPrinted == GT_FALSE)
    {
        /* print the last range info */
        pringSegmentTypeRangeInfo(prevSegmentType,
            startRangeLogicalIndex,
            inRangeLogicalIndex,
            prevDiffOldAndNew);
    }

    cpssOsPrintf("prvCpssDxChVirtualTcamDbResizeHwIndexDbArr: ended \n");
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDump function
* @endinternal
*
* @brief   Dump DB info about a vTcam in vTcam manager.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDump
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam */

    cpssOsPrintf("\n"); /* make sure the printings will start in new line */

    /* get's a pointer to the DB of the vTcam */
    vTcamCfgPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if (vTcamCfgPtr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtr is NULL");
        return;
    }
    printTcamInfo(vTcamCfgPtr->tcamInfo); /* printing tcamInfo structure */

    cpssOsPrintf("rulesAmount = %d\n", vTcamCfgPtr->rulesAmount); /* printing rules amount */

    cpssOsPrintf("usedRulesAmount = %d\n", vTcamCfgPtr->usedRulesAmount); /* printing used rules amount */

    printUsedRulesBitmapArr(vTcamCfgPtr->usedRulesBitmapArr,vTcamCfgPtr->rulesAmount,vTcamCfgPtr->usedRulesAmount); /* printing used usedRulesBitmapArr */

    printRulePhysicalIndexArr(vTcamCfgPtr->rulePhysicalIndexArr,vTcamCfgPtr->rulesAmount); /* printing rulePhysicalIndexArr */

    printSegmentsTree(vTcamCfgPtr->segmentsTree); /* printing segmentsTree */

    printRuleIdTree(vTcamCfgPtr->ruleIdIdTree,"ruleIdIdTree"); /* printing ruleIdIdTree */

    printRuleIdTree(vTcamCfgPtr->ruleIdIndexTree,"ruleIdIndexTree"); /* printing ruleIdIndexTree */

    printPriorityTree(vTcamCfgPtr->priorityPriTree,"priorityPriTree"); /* printing priorityPriTree */

    printPriorityTree(vTcamCfgPtr->priorityIndexTree,"priorityIndexTree"); /* printing priorityIndexTree */
}

GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintPriorityTree
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr
)
{
    printPriorityTree(vTcamCfgPtr->priorityIndexTree,"priorityIndexTree");
}
GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintRuleIdTree
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr
)
{
    printRuleIdTree(vTcamCfgPtr->ruleIdIndexTree,"ruleIdIndexTree"); /* printing ruleIdIndexTree */
}
GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintRulesLogIndexesBitmap
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr
)
{
    printUsedRulesBitmapArr(
        vTcamCfgPtr->usedRulesBitmapArr,vTcamCfgPtr->rulesAmount,vTcamCfgPtr->usedRulesAmount);
}

GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintPriorityTreeById
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam */

    /* get's a pointer to the DB of the vTcam */
    vTcamCfgPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if (vTcamCfgPtr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtr is NULL");
        return;
    }
    printPriorityTree(vTcamCfgPtr->priorityIndexTree,"priorityIndexTree");
}

GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintRuleIdTreeById
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam */

    /* get's a pointer to the DB of the vTcam */
    vTcamCfgPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if (vTcamCfgPtr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtr is NULL");
        return;
    }
    printRuleIdTree(vTcamCfgPtr->ruleIdIndexTree,"ruleIdIndexTree"); /* printing ruleIdIndexTree */
}

GT_VOID prvCpssDxChVirtualTcamDbVTcamPrintRulesLogIndexesBitmapById
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam */

    /* get's a pointer to the DB of the vTcam */
    vTcamCfgPtr = prvCpssDxChVirtualTcamDbVTcamGet(vTcamMngId,vTcamId);
    if (vTcamCfgPtr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtr is NULL");
        return;
    }
    printUsedRulesBitmapArr(
        vTcamCfgPtr->usedRulesBitmapArr,vTcamCfgPtr->rulesAmount,vTcamCfgPtr->usedRulesAmount);
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr function
* @endinternal
*
* @brief   Dump block free space.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*                                      (usefull for dummy vTCAM Manager that has no Id)
* @param[in] perLookupOnly            - GT_TRUE  - amounts per lookup only
*                                      - GT_FALSE - amounts per block and per lookup
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC* tcamSegCfgPtr,
    IN  GT_BOOL                                 perLookupOnly
)
{
    GT_U32                                        blockIdx;
    GT_U32                                        *blockFreeSpaceArrPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_ENT columnsType;
    GT_U32                                        sumArr[
        PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E];
    GT_U32                                        startBlockIdx;
    GT_U32                                        lookupId;
    GT_U32                                        sumReservedColumns;

    cpssOsPrintf("\n"); /* make sure the printings will start in new line */

    if (tcamSegCfgPtr == NULL)
    {
        cpssOsPrintf("vTcamMngPtr == NULL\n");
        return;
    }

    if (perLookupOnly == GT_FALSE)
    {
        cpssOsPrintf("block ");
        for (columnsType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
              (columnsType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
              columnsType++)
        {
            cpssOsPrintf("\t %s", prvCpssDxChVirtualTcamDbSegmentColumnsEnumNames[columnsType]);
        }

        for (blockIdx = 0; (blockIdx < tcamSegCfgPtr->blocksAmount); blockIdx++)
        {
            blockFreeSpaceArrPtr = &(tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[0]);
            cpssOsPrintf("\n");
            cpssOsPrintf("%d ", blockIdx);
            for (columnsType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
                  (columnsType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
                  columnsType++)
            {
                cpssOsPrintf("\t\t %d", blockFreeSpaceArrPtr[columnsType]);
            }
            cpssOsPrintf("\t\t reservedColumns %d\n", tcamSegCfgPtr->blockInfArr[blockIdx].reservedColumnsSpace);
        }
        cpssOsPrintf("\n");
    }

    cpssOsPrintf("sums per lookup \n");
    cpssOsPrintf("lookupId ");
    for (columnsType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
          (columnsType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
          columnsType++)
    {
        cpssOsPrintf("\t %s", prvCpssDxChVirtualTcamDbSegmentColumnsEnumNames[columnsType]);
    }

    for (startBlockIdx = 0; (startBlockIdx < tcamSegCfgPtr->blocksAmount); startBlockIdx++)
    {
        lookupId = tcamSegCfgPtr->blockInfArr[startBlockIdx].lookupId;

        /* bypass all not first from the same lookupId value owners */
        for (blockIdx = 0; (blockIdx < startBlockIdx); blockIdx++)
        {
            if (tcamSegCfgPtr->blockInfArr[blockIdx].lookupId == lookupId) break;
        }
        /* found block with the less index and the same lookup iD */
        if (blockIdx < startBlockIdx) continue;

        /* clear sums array */
        cpssOsMemSet(sumArr, 0, sizeof(sumArr));
        sumReservedColumns = 0;
        /* summing over all blocks with the same lookup */
        for (blockIdx = startBlockIdx; (blockIdx < tcamSegCfgPtr->blocksAmount); blockIdx++)
        {
            if (tcamSegCfgPtr->blockInfArr[blockIdx].lookupId != lookupId) continue;

            blockFreeSpaceArrPtr = &(tcamSegCfgPtr->blockInfArr[blockIdx].freeSpace[0]);

            for (columnsType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
                  (columnsType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
                  columnsType++)
            {
                sumArr[columnsType] += blockFreeSpaceArrPtr[columnsType];
            }
            sumReservedColumns += tcamSegCfgPtr->blockInfArr[blockIdx].reservedColumnsSpace;
        }
        cpssOsPrintf("\n");
        cpssOsPrintf("%d ", lookupId);
        for (columnsType = PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_NONE_E;
              (columnsType < PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_COLUMNS_LAST_E);
              columnsType++)
        {
            cpssOsPrintf("\t\t %d", sumArr[columnsType]);
        }
        cpssOsPrintf("\t\t reservedColumns %d ", sumReservedColumns);
    }
    cpssOsPrintf("\n");
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpace function
* @endinternal
*
* @brief   Dump block free space.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] perLookupOnly            - GT_TRUE  - amounts per lookup only
*                                      - GT_FALSE - amounts per block and per lookup
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpace
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_BOOL                                     perLookupOnly
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*     vTcamMngPtr;

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if (vTcamMngPtr == NULL)
    {
        cpssOsPrintf("vTcamMngPtr is NULL, vTcamMngId is %d\n", vTcamMngId);
        return;
    }

    prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
        &vTcamMngPtr->tcamSegCfg, perLookupOnly);

    prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
        &vTcamMngPtr->tcamTtiSegCfg, perLookupOnly);

    prvCpssDxChVirtualTcamDbVTcamDumpBlockFreeSpaceByPtr(
        &vTcamMngPtr->pclTcam1SegCfg, perLookupOnly);
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpByFieldByPtr function
* @endinternal
*
* @brief   Dump DB info about a vTcam in vTcam manager of specific field.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngPtr              - (pointer to)Virtual TCAM Manager structure
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] fieldNamePtr             - name of the required field
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpByFieldByPtr
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*         vTcamMngPtr,
    IN  GT_U32                                      vTcamId,
    IN  GT_CHAR_PTR                                 fieldNamePtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam         */

    cpssOsPrintf("\n"); /* make sure the printings will start in new line */

    /* dump of data per vTCAM Manager */

    if (cpssOsStrCmp(fieldNamePtr,"segFreePrefTree") == 0)
    {
        printSegmentsTree(vTcamMngPtr->tcamSegCfg.segFreePrefTree); /* printing segFreePrefTree */
        printSegmentsTree(vTcamMngPtr->tcamTtiSegCfg.segFreePrefTree); /* printing segFreePrefTree */
        printSegmentsTree(vTcamMngPtr->pclTcam1SegCfg.segFreePrefTree); /* printing segFreePrefTree */
        return;
    }
    if (cpssOsStrCmp(fieldNamePtr,"segFreeLuTree") == 0)
    {
        printSegmentsTree(vTcamMngPtr->tcamSegCfg.segFreeLuTree); /* printing segFreeLuTree */
        printSegmentsTree(vTcamMngPtr->tcamTtiSegCfg.segFreeLuTree); /* printing segFreeLuTree */
        printSegmentsTree(vTcamMngPtr->pclTcam1SegCfg.segFreeLuTree); /* printing segFreeLuTree */
        return;
    }
    if (cpssOsStrCmp(fieldNamePtr,"segmentsPoolUsage") == 0)
    {
        cpssBmPoolStatsPrint(vTcamMngPtr->segmentsPool);
        return;
    }

    /* dump of data per vTCAM */

    /* get's a pointer to the DB of the vTcam */
    if (vTcamMngPtr->vTcamCfgPtrArr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtrArr is NULL (all array not allocated)\n");
        return;
    }
    if(vTcamId >= vTcamMngPtr->vTcamCfgPtrArrSize)
    {
        cpssOsPrintf("vTcamId out of range\n");
        return;
    }
    vTcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];
    if (vTcamCfgPtr == NULL)
    {
        cpssOsPrintf("vTcamCfgPtr is NULL\n");
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"tcamInfo") == 0)
    {
        printTcamInfo(vTcamCfgPtr->tcamInfo); /* printing tcamInfo structure */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"rulesAmount") == 0)
    {
        cpssOsPrintf("rulesAmount = %d\n", vTcamCfgPtr->rulesAmount); /* printing rules amount */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"usedRulesAmount") == 0)
    {
        cpssOsPrintf("usedRulesAmount = %d\n", vTcamCfgPtr->usedRulesAmount); /* printing used rules amount */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"usedRulesBitmapArr") == 0) {
        printUsedRulesBitmapArr(vTcamCfgPtr->usedRulesBitmapArr,vTcamCfgPtr->rulesAmount,vTcamCfgPtr->usedRulesAmount); /* printing used usedRulesBitmapArr */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"rulePhysicalIndexArr") == 0) {
        printRulePhysicalIndexArr(vTcamCfgPtr->rulePhysicalIndexArr,vTcamCfgPtr->rulesAmount); /* printing rulePhysicalIndexArr */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"segmentsTree") == 0)
    {
        printSegmentsTree(vTcamCfgPtr->segmentsTree); /* printing segmentsTree */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"ruleIdIdTree") == 0)
    {
        printRuleIdTree(vTcamCfgPtr->ruleIdIdTree, "ruleIdIdTree"); /* printing ruleIdIdTree */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"ruleIdIndexTree") == 0)
    {
        printRuleIdTree(vTcamCfgPtr->ruleIdIndexTree, "ruleIdIndexTree"); /* printing ruleIdIndexTree */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"priorityPriTree") == 0)
    {
        printPriorityTree(vTcamCfgPtr->priorityPriTree, "priorityPriTree"); /* printing priorityPriTree */
        return;
    }

    if (cpssOsStrCmp(fieldNamePtr,"priorityIndexTree") == 0)
    {
        printPriorityTree(vTcamCfgPtr->priorityIndexTree, "priorityIndexTree"); /* printing priorityIndexTree */
        return;
    }

    if(cpssOsStrCmp(fieldNamePtr,"prvCpssDxChVirtualTcamDbResizeHwIndexDbArr") == 0)
    {

        print_resizeHwIndexDbArr(vTcamCfgPtr->tcamSegCfgPtr, vTcamCfgPtr->tcamInfo.ruleSize);
        return;
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpByField function
* @endinternal
*
* @brief   Dump DB info about a vTcam in vTcam manager of specific field.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] vTcamId                  - unique Id of  Virtual TCAM
* @param[in] fieldNamePtr             - name of the required field
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpByField
(
    IN  GT_U32                                      vTcamMngId,
    IN  GT_U32                                      vTcamId,
    IN  GT_CHAR_PTR                                 fieldNamePtr
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr; /* pointer to vTcam Manager */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if (vTcamMngPtr == NULL)
    {
        cpssOsPrintf("vTcamMngPtr is NULL, vTcamMngId is %d\n", vTcamMngId);
        return;
    }
    prvCpssDxChVirtualTcamDbVTcamDumpByFieldByPtr(vTcamMngPtr, vTcamId, fieldNamePtr);
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpFullTcamPriorityModeData function
* @endinternal
*
* @brief   Dump all Trees Created in Priority Mode vTCAMs
*
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpFullTcamPriorityModeData ()
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr;
    GT_U32                                        vTcamMngId = 0, vTcamId = 0;
    GT_BOOL                                       haVtcamDebug;

    haVtcamDebug = VTCAM_GLOVAR(haVtcamDebug);
    if(!haVtcamDebug)
    {
        return;
    }

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    for (vTcamId = 0; (vTcamId < vTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
    {
        if (!vTcamMngPtr->vTcamCfgPtrArr[vTcamId]) continue;

        cpssOsPrintf("\nPriority Mode DS - vTcamId = %d\n", vTcamId);
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId, vTcamId, "ruleIdIdTree");
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId, vTcamId, "ruleIdIndexTree");
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId, vTcamId, "priorityPriTree");
        prvCpssDxChVirtualTcamDbVTcamDumpByField(vTcamMngId, vTcamId, "priorityIndexTree");
    }
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr function
* @endinternal
*
* @brief   Dump DB info about full TCAM layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] tcamSegCfgPtr            - (pointer to) segment configuration information.
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr
(
    IN  PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_SEG_TAB_CFG_STC*   tcamSegCfgPtr
)
{
    GT_STATUS                                     rc;          /* return code              */
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_VTCAM_CFG_STC*  vTcamCfgPtr; /* pointer to vTcam         */
    GT_U32                                        vTcamId;     /* vTcamId                  */
    CPSS_BM_POOL_ID                               nodesPool;   /* pool for tree nodes      */
    PRV_CPSS_AVL_TREE_ID                          segLuTree;   /* tree of all segments     */
    GT_BOOL                                       seeked;      /* seeked                   */
    GT_VOID                                       *dbEntryPtr; /* DB Entry pointer         */
    PRV_CPSS_AVL_TREE_PATH                        path;        /* tree iterator            */
    GT_U32                                        space;       /* space                    */
    GT_U32                                        totalSpace;  /* total space              */

    cpssOsPrintf("\n"); /* make sure the printings will start in new line */

    if (tcamSegCfgPtr == NULL)
    {
        cpssOsPrintf("tcamSegCfgPtr == NULL\n");
        return;
    }

    if (tcamSegCfgPtr->maxSegmentsInDb == 0)
    {
        cpssOsPrintf("tcamSegCfgPtr->maxSegmentsInDb == 0\n");
        return;
    }

    /* pool for tree for all busy and free TCAM segments */
    rc = prvCpssAvlMemPoolCreate(
        tcamSegCfgPtr->maxSegmentsInDb,
        &nodesPool);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Temporary tree nodes pool allocation failed\n");
        return;
    }
    rc = prvCpssAvlTreeCreate(
        (GT_INTFUNCPTR)prvCpssDxChVirtualTcamDbSegmentTableLookupOrderCompareFunc,
        nodesPool,&segLuTree);
    if (rc != GT_OK)
    {
        cpssOsPrintf("Temporary tree creation failed\n");
        cpssBmPoolReCreate(nodesPool);
        prvCpssAvlMemPoolDelete(nodesPool);
        return;
    }

    cpssOsPrintf("Id lookup rule_size r_total r_used space\n");

    vTcamMngPtr = tcamSegCfgPtr->pVTcamMngPtr;
    totalSpace = 0;
    for (vTcamId = 0; (vTcamId < vTcamMngPtr->vTcamCfgPtrArrSize); vTcamId++)
    {
        vTcamCfgPtr = vTcamMngPtr->vTcamCfgPtrArr[vTcamId];
        if (vTcamCfgPtr == NULL) continue;
        if (vTcamCfgPtr->tcamSegCfgPtr != tcamSegCfgPtr) continue;
        space = (vTcamCfgPtr->rulesAmount * (vTcamCfgPtr->tcamInfo.ruleSize + 1));
        totalSpace += space;
        cpssOsPrintf(
            "%4.4d  %4.4d  %4.4d  %4.4d  %4.4d  %4.4d\n",
            vTcamId,
            PRV_CPSS_DXCH_VIRTUAL_TCAM_DB_LOOKUP_ID_MAC(
                vTcamCfgPtr->tcamInfo.clientGroup, vTcamCfgPtr->tcamInfo.hitNumber),
            (vTcamCfgPtr->tcamInfo.ruleSize + 1),
            vTcamCfgPtr->rulesAmount, vTcamCfgPtr->usedRulesAmount,
            space);
        /* add all vTcam used segments to segments tree */
        seeked = prvCpssAvlPathSeek(
                vTcamCfgPtr->segmentsTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
                path, &dbEntryPtr);
        while (seeked != GT_FALSE)
        {
            rc = prvCpssAvlItemInsert(segLuTree, dbEntryPtr);
            if (rc != GT_OK)
            {
                cpssOsPrintf("Segment adding to temporary tree failed - will not be printed\n");
            }
            seeked = prvCpssAvlPathSeek(
                    vTcamCfgPtr->segmentsTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                    path, &dbEntryPtr);
        }
    }

    /* add all free segments to segments tree */
    seeked = prvCpssAvlPathSeek(
            tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_FIRST_E,
            path, &dbEntryPtr);
    while (seeked != GT_FALSE)
    {
        rc = prvCpssAvlItemInsert(segLuTree, dbEntryPtr);
        if (rc != GT_OK)
        {
            cpssOsPrintf("Segment adding to temporary tree failed - will not be printed\n");
        }
        seeked = prvCpssAvlPathSeek(
                tcamSegCfgPtr->segFreeLuTree, PRV_CPSS_AVL_TREE_SEEK_NEXT_E,
                path, &dbEntryPtr);
    }

    cpssOsPrintf("Total Space used by vTcams %d\n", totalSpace);

    /* dump temporary table with all segments */
    cpssOsPrintf("All TCAM Segments (busy and free)\n");
    printSegmentsTree(segLuTree);

    /* cleanup */
    prvCpssAvlTreeDelete(
        segLuTree, (GT_VOIDFUNCPTR)NULL, (GT_VOID*)NULL);
    cpssBmPoolReCreate(nodesPool);
    prvCpssAvlMemPoolDelete(nodesPool);
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayout function
* @endinternal
*
* @brief   Dump DB info about full TCAM layout.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                       None.
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayout
(
    IN  GT_U32                                      vTcamMngId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC*           vTcamMngPtr; /* pointer to vTcam Manager */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if (vTcamMngPtr == NULL)
    {
        cpssOsPrintf("vTcamMngPtr is NULL, vTcamMngId is %d\n", vTcamMngId);
        return;
    }

    prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamSegCfg);
    prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->tcamTtiSegCfg);
    prvCpssDxChVirtualTcamDbVTcamDumpFullTcamLayoutByPtr(&vTcamMngPtr->pclTcam1SegCfg);
}

/**
* @internal prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree function
* @endinternal
*
* @brief   print the avl segments tree..
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] treeIdPtr                - a pointer to the id of the segments tree
*                                       None
*/
GT_VOID prvCpssDxChVirtualTcamDbVTcamDumpSegmentsTree
(
    IN PRV_CPSS_AVL_TREE_ID treeIdPtr
)
{
    printSegmentsTree(treeIdPtr);
}

/***************************************************************************/

/**
* @internal prvCpssDxChVirtualTcamRuleMoveAndInvalidateCountersReset function
* @endinternal
*
* @brief   Reset counter of rule moves and rule invalidates.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval - none
*/
void prvCpssDxChVirtualTcamRuleMoveAndInvalidateCountersReset
(
    IN   GT_U32                                 vTcamMngId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC* vTcamMngPtr; /* pointer to vTcam Manager */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if(vTcamMngPtr == NULL)
    {
        cpssOsPrintf("TCAM manager id [%d] was not found", vTcamMngId);
        return;
    }
    vTcamMngPtr->ruleMoveCounter       = 0;
    vTcamMngPtr->ruleInvalidateCounter = 0;
}

/**
* @internal prvCpssDxChVirtualTcamRuleMoveAndInvalidateCountersPrint function
* @endinternal
*
* @brief   Print counter of rule moves and rule invalidates.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval - none
*/
void prvCpssDxChVirtualTcamRuleMoveAndInvalidateCountersPrint
(
    IN   GT_U32                                 vTcamMngId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC* vTcamMngPtr; /* pointer to vTcam Manager */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if(vTcamMngPtr == NULL)
    {
        cpssOsPrintf("TCAM manager id [%d] was not found", vTcamMngId);
        return;
    }
    cpssOsPrintf(
        "vTcamRuleMoveCounter %d vTcamRuleInvalidateCounter %d\n",
        vTcamMngPtr->ruleMoveCounter, vTcamMngPtr->ruleInvalidateCounter);
}

/**
* @internal prvCpssDxChVirtualTcamStartTimeInitialize function
* @endinternal
*
* @brief   Initialize start time before measuring time.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval - none
*/
void prvCpssDxChVirtualTcamStartTimeInitialize
(
    IN   GT_U32                                 vTcamMngId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC* vTcamMngPtr; /* pointer to vTcam Manager */

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if(vTcamMngPtr == NULL)
    {
        cpssOsPrintf("TCAM manager id [%d] was not found", vTcamMngId);
        return;
    }
    cpssOsTimeRT(&(vTcamMngPtr->startSeconds), &(vTcamMngPtr->startNanoSeconds));
}

/**
* @internal prvCpssDxChVirtualTcamPassedTimePrint function
* @endinternal
*
* @brief   Print time passed from start time.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId               - Virtual TCAM Manager Id
*                                      (APPLICABLE RANGES: 0..31)
*
* @retval - none
*/
void prvCpssDxChVirtualTcamPassedTimePrint
(
    IN   GT_U32                                 vTcamMngId
)
{
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC* vTcamMngPtr; /* pointer to vTcam Manager */
    GT_U32 endSeconds;
    GT_U32 endNanoSeconds;
    GT_U32 elapsedSeconds;
    GT_U32 elapsedNanoSeconds;

    vTcamMngPtr = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngId);
    if(vTcamMngPtr == NULL)
    {
        cpssOsPrintf("TCAM manager id [%d] was not found", vTcamMngId);
        return;
    }
    cpssOsTimeRT(&endSeconds, &endNanoSeconds);
    prvCpssTimeSecondsAndNanoDiff(
        vTcamMngPtr->startSeconds, vTcamMngPtr->startNanoSeconds,
        endSeconds, endNanoSeconds,
        &elapsedSeconds, &elapsedNanoSeconds);

    cpssOsPrintf(
        "Elapsed Seconds %d Elapsed Nano-Seconds %d\n",
        elapsedSeconds, elapsedNanoSeconds);
}


/**
* @internal prvCpssDxChVirtualTcamHaHwDbLpmTableEntriesIndexAndOffsetGet function
* @endinternal
*
* @brief   Calculate Start point of 44-bit DB record inside LPM Memory 32-bit words.
* @brief   The LPM memory fulfilled from the end of the memory to the start.       
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in]  devNum                   - the device number
* @param[in]  rulePhysicalIndex        - TCAM Rule Physical Index 
* @param[out] lpmLineIndexPtr          - (pointer to) LPM Memory Line Index
* @param[out] lpmLineBitOffsetPtr      - (pointer to) LPM Memory Line Bit Offset
*
* @retval GT_OK                    - on success, others on fail
*
*/
static GT_STATUS prvCpssDxChVirtualTcamHaHwDbLpmTableEntriesIndexAndOffsetGet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   rulePhysicalIndex,
    OUT GT_U32                                   *lpmLineIndexPtr,
    OUT GT_U32                                   *lpmLineBitOffsetPtr
)
{
    GT_U32 numOfBitsInBlock;
    GT_U32 numOfDbEntriesInBlock;
    GT_U32 blockIndexFromEnd;
    GT_U32 dbEntryInBlockFromEnd;
    GT_U32 lpmLineInBlockFromEnd;
    GT_U32 lpmLineBitOffset;
    GT_U32 blockIndex;
    GT_U32 lpmLineInBlock;

    numOfBitsInBlock = 
        (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock * PRV_CPSS_DXCH_LPM_LINE_BITS_CNS);
    numOfDbEntriesInBlock = (numOfBitsInBlock / PRV_CPSS_DXCH_VTCAM_HA_HW_DB_ENTRY_BITS_CNS);
    blockIndexFromEnd = (rulePhysicalIndex / numOfDbEntriesInBlock);
    if (blockIndexFromEnd >= PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    dbEntryInBlockFromEnd = (rulePhysicalIndex % numOfDbEntriesInBlock);
    /* line that contains start of indexed entry - include size of the entry */
    lpmLineInBlockFromEnd = 
        (((dbEntryInBlockFromEnd + 1) * PRV_CPSS_DXCH_VTCAM_HA_HW_DB_ENTRY_BITS_CNS)
         + PRV_CPSS_DXCH_LPM_LINE_BITS_CNS - 1)
        / PRV_CPSS_DXCH_LPM_LINE_BITS_CNS;
    lpmLineBitOffset = 
        ((lpmLineInBlockFromEnd * PRV_CPSS_DXCH_LPM_LINE_BITS_CNS)
        - ((dbEntryInBlockFromEnd + 1) * PRV_CPSS_DXCH_VTCAM_HA_HW_DB_ENTRY_BITS_CNS));
    blockIndex = 
        (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks - blockIndexFromEnd - 1);
    lpmLineInBlock = 
        (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock - lpmLineInBlockFromEnd);
    *lpmLineIndexPtr     = 
        (blockIndex * PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap)
        + lpmLineInBlock;
    *lpmLineBitOffsetPtr = lpmLineBitOffset;
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    GT_U32    portGroupId;
    GT_U32    lpmLineIndex;
    GT_U32    lpmLineBitOffset;
    GT_U32    remainedBits;
    GT_U32    dataArr[2];
    GT_U32    dataIndex;
    GT_U32    dataOffset;
    GT_U32    hwField;
    GT_U32    fieldLen;
    GT_U32    subFieldLen0;
    GT_U32    subFieldLen1;
    GT_U32    subFieldBase;
    GT_U32    mask;
    
    rc = prvCpssDxChVirtualTcamHaHwDbLpmTableEntriesIndexAndOffsetGet(
        devNum, ruleHwIndex, &lpmLineIndex, &lpmLineBitOffset);
    if (rc != GT_OK)
    {
        return rc;
    }

    remainedBits = PRV_CPSS_DXCH_VTCAM_HA_HW_DB_ENTRY_BITS_CNS;
    dataArr[0] = ruleId;
    dataArr[1] = vTcamId;

    while (remainedBits > 0)
    {
        fieldLen = PRV_CPSS_DXCH_LPM_LINE_BITS_CNS - lpmLineBitOffset;
        if (fieldLen > remainedBits)
        {
            fieldLen = remainedBits;
        }
        subFieldBase = PRV_CPSS_DXCH_VTCAM_HA_HW_DB_ENTRY_BITS_CNS - remainedBits;
        subFieldLen0  = fieldLen;
        subFieldLen1  = 0;
        dataOffset = subFieldBase % 32;
        dataIndex  = subFieldBase / 32;
        if (subFieldLen0 > (32 - dataOffset))
        {
            subFieldLen0 = (32 - dataOffset);
            subFieldLen1  = (fieldLen - subFieldLen0);
        }
        mask = (subFieldLen0 >= 32) ? (GT_U32)0xFFFFFFFF : (GT_U32)((1 << subFieldLen0) - 1);
        hwField = (dataArr[dataIndex] >> dataOffset) & mask;
        if (subFieldLen1 > 0)
        {
            mask = (subFieldLen1 >= 32) ? (GT_U32)0xFFFFFFFF : (GT_U32)((1 << subFieldLen1) - 1);
            hwField |= ((dataArr[dataIndex + 1]) & mask) << subFieldLen0;
        }

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)
        {
            rc = prvCpssDxChPortGroupWriteTableEntryField(
                devNum,
                portGroupId,
                CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                lpmLineIndex,
                0 /*fieldWordNum*/,
                lpmLineBitOffset,
                fieldLen,
                hwField);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum, portGroupsBmp, portGroupId)

        remainedBits -= fieldLen;
        /* code below relevant (and correct) only if remainedBits > 0 yet */
        lpmLineIndex ++;
        lpmLineBitOffset = 0;
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    GT_U32    portGroupId;
    GT_U32    lpmLineIndex;
    GT_U32    lpmLineBitOffset;
    GT_U32    remainedBits;
    GT_U32    dataArr[2];
    GT_U32    dataIndex;
    GT_U32    dataOffset;
    GT_U32    hwField;
    GT_U32    fieldLen;
    GT_U32    subFieldLen0;
    GT_U32    subFieldLen1;
    GT_U32    subFieldBase;
    GT_U32    mask;
    
    rc = prvCpssDxChVirtualTcamHaHwDbLpmTableEntriesIndexAndOffsetGet(
        devNum, ruleHwIndex, &lpmLineIndex, &lpmLineBitOffset);
    if (rc != GT_OK)
    {
        return rc;
    }

    dataArr[0] = 0;
    dataArr[1] = 0;
    remainedBits = PRV_CPSS_DXCH_VTCAM_HA_HW_DB_ENTRY_BITS_CNS;
    while (remainedBits > 0)
    {
        fieldLen = PRV_CPSS_DXCH_LPM_LINE_BITS_CNS - lpmLineBitOffset;
        if (fieldLen > remainedBits)
        {
            fieldLen = remainedBits;
        }
        /* Get the first active port group */
        PRV_CPSS_MULTI_PORT_GROUPS_BMP_GET_FIRST_ACTIVE_MAC(
            devNum, portGroupsBmp, portGroupId);
        rc = prvCpssDxChPortGroupReadTableEntryField(
            devNum,
            portGroupId,
            CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
            lpmLineIndex,
            0 /*fieldWordNum*/,
            lpmLineBitOffset,
            fieldLen,
            &hwField);
        if (rc != GT_OK)
        {
            return rc;
        }
        subFieldBase = PRV_CPSS_DXCH_VTCAM_HA_HW_DB_ENTRY_BITS_CNS - remainedBits;
        subFieldLen0  = fieldLen;
        subFieldLen1  = 0;
        dataOffset = subFieldBase % 32;
        dataIndex  = subFieldBase / 32;
        if (subFieldLen0 > (32 - dataOffset))
        {
            subFieldLen0 = (32 - dataOffset);
            subFieldLen1  = (fieldLen - subFieldLen0);
        }

        mask = (subFieldLen0 >= 32) ? (GT_U32)0xFFFFFFFF : (GT_U32)((1 << subFieldLen0) - 1);
        dataArr[dataIndex] |= ((hwField & mask) << dataOffset);
        if (subFieldLen1 > 0)
        {
            mask = (subFieldLen1 >= 32) ? (GT_U32)0xFFFFFFFF : (GT_U32)((1 << subFieldLen1) - 1);
            dataArr[dataIndex + 1] |= (((hwField >> subFieldLen0) & mask));
        }

        remainedBits -= fieldLen;
        /* code below relevant (and correct) only if remainedBits > 0 yet */
        lpmLineIndex ++;
        lpmLineBitOffset = 0;
    }

    *ruleIdPtr  = dataArr[0];
    *vTcamIdPtr = dataArr[1];
    return GT_OK;
}

#define PRV_VTCAM_HA_HW_DB_ACCESS_DEBUG_MAC
#ifdef PRV_VTCAM_HA_HW_DB_ACCESS_DEBUG_MAC

/* write-fill and read-check function */
static GT_STATUS prvCpssDxChVirtualTcamHaHwDbAccessDebugWriteAndCheck
(
    IN   GT_U8               devNum,
    IN   GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN   GT_U32              numOfEntries,
    IN   GT_U32              vTcamIdBase,
    IN   GT_U32              vTcamIdIncrement,
    IN   GT_U32              ruleIdBase,
    IN   GT_U32              ruleIdIncrement
)
{
    GT_STATUS               rc;
    GT_U32                  vTcamId;
    GT_U32                  ruleId;
    GT_U32                  vTcamIdGet;
    GT_U32                  ruleIdGet;
    GT_U32                  entryIndex;

    cpssOsPrintf(
        "prvCpssDxChVirtualTcamHaHwDbAccessDebugWriteAndCheck dev %d portGroupsBmp 0x%08X \n",
        devNum, portGroupsBmp);
    cpssOsPrintf(
        "numOfEntries %d vTcamIdBase 0x%08X vTcamIdIncrement 0x%08X ruleIdBase 0x%08X ruleIdIncrement 0x%08X\n",
        numOfEntries, vTcamIdBase, vTcamIdIncrement, ruleIdBase, ruleIdIncrement);

    vTcamId = vTcamIdBase & 0xFFF;
    ruleId  = ruleIdBase;
    for (entryIndex = 0; (entryIndex < numOfEntries); entryIndex++)
    {
        rc = prvCpssDxChVirtualTcamHaHwDbLpmWrite(
                devNum, portGroupsBmp, entryIndex, vTcamId, ruleId);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamHaHwDbLpmWrite failed rc %d entryIndex %d\n",
                rc, entryIndex);
            return rc;
        }

        vTcamId += vTcamIdIncrement;
        ruleId  += ruleIdIncrement;
        vTcamId &= 0xFFF;
    }

    vTcamId = vTcamIdBase & 0xFFF;
    ruleId  = ruleIdBase;
    for (entryIndex = 0; (entryIndex < numOfEntries); entryIndex++)
    {
        rc = prvCpssDxChVirtualTcamHaHwDbLpmRead(
                devNum, portGroupsBmp, entryIndex, &vTcamIdGet, &ruleIdGet);
        if (rc != GT_OK)
        {
            cpssOsPrintf(
                "prvCpssDxChVirtualTcamHaHwDbLpmRead failed rc %d entryIndex %d\n",
                rc, entryIndex);
            return rc;
        }

        if ((vTcamIdGet != vTcamId) || (ruleIdGet != ruleId))
        {
            cpssOsPrintf(
                "Read values different from written entryIndex %d\n",
                entryIndex);
            cpssOsPrintf(
                "vTcamIdGet 0x%08X vTcamId 0x%08X ruleIdGet 0x%08X ruleId 0x%08X\n",
                vTcamIdGet, vTcamId, ruleIdGet, ruleId);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        vTcamId += vTcamIdIncrement;
        ruleId  += ruleIdIncrement;
        vTcamId &= 0xFFF;
    }

    cpssOsPrintf("All values written and read as expected\n");
    return GT_OK;
}

/* write-fill and read-check test */
GT_STATUS prvCpssDxChVirtualTcamHaHwDbAccessDebugTest
(
    IN   GT_U8               devNum,
    IN   GT_PORT_GROUPS_BMP  portGroupsBmp
)
{
    GT_STATUS           rc;
    GT_U32              numOfEntries;
    GT_U32              numOfBitsInBlock;
    GT_U32              numOfDbEntriesInBlock;

    numOfBitsInBlock = 
        (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock * PRV_CPSS_DXCH_LPM_LINE_BITS_CNS);
    numOfDbEntriesInBlock = (numOfBitsInBlock / PRV_CPSS_DXCH_VTCAM_HA_HW_DB_ENTRY_BITS_CNS);
    numOfEntries =
         (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfBlocks - 2) * numOfDbEntriesInBlock;

    rc = prvCpssDxChVirtualTcamHaHwDbAccessDebugWriteAndCheck(
        devNum, portGroupsBmp, numOfEntries,
        0x555/*vTcamIdBase*/,
        0/*vTcamIdIncrement*/,
        0xAAAAAAAA/*ruleIdBase*/,
        0/*ruleIdIncrement*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChVirtualTcamHaHwDbAccessDebugWriteAndCheck(
        devNum, portGroupsBmp, numOfEntries,
        0xFFF/*vTcamIdBase*/,
        0/*vTcamIdIncrement*/,
        0x55555555/*ruleIdBase*/,
        0/*ruleIdIncrement*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChVirtualTcamHaHwDbAccessDebugWriteAndCheck(
        devNum, portGroupsBmp, numOfEntries,
        0/*vTcamIdBase*/,
        1/*vTcamIdIncrement*/,
        0/*ruleIdBase*/,
        1/*ruleIdIncrement*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChVirtualTcamHaHwDbAccessDebugWriteAndCheck(
        devNum, portGroupsBmp, numOfEntries,
        0xFFF/*vTcamIdBase*/,
        0xFFFFFFFF/*vTcamIdIncrement*/,
        0xFFFFFFFF/*ruleIdBase*/,
        0xFFFFFFFF/*ruleIdIncrement*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}
#endif /*PRV_VTCAM_HA_HW_DB_ACCESS_DEBUG_MAC*/

