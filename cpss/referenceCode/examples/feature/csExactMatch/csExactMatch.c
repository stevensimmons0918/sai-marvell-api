/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* csExactMatch.c
*
* DESCRIPTION:
*  This files provide APIs to configure EM - low level based or EM Manager based also the configuration of TTI engine, to which 
*  EM lookup serves as a client.
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpssCommon/private/prvCpssMath.h>

/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/policer/cpssGenPolicerTypes.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/exactMatch/cpssDxChExactMatch.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/cpssDxChExactMatchManager.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include "../../infrastructure/csRefServices/tcamUtils.h"

static GT_U32   exactMatchManagerId = 2;
static GT_U32   index=0;
static GT_U32   firstIndexTTI = 0;

/**
* @internal csRefEmBillingConfigure function
* @endinternal
*
* @brief   configure a billing counter.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum    - device number
* @param[in] policerIndex        - billing entry index
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note  we assign all metering memeory to ingress PLR stage 0
*
*/

static GT_STATUS csRefEmBillingConfigure
(
    IN  GT_U8   devNum,
    IN  GT_U32  policerIndex
)
{
    GT_STATUS                             rc = GT_OK;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC  billEntry;
    CPSS_DXCH_POLICER_MEMORY_STC          memoryCfg;
    GT_U32                                i;

    /* stage 0 only exists in this configuraiton */
    rc = cpssDxChPolicerCountingWriteBackCacheFlush(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E);
    if(rc != GT_OK)
        return rc;
    memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] = 4096;
    memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] = 0;
    memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_EGRESS_E]    = 0;
    memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] = 4096;
    memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] = 0;
    memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_EGRESS_E]    = 0;

    rc = cpssDxChPolicerMemorySizeSet(devNum, &memoryCfg);
    if(rc != GT_OK)
        return rc;
/*
    rc = cpssDxChPolicerMemorySizeModeSet(devNum, CPSS_DXCH_POLICER_MEMORY_CTRL_MODE_1_E,100,100);
    if(rc != GT_OK)
        return rc;
*/
    rc = cpssDxChPolicerCountingModeSet(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                                        CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChPolicerStageMeterModeSet(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxCh3PolicerCountingColorModeSet(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, CPSS_DXCH3_POLICER_COLOR_COUNT_CL_E);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxCh3PolicerPacketSizeModeSet(devNum,CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
    if(rc != GT_OK)
        return rc;

    cpssOsMemSet(&billEntry, 0, sizeof(CPSS_DXCH3_POLICER_BILLING_ENTRY_STC));
    billEntry.billingCntrMode      = CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E;
    billEntry.packetSizeMode       = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    billEntry.billingCntrAllEnable = GT_TRUE;
    for(i=0;i<3;i++)
    {
        rc = cpssDxChPolicerPortGroupBillingEntrySet(devNum,
            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
            policerIndex+i,
            &billEntry);
        if(rc != GT_OK)
            return rc;
    }
    return rc;
}


/**
* @internal csRefEmBillingDump function
* @endinternal
*
* @brief   print all non-zero active billing counterst.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum    - device number
* @param[in] printAll        - if to print all counters or only the first non-zero counter
* @param[in] clearOnRead - clear-on-read
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note  use like:
*            shell-execute csRefEmBillingDump 0,1,1
*
*/
GT_U64 csRefEmBillingDump
(
    IN GT_U8   devNum,
    IN GT_BOOL printAll,
    IN GT_BOOL clearOnRead
)
{
    GT_U32  i, maxind;
    CPSS_DXCH3_POLICER_BILLING_ENTRY_STC  billEntry;
    GT_U64  sum, totalsum;
    GT_U64  zero64;
    GT_STATUS                       rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT stage;

    maxind = 4096;
    stage  = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    totalsum.l[0] = 0;
    totalsum.l[1] = 0;
    zero64.l[0] = 0;
    zero64.l[1] = 0;

    for(i=0;i<maxind;i++)
    {
        sum.l[0] = 0;
        sum.l[1] = 0;

        rc = cpssDxChPolicerPortGroupBillingEntryGet(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, stage, i, clearOnRead, &billEntry);
        if(rc != GT_OK)
        {
            cpssOsPrintf("billing read %d: failed\n",i);
        }

        sum = prvCpssMathAdd64(billEntry.yellowCntr ,billEntry.greenCntr);
        sum = prvCpssMathAdd64(billEntry.redCntr ,sum);
        if(prvCpssMathCompare64(sum, zero64))
        {
            totalsum = prvCpssMathAdd64(sum, totalsum);
            cpssOsPrintf("billing counter %d: counts ", i);

            switch(billEntry.billingCntrMode)
            {
            case CPSS_DXCH3_POLICER_BILLING_CNTR_1_BYTE_E:
                cpssOsPrintf("bytes ");
                break;

            case CPSS_DXCH3_POLICER_BILLING_CNTR_16_BYTES_E:
                cpssOsPrintf("16 bytes ");
                break;

            case CPSS_DXCH3_POLICER_BILLING_CNTR_PACKET_E:
                cpssOsPrintf("packets ");
                break;

            default:
                cpssOsPrintf("bad value ");
                return zero64;
            }
            cpssOsPrintf("green 0x%x%x, yellow 0x%x%x, red 0x%x%x\n",
                     billEntry.greenCntr.l[1], billEntry.greenCntr.l[0], billEntry.yellowCntr.l[1],
                     billEntry.yellowCntr.l[1], billEntry.redCntr.l[1], billEntry.redCntr.l[0]
                     );
            if(!printAll) return sum;
        }
    }
    return totalsum;
}

/**
* @internal csRefEmRedirectTtiEmConfigCreate function
* @endinternal
*
* @brief   In this example packet matched by its ingress port is forwarded to one of two egress ports depending
*             if match was done by TTI or EM. Different CNC counters and billing counters are connected to rules.
*             Traffic classification done on ingress port only, any valid ethernet packet can be used in test.
*
* @note   APPLICABLE DEVICES:       Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
* @param[in] ingressPort         port number where sample packet enters
* @param[in] egressPortTTI - where packet is forwarded by TTI rule
* @param[in] egressPortEM  - where packet is forwarded by EM rule
* @param[in] emOverTtiEn  - if true, EM match takes precedence over TTI match
* @param[in] useEmManager - if true, EM is configured by EM Manager API's, if not by low-level API's
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note  use like:
*            shell-execute csRefEmRedirectTtiEmConfigCreate 0,0,32,36,1,1
*            to change EM share of Shared Tables call
*            shell-execute appDemoDbEntryAdd "sharedTableMode" <MODE>
*            before cpssInitSystem
*
*/

GT_STATUS csRefEmRedirectTtiEmConfigCreate
(
    IN  GT_U8   devNum,
    IN  GT_U32  ingressPort,
    IN  GT_U32  egressPortTTI,
    IN  GT_U32  egressPortEM,
    IN  GT_BOOL emOverTtiEn,
    IN  GT_BOOL useEmManager
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          firstIndexIPCL, firstIndexEPCL;
    GT_U8                           ipclBlocks = 0;
    GT_U8                           epclBlocks = 0;
    GT_U8                           ttiBlocks = 2;
    GT_U32                          pclId;
    CPSS_DXCH_TTI_RULE_UNT          pattern ;
    CPSS_DXCH_TTI_RULE_UNT          mask    ;
    CPSS_DXCH_TTI_ACTION_STC        action  ;
    CPSS_DXCH_TTI_KEY_TYPE_ENT      keyType;
    CPSS_DXCH_TTI_KEY_SIZE_ENT      ruleSize;
    CPSS_DXCH_TTI_RULE_TYPE_ENT     ruleType;
    GT_BOOL                         billingEn = GT_TRUE;

    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT                  exactMatchLookupNum;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT                  firstLookupClientType;
    GT_U32                                            profileIndex = 2;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    GT_U32                                            numberOfElemInCrcMultiHashArr;
    GT_U32                                            exactMatchCrcMultiHashArr[16];
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                  actionData, actionDefault;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT             actionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
    GT_U32                                            expandedActionIndex = 3;
    GT_U32                                            policerIndex = 4090;

    if(billingEn)
    {
        rc = csRefEmBillingConfigure(devNum, policerIndex);
        if(rc != GT_OK)
            return rc;
    }

    rc = csRefInfraFindTCAMFirstRule(devNum, &firstIndexIPCL, &firstIndexEPCL, &firstIndexTTI);
    if(rc != GT_OK)
        return rc;
    rc = csRefInfraTCAMCountersSet(devNum,ipclBlocks,epclBlocks,ttiBlocks);
    if(rc != GT_OK)
        return rc;

    pclId       = 5;

    keyType     = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
    ruleType    = CPSS_DXCH_TTI_RULE_UDB_10_E;
    ruleSize    = CPSS_DXCH_TTI_KEY_SIZE_10_B_E;

    cpssOsPrintf("devNUm               = %-4d\n",devNum);
    cpssOsPrintf("tcamIndex            = %-4d\n",firstIndexTTI);

    rc = cpssDxChTtiPclIdSet(devNum, keyType, pclId);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChTtiPacketTypeKeySizeSet(devNum,keyType,ruleSize);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChTtiPortLookupEnableSet(0, ingressPort, keyType, GT_TRUE);
    if(rc != GT_OK)
        return rc;

/* rule configuration match on:   ingress port   */

/*        UDB for Ethernet keys             */

    /* UDB for source ePort*/
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, 0,
                                       CPSS_DXCH_TTI_OFFSET_METADATA_E, 26);
    if(rc != GT_OK)
        return rc;

    cpssOsMemSet(&mask    , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT    ));
    cpssOsMemSet(&pattern , 0, sizeof(CPSS_DXCH_TTI_RULE_UNT    ));
    cpssOsMemSet(&action  , 0, sizeof(CPSS_DXCH_TTI_ACTION_STC));

    /* source ePort*/
    pattern.udbArray.udb[0]   = ingressPort;
    mask.udbArray.udb[0]      = 0xFF;

    action.command            = CPSS_PACKET_CMD_FORWARD_E;
    action.tag1VlanCmd        = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    action.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 1;

    /* For debug the flow - bind the flow to CNC Counter */
    action.bindToCentralCounter             = GT_TRUE;
    action.centralCounterIndex              = (ipclBlocks+epclBlocks)*1024 + 1;

    action.egressInterface.type             = CPSS_INTERFACE_PORT_E;
    action.egressInterface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum); /* the port is on local device*/;
    action.egressInterface.devPort.portNum  = egressPortTTI;

    action.redirectCommand     = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
    action.bridgeBypass        = GT_TRUE;
    action.ingressPipeBypass   = GT_FALSE;
    action.actionStop          = GT_TRUE;
    action.policerIndex          = policerIndex;
    action.bindToPolicer          = billingEn;

    /* EM match takes precedence over TTI match */
    action.exactMatchOverTtiEn = emOverTtiEn;

    cpssOsPrintf("\n CNC counter for TTI action is %d on rule at index %d\n",
        action.centralCounterIndex, firstIndexTTI);

    rc = cpssDxChTtiRuleSet(devNum, firstIndexTTI, ruleType, &pattern, &mask,
                            &action);
    if(rc != GT_OK)
        return rc;

/***************  EXACT MATCH Configuration ***************************/

    exactMatchLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

/* define default action */
    cpssOsMemSet((GT_VOID*) &actionDefault, 0, sizeof(actionDefault));
    actionDefault.ttiAction.command                 = CPSS_PACKET_CMD_FORWARD_E;
    actionDefault.ttiAction.tag1VlanCmd             = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    actionDefault.ttiAction.userDefinedCpuCode      = CPSS_NET_FIRST_USER_DEFINED_E + 1;
    actionDefault.ttiAction.bindToCentralCounter    = GT_TRUE                               ;
    actionDefault.ttiAction.centralCounterIndex     = (ipclBlocks+epclBlocks)*1024 + 3;
    actionDefault.ttiAction.redirectCommand         = CPSS_DXCH_TTI_NO_REDIRECT_E;
    actionDefault.ttiAction.bridgeBypass            = GT_FALSE;
    actionDefault.ttiAction.ingressPipeBypass       = GT_FALSE;
    actionDefault.ttiAction.actionStop              = GT_FALSE;

    /* define expanded action */
    /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    /* take all fields from Expanded Action: all fields overwriteExpande=GT_FALSE*/
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));

    actionData.ttiAction.command            = CPSS_PACKET_CMD_FORWARD_E;
    actionData.ttiAction.tag1VlanCmd        = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    actionData.ttiAction.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 1;

    /* For debug of the the flow - bind the flow to CNC Counter */
    actionData.ttiAction.bindToCentralCounter               = GT_TRUE;
    actionData.ttiAction.centralCounterIndex                = (ipclBlocks+epclBlocks)*1024 + 2;
    actionData.ttiAction.egressInterface.type               = CPSS_INTERFACE_PORT_E;
    actionData.ttiAction.egressInterface.devPort.hwDevNum   = PRV_CPSS_HW_DEV_NUM_MAC(devNum); /* the port is on local device*/;
    actionData.ttiAction.egressInterface.devPort.portNum    = 0;
    actionData.ttiAction.redirectCommand                    = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
    actionData.ttiAction.bridgeBypass                       = GT_TRUE;
    actionData.ttiAction.ingressPipeBypass                  = GT_FALSE;
    actionData.ttiAction.actionStop                         = GT_TRUE;
    actionData.ttiAction.policerIndex                       = policerIndex;
    actionData.ttiAction.bindToPolicer                      = billingEn;

    /* the outInterface should be taken from reduced entry, all others are taken from "default" action */
    expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionEgressInterface     = GT_TRUE;
    expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionCentralCounterIndex = GT_TRUE;
    expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionBindToPolicer       = GT_TRUE;
    expandedActionOrigin.ttiExpandedActionOrigin.overwriteExpandedActionPolicerIndex        = GT_TRUE;

    /* set Exact Match Entry - Reduced entry */
    cpssOsMemSet(&exactMatchEntry, 0, sizeof(exactMatchEntry));
    exactMatchEntry.key.keySize     =CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
    exactMatchEntry.key.pattern[0]  =ingressPort;  /* ingress port  */
    exactMatchEntry.lookupNum       = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;

    if(useEmManager == GT_FALSE)
    {
        rc = cpssDxChExactMatchExpandedActionSet(devNum, expandedActionIndex,
                                             actionType,
                                             &actionData,
                                             &expandedActionOrigin);
        if (GT_OK != rc)
            return rc;

        /* set first lookup client type */
        firstLookupClientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
        rc = cpssDxChExactMatchClientLookupSet(devNum, exactMatchLookupNum, firstLookupClientType);
        if (GT_OK != rc)
            return rc;

        /* EM profile index mapping table: keyType as in cpssDxChTtiPacketTypeKeySizeSet */
        rc = cpssDxChExactMatchTtiProfileIdModePacketTypeSet(devNum,
                                                             keyType,
                                                             exactMatchLookupNum,
                                                             GT_TRUE, profileIndex);
        if (GT_OK != rc)
            return rc;

        cpssOsMemSet(&keyParams, 0, sizeof(keyParams));
        keyParams.keySize   = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        keyParams.keyStart  =0;
        keyParams.mask[0]   = 0xff; /* match only one byte */

        rc = cpssDxChExactMatchProfileKeyParamsSet(devNum, profileIndex, &keyParams);
        if (GT_OK != rc)
            return rc;

        rc = cpssDxChExactMatchProfileDefaultActionSet(devNum,
                                                    profileIndex,
                                                    actionType,
                                                    &actionDefault,
                                                    GT_TRUE);


        /*   calculate the hash index to put EM entry */
        rc = cpssDxChExactMatchHashCrcMultiResultsByParamsCalc(devNum,
                                                              &exactMatchEntry.key,
                                                              &numberOfElemInCrcMultiHashArr,
                                                              exactMatchCrcMultiHashArr);
        if (GT_OK != rc)
            return rc;

        if(numberOfElemInCrcMultiHashArr!=0)
        {
             index = exactMatchCrcMultiHashArr[0];
             cpssOsPrintf("\n EM entry is stored to index hash result = 0x%x \n", index);
        }
        else
        {
             rc = GT_BAD_VALUE;
             cpssOsPrintf("numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED\n");
             return rc;
        }

        /* same action as set in cpssDxChExactMatchExpandedActionSet, except owerwritten field */
        actionData.ttiAction.egressInterface.devPort.portNum  = egressPortEM;
        actionData.ttiAction.centralCounterIndex              = (ipclBlocks+epclBlocks)*1024 + 5;
        actionData.ttiAction.policerIndex                     = policerIndex+1;
        actionData.ttiAction.bindToPolicer                    = billingEn;
        cpssOsPrintf("\n CNC counter for EM low-level API configuration action is %d \n", actionData.ttiAction.centralCounterIndex);


        rc = cpssDxChExactMatchPortGroupEntrySet(devNum,
                                              CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                              index,
                                              expandedActionIndex,
                                              &exactMatchEntry,
                                              actionType,
                                              &actionData);
        if (GT_OK != rc)
            return rc;
    }
    else /* use EM Manager for the same configuration*/
    {
        CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC          capacityEMM;
        CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC            lookup;
        CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC  entryAttr;
        CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC             aging;
        CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[1];
        CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC             entryEMM;
        CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC  paramsEMM;
        GT_U32                                              numOfBanks;

        cpssOsMemSet(&capacityEMM, 0, sizeof(capacityEMM));
        cpssOsPrintf("exact match number of entries %d, of banks %d\n",
            PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.exactMatchNumOfBanks);

        numOfBanks = PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.exactMatchNumOfBanks;

        capacityEMM.hwCapacity.numOfHwIndexes = _16KB;
        capacityEMM.hwCapacity.numOfHashes    = numOfBanks;
        capacityEMM.maxTotalEntries           = _16KB;


        capacityEMM.maxEntriesPerAgingScan    = 256;
        capacityEMM.maxEntriesPerDeleteScan   = 256;

        cpssOsMemSet(&lookup, 0, sizeof(lookup));
        lookup.lookupsArray[exactMatchLookupNum].lookupClient               = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
        lookup.lookupsArray[exactMatchLookupNum].lookupEnable               = GT_TRUE;
        lookup.lookupsArray[exactMatchLookupNum].lookupClientMappingsNum    = 1;
        lookup.lookupsArray[exactMatchLookupNum].lookupClientMappingsArray[0].ttiMappingElem.keyType                = keyType;
        lookup.lookupsArray[exactMatchLookupNum].lookupClientMappingsArray[0].ttiMappingElem.enableExactMatchLookup = GT_TRUE;
        lookup.lookupsArray[exactMatchLookupNum].lookupClientMappingsArray[0].ttiMappingElem.profileId              = profileIndex;

        lookup.profileEntryParamsArray[profileIndex].keyParams.keySize  = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        lookup.profileEntryParamsArray[profileIndex].keyParams.keyStart =0;
        lookup.profileEntryParamsArray[profileIndex].keyParams.mask[0]  = 0xff; /* match only one byte */
        lookup.profileEntryParamsArray[profileIndex].defaultActionEn    = GT_TRUE;
        lookup.profileEntryParamsArray[profileIndex].defaultActionType  = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
        cpssOsMemCpy (&(lookup.profileEntryParamsArray[profileIndex].defaultAction), &actionDefault, sizeof(CPSS_DXCH_EXACT_MATCH_ACTION_UNT));

        cpssOsMemSet(&entryAttr, 0, sizeof(entryAttr));
        entryAttr.expandedArray[expandedActionIndex].exactMatchExpandedEntryValid = GT_TRUE;
        entryAttr.expandedArray[expandedActionIndex].expandedActionType           = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
        cpssOsMemCpy (&(entryAttr.expandedArray[expandedActionIndex].expandedAction), &actionData, sizeof(CPSS_DXCH_EXACT_MATCH_ACTION_UNT));
        cpssOsMemCpy (&(entryAttr.expandedArray[expandedActionIndex].expandedActionOrigin), &expandedActionOrigin, sizeof(CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT));

        cpssOsMemSet(&aging, 0, sizeof(aging));
        aging.agingRefreshEnable = GT_FALSE;

        rc = cpssDxChExactMatchManagerCreate(exactMatchManagerId,
                                            &capacityEMM,
                                            &lookup,
                                            &entryAttr,
                                            &aging);
        if ((GT_OK != rc) && (rc != GT_ALREADY_EXIST))
            return rc;

        if(rc != GT_ALREADY_EXIST) /*only change priority of EM over TTI*/
        {
            pairListArr[0].devNum        = devNum;
            pairListArr[0].portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
            rc = cpssDxChExactMatchManagerDevListAdd(exactMatchManagerId, pairListArr, 1);
            if (GT_OK != rc)
                return rc;

            /* use the same rule and action as in non-manager configuration */
            cpssOsMemSet(&entryEMM, 0, sizeof(entryEMM));
            cpssOsMemCpy (&(entryEMM.exactMatchEntry), &exactMatchEntry, sizeof(exactMatchEntry));
            entryEMM.exactMatchActionType = CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E;
            cpssOsMemCpy (&(entryEMM.exactMatchAction), &actionData, sizeof(CPSS_DXCH_EXACT_MATCH_ACTION_UNT));
            entryEMM.exactMatchAction.ttiAction.egressInterface.devPort.portNum  = egressPortEM;
            entryEMM.exactMatchAction.ttiAction.centralCounterIndex              = (ipclBlocks+epclBlocks)*1024 + 3;
            entryEMM.exactMatchAction.ttiAction.policerIndex                      = policerIndex+2;
            entryEMM.exactMatchAction.ttiAction.bindToPolicer                     = billingEn;
            entryEMM.expandedActionIndex                                         = expandedActionIndex;
            cpssOsPrintf("\n CNC counter for EM Manager configuration action is %d \n", entryEMM.exactMatchAction.ttiAction.centralCounterIndex);

            cpssOsMemSet(&paramsEMM, 0, sizeof(paramsEMM));
            rc = cpssDxChExactMatchManagerEntryAdd(exactMatchManagerId, &entryEMM, &paramsEMM);
            if (GT_OK != rc)
                return rc;
        }


    }
    return GT_OK;
}


/**
* @internal csRefEmRedirectTtiEmConfigDelete function
* @endinternal
*
* @brief   The function cleans all configurations created by csRefIoamNodeConfigCreate .
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
*/
GT_STATUS csRefEmRedirectTtiEmConfigDelete
(
    IN  GT_U8 devNum
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC entry;
    GT_U32                                  num = 0;

    cpssOsMemSet(&entry, 0, sizeof(entry));
    do
    {
        rc = cpssDxChExactMatchManagerEntryGet(exactMatchManagerId, &entry);
        if ((GT_OK != rc) && (rc != GT_NOT_FOUND))
            return rc;

        if (rc == GT_NOT_FOUND)
            break;
        num++;
        rc = cpssDxChExactMatchManagerEntryDelete(exactMatchManagerId, &entry);
        if (GT_OK != rc)
            return rc;
    }while(1);

    cpssOsPrintf("Deleted %d entries\n", num);

    rc = cpssDxChExactMatchManagerDelete(exactMatchManagerId);
    if ((GT_OK != rc) && (rc != GT_NOT_INITIALIZED))
        return rc;

    rc = cpssDxChExactMatchPortGroupEntryInvalidate(devNum,
                                          CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          index);
    if (GT_OK != rc)
        return rc;

    rc = cpssDxChTtiRuleValidStatusSet(devNum, firstIndexTTI, GT_FALSE);
    if (GT_OK != rc)
        return rc;

    return GT_OK;
}

