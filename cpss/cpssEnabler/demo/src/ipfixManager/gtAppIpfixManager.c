/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file gtAppIpfixManager.c
*
* @brief Application Code for IPFIX Manager
*
* @version   1
*********************************************************************************
**/

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <ipfixManager/prvAppIpfixManager.h>
#include <ipfixManager/prvAppIpfixManagerDbg.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/prvCpssDxChIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/cpssDxChIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManagerSamples.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>

PRV_APP_IPFIX_MGR_DB_STC *appIpfixManagerDbPtr;

static void prvAppDemoIpfixManagerEventNotifyFunc
(
    IN CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT *ipcMsgPtr
);

static GT_STATUS prvAppDemoIpfixManagerExactMatchActionSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              egressPort,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT        *actionPtr,
    IN GT_U32                                   flowId,
    IN CPSS_PACKET_CMD_ENT                      packetCmd
);

extern void dxChNetIfRxPacketParse_DebugDumpEnable
(
    IN GT_BOOL  enableDumpRxPacket
);

/**
 * @internal appDemoIpfixManagerDbCreate function
 * @endinternal
 *
 * @brief  Create IPFIX application database and initialize parameters
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerDbCreate
(
    GT_U8           devNum
)
{
    GT_STATUS                            rc;
    CPSS_DXCH_NET_DSA_PARAMS_STC         dsaInfo;
    GT_HW_DEV_NUM                        hwDevNum;
    GT_U32                               i;

    /* Allocate memory for IPFIX Manager application database */
    appIpfixManagerDbPtr = (PRV_APP_IPFIX_MGR_DB_STC *) cpssOsMalloc(sizeof(PRV_APP_IPFIX_MGR_DB_STC));

    /* Initialize Parameters */
    cpssOsMemSet(appIpfixManagerDbPtr, 0, sizeof(PRV_APP_IPFIX_MGR_DB_STC));

    appIpfixManagerDbPtr->devNum = devNum;

    /* config params initialization */
    appIpfixManagerDbPtr->firstPktsQueueNum = PRV_APP_IPFIX_MGR_FIRST_PKTS_QUEUE_NUM_CNS;
    appIpfixManagerDbPtr->attributes.serviceCpuNum = PRV_APP_IPFIX_MGR_SERVICE_CPU_NUM_CNS;
    appIpfixManagerDbPtr->globalCfg.activeTimeout = PRV_APP_IPFIX_MGR_ACTIVE_TIMEOUT_CNS; /* in seconds */
    appIpfixManagerDbPtr->globalCfg.ipfixEntriesPerFlow = CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E;
    appIpfixManagerDbPtr->globalCfg.dataPktMtu = PRV_APP_IPFIX_MGR_DATA_PKTS_MTU_CNS;
    appIpfixManagerDbPtr->globalCfg.idleTimeout = PRV_APP_IPFIX_MGR_IDLE_TIMEOUT_CNS;
    appIpfixManagerDbPtr->globalCfg.ipfixDataQueueNum = PRV_APP_IPFIX_MGR_DATA_PKTS_QUEUE_NUM_CNS;

    /* Threads params initialization */
    appIpfixManagerDbPtr->newFlows.burstSize = PRV_APP_IPFIX_MGR_NEW_FLOWS_BURST_SIZE_CNS;
    appIpfixManagerDbPtr->dataPkts.burstSize = PRV_APP_IPFIX_MGR_DATA_PKTS_BURST_SIZE_CNS;
    appIpfixManagerDbPtr->ipcFetch.burstSize = PRV_APP_IPFIX_MGR_IPC_EVENTS_BURST_SIZE_CNS;
    appIpfixManagerDbPtr->agingEnable = GT_FALSE;
    appIpfixManagerDbPtr->debugPrintsEnable = GT_TRUE;
    appIpfixManagerDbPtr->debugCountersEnable = GT_TRUE;

    for (i=0; i<CPSS_DXCH_TILES_MAX_CNS; i++)
    {
        appIpfixManagerDbPtr->exactMatchManagerId[i] = i*2;
    }

    appIpfixManagerDbPtr->testParams.waitTimeForExportComplete = PRV_APP_IPFIX_MGR_EXPORT_COMPLETE_WAIT_TIME;
    appIpfixManagerDbPtr->testParams.newFlowsPollInterval = PRV_APP_IPFIX_MGR_NEW_FLOWS_SLEEP_TIME_CNS;
    appIpfixManagerDbPtr->testParams.dataPktsPollInterval = PRV_APP_IPFIX_MGR_DATA_PKTS_SLEEP_TIME_CNS;
    appIpfixManagerDbPtr->testParams.ipcFetchPollInterval = PRV_APP_IPFIX_MGR_IPC_EVENTS_SLEEP_TIME_CNS;
    appIpfixManagerDbPtr->testParams.emm.expandedActionIndex = PRV_APP_IPFIX_MGR_EM_EXPANDED_ACTION_IDX_CNS;
    appIpfixManagerDbPtr->testParams.emm.lookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
    appIpfixManagerDbPtr->testParams.emm.actionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
    appIpfixManagerDbPtr->testParams.pcl.direction      = CPSS_PCL_DIRECTION_INGRESS_E;
    appIpfixManagerDbPtr->testParams.pcl.lookupNum      = CPSS_PCL_LOOKUP_NUMBER_0_E;
    appIpfixManagerDbPtr->testParams.pcl.packetType     = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
    appIpfixManagerDbPtr->testParams.pcl.ruleFormatType = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
    appIpfixManagerDbPtr->testParams.newFlowsSdmaQNum = (GT_U8)(PRV_APP_IPFIX_MGR_FIRST_PKTS_QUEUE_NUM_CNS & 0x7);
    appIpfixManagerDbPtr->testParams.dataPktsSdmaQNum = (GT_U8)(PRV_APP_IPFIX_MGR_DATA_PKTS_QUEUE_NUM_CNS & 0x7);
    appIpfixManagerDbPtr->testParams.newFlowsCpuCode = PRV_APP_IPFIX_MGR_FIRST_PKTS_CPU_CODE;
    appIpfixManagerDbPtr->testParams.dataPktsCpuCode = PRV_APP_IPFIX_MGR_DATA_PKTS_CPU_CODE;
    appIpfixManagerDbPtr->testParams.idleTimeout        = PRV_APP_IPFIX_MGR_ENTRY_IDLE_TIMEOUT_CNS;
    appIpfixManagerDbPtr->testParams.activeTimeout      = PRV_APP_IPFIX_MGR_ENTRY_ACTIVE_TIMEOUT_CNS;

    prvAppDemoIpfixManagerExactMatchActionSet(devNum,
                                              appIpfixManagerDbPtr->testParams.egressPort,
                                              appIpfixManagerDbPtr->testParams.emm.actionType,
                                              &appIpfixManagerDbPtr->testParams.emm.action,
                                              0,
                                              CPSS_PACKET_CMD_FORWARD_E);

    cpssOsMemSet(appIpfixManagerDbPtr->testParams.emm.keyMask, 0xFF,
                 CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS);

    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Build Extended TO_CPU DSA tag; 4 words */
    cpssOsMemSet(&dsaInfo,0,sizeof(dsaInfo));
    dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
    dsaInfo.dsaInfo.toCpu.hwDevNum = (hwDevNum + 1) % 32;
    dsaInfo.dsaInfo.toCpu.cpuCode = CPSS_DXCH_IPFIX_MANAGER_DATA_PKTS_CPU_CODE_DEFAULT;

    /* build DSA Tag buffer */
    rc = cpssDxChNetIfDsaTagBuild(devNum,
        &dsaInfo,
        &appIpfixManagerDbPtr->globalCfg.txDsaTag[0]);

    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChNetIfDsaTagBuild failed rc = %d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixManagerDbDelete function
 * @endinternal
 *
 * @brief  Delete IPFIX application database
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerDbDelete
(
    GT_U8           devNum
)
{

    (void) devNum;

    if(appIpfixManagerDbPtr)
    {
        cpssOsFree(appIpfixManagerDbPtr);
    }

    return GT_OK;
}

/**
* @internal prvAppDemoIpfixManagerExactMatchActionSet function
* @endinternal
*
* @brief   This routine set default values to an exact match
*          Action
*
* @param[in] type                - action type (TTI/PCL/EPCL)
* @param[in] actionPtr           - (pointer to) action
* @param[in] flowId              - flow identifier
* @param[in] packetCmd           - packet command
*/
static GT_STATUS prvAppDemoIpfixManagerExactMatchActionSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              egressPort,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT        *actionPtr,
    IN GT_U32                                   flowId,
    IN CPSS_PACKET_CMD_ENT                      packetCmd
)
{
    GT_STATUS rc;
    GT_HW_DEV_NUM      hwDevNum = 0;
    cpssOsBzero((GT_VOID*) actionPtr, sizeof(CPSS_DXCH_EXACT_MATCH_ACTION_UNT));

    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChExactMatchManagerCreate failed: rc=%d\n", rc);
        return rc;
    }

    switch(actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        actionPtr->ttiAction.ttPassengerPacketType              = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
        actionPtr->ttiAction.tsPassengerPacketType              = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
        actionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable = GT_TRUE;
        actionPtr->ttiAction.command                            = CPSS_PACKET_CMD_FORWARD_E;

        actionPtr->ttiAction.redirectCommand                    = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
        actionPtr->ttiAction.egressInterface.type               = CPSS_INTERFACE_PORT_E;
        actionPtr->ttiAction.egressInterface.devPort.hwDevNum   = hwDevNum;
        actionPtr->ttiAction.egressInterface.devPort.portNum    = egressPort;
        actionPtr->ttiAction.egressInterface.trunkId            = egressPort;

        actionPtr->ttiAction.pcl0OverrideConfigIndex            = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
        actionPtr->ttiAction.pcl0_1OverrideConfigIndex          = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;

        actionPtr->ttiAction.iPclUdbConfigTableEnable           = GT_FALSE;
        actionPtr->ttiAction.iPclUdbConfigTableIndex            = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;

        actionPtr->ttiAction.mirrorToIngressAnalyzerEnable      = GT_FALSE;
        actionPtr->ttiAction.mirrorToIngressAnalyzerIndex       = 0;
        actionPtr->ttiAction.userDefinedCpuCode                 = CPSS_NET_FIRST_USER_DEFINED_E + 1;

        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:

        actionPtr->pclAction.pktCmd       = packetCmd;
        actionPtr->pclAction.mirror.cpuCode = appIpfixManagerDbPtr->testParams.newFlowsCpuCode;
        actionPtr->pclAction.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

        actionPtr->pclAction.redirect.data.outIf.outInterface.type   = CPSS_INTERFACE_PORT_E;
        actionPtr->pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum   = hwDevNum;
        actionPtr->pclAction.redirect.data.outIf.outInterface.devPort.portNum    = egressPort;
        actionPtr->pclAction.vlan.ingress.vlanId     = PRV_APP_IPFIX_MGR_VLAN_ID_CNS;
        actionPtr->pclAction.bypassBridge = GT_TRUE;

        if(actionType==CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E)
        {
            actionPtr->pclAction.egressPolicy = GT_TRUE;
        }
        else
        {
            actionPtr->pclAction.egressPolicy = GT_FALSE;
        }

        actionPtr->pclAction.flowId = flowId;

        actionPtr->pclAction.qos.ingress.modifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
/*        actionPtr->pclAction.policer.policerEnable  = CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
        actionPtr->pclAction.policer.policerId      = flowId;*/

        break;
    default:
        break;
    }
    return GT_OK;
}

/**
 * @internal appDemoIpfixManagerEmmInit function
 * @endinternal
 *
 * @brief  Initialize Exact Match Maanger for IPFIX manager
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerEmmInit
(
    IN GT_U8           devNum
)
{
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            emmCapacity;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              emmLookup;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    emmEntryAttr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               emmAging;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC pairListArr[1];
    GT_U32                                                numOfPairs = 1;
    GT_U32                                                emmProfileEntryIndex = PRV_APP_IPFIX_MGR_EM_PROFILE_IDX_CNS;
    GT_U32                                                expandedActionIndex;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT      expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT                 actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                      actionData;
    GT_U32                                                i = 0;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT                      lookupNum;

    cpssOsMemSet(&emmEntryAttr, 0, sizeof(emmEntryAttr));
    cpssOsMemSet(&emmLookup, 0, sizeof(emmLookup));
    cpssOsMemSet(&emmCapacity, 0, sizeof(emmCapacity));
    cpssOsMemSet(&emmAging, 0, sizeof(emmAging));
    cpssOsMemSet(&pairListArr, 0, sizeof(pairListArr[1]));
    cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
    cpssOsMemSet(&actionData, 0, sizeof(actionData));

    expandedActionIndex = appIpfixManagerDbPtr->testParams.emm.expandedActionIndex;
    lookupNum           = appIpfixManagerDbPtr->testParams.emm.lookupNum;

    /* Create IPFIX manager */
    emmCapacity.hwCapacity.numOfHwIndexes = PRV_APP_IPFIX_MGR_HW_INDEXES_MAX_CNS;
    emmCapacity.hwCapacity.numOfHashes    = PRV_APP_IPFIX_MGR_HW_HASHES_MAX_CNS;
    emmCapacity.maxTotalEntries           = PRV_APP_IPFIX_MGR_EMM_ENTRIES_MAX_CNS;
    emmCapacity.maxEntriesPerAgingScan    = PRV_APP_IPFIX_MGR_MAX_NUM_ENTRIES_STEPS_CNS;
    emmCapacity.maxEntriesPerDeleteScan   = PRV_APP_IPFIX_MGR_MAX_NUM_ENTRIES_STEPS_CNS;

    actionType = appIpfixManagerDbPtr->testParams.emm.actionType;
    emmLookup.lookupsArray[lookupNum].lookupEnable=GT_TRUE;
    emmLookup.lookupsArray[lookupNum].lookupClient=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;
    emmLookup.lookupsArray[lookupNum].lookupClientMappingsNum=1;
    emmLookup.lookupsArray[lookupNum].lookupClientMappingsArray[0].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
    emmLookup.lookupsArray[lookupNum].lookupClientMappingsArray[0].pclMappingElem.subProfileId = 0;
    emmLookup.lookupsArray[lookupNum].lookupClientMappingsArray[0].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    emmLookup.lookupsArray[lookupNum].lookupClientMappingsArray[0].pclMappingElem.profileId = emmProfileEntryIndex;

    emmLookup.profileEntryParamsArray[emmProfileEntryIndex].keyParams.keySize  = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
    emmLookup.profileEntryParamsArray[emmProfileEntryIndex].keyParams.keyStart = 0;
    emmLookup.profileEntryParamsArray[emmProfileEntryIndex].defaultActionType = actionType;
    emmLookup.profileEntryParamsArray[emmProfileEntryIndex].defaultActionEn = GT_TRUE;
    prvAppDemoIpfixManagerExactMatchActionSet(devNum,
                                              appIpfixManagerDbPtr->testParams.egressPort,
                                              actionType,
                                              &emmLookup.profileEntryParamsArray[emmProfileEntryIndex].defaultAction,
                                              0 /* flowId */, CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
    for (i=0; i<14; i++)
    {
        emmLookup.profileEntryParamsArray[emmProfileEntryIndex].keyParams.mask[i] = 0xFF;
    }

    /* Ignore port number */
    if (appIpfixManagerDbPtr->globalCfg.ipfixEntriesPerFlow ==
                  CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E)
    {
        emmLookup.profileEntryParamsArray[emmProfileEntryIndex].keyParams.mask[4] = 0x0;
    }

    cpssOsMemCpy(appIpfixManagerDbPtr->testParams.emm.keyMask,
                 emmLookup.profileEntryParamsArray[emmProfileEntryIndex].keyParams.mask,
                 CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS);

    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex = GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable = GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId = GT_TRUE;
    prvAppDemoIpfixManagerExactMatchActionSet(devNum,
                                              appIpfixManagerDbPtr->testParams.egressPort,
                                              actionType,
                                              &actionData,
                                              0 /* flowId */, CPSS_PACKET_CMD_FORWARD_E);

    emmEntryAttr.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid=GT_TRUE;
    emmEntryAttr.expandedArray[expandedActionIndex].expandedActionType=actionType;
    cpssOsMemCpy(&emmEntryAttr.expandedArray[expandedActionIndex].expandedAction,&actionData,sizeof(actionData));
    cpssOsMemCpy(&emmEntryAttr.expandedArray[expandedActionIndex].expandedActionOrigin.pclExpandedActionOrigin,
                 &expandedActionOriginData, sizeof(expandedActionOriginData));

    emmAging.agingRefreshEnable = GT_FALSE;

    for (i=0; i<CPSS_DXCH_TILES_MAX_CNS; i++)
    {
        rc = cpssDxChExactMatchManagerCreate(appIpfixManagerDbPtr->exactMatchManagerId[i],
                                             &emmCapacity, &emmLookup,
                                             &emmEntryAttr, &emmAging);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChExactMatchManagerCreate failed: rc=%d\n", rc);
            return rc;
        }

        /* Add device list */
        pairListArr[0].devNum        = devNum;
        pairListArr[0].portGroupsBmp = (1 << (i*2));
        numOfPairs = 1;
        rc = cpssDxChExactMatchManagerDevListAdd(appIpfixManagerDbPtr->exactMatchManagerId[i],
                                                 pairListArr, numOfPairs);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChExactMatchManagerDevListAdd failed: rc=%d\n", rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixManagerEmmRestore function
 * @endinternal
 *
 * @brief  Restore Exact Match Maanger configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerEmmRestore
(
    IN  GT_U8           devNum
)
{
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC pairListArr[1];
    GT_U32    numOfPairs = 1;
    GT_U32    i;


    for (i=0; i<CPSS_DXCH_TILES_MAX_CNS; i++)
    {
        /* Add device list */
        pairListArr[0].devNum        = devNum;
        pairListArr[0].portGroupsBmp = (1 << (i*2));
        numOfPairs = 1;
        rc = cpssDxChExactMatchManagerDevListRemove(appIpfixManagerDbPtr->exactMatchManagerId[i],
                                                    pairListArr, numOfPairs);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChExactMatchManagerDevListRemove failed: rc=%d\n", rc);
            return rc;
        }

        rc = cpssDxChExactMatchManagerDelete(appIpfixManagerDbPtr->exactMatchManagerId[i]);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChExactMatchManagerDelete failed: rc=%d\n", rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixManagerTxInit function
 * @endinternal
 *
 * @brief  Initialize/Configure TX queue configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerTxInit
(
    IN      GT_U8           devNum
)
{
    GT_STATUS                              rc = GT_OK;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC cpuCodeEntry;
    CPSS_NET_RX_CPU_CODE_ENT  dataPktsCpuCode = appIpfixManagerDbPtr->testParams.dataPktsCpuCode;
    CPSS_NET_RX_CPU_CODE_ENT  firstPktsCpuCode = appIpfixManagerDbPtr->testParams.newFlowsCpuCode;

    cpssOsBzero((GT_CHAR*)&cpuCodeEntry, sizeof(cpuCodeEntry));

    /* Trancated packets to CPU */
    rc = cpssDxChNetIfCpuCodeTableGet(devNum, dataPktsCpuCode, &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChNetIfCpuCodeTableGet failed: rc=%d\n", rc);
        return rc;
    }

    /* cpuCodeEntry.truncate = GT_TRUE; */
    cpuCodeEntry.tc = appIpfixManagerDbPtr->testParams.dataPktsSdmaQNum;

    rc = cpssDxChNetIfCpuCodeTableSet(devNum, dataPktsCpuCode, &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChNetIfCpuCodeTableSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Assigning queue num for cpucode received on msg from Fw */
    rc = cpssDxChNetIfCpuCodeTableGet(devNum, firstPktsCpuCode, &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChNetIfCpuCodeTableGet failed: rc=%d\n", rc);
        return rc;
    }

    cpuCodeEntry.tc = appIpfixManagerDbPtr->testParams.newFlowsSdmaQNum;

    rc = cpssDxChNetIfCpuCodeTableSet(devNum, firstPktsCpuCode, &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChNetIfCpuCodeTableSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Bind CPU Port To Scheduler Profile and associate Message packet Queue with SP arbitration
     * group. Message packets are more important than packets for new flow learning and hence
     * assigning message packet queue to SP group for highest priority treatment.
     */
    rc = cpssDxChPortTxBindPortToSchedulerProfileGet(devNum, CPSS_CPU_PORT_NUM_CNS,
                                                     &appIpfixManagerDbPtr->defaultSwitchCfg.profile);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPortTxBindPortToSchedulerProfileGet failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChPortTxBindPortToSchedulerProfileSet(devNum, CPSS_CPU_PORT_NUM_CNS,
                                                     CPSS_PORT_TX_SCHEDULER_PROFILE_5_E);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPortTxBindPortToSchedulerProfileSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChPortTxQArbGroupGet(devNum, appIpfixManagerDbPtr->testParams.newFlowsSdmaQNum,
                                    appIpfixManagerDbPtr->defaultSwitchCfg.profile,
                                    &appIpfixManagerDbPtr->defaultSwitchCfg.arbGroup);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPortTxQArbGroupGet failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChPortTxQArbGroupSet(devNum, appIpfixManagerDbPtr->testParams.newFlowsSdmaQNum,
                                    CPSS_PORT_TX_SP_ARB_GROUP_E,
                                    CPSS_PORT_TX_SCHEDULER_PROFILE_5_E);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPortTxQArbGroupSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Disabling the below events generated by trapped packets
     */
    rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_RX_BUFFER_QUEUE0_E +
                                appIpfixManagerDbPtr->testParams.newFlowsSdmaQNum,
                                CPSS_EVENT_MASK_E);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssEventDeviceMaskSet FAILED, rc [%d] devNum [%d] uniEv %d\n",
                     __FUNCNAME__, __LINE__, rc, devNum, CPSS_PP_RX_BUFFER_QUEUE6_E);
        return rc;
    }

    rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_RX_ERR_QUEUE0_E +
                                appIpfixManagerDbPtr->testParams.newFlowsSdmaQNum,
                                CPSS_EVENT_MASK_E);
    if (GT_OK != rc)
    {
        cpssOsPrintf("[%s:%d] cpssEventDeviceMaskSet FAILED, rc [%d] devNum [%d] uniEv %d\n",
                     __FUNCNAME__, __LINE__, rc, devNum, CPSS_PP_RX_ERR_QUEUE6_E);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixManagerTxRestore function
 * @endinternal
 *
 * @brief  Restore TX queue configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerTxRestore
(
    IN      GT_U8           devNum
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC cpuCodeEntry;
    CPSS_NET_RX_CPU_CODE_ENT  dataPktsCpuCode = appIpfixManagerDbPtr->testParams.dataPktsCpuCode;
    CPSS_NET_RX_CPU_CODE_ENT  firstPktsCpuCode = appIpfixManagerDbPtr->testParams.newFlowsCpuCode;

    /* Trancated packets to CPU */
    rc = cpssDxChNetIfCpuCodeTableGet(devNum,
                                      dataPktsCpuCode,
                                      &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChNetIfCpuCodeTableGet failed: rc=%d\n", rc);
        return rc;
    }

    cpuCodeEntry.truncate = GT_FALSE;
    cpuCodeEntry.tc = 0;

    rc = cpssDxChNetIfCpuCodeTableSet(devNum,
                                      dataPktsCpuCode,
                                      &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChNetIfCpuCodeTableSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Assigning queue num for cpucode received on msg from Fw */
    rc = cpssDxChNetIfCpuCodeTableGet(devNum,
                                      firstPktsCpuCode,
                                      &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChNetIfCpuCodeTableGet failed: rc=%d\n", rc);
        return rc;
    }

    cpuCodeEntry.tc = 0;

    rc = cpssDxChNetIfCpuCodeTableSet(devNum,
                                      firstPktsCpuCode,
                                      &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChNetIfCpuCodeTableSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChPortTxBindPortToSchedulerProfileSet(devNum, CPSS_CPU_PORT_NUM_CNS,
                                                     appIpfixManagerDbPtr->defaultSwitchCfg.profile);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPortTxBindPortToSchedulerProfileSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChPortTxQArbGroupSet(devNum, appIpfixManagerDbPtr->testParams.newFlowsSdmaQNum,
                                    appIpfixManagerDbPtr->defaultSwitchCfg.arbGroup,
                                    appIpfixManagerDbPtr->defaultSwitchCfg.profile);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPortTxQArbGroupSet failed: rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixManagerPolicerInit function
 * @endinternal
 *
 * @brief  Initialize policer engine for IPFIX manager
 *
 * @param[in] devNum - device number
 * @param[in] policerStage - policer stage ingress/egress
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerPolicerInit
(
    IN  GT_U8                            devNum,
    IN  CPSS_DXCH_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32                                  numPolicerEntries = 0;
    GT_U32                                  ipfixEntryIndex = 0;
    CPSS_DXCH_IPFIX_ENTRY_STC               ipfixCntr;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC      ipfixWaCfg;
    CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_ENT  mode;

    rc = cpssDxChPolicerStageMeterModeSet( devNum, policerStage,
                    CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E);
    if(rc != GT_OK )
    {
        __IPFIX_MANAGER_LOG("cpssDxChPolicerStageMeterModeSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChPolicerCountingModeSet( devNum, policerStage,
                    CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
    if(rc != GT_OK )
    {
        __IPFIX_MANAGER_LOG("cpssDxChPolicerCountingModeSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxCh3PolicerPacketSizeModeSet( devNum, policerStage,
                    CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
    if(rc != GT_OK )
    {
        __IPFIX_MANAGER_LOG("cpssDxCh3PolicerPacketSizeModeSet failed: rc=%d\n", rc);
        return rc;
    }

    cpssOsBzero((GT_CHAR*) &mode, sizeof(mode));
    cpssOsBzero((GT_CHAR*) &ipfixWaCfg, sizeof(ipfixWaCfg));
    cpssOsBzero((GT_CHAR*) &ipfixCntr, sizeof(ipfixCntr));

    rc = cpssDxChPolicerMemorySizeModeGet(devNum, &mode, &numPolicerEntries, NULL, NULL);
    if(rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPolicerMemorySizeModeGet failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChPolicerMemorySizeModeSet(devNum, mode, numPolicerEntries, numPolicerEntries);
    if(rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPolicerMemorySizeModeSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChIpfixDropCountModeSet(devNum, policerStage,
                                       CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E);
    if(rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixDropCountModeSet failed: rc=%d\n", rc);
        return rc;
    }

    ipfixWaCfg.action = CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E;
    ipfixWaCfg.byteThreshold.l[0] = 0xFFFFFFFF;
    ipfixWaCfg.byteThreshold.l[1] = 0;
    ipfixWaCfg.packetThreshold = BIT_30 - 1;
    ipfixWaCfg.dropThreshold = BIT_30 - 1;
    rc = cpssDxChIpfixWraparoundConfSet(devNum, policerStage, &ipfixWaCfg);
    if(rc != GT_OK )
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixWraparoundConfSet failed: rc=%d\n", rc);
        return rc;
    }

    /* WBC flush to ensure the mode is set to billing, even in scaled environment */
    rc = cpssDxChPolicerCountingWriteBackCacheFlush(devNum, policerStage);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPolicerCountingWriteBackCacheFlush failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChIpfixTimestampToCpuEnableSet(devNum, GT_TRUE);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPolicerCountingWriteBackCacheFlush failed: rc=%d\n", rc);
        return rc;
    }

    for(ipfixEntryIndex = 0; ipfixEntryIndex < numPolicerEntries; ipfixEntryIndex++)
    {
        /* Configuring sampling parameters to ipfix entry index */
        cpssOsMemSet(&ipfixCntr, 0, sizeof(ipfixCntr));
        rc = cpssDxChIpfixEntrySet(devNum,
                                   policerStage,
                                   ipfixEntryIndex,
                                   &ipfixCntr);
        if (GT_OK != rc)
        {
            __IPFIX_MANAGER_LOG("cpssDxChIpfixEntrySet failed: rc=%d\n", rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixManagerPolicerRestore function
 * @endinternal
 *
 * @brief  Restore policer configuration
 *
 * @param[in] devNum - device number
 * @param[in] policerStage - policer stage ingress/egress
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerPolicerRestore
(
    GT_U8                            devNum,
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT policerStage
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC      ipfixWaCfg;

    rc = cpssDxChPolicerCountingModeSet( devNum, policerStage,
                    CPSS_DXCH_POLICER_COUNTING_DISABLE_E);
    if(rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPolicerCountingModeSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChIpfixDropCountModeSet(devNum, policerStage,
                                       CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E);
    if(rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixDropCountModeSet failed: rc=%d\n", rc);
        return rc;
    }

    ipfixWaCfg.action = CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_NONE_E;
    ipfixWaCfg.byteThreshold.l[0] = 0x100000;
    ipfixWaCfg.byteThreshold.l[1] = 0xF;
    ipfixWaCfg.packetThreshold = 0x1000;
    ipfixWaCfg.dropThreshold = 0x3FFFFFFF;
    rc = cpssDxChIpfixWraparoundConfSet(devNum, policerStage, &ipfixWaCfg);
    if(rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixWraparoundConfSet failed: rc=%d\n", rc);
        return rc;
    }


    /* WBC flush to ensure the mode is set to billing, even in scaled environment */
    rc = cpssDxChPolicerCountingWriteBackCacheFlush(devNum, policerStage);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPolicerCountingWriteBackCacheFlush failed: rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal prvAppDemoIpfixManagerIpv4UdpFlowConfig function
 * @endinternal
 *
 * @brief  Configure UDB Key for IPv4 UDP key
 *
 * @param[in] devNum - device number
 * @param[in] pclDirection - enumerator for ingress/egress pcl direction
 * @param[in] lookupnum    - lookup number
 *
 * @retval GT_OK - on success
 */
/* Function to be used to enable all key fields for Ipv4-UDP pkt type */
static GT_STATUS prvAppDemoIpfixManagerIpv4UdpFlowConfig
(
    IN  GT_U8                       devNum,
    IN  CPSS_PCL_DIRECTION_ENT      pclDirection,
    IN  CPSS_PCL_LOOKUP_NUMBER_ENT  lookupNum
)
{
    GT_STATUS                      rc = GT_OK;
    GT_U32                         udbIndex = 0;
    GT_U8                          offset = 0; /* UDB offset from anchor */
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT      packetType;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormatType;

    packetType   = appIpfixManagerDbPtr->testParams.pcl.packetType;
    ruleFormatType = appIpfixManagerDbPtr->testParams.pcl.ruleFormatType;

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    /* UDB[0-3] offset for SIPv4 */
    for(udbIndex = 0, offset = 14 ; udbIndex < 4; udbIndex++,offset++)
    {
        rc = cpssDxChPclUserDefinedByteSet(devNum,
                                           ruleFormatType,
                                           packetType,
                                           pclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                           offset);
        if (GT_OK != rc)
        {
            __IPFIX_MANAGER_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
            return rc;
        }
    }

    /* UDB[5-8] offset for DIPv4 */
    for(udbIndex = 5, offset = 18 ; udbIndex < 9; udbIndex++,offset++)
    {
        rc = cpssDxChPclUserDefinedByteSet(devNum,
                                           ruleFormatType,
                                           packetType,
                                           pclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                           offset);
        if (GT_OK != rc)
        {
            __IPFIX_MANAGER_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
            return rc;
        }
    }

    /* UDB[9] TCP/UDP Protocol */
    udbIndex = 9;
    offset = 11;
    rc = cpssDxChPclUserDefinedByteSet(devNum,
                                       ruleFormatType,
                                       packetType,
                                       pclDirection,
                                       udbIndex,
                                       CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                       offset);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
        return rc;
    }

    /* UDB[10-11] offset for UDP Src Port */
    for(udbIndex = 10, offset = 0 ; udbIndex < 12; udbIndex++,offset++)
    {
        rc = cpssDxChPclUserDefinedByteSet(devNum,
                                           ruleFormatType,
                                           packetType,
                                           pclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_L4_E,
                                           offset);
        if (GT_OK != rc)
        {
            __IPFIX_MANAGER_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
            return rc;
        }
    }

    /* UDB[12-13] offset for UDP Dst Port */
    for(udbIndex = 12, offset = 2 ; udbIndex < 14; udbIndex++,offset++)
    {
        rc = cpssDxChPclUserDefinedByteSet(devNum,
                                           ruleFormatType,
                                           packetType,
                                           pclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_L4_E,
                                           offset);
        if (GT_OK != rc)
        {
            __IPFIX_MANAGER_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
            return rc;
        }
    }

    /* UDB[40] offset for Applicable Flow Sub-template */
    udbIndex = 40;
    offset = 91;

    rc = cpssDxChPclUserDefinedByteSet(devNum,
                                       ruleFormatType,
                                       packetType,
                                       pclDirection,
                                       udbIndex,
                                       CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                       offset /* Applicable Flow Sub-template */);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
        return rc;
    }

    /* it's a must to enable UDB's */
    udbSelectStruct.udbSelectArr[0]      = 0; /* SIPv4 */
    udbSelectStruct.udbSelectArr[1]      = 1; /* SIPv4 */
    udbSelectStruct.udbSelectArr[2]      = 2; /* SIPv4 */
    udbSelectStruct.udbSelectArr[3]      = 3; /* SIPv4 */
    if (appIpfixManagerDbPtr->globalCfg.ipfixEntriesPerFlow !=
            CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E)
    {
        udbSelectStruct.ingrUdbReplaceArr[4] = GT_TRUE; /* Source ePort */
    }
    udbSelectStruct.udbSelectArr[5]      = 5; /* DIPv4 */
    udbSelectStruct.udbSelectArr[6]      = 6; /* DIPv4 */
    udbSelectStruct.udbSelectArr[7]      = 7; /* DIPv4 */
    udbSelectStruct.udbSelectArr[8]      = 8; /* DIPv4 */
    udbSelectStruct.udbSelectArr[9]      = 9; /* Protocol TCP/UDP */
    udbSelectStruct.udbSelectArr[10]     = 10; /* UDP Src Port */
    udbSelectStruct.udbSelectArr[11]     = 11; /* UDP Src Port */
    udbSelectStruct.udbSelectArr[12]     = 12; /* UDP Dst Port */
    udbSelectStruct.udbSelectArr[13]     = 13; /* UDP Dst Port */
    udbSelectStruct.udbSelectArr[14]     = 40; /* Applicable Flow Sub-template */

    rc = cpssDxChPclUserDefinedBytesSelectSet(devNum,
                                              ruleFormatType,
                                              packetType,
                                              lookupNum,
                                              &udbSelectStruct);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPclUserDefinedBytesSelectSet failed: rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixManagerPclInit function
 * @endinternal
 *
 * @brief  Initialize PCL and Configure UDB configuration
 *          for PCL Keys
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerPclInit
(
    GT_U8           devNum
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          subLookupNum = 0;
    GT_U32                          portTxqNum = 0;
    GT_PORT_NUM                     portNum = 0;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg;
    GT_U32                          pclId;
    CPSS_PORTS_BMP_STC              portBitmap = {{ 0xFFFFFFFF, 0x87FFFFFF, 0xFFFFFFFF, 0xFEFFFFFF}};

    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    /* PCL init */
    rc = cpssDxChPclInit(devNum);
    if(rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPclInit failed, rc = %d\n", rc);
        return rc;
    }

    /* Enable Ingress Pcl */
    rc = cpssDxChPclIngressPolicyEnable(devNum, GT_TRUE);
    if(rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPclIngressPolicyEnable failed, rc = %d\n", rc);
        return rc;
    }

    subLookupNum = 0;
    for (portNum = 0; portNum < PRV_APP_IPFIX_MGR_MAX_PORT_NUM_CNS; portNum++)
    {
        if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(
                    (&portBitmap), portNum) == 0) continue;

        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum,
                                       PRV_CPSS_DXCH_PORT_TYPE_TXQ_E, &portTxqNum);
        if( rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("prvCpssDxChPortPhysicalPortMapCheckAndConvert failed, portNum = %d, rc = %d\n", portNum, rc);
            continue;
        }

        rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, GT_TRUE);
        if(rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChPclPortIngressPolicyEnable failed, rc = %d\n", rc);
            return rc;
        }

        rc = cpssDxChPclPortLookupCfgTabAccessModeSet(
                        devNum, portNum, CPSS_PCL_DIRECTION_INGRESS_E,
                        CPSS_PCL_LOOKUP_NUMBER_0_E, subLookupNum,
                        CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        if(rc != GT_OK )
        {
            __IPFIX_MANAGER_LOG("cpssDxChPclPortLookupCfgTabAccessModeSet failed, rc = %d\n", rc);
            return rc;
        }

        interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
        interfaceInfo.devPort.hwDevNum   = devNum;
        interfaceInfo.devPort.portNum    = portNum;

        pclId = PRV_APP_IPFIX_MANAGER_PCL_DEFAULT_ID_MAC(CPSS_PCL_DIRECTION_INGRESS_E,
                                                         CPSS_PCL_LOOKUP_0_E, portNum);

        lookupCfg.enableLookup           = GT_TRUE;
        lookupCfg.pclId                  = pclId;
        lookupCfg.groupKeyTypes.nonIpKey = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        lookupCfg.groupKeyTypes.ipv4Key  = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
        lookupCfg.groupKeyTypes.ipv6Key  = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
        lookupCfg.udbKeyBitmapEnable     = GT_FALSE;
        lookupCfg.tcamSegmentMode        = CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_1_TCAM_E;

        rc = cpssDxChPclCfgTblSet(devNum, &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                                  CPSS_PCL_LOOKUP_NUMBER_0_E, &lookupCfg);
        if(rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChPclCfgTblSet failed, rc = %d\n", rc);
            return rc;
        }
    }

    /* Enable IPv4 UDP Flows */
    rc = prvAppDemoIpfixManagerIpv4UdpFlowConfig(devNum,
                                                 appIpfixManagerDbPtr->testParams.pcl.direction,
                                                 appIpfixManagerDbPtr->testParams.pcl.lookupNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("prvAppDemoIpfixManagerIpv4UdpFlowConfig failed: rc=%d\n", rc);
        return rc;
    }

    return rc;
}

/**
 * @internal appDemoIpfixManagerPclRestore function
 * @endinternal
 *
 * @brief  Restore UDB and PCL configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerPclRestore
(
    GT_U8           devNum
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;
    GT_PORT_NUM                     portNum = 0;

        for (portNum = 0; portNum < PRV_APP_IPFIX_MGR_MAX_PORT_NUM_CNS; portNum++)
        {
            rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, GT_FALSE);
            if(rc != GT_OK)
            {
                __IPFIX_MANAGER_LOG("cpssDxChPclPortIngressPolicyEnable failed, rc = %d\n", rc);
                return rc;
            }
        }

        /* Disable Ingress Pcl */
        rc = cpssDxChPclIngressPolicyEnable(devNum, GT_FALSE);
        if(rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChPclIngressPolicyEnable failed, rc = %d\n", rc);
            return rc;
        }

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    /* Clearing UDB config */
    rc = cpssDxChPclUserDefinedBytesSelectSet(devNum,
                                              CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
                                              CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                              CPSS_PCL_LOOKUP_0_E,
                                              &udbSelectStruct);
    if (GT_OK != rc)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPclUserDefinedBytesSelectSet failed: rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal prvAppDemoIpfixManagerFlowKeyGet function
 * @endinternal
 *
 * @brief This function parses the packet recieved and extract the key
 *
 * @param[in]  pktBuffsArr  -  (pointer to) packet Buffer array
 * @param[in]  rxParamsPtr  -  (pointer to) RX parameters
 * @param[out] flowPtr      -  (pointer to) flow structure
 *
 * @retval  GT_OK   - on success
 */
static GT_STATUS prvAppDemoIpfixManagerFlowKeyGet
(
    IN GT_U8                       devNum,
    IN GT_PHYSICAL_PORT_NUM        portNum,
    IN GT_U8    **pktBuffsArr,
    IN GT_U32                      *buffLenArr,
    IN CPSS_DXCH_NET_RX_PARAMS_STC *rxParamsPtr,
    OUT PRV_APP_IPFIX_MGR_FLOWS_STC   *flowPtr
)
{
    GT_U32                          l3minus2Offset = 16;
    GT_U32                          l4Offset = l3minus2Offset + 22;
    GT_U8                           *pktBuff = *pktBuffsArr;
    GT_STATUS                       rc = GT_OK;
    CPSS_DXCH_PCL_UDB_SELECT_STC    udbSelectStruct;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT   offsetType[CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];
    GT_U8                           offset[CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];
    GT_U32                          startOffset = 0;
    GT_U32                          i;
    CPSS_PCL_DIRECTION_ENT             pclDirection;
    CPSS_PCL_LOOKUP_NUMBER_ENT         pclLookupNum;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT      packetType;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormatType;

    (void) portNum;
    pclDirection = appIpfixManagerDbPtr->testParams.pcl.direction;
    pclLookupNum = appIpfixManagerDbPtr->testParams.pcl.lookupNum;
    packetType   = appIpfixManagerDbPtr->testParams.pcl.packetType;
    ruleFormatType = appIpfixManagerDbPtr->testParams.pcl.ruleFormatType;

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    flowPtr->data.firstTs = rxParamsPtr->dsaParam.dsaInfo.toCpu.timestamp;
    flowPtr->data.flowId  = (GT_U16)(rxParamsPtr->dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId &
                                     0xFFFF);
    flowPtr->pktType = APP_IPFIX_MGR_PACKET_TYPE_IPV4_UDP_E;
    flowPtr->key.keySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;

    /* Read from UDB select */
    rc = cpssDxChPclUserDefinedBytesSelectGet(devNum,
                                              ruleFormatType,
                                              packetType,
                                              pclLookupNum,
                                              &udbSelectStruct);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChPclUserDefinedBytesSelectGet failed: rc=%d", rc);
    }
    for(i=0; i<CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS; i++)
    {
        if (pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
        {
            if ((i < CPSS_DXCH_PCL_INGRESS_UDB_REPLACE_MAX_CNS) &&
                (udbSelectStruct.ingrUdbReplaceArr[i] == GT_TRUE))
            {
                continue;
            }
        }
        else
        {
            /* [TBD] pclDirection == Egress not tested*/
        }

        rc = cpssDxChPclUserDefinedByteGet(devNum,
                                           ruleFormatType,
                                           packetType,
                                           pclDirection,
                                           udbSelectStruct.udbSelectArr[i],
                                           &offsetType[i],
                                           &offset[i]);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChPclUserDefinedByteGet failed: rc=%d", rc);
        }
        if (offsetType[i] == CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E)
        {
            startOffset = l3minus2Offset;
        }
        else if (offsetType[i] == CPSS_DXCH_PCL_OFFSET_L4_E)
        {
            startOffset = l4Offset;
        }
        else
        {
            /* [TBD] other offset types not defined yet */
        }

        if ((i < CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS) &&
            (startOffset + offset[i] < buffLenArr[0]))
        {
            flowPtr->key.pattern[i] = (pktBuff[startOffset + offset[i]]) &
                                      (appIpfixManagerDbPtr->testParams.emm.keyMask[i]);
        }
    }

    return GT_OK;
}

/**
 * @internal prvAppDemoIpfixManagerFlowKeySearch function
 * @endinternal
 *
 * @brief  Search if a key is present in the IPFIX maanger database
 *
 * @param[in]  flowPtr     - (pointer to) flow structure
 * @param[out] isFoundPtr  - (pointer to) GT_TRUE - if key is found
 *                      GT_FALSE - if the key is not found.
 *
 * @retval GT_OK    -  on success
 */
static GT_STATUS prvAppDemoIpfixManagerFlowKeySearch
(
    IN  GT_U32                        portGroupId,
    IN  PRV_APP_IPFIX_MGR_FLOWS_STC   *flowPtr,
    OUT GT_BOOL                       *isFoundPtr
)
{
    GT_U32      i = 0;
    GT_32       ret = 0;

    *isFoundPtr = GT_FALSE;

    for (i=0; i<PRV_APP_IPFIX_MGR_FLOWS_PER_PORT_GROUP_MAX_CNS; i++)
    {
        if (appIpfixManagerDbPtr->flowsDb[portGroupId][i].isactive == GT_FALSE)
        {
            continue;
        }
        ret = cpssOsMemCmp(&flowPtr->key,
                     &(appIpfixManagerDbPtr->flowsDb[portGroupId][i].key),
                     sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
        if (ret == 0)
        {
            *isFoundPtr = GT_TRUE;
            break;
        }
    }

    return GT_OK;
}

/**
 * @internal  prvAppDemoIpfixManagerNewFlowsHandle funtion
 * @endinternal
 *
 * @brief  Task to handle first packets of the new flows.
 *         It extracts key and installs entry in the exact match manager
 *         and IPFUX manager
 *
 * @param[in] arg  - input arguments for the task. It includes device number
 *
 * @retval unsigned __TASKCONV
 */
static unsigned __TASKCONV prvAppDemoIpfixManagerNewFlowsHandle
(
    GT_VOID *arg
)
{
    GT_STATUS                                             rc = GT_OK;
    GT_STATUS                                             rc2 = GT_OK;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC               emmEntry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC    emmEntryParams;
    GT_U8                                       devNum = 0;
    GT_U32                                      numOfBuffs = 1;
    GT_U8                                       *packetBuffs[PRV_APP_IPFIX_MGR_PKT_BUFF_LEN];
    GT_U32                                      buffLenArr[PRV_APP_IPFIX_MGR_PKT_BUFF_LEN];
    CPSS_DXCH_NET_RX_PARAMS_STC                 rxParams;
    GT_BOOL                                     isFound;
    PRV_APP_IPFIX_MGR_FLOWS_STC                 flow;
    GT_U32                                      portGroupId = 1;
    GT_PORT_GROUPS_BMP                          portGroupsBmp;
    CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC    entryParams;
    GT_U32                                      newFlowsCount = 0;
    GT_U32                                      flowId = 0;
    CPSS_DXCH_DETAILED_PORT_MAP_STC             portMapShadow;
    GT_U32                                      portNum;
    GT_HW_DEV_NUM                               hwDevNum;
    GT_U32                                      expandedActionIndex;
    GT_U8                                       sdmaQNum;

    devNum = *((GT_U8 *)arg);
    expandedActionIndex = appIpfixManagerDbPtr->testParams.emm.expandedActionIndex;
    sdmaQNum = appIpfixManagerDbPtr->testParams.newFlowsSdmaQNum;

    if (appIpfixManagerDbPtr->newFlows.isTaskActive == GT_TRUE)
    {
        return GT_BAD_STATE;
    }

    appIpfixManagerDbPtr->newFlows.taskTerminate = GT_FALSE;
    appIpfixManagerDbPtr->newFlows.isTaskActive = GT_TRUE;

    newFlowsCount = 0;
    while ((appIpfixManagerDbPtr->allTasksTerminate == GT_FALSE) &&
           (appIpfixManagerDbPtr->newFlows.taskTerminate == GT_FALSE))
    {
        if (rc != GT_OK)
        {
            /* wait for some time and continue */
            cpssOsTimerWkAfter(appIpfixManagerDbPtr->testParams.newFlowsPollInterval);
        }
        /* get the packet from the device */
        if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        {
            numOfBuffs = 1;
            rc = cpssDxChNetIfSdmaRxPacketGet(devNum, sdmaQNum,
                                              &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr, &rxParams);
            if (rc != GT_OK)
            {
                if (rc != GT_NO_MORE)
                {
                    __IPFIX_MANAGER_LOG("cpssDxChNetIfSdmaRxPacketGet failed: rc = %d\n", rc);
                }
                continue;
            }
        }
        else if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
        {
            rc = cpssDxChNetIfMiiRxPacketGet(devNum, sdmaQNum,
                                             &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr,&rxParams);
            if (rc != 0)
            {
                __IPFIX_MANAGER_LOG("cpssDxChNetIfMiiRxPacketGet failed: rc = %d\n", rc);
                continue;
            }
        }
        else
        {
            __IPFIX_MANAGER_LOG("Invalid CPU port mode\n");
            rc = GT_NOT_SUPPORTED;
            break;
        }

        __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(pktsFetchedByHost);

        hwDevNum  = rxParams.dsaParam.dsaInfo.toCpu.hwDevNum;
        portNum = (GT_PHYSICAL_PORT_NUM)rxParams.dsaParam.dsaInfo.toCpu.interface.portNum;

        (void) hwDevNum;

        /* Get Port Group Id from port number */
        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMapShadow);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChPortPhysicalPortDetailedMapGet failed: rc=%d\n", rc);
            continue;
        }
        if(portMapShadow.valid)
        {
            /* global pipeId (not relative to the tileId) */
            rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum,
                               portMapShadow.portMap.macNum/*global MAC port*/,
                               &portGroupId,
                               NULL);/*local MAC*/ 
        }

        if (appIpfixManagerDbPtr->globalCfg.ipfixEntriesPerFlow == 
                CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E)
        {
            portGroupId &= 0x6;
        }

        /* flow key get */
        cpssOsMemSet(&flow, 0, sizeof(flow));
        rc = prvAppDemoIpfixManagerFlowKeyGet(devNum, portNum, (GT_U8 **)packetBuffs,
                                              buffLenArr, &rxParams, &flow);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("prvAppDemoIpfixManagerFlowKeyGet failed: rc = %d\n", rc);
            continue;
        }

        /* Check if the key already exists */
        rc = prvAppDemoIpfixManagerFlowKeySearch(portGroupId, &flow, &isFound);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("prvAppDemoIpfixManagerFlowKeySearch failed: rc = %d\n", rc);
            continue;
        }

        if (isFound == GT_FALSE)
        {
            /* Add IPFIX Manager entry */
            portGroupsBmp = (1 << portGroupId);
            entryParams.firstTsValid = GT_TRUE;
            entryParams.firstTs = (GT_U16) rxParams.dsaParam.dsaInfo.toCpu.timestamp;
            rc = cpssDxChIpfixManagerEntryAdd(devNum, portGroupsBmp, &entryParams, &flowId);
            if (rc != GT_OK)
            {
                __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEntryAdd failed: rc = %d\n", rc);
                continue;
            }

            if (flowId >= appIpfixManagerDbPtr->portGroupCfg[(portGroupId/2)*2].maxIpfixIndex)
            {
                __IPFIX_MANAGER_LOG("FlowId=%d for portGroupId=%d out of range. Max = %d\n",
                                    flowId, portGroupId,
                                    appIpfixManagerDbPtr->portGroupCfg[portGroupId/2*2].maxIpfixIndex);

                /* Delete ipfix manager entry */
                rc = cpssDxChIpfixManagerEntryDelete(devNum,portGroupsBmp,flowId);
                if (rc != GT_OK)
                {
                    __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEntryDelete failed: rc = %d\n", rc);
                }

                continue;
            }

            /* Add EMM Rule */
            /* Enable Cuckoo algorithm in case of collision */
            cpssOsMemSet(&emmEntryParams, 0, sizeof(emmEntryParams));
            emmEntryParams.rehashEnable = GT_TRUE;

            cpssOsMemSet(&emmEntry, 0, sizeof(emmEntry));
            cpssOsMemCpy(&emmEntry.exactMatchEntry.key, &flow.key,
                         sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
            emmEntry.exactMatchEntry.lookupNum = appIpfixManagerDbPtr->testParams.emm.lookupNum;
            emmEntry.exactMatchActionType = appIpfixManagerDbPtr->testParams.emm.actionType;
            emmEntry.expandedActionIndex = expandedActionIndex;

            cpssOsMemCpy(&emmEntry.exactMatchAction,
                         &appIpfixManagerDbPtr->testParams.emm.action,
                         sizeof(CPSS_DXCH_EXACT_MATCH_ACTION_UNT));

            emmEntry.exactMatchAction.pclAction.flowId  = flowId;
            emmEntry.exactMatchAction.pclAction.policer.policerEnable  = CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
            emmEntry.exactMatchAction.pclAction.policer.policerId = flowId;


            rc = cpssDxChExactMatchManagerEntryAdd(appIpfixManagerDbPtr->exactMatchManagerId[portGroupId/2],
                                                   &emmEntry, &emmEntryParams);
            if (rc != GT_OK)
            {
                __IPFIX_MANAGER_LOG("cpssDxChExactMatchManagerEntryAdd failed: rc = %d\n", rc);

                __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(flowsLearningFails);

                rc2 = cpssDxChIpfixManagerEntryDelete(devNum, portGroupsBmp, flowId);
                if (rc2 != GT_OK)
                {
                    __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEntryDelete failed: rc = %d\n", rc);
                }

                continue;
            }

            __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(activeEntries);

            flow.isactive = GT_TRUE;
            flow.data.flowId = ((portGroupId & 0x7) << 12) | (flowId & 0xFFF);

            /* Update the local database */
            cpssOsMemCpy(&appIpfixManagerDbPtr->flowsDb[portGroupId][flowId], &flow,
                        sizeof(flow));
        }
        else
        {
            __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(pktsFilteredByHost);
        }

        if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        {
            /* now you need to free the buffers */
            if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.allocMethod[sdmaQNum] != CPSS_RX_BUFF_NO_ALLOC_E)
            {
                rc = cpssDxChNetIfRxBufFree(devNum,
                                            sdmaQNum,
                                            (GT_U8 **)packetBuffs,numOfBuffs);
            }
            else
            {
                /* move the first buffer pointer to the original place*/
                packetBuffs[0] = (GT_U8*)((((GT_UINTPTR)(packetBuffs[0])) >> APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
                                          << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF);

                /* fix the last buffer's size*/
                buffLenArr[numOfBuffs-1] = APP_DEMO_RX_BUFF_SIZE_DEF;

                rc = cpssDxChNetIfRxBufFreeWithSize(devNum,
                                                    sdmaQNum,
                                                    (GT_U8 **)packetBuffs,buffLenArr,numOfBuffs);
            }

        }
        else /* CPSS_NET_CPU_PORT_MODE_MII_E */
        {
            rc = cpssDxChNetIfMiiRxBufFree(devNum,
                                           sdmaQNum,
                                           (GT_U8 **)packetBuffs,numOfBuffs);
        }

        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("RX Buff Free failed: rc=%d\n",rc);
        }

        newFlowsCount++;
        if (newFlowsCount == appIpfixManagerDbPtr->newFlows.burstSize)
        {
            newFlowsCount = 0;
            /* [TBD] Stop after adding 1K new flows */
        }
    }

    appIpfixManagerDbPtr->newFlows.taskTerminate = GT_TRUE;
    appIpfixManagerDbPtr->newFlows.isTaskActive = GT_FALSE;

#ifdef __GNUC__
    /* to avoid warnings in gcc */
    return 0;
#endif
}

/**
 * @internal prvAppDemoIpfixManagerBitsGet function
 * @endinternal
 *
 * @brief Get bits from input stream
 *
 * @param[in] data    - (pointer to) input data stream
 * @param[in] offset  - start bit offset
 * @param[in] numBits - size in bits to read
 *
 * @retval Extracted value from the data stream
 */
static GT_U32 prvAppDemoIpfixManagerBitsGet
(
    IN  GT_U8    *data,
    IN  GT_U32   offset,
    IN  GT_U32   numBits
)
{
    GT_U32 ii, byte, bitShift, outData=0;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        outData  |= ((data[byte] >> bitShift) & 1) << ii;
    }
    return outData;
}

/**
 * @internal  prvAppDemoIpfixManagerDataRecordGet function
 * @endinternal
 *
 * @brief   Parse data records from the data packet
 *
 * @param[in]   pktBuffsArr   -  packet buffers array.
 * @param[in]   buffLen       -  buffer length
 * @param[in]   dataPktPtr    -  pointer to data packet
 *
 * @return GT_OK  -  on success.
 */
static GT_STATUS prvAppDemoIpfixManagerDataRecordGet
(
    IN GT_U8    **pktBuffsArr,
    IN GT_U32   buffLen,
    OUT PRV_APP_IPFIX_MGR_DATA_PKT_STC   *dataPktPtr
)
{
    GT_U8       *pktBuff = *pktBuffsArr;
    GT_U32      startOffset = 0;
    GT_U32      dataRecordSize = 24; /*in bytes*/
    GT_U32      l2HeaderSize = 12;
    GT_U32      headerSize   = 16; /*in bytes*/
    PRV_APP_IPFIX_MGR_DATA_RECORD_STC *dataRecordPtr;
    PRV_APP_IPFIX_MGR_DATA_PKT_HEADER_STC *headerPtr;
    GT_U32      i = 0 ;

    if (buffLen < 16)
    {
        return GT_OUT_OF_RANGE;
    }

    /* IPFIX Data Packet Format: DMAC(6B)|SMAC(6B)|eDSA(16B)|IPFIX Packet Header(16B)|Data(24B)
     * Header 16B
     * PacketType(1B)|TODin4msunits(7B)|SequenceNumber(2B)|ExportedEntriesNumber(2B)|FirstFlowId(2B)|LastFlowId(2B)
     * Data 24B
     * FlowId(2B)|FirstTs(20b)|LastTs(20b)|PacketCount(40b)|DropCount(40b)|ByteCount(46b)|EntryStatus(3b)|Padding(7b)
     */

    headerPtr = &dataPktPtr->header;

    /* packet type (8 bits) */
    headerPtr->packetType = (GT_U32) pktBuff[l2HeaderSize];

    /* time of the day sampled while forming the packet (7 bytes) */
    startOffset = l2HeaderSize*8 + 8;
    headerPtr->tod[0] = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 32);
    headerPtr->tod[1] = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset+32, 24);

    /* sequence number (16 bits) */
    startOffset += 56;
    headerPtr->seqNum = (GT_U16) prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 16);

    /* number of data records exported (16 bits) */
    startOffset += 16;
    headerPtr->numExported = (GT_U16) prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 16);

    /* flowId of the first data record exported (16 bits) */
    startOffset += 16;
    headerPtr->firstFlowId = (GT_U16) prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 16);

    /* flowId of the lat data record exported (16 bits) */
    startOffset += 16;
    headerPtr->lastFlowId = (GT_U16) prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 16);

    if (buffLen < (GT_U32)(16 + headerPtr->numExported * 24))
    {
        return GT_OUT_OF_RANGE;
    }

    for (i=0; i<headerPtr->numExported; i++)
    {
        dataRecordPtr = &dataPktPtr->dataRecord[i];

        /* flow identifier (16 bits) */
        startOffset = 8 * (l2HeaderSize + headerSize + i*dataRecordSize);
        dataRecordPtr->flowId  = (GT_U16) prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 16);

        /* Time Stamp of the first packet in the flow (20 bits)*/
        startOffset += 16;
        dataRecordPtr->firstTs = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 20);

        /* Time stamp of the last packet in the flow (20 bits) */
        startOffset += 20;
        dataRecordPtr->lastTs  = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 20);

        /* packet count (40 bits) */
        startOffset += 20;
        dataRecordPtr->packetCount[0] = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 32);
        dataRecordPtr->packetCount[1] = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset + 32, 8);

        /* drop count (40 bits) */
        startOffset += 40;
        dataRecordPtr->dropCount[0] = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 32);
        dataRecordPtr->dropCount[1] = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset + 32, 8);

        /* byte count (46 bits)*/
        startOffset += 40;
        dataRecordPtr->byteCount[0] = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 14);
        dataRecordPtr->byteCount[1] = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset + 14, 32);

        /* entry status (2 bits) */
        startOffset += 46;
        dataRecordPtr->entryStatus = prvAppDemoIpfixManagerBitsGet(pktBuff, startOffset, 2);
    }

    return GT_OK;
}

/**
 * @internal   prvAppDemoIpfixManagerFlowDbStatsUpdate function
 * @endinternal
 *
 * @brief   Update statistics of a flow in aplication flows database
 *
 * @param[in] dataPktPtr  - (pointer to) data packets pointer
 *
 * @return GT_OK   -  on success.
 */
static GT_STATUS prvAppDemoIpfixManagerFlowDbStatsUpdate
(
    IN  PRV_APP_IPFIX_MGR_DATA_PKT_STC   *dataPktPtr
)
{
    GT_U32      i;
    GT_U16      flowId;
    PRV_APP_IPFIX_MGR_DATA_RECORD_STC   *dbDataRecordPtr;

    for (i=0; i<dataPktPtr->header.numExported; i++)
    {
        flowId = dataPktPtr->dataRecord[i].flowId;
        dbDataRecordPtr = &appIpfixManagerDbPtr->flowsDb[flowId >> 12][flowId & 0xFFF].data;
        __IPFIX_MANAGER_VAL_COMPARE(flowId, dbDataRecordPtr->flowId);

        cpssOsMemCpy(dbDataRecordPtr, &dataPktPtr->dataRecord[i],
                     sizeof(PRV_APP_IPFIX_MGR_DATA_RECORD_STC));
    }

    return GT_OK;
}

static GT_BOOL prvAppDemoIpfixManagerIsTimedOut
(
    IN GT_U32   firstTsSec,
    IN GT_U32   firstTsNanoSec,
    IN GT_U32   lastTsSec,
    IN GT_U32   lastTsNanoSec,
    IN GT_U32   timeOutInSec
)
{
    GT_U32              ageInSeconds = 0;
    GT_U32              nanoSeconds = 0;

    if (lastTsSec >= firstTsSec)
    {
        ageInSeconds = lastTsSec - firstTsSec;
    }
    else
    {
        ageInSeconds = 0xFF - (firstTsSec - lastTsSec);
    }

    if(lastTsNanoSec >= firstTsNanoSec)
    {
        nanoSeconds = lastTsNanoSec - firstTsNanoSec;
    }
    else
    {
        nanoSeconds = 0xFF - (firstTsNanoSec -  lastTsNanoSec);
    }

    if ((nanoSeconds + firstTsNanoSec) > 0xFF)
    {
        ageInSeconds++;
    }

    if (ageInSeconds >= timeOutInSec)
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}

/**
 * @internal   prvAppDemoIpfixManagerAgedOutCheck function
 * @endinternal
 *
 * @brief   Check whether a particular flow is aged out.
 *
 * @param[in] portGroupId  - port group identifier
 * @param[in] flowId       - flow identifier
 *
 * @return GT_TRUE   -  if the flow is aged out. 
 *         GT_FALSE  -  otherwise 
 */
static GT_BOOL prvAppDemoIpfixManagerAgedOutCheck
(
    IN  GT_U32  portGroupId,
    IN  GT_U32  flowId,
    IN  GT_U32  tod,
    OUT GT_BOOL *isIdleTimeOutPtr
)
{
    PRV_APP_IPFIX_MGR_DATA_RECORD_STC *dataRecordPtr;
    GT_U32              firstTsNanoSec = 0;
    GT_U32              lastTsNanoSec = 0;
    GT_U32              firstTsSec = 0;
    GT_U32              lastTsSec = 0;
    GT_U32              timeoutInSec = 0;
    GT_BOOL             isTimedOut;

    if(appIpfixManagerDbPtr->flowsDb[portGroupId][flowId].isactive == GT_FALSE)
    {
        __IPFIX_MANAGER_LOG("Flow: {%d %d} is not active\n"
                           , portGroupId, flowId);
        return GT_FALSE;
    }

    dataRecordPtr  = &appIpfixManagerDbPtr->flowsDb[portGroupId][flowId].data;

    if (dataRecordPtr == NULL)
    {
        __IPFIX_MANAGER_LOG("dataRecord for portGroupId:%d and flowId: %d is NULL\n",
                            portGroupId, flowId);
        *isIdleTimeOutPtr = GT_TRUE;
        return GT_FALSE;
    }

    if((dataRecordPtr->lastTs == 0) || (dataRecordPtr->firstTs == 0) ||
       (tod == 0))
    {
        return GT_FALSE;
    }

    /* check idle timeout. TOD - LastTs >= idleTimeout */
    firstTsSec     = (dataRecordPtr->lastTs >> 8) & 0xFF;
    firstTsNanoSec = dataRecordPtr->lastTs & 0xFF;
    lastTsSec      = (tod >> 8) & 0xFF;
    lastTsNanoSec  = tod & 0xFF;
    timeoutInSec   = appIpfixManagerDbPtr->testParams.idleTimeout;
    isTimedOut = prvAppDemoIpfixManagerIsTimedOut(firstTsSec, firstTsNanoSec, lastTsSec,
                                     lastTsNanoSec, timeoutInSec);
    *isIdleTimeOutPtr = GT_TRUE;
    if (isTimedOut == GT_TRUE)
    {
        __IPFIX_MANAGER_DBG_LOG("Flow: {%d, %d} IdleAgedOut. TOD: (%X,%X), LastTs: (%X,%X)\n",
                            portGroupId, flowId, lastTsSec, lastTsNanoSec,
                            firstTsSec, firstTsNanoSec);
        return GT_TRUE;
    }

    /* check active timeout. LastTs - FirstTs >= activeTimeOut */
    firstTsSec     = (dataRecordPtr->firstTs >> 8) & 0xFF;
    firstTsNanoSec = dataRecordPtr->firstTs & 0xFF;
    lastTsSec      = (dataRecordPtr->lastTs >> 8) & 0xFF;
    lastTsNanoSec  = dataRecordPtr->lastTs & 0xFF;
    timeoutInSec   = appIpfixManagerDbPtr->testParams.activeTimeout;
    isTimedOut = prvAppDemoIpfixManagerIsTimedOut(firstTsSec, firstTsNanoSec, lastTsSec,
                                     lastTsNanoSec, timeoutInSec);
    *isIdleTimeOutPtr = GT_FALSE;
    if (isTimedOut == GT_TRUE)
    {
        __IPFIX_MANAGER_DBG_LOG("Flow: {%d, %d} ActiveAgedOut. LastTs: (%X,%X), FirstTs: (%X,%X)\n",
                            portGroupId, flowId, lastTsSec, lastTsNanoSec,
                            firstTsSec, firstTsNanoSec);
        return GT_TRUE;
    }

    return GT_FALSE;
}

/**
 * @internal prvAppDemoIpfixManagerDataPktsCollect function
 * @endinternal
 *
 * @brief   This task periodically reads data pkts queue and updates
 *          the application IPFIX database
 *
 * @param[in] arg  - input arguments for the task.
 *                   includes device number
 *
 * @return handle to the task
 */
static unsigned __TASKCONV prvAppDemoIpfixManagerDataPktsCollect
(
    GT_VOID *arg
)
{
    GT_STATUS rc = GT_OK;
    PRV_APP_IPFIX_MGR_DATA_PKT_STC  dataPkt;
    GT_U32                          dataRecordCount = 0;
    GT_U8                           devNum;
    GT_U32                          numOfBuffs;
    GT_U8                           *packetBuffs[PRV_APP_IPFIX_MGR_PKT_BUFF_LEN];
    GT_U32                          buffLenArr[PRV_APP_IPFIX_MGR_PKT_BUFF_LEN];
    CPSS_DXCH_NET_RX_PARAMS_STC     rxParams;
    PRV_APP_IPFIX_MGR_DATA_RECORD_STC dataRecord;
    GT_U32                          waitTime = 0;
    GT_U32                          i;
    GT_U32                          j;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC     entry;
    GT_U8                           sdmaQNum;
    GT_U32                          tod = 0;
    GT_BOOL                         isIdleTimeout;

    devNum = *((GT_U8 *)arg);
    sdmaQNum = appIpfixManagerDbPtr->testParams.dataPktsSdmaQNum;

    if (appIpfixManagerDbPtr->dataPkts.isTaskActive == GT_TRUE)
    {
        return GT_BAD_STATE;
    }

    appIpfixManagerDbPtr->dataPkts.taskTerminate = GT_FALSE;
    appIpfixManagerDbPtr->dataPkts.isTaskActive = GT_TRUE;

    dataRecordCount = 0;
    while ((appIpfixManagerDbPtr->allTasksTerminate == GT_FALSE) &&
           (appIpfixManagerDbPtr->dataPkts.taskTerminate == GT_FALSE))
    {
        cpssOsTimerWkAfter(appIpfixManagerDbPtr->testParams.dataPktsPollInterval);
        if((appIpfixManagerDbPtr->allTasksTerminate == GT_TRUE) ||
           (appIpfixManagerDbPtr->dataPkts.taskTerminate == GT_TRUE))
        {
            break;
        }

        appIpfixManagerDbPtr->pendingEvents.waitForExportComplete = GT_TRUE;
        rc = cpssDxChIpfixManagerIpfixDataGetAll(devNum);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerIpfixDataGetAll failed: rc = %d\n", rc);
            continue;
        }

        __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(dataGetAllRequests);

        waitTime = 0;
        while (appIpfixManagerDbPtr->pendingEvents.waitForExportComplete == GT_TRUE)
        {
            cpssOsTimerWkAfter(PRV_APP_IPFIX_MGR_EXPORT_COMPLETE_TIME_PER_TIK);
            waitTime ++;
            if (waitTime > appIpfixManagerDbPtr->testParams.waitTimeForExportComplete /
                            PRV_APP_IPFIX_MGR_EXPORT_COMPLETE_TIME_PER_TIK)
            {
                /* __IPFIX_MANAGER_LOG("Didn't Receive exportComplete until timeout\n");*/
                appIpfixManagerDbPtr->pendingEvents.waitForExportComplete = GT_FALSE;
                break;
            }
        }

        while (1) {
            /* get the packet from the device */
            if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
            {
                numOfBuffs = 1;
                rc = cpssDxChNetIfSdmaRxPacketGet(devNum,
                                                  sdmaQNum,
                                                  &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr, &rxParams);
                if (rc != 0)
                {
                    if (rc != GT_NO_MORE)
                    {
                        __IPFIX_MANAGER_LOG("cpssDxChNetIfSdmaRxPacketGet failed: rc = %d\n", rc);
                    }
                    else
                    {
                        rc = GT_OK;
                    }

                    break;
                }
            }
            else if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
            {
                rc = cpssDxChNetIfMiiRxPacketGet(devNum,sdmaQNum,
                                                 &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr,&rxParams);
                if (rc != 0)
                {
                    __IPFIX_MANAGER_LOG("cpssDxChNetIfMiiRxPacketGet failed: rc = %d\n", rc);
                    break;
                }
            }
            else
            {
                __IPFIX_MANAGER_LOG("Invalid CPU port mode\n");
                rc = GT_BAD_STATE;
                break;
            }

            __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(dataPktsFetchedByHost);

            /* flow key get */
            cpssOsMemSet(&dataRecord, 0, sizeof(dataRecord));
            rc = prvAppDemoIpfixManagerDataRecordGet((GT_U8 **)packetBuffs, buffLenArr[0],
                                                     &dataPkt);
            if (rc != GT_OK)
            {
                __IPFIX_MANAGER_LOG("prvAppDemoIpfixManagerDataRecordGet failed: rc = %d\n", rc);

                __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(dataPktsDrops);

                break;
            }

            tod = dataPkt.header.tod[0];

            if (appIpfixManagerDbPtr->cpuRxDumpEnable == GT_TRUE)
            {
                /* Print parsed data packet prints */
                appDemoIpfixManagerDataRecordDump(&dataPkt);
            }

            /* Check if the key already exists and update the stats */
            rc = prvAppDemoIpfixManagerFlowDbStatsUpdate(&dataPkt);
            if (rc != GT_OK)
            {
                __IPFIX_MANAGER_LOG("prvAppDemoIpfixManagerFlowDbStatsUpdate failed: rc = %d\n", rc);
                break;
            }

            if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
            {
                /* now you need to free the buffers */
                if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.allocMethod[sdmaQNum]!= CPSS_RX_BUFF_NO_ALLOC_E)
                {
                    rc = cpssDxChNetIfRxBufFree(devNum,sdmaQNum,
                                                (GT_U8 **)packetBuffs,numOfBuffs);
                }
                else
                {
                    /* move the first buffer pointer to the original place*/
                    packetBuffs[0] = (GT_U8*)((((GT_UINTPTR)(packetBuffs[0])) >> APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
                                              << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF);

                    /* fix the last buffer's size*/
                    buffLenArr[numOfBuffs-1] = APP_DEMO_RX_BUFF_SIZE_DEF;

                    rc = cpssDxChNetIfRxBufFreeWithSize(devNum,sdmaQNum,
                                                        (GT_U8 **)packetBuffs,buffLenArr,numOfBuffs);
                }
            }
            else /* CPSS_NET_CPU_PORT_MODE_MII_E */
            {
                rc = cpssDxChNetIfMiiRxBufFree(devNum,sdmaQNum,
                                               (GT_U8 **)packetBuffs,numOfBuffs);
            }

            if (rc != GT_OK)
            {
                __IPFIX_MANAGER_LOG("RX Buff Free failed\n");
                break;
            }
        }

        if (rc != GT_OK)
        {
            continue;
        }

        if ((appIpfixManagerDbPtr->agingEnable == GT_TRUE) &&
            (appIpfixManagerDbPtr->globalCfg.agingOffload == GT_FALSE))
        {
            /* Aging scan */
            for (i=0; i<CPSS_DXCH_MAX_PORT_GROUPS_CNS; i++)
            {
                for (j=0; j<PRV_APP_IPFIX_MGR_FLOWS_PER_PORT_GROUP_MAX_CNS ; j++)
                {
                    if (appIpfixManagerDbPtr->flowsDb[i][j].isactive == GT_TRUE)
                    {
                        if (prvAppDemoIpfixManagerAgedOutCheck(i, j, tod, &isIdleTimeout))
                        {
                            cpssOsMemSet(&entry, 0, sizeof(entry));
                            cpssOsMemCpy(&entry.exactMatchEntry.key,
                                         &appIpfixManagerDbPtr->flowsDb[i][j].key,
                                         sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));

                            /* Get the exact match manager entry */
                            rc = cpssDxChExactMatchManagerEntryGet(appIpfixManagerDbPtr->exactMatchManagerId[i/2],
                                                                  &entry);
                            if (rc != GT_OK)
                            {
                                __IPFIX_MANAGER_LOG("cpssDxChExactMachManagerEntryGet failed: rc=%d\n", rc);
                                continue;
                            }

                            /* delete Exact Match Manager */
                            rc = cpssDxChExactMatchManagerEntryDelete(appIpfixManagerDbPtr->exactMatchManagerId[i/2],
                                                                      &entry);
                            if (rc != GT_OK)
                            {
                                __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEntryDelete failed: rc=%d\n", rc);
                                continue;
                            }

                            /* delete Ipfix Manager Entry */
                            rc = cpssDxChIpfixManagerEntryDelete(devNum, (1 << i), j);
                            if (rc != GT_OK)
                            {
                                __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEntryDelete failed: rc=%d\n", rc);
                                continue;
                            }

                            if (isIdleTimeout == GT_TRUE)
                            {
                                __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(flowsTerminatedByIdleTimeout);
                            }
                            else
                            {
                                __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(flowsTerminatedByActiveTimeout);
                            }
                            __IPFIX_MANAGER_DBG_COUNTER_DECREMENT(activeEntries);

                            /* update database */
                            cpssOsMemSet(&appIpfixManagerDbPtr->flowsDb[i][j], 0,
                                         sizeof(appIpfixManagerDbPtr->flowsDb[i][j]));
                            
                        }
                    }
                }
            }
        }

        dataRecordCount ++;
        if (dataRecordCount == appIpfixManagerDbPtr->dataPkts.burstSize)
        {
            dataRecordCount = 0;
            /* [TBD] Stop after handling 1K data packets */
        }
    }

    appIpfixManagerDbPtr->dataPkts.taskTerminate = GT_TRUE;
    appIpfixManagerDbPtr->dataPkts.isTaskActive = GT_FALSE;

#ifdef __GNUC__
    /* to avoid warnings in gcc */
    return 0;
#endif
}

/**
 * @internal   appDemoIpfixManagerFlowDelete function
 * @endinternal
 *
 * @brief Delete one flow
 *
 * @param[in]  devNum        - device number
 * @param[in]  portGroupId   - port group identifier
 * @param[in]  flowId        - flow identifier
 *
 * @return GT_OK    - on success
 */
GT_STATUS appDemoIpfixManagerFlowDelete
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 flowId
)
{
    GT_STATUS                                rc;
    GT_U32                                   emmId;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC  emmEntry;
    GT_PORT_GROUPS_BMP                       portGroupsBmp;

    emmId = appIpfixManagerDbPtr->exactMatchManagerId[portGroupId/2];
    cpssOsMemCpy(&emmEntry.exactMatchEntry.key,
                 &appIpfixManagerDbPtr->flowsDb[portGroupId][flowId].key,
                 sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
    emmEntry.exactMatchEntry.lookupNum = appIpfixManagerDbPtr->testParams.emm.lookupNum;
    emmEntry.exactMatchActionType = appIpfixManagerDbPtr->testParams.emm.actionType;
    rc = cpssDxChExactMatchManagerEntryDelete(emmId, &emmEntry);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChExactMatchManagerEntryDelete failed: rc=%d\n",rc);
        return rc;
    }

    appIpfixManagerDbPtr->flowsDb[portGroupId][flowId].isactive = GT_FALSE;
    __IPFIX_MANAGER_DBG_COUNTER_DECREMENT(activeEntries);

    portGroupsBmp = 1 << portGroupId;
    rc = cpssDxChIpfixManagerEntryDelete(devNum, portGroupsBmp, flowId);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEntryDelete failed: rc=%d\n",rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal   appDemoIpfixManagerFlowDeleteAll function
 * @endinternal
 *
 * @brief Delete all flows
 *
 * @param[in]  devNum        - device number
 *
 * @return GT_OK    - on success
 */
GT_STATUS appDemoIpfixManagerFlowDeleteAll
(
    IN  GT_U8     devNum
)
{
    GT_STATUS                                rc;
    GT_U32                                   i,j;
    GT_U32                                   emmId;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC  emmEntry;
    GT_U32                                   numEmDeleted;
    GT_U32                                   numDeleted = 0;
    GT_U32                                   *numDeletedPtr = &numDeleted;

    numEmDeleted = 0;
    for (i=0; i<CPSS_DXCH_MAX_PORT_GROUPS_CNS; i++)
    {
        for (j = 0; j < PRV_APP_IPFIX_MGR_FLOWS_PER_PORT_GROUP_MAX_CNS; j++)
        {
            if(appIpfixManagerDbPtr->flowsDb[i][j].isactive == GT_FALSE)
            {
                continue;
            }

            emmId = appIpfixManagerDbPtr->exactMatchManagerId[i/2];
            cpssOsMemCpy(&emmEntry.exactMatchEntry.key,
                         &appIpfixManagerDbPtr->flowsDb[i][j].key,
                         sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
            emmEntry.exactMatchEntry.lookupNum = appIpfixManagerDbPtr->testParams.emm.lookupNum;
            emmEntry.exactMatchActionType = appIpfixManagerDbPtr->testParams.emm.actionType;
            rc = cpssDxChExactMatchManagerEntryDelete(emmId, &emmEntry);
            if (rc != GT_OK)
            {
                __IPFIX_MANAGER_LOG("cpssDxChExactMatchManagerEntryDelete failed: rc=%d\n",rc);
                return rc;
            }
            appIpfixManagerDbPtr->flowsDb[i][j].isactive = GT_FALSE;
            __IPFIX_MANAGER_DBG_COUNTER_DECREMENT(activeEntries);
            numEmDeleted++;
        }
    }

    /* Delete all IPFIX entries */
    rc = cpssDxChIpfixManagerEntryDeleteAll(devNum, numDeletedPtr);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEntryDeleteAll failed: rc=%d\n", rc);
        return rc;
    }

    if (*numDeletedPtr != numEmDeleted)
    {
        __IPFIX_MANAGER_LOG("Not Matching:\n"
                            "    Number of Exact match entries Deleted  : %d\n"
                            "    Number of Ipfix Manager entries Deleted: %d\n",
                            numEmDeleted, *numDeletedPtr);
    }

    __IPFIX_MANAGER_DBG_LOG("Deleted %d Entries\n", *numDeletedPtr);

    return GT_OK;
}

/**
 * @internal prvAppDemoIpfixManagerSingleEventHandle function
 * @endinternal
 *
 * @brief  Handle a single event.
 *
 * @param[in] handledIpcMsgPtr - (pointer to) IPC message that we handle.
 *
 * @return GT_OK    -  on success
 */
GT_STATUS prvAppDemoIpfixManagerSingleEventHandle
(
    OUT CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT *handledIpcMsgPtr
)
{
    GT_STATUS                                rc = GT_OK;
    CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT *ipcMsgPtr;
    GT_U32                                   startIndex = appIpfixManagerDbPtr->pendingEvents.startIndex;
    GT_U16                                   flowId;
    GT_U8                                    devNum;

    ipcMsgPtr = &(appIpfixManagerDbPtr->pendingEvents.ipcMsg[startIndex]);
    devNum = appIpfixManagerDbPtr->devNum;

    if (handledIpcMsgPtr != NULL)
    {
        cpssOsMemCpy(handledIpcMsgPtr, ipcMsgPtr,
                     sizeof(CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT));
    }

    switch (ipcMsgPtr->opcodeGet.opcode)
    {
    /* Entry Invalidate */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_ENTRY_INVALIDATE_E:
        /* Delete Exactmatch manager entry */
        flowId = ipcMsgPtr->entryInvalidate.flowId;
        rc = appDemoIpfixManagerFlowDelete(devNum, flowId >> 13, flowId & 0x1FFF);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("appDemoIpfixManagerFlowDelete failed: rc = %d\n", rc);
            return rc;
        }
        break;

    /* Export Completion */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_EXPORT_COMPLETION_E:
        appIpfixManagerDbPtr->pendingEvents.waitForExportComplete = GT_FALSE;
        __IPFIX_MANAGER_DBG_COUNTER_INCREMENT(exportCompleteMsgs);
        break;

    /* config return */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_RETURN_E:
        cpssOsMemCpy(&appIpfixManagerDbPtr->pendingEvents.lastConfigReturn, ipcMsgPtr,
                     sizeof(CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_CONFIG_STC));
        appIpfixManagerDbPtr->pendingEvents.configReturnReadStatus = GT_TRUE;
        break;

    /* config error */
    case CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_OPCODE_CONFIG_ERROR_E:
        appIpfixManagerDbPtr->pendingEvents.configErrorOccured = GT_TRUE;
        appIpfixManagerDbPtr->pendingEvents.configErrorBitmap = ipcMsgPtr->configError.errBmp;
        break;

    default:
        __IPFIX_MANAGER_LOG("Invalid opcode: %d\n", ipcMsgPtr->opcodeGet.opcode);
        return GT_BAD_VALUE;
    }

    cpssOsMemSet(ipcMsgPtr, 0, sizeof(CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT));
    appIpfixManagerDbPtr->pendingEvents.startIndex ++;
    return GT_OK;
}

/**
 * @internal  prvAppDemoIpfixManagerEventNotifyFunc function
 * @endinternal
 *
 * @brief  This function is used to bind it to cpss event handler.
 *
 * @param[in] ipcMsgPtr - (pointer to) IPC message.
 */
static void prvAppDemoIpfixManagerEventNotifyFunc
(
    IN CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT *ipcMsgPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      endIndex = appIpfixManagerDbPtr->pendingEvents.endIndex;

    if (appIpfixManagerDbPtr->pendingEvents.isEventsBuffFull == GT_TRUE)
    {
        /* Events buffer is full. Handle one event first then add new event */
        rc = prvAppDemoIpfixManagerSingleEventHandle(NULL);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("prvAppDemoIpfixManagerSingleEventHandle failed: rc=%d\n",rc);
            return;
        }

        appIpfixManagerDbPtr->pendingEvents.startIndex ++;
        if (appIpfixManagerDbPtr->pendingEvents.startIndex == PRV_APP_IPFIX_MGR_EVENT_ARRAY_SIZE_CNS)
        {
            appIpfixManagerDbPtr->pendingEvents.startIndex = 0;
        }
        appIpfixManagerDbPtr->pendingEvents.isEventsBuffFull = GT_FALSE;
    }

    cpssOsMemCpy(&appIpfixManagerDbPtr->pendingEvents.ipcMsg[endIndex], ipcMsgPtr,
               sizeof(CPSS_DXCH_IPFIX_MANAGER_IPC_MSG_RECV_UNT));

    (appIpfixManagerDbPtr->pendingEvents.endIndex) ++;
    if (appIpfixManagerDbPtr->pendingEvents.endIndex == PRV_APP_IPFIX_MGR_EVENT_ARRAY_SIZE_CNS)
    {
        appIpfixManagerDbPtr->pendingEvents.endIndex = 0;
    }

    if (appIpfixManagerDbPtr->pendingEvents.endIndex == appIpfixManagerDbPtr->pendingEvents.startIndex)
    {
        /* IPC Msg memory is full */
        appIpfixManagerDbPtr->pendingEvents.isEventsBuffFull = GT_TRUE;
    }
}

/**
 * @internal  prvAppDemoIpfixManagerEventsHandleAll function
 * @endinternal
 *
 * @brief  Handle all events in the events array
 *
 * @return GT_OK  - on success
 */
static GT_STATUS prvAppDemoIpfixManagerEventsHandleAll
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    while (appIpfixManagerDbPtr->pendingEvents.startIndex !=
                appIpfixManagerDbPtr->pendingEvents.endIndex)
    {
        rc = prvAppDemoIpfixManagerSingleEventHandle(NULL);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("prvAppDemoIpfixManagerSingleEventHandle failed: rc=%d\n", rc);
            break;
        }
    }

    return rc;
}

/**
 * @internal  prvAppDemoIpfixManagerIpcEventHandle function
 * @endinternal
 *
 * @brief  This function periodically scans IPC message queue and
 *         handles all IPC events
 *
 * @param[in[   arg  - input arguments
 *
 * @retval handle for the task
 */
static unsigned __TASKCONV prvAppDemoIpfixManagerIpcEventHandle
(
    GT_VOID *arg
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      ipcEventCount = 0;
    GT_U8       devNum = 0;
    GT_U32      numOfFetched = 0;

    devNum = *((GT_U8 *)arg);

    if (appIpfixManagerDbPtr->ipcFetch.isTaskActive == GT_TRUE)
    {
        return GT_BAD_STATE;
    }

    appIpfixManagerDbPtr->ipcFetch.taskTerminate = GT_FALSE;
    appIpfixManagerDbPtr->ipcFetch.isTaskActive = GT_TRUE;

    ipcEventCount = 0;
    while ((appIpfixManagerDbPtr->allTasksTerminate == GT_FALSE) &&
           (appIpfixManagerDbPtr->ipcFetch.taskTerminate == GT_FALSE))
    {
        cpssOsTimerWkAfter(appIpfixManagerDbPtr->testParams.ipcFetchPollInterval);
        if ((appIpfixManagerDbPtr->allTasksTerminate == GT_TRUE) ||
            (appIpfixManagerDbPtr->ipcFetch.taskTerminate == GT_TRUE))
        {
            break;
        }

        rc = cpssDxChIpfixManagerIpcMsgFetchAll(devNum, &numOfFetched);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerIpcMsgFetchAll failed: rc=%d\n", rc);
            continue;
        }

        if (numOfFetched > 0)
        {
            rc = prvAppDemoIpfixManagerEventsHandleAll();
            if (rc != GT_OK)
            {
                __IPFIX_MANAGER_LOG("prvAppDemoIpfixManagerEventsHandleAll failed: rc=%d\n", rc);
                continue;
            }
        }
        else
        {
            continue;
        }

        ipcEventCount++;
        if (ipcEventCount == appIpfixManagerDbPtr->ipcFetch.burstSize)
        {
            ipcEventCount = 0;
            /* [TBD] Stop after handling 1K IPC messages */
        }
    }

    appIpfixManagerDbPtr->ipcFetch.isTaskActive = GT_FALSE;
    appIpfixManagerDbPtr->ipcFetch.taskTerminate = GT_TRUE;

#ifdef __GNUC__
    /* to avoid warnings in gcc */
    return 0;
#endif
}

/**
 * @internal   appDemoIpfixManagerThreadsStart function
 * @endinternal
 *
 * @brief Start IPFIX Manager tasks
 *
 * @param[in] devNum  - device number
 *
 * @return GT_OK    - on success
 */
GT_STATUS appDemoIpfixManagerThreadsStart
(
    IN  GT_U8   devNum
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       param1, param2, param3 = 0;

    param1 = devNum;
    param2 = devNum;
    param3 = devNum;

    /* allow to 'see' the packets that we get (with the DSA) */
    /* dxChNetIfRxPacketParse_DebugDumpEnable(GT_FALSE); */

    /* Handling of New Flows thread */
    rc = cpssOsTaskCreate("new_flows_handle",
                      300,
                      _1KB,
                      prvAppDemoIpfixManagerNewFlowsHandle,
                      &param1,
                      &appIpfixManagerDbPtr->newFlows.taskId);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("Create new flows handle task failed: rc=%d\n",rc);
        return rc;
    }

    /* wait for 10ms such that the thread starts properly */
    cpssOsTimerWkAfter(PRV_APP_IPFIX_MGR_WAIT_AFTER_TASK_CREATE_CNS);

    /* Collecting IPFIX Data Packets thread */
    rc = cpssOsTaskCreate("data_pkts_collect",
                      300,
                      _1KB,
                      prvAppDemoIpfixManagerDataPktsCollect,
                      &param2,
                      &appIpfixManagerDbPtr->dataPkts.taskId);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("Create data packets collect task failed: rc=%d\n",rc);
        return rc;
    }

    /* wait for 10ms such that the thread starts properly */
    cpssOsTimerWkAfter(PRV_APP_IPFIX_MGR_WAIT_AFTER_TASK_CREATE_CNS);

    /* Triggering Events Reporting thread */
    rc = cpssOsTaskCreate("ipc_even_handle",
                      300,
                      _1KB,
                      prvAppDemoIpfixManagerIpcEventHandle,
                      &param3,
                      &appIpfixManagerDbPtr->ipcFetch.taskId);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("Create event handle task failed: rc=%d\n",rc);
        return rc;
    }

    /* wait for 10ms such that the thread starts properly */
    cpssOsTimerWkAfter(PRV_APP_IPFIX_MGR_WAIT_AFTER_TASK_CREATE_CNS);
    return GT_OK;
}

/**
 * @internal appDemoIpfixManagerThreadsStop
 * @endinternal
 *
 * @brief   Stop all IPFIX Manager tasks
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK  - on success
 */
GT_STATUS appDemoIpfixManagerThreadsStop
(
    IN  GT_U8   devNum
)
{
    GT_STATUS    rc = GT_OK;

    (void) devNum;

    appIpfixManagerDbPtr->allTasksTerminate = GT_TRUE;
    cpssOsTimerWkAfter(2000);

    if (appIpfixManagerDbPtr->newFlows.isTaskActive == GT_TRUE)
    {
        /* Delete task new flows handle */
        rc = cpssOsTaskDelete(appIpfixManagerDbPtr->newFlows.taskId);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("Delete new flows handle task failed: rc=%d\n",rc);
            return rc;
        }
    }

    /* wait for 10ms such that the thread close properly */
    cpssOsTimerWkAfter(PRV_APP_IPFIX_MGR_WAIT_AFTER_TASK_CREATE_CNS);

    if (appIpfixManagerDbPtr->dataPkts.isTaskActive == GT_TRUE)
    {
        /* Delete task data packets colection */
        rc = cpssOsTaskDelete(appIpfixManagerDbPtr->dataPkts.taskId);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("Delete data packets collect task failed: rc=%d\n",rc);
            return rc;
        }
    }

    /* wait for 10ms such that the thread close properly */
    cpssOsTimerWkAfter(PRV_APP_IPFIX_MGR_WAIT_AFTER_TASK_CREATE_CNS);

    if (appIpfixManagerDbPtr->ipcFetch.isTaskActive == GT_TRUE)
    {
        /* Delete task ipcEventHandle */
        rc = cpssOsTaskDelete(appIpfixManagerDbPtr->ipcFetch.taskId);
        if (rc != GT_OK)
        {
            __IPFIX_MANAGER_LOG("Delete event handle task failed: rc=%d\n",rc);
            return rc;
        }
    }

    /* wait for 10ms such that the thread close properly */
    cpssOsTimerWkAfter(PRV_APP_IPFIX_MGR_WAIT_AFTER_TASK_CREATE_CNS);

    /* allow to 'see' the packets that we get (with the DSA) */
    dxChNetIfRxPacketParse_DebugDumpEnable(GT_FALSE);

    return GT_OK;
}

/**
 * @internal    appDemoIpfixManagerMain function
 * @endinternal
 *
 * @brief   Perform all initializations and start IPFIX
 *          Manager tasks
 *
 * @param[in] devNum   -   device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixManagerMain
(
    IN  GT_U8   devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_PORT_GROUPS_BMP                              portGroupsBmp;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC          attributes;
    CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC       globalCfg;
    CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC   portGroupCfg;
    CPSS_DXCH_NET_DSA_PARAMS_STC         dsaInfo;
    GT_HW_DEV_NUM                        hwDevNum;

    /* Initialize application ipfix manager */
    rc = appDemoIpfixManagerDbCreate(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerDbCreate failed: rc=%d\n", rc);
        return rc;
    }

    /* Initialize exact match manager */
    rc = appDemoIpfixManagerEmmInit(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerEmmInit failed: rc=%d\n", rc);
        return rc;
    }

    /* Initialize policer engine */
    rc = appDemoIpfixManagerPolicerInit(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerPolicerInit failed: rc=%d\n", rc);
        return rc;
    }

    /* Set flow key configuration */
    rc = appDemoIpfixManagerPclInit(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerKeysInit failed: rc=%d\n", rc);
        return rc;
    }

    rc = appDemoIpfixManagerTxInit(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerTxInit failed: rc=%d\n", rc);
        return rc;
    }

    /* Create CPSS IPFIX Manger */
    attributes.serviceCpuNum     = PRV_APP_IPFIX_MGR_SERVICE_CPU_NUM_CNS;
    rc = cpssDxChIpfixManagerCreate(devNum, &attributes);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerCreate failed: rc=%d\n", rc);
        return rc;
    }

    /* Bind event notfication function */
    rc = cpssDxChIpfixManagerEventNotifyBind(devNum, prvAppDemoIpfixManagerEventNotifyFunc);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEventNotifyBind failed: rc=%d\n", rc);
        return rc;
    }

    /* Set CPSS ipfix manager global configuration */
    cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
    globalCfg.activeTimeout = 30;   /* 30 seconds */
    globalCfg.dataPktMtu    = 1150; /* Max mtu */
    globalCfg.idleTimeout   = 30;   /* 30 seconds */
    globalCfg.ipfixDataQueueNum =  appIpfixManagerDbPtr->testParams.dataPktsSdmaQNum;
    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* Build Extended TO_CPU DSA tag; 4 words */
    cpssOsMemSet(&dsaInfo,0,sizeof(dsaInfo));
    dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
    dsaInfo.dsaInfo.toCpu.hwDevNum = (hwDevNum + 1) % 32;
    dsaInfo.dsaInfo.toCpu.cpuCode = appIpfixManagerDbPtr->testParams.dataPktsCpuCode;

    /* build DSA Tag buffer */
    rc = cpssDxChNetIfDsaTagBuild(devNum, &dsaInfo, &globalCfg.txDsaTag[0]);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChNetIfDsaTagBuild failed rc = %d\n", rc);
        return rc;
    }

    cpssOsMemCpy(&appIpfixManagerDbPtr->globalCfg, &globalCfg, sizeof(globalCfg));
    appIpfixManagerDbPtr->globalCfg.ipfixEntriesPerFlow =
                                CPSS_DXCH_IPFIX_MANAGER_ENTRIES_PER_FLOW_IS_TWO_E;
    rc = cpssDxChIpfixManagerGlobalConfigSet(devNum, &globalCfg);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerGlobalConfigSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Set CPSS ipfix manager port group configuration */
    cpssOsMemSet(&portGroupCfg, 0, sizeof(portGroupCfg));
    portGroupCfg.maxIpfixIndex = 2 * _1K;
    portGroupsBmp    =  0xFF;
    cpssOsMemSet(appIpfixManagerDbPtr->portGroupCfg, 0,
                 CPSS_DXCH_MAX_PORT_GROUPS_CNS * sizeof(CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC));
    cpssOsMemCpy(&appIpfixManagerDbPtr->portGroupCfg[0], &portGroupCfg, sizeof(portGroupCfg));
    cpssOsMemCpy(&appIpfixManagerDbPtr->portGroupCfg[2], &portGroupCfg, sizeof(portGroupCfg));
    cpssOsMemCpy(&appIpfixManagerDbPtr->portGroupCfg[4], &portGroupCfg, sizeof(portGroupCfg));
    cpssOsMemCpy(&appIpfixManagerDbPtr->portGroupCfg[6], &portGroupCfg, sizeof(portGroupCfg));
    rc = cpssDxChIpfixManagerPortGroupConfigSet(devNum, portGroupsBmp, &portGroupCfg);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerPortGroupConfigSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Ipfix manager enable */
    rc = cpssDxChIpfixManagerEnableSet(devNum, GT_FALSE);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEnableSet failed: rc=%d\n", rc);
        return rc;
    }

#ifndef ASIC_SIMULATION
    rc = appDemoIpfixManagerConfigCheck(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerConfigCheck failed: rc=%d\n", rc);
        return rc;
    }
#endif

    /* Ipfix manager enable */
    rc = cpssDxChIpfixManagerEnableSet(devNum, GT_TRUE);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEnableSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = appDemoIpfixManagerThreadsStart(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerThreadsStart failed: rc=%d\n", rc);
        return rc;
    }

    return rc;
}

/**
 * @internal appDemoIpfixManagerClear function
 * @endinternal
 *
 * @brief   Stop IPFIX Manager tasks and restore configuration
 *
 * @param[in] devNum         - device number
 *
 * @retval   GT_OK  - on success
 */
GT_STATUS appDemoIpfixManagerClear
(
    IN  GT_U8   devNum
)
{
    GT_STATUS                         rc            = GT_OK;

    rc = appDemoIpfixManagerThreadsStop(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerThreadsStop failed: rc=%d\n", rc);
        return rc;
    }

    /* Ipfix manager disable */
    rc = cpssDxChIpfixManagerEnableSet(devNum, GT_FALSE);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEnableSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Delete all IPFIX entries */
    rc = appDemoIpfixManagerFlowDeleteAll(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerEntryDeleteAll failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore EMM configuration */
    rc = appDemoIpfixManagerEmmRestore(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerEmmRestore failed: rc=%d\n", rc);
        return rc;
    }

    /* Delete CPSS IPFIX Manager */
    rc = cpssDxChIpfixManagerDelete(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("cpssDxChIpfixManagerDelete failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore Policer configuration*/
    rc = appDemoIpfixManagerPolicerRestore(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerPolicerRestore failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore Keys configuration */
    rc = appDemoIpfixManagerPclRestore(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerKeysRestore failed: rc=%d\n", rc);
        return rc;
    }

    rc = appDemoIpfixManagerTxRestore(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerTxRestore failed: rc=%d\n", rc);
        return rc;
    }

    /* Delete ipfix manager in applicaiton */
    rc = appDemoIpfixManagerDbDelete(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_MANAGER_LOG("appDemoIpfixManagerDbDelete failed: rc=%d\n", rc);
        return rc;
    }

    return rc;
}
