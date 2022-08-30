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
* @file cpssDxChExactMatchManager_db.c
*
* @brief Exact Match manager support - manipulations of 'manager DB'
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_utils.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_db.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_hw.h>
#include <cpss/dxCh/dxChxGen/exactMatchManager/private/prvCpssDxChExactMatchManager_debug.h>
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchHash.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/cuckoo/prvCpssDxChCuckoo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/mirror/private/prvCpssDxChMirror.h>
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* sip6 [0-7], sip6_10 [0-15] */
#define PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_SUB_PROFILE_ID_VAL_CNS    15

extern GT_STATUS ttiGetIndexForKeyType
(
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             isLegacyKeyTypeValid,
    OUT GT_U32                              *indexPtr
);

extern GT_STATUS sip5PclUDBEntryIndexGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    OUT GT_U32                               *entryIndexPtr
);

/* macro to allow the 'references' of API to 'see' relation to parameter */
#define EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(paramName,lowLevel_apiName)    /*empty*/

#define EM_NUM_ENTRIES_STEPS_CHECK_MAC(_numEntries)         \
    if(0 == (_numEntries))                               \
    {                                                    \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,      \
            "[%s] must not be ZERO",                     \
            #_numEntries);                               \
    }                                                    \
    if(0 != ((_numEntries) % PRV_EXACT_MATCH_MANAGER_NUM_ENTRIES_STEPS_CNS)) \
    {                                                    \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,      \
            "[%s] [%d] must be value that is multiply value of [%d] (without leftovers)", \
            #_numEntries,                                \
            _numEntries,                                 \
            PRV_EXACT_MATCH_MANAGER_NUM_ENTRIES_STEPS_CNS); \
    }

#define EM_SELF_TEST_BMP_SET_MAC(_exactMatchManagerId, _bitPos)   EM_BMP_SET_MAC(&_exactMatchManagerId->selfTestBitmap, _bitPos)
#define EM_SELF_TEST_BMP_CLR_MAC(_exactMatchManagerId, _bitPos)   EM_BMP_CLEAR_MAC(&_exactMatchManagerId->selfTestBitmap, _bitPos)
#define PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(_resultArr, err, errNumPtr)    \
    {                                                                               \
        _resultArr[*errNumPtr] = err;                                               \
        *errNumPtr += 1;                                                            \
        /* give error to the LOG ... for debug !!! */                               \
        CPSS_LOG_ERROR_MAC("Debug Error number [%d] of Type[%s]",                   \
            *errNumPtr,#err);                                                       \
    }

extern GT_STATUS ttiGetIndexForKeyType
(
    IN  CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType,
    IN  GT_BOOL                             isLegacyKeyTypeValid,
    OUT GT_U32                              *indexPtr
);

extern GT_STATUS sip5PclUDBEntryIndexGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_PCL_PACKET_TYPE_ENT        packetType,
    OUT GT_U32                               *entryIndexPtr
);

/*
* @internal saveEntryFormatParamsToDb_exactMatchEntryTtiFormat_redirectConfig function
* @endinternal
*
* @brief  function to save retirectPtr values of the Exact Match entry into DB in format
*         PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
*/
static GT_STATUS saveEntryFormatParamsToDb_exactMatchEntryTtiFormat_redirectConfig
(
    IN  CPSS_DXCH_TTI_ACTION_STC                                    *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC   *dbEntryPtr
)
{
    /*treat redirectPtr field as:
        routerLttPtr when Redirect_Command="IPNextHop"
        vrfId when Redirect_Command="Assign_VRF_ID"
        flowId when Redirect_Command="NoRedirect"
        TS_ptr when Redirect_Command="Egress_Interface" and TunnelStart="True"
        ARP_ptr when Redirect_Command="Egress_Interface" and TunnelStart="false" */


    /* save to DB format from application format  */
    switch(entryPtr->redirectCommand)
    {
        case CPSS_DXCH_TTI_NO_REDIRECT_E:
            dbEntryPtr->redirectPtr=entryPtr->flowId;
            break;
        case CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E:
            dbEntryPtr->egressInterface_type  = entryPtr->egressInterface.type;
            switch(entryPtr->egressInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    dbEntryPtr->egressInterface.devPort.hwDevNum  = entryPtr->egressInterface.devPort.hwDevNum;
                    dbEntryPtr->egressInterface.devPort.portNum   = entryPtr->egressInterface.devPort.portNum;
                    break;
                case CPSS_INTERFACE_TRUNK_E:
                    dbEntryPtr->egressInterface.trunkId   = entryPtr->egressInterface.trunkId;
                    break;
                case CPSS_INTERFACE_VIDX_E:
                    dbEntryPtr->egressInterface.vidx      = entryPtr->egressInterface.vidx;
                    break;
                default:
                    break;
            }

            dbEntryPtr->tunnelStart  = entryPtr->tunnelStart;
            if (entryPtr->tunnelStart==GT_TRUE)
            {
                 dbEntryPtr->redirectPtr = entryPtr->tunnelStartPtr;
            }
            else
            {
                 dbEntryPtr->redirectPtr = entryPtr->arpPtr;
            }
            break;
        case CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
            dbEntryPtr->redirectPtr=entryPtr->routerLttPtr;
            break;
        case CPSS_DXCH_TTI_VRF_ID_ASSIGN_E:
            dbEntryPtr->redirectPtr=entryPtr->vrfId;
            break;
        default:
            break;
    }

    return GT_OK;
}

/*
* @internal saveEntryFormatParamsToDb_exactMatchEntryPclFormat_redirectConfig function
* @endinternal
*
* @brief  function to save retirectPtr values of the Exact Match entry into DB in format
*         PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
*/
static GT_STATUS saveEntryFormatParamsToDb_exactMatchEntryPclFormat_redirectConfig
(
    IN  CPSS_DXCH_PCL_ACTION_STC                                    *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC   *dbEntryPtr
)
{
    /*treat redirectPtr field as:
        routerLttPtr when Redirect_Command="IPNextHop"
        vrfId when Redirect_Command="Assign_VRF_ID"
        TS_ptr when Redirect_Command="Egress_Interface" and TunnelStart="True"
        ARP_ptr when Redirect_Command="Egress_Interface" and TunnelStart="false" */


    /* save to DB format from application format  */
    switch(entryPtr->redirect.redirectCmd)
    {
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E:
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:
            dbEntryPtr->outInterface_type  = entryPtr->redirect.data.outIf.outInterface.type;
            switch(entryPtr->redirect.data.outIf.outInterface.type)
            {
                case CPSS_INTERFACE_PORT_E:
                    dbEntryPtr->outInterface.devPort.hwDevNum  = entryPtr->redirect.data.outIf.outInterface.devPort.hwDevNum;
                    dbEntryPtr->outInterface.devPort.portNum   = entryPtr->redirect.data.outIf.outInterface.devPort.portNum ;
                    break;
                case CPSS_INTERFACE_TRUNK_E:
                    dbEntryPtr->outInterface.trunkId   = entryPtr->redirect.data.outIf.outInterface.trunkId;
                    break;
                case CPSS_INTERFACE_VIDX_E:
                    dbEntryPtr->outInterface.vidx      = entryPtr->redirect.data.outIf.outInterface.vidx;
                    break;
                default:
                    break;
            }

            dbEntryPtr->tunnelStart  = entryPtr->redirect.data.outIf.tunnelStart;
            if (entryPtr->redirect.data.outIf.tunnelStart==GT_TRUE)
            {
                 dbEntryPtr->redirectPtr = entryPtr->redirect.data.outIf.tunnelPtr;

            }
            else
            {
                 dbEntryPtr->redirectPtr = entryPtr->redirect.data.outIf.arpPtr;

            }

            dbEntryPtr->vntL2Echo = entryPtr->redirect.data.outIf.vntL2Echo;
            dbEntryPtr->tunnelType = entryPtr->redirect.data.outIf.tunnelType;
            dbEntryPtr->modifyMacDa = entryPtr->redirect.data.outIf.modifyMacDa;
            dbEntryPtr->modifyMacSa = entryPtr->redirect.data.outIf.modifyMacSa;

            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E:
            dbEntryPtr->redirectPtr = PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_SW_INDEX_TO_LEAF_HW_INDEX_MAC(entryPtr->redirect.data.routerLttIndex);
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
            dbEntryPtr->redirectPtr=entryPtr->redirect.data.vrfId;
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E:
            dbEntryPtr->redirectPtr=entryPtr->redirect.data.modifyMacSa.arpPtr;

            dbEntryPtr->macSa_low_32 = entryPtr->redirect.data.modifyMacSa.macSa.arEther[2] << 24 |
                         entryPtr->redirect.data.modifyMacSa.macSa.arEther[3] << 16 |
                         entryPtr->redirect.data.modifyMacSa.macSa.arEther[4] <<  8 |
                         entryPtr->redirect.data.modifyMacSa.macSa.arEther[5] <<  0 ;

            dbEntryPtr->macSa_high_16 = entryPtr->redirect.data.modifyMacSa.macSa.arEther[0] <<  8 |
                         entryPtr->redirect.data.modifyMacSa.macSa.arEther[1] <<  0 ;
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E:
            dbEntryPtr->redirectPtr = PRV_CPSS_DXCH_SIP6_CONVERT_LEAF_SW_INDEX_TO_LEAF_HW_INDEX_MAC(entryPtr->redirect.data.routeAndVrfId.routerLttIndex);
            dbEntryPtr->vrfId = entryPtr->redirect.data.routeAndVrfId.vrfId;
            dbEntryPtr->pbrMode     = 0;
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_E:
            dbEntryPtr->redirectPtr = entryPtr->redirect.data.routerLttIndex; /* ecmp table index */
            dbEntryPtr->pbrMode     = 1;
            break;
        case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_AND_ASSIGN_VRF_ID_E:
            dbEntryPtr->redirectPtr = entryPtr->redirect.data.routeAndVrfId.routerLttIndex; /* ecmp table index */
            dbEntryPtr->vrfId = entryPtr->redirect.data.routeAndVrfId.vrfId;
            dbEntryPtr->pbrMode       = 1;
            break;
        default:
            break;
    }

    return GT_OK;
}
/*
* @internal saveEntryFormatParamsToDb_exactMatchEntryTtiFormat function
* @endinternal
*
* @brief  function to save values of the Exact Match entry into DB in format
*         PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
* Note:
* save values that will be set to HW. use ttiActionType2Hw2LogicFormat as reference
*/
GT_STATUS saveEntryFormatParamsToDb_exactMatchEntryTtiFormat
(
    IN  CPSS_DXCH_TTI_ACTION_STC                                    *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC   *dbEntryPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;

    /* save to DB format from application format  */
    dbEntryPtr->tunnelTerminate = entryPtr->tunnelTerminate;
    dbEntryPtr->tsPassengerPacketType = entryPtr->tsPassengerPacketType;
    dbEntryPtr->ttHeaderLength = entryPtr->ttHeaderLength;
    dbEntryPtr->tunnelHeaderLengthAnchorType = entryPtr->tunnelHeaderLengthAnchorType;
    dbEntryPtr->continueToNextTtiLookup = entryPtr->continueToNextTtiLookup;
    dbEntryPtr->copyTtlExpFromTunnelHeader = entryPtr->copyTtlExpFromTunnelHeader;
    dbEntryPtr->mplsCommand = entryPtr->mplsCommand;
    if( entryPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
    {
        /* translate tag 1 VLAN command from action parameter */
        switch (entryPtr->tag1VlanCmd)
        {
        case CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E:
            dbEntryPtr->tag1VlanCmd = 0;
            break;
        case CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E:
            dbEntryPtr->tag1VlanCmd = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        dbEntryPtr->tag1VlanId = entryPtr->tag1VlanId;
    }
    else
    {
        if (entryPtr->mplsCommand != CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
        {
            dbEntryPtr->mplsTtl = entryPtr->mplsTtl;
        }
        dbEntryPtr->enableDecrementTtl = entryPtr->enableDecrementTtl;
    }

    dbEntryPtr->passengerParsingOfTransitMplsTunnelMode = entryPtr->passengerParsingOfTransitMplsTunnelMode;
    dbEntryPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable = entryPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable;

    dbEntryPtr->redirectCommand = entryPtr->redirectCommand;
    rc = saveEntryFormatParamsToDb_exactMatchEntryTtiFormat_redirectConfig(entryPtr,dbEntryPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (entryPtr->redirectCommand==CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        dbEntryPtr->vntl2Echo = entryPtr->vntl2Echo;
        dbEntryPtr->modifyMacSa = entryPtr->modifyMacSa;
        dbEntryPtr->modifyMacDa = entryPtr->modifyMacDa;
    }
    else
    {
        dbEntryPtr->iPclConfigIndex = entryPtr->iPclConfigIndex;
        dbEntryPtr->pcl0OverrideConfigIndex = entryPtr->pcl0OverrideConfigIndex;
        dbEntryPtr->pcl0_1OverrideConfigIndex = entryPtr->pcl0_1OverrideConfigIndex;
        dbEntryPtr->pcl1OverrideConfigIndex  = entryPtr->pcl1OverrideConfigIndex;
        dbEntryPtr->copyReservedAssignmentEnable = entryPtr->copyReservedAssignmentEnable;
        dbEntryPtr->copyReserved = entryPtr->copyReserved;
    }

    dbEntryPtr->triggerHashCncClient = entryPtr->triggerHashCncClient;
    dbEntryPtr->bridgeBypass = entryPtr->bridgeBypass;
    dbEntryPtr->tag0VlanCmd = entryPtr->tag0VlanCmd;
    dbEntryPtr->tag0VlanId = entryPtr->tag0VlanId;
    dbEntryPtr->tag0VlanPrecedence = entryPtr->tag0VlanPrecedence;
    dbEntryPtr->nestedVlanEnable = entryPtr->nestedVlanEnable;
    dbEntryPtr->actionStop = entryPtr->actionStop;
    dbEntryPtr->bindToPolicerMeter  = entryPtr->bindToPolicerMeter;
    dbEntryPtr->bindToPolicer  = entryPtr->bindToPolicer;
    if(entryPtr->bindToPolicer==GT_FALSE&&entryPtr->bindToPolicerMeter==GT_FALSE)
    {
        dbEntryPtr->policerIndex = 0;
    }
    else
    {
        dbEntryPtr->policerIndex = entryPtr->policerIndex;
    }

    dbEntryPtr->qosPrecedence = entryPtr->qosPrecedence;
    dbEntryPtr->keepPreviousQoS = entryPtr->keepPreviousQoS;
    if (entryPtr->keepPreviousQoS==GT_FALSE)
    {
        dbEntryPtr->qosProfile = entryPtr->qosProfile;
    }
    dbEntryPtr->trustUp = entryPtr->trustUp;
    dbEntryPtr->trustDscp = entryPtr->trustDscp;
    dbEntryPtr->trustExp = entryPtr->trustExp;
    dbEntryPtr->modifyTag0Up = entryPtr->modifyTag0Up;
    dbEntryPtr->tag1UpCommand = entryPtr->tag1UpCommand;
    dbEntryPtr->modifyDscp = entryPtr->modifyDscp;
    dbEntryPtr->tag0Up = entryPtr->tag0Up;
    dbEntryPtr->tag1Up = entryPtr->tag1Up;
    dbEntryPtr->remapDSCP = entryPtr->remapDSCP;
    dbEntryPtr->qosUseUpAsIndexEnable = entryPtr->qosUseUpAsIndexEnable;
    dbEntryPtr->qosMappingTableIndex = entryPtr->qosMappingTableIndex;
    dbEntryPtr->mplsLLspQoSProfileEnable = entryPtr->mplsLLspQoSProfileEnable;

    /* translate tunnel termination action forwarding command parameter */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(dbEntryPtr->command,entryPtr->command);

    if (entryPtr->command!=CPSS_PACKET_CMD_FORWARD_E)
    {
        rc = prvCpssDxChNetIfCpuToDsaCode(entryPtr->userDefinedCpuCode,
                                      &dsaCpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }
        dbEntryPtr->userDefinedCpuCode = dsaCpuCode;
    }
    else
    {
        dbEntryPtr->userDefinedCpuCode=0;
    }

    /* check and convert tt passenger packet type */
    switch (entryPtr->ttPassengerPacketType)
    {
    case CPSS_DXCH_TTI_PASSENGER_IPV4V6_E:
        dbEntryPtr->ttPassengerPacketType = 0;
        break;
    case CPSS_DXCH_TTI_PASSENGER_MPLS_E:
        dbEntryPtr->ttPassengerPacketType = 1;
        break;
    case CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E:
        /* not supported for sip6 */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "CPSS_DXCH_TTI_PASSENGER_ETHERNET_CRC_E not supported for sip6. "
                                                    "fail in saveEntryFormatParamsToDb_exactMatchEntryTtiFormat");

    case CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E:
        dbEntryPtr->ttPassengerPacketType = 3;
        break;
    default:
        /* if not TT then TT Passenger Type is not relevant */
        if(entryPtr->tunnelTerminate == GT_FALSE)
        {
            dbEntryPtr->ttPassengerPacketType = 0;
            break;
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ttPassengerPacketType not supported "
                                                    "fail in saveEntryFormatParamsToDb_exactMatchEntryTtiFormat");
    }

    if (entryPtr->iPclUdbConfigTableEnable==GT_FALSE)
    {
        dbEntryPtr->iPclUdbConfigTableIndex = 0;
    }
    else
    {
        switch (entryPtr->iPclUdbConfigTableIndex)
        {
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE_E:
                dbEntryPtr->iPclUdbConfigTableIndex = 1;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E:
                dbEntryPtr->iPclUdbConfigTableIndex = 2;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E:
                dbEntryPtr->iPclUdbConfigTableIndex = 3;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E:
                dbEntryPtr->iPclUdbConfigTableIndex = 4;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E:
                dbEntryPtr->iPclUdbConfigTableIndex = 5;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E:
                dbEntryPtr->iPclUdbConfigTableIndex = 6;
                break;
            case CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E:
                dbEntryPtr->iPclUdbConfigTableIndex = 7;
                break;
            default:
                dbEntryPtr->iPclUdbConfigTableIndex=0;
        }
    }

    dbEntryPtr->sourceEPortAssignmentEnable = entryPtr->sourceEPortAssignmentEnable;
    dbEntryPtr->sourceEPort = entryPtr->sourceEPort;

    dbEntryPtr->bindToCentralCounter = entryPtr->bindToCentralCounter;
    dbEntryPtr->centralCounterIndex = entryPtr->centralCounterIndex;

    dbEntryPtr->hashMaskIndex = entryPtr->hashMaskIndex;
    dbEntryPtr->setMacToMe = entryPtr->setMacToMe;
    dbEntryPtr->rxProtectionSwitchEnable = entryPtr->rxProtectionSwitchEnable;
    dbEntryPtr->rxIsProtectionPath = entryPtr->rxIsProtectionPath;
    dbEntryPtr->pwTagMode = entryPtr->pwTagMode;
    dbEntryPtr->oamProcessEnable = entryPtr->oamProcessEnable;
    dbEntryPtr->oamProcessWhenGalOrOalExistsEnable = entryPtr->oamProcessWhenGalOrOalExistsEnable;
    dbEntryPtr->oamProfile = entryPtr->oamProfile;
    dbEntryPtr->oamChannelTypeToOpcodeMappingEnable = entryPtr->oamChannelTypeToOpcodeMappingEnable;
    dbEntryPtr->isPtpPacket = entryPtr->isPtpPacket;
    if (entryPtr->isPtpPacket==GT_TRUE)
    {
        dbEntryPtr->ptpTriggerType = entryPtr->ptpTriggerType;
        dbEntryPtr->ptpOffset = entryPtr->ptpOffset;
    }
    else
    {
        dbEntryPtr->oamTimeStampEnable = entryPtr->oamTimeStampEnable;
        dbEntryPtr->oamOffsetIndex = entryPtr->oamOffsetIndex;
    }

    dbEntryPtr->ipv6SegmentRoutingEndNodeEnable = entryPtr->ipv6SegmentRoutingEndNodeEnable;
    dbEntryPtr->sourceIdSetEnable = entryPtr->sourceIdSetEnable;
    dbEntryPtr->sourceId = entryPtr->sourceId;

    dbEntryPtr->ingressPipeBypass = entryPtr->ingressPipeBypass;
    dbEntryPtr->mirrorToIngressAnalyzerIndex = entryPtr->mirrorToIngressAnalyzerIndex;
    dbEntryPtr->cwBasedPw = entryPtr->cwBasedPw;
    dbEntryPtr->ttlExpiryVccvEnable = entryPtr->ttlExpiryVccvEnable;
    dbEntryPtr->pwe3FlowLabelExist = entryPtr->pwe3FlowLabelExist;
    dbEntryPtr->pwCwBasedETreeEnable = entryPtr->pwCwBasedETreeEnable;
    dbEntryPtr->applyNonDataCwCommand = entryPtr->applyNonDataCwCommand;
    dbEntryPtr->skipFdbSaLookupEnable = entryPtr->skipFdbSaLookupEnable;
    dbEntryPtr->exactMatchOverTtiEn = entryPtr->exactMatchOverTtiEn;

    return GT_OK;
}

/*
* @internal saveEntryFormatParamsToDb_exactMatchEntryPclFormat function
* @endinternal
*
* @brief  function to save values of the Exact Match entry into DB in format
*         PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
* Note:
* save values that will be set to HW. use sip5PclTcamRuleIngressActionHw2SwConvert as reference
*/
GT_STATUS saveEntryFormatParamsToDb_exactMatchEntryPclFormat
(
    IN  CPSS_DXCH_PCL_ACTION_STC                                    *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC   *dbEntryPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;

    dbEntryPtr->exactMatchOverPclEn = entryPtr->exactMatchOverPclEn;
    dbEntryPtr->bindToCncCounter = entryPtr->matchCounter.enableMatchCount;
    dbEntryPtr->cncCounterIndex = entryPtr->matchCounter.matchCounterIndex;

    dbEntryPtr->oamTimeStampEnable= entryPtr->oam.timeStampEnable;
    dbEntryPtr->oamOffsetIndex = entryPtr->oam.offsetIndex;

    dbEntryPtr->oamProcessEnable = entryPtr->oam.oamProcessEnable;
    dbEntryPtr->oamProfile = entryPtr->oam.oamProfile;
    dbEntryPtr->flowId = entryPtr->flowId;
    dbEntryPtr->actionStop = entryPtr->actionStop;
    dbEntryPtr->bypassBridge = entryPtr->bypassBridge;
    dbEntryPtr->bypassIngressPipe = entryPtr->bypassIngressPipe;

    if((entryPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E) ||
       (entryPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E) ||
       (entryPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E) ||
       (entryPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E) ||
       (entryPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_E) ||
       (entryPtr->redirect.redirectCmd == CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_AND_ASSIGN_VRF_ID_E))
    {
        dbEntryPtr->ipclConfigIndex = entryPtr->lookupConfig.ipclConfigIndex;
        dbEntryPtr->pcl0_1OverrideConfigIndex = entryPtr->lookupConfig.pcl0_1OverrideConfigIndex;
        dbEntryPtr->pcl1OverrideConfigIndex = entryPtr->lookupConfig.pcl1OverrideConfigIndex;
    }
    dbEntryPtr->copyReserved = entryPtr->copyReserved.copyReserved;
    dbEntryPtr->copyReservedAssignEnable = entryPtr->copyReserved.assignEnable;
    dbEntryPtr->triggerHashCncClient = entryPtr->triggerHashCncClient;
    PRV_CPSS_PCL_CONVERT_ATTR_MODIFY_TO_HW_VAL_MAC(dbEntryPtr->modifyDscp, entryPtr->qos.ingress.modifyDscp);
    PRV_CPSS_PCL_CONVERT_ATTR_MODIFY_TO_HW_VAL_MAC(dbEntryPtr->modifyUp, entryPtr->qos.ingress.modifyUp);
    dbEntryPtr->profileIndex = entryPtr->qos.ingress.profileIndex;
    dbEntryPtr->profileAssignIndex = entryPtr->qos.ingress.profileAssignIndex;
    PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(dbEntryPtr->profilePrecedence, entryPtr->qos.ingress.profilePrecedence);
    dbEntryPtr->up1Cmd = entryPtr->qos.ingress.up1Cmd;
    dbEntryPtr->up1 = entryPtr->qos.ingress.up1;
    dbEntryPtr->setMacToMe = entryPtr->setMacToMe;
    dbEntryPtr->skipFdbSaLookup = entryPtr->skipFdbSaLookup;
    dbEntryPtr->triggerInterrupt = entryPtr->triggerInterrupt;

    dbEntryPtr->redirectCmd = entryPtr->redirect.redirectCmd;
    rc = saveEntryFormatParamsToDb_exactMatchEntryPclFormat_redirectConfig(entryPtr,dbEntryPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    dbEntryPtr->sourceIdValue = entryPtr->sourceId.sourceIdValue;
    dbEntryPtr->assignSourceId = entryPtr->sourceId.assignSourceId;
    dbEntryPtr->assignSourcePortEnable = entryPtr->sourcePort.assignSourcePortEnable;
    dbEntryPtr->sourcePortValue = entryPtr->sourcePort.sourcePortValue;
    dbEntryPtr->modifyVlan = entryPtr->vlan.ingress.modifyVlan;
    dbEntryPtr->nestedVlan= entryPtr->vlan.ingress.nestedVlan;
    dbEntryPtr->vlanId= entryPtr->vlan.ingress.vlanId;
    PRV_CPSS_CONVERT_PRECEDENCE_TO_HW_VAL_MAC(dbEntryPtr->precedence, entryPtr->vlan.ingress.precedence);
    dbEntryPtr->vlanId1Cmd= entryPtr->vlan.ingress.vlanId1Cmd;
    dbEntryPtr->vlanId1= entryPtr->vlan.ingress.vlanId1;
    dbEntryPtr->monitoringEnable = entryPtr->latencyMonitor.monitoringEnable;
    dbEntryPtr->latencyProfile = entryPtr->latencyMonitor.latencyProfile;

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(dbEntryPtr->pktCmd, entryPtr->pktCmd);
    if (entryPtr->pktCmd != CPSS_PACKET_CMD_FORWARD_E)
    {
        rc = prvCpssDxChNetIfCpuToDsaCode(entryPtr->mirror.cpuCode, &dsaCpuCode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "The action.pktCmd[%d] got invalid action.mirror.cpuCode[%d]",
                entryPtr->pktCmd ,
                entryPtr->mirror.cpuCode);
        }
        dbEntryPtr->cpuCode = dsaCpuCode;
    }
    else
    {
        dbEntryPtr->cpuCode=0;
    }

    switch (entryPtr->policer.policerEnable)
    {
    case CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E:
         dbEntryPtr->bindToPolicerMeter=0;
         dbEntryPtr->bindToPolicerCounter=0;
        break;
    case CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E:
         dbEntryPtr->bindToPolicerMeter=1;
         dbEntryPtr->bindToPolicerCounter=1;
        break;
    case CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E:
         dbEntryPtr->bindToPolicerMeter=1;
         dbEntryPtr->bindToPolicerCounter=0;
        break;
    case CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E:
         dbEntryPtr->bindToPolicerMeter=0;
         dbEntryPtr->bindToPolicerCounter=1;
        break;
    default:
        break;
    }
    dbEntryPtr->policerIndex = entryPtr->policer.policerId;

    dbEntryPtr->mirrorTcpRstAndFinPacketsToCpu = entryPtr->mirror.mirrorTcpRstAndFinPacketsToCpu;
    if (entryPtr->mirror.mirrorToRxAnalyzerPort==GT_FALSE)
    {
        dbEntryPtr->ingressMirrorToAnalyzerIndex = 0;
    }
    else
    {
        dbEntryPtr->ingressMirrorToAnalyzerIndex = entryPtr->mirror.ingressMirrorToAnalyzerIndex+1;
    }

    return GT_OK;
}

/*
* @internal saveEntryFormatParamsToDb_exactMatchEntryEpclFormat function
* @endinternal
*
* @brief  function to save values of the Exact Match entry into DB in format
*         PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC
*         copy values from application format to manager format
*
* @param[in] entryPtr               - (pointer to) the entry (application format)
* @param[out] dbEntryPtr            - (pointer to) the manager entry format (manager format)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @retval none
* * Note:
* save values that will be set to HW. use sip5PclTcamRuleEgressActionHw2SwConvert as reference
*/
GT_STATUS saveEntryFormatParamsToDb_exactMatchEntryEpclFormat
(
    IN  CPSS_DXCH_PCL_ACTION_STC                                    *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_EPCL_FORMAT_STC  *dbEntryPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;

    dbEntryPtr->exactMatchOverPclEn = entryPtr->exactMatchOverPclEn;

    dbEntryPtr->bindToCncCounter = entryPtr->matchCounter.enableMatchCount;
    dbEntryPtr->cncCounterIndex = entryPtr->matchCounter.matchCounterIndex;

    dbEntryPtr->oamTimeStampEnable= entryPtr->oam.timeStampEnable;
    dbEntryPtr->oamOffsetIndex = entryPtr->oam.offsetIndex;

    dbEntryPtr->oamProcessEnable = entryPtr->oam.oamProcessEnable;
    dbEntryPtr->oamProfile = entryPtr->oam.oamProfile;

    dbEntryPtr->flowId = entryPtr->flowId;
    dbEntryPtr->copyReservedAssignEnable = entryPtr->copyReserved.assignEnable;
    dbEntryPtr->copyReserved = entryPtr->copyReserved.copyReserved;

    dbEntryPtr->modifyDscp = entryPtr->qos.egress.modifyDscp;
    dbEntryPtr->dscp = entryPtr->qos.egress.dscp;
    dbEntryPtr->modifyUp = entryPtr->qos.egress.modifyUp;
    dbEntryPtr->up = entryPtr->qos.egress.up;
    dbEntryPtr->up1ModifyEnable = entryPtr->qos.egress.up1ModifyEnable;
    dbEntryPtr->up1 = entryPtr->qos.egress.up1;

    dbEntryPtr->vlanCmd = entryPtr->vlan.egress.vlanCmd;
    dbEntryPtr->vlanId1ModifyEnable = entryPtr->vlan.egress.vlanId1ModifyEnable;
    dbEntryPtr->vlanId = entryPtr->vlan.egress.vlanId;
    dbEntryPtr->vlanId1= entryPtr->vlan.egress.vlanId1;

    dbEntryPtr->channelTypeToOpcodeMapEnable = entryPtr->channelTypeToOpcodeMapEnable;
    dbEntryPtr->terminateCutThroughMode =  entryPtr->terminateCutThroughMode;

    dbEntryPtr->monitoringEnable = entryPtr->latencyMonitor.monitoringEnable;
    dbEntryPtr->latencyProfile = entryPtr->latencyMonitor.latencyProfile;

    dbEntryPtr->egressCncIndexMode = entryPtr->egressCncIndexMode;
    dbEntryPtr->enableEgressMaxSduSizeCheck = entryPtr->enableEgressMaxSduSizeCheck;
    dbEntryPtr->egressMaxSduSizeProfile = entryPtr->egressMaxSduSizeProfile;

    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(dbEntryPtr->pktCmd, entryPtr->pktCmd);
    if (entryPtr->pktCmd != CPSS_PACKET_CMD_FORWARD_E)
    {
        rc = prvCpssDxChNetIfCpuToDsaCode(entryPtr->mirror.cpuCode, &dsaCpuCode);
        if (rc != GT_OK)
        {
            return rc;
        }
        dbEntryPtr->cpuCode = dsaCpuCode;
    }
    else
    {
        dbEntryPtr->cpuCode=0;
    }

    switch (entryPtr->policer.policerEnable)
    {
    case CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E:
         dbEntryPtr->bindToPolicerMeter=0;
         dbEntryPtr->bindToPolicerCounter=0;
        break;
    case CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E:
         dbEntryPtr->bindToPolicerMeter=1;
         dbEntryPtr->bindToPolicerCounter=1;
        break;
    case CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E:
         dbEntryPtr->bindToPolicerMeter=1;
         dbEntryPtr->bindToPolicerCounter=0;
        break;
    case CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E:
         dbEntryPtr->bindToPolicerMeter=0;
         dbEntryPtr->bindToPolicerCounter=1;
        break;
    default:
        break;
    }
    dbEntryPtr->policerIndex = entryPtr->policer.policerId;

    if (entryPtr->mirror.mirrorToTxAnalyzerPortEn==GT_FALSE)
    {
        dbEntryPtr->egressMirrorToAnalyzerIndex = 0;
    }
    else
    {
        dbEntryPtr->egressMirrorToAnalyzerIndex = entryPtr->mirror.egressMirrorToAnalyzerIndex+1;
    }

    PRV_CPSS_DXCH_MIRROR_CONVERT_EGRESS_MIRROR_MODE_TO_HW_VAL_MAC((dbEntryPtr->egressMirrorToAnalyzerMode), (entryPtr->mirror.egressMirrorToAnalyzerMode));

    switch(entryPtr->epclPhaInfo.phaThreadIdAssignmentMode)
    {
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_DISABLED_E:
            dbEntryPtr->phaThreadNumberAssignmentEnable = 0;
            dbEntryPtr->phaThreadNumber =  0;
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E:
            dbEntryPtr->phaThreadNumberAssignmentEnable = 1;
            dbEntryPtr->phaThreadNumber =  entryPtr->epclPhaInfo.phaThreadId;
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_BYPASS_PHA_E:
            dbEntryPtr->phaThreadNumberAssignmentEnable = 1;
            dbEntryPtr->phaThreadNumber =  0;
            break;
        default:
            break;
    }

    dbEntryPtr->phaThreadType = entryPtr->epclPhaInfo.phaThreadType;
    switch(entryPtr->epclPhaInfo.phaThreadType)
    {
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E:
            dbEntryPtr->phaMetadataAssignEnable = 0;
            dbEntryPtr->phaMetadata = 0;
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E:
            dbEntryPtr->phaMetadataAssignEnable = 1;
            /* set the most significant bit */
            dbEntryPtr->phaMetadata = BOOL2BIT_MAC(entryPtr->epclPhaInfo.phaThreadUnion.epclIoamIngressSwitch.ioamIncludesBothE2EOptionAndTraceOption) << 31;
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INT_IOAM_MIRROR_E:
            dbEntryPtr->phaMetadataAssignEnable = 1;
            /* set the most significant bit - Indicates if payload is ethernet */
            dbEntryPtr->phaMetadata = BOOL2BIT_MAC(entryPtr->epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_P) << 31;

            /* frameType[30:26] */
            if (entryPtr->epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_FT)
            {
                dbEntryPtr->phaMetadata |= ((entryPtr->epclPhaInfo.phaThreadUnion.epclIntIoamMirror.erspanIII_FT & 0x1F) << 26);
            }
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE_E:
            dbEntryPtr->phaMetadataAssignEnable = 1;
            dbEntryPtr->phaMetadata =  0;

            /* Updates Desc<pha metadata>[31:16] which represents TenantID field in NSH metadata */
            if (entryPtr->epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_TenantID)
            {
                dbEntryPtr->phaMetadata = ((entryPtr->epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_TenantID & 0xFFFF)<<16);
            }

            /* Updates Desc<pha metadata>[15:0] which represents SourceClass field in NSH metadata */
            if (entryPtr->epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_SourceClass)
            {
                dbEntryPtr->phaMetadata |= (entryPtr->epclPhaInfo.phaThreadUnion.epclClassifierNshOverVxlanGpe.classifierNshOverVxlanGpe_SourceClass & 0xFFFF);
            }
            break;
        case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_E:
            dbEntryPtr->phaMetadataAssignEnable = 1;
            /* set the LSB 3 bits for ERSPAN analyzer index */
            dbEntryPtr->phaMetadata = ((entryPtr->epclPhaInfo.phaThreadUnion.epclErspanTypeIImirror.erspanAnalyzerIndex & 0x7));
            break;

         default:
            break;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbValidateTtiAction function
* @endinternal
*
* @brief   validate the Exact Match TTI Action
*
* @param[in] logicFormatPtr        - points to TTI action in logic format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*
* Note:
*   for fields that are not set to HW due to disable flag, a
*   default value will be set instead of the configured one
*/
GT_STATUS prvCpssDxChExactMatchManagerDbValidateTtiAction
(
    IN  CPSS_DXCH_TTI_ACTION_STC   *logicFormatPtr
)
{
    GT_U32      value;

    CPSS_NULL_PTR_CHECK_MAC(logicFormatPtr);

    /*****************/
    /* check params  */
    /*****************/

    /* check and convert tt passenger packet type */
    switch (logicFormatPtr->ttPassengerPacketType)
    {
    case CPSS_DXCH_TTI_PASSENGER_IPV4V6_E:
    case CPSS_DXCH_TTI_PASSENGER_MPLS_E:
    case CPSS_DXCH_TTI_PASSENGER_ETHERNET_NO_CRC_E:
        break;
    default:
        /* if not TT then TT Passenger Type is not relevant */
        if(logicFormatPtr->tunnelTerminate == GT_FALSE)
        {
            /* reset field for future use - not relevant */
            logicFormatPtr->ttPassengerPacketType=0;
            break;
        }
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->ttPassengerPacketType);
    }

    /* in MPLS LSR functionality, tt passenger type must be MPLS */
    if (logicFormatPtr->tunnelTerminate == GT_FALSE)
    {
        if ((logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_POP1_CMD_E ||
            logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_POP2_CMD_E ||
            logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E))
        {
            if (logicFormatPtr->ttPassengerPacketType != CPSS_DXCH_TTI_PASSENGER_MPLS_E)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "logicFormatPtr->ttPassengerPacketType[%d]",
                        logicFormatPtr->ttPassengerPacketType);
            }
        }
    }

    if ((logicFormatPtr->tunnelTerminate == GT_FALSE) &&
        (logicFormatPtr->tunnelHeaderLengthAnchorType == CPSS_DXCH_TUNNEL_HEADER_LENGTH_L4_ANCHOR_TYPE_E))
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "logicFormatPtr->tunnelTerminate[%d] can not be FALSE with the "
                        "logicFormatPtr->tunnelHeaderLengthAnchorType[%d] as CPSS_DXCH_TUNNEL_HEADER_LENGTH_L4_ANCHOR_TYPE_E ",
                        logicFormatPtr->tunnelTerminate,logicFormatPtr->tunnelHeaderLengthAnchorType);
    }

    /* check and convert ts passenger type */
    if (logicFormatPtr->redirectCommand == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E &&
        logicFormatPtr->tunnelStart == GT_TRUE)
    {
        if ((logicFormatPtr->tunnelTerminate == GT_FALSE) &&
            (logicFormatPtr->mplsCommand != CPSS_DXCH_TTI_MPLS_NOP_CMD_E))
        {
            if (logicFormatPtr->tsPassengerPacketType != CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E)
            {
               CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "illegal case when :"
                        "logicFormatPtr->redirectCommand[%d] and logicFormatPtr->tunnelStart[%d] and"
                        "logicFormatPtr->tunnelTerminate[%d] and logicFormatPtr->mplsCommand[%d]!=CPSS_DXCH_TTI_MPLS_NOP_CMD_E"
                        "and logicFormatPtr->tsPassengerPacketType[%d]!=CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E ",
                        logicFormatPtr->redirectCommand,logicFormatPtr->tunnelStart,logicFormatPtr->tunnelTerminate,
                        logicFormatPtr->mplsCommand,logicFormatPtr->tsPassengerPacketType);
            }
        }

        /* modifyMacDa == GT_TRUE requires that tunnelStart == GT_FALSE*/
        if (logicFormatPtr->modifyMacDa == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "illegal case when :"
                        "logicFormatPtr->modifyMacDa[%d]==GT_TRUE and logicFormatPtr->tunnelStart[%d]==GT_TRUE",
                        logicFormatPtr->modifyMacDa,logicFormatPtr->tunnelStart);
        }

        switch (logicFormatPtr->tsPassengerPacketType)
        {
        case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
        case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->tsPassengerPacketType);
        }
    }

    /* check and convert mpls command */
    switch (logicFormatPtr->mplsCommand)
    {
    case CPSS_DXCH_TTI_MPLS_NOP_CMD_E:
    case CPSS_DXCH_TTI_MPLS_SWAP_CMD_E:
    case CPSS_DXCH_TTI_MPLS_PUSH1_CMD_E:
    case CPSS_DXCH_TTI_MPLS_POP1_CMD_E:
    case CPSS_DXCH_TTI_MPLS_POP2_CMD_E:
    case CPSS_DXCH_TTI_MPLS_POP3_CMD_E:
    case CPSS_DXCH_TTI_MPLS_POP_AND_SWAP_CMD_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->mplsCommand);
    }

    if (logicFormatPtr->mplsCommand != CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
    {
        if (logicFormatPtr->mplsTtl > TTL_MAX_CNS)
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->mplsCommand[%d] != CPSS_DXCH_TTI_MPLS_NOP_CMD_E and logicFormatPtr->mplsTtl[%d]",
                        logicFormatPtr->mplsCommand,logicFormatPtr->mplsTtl);
        }
    }
    else
    {
        /* logicFormatPtr->mplsTtl is not relevant need to reset it  */
        logicFormatPtr->mplsTtl = 0;
    }

    /* translate tunnel termination action redirect command parameter */
    switch (logicFormatPtr->redirectCommand)
    {
    case CPSS_DXCH_TTI_NO_REDIRECT_E:
    case CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E:
    case CPSS_DXCH_TTI_LOGICAL_PORT_ASSIGN_E:
        /* in this case routerLttPtr and vrfId are not relevant - reset it */
        logicFormatPtr->routerLttPtr=0;
        logicFormatPtr->vrfId=0;
        break;
    case CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
        /* in this case vrfId is not relevant - reset it */
        logicFormatPtr->vrfId=0;
        if(logicFormatPtr->routerLttPtr >= BIT_15)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->routerLttPtr[%d]", logicFormatPtr->routerLttPtr);
        }
        break;
    case CPSS_DXCH_TTI_VRF_ID_ASSIGN_E:
        /* in this case routerLttPtr is not relevant - reset it */
        logicFormatPtr->routerLttPtr=0;
        if (logicFormatPtr->vrfId > VRF_ID_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->vrfId[%d]", logicFormatPtr->vrfId);
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->redirectCommand);
    }

    if(logicFormatPtr->redirectCommand == CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        switch (logicFormatPtr->egressInterface.type)
        {
        case CPSS_INTERFACE_PORT_E:
            PRV_CPSS_DXCH_DUAL_HW_DEVICE_AND_PORT_CHECK_MAC(
                logicFormatPtr->egressInterface.devPort.hwDevNum,
                logicFormatPtr->egressInterface.devPort.portNum);
            break;
        case CPSS_INTERFACE_TRUNK_E:
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->egressInterface.type);
        }
    }
    else
    {
        /* reset all fields that are not relevant */
        cpssOsMemSet(&logicFormatPtr->egressInterface,0,sizeof(logicFormatPtr->egressInterface));
    }

    if (logicFormatPtr->sourceId >= BIT_12)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->sourceId[%d]", logicFormatPtr->sourceId);

    switch (logicFormatPtr->tag0VlanCmd)
    {
    case CPSS_DXCH_TTI_VLAN_DO_NOT_MODIFY_E:
    case CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E:
    case CPSS_DXCH_TTI_VLAN_MODIFY_TAGGED_E:
    case CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->tag0VlanCmd);
    }

    /* the real value supported (12 bits or 16 bits) will be chacked when device is added */
    if (logicFormatPtr->tag0VlanCmd >= BIT_16)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->tag0VlanCmd[%d]", logicFormatPtr->tag0VlanCmd);

    if( logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
    {
        switch (logicFormatPtr->tag1VlanCmd)
        {
        case CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E:
        case CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->tag1VlanCmd);
        }

       /*   the tag1VlanId  is checked with PRV_CPSS_VLAN_VALUE_CHECK_MAC  and
            not with PRV_CPSS_DXCH_VLAN_VALUE_CHECK_MAC , because tag1VlanId not
            supporting enhanced vid */
        if (logicFormatPtr->tag1VlanId >= MAX_NUM_VLANS_CNS)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->tag1VlanId[%d]", logicFormatPtr->tag1VlanId);

        /* reset field to keep in DB - not relevant */
        logicFormatPtr->enableDecrementTtl=0;
        logicFormatPtr->mplsTtl = 0;
    }
    else
    {
        /* reset field to keep in DB - not relevant */
        logicFormatPtr->tag1VlanCmd=0;
        logicFormatPtr->tag1VlanId=0;

        if (logicFormatPtr->mplsCommand == CPSS_DXCH_TTI_MPLS_NOP_CMD_E)
        {
            /* reset field to keep in DB - not relevant */
            logicFormatPtr->mplsTtl = 0;
        }
    }

    /* translate VLAN precedence from action parameter */
    switch (logicFormatPtr->tag0VlanPrecedence)
    {
    case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:
    case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:
        break;
    default:
       CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->tag0VlanPrecedence);
    }

    if (logicFormatPtr->bindToPolicerMeter == GT_TRUE ||
        logicFormatPtr->bindToPolicer == GT_TRUE)
    {
        if (logicFormatPtr->policerIndex >= BIT_16)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->policerIndex[%d]", logicFormatPtr->policerIndex);
    }
    else
    {
        /* in this case policerIndex is not relevant - reset it */
        logicFormatPtr->policerIndex=0;
    }

    switch (logicFormatPtr->qosPrecedence)
    {
    case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:
    case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:
       break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->qosPrecedence);
    }

    if (logicFormatPtr->keepPreviousQoS == GT_FALSE)
    {
        if (logicFormatPtr->qosProfile > TTI_QOS_PROFILE_E_ARCH_MAX_CNS)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->qosProfile[%d]", logicFormatPtr->qosProfile);
    }
    else
    {
        /* in this case qosProfile is not relevant - reset it */
        logicFormatPtr->qosProfile=0;
    }

    switch (logicFormatPtr->modifyTag0Up)
    {
    case CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_UP_E:
    case CPSS_DXCH_TTI_MODIFY_UP_ENABLE_E:
    case CPSS_DXCH_TTI_MODIFY_UP_DISABLE_E:
        break;
    default:
         CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->modifyTag0Up);
    }

    /* translate tunnel termination tag1 up command parameter */
    switch (logicFormatPtr->tag1UpCommand)
    {
    case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN1_UNTAGGED_E:
    case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_VLAN0_UNTAGGED_E:
    case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_ALL_E:
    case CPSS_DXCH_TTI_TAG1_UP_ASSIGN_NONE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->tag1UpCommand);
    }

    switch (logicFormatPtr->modifyDscp)
    {
    case CPSS_DXCH_TTI_DO_NOT_MODIFY_PREV_DSCP_E:
    case CPSS_DXCH_TTI_MODIFY_DSCP_ENABLE_E:
    case CPSS_DXCH_TTI_MODIFY_DSCP_DISABLE_E:
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->modifyDscp);
    }

    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->tag0Up);
    PRV_CPSS_DXCH_COS_CHECK_UP_MAC(logicFormatPtr->tag1Up);

    if (logicFormatPtr->hashMaskIndex >= BIT_4)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->hashMaskIndex[%d]", logicFormatPtr->hashMaskIndex);
    }

    if (logicFormatPtr->redirectCommand != CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E)
    {
        if(logicFormatPtr->copyReserved >= BIT_12)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"logicFormatPtr->copyReserved[%d]",
                                        logicFormatPtr->copyReserved);
        }
        switch (logicFormatPtr->pcl0OverrideConfigIndex)
        {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->pcl0OverrideConfigIndex);
        }

        switch (logicFormatPtr->pcl0_1OverrideConfigIndex)
        {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->pcl0_1OverrideConfigIndex);
        }

        switch (logicFormatPtr->pcl1OverrideConfigIndex)
        {
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
        case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
            break;
        default:
           CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->pcl1OverrideConfigIndex);
        }

        if (logicFormatPtr->iPclConfigIndex >= (_4K + 1024))  /*5120*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->iPclConfigIndex[%d]", logicFormatPtr->iPclConfigIndex);

        /* reset field to keep in DB - not relevant */
        logicFormatPtr->vntl2Echo = 0;
        logicFormatPtr->modifyMacSa = 0;
        logicFormatPtr->modifyMacDa = 0;
    }
    else
    {
        /* reset field to keep in DB - not relevant */
        logicFormatPtr->iPclConfigIndex=0;
        logicFormatPtr->pcl0OverrideConfigIndex = 0;
        logicFormatPtr->pcl0_1OverrideConfigIndex = 0;
        logicFormatPtr->pcl1OverrideConfigIndex  = 0;
    }

    if ((logicFormatPtr->command == CPSS_PACKET_CMD_MIRROR_TO_CPU_E) ||
        (logicFormatPtr->command == CPSS_PACKET_CMD_TRAP_TO_CPU_E) ||
        (logicFormatPtr->command == CPSS_PACKET_CMD_DROP_HARD_E)||
        (logicFormatPtr->command == CPSS_PACKET_CMD_DROP_SOFT_E))
    {
        if ((logicFormatPtr->userDefinedCpuCode < CPSS_NET_FIRST_USER_DEFINED_E) &&
            (logicFormatPtr->userDefinedCpuCode > CPSS_NET_LAST_USER_DEFINED_E))
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->userDefinedCpuCode[%d]", logicFormatPtr->userDefinedCpuCode);
    }
    else
    {
        /* in this case userDefinedCpuCode is not relevant - reset it */
        logicFormatPtr->userDefinedCpuCode=0;
    }

    if(logicFormatPtr->mirrorToIngressAnalyzerEnable == GT_TRUE)
    {
        if(0 == CHECK_BITS_DATA_RANGE_MAC(logicFormatPtr->mirrorToIngressAnalyzerIndex + 1, 3))
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->mirrorToIngressAnalyzerIndex[%d]", logicFormatPtr->mirrorToIngressAnalyzerIndex);
        }
    }
    else
    {
        /* in this case mirrorToIngressAnalyzerIndex is not relevant - reset it */
        logicFormatPtr->mirrorToIngressAnalyzerIndex=0;
    }

    switch (logicFormatPtr->redirectCommand)
    {
    case CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E:
        switch (logicFormatPtr->egressInterface.type)
        {
        case CPSS_INTERFACE_PORT_E:
        case CPSS_INTERFACE_TRUNK_E:
        case CPSS_INTERFACE_VIDX_E:
        case CPSS_INTERFACE_VID_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->egressInterface.type);
        }
        break;
    case CPSS_DXCH_TTI_REDIRECT_TO_ROUTER_LOOKUP_E:
    case CPSS_DXCH_TTI_VRF_ID_ASSIGN_E:
        /* in this case egressInterface is not relevant - reset it */
        cpssOsMemSet(&logicFormatPtr->egressInterface,0,sizeof(logicFormatPtr->egressInterface));
        break;
    case CPSS_DXCH_TTI_NO_REDIRECT_E:
        if (logicFormatPtr->iPclUdbConfigTableEnable == GT_TRUE)
        {
            switch (logicFormatPtr->iPclUdbConfigTableIndex)
            {
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE_E:
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E:
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E:
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E:
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E:
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E:
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->iPclUdbConfigTableIndex);
            }
        }
        else
        {
            /* in this case iPclUdbConfigTableIndex is not relevant - reset it */
            logicFormatPtr->iPclUdbConfigTableIndex=0;
        }
        /* in this case egressInterface is not relevant - reset it */
        cpssOsMemSet(&logicFormatPtr->egressInterface,0,sizeof(logicFormatPtr->egressInterface));
        break;
    default:
        /* do nothing */
        /* in this case egressInterface is not relevant - reset it */
        cpssOsMemSet(&logicFormatPtr->egressInterface,0,sizeof(logicFormatPtr->egressInterface));
        logicFormatPtr->routerLttPtr=0;
        logicFormatPtr->vrfId=0;
        break;
    }

    if (logicFormatPtr->qosUseUpAsIndexEnable == GT_TRUE)
    {
        if (logicFormatPtr->qosMappingTableIndex >= QOS_MAPPING_TABLE_INDEX_MAX_CNS)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->qosMappingTableIndex[%d]", logicFormatPtr->qosMappingTableIndex);
        }
    }
    else
    {
        /* in this case qosMappingTableIndex is not relevant - reset it */
        logicFormatPtr->qosMappingTableIndex=0;
    }

    if (logicFormatPtr->isPtpPacket == GT_TRUE)
    {
        switch (logicFormatPtr->ptpTriggerType)
        {
        case CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_L2_E:
        case CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E:
         case CPSS_DXCH_TTI_PTP_TRIGGER_TYPE_PTP_OVER_IPV6_UDP_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->ptpTriggerType);
        }
    }
    else
    {
        /* in this case ptpTriggerType is not relevant - reset it */
        logicFormatPtr->ptpTriggerType=0;
        logicFormatPtr->ptpOffset=0;
    }

    switch (logicFormatPtr->passengerParsingOfTransitMplsTunnelMode)
    {
        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_DISABLE_E:
        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_IP_E:
        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_ETH_E:
        case CPSS_DXCH_TTI_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_MODE_AS_CW_ETH_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->passengerParsingOfTransitMplsTunnelMode);
    }

    value = BOOL2BIT_MAC(logicFormatPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable);

    /* When setting <Passenger Parsing of Non-MPLS Transit tunnel Enable> == 1 and
       <TT Passenger Type> == MPLS --> unexpected behavior */
    if (value && logicFormatPtr->ttPassengerPacketType == CPSS_DXCH_TTI_PASSENGER_MPLS_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal: When setting <Passenger Parsing of Non-MPLS Transit tunnel Enable> == 1 and"
       "<TT Passenger Type> == MPLS --> unexpected behavior logicFormatPtr->ttPassengerPacketType[%d,"
       " logicFormatPtr->passengerParsingOfTransitNonMplsTransitTunnelEnable[%d]", logicFormatPtr->ttPassengerPacketType, value);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbValidatePclAction function
* @endinternal
*
* @brief   validate the Exact Match PCL/EPCL Action
*
* @param[in] logicFormatPtr        - points to PCL action in logic format
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChExactMatchManagerDbValidatePclAction
(
    IN  CPSS_DXCH_PCL_ACTION_STC   *logicFormatPtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;
    GT_BOOL useIpclProfileIndexAndEnablers = GT_FALSE;
    GT_BOOL usePolicer = GT_TRUE;

    switch (logicFormatPtr->pktCmd)
    {
        case CPSS_PACKET_CMD_FORWARD_E:
        case CPSS_PACKET_CMD_MIRROR_TO_CPU_E:
        case CPSS_PACKET_CMD_TRAP_TO_CPU_E:
        case CPSS_PACKET_CMD_DROP_HARD_E:
        case CPSS_PACKET_CMD_DROP_SOFT_E:
            break;
        default:
               CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->pktCmd);
    }
    if (logicFormatPtr->pktCmd != CPSS_PACKET_CMD_FORWARD_E)
    {
        /* in Bobcat2, Caelum, Bobcat3 the CPU code is also 'drop code' so relevant for drops too. */
        rc = prvCpssDxChNetIfCpuToDsaCode(logicFormatPtr->mirror.cpuCode, &dsaCpuCode);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"logicFormatPtr->mirror.cpuCode[%d]",logicFormatPtr->mirror.cpuCode);
        }
    }
    else
    {
        logicFormatPtr->mirror.cpuCode=0;
    }
    if(logicFormatPtr->mirror.mirrorToRxAnalyzerPort == GT_TRUE)
    {

        if (logicFormatPtr->mirror.ingressMirrorToAnalyzerIndex>INGRESS_MIRROE_TO_ANALYZER_INDEX_MAX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"logicFormatPtr->mirror.ingressMirrorToAnalyzerIndex[%d] applicable range (0-6)",
                                          logicFormatPtr->mirror.ingressMirrorToAnalyzerIndex);
        }
    }

    if (logicFormatPtr->matchCounter.matchCounterIndex >= BIT_15)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->matchCounter.matchCounterIndex[%d]", logicFormatPtr->matchCounter.matchCounterIndex);

    if(logicFormatPtr->copyReserved.copyReserved >= BIT_19)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"logicFormatPtr->copyReserved.copyReserved[%d]",
                                        logicFormatPtr->copyReserved.copyReserved);
    }

    if(logicFormatPtr->egressPolicy == GT_FALSE)
    {
        /* check ingress PCL validity*/
        switch(logicFormatPtr->redirect.redirectCmd)
        {
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E:
                useIpclProfileIndexAndEnablers = GT_TRUE;
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E:
                switch (logicFormatPtr->redirect.data.outIf.outInterface.type)
                {
                    case CPSS_INTERFACE_PORT_E:
                    case CPSS_INTERFACE_TRUNK_E:
                    case CPSS_INTERFACE_VIDX_E:
                    case CPSS_INTERFACE_VID_E:
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->redirect.data.outIf.outInterface.type);
                }

                if (logicFormatPtr->redirect.data.outIf.tunnelStart==GT_TRUE)/* tunnel start */
                {
                    switch (logicFormatPtr->redirect.data.outIf.tunnelType)
                    {
                        case CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_ETHERNET_E:
                        case CPSS_DXCH_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E:
                            break;
                        default:
                            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->redirect.data.outIf.tunnelType);
                    }
                }
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_E: /* Fall Through */
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_E:
                useIpclProfileIndexAndEnablers = GT_TRUE;
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
                useIpclProfileIndexAndEnablers = GT_TRUE;
                if (logicFormatPtr->redirect.data.vrfId >= BIT_12)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"logicFormatPtr->redirect.data.vrfId[%d]",
                                              logicFormatPtr->redirect.data.vrfId);
                }
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_REPLACE_MAC_SA_E:
                useIpclProfileIndexAndEnablers = GT_FALSE;
                usePolicer = GT_FALSE;
                break;
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ECMP_AND_ASSIGN_VRF_ID_E: /* Fall Through */
            case CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_ROUTER_AND_ASSIGN_VRF_ID_E:
                useIpclProfileIndexAndEnablers = GT_TRUE;

                if (logicFormatPtr->redirect.data.routeAndVrfId.vrfId >= BIT_12)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"logicFormatPtr->redirect.data.routeAndVrfId.vrfId[%d]",
                                              logicFormatPtr->redirect.data.routeAndVrfId.vrfId);
                }
                break;
            default:
                 CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->redirect.redirectCmd);
        }

        if(useIpclProfileIndexAndEnablers == GT_TRUE)
        {
            switch (logicFormatPtr->lookupConfig.pcl0_1OverrideConfigIndex)
            {
            case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->lookupConfig.pcl0_1OverrideConfigIndex);
            }

            switch (logicFormatPtr->lookupConfig.pcl1OverrideConfigIndex)
            {
            case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_RETAIN_E:
            case CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->lookupConfig.pcl1OverrideConfigIndex);
            }
        }
        else
        {
            /* reset field to keep in DB - not relevant */
            logicFormatPtr->lookupConfig.ipclConfigIndex=0;
            logicFormatPtr->lookupConfig.pcl0_1OverrideConfigIndex=0;
            logicFormatPtr->lookupConfig.pcl1OverrideConfigIndex=0;
        }

        if (usePolicer != GT_FALSE)
        {
            switch (logicFormatPtr->policer.policerEnable)
            {
                case CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E:
                case CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E:
                case CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E:
                case CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E:
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->policer.policerEnable);
            }
        }

        switch (logicFormatPtr->vlan.ingress.precedence)
        {
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->vlan.ingress.precedence);
        }

        switch (logicFormatPtr->vlan.ingress.modifyVlan)
        {
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_DISABLED_E:
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_UNTAGGED_E:
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_TAGGED_E:
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_FOR_ALL_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->vlan.ingress.modifyVlan);
        }
        switch (logicFormatPtr->qos.ingress.profilePrecedence)
        {
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E:
            case CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->qos.ingress.profilePrecedence);
        }
        switch (logicFormatPtr->qos.ingress.modifyDscp)
        {
            case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
            case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
            case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->qos.ingress.modifyDscp);
        }
        switch (logicFormatPtr->qos.ingress.modifyUp)
        {
            case CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E:
            case CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E:
            case CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->qos.ingress.modifyUp);
        }

        switch (logicFormatPtr->vlan.ingress.vlanId1Cmd)
        {
            case CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_DO_NOT_MODIFY_E:
            case CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_UNTAGGED_E:
            case CPSS_DXCH_PCL_ACTION_INGRESS_VLAN_ID1_CMD_ALL_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->vlan.ingress.vlanId1Cmd);
        }

        switch (logicFormatPtr->qos.ingress.up1Cmd)
        {
            case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_DO_NOT_MODIFY_E:
            case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG1_UNTAGGED_E:
            case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_TAG0_UNTAGGED_E:
            case CPSS_DXCH_PCL_ACTION_INGRESS_UP1_CMD_ALL_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->qos.ingress.up1Cmd);
        }
    }
    else
    {
        /* check egress PCL validity */

        if (logicFormatPtr->actionStop != GT_FALSE)
        {
            /* legacy behavior - to enforce this field to be 'not stop' (instead of 'ignore' the field)*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"logicFormatPtr->actionStop[%d]",
                                            logicFormatPtr->actionStop);
        }

        switch (logicFormatPtr->mirror.egressMirrorToAnalyzerMode)
        {
            case CPSS_DXCH_MIRROR_EGRESS_NOT_DROPPED_E:
            case CPSS_DXCH_MIRROR_EGRESS_TAIL_DROP_E:
            case CPSS_DXCH_MIRROR_EGRESS_CONGESTION_E:
            case CPSS_DXCH_MIRROR_EGRESS_CONGESTION_OR_TAIL_DROP_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->qos.ingress.up1Cmd);
        }

        switch(logicFormatPtr->epclPhaInfo.phaThreadIdAssignmentMode)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_DISABLED_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_BYPASS_PHA_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->epclPhaInfo.phaThreadIdAssignmentMode);
        }

        switch(logicFormatPtr->epclPhaInfo.phaThreadType)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_IOAM_INGRESS_SWITCH_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_INT_IOAM_MIRROR_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_CLASSIFIER_NSH_OVER_VXLAN_GPE_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->epclPhaInfo.phaThreadType);
        }
        switch (logicFormatPtr->qos.egress.modifyDscp)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_KEEP_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_OUTER_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_DSCP_EXP_CMD_MODIFY_INNER_E:
               break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->qos.egress.modifyDscp);
        }
        switch (logicFormatPtr->qos.egress.modifyUp)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->qos.egress.modifyUp);
        }

        switch (logicFormatPtr->vlan.egress.vlanCmd)
        {
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_DO_NOT_MODIFY_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_OUTER_TAG_E:
            case CPSS_DXCH_PCL_ACTION_EGRESS_TAG0_CMD_MODIFY_TAG0_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->vlan.egress.vlanCmd);
        }

        switch (logicFormatPtr->policer.policerEnable)
        {
            case CPSS_DXCH_PCL_POLICER_DISABLE_ALL_E:
            case CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E:
            case CPSS_DXCH_PCL_POLICER_ENABLE_METER_ONLY_E:
            case CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E:
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(logicFormatPtr->policer.policerEnable);
        }

        if (logicFormatPtr->oam.oamProfile > OAM_PROFILE_MAX_CNS)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->oam.oamProfile[%d]", logicFormatPtr->oam.oamProfile);
        }

        if (logicFormatPtr->oam.offsetIndex > OAM_INDEX_MAX_CNS)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM," illegal logicFormatPtr->oam.offsetIndex[%d]", logicFormatPtr->oam.offsetIndex);
        }
    }
    return GT_OK;
}
/**
* @internal validateExactMatchExpandedActionParams function
* @endinternal
*
* @brief   validate the Exact Match Expander Action parameters
*
* @param[in] expandedActionIndex   - index of the expanded entry (0-15)
* @param[in] logicFormatPtr        - points to Expander Action parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS validateExactMatchExpandedActionParams
(
    IN  GT_U32                                                         expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC   *expandedActionParamsPtr
)
{
    GT_STATUS   rc=GT_OK;

#ifndef CPSS_LOG_ENABLE
    GT_UNUSED_PARAM(expandedActionIndex);
#endif

    if (expandedActionParamsPtr->exactMatchExpandedEntryValid==GT_TRUE)
    {
        EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(expandedActionParamsPtr->expandedActionType,cpssDxChExactMatchExpandedActionSet);
        EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(expandedActionParamsPtr->expandedAction , cpssDxChExactMatchExpandedActionSet);
        switch(expandedActionParamsPtr->expandedActionType)
        {
            case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                /* check validity of TTI Action entry */
                rc = prvCpssDxChExactMatchManagerDbValidateTtiAction(&(expandedActionParamsPtr->expandedAction.ttiAction));
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"Error in validation of TTI Action expandedActionParamsPtr->expandedAction, expandedActionIndex[%d] ",
                                                  expandedActionIndex);
                }
            break;
            case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
            case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                /* check validity of PCL Action entry */
                rc = prvCpssDxChExactMatchManagerDbValidatePclAction(&(expandedActionParamsPtr->expandedAction.pclAction));
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"Error in validation of PCL/EPCL Action expandedActionParamsPtr->expandedAction, expandedActionIndex[%d] ",
                                                  expandedActionIndex);
                }
            break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(expandedActionParamsPtr->expandedActionType);
        }
        EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(expandedActionParamsPtr->expandedActionOrigin , cpssDxChExactMatchExpandedActionSet);
    }

    return rc;
}

/**
* @internal validateNewManagerParams function
* @endinternal
*
* @brief  The function validate 'API level' 'manager create' parameters.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs ExactMatchs –
*                                    like number of indexes, the amount of hashes etc.
*                                    As Exact Match Manager is created and entries may be added before PP registered –
*                                    we must know in advance the relevant capacity –
*                                    in Falcon for example we must know the shared memory allocation mode.
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries*
* @param[in] entryAttrPtr          - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
* @param[in] dbExist               - Indicates db already exist or not
*                                    GT_TRUE  - DB Exist
*                                    GT_FALSE - Not Exist
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if the Exact Match Manager id already exists.
*/
static GT_STATUS    validateNewManagerParams
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC              *agingPtr,
    IN GT_BOOL                                              dbExist
)
{
    GT_STATUS                                   rc=GT_OK;
    GT_U32                                      i,lookupNum,entryIndex;
    GT_BOOL                                     lookupEnable;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT            lookupClient;
    GT_U32                                      lookupClientMappingsNum;
    GT_U32                                      profileIdVal=0;
    GT_BOOL                                     enableExactMatchLookupEnable=GT_FALSE;
    CPSS_DXCH_EXACT_MATCH_MANAGER_PROFILE_ID_MAPPING_UNT *lookupClientMappingsArray;
    GT_U32                                      clientMappingProfileBmpArr[8]={0,0,0,0,0,0,0,0};/* bitmap to be used to check there is no double configuration */
    GT_U32                                      packetEntryIndex;

    CPSS_NULL_PTR_CHECK_MAC(capacityPtr);
    CPSS_NULL_PTR_CHECK_MAC(lookupPtr);
    CPSS_NULL_PTR_CHECK_MAC(entryAttrPtr);
    CPSS_NULL_PTR_CHECK_MAC(agingPtr);


    if(exactMatchManagerId >= CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "exactMatchManagerId [%d] >= [%d] (out of range)",
            exactMatchManagerId,
            CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS);
    }

    if((dbExist == GT_FALSE) && PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbArr)[exactMatchManagerId])
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,
            "exactMatchManagerId [%d] already exists",
            exactMatchManagerId);
    }

    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(capacityPtr->hwCapacity.numOfHwIndexes,prvCpssDxChCfgEmSizeSet);
    /** number of indexes in HW */
    switch(capacityPtr->hwCapacity.numOfHwIndexes)
    {
        /*case   _4K:*/
        case  _8K:
        case  _16K:
        case  _32K:
        case  _64K:
        case  _128K:
        case  _256K:
        /*case _512K:*/
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(capacityPtr->hwCapacity.numOfHwIndexes);
    }

    if(capacityPtr->hwCapacity.numOfHwIndexes > (1 << HW_INDEX_NUM_BITS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
            "numOfHwIndexes[%d] is more than the 'manager' supports[%d] (need to set new value to HW_INDEX_NUM_BITS)",
            capacityPtr->hwCapacity.numOfHwIndexes,
            (1 << HW_INDEX_NUM_BITS));
    }


    /** number of hashes (banks) : 16,8,4 */
    switch(capacityPtr->hwCapacity.numOfHashes)
    {
        case  4:
        case  8:
        case  CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS/*16*/: /* max supported value */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(capacityPtr->hwCapacity.numOfHashes);
    }

    /* check values  */
    EM_NUM_ENTRIES_STEPS_CHECK_MAC(capacityPtr->maxTotalEntries);
    EM_NUM_ENTRIES_STEPS_CHECK_MAC(capacityPtr->maxEntriesPerAgingScan);
    EM_NUM_ENTRIES_STEPS_CHECK_MAC(capacityPtr->maxEntriesPerDeleteScan);

    /* check that values make sence */
    EM_CHECK_X_NOT_MORE_THAN_Y_MAC(capacityPtr->maxTotalEntries            ,capacityPtr->hwCapacity.numOfHwIndexes);
    EM_CHECK_X_NOT_MORE_THAN_Y_MAC(capacityPtr->maxEntriesPerAgingScan     ,capacityPtr->hwCapacity.numOfHwIndexes);
    EM_CHECK_X_NOT_MORE_THAN_Y_MAC(capacityPtr->maxEntriesPerDeleteScan    ,capacityPtr->hwCapacity.numOfHwIndexes);


    for(lookupNum=0;lookupNum<CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E;lookupNum++)
    {
        cpssOsMemSet(clientMappingProfileBmpArr,0,sizeof(clientMappingProfileBmpArr));

        lookupEnable                =   lookupPtr->lookupsArray[lookupNum].lookupEnable;
        lookupClient                =   lookupPtr->lookupsArray[lookupNum].lookupClient;
        lookupClientMappingsNum     =   lookupPtr->lookupsArray[lookupNum].lookupClientMappingsNum;
        lookupClientMappingsArray   =   lookupPtr->lookupsArray[lookupNum].lookupClientMappingsArray;

        if(lookupEnable==GT_TRUE)
        {
            EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupClient,cpssDxChExactMatchClientLookupSet);
            switch(lookupClient)
            {
                case CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E:
                case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E:
                case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E:
                case CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E:
                case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E: /* valid only for AC5P - will be validate again when adding device to manager */
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(lookupClient);
            }

            if(lookupClientMappingsNum > MAX_NUM_PROFILE_ID_MAPPING_ENTRIES_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                    "lookupClientMappingsNum[%d], lookupNum[%d] must be in range[0..256]",
                    lookupClientMappingsNum,lookupNum);
            }

            for(i=0;i<lookupClientMappingsNum;i++)
            {
                if(lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
                {
                    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupClientMappingsArray[i].ttiMappingElem.keyType,
                                                         cpssDxChExactMatchTtiProfileIdModePacketTypeSet);
                    rc = ttiGetIndexForKeyType(lookupClientMappingsArray[i].ttiMappingElem.keyType, GT_TRUE, &entryIndex);/* check keyType is legal*/
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(lookupClientMappingsArray[i].ttiMappingElem.keyType);
                    }

                    /* check if the keyType already registered */
                    if(EM_IS_PROFILE_ID_MAPPING_BMP_SET_MAC(clientMappingProfileBmpArr,entryIndex))
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                "lookupClientMappingsArray[i].ttiMappingElem.profileId[%d], lookupNum[%d], keyType[%d] was already configured, "
                                "can not configure different profiles to same keyType\n",
                                 lookupClientMappingsArray[i].ttiMappingElem.profileId,lookupNum,lookupClientMappingsArray[i].ttiMappingElem.keyType);
                    }
                    else
                    {
                        /* set entryIndex as already configured */
                        EM_PROFILE_ID_MAPPING_BMP_SET_MAC(clientMappingProfileBmpArr,entryIndex);
                    }

                    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupClientMappingsArray[i].ttiMappingElem.enableExactMatchLookup,
                                                         cpssDxChExactMatchTtiProfileIdModePacketTypeSet);

                    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupClientMappingsArray[i].ttiMappingElem.profileId,
                                                         cpssDxChExactMatchTtiProfileIdModePacketTypeSet);

                    if(lookupClientMappingsArray[i].ttiMappingElem.enableExactMatchLookup==GT_TRUE)
                    {
                        if((lookupClientMappingsArray[i].ttiMappingElem.profileId < CPSS_DXCH_EXACT_MATCH_MANAGER_MIN_PROFILE_ID_CNS)||
                           (lookupClientMappingsArray[i].ttiMappingElem.profileId > CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_PROFILE_ID_CNS))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                "lookupClientMappingsArray[i].ttiMappingElem.profileId[%d], lookupNum[%d] must be in range[1..15]",
                                 lookupClientMappingsArray[i].ttiMappingElem.profileId,lookupNum);
                        }
                        else
                        {
                            enableExactMatchLookupEnable = lookupClientMappingsArray[i].ttiMappingElem.enableExactMatchLookup;
                            profileIdVal = lookupClientMappingsArray[i].ttiMappingElem.profileId;
                        }
                    }
                }
                else
                {
                    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupClientMappingsArray[i].pclMappingElem.packetType,
                                                         cpssDxChExactMatchPclProfileIdSet);

                    switch(lookupClientMappingsArray[i].pclMappingElem.packetType)
                    {
                        case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_UDE_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E:
                        case CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E:
                            break;
                        default:
                            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(lookupClientMappingsArray[i].pclMappingElem.packetType);
                    }

                    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupClientMappingsArray[i].pclMappingElem.subProfileId,
                                                         cpssDxChExactMatchPclProfileIdSet);
                    if (lookupClientMappingsArray[i].pclMappingElem.subProfileId > PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_SUB_PROFILE_ID_VAL_CNS)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(lookupClientMappingsArray[i].pclMappingElem.subProfileId);
                    }

                    /* check if the packetType+subProfileId already registered */
                    /* Calculate entry index by packet type + subProfile Id */
                    rc = sip5PclUDBEntryIndexGet(0, lookupClientMappingsArray[i].pclMappingElem.packetType, &packetEntryIndex);/* set devNum as 0, it is not used in the function so no meaning for the value*/
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                    entryIndex = (((packetEntryIndex & 0xF)<<4)|(lookupClientMappingsArray[i].pclMappingElem.subProfileId & 0xF));

                    if(EM_IS_PROFILE_ID_MAPPING_BMP_SET_MAC(clientMappingProfileBmpArr,entryIndex))
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                "lookupClientMappingsArray[i].pclMappingElem.profileId[%d], lookupNum[%d], packetType[%d], "
                                "subProfileId[%d] was already configured, "
                                "can not configure different profiles to same keyType\n",
                                 lookupClientMappingsArray[i].pclMappingElem.profileId,lookupNum,lookupClientMappingsArray[i].pclMappingElem.packetType,
                                 lookupClientMappingsArray[i].pclMappingElem.subProfileId);
                    }
                    else
                    {
                        /* set entryIndex as already configured */
                        EM_PROFILE_ID_MAPPING_BMP_SET_MAC(clientMappingProfileBmpArr,entryIndex);
                    }

                    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupClientMappingsArray[i].pclMappingElem.enableExactMatchLookup,
                                                         cpssDxChExactMatchPclProfileIdSet);
                    if(lookupClientMappingsArray[i].pclMappingElem.enableExactMatchLookup==GT_TRUE)
                    {
                        if((lookupClientMappingsArray[i].pclMappingElem.profileId < CPSS_DXCH_EXACT_MATCH_MANAGER_MIN_PROFILE_ID_CNS)||
                           (lookupClientMappingsArray[i].pclMappingElem.profileId > CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_PROFILE_ID_CNS))
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                "lookupClientMappingsArray[i].pclMappingElem.profileId[%d], lookupNum[%d] must be in range[1..15]",
                                lookupClientMappingsArray[i].pclMappingElem.profileId,lookupNum);
                        }
                        else
                        {
                            enableExactMatchLookupEnable = lookupClientMappingsArray[i].pclMappingElem.enableExactMatchLookup;
                            profileIdVal = lookupClientMappingsArray[i].pclMappingElem.profileId;
                        }
                    }
                }

                if(enableExactMatchLookupEnable==GT_TRUE)
                {
                    /* profileId is ok, check the profileId parameters */
                    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->profileEntryParamsArray[profileIdVal].keyParams.keySize,
                                                         cpssDxChExactMatchProfileKeyParamsSet);

                    switch(lookupPtr->profileEntryParamsArray[profileIdVal].keyParams.keySize)
                    {
                        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
                        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
                        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
                        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
                            break;
                        default:
                            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(lookupPtr->profileEntryParamsArray[profileIdVal].keyParams.keySize);
                    }

                    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->profileEntryParamsArray[profileIdVal].keyParams.keyStart,
                                                         cpssDxChExactMatchProfileKeyParamsSet);
                    /* check validity of keyStart (6 bits)*/
                    if(lookupPtr->profileEntryParamsArray[profileIdVal].keyParams.keyStart >= BIT_6)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "lookupPtr->profileEntryParamsArray[profileIdVal].keyParams.keyStart[%d] is more than the 'manager' supports[%d]",
                        lookupPtr->profileEntryParamsArray[profileIdVal].keyParams.keyStart,BIT_6);
                    }

                    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->profileEntryParamsArray[profileIdVal].keyParams.mask,
                                                         cpssDxChExactMatchProfileKeyParamsSet);

                    if (lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionEn == GT_TRUE)
                    {
                        EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType,
                                                         cpssDxChExactMatchProfileDefaultActionSet);

                        switch(lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType)
                        {
                            case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                            case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                            case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                                break;
                            default:
                                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType);
                        }
                        EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->profileEntryParamsArray[profileIdVal].defaultAction,
                                                             cpssDxChExactMatchProfileDefaultActionSet);

                        switch(lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType)
                        {
                            /* check that the action configure for profileId match the client configured for this lookup */
                            case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
                                if(lookupClient!=CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                    "lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType[%d], lookupNum[%d] must be the same as lookupClient[%d]",
                                    lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType,lookupNum,lookupClient);
                                }
                                else
                                {
                                    /* check validity of default TTI entry */
                                    rc = prvCpssDxChExactMatchManagerDbValidateTtiAction(&(lookupPtr->profileEntryParamsArray[profileIdVal].defaultAction.ttiAction));
                                    if (rc != GT_OK)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"lookupPtr->profileEntryParamsArray[profileIdVal].defaultAction.ttiAction, lookupNum[%d] ",
                                                                      lookupNum);
                                    }
                                }
                                break;
                            case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
                                if((lookupClient!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E)&&
                                   (lookupClient!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E)&&
                                   (lookupClient!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E))/* valid only for AC5P - will be validate again when adding device to manager */
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                    "lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType[%d], lookupNum[%d] must be the same as lookupClient[%d]",
                                    lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType,lookupNum,lookupClient);
                                }
                                else
                                {
                                    /* check validity of default PCL entry */
                                    rc = prvCpssDxChExactMatchManagerDbValidatePclAction(&(lookupPtr->profileEntryParamsArray[profileIdVal].defaultAction.pclAction));
                                    if (rc != GT_OK)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"Error in validation of PCL/EPCL Action "
                                                                      "lookupPtr->profileEntryParamsArray[profileIdVal].defaultAction.pclAction, profileIdVal[%d] ",
                                                                      profileIdVal);
                                    }
                                }
                                break;
                            case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
                                if(lookupClient!=CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E)
                                {
                                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                                    "lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType[%d], lookupNum[%d] must be the same as lookupClient[%d]",
                                    lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType,lookupNum,lookupClient);
                                }
                                else
                                {
                                    /* check validity of default EPCL entry */
                                    rc = prvCpssDxChExactMatchManagerDbValidatePclAction(&(lookupPtr->profileEntryParamsArray[profileIdVal].defaultAction.pclAction));
                                    if (rc != GT_OK)
                                    {
                                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"Error in validation of PCL/EPCL Action "
                                                                      "lookupPtr->profileEntryParamsArray[profileIdVal].defaultAction.pclAction, profileIdVal[%d] ",
                                                                      profileIdVal);
                                    }
                                }
                                break;
                            default:
                                CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionType);
                        }
                    }

                   EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(lookupPtr->profileEntryParamsArray[profileIdVal].defaultActionEn,
                                                         cpssDxChExactMatchProfileDefaultActionSet);
                }
            }
        }
    }

    /* validate expandedArray */
    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;i++)
    {
        rc = validateExactMatchExpandedActionParams(i,&entryAttrPtr->expandedArray[i]);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in validation of entryAttrPtr->expandedArray[%d]",i);
        }
    }
    EM_BIND_PARAM_TO_LOW_LEVEL_CPSS_API_MAC(agingPtr->globalActivityBitEnable , cpssDxChExactMatchActivityBitEnableSet);

    return GT_OK;
}
/**
* @internal createNewManager function
* @endinternal
*
* @brief  The function validate application parameters and then create 'manager' :
*         'prvCpssDxChExactMatchManagerDbArr[exactMatchManagerId]' with info from the caller.
*         and save the info from the application
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs ExactMatchs –
*                                    like number of indexes, the amount of hashes etc.
*                                    As Exact Match Manager is created and entries may be added before PP registered –
*                                    we must know in advance the relevant capacity –
*                                    in Falcon for example we must know the shared memory allocation mode.
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries*
* @param[in] entryAttrPtr          - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
* @param[out] managerAllocatedPtr  - (pointer to) indication that the manager was allocated.
*                                    so on error need to do 'clean-up'
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if the Exact Match Manager id already exists.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
*/
static GT_STATUS    createNewManager
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC              *agingPtr,
    OUT GT_BOOL                                             *managerAllocatedPtr
)
{
    GT_STATUS                                               rc ;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC *exactMatchManagerPtr;
    GT_U32                                                  numBytesToAlloc;
    GT_U32                                                  i=0;

    /********************************************/
    /* validity checks before the DB allocation */
    /********************************************/
    rc = validateNewManagerParams(exactMatchManagerId,capacityPtr,lookupPtr,entryAttrPtr,agingPtr,GT_FALSE/*EXACT_MATCH_MANAGER_DB_EXIST*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*************************/
    /* allocate new manager  */
    /*************************/
    numBytesToAlloc = sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC);
    EM_MEM_CALLOC_MAC(exactMatchManagerId,PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbArr)[exactMatchManagerId],numBytesToAlloc);

    /* indication that the manager was allocated */
    *managerAllocatedPtr = GT_TRUE;

    /* we have valid new manager pointer */
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /*****************/
    /* save the info */
    /*****************/
    exactMatchManagerPtr->capacityInfo  = *capacityPtr;
    exactMatchManagerPtr->lookupInfo    = *lookupPtr;
    exactMatchManagerPtr->agingInfo     = *agingPtr;

    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;i++)
    {
        cpssOsMemCpy(&exactMatchManagerPtr->exactMatchExpanderArray[i].exactMatchExpandedEntry,
                     &entryAttrPtr->expandedArray[i],
                     sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC));

        exactMatchManagerPtr->exactMatchExpanderArray[i].exactMatchExpandedEntryValid=entryAttrPtr->expandedArray[i].exactMatchExpandedEntryValid;
        exactMatchManagerPtr->exactMatchExpanderArray[i].exactMatchEntriesCounter=0;
        cpssOsMemSet(exactMatchManagerPtr->exactMatchExpanderArray[i].reducedMaskArr,0,
                     sizeof(exactMatchManagerPtr->exactMatchExpanderArray[i].reducedMaskArr));
    }

    return GT_OK;
}

/**
* @internal initNewManager function
* @endinternal
*
* @brief  The function init 'manager' needed DBs (and allocate needed large arrays)
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
*/
static GT_STATUS    initNewManager
(
    IN GT_U32   exactMatchManagerId
)
{
    GT_U32                                                  numBytesToAlloc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *currEntryPtr;
    GT_U32                                                  ii,iiMax;

    numBytesToAlloc = exactMatchManagerPtr->capacityInfo.maxTotalEntries * sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC);
    EM_MEM_CALLOC_MAC(exactMatchManagerId,exactMatchManagerPtr->entryPoolPtr,numBytesToAlloc);

    numBytesToAlloc = exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes * sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INDEX_STC);
    EM_MEM_CALLOC_MAC(exactMatchManagerId,exactMatchManagerPtr->indexArr,numBytesToAlloc);

    currEntryPtr = &exactMatchManagerPtr->entryPoolPtr[0];

    iiMax = exactMatchManagerPtr->capacityInfo.maxTotalEntries;

    /* init the 'linked list' of 'free list' */
    for(ii = 0 ; ii < iiMax; ii++,currEntryPtr++)
    {
        if(ii != 0)
        {
            currEntryPtr->isValid_prevEntryPointer = 1;
            currEntryPtr->prevEntryPointer         = ii - 1;
        }

        if(ii != (iiMax-1))
        {
            currEntryPtr->isValid_nextEntryPointer = 1;
            /*currEntryPtr->nextEntryPointer         = ii + 1;*/
            EM_SET_nextEntryPointer_MAC(currEntryPtr,(ii + 1));
        }
    }

    /* init the head of free list */
    exactMatchManagerPtr->headOfFreeList.isValid_entryPointer = 1;/* validate the head */
    exactMatchManagerPtr->headOfFreeList.entryPointer         = 0;/* point to the first index in entryPoolPtr */

    /* init the head of used list */
    exactMatchManagerPtr->headOfUsedList.isValid_entryPointer = 0;/* no 'used' entries at this point */
    exactMatchManagerPtr->headOfUsedList.entryPointer         = 0;/* don't care */

    /* init the tail of used list */
    exactMatchManagerPtr->tailOfUsedList.isValid_entryPointer = 0;/* no 'used' entries at this point */
    exactMatchManagerPtr->tailOfUsedList.entryPointer         = 0;/* don't care */

    switch(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes)
    {
        case   _8K:exactMatchManagerPtr->cpssHashParams.size = PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_8K_E; break;
        case  _16K:exactMatchManagerPtr->cpssHashParams.size = PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_16K_E; break;
        case  _32K:exactMatchManagerPtr->cpssHashParams.size = PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_32K_E; break;
        case  _64K:exactMatchManagerPtr->cpssHashParams.size = PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_64K_E; break;
        case _128K:exactMatchManagerPtr->cpssHashParams.size = PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_128K_E; break;
        case _256K:exactMatchManagerPtr->cpssHashParams.size = PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_256K_E; break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
    }

    /* do single time calculation */
    exactMatchManagerPtr->maxPopulationPerBank =
        exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes/
        exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes;

    /* currently only SIP6 devices can be added to this Exact Match manager ! */
    EM_BMP_SET_MAC(exactMatchManagerPtr->supportedSipArr,CPSS_GEN_SIP_6_E);

    /* age-bin usage matrix allocation */
    exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated = (exactMatchManagerPtr->capacityInfo.maxTotalEntries + (exactMatchManagerPtr->capacityInfo.maxEntriesPerAgingScan - 1)) / exactMatchManagerPtr->capacityInfo.maxEntriesPerAgingScan;

    /* Level 1 usage allocation. Each member of array holds flag about 32 age bins  */
    numBytesToAlloc = sizeof(GT_U32) * (1 + (exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated >> 5));
    EM_MEM_CALLOC_MAC(exactMatchManagerId, exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr, numBytesToAlloc);

    /* age-bin list allocation */
    numBytesToAlloc = exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated * sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_BIN_STC);
    EM_MEM_CALLOC_MAC(exactMatchManagerId, exactMatchManagerPtr->agingBinInfo.ageBinListPtr, numBytesToAlloc);

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbCreate function
* @endinternal
*
* @brief The function creates the Exact Match Manager and its databases according to
*        input capacity structure. Internally all ExactMatch global parameters / modes
*        initialized to their defaults (No HW access - just SW defaults that will
*        be later applied to registered devices).
*
*        NOTE: the function doing only DB operations (no HW operations)
*
*        NOTE: incase of 'error' : if the manager was allocated the caller must
*           call prvCpssDxChExactMatchManagerDbDelete(...) to 'undo' the partial create.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id to associate with the newly created manager.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] capacityPtr           - (pointer to) parameters of the HW structure and capacity of its managed PPs ExactMatchs –
*                                    like number of indexes, the amount of hashes etc.
*                                    As Exact Match Manager is created and entries may be added before PP registered –
*                                    we must know in advance the relevant capacity –
*                                    in Falcon for example we must know the shared memory allocation mode.
* @param[in] lookupPtr             - (pointer to) parameters that affect lookup of entries*
* @param[in] entryAttrPtr          - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[in] agingPtr              - (pointer to) parameters that affect refresh and aging
* @param[out] managerAllocatedPtr  - (pointer to) indication that the manager was allocated.
*                                    so on error need to do 'clean-up'
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_OUT_OF_CPU_MEM        - on fail to do Cpu memory allocation.
* @retval GT_ALREADY_EXIST         - if the Exact Match Manager id already exists.
*/
GT_STATUS prvCpssDxChExactMatchManagerDbCreate
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC           *capacityPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC             *lookupPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC   *entryAttrPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC              *agingPtr,
    OUT GT_BOOL                                             *managerAllocatedPtr
)
{
    GT_STATUS   rc;

    /*
    *  NOTE: incase of 'error' : if the manager was allocated the caller must
    *     call prvCpssDxChExactMatchManagerDbDelete(...) to 'undo' the partial create.
    */
    *managerAllocatedPtr = GT_FALSE;

    /***********************/
    /* create new manager  */
    /***********************/
    rc = createNewManager(exactMatchManagerId,capacityPtr,lookupPtr,entryAttrPtr,agingPtr,managerAllocatedPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*************************/
    /* init the new manager  */
    /*************************/
    rc = initNewManager(exactMatchManagerId);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChExactMatchManagerDbDelete function
* @endinternal
*
* @brief  The function de-allocates specific Exact Match Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*/
GT_STATUS prvCpssDxChExactMatchManagerDbDelete
(
    IN GT_U32 exactMatchManagerId
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U8  devId;
    GT_U32  numOfAllocatedPointers;
    GT_U32  i=0;

    EM_MANAGER_ID_CHECK(exactMatchManagerId)
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* remove portGroup for specific device from the 'global bmp' */
    for(devId = 0; devId < PRV_CPSS_MAX_PP_DEVICES_CNS; devId++)
    {
        PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalPortGroupBmpArr)[devId] &= ~exactMatchManagerPtr->portGroupsBmpArr[devId];
        if (PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalPortGroupBmpArr)[devId]==0)
        {
            /* no more portGroups defined on the device meaning,
               no more managers defined on the device - so we can remove the device from the 'global bmp' */

            if(EM_IS_DEV_BMP_SET_MAC(PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalDevsBmpArr),devId))
            {
                /* invalidate all expanded entries in SW */
                for (i=0;i<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;i++)
                {
                    if(exactMatchManagerPtr->exactMatchExpanderArray[i].exactMatchExpandedEntryValid==GT_TRUE)
                    {
                        /* the entry can be invalidated */
                        prvCpssDxChExpandedActionInvalidateShadow(devId,i);
                    }
                }
            }

            /* state that the device removed from the global DB of devices */
             EM_DEV_BMP_CLEAR_MAC(PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalDevsBmpArr), devId);
        }
    }

    /* deallocate all 'malloc' that where done for this manager */
    EM_MEM_FREE_MAC(exactMatchManagerId,exactMatchManagerPtr->agingBinInfo.ageBinListPtr);
    EM_MEM_FREE_MAC(exactMatchManagerId,exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr);
    EM_MEM_FREE_MAC(exactMatchManagerId,exactMatchManagerPtr->indexArr);
    EM_MEM_FREE_MAC(exactMatchManagerId,exactMatchManagerPtr->entryPoolPtr);

    /* get the number of pending allocated pointers just before we free the manager itself */
    /* because it is decremented for every 'free' that we do */
    numOfAllocatedPointers = exactMatchManagerPtr->numOfAllocatedPointers;
    EM_MEM_FREE_MAC(exactMatchManagerId,exactMatchManagerPtr);

    /* detach the manager-ID that was just free by upper line */
    PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbArr)[exactMatchManagerId] = NULL;

    if(numOfAllocatedPointers != 1)
    {
        /* not all the pointers released as should have been !!! */
        /* we removed the manager successfully ... but we have memory leakage !*/
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "(manager[%d] successfully removed , but we have memory leakage from [%d] pointers (forgot to free)",
            exactMatchManagerId,
            (numOfAllocatedPointers-1));
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerAllDbDelete function
* @endinternal
*
* @brief  The function de-allocates All Exact Match Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager
*                                    id is not initialized.
*/
GT_STATUS prvCpssDxChExactMatchManagerAllDbDelete()
{
    GT_U32              exactMatchManagerId;
    GT_STATUS           rc;

    for(exactMatchManagerId = 0; exactMatchManagerId < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS; exactMatchManagerId++)
    {
        rc = prvCpssDxChExactMatchManagerDbDelete(exactMatchManagerId);
        if((rc != GT_OK) && (rc != GT_NOT_INITIALIZED))
        {
            return rc;
        }
        /* delete cuckoo DB */
        rc = prvCpssDxChCuckooDbDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbConfigGet function
* @endinternal
*
* @brief The function fetches current Exact Match Manager setting as they were defined in Exact Match Manager Create.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] capacityPtr          - (pointer to) parameters of the HW structure and capacity of its managed
*                                    PPs EMs: like number of indexes, the amount of hashes etc. As Exact Match
*                                    Manager is created and entries may be added before PP registered,
*                                    we must know in advance the relevant capacity. In Falcon, for
*                                    example, we must know the shared memory allocation mode.
* @param[out] lookupPtr            - (pointer to) parameters that affect lookup of entries
* @param[out] entryAttrPtr         - (pointer to) parameters that affect exact match entries:
*                                    expanded configuration
* @param[out] agingPtr             - (pointer to) parameters that affect refresh and aging
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
void prvCpssDxChExactMatchManagerDbConfigGet
(
    IN  GT_U32                                              exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC          *capacityPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC            *lookupPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC  *entryAttrPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC             *agingPtr
)
{
    GT_U32 i=0;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;

    /* we have valid new manager pointer */
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /********************/
    /* extract the info */
    /********************/
    *capacityPtr    = exactMatchManagerPtr->capacityInfo;
    *lookupPtr      = exactMatchManagerPtr->lookupInfo;
    *agingPtr       = exactMatchManagerPtr->agingInfo;

    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;i++)
    {
        cpssOsMemCpy(&entryAttrPtr->expandedArray[i],
                     &exactMatchManagerPtr->exactMatchExpanderArray[i].exactMatchExpandedEntry,
                     sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC));
    }

    return;
}
/**
* @internal prvCpssDxChExactMatchManagerDbIsAnyDeviceRegistred function
* @endinternal
*
* @brief   This function check if is there are registered device.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr    - (pointer to) the Exact Match manager.
*
* @retval 0 - no device registered.
* @retval 1 - at least one device registered.
*/
GT_U32 prvCpssDxChExactMatchManagerDbIsAnyDeviceRegistred
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC     *exactMatchManagerPtr
)
{
    return exactMatchManagerPtr->numOfDevices ? 1 : 0;
}

/**
* @internal prvCpssDxChExactMatchManagerDbEntryIteratorGetNext function
* @endinternal
*
* @brief   This function get first/next valid (pointer to) entry from the manager.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] isFirst               - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[in] dbEntryPtrPtr         - (pointer to) the '(pointer to)current entry' for which we ask for 'next'
*                                     relevant only when isFirst == GT_FALSE (ask for next)
* @param[out] dbEntryPtrPtr        - (pointer to) the '(pointer to)first/next entry'
*
*
* @retval GT_OK - there is 'first/next valid entry' from the manager
* @retval GT_NO_MORE - there are no more entries
*                      if isFirst == GT_TRUE --> no entries at all
*                      else                  --> no more entries after 'current' (IN *dbEntryPtrPtr)
*         GT_BAD_STATE - there is error in DB management.
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbEntryIteratorGetNext
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN  GT_BOOL                                                     isFirst,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC     **dbEntryPtrPtr
)
{
    GT_U32                                                  dbIndex;/* index in exactMatchManagerPtr->entryPoolPtr[] */
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *currDbPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *nextDbPtr;

    if(GT_TRUE == isFirst)
    {
        if(!exactMatchManagerPtr->headOfUsedList.isValid_entryPointer)
        {
            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }

        /* we have valid 'head' .. so return it */
        dbIndex        = exactMatchManagerPtr->headOfUsedList.entryPointer;
    }
    else
    {
        currDbPtr = *dbEntryPtrPtr;  /* current entry */

        /* check for next pointer */
        if(!currDbPtr->isValid_nextEntryPointer)
        {
            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }

        /*dbIndex        = currDbPtr->nextEntryPointer;*/
        EM_GET_nextEntryPointer_MAC(currDbPtr,dbIndex);
    }

    nextDbPtr      = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
    if(nextDbPtr->isUsedEntry == 0)/* sanity check */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "manager error 'isUsedEntry' expected to be valid on the entry , dbIndex[%d]",
            dbIndex);
    }

    *dbEntryPtrPtr = nextDbPtr;
    return GT_OK;
}


/**
* @internal prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext function
* @endinternal
*
* @brief   This function get first/next registered device.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] isFirst               - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[in] devNumPtr             - (pointer to) the 'current device' for which we ask for 'next'
*                                     relevant only when isFirst == GT_FALSE (ask for next)
* @param[out] devNumPtr            - (pointer to) the 'first/next device'
* @param[out] portGroupBmpPtr      - (pointer to) the 'first/next device portGroupBmp'
*
* @retval GT_OK - there is 'first/next device'
* @retval GT_NO_MORE - there are no more devices
*                if isFirst == GT_TRUE --> no devices at all
*                else                  --> no more devices after 'current' (IN *devNumPtr)
*/
GT_STATUS prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN  GT_BOOL                                                     isFirst,
    INOUT GT_U8                                                     *devNumPtr,
    OUT GT_PORT_GROUPS_BMP                                          *portGroupBmpPtr
)
{
    GT_U32  wordId,bitId;

    if(isFirst == GT_TRUE)
    {
        wordId = 0;
        bitId  = 0;
    }
    else  /* the '+1' is to start from 'next' device */
    {
        wordId = ((*devNumPtr) + 1) >> 5;   /* /32 */
        bitId  = ((*devNumPtr) + 1) & 0x1f; /* %32 */
    }

    for(/* continue wordId */; wordId < NUM_WORDS_DEVS_BMP_CNS; wordId++)
    {
        if(exactMatchManagerPtr->devsBmpArr[wordId] == 0)
        {
            /* allow to jump over a lot of non existing devices */
            /* jump to next word (next 32 devices)              */
            bitId = 0;
            continue;
        }

        for(/* continue bitId */; bitId < 32; bitId++)
        {
            if(exactMatchManagerPtr->devsBmpArr[wordId] & (1 << bitId))
            {
                *devNumPtr = (GT_U8)((wordId << 5) + bitId);
                *portGroupBmpPtr = exactMatchManagerPtr->portGroupsBmpArr[*devNumPtr];
                return GT_OK;
            }
        }

        bitId = 0;
    }

    return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
}

/**
* @internal prvCpssDxChExactMatchManagerDbDevListAdd function
* @endinternal
*
* @brief   This function adding (registering) device(s) to
*          specific Exact Match Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to add to the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_ALREADY_EXIST         - if device already exist in DB
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbDevListAdd
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32  ii;
    GT_U32  areOverlap;
    GT_U8   devNum = 0;
    GT_U32  numOfHashes;
    GT_PORT_GROUPS_BMP   portGroupsBmp;
    GT_U32  exactMatchNumOfBanks=0;
    GT_U32  isNewDev;

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    if(numOfPairs > PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfPairs [%d] > max[%d] ",
            numOfPairs,
            PRV_CPSS_MAX_PP_DEVICES_CNS);
    }
    else
    if(numOfPairs == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfPairs is ZERO ... no meaning calling the function");
    }

    /***********************************************/
    /* validity check on the 'to be added' devices */
    /***********************************************/
    for(ii = 0 ; ii < numOfPairs ; ii++)
    {
        devNum = pairListArr[ii].devNum;
        portGroupsBmp=pairListArr[ii].portGroupsBmp;

        /* check if the device already registered with any of the Exact Match managers */
        if(EM_IS_DEV_BMP_SET_MAC(PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalDevsBmpArr),devNum))
        {
            /* check if portGroupsBmp was already registered for that device  */
            if ((PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalPortGroupBmpArr)[devNum] & portGroupsBmp)!=0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,
                    "devNum[%d]+portGroupsBmp[%d] already registered at one of the Exact Match managers",
                    devNum,portGroupsBmp);
            }
        }

        /* we lock the CPSS DB ... to be able to access 'PRV_CPSS_PP_MAC(devNum)' */
        /* we must unlock this DB if exit on ERROR !!! */
        EM_LOCK_DEV_NUM(devNum);

        /* check that the device supports the ExactMatch manager */
       EM_ARE_2_BMPS_OVERLAP_MAC(exactMatchManagerPtr->supportedSipArr,
                                  PRV_CPSS_PP_MAC(devNum)->supportedSipArr,
                                  areOverlap);
        if(areOverlap == 0)
        {
            EM_UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
                "devNum[%d] is not applicable for this Exact Match manager (not supported)",
                devNum);
        }

        if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum <
           exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes)
        {
            EM_UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */

            /* the 'fineTuning.tableSize.emNum' reflect the Exact Match size as given to the 'shared tables' */
            /* so we can't have lower value */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "Exact Match size in 'shared tables' is [%d] and not support Exact Match manager [%d] entries",
                    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum,
                    exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
        }

        PRV_CPSS_DXCH_EXACT_MATCH_NUM_OF_BANKS_GET_MAC(devNum,exactMatchNumOfBanks);
        switch (exactMatchNumOfBanks)
        {
            case 4:
                numOfHashes =  4;/* CPSS_EXACT_MATCH_MHT_4_E */
                break;
            case 8:
                numOfHashes =  8;/* CPSS_EXACT_MATCH_MHT_8_E */
                break;
            case 16:
                numOfHashes =  16;/* CPSS_EXACT_MATCH_MHT_16_E */
                break;
            default:
                EM_UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
        }

        if (exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes != numOfHashes)
        {
            EM_UNLOCK_DEV_NUM(devNum);/* release DB before we exists on error */

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "Number of hashes[%d] for Exact Match size[%d] in 'shared tables' is not supported for SIP6 devices",
                    exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes, exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);
        }
        EM_UNLOCK_DEV_NUM(devNum);/* we are done with this device internal DB */
    }

    /* start DB operations on the new devices */
    for(ii = 0 ; ii < numOfPairs ; ii++)
    {
        devNum = pairListArr[ii].devNum;
        portGroupsBmp = pairListArr[ii].portGroupsBmp;

        /* keep a flag if this is a new device
           there is an option we are adding an existing device with a new portGroup
           this will lead to change in existing values of the devNum , no new addition is done*/
        isNewDev = EM_IS_DEV_BMP_SET_MAC(exactMatchManagerPtr->devsBmpArr,devNum);

        /* state that the device was added to global DB of device */
        EM_DEV_BMP_SET_MAC(PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalDevsBmpArr),devNum);
        PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalPortGroupBmpArr)[devNum]|=portGroupsBmp;

        /* state that the device was added to local DB of current manager */
        EM_DEV_BMP_SET_MAC(exactMatchManagerPtr->devsBmpArr,devNum);
        exactMatchManagerPtr->portGroupsBmpArr[devNum] |= portGroupsBmp;

        /* update devices counter */
        if(isNewDev==0)
            exactMatchManagerPtr->numOfDevices++;
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChExactMatchManagerDbDevListRemove function
* @endinternal
*
* @brief   This function removing (unregistering) device(s) from
*          specific Exact Match Manager Instance.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] pairListArr[]         - the array of pairs (device ids + portGroupBmp)
*                                    to remove from the Exact Match Manager.
* @param[in] numOfPairs            - the number of pairs in the array.
*
* @retval GT_OK                    - on success
* @retval GT_NO_SUCH               - if one of the devices not in DB.
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager id is not initialized.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbDevListRemove
(
    IN GT_U32                                                   exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListArr[],/*arrSizeVarName=numOfPairs*/
    IN GT_U32                                                   numOfPairs
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32  ii;
    GT_U8   devNum;
    GT_PORT_GROUPS_BMP   portGroupsBmp;

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    if(numOfPairs > PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfPairs [%d] > max[%d] ",
            numOfPairs,
            PRV_CPSS_MAX_PP_DEVICES_CNS);
    }
    else
    if(numOfPairs == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfPairs is ZERO ... no meaning calling the function");
    }

    /*************************************************/
    /* validity check on the 'to be removed' devices */
    /*************************************************/
    for(ii = 0 ; ii < numOfPairs ; ii++)
    {
        devNum = pairListArr[ii].devNum;
        portGroupsBmp = pairListArr[ii].portGroupsBmp;

        /* check that the device registered with the Exact Match managers */
        if(!EM_IS_DEV_BMP_SET_MAC(exactMatchManagerPtr->devsBmpArr,devNum)||
           ((exactMatchManagerPtr->portGroupsBmpArr[devNum] & portGroupsBmp)==0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH,
                "devNum[%d]+ portGroupsBmp[%d] is not registered at manager",
                devNum,portGroupsBmp);
        }
    }

    /* remove devices from the bmp of devices */
    for(ii = 0 ; ii < numOfPairs ; ii++)
    {
        devNum = pairListArr[ii].devNum;
        portGroupsBmp = pairListArr[ii].portGroupsBmp;

        /* state that the portGroup removed from the global DB of devices */
        PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalPortGroupBmpArr)[devNum] &= ~portGroupsBmp;

        /* if there is no portGroup configured on the device,
           we can remove the device from the global list*/
        if (PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalPortGroupBmpArr)[devNum]==0)
        {
            /* state that the device removed from the global DB of devices */
             EM_DEV_BMP_CLEAR_MAC(PRV_SHARED_EMM_DIR_EMM_DB_SRC_GLOBAL_VAR_GET(prvCpssDxChExactMatchManagerDbGlobalDevsBmpArr), devNum);
        }

        /* state that the device removed from the local DB of devices in the manager*/
        exactMatchManagerPtr->portGroupsBmpArr[devNum] &= ~portGroupsBmp;
        if (exactMatchManagerPtr->portGroupsBmpArr[devNum]==0)
        {
            /* only if there is no portGroup defined on the devNum
               we can then remove the devNum from the list */
            EM_DEV_BMP_CLEAR_MAC(exactMatchManagerPtr->devsBmpArr, devNum);
            /* update devices counter */
            /* coherency check -- can't be ZERO because we are going to decrement it */
            EM_CHECK_X_NOT_ZERO_MAC(exactMatchManagerPtr->numOfDevices);
            exactMatchManagerPtr->numOfDevices--;
        }
    }

    return GT_OK;
}
/**
* @internal prvCpssDxChExactMatchManagerDbDevListGet function
* @endinternal
*
* @brief The function returns current Exact Match Manager registered devices (added by 'add device' API).
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[inout] numOfPairsPtr      - (pointer to) the number of devices in the array pairListArr as 'input parameter'
*                                    and actual number of pairs as 'output parameter'.
* @param[out] pairListManagedArray[]- array of registered pairs (no more than list length).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbDevListGet
(
    IN    GT_U32                                                   exactMatchManagerId,
    INOUT GT_U32                                                   *numOfPairsPtr,
    OUT   CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC    pairListManagedArray[]/*arrSizeVarName=*numOfPairsPtr*/
)
{
    GT_U32 numOfDevs = *numOfPairsPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    GT_U32  ii, jj;
    GT_U8   devNum;

    if(numOfDevs > PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfDevs [%d] > max[%d] ",
            numOfDevs,
            PRV_CPSS_MAX_PP_DEVICES_CNS);
    }
    else
    if(numOfDevs == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
            "numOfDevs is ZERO ... no meaning calling the function");
    }

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    jj = 0;

    /* the input number of devices should not exceed the actual number in DB */
    if (numOfDevs > exactMatchManagerPtr->numOfDevices)
    {
        numOfDevs = exactMatchManagerPtr->numOfDevices;
    }
    /* retrieve devices from EXACT_MATCH manager DB */
    for(ii = 0; ii < numOfDevs; ii++)
    {
        if(EM_IS_DEV_BMP_SET_MAC(exactMatchManagerPtr->devsBmpArr, ii))

        {
            devNum = (GT_U8)ii;
            pairListManagedArray[jj].devNum = devNum;
            pairListManagedArray[jj++].portGroupsBmp =  exactMatchManagerPtr->portGroupsBmpArr[devNum];
        }
    }

    /* actual number of retrieved devices */
    *numOfPairsPtr = jj;

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbExpandedActionUpdate function
* @endinternal
*
* @brief   The function set the Expanded Action table DB.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[in] paramsPtr             - the expanded action attributes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*         The Expander Action Entry cannot be changed if an
*         Exact Match Rule is using this entry
*/
GT_STATUS prvCpssDxChExactMatchManagerDbExpandedActionUpdate
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
)
{
    GT_STATUS rc=GT_OK;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* check no Exact Match Rule using this entry */
    if ((exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntryValid==GT_TRUE)&&
        (exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchEntriesCounter!=0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"The Expander Action Entry cannot be changed if an Exact Match Rule is using this entry:"
                                         " exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex][%d],exactMatchEntriesCounter[%d] \n",
                                      expandedActionIndex,
                                      exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchEntriesCounter);
    }

    rc = validateExactMatchExpandedActionParams(expandedActionIndex,paramsPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in validation of entryAttrPtr->expandedArray[%d]",expandedActionIndex);
    }

    /* set the action parameters in the DB*/
    cpssOsMemCpy(&exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntry,
                 paramsPtr, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC));

    /* set the entry validity in the DB */
    exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntryValid=paramsPtr->exactMatchExpandedEntryValid;

    /* reset the reducedMaskArr parameter - this will be built again in prvCpssDxChExactMatchManagerHwExpandedActionUpdate */
    cpssOsMemSet(exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].reducedMaskArr,0,sizeof(exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].reducedMaskArr));

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbExpandedActionGet function
* @endinternal
*
* @brief  The function return the Expanded Action table Entry.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in] expandedActionIndex   - Exact Match Expander table index
*                                    (APPLICABLE RANGES:0..15)
* @param[out] paramsPtr             - the expanded action attributes.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong exactMatchManagerId and paramsPtr data
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - if the Exact Match Manager is not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*  None
*/
GT_STATUS prvCpssDxChExactMatchManagerDbExpandedActionGet
(
    IN  GT_U32                                                          exactMatchManagerId,
    IN  GT_U32                                                          expandedActionIndex,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC    *paramsPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* set the action parameters from the DB*/
    cpssOsMemCpy(paramsPtr,
                 &(exactMatchManagerPtr->exactMatchExpanderArray[expandedActionIndex].exactMatchExpandedEntry),
                 sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC));

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbTableFullDiagnostic function
* @endinternal
*
* @brief   This function do diagnostic to 'Exact Match full'
*          state the caller function will return this error to
*          the application NOTE: the function always return
*          'ERROR'. the 'GT_FULL' is expected ,
*                other values are not
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
*
* @retval GT_BAD_STATE             - non-consistency between different parts of
*                                    the manager.
* @retval GT_FULL                  - the table is really FULL (all banks are full).
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbTableFullDiagnostic
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr
)
{
    GT_U32  bankId,totalPopulation;

    if(exactMatchManagerPtr->totalPopulation != exactMatchManagerPtr->capacityInfo.maxTotalEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "the Exact Match is considered FULL , but coherency error detected maxTotalEntries[%d] != totalPopulation[%d]",
            exactMatchManagerPtr->capacityInfo.maxTotalEntries,
            exactMatchManagerPtr->totalPopulation);
    }

    /* do checking of 'summary' to see that really FULL and not management
       calculation error */
    totalPopulation = 0;
    for(bankId = 0 ; bankId < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes ; bankId++)
    {
        totalPopulation += exactMatchManagerPtr->bankInfoArr[bankId].bankPopulation;
    }

    if(totalPopulation != exactMatchManagerPtr->capacityInfo.maxTotalEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "the Exact Match is considered FULL , but coherency error detected maxTotalEntries[%d] != totalPopulation[%d] (banks summary)",
            exactMatchManagerPtr->capacityInfo.maxTotalEntries,
            totalPopulation);
    }

    /* the table is FULL ? ... no need to to 'try' the entry can't be 'added'
        if already exists  ... we not update it anyway
        if not exists      ... we can't add it
    */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL,
        "the Exact Match is FULL with [%d] entries",
        exactMatchManagerPtr->capacityInfo.maxTotalEntries);
}

/**
* @internal prvCpssDxChExactMatchManagerDbGetMostPopulatedBankId function
* @endinternal
*
* @brief   This function choose the best bank to use for the entry.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] calcInfoPtr           - using calcInfoPtr->usedBanksArr[] ,
*                                    using calcInfoPtr->lastBankIdChecked
* @param[out] calcInfoPtr          - updating calcInfoPtr->selectedBankId
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - not found bank to use
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbGetMostPopulatedBankId
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC  *calcInfoPtr
)
{
    GT_U32  bankId;
    GT_U32  numUsed    = GT_NA;
    GT_U32  bestBankId = GT_NA;

    for(bankId = 0 ; bankId <= calcInfoPtr->lastBankIdChecked ; (bankId=bankId+calcInfoPtr->bankStep))
    {
        switch (calcInfoPtr->bankStep)
        {
        case 1:
            if (calcInfoPtr->usedBanksArr[bankId])
            {
                 /* if one of the banks are used , it can't be used for new entry */
                continue;
            }
            break;
        case 2:
            if ((calcInfoPtr->usedBanksArr[bankId])||(calcInfoPtr->usedBanksArr[bankId+1]))
            {
                 /* if one of the banks are used , it can't be used for new entry */
                continue;
            }
            break;
        case 3:
            if ((calcInfoPtr->usedBanksArr[bankId])||(calcInfoPtr->usedBanksArr[bankId+1])||
                (calcInfoPtr->usedBanksArr[bankId+2]))
            {
                 /* if one of the banks are used , it can't be used for new entry */
                continue;
            }
            break;
        case 4:
            if ((calcInfoPtr->usedBanksArr[bankId])||(calcInfoPtr->usedBanksArr[bankId+1])||
                (calcInfoPtr->usedBanksArr[bankId+2])||(calcInfoPtr->usedBanksArr[bankId+3]))
            {
                /* if one of the banks are used , it can't be used for new entry */
                continue;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(calcInfoPtr->bankStep);

        }

        if((numUsed == GT_NA) ||
           (numUsed < exactMatchManagerPtr->bankInfoArr[bankId].bankPopulation))
        {
            numUsed     = exactMatchManagerPtr->bankInfoArr[bankId].bankPopulation;
            bestBankId  = bankId;
        }
    }

    if(bestBankId == GT_NA)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"not found best bankId");
    }

    /* in EM bankStep can be 1,2,3,4 .
       in case of step=1 bestBankId can be any bank
       in case of step=2 bestBankId can be only even banks
       in case of step=3 bestBankId can be only a multiple of 3
       in case of step=4 bestBankId can be only a multiple of 4 */
    if((bestBankId % calcInfoPtr->bankStep)!=0)
        bestBankId = (bestBankId - (bestBankId % calcInfoPtr->bankStep));

    calcInfoPtr->selectedBankId = bestBankId;

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerDbFreeAgeBinIndexGet function
* @endinternal
*
* @brief  The function gets the next free ageBinIndex.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] numberOfIndexesNeeded - according to the size of the entry
*                                    we will need corresponding number
*                                    of ageBin indexes.
* @param[inout]lastLocationLeve2HitPtr - (pointer to) the last location of zero bit in the level2UsageBitmap
* @param[out] ageBinIndex           - (pointer to) the agebin index.
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - not found bank to use
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbFreeAgeBinIndexGet
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr,
    IN  GT_U32                                                   numberOfIndexesNeeded,
    INOUT  GT_U32                                               *lastLocationLevel1HitPtr,
    INOUT  GT_U32                                               *lastLocationLevel2HitPtr,
    OUT GT_U32                                                  *ageBinIndex
)
{
    GT_U32          level1Index, level2Index;
    GT_U32          level1Size;
    GT_U32          i,level2HitNum,pos;

    level2HitNum = *lastLocationLevel2HitPtr;
    /* find the ageBbin that can fit the needed number of indexes */
    while (GT_TRUE)
    {
        if(level2HitNum > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "(expected to have free bins to fit the entry)");
        }

        /* check the Right Most unset Bit */
        /* Check the reset bit position of LSB in the bitmap according to the level2HitNum
         * e.g  level2HitNum = 1
         *      bmp - 0001 1011 (pos = 2)
         *      bmp - 0010 0010 (pos = 0)
         * e.g  level2HitNum = 2
         *      bmp - 0001 1011 (pos = 5)
         *      bmp - 0010 0010 (pos = 2)
         **/
        if((exactMatchManagerPtr->agingBinInfo.level2UsageBitmap == 0xFFFFFFFF)||(level2HitNum==0))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"(expected at least one bit reset)");
        }
        i=0;
        for(level2Index= 0; level2Index<32; level2Index++)
        {
            if((exactMatchManagerPtr->agingBinInfo.level2UsageBitmap & (1 << level2Index)) == 0)
            {
                i++;
                if(level2HitNum==i)
                    break;
            }
        }
        level1Size = 1 + (exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated >> 5);
        /* Index validation - Level1 array index should not be more than allocated size */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(level2Index, level1Size);

        /* level2 bit position is the index to the level1Bitmap array */
        if(exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr[level2Index] == 0xFFFFFFFF)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "(expected at least one bit reset)");
        }

        for(pos=(*lastLocationLevel1HitPtr); pos<32; pos++)
        {
            if((exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr[level2Index] & (1 << pos)) == 0)
            {
                /* save the Right Most unset Bit  */
                level1Index = pos;
                *ageBinIndex = (level2Index * 32) + level1Index;

                if((exactMatchManagerPtr->agingBinInfo.ageBinListPtr[*ageBinIndex].totalUsedEntriesCnt+numberOfIndexesNeeded)<=
                   exactMatchManagerPtr->capacityInfo.maxEntriesPerAgingScan)
                {
                     *lastLocationLevel2HitPtr = level2HitNum;
                     *lastLocationLevel1HitPtr = level1Index;
                     return GT_OK;
                }
            }
        }

        /* if we get here - need to look for nother potential bin */
        level2HitNum++;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "(we could not find a valid ageBin)");
}
/*
* @internal prvCpssDxChExactMatchManagerDbAddNewEntry function
* @endinternal
*
* @brief   This function get free entry to be used for new added entry.
*           adding new entry :
*           update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[]
*           update headOfFreeList , headOfUsedList , tailOfUsedList
*           update calcInfoPtr->dbEntryPtr as valid pointer
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] calcInfoPtr           - using calcInfoPtr->selectedBankId ,
*                                          calcInfoPtr->counterType ,
*                                          calcInfoPtr->crcMultiHashArr[] ,
* @param[out] calcInfoPtr          - updating calcInfoPtr->dbEntryPtr
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE /GT_BAD_PARAM - non-consistency between different parts of
*                                    the manager.
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbAddNewEntry
(
    IN    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC    *exactMatchManagerPtr,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC  *calcInfoPtr
)
{
    GT_U32  step;
    GT_U32  bankId;
    GT_U32  hwIndex;
    GT_U32  currFree_dbIndex;
    GT_U32  oldTail_dbIndex;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *freeInPoolPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *oldTailUsedInPoolPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *oldHeadOfAgeBinInPoolPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC       *headOfOldAgeBinListPtr;
    GT_U32                                                  ageBinIndex;
    GT_STATUS                                               rc;
    GT_U32                                                  lastLocationLevel1Hit=0;
    GT_U32                                                  lastLocationLevel2Hit=1;


    /* calcInfoPtr->dbEntryPtr is NULL at this point */
    /* the entry was not found as 'existing' in the DB */

    bankId  =  calcInfoPtr->selectedBankId;

    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC((bankId+calcInfoPtr->bankStep-1),CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS);

    /* coherency check - we do not oversize the population */
    for (step=0; step<calcInfoPtr->bankStep; step++)
    {
        EM_CHECK_X_NOT_MORE_THAN_Y_MAC(
                (exactMatchManagerPtr->bankInfoArr[bankId+step].bankPopulation+1),
                exactMatchManagerPtr->maxPopulationPerBank);
    }
    /* coherency check */
    EM_CHECK_X_NOT_MORE_THAN_Y_MAC(
        (exactMatchManagerPtr->totalPopulation+calcInfoPtr->bankStep),
        exactMatchManagerPtr->capacityInfo.maxTotalEntries);


    /* only after we verify we can add the full entry we start to add it*/
    for (step=0; step<calcInfoPtr->bankStep; step++)
    {
        /*********************************************/
        /* update bankInfoArr[].bankPopulation */
        /*********************************************/
        /* update the number of entries in the bank */

        exactMatchManagerPtr->bankInfoArr[bankId+step].bankPopulation++;
        /* coherency check */
        EM_CHECK_X_NOT_MORE_THAN_Y_MAC(
            exactMatchManagerPtr->bankInfoArr[bankId+step].bankPopulation,
            exactMatchManagerPtr->maxPopulationPerBank);

       /* totalPopulation is updated according to keySize
          (number of steps = number of banks occupied in HW)
          Note: 5_BYTES entries are counted as 1 entry
                19_BYTES entries are counted as 2 entries
                33_BYTES entries are counted as 3 entries
                47_BYTES entries are counted as 4 entries */
        exactMatchManagerPtr->totalPopulation++;
        /* coherency check */
        EM_CHECK_X_NOT_MORE_THAN_Y_MAC(
            exactMatchManagerPtr->totalPopulation,
            exactMatchManagerPtr->capacityInfo.maxTotalEntries);

        hwIndex = calcInfoPtr->crcMultiHashArr[bankId+step];

        /*********************/
        /* update indexArr[] */
        /*********************/
        exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer = 1;
        /* 'take' entry from the headOfFreeList */
        exactMatchManagerPtr->indexArr[hwIndex].entryPointer = exactMatchManagerPtr->headOfFreeList.entryPointer;

        /*****************************/
        /* update the headOfFreeList */
        /*****************************/
        if(!exactMatchManagerPtr->headOfFreeList.isValid_entryPointer)
        {
            /* NOTE: this is NOT 'GT_FULL' case , because prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult(..) show that we have free space ! */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "the DB not found 'free' entry in exactMatchManagerPtr->headOfFreeList");
        }

        currFree_dbIndex = exactMatchManagerPtr->headOfFreeList.entryPointer;

        freeInPoolPtr = &exactMatchManagerPtr->entryPoolPtr[currFree_dbIndex];

        dbEntryPtr = freeInPoolPtr;

        /*****************************/
        /* update the headOfFreeList */
        /*****************************/
        if(freeInPoolPtr->isValid_nextEntryPointer)
        {
            /* use 'next' index in the pool as new 'headOfFreeList' indication */
            exactMatchManagerPtr->headOfFreeList.isValid_entryPointer = 1;
            /*exactMatchManagerPtr->headOfFreeList.entryPointer         = freeInPoolPtr->nextEntryPointer;*/
            EM_GET_nextEntryPointer_MAC(freeInPoolPtr,exactMatchManagerPtr->headOfFreeList.entryPointer);
        }
        else
        {
            /* there are no more free elements ! */
            exactMatchManagerPtr->headOfFreeList.isValid_entryPointer = 0;
        }

        /*****************************/
        /* update the headOfUsedList */
        /*****************************/
        if(!exactMatchManagerPtr->headOfUsedList.isValid_entryPointer)
        {
            /* the tail was not yet valid ... now it can be valid ! */
            exactMatchManagerPtr->headOfUsedList.isValid_entryPointer = 1;
            exactMatchManagerPtr->headOfUsedList.entryPointer         = currFree_dbIndex;
        }

        /*****************************/
        /* update the tailOfUsedList */
        /*****************************/
        if(exactMatchManagerPtr->tailOfUsedList.isValid_entryPointer)
        {
            /* Add the new entry to the tail of the list, as the new last entry.
               Connect the previous last with the new entry and set the global last entry
               pointer to the new entry.
            */
            oldTail_dbIndex = exactMatchManagerPtr->tailOfUsedList.entryPointer;

            oldTailUsedInPoolPtr = &exactMatchManagerPtr->entryPoolPtr[oldTail_dbIndex];
            oldTailUsedInPoolPtr->isValid_nextEntryPointer = 1;
            /*oldTailUsedInPoolPtr->nextEntryPointer         = currFree_dbIndex;*/
            EM_SET_nextEntryPointer_MAC(oldTailUsedInPoolPtr,currFree_dbIndex);

            /* point the 'new used' (new tail) to the previous tail */
            dbEntryPtr->isValid_prevEntryPointer = 1;
            dbEntryPtr->prevEntryPointer         = oldTail_dbIndex;
        }
        else
        {
            exactMatchManagerPtr->tailOfUsedList.isValid_entryPointer = 1;

            /* point the 'new used' to have no previous */
            dbEntryPtr->isValid_prevEntryPointer = 0;
            dbEntryPtr->prevEntryPointer         = 0;/* don't care */
        }

        exactMatchManagerPtr->tailOfUsedList.entryPointer         = currFree_dbIndex;

        /************************************/
        /* update the age bin related data */
        /************************************/
        /* Get the Free age-bin Index */
        if (step==0)
        {
            rc = prvCpssDxChExactMatchManagerDbFreeAgeBinIndexGet(exactMatchManagerPtr, calcInfoPtr->bankStep, &lastLocationLevel1Hit, &lastLocationLevel2Hit, &ageBinIndex);
        }
        else
        {
            rc = prvCpssDxChExactMatchManagerDbFreeAgeBinIndexGet(exactMatchManagerPtr,1,&lastLocationLevel1Hit, &lastLocationLevel2Hit, &ageBinIndex);
        }
        if(rc != GT_OK)
        {
            return rc;
        }

        if (ageBinIndex >= exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* Get the Head of existing age-bin list */
        headOfOldAgeBinListPtr  = &exactMatchManagerPtr->agingBinInfo.ageBinListPtr[ageBinIndex].headOfAgeBinEntryList;

        /* Add to top of the existing list */
        if(headOfOldAgeBinListPtr->isValid_entryPointer == 0)
        { /* First Node */
            dbEntryPtr->isValid_age_nextEntryPointer                = 0;
            dbEntryPtr->age_nextEntryPointer                        = 0; /* don't care */
        }
        else
        {
            oldHeadOfAgeBinInPoolPtr = &exactMatchManagerPtr->entryPoolPtr[headOfOldAgeBinListPtr->entryPointer];

            /* Old Head */
            oldHeadOfAgeBinInPoolPtr->isValid_age_prevEntryPointer  = 1;
            oldHeadOfAgeBinInPoolPtr->age_prevEntryPointer          = currFree_dbIndex;

            /* New Head */
            dbEntryPtr->age_nextEntryPointer                        = headOfOldAgeBinListPtr->entryPointer;
            dbEntryPtr->isValid_age_nextEntryPointer                = 1;
        }

        /* Update the Head of age-bin */
        headOfOldAgeBinListPtr->entryPointer                        = currFree_dbIndex;
        headOfOldAgeBinListPtr->isValid_entryPointer                = 1;
        dbEntryPtr->ageBinIndex                                     = ageBinIndex;

        /* New entry inserted at the top, so prev pointer always NOT VALID */
        dbEntryPtr->isValid_age_prevEntryPointer                    = 0;
        dbEntryPtr->age_prevEntryPointer                            = 0; /* don't care */

        EM_INCREASE_AGE_BIN_COUNTER_MAC(exactMatchManagerPtr, ageBinIndex);

        /********************************/
        /* update the entry in the pool */
        /********************************/
        dbEntryPtr->isUsedEntry = 1;
        dbEntryPtr->hwIndex     = hwIndex;
        /* point the 'new used' to have no 'next' */
        dbEntryPtr->isValid_nextEntryPointer = 0;
        /*dbEntryPtr->nextEntryPointer         = 0;*//* don't care */
        EM_SET_nextEntryPointer_MAC(dbEntryPtr,0);/* don't care */
        /* mark that this is the 'head' of the entry */
        if (step==0)
        {
            dbEntryPtr->isFirst = GT_TRUE;
        }
        else
        {
            dbEntryPtr->isFirst = GT_FALSE;
        }
        /********************************/
        /* update the typeCountersArr */
        /********************************/

        if (step==0)
        {
            /* coherency check */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                calcInfoPtr->counterClientType,
                CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E);

            /* coherency check */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                calcInfoPtr->counterKeyType,
                CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E);

            /* we only update the counter if this is the first bank of the Exact Match entry
               Exact match entry can hold 1, 2, 3, 4 banks. we will count each entry only once in the
               typeCountersArr */
            exactMatchManagerPtr->typeCountersArr[calcInfoPtr->counterClientType][calcInfoPtr->counterKeyType].entryTypePopulation++;

            /**************************************************/
            /*  save the pointer for later use , by the caller */
            /**************************************************/
            calcInfoPtr->dbEntryPtr = dbEntryPtr;
        }
    }
    return GT_OK;
}

/**
* @internal deleteOldEntryFromDbMetadata_NodesCheck function
* @endinternal
*
* @brief   This function check impact on specific 'node' that may point to removed index.
*          update the 'node' if it point to the removed entry
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] removed_dbIndex       - the db index of the removed entry
* @param[in] isValid_prevEntryPointer - indication that the removed entry hold
*                                   valid previous entry in the 'used list'.
* @param[in] prevEntryPointer      -  the previous entry index in the 'used list'.
* @param[in] nodePtr               - (pointer to) the node that need to be checked
*                                   if the 'removed' index impact it.
* @param[out] nodePtr              - (pointer to) the node updated since pointed to removed index.
*
*/
static void   deleteOldEntryFromDbMetadata_specificNode
(
    IN GT_U32                                               removed_dbIndex,
    IN GT_U32                                               isValid_prevEntryPointer,
    IN GT_U32                                               prevEntryPointer,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC   *nodePtr
)
{
    if(!nodePtr->isValid_entryPointer)
    {
        /* no impact on the 'non-valid' node */
        return;
    }

    if(nodePtr->entryPointer != removed_dbIndex)
    {
        /* no impact on this node */
        return;
    }

    /*******************/
    /* update the node */
    /*******************/
    /* we removed the entry from DB ... so need to move the node to point to
       previous entry in 'used' DB */
    if(isValid_prevEntryPointer)
    {
        /* we can move it */
        nodePtr->entryPointer = prevEntryPointer;
    }
    else
    {
        /* we can't move it --> so invalidate it */
        nodePtr->isValid_entryPointer = 0;
        nodePtr->entryPointer         = 0;/* don't care */
    }

    return;
}

/**
* @internal deleteOldEntryFromDbMetadata_NodesCheck function
* @endinternal
*
* @brief   This function check impact on ALL 'node' that may point to the removed index.
*          check impact on :
*          tailOfUsedList ,headOfUsedList , lastGetNextInfo ,lastDeleteInfo
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] removed_dbIndex       - the db index of the removed entry
* @param[in] isValid_prevEntryPointer - indication that the removed entry hold
*                                   valid previous entry in the 'used list'.
* @param[in] prevEntryPointer      -  the previous entry index in the 'used list'.
* @param[in] isValid_nextEntryPointer - indication that the removed entry hold
*                                   valid next entry in the 'used list'.
* @param[in] nextEntryPointer      -  the next entry index in the 'used list'.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE /GT_BAD_PARAM - non-consistency between different parts of
*                                    the manager.
*
*/
static GT_STATUS   deleteOldEntryFromDbMetadata_NodesCheck
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN GT_U32                                                   removed_dbIndex,
    IN GT_U32                                                   isValid_prevEntryPointer,
    IN GT_U32                                                   prevEntryPointer,
    IN GT_U32                                                   isValid_nextEntryPointer,
    IN GT_U32                                                   nextEntryPointer
)
{
    /*****************************/
    /* update the tailOfUsedList */
    /*****************************/
    EM_CHECK_X_NOT_ZERO_MAC(exactMatchManagerPtr->tailOfUsedList.isValid_entryPointer);
    deleteOldEntryFromDbMetadata_specificNode(removed_dbIndex,
                                            isValid_prevEntryPointer,
                                            prevEntryPointer,
                                            &exactMatchManagerPtr->tailOfUsedList);

    /*****************************/
    /* update the headOfUsedList */
    /*****************************/
    EM_CHECK_X_NOT_ZERO_MAC(exactMatchManagerPtr->headOfUsedList.isValid_entryPointer);
    if(exactMatchManagerPtr->headOfUsedList.entryPointer == removed_dbIndex)
    {
        /* we remove the 'head of used' ... need to move the 'head' to next */
        exactMatchManagerPtr->headOfUsedList.isValid_entryPointer = isValid_nextEntryPointer;
        exactMatchManagerPtr->headOfUsedList.entryPointer = nextEntryPointer;
    }


    /******************************/
    /* update the lastGetNextInfo */
    /******************************/
    deleteOldEntryFromDbMetadata_specificNode(removed_dbIndex,
                                            isValid_prevEntryPointer,
                                            prevEntryPointer,
                                            &exactMatchManagerPtr->lastGetNextInfo);

    /******************************/
    /* update the lastDeleteInfo  */
    /******************************/
    deleteOldEntryFromDbMetadata_specificNode(removed_dbIndex,
                                            isValid_prevEntryPointer,
                                            prevEntryPointer,
                                            &exactMatchManagerPtr->lastDeleteInfo);
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbDeleteOldEntry function
* @endinternal
*
* @brief   This function return entry to the free list removing from the used list.
*           delete old entry :
*           update metadata DB :indexArr[] , bankInfoArr[] , typeCountersArr[]
*           update headOfFreeList , headOfUsedList , tailOfUsedList
*           update calcInfoPtr->dbEntryPtr about valid pointers
*           lastGetNextInfo , lastTranslplantInfo , lastDeleteInfo
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] calcInfoPtr           - using calcInfoPtr->selectedBankId ,
*                                          calcInfoPtr->counterType ,
*                                          calcInfoPtr->dbEntryPtr ,
*                                          calcInfoPtr->crcMultiHashArr[] ,
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE /GT_BAD_PARAM - non-consistency between different parts of
*                                    the manager.
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbDeleteOldEntry
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC          *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC        *calcInfoPtr
)
{
    GT_STATUS   rc;
    GT_U32  step;
    GT_U32  bankId;
    GT_U32  dbIndex;
    GT_U32  hwIndex;
    GT_U32  currFree_dbIndex;
    GT_U32  prev_dbIndex;
    GT_U32  next_dbIndex;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC *freeInPoolPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC *dbEntryPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC *prevInPoolPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC *nextInPoolPtr;
    GT_U32 isValid_prevEntryPointer;
    GT_U32 prevEntryPointer;

    dbEntryPtr = calcInfoPtr->dbEntryPtr;
    bankId  = calcInfoPtr->selectedBankId;

    isValid_prevEntryPointer = dbEntryPtr->isValid_prevEntryPointer;
    prevEntryPointer         = dbEntryPtr->prevEntryPointer;

    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC((bankId+calcInfoPtr->bankStep-1),CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS);

    for (step=0; step<calcInfoPtr->bankStep; step++)
    {
        /*********************************************/
        /* update bankInfoArr[].bankPopulation */
        /*********************************************/
        /* coherency check -- can't be ZERO because we are going to decrement it */
        EM_CHECK_X_NOT_ZERO_MAC(exactMatchManagerPtr->bankInfoArr[bankId+step].bankPopulation);

        /* update the number of entries in the bank */
        exactMatchManagerPtr->bankInfoArr[bankId+step].bankPopulation--;

        /* coherency check -- can't be ZERO because we are going to decrement it */
        EM_CHECK_X_NOT_ZERO_MAC(exactMatchManagerPtr->totalPopulation);
        exactMatchManagerPtr->totalPopulation--;

        hwIndex = calcInfoPtr->crcMultiHashArr[bankId+step];


        EM_CHECK_X_NOT_ZERO_MAC(exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer);
        dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
        /*********************/
        /* update indexArr[] */
        /*********************/
        exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer = 0;
        exactMatchManagerPtr->indexArr[hwIndex].entryPointer         = 0;/* don't care */

        /***********************************/
        if(dbEntryPtr->isValid_nextEntryPointer)
        {
            /*next_dbIndex = dbEntryPtr->nextEntryPointer;*/
            EM_GET_nextEntryPointer_MAC(dbEntryPtr,next_dbIndex);
        }
        else
        {
            next_dbIndex = GT_NA;
        }

        if(dbEntryPtr->isValid_prevEntryPointer)
        {
            /* update the previous dbEntry to point to the next of the removed dbEntry entry */
            prev_dbIndex = dbEntryPtr->prevEntryPointer;
        }
        else
        {
            prev_dbIndex = GT_NA;
        }

        if(prev_dbIndex != GT_NA)
        {
            /***********************************/
            /* update prev entry in   the pool */
            /* to point to next  in   the pool */
            /***********************************/
            prevInPoolPtr = &exactMatchManagerPtr->entryPoolPtr[prev_dbIndex];
            if(next_dbIndex != GT_NA)
            {
                /*prevInPoolPtr->nextEntryPointer = next_dbIndex;*/
                EM_SET_nextEntryPointer_MAC(prevInPoolPtr,next_dbIndex);
            }
            else
            {
                prevInPoolPtr->isValid_nextEntryPointer = 0;
                /*prevInPoolPtr->nextEntryPointer         = 0;*//* don't care */
                EM_SET_nextEntryPointer_MAC(prevInPoolPtr,0);/* don't care */
            }
        }

        if(next_dbIndex != GT_NA)
        {
            /***********************************/
            /* update next entry in   the pool */
            /* to point to prev  in   the pool */
            /***********************************/
            nextInPoolPtr = &exactMatchManagerPtr->entryPoolPtr[next_dbIndex];
            if(prev_dbIndex != GT_NA)
            {
                nextInPoolPtr->prevEntryPointer = prev_dbIndex;
            }
            else
            {
                nextInPoolPtr->isValid_prevEntryPointer = 0;
                nextInPoolPtr->prevEntryPointer         = 0;/* don't care */
            }
        }

        /**************************************************/
        /* move curr entry from the 'used' to 'free' list */
        /**************************************************/
        if(!exactMatchManagerPtr->headOfFreeList.isValid_entryPointer)
        {
            /* the 'old' have no 'next' */
            dbEntryPtr->isValid_nextEntryPointer = 0;
            /*dbEntryPtr->nextEntryPointer         = 0;*//* don't care */
            EM_SET_nextEntryPointer_MAC(dbEntryPtr,0);/* don't care */
        }
        else
        {
            /* the 'old' have 'next' */
            dbEntryPtr->isValid_nextEntryPointer = 1;
            /*dbEntryPtr->nextEntryPointer         = exactMatchManagerPtr->headOfFreeList.entryPointer;*/
            EM_SET_nextEntryPointer_MAC(dbEntryPtr,exactMatchManagerPtr->headOfFreeList.entryPointer);
        }

        /**********************************/
        /* update the ageBin related info */
        /**********************************/
        if (dbEntryPtr->ageBinIndex >= exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        EM_CHECK_X_NOT_ZERO_MAC(exactMatchManagerPtr->agingBinInfo.ageBinListPtr[dbEntryPtr->ageBinIndex].headOfAgeBinEntryList.isValid_entryPointer);

        if(exactMatchManagerPtr->agingBinInfo.ageBinListPtr[dbEntryPtr->ageBinIndex].headOfAgeBinEntryList.entryPointer == dbIndex)
        {
            /* HEAD of the age-bin list */
            exactMatchManagerPtr->agingBinInfo.ageBinListPtr[dbEntryPtr->ageBinIndex].headOfAgeBinEntryList.isValid_entryPointer = dbEntryPtr->isValid_age_nextEntryPointer;
            exactMatchManagerPtr->agingBinInfo.ageBinListPtr[dbEntryPtr->ageBinIndex].headOfAgeBinEntryList.entryPointer = dbEntryPtr->age_nextEntryPointer;

            if(dbEntryPtr->isValid_age_nextEntryPointer)
            {
                /* Head of List is first one and does not have previous member */
                nextInPoolPtr = &exactMatchManagerPtr->entryPoolPtr[dbEntryPtr->age_nextEntryPointer];
                nextInPoolPtr->isValid_age_prevEntryPointer = 0;
            }
        }
        else
        {
            /* Middle/End of a age-Bin List */
            if(dbEntryPtr->isValid_age_prevEntryPointer == 0)
            {
                /* middle/end member must have valid previous pointer. */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }

            prevInPoolPtr = &exactMatchManagerPtr->entryPoolPtr[dbEntryPtr->age_prevEntryPointer];
            prevInPoolPtr->isValid_age_nextEntryPointer = dbEntryPtr->isValid_age_nextEntryPointer;
            prevInPoolPtr->age_nextEntryPointer         = dbEntryPtr->age_nextEntryPointer;

            /* If its not the last entry */
            if(dbEntryPtr->isValid_age_nextEntryPointer)
            {
                nextInPoolPtr = &exactMatchManagerPtr->entryPoolPtr[dbEntryPtr->age_nextEntryPointer];
                nextInPoolPtr->isValid_age_prevEntryPointer = dbEntryPtr->isValid_age_prevEntryPointer;
                nextInPoolPtr->age_prevEntryPointer         = dbEntryPtr->age_prevEntryPointer;
            }
        }
        EM_DECREASE_AGE_BIN_COUNTER_MAC(exactMatchManagerPtr, dbEntryPtr->ageBinIndex);

        /********************************/
        /* invalidate the removed entry */
        /********************************/
        EM_CHECK_X_NOT_ZERO_MAC(dbEntryPtr->isUsedEntry);
        dbEntryPtr->isUsedEntry = 0;
        dbEntryPtr->hwIndex     = 0;/* don't care */
        dbEntryPtr->isValid_prevEntryPointer = 0;
        dbEntryPtr->prevEntryPointer = 0;/* don't care */
        dbEntryPtr->isFirst = GT_FALSE;
        /*****************************/
        /* update the headOfFreeList */
        /*****************************/
        if(!exactMatchManagerPtr->headOfFreeList.isValid_entryPointer)
        {
            exactMatchManagerPtr->headOfFreeList.isValid_entryPointer = 1;
        }
        else
        {
            currFree_dbIndex = exactMatchManagerPtr->headOfFreeList.entryPointer;

            freeInPoolPtr = &exactMatchManagerPtr->entryPoolPtr[currFree_dbIndex];

            freeInPoolPtr->isValid_prevEntryPointer = 1;
            freeInPoolPtr->prevEntryPointer         = dbIndex;
        }

        exactMatchManagerPtr->headOfFreeList.entryPointer = dbIndex;

        /********************************************/
        /* check impact on : tailOfUsedList         */
        /* check impact on : headOfUsedList         */
        /* check impact on : lastGetNextInfo        */
        /* check impact on : lastTranslplantInfo    */
        /* check impact on : lastDeleteInfo         */
        /********************************************/
        /* check impact on ALL 'node' that may point to the removed index */
        rc = deleteOldEntryFromDbMetadata_NodesCheck(exactMatchManagerPtr,dbIndex,
                                                     isValid_prevEntryPointer,prevEntryPointer,
                                                     (next_dbIndex != GT_NA)?1:0,next_dbIndex);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* counter is updated only once for the head of the entry */
        if (step==0)
        {
            /********************************/
            /* update the typeCountersArr   */
            /********************************/

             /* coherency check */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                calcInfoPtr->counterClientType,
                CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E);

            /* coherency check */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(
                calcInfoPtr->counterKeyType,
                CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E);

            /* coherency check -- can't be ZERO because we are going to decrement it */
            EM_CHECK_X_NOT_ZERO_MAC(exactMatchManagerPtr->typeCountersArr[calcInfoPtr->counterClientType][calcInfoPtr->counterKeyType].entryTypePopulation);
            exactMatchManagerPtr->typeCountersArr[calcInfoPtr->counterClientType][calcInfoPtr->counterKeyType].entryTypePopulation--;
        }

        /* if we are not in the last step then need to update dbEntryPtr*/
        if((step!=(calcInfoPtr->bankStep-1))&&(next_dbIndex != GT_NA))
        {
            /************************************/
            /* move to next element in the list */
            /************************************/
            /* we need to invalidate all elements related to the entry
               only the first element holds the data of the exact match entry
               the rest are used to hold if the index is in used */
            dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[next_dbIndex];
            isValid_prevEntryPointer = dbEntryPtr->isValid_prevEntryPointer;
            prevEntryPointer         = dbEntryPtr->prevEntryPointer;
        }
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerDbHashCalculate function
* @endinternal
*
* @brief   This function calculates hash for the new/existing entry.
*
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbHashCalculate
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                      *entryPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC    *calcInfoPtr
)
{
    GT_STATUS                   rc;
    CPSS_EXACT_MATCH_MHT_ENT    exactMatchBanksNum;
    CPSS_EXACT_MATCH_TABLE_SIZE_ENT exactMatchSize;

    switch(entryPtr->exactMatchActionType)
    {
        case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
        case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:
              break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchActionType);
    }

    switch(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes)
    {
        case 4:
            exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_4_E;
            break;
        case 8:
            exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_8_E;
            break;
        case 16:
            exactMatchBanksNum=CPSS_EXACT_MATCH_MHT_16_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchActionType);
    }

    switch(exactMatchManagerPtr->cpssHashParams.size)
    {
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_8K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_8KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_16K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_16KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_32K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_32KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_64K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_64KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_128K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_128KB;
            break;
        case PRV_CPSS_DXCH_EXACT_MATCH_TBL_SIZE_256K_E:
            exactMatchSize=CPSS_EXACT_MATCH_TABLE_SIZE_256KB;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(exactMatchManagerPtr->cpssHashParams.size);
    }

    /*******************************/
    /* calc the 4/8/16 hash values */
    /*******************************/
    rc = prvCpssDxChExactMatchHashCrcMultiResultsByParamsCalcExt(exactMatchSize,
                                                                 exactMatchBanksNum,
                                                                 &entryPtr->exactMatchEntry.key,
                                                                 &calcInfoPtr->numberOfElemInCrcMultiHashArr,
                                                                 &calcInfoPtr->crcMultiHashArr[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    switch(entryPtr->exactMatchEntry.key.keySize)
        {
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
        calcInfoPtr->bankStep = 1;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
        calcInfoPtr->bankStep = 2;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
        calcInfoPtr->bankStep = 3;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
        calcInfoPtr->bankStep = 4;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(entryPtr->exactMatchEntry.key.keySize);
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbIsEntryKeyMatchDbEntryKey function
* @endinternal
*
* @brief   The function check if the key of the entry from application (entryPtr)
*          match the key of the entry from the manager DB (dbEntryPtr).
*          the result retrieved in (*isMatchPtr)
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[in] dbEntryPtr            - (pointer to) the entry (manager format)
*                                     the function actually uses only the 'key' of the entry.
* @param[out] isMatchPtr           - (pointer to) is the key match ?
*                                    GT_TRUE  - the key     match
*                                    GT_FALSE - the key not match
*
* @retval GT_OK                    - on success
* @retval other                    - unexpected error
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbIsEntryKeyMatchDbEntryKey
(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                      *entryPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC        *dbEntryPtr,
    OUT GT_BOOL                                                     *isMatchPtr
)
{
    GT_BOOL                                 isEqual=GT_TRUE;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT      exactMatchEntryKeySize;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT        exactMatchEntryLookupNum;

    *isMatchPtr = GT_FALSE;

    if (dbEntryPtr->isFirst != GT_TRUE)
    {
        /* this is not the 'head' of the entry ... so check next index */
        return GT_OK;
    }

    switch(dbEntryPtr->hwExactMatchKeySize)
    {
        case 0:
            exactMatchEntryKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
            break;
        case 1:
            exactMatchEntryKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
            break;
        case 2:
            exactMatchEntryKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
            break;
        case 3:
            exactMatchEntryKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
            break;
        default:
            return GT_OK;
    }
    if(exactMatchEntryKeySize != entryPtr->exactMatchEntry.key.keySize)
    {
        /* the entry type in ExactMatch manager is not the same ... so check next index */
        return GT_OK;
    }

    switch(dbEntryPtr->hwExactMatchLookupNum)
    {
        case 0:
            exactMatchEntryLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
            break;
        case 1:
            exactMatchEntryLookupNum = CPSS_DXCH_EXACT_MATCH_LOOKUP_SECOND_E;
            break;
        default:
            return GT_OK;
    }
    if(exactMatchEntryLookupNum != entryPtr->exactMatchEntry.lookupNum)
    {
        /* the entry type in ExactMatch manager is not the same ... so check next index */
        return GT_OK;
    }

    isEqual = (cpssOsMemCmp(&(entryPtr->exactMatchEntry.key.pattern[0]),&(dbEntryPtr->pattern[0]), sizeof(GT_U8)*CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS)==0) ? GT_TRUE : GT_FALSE;
    if (isEqual==GT_FALSE)
    {
         /* the entry type in ExactMatch manager is not the same ... so check next index */
        return GT_OK;
    }

    *isMatchPtr = GT_TRUE;
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerBankCalcInfoGet function
* @endinternal
*
* @brief   This function gets DB calc info per bank in hash calculated in prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match manager
* @param[in] bankId                 - bank ID to get calc info
* @param[in] entryPtr               - (pointer to) the current Exact Match entry need to be rehashed
* @param[in] addParamsPtr           - (pointer to) parameters used for rehash
* @param[inout] calcInfoPtr         - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS  prvCpssDxChExactMatchManagerBankCalcInfoGet
(
    IN    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN    GT_U32                                                    bankId,
    IN    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                   *entryPtr,
    IN    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC        *addParamsPtr,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC  *calcInfoPtr
)
{
    GT_STATUS                                               rc;
    GT_U32                                                  hwIndex,dbIndex;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INDEX_STC          *dbIndexPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    GT_BOOL                                                 isMatch;
    GT_U32                                                  freeBanksCounter=0;
    GT_U32                                                  usedBanksCounter=0;
    GT_U32                                                  step;

    /* to avoind warnings - may be used wehn doing rehash */
    addParamsPtr = addParamsPtr;

    for (step=0;step<calcInfoPtr->bankStep;step++)
    {
        /* sanity check */
        if ((bankId + step)>=calcInfoPtr->numberOfElemInCrcMultiHashArr)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,
                    "ERROR - illegal case when bankId+step is bigger then number of elements defined "
                    "in the index hash array :bankId[%d],step[%d], numberOfElemInCrcMultiHashArr[%d]",
                    bankId,step,calcInfoPtr->numberOfElemInCrcMultiHashArr);
        }
        hwIndex = calcInfoPtr->crcMultiHashArr[bankId + step];

        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);


        dbIndexPtr = &exactMatchManagerPtr->indexArr[hwIndex];
        calcInfoPtr->usedBanksArr[bankId+step] = dbIndexPtr->isValid_entryPointer;

        if(!dbIndexPtr->isValid_entryPointer)
        {
            /* the index is not used --> good until here ,
               keep checking if rest of the indexes are free */
            freeBanksCounter++;
        }
        else
        {
            dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndexPtr->entryPointer];
            /* do sanity check of 'good' value in the DB */
            if(!dbEntryPtr->isUsedEntry)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                    "hwIndex[%d] , dbIndex[%d] : entry expected to be isUsedEntry = 1 , but is not as expected",
                    hwIndex,
                    dbIndexPtr->entryPointer);
            }
            /* do sanity check of 'good' value in the DB */
            EM_CHECK_X_EQUAL_Y_MAC(hwIndex,dbEntryPtr->hwIndex);

            usedBanksCounter++;
        }
    }

    /* if all the indexes we checked are free */
    if (freeBanksCounter==calcInfoPtr->bankStep)
    {
         /* if we got here it means we did not found the entry but we have found potential place for it */
        calcInfoPtr->calcEntryType     = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_FREE_E;

        /* the caller need to select proper 'free index' depend on 'most papulated bank' */
        calcInfoPtr->dbEntryPtr        = NULL;
        return GT_OK;
    }
    if (usedBanksCounter!=calcInfoPtr->bankStep)
    {
        /* one of the parts is not free, so can't compare with my entry and can't use as free */
        return GT_OK;
    }

    /* if we got here we have a potential entry to check*/

    /* check if the entry already exists */
    hwIndex = calcInfoPtr->crcMultiHashArr[bankId];
    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

    dbIndexPtr = &exactMatchManagerPtr->indexArr[hwIndex];
    dbIndex = dbIndexPtr->entryPointer;

    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex,exactMatchManagerPtr->capacityInfo.maxTotalEntries);

    dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];

    /* do sanity check of 'good' value in the DB */
    if(!dbEntryPtr->isUsedEntry)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
            "hwIndex[%d] , dbIndex[%d] : entry expected to be isUsedEntry = 1 , but is not as expected",
            hwIndex,
            dbIndex);
    }

    /* do sanity check of 'good' value in the DB */
    EM_CHECK_X_EQUAL_Y_MAC(hwIndex,dbEntryPtr->hwIndex);

    if (entryPtr)
    {
        /* check if the key of the entry from application (entryPtr) match
                    the key of the entry from the manager DB (dbEntryPtr) */
        rc = prvCpssDxChExactMatchManagerDbIsEntryKeyMatchDbEntryKey(entryPtr,dbEntryPtr,&isMatch);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(isMatch == GT_FALSE)
        {
            /* the entry in the DB not match the entry that application trying to add */
            return GT_OK;
        }
    }

    calcInfoPtr->dbEntryPtr        = dbEntryPtr;
    calcInfoPtr->calcEntryType     = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E;
    calcInfoPtr->lastBankIdChecked = bankId;
    calcInfoPtr->selectedBankId    = bankId;

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbEntryReadByHwIndex
*           function
* @endinternal
*
* @brief   This function retrieves Exact Match entry from
*          DB in specific hwIndex.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match manager
* @param[in] hwIndex                - hardware index to read from
* @param[out] entryPtr              - (pointer to) Exact Match entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS  prvCpssDxChExactMatchManagerDbEntryReadByHwIndex
(
    IN    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN    GT_U32                                                    hwIndex,
    OUT   CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                   *entryPtr
)
{
    GT_STATUS                                               rc;
    GT_U32                                                  dbIndex;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INDEX_STC          *dbIndexPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;

    /* check if the entry already exists */

    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(hwIndex,exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes);

    dbIndexPtr = &exactMatchManagerPtr->indexArr[hwIndex];
    dbIndex = dbIndexPtr->entryPointer;

    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex,exactMatchManagerPtr->capacityInfo.maxTotalEntries);

    dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
    if (dbEntryPtr)
    {
        /* do sanity check of 'good' value in the DB */
        if(!dbEntryPtr->isUsedEntry)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                "hwIndex[%d] , dbIndex[%d] : entry expected to be isUsedEntry = 1 , but is not as expected",
                hwIndex,
                dbIndex);
        }

        /* do sanity check of 'good' value in the DB */
        EM_CHECK_X_EQUAL_Y_MAC(hwIndex,dbEntryPtr->hwIndex);

        /* convert DB format to application format*/
        rc = prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb(dbEntryPtr, entryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        entryPtr = NULL;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult function
* @endinternal
*
* @brief   This function hash calculate place for the new/existing entry.
*          the HASH info filled into calcInfoPtr , with indication if entry found ,
*          or not found and no place for it ,
*          or not found but have place for it.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)
*                                     the function actually uses only the 'key' of the entry.
* @param[in] addParamsPtr          - (pointer to) info given for 'entry add' that
*                                     may use specific bank from the hash.
*                                     NOTE: ignored if NULL
* @param[out] calcInfoPtr          - (pointer to) the calculated hash and lookup results
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbCalcHashAndLookupResult
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                      *entryPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC           *addParamsPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC    *calcInfoPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      bankId,chosenBankId=0;
    GT_BOOL                     foundValidEntry=GT_FALSE;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_BOOL                       managerHwWriteBlock;

    /* calculate hash for Exact Match entry */
    rc = prvCpssDxChExactMatchManagerDbHashCalculate(exactMatchManagerPtr, entryPtr, calcInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
    /*  set defaults */
    calcInfoPtr->calcEntryType     = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_NOT_FOUND_AND_NO_FREE_E;
    calcInfoPtr->dbEntryPtr        = NULL;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_EXACT_MATCH_MANAGER_E);
    if ((((tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
          (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)) || (managerHwWriteBlock == GT_TRUE) )&&
        (addParamsPtr!=NULL)) /* only in add case we need to get into this code */
    {
        /* get the valid index option out of all the options given in calcInfoPtr.
           the code will go over the options and will check if there is the same
           valid entry in the HW, if there is then the proper chosen index will
           be set and later on the value from the HW will be set in the SW.
           if a match is not found then need to return GT_FAIL - this should not
           happen in HA since the assumption is that the application replay exactly
           the same entries that was added before the crash */
        for( bankId = 0 ;
             ((bankId < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes)&&
              (bankId < calcInfoPtr->numberOfElemInCrcMultiHashArr));
             bankId += calcInfoPtr->bankStep)
        {
            /* find if there is a match to a valid Index in the HA tempDb */
            rc = prvCpssDxChExactMatchManagerDbFindValidIndexInHaTempDb(exactMatchManagerPtr,entryPtr,bankId,calcInfoPtr,&foundValidEntry);
            if (rc!=GT_OK)
            {
                return rc;
            }
            if (foundValidEntry==GT_TRUE)
            {
                chosenBankId = bankId;
                break;
            }
        }
        if (foundValidEntry!=GT_TRUE)
        {
            /* in this case we have a replay of an entry that is not in the HW. We need to add it to the HW */

            /* entry is keept to be added later after we exit the CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, "HA - RECOVERY_HW_CATCH_UP_STATE : replay of an entry that is not in the HW");
        }
        /* need to call the following code only with the correct index from HW
           this is for SW update reassons */
        rc = prvCpssDxChExactMatchManagerBankCalcInfoGet(exactMatchManagerPtr, chosenBankId, entryPtr, addParamsPtr, calcInfoPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        if (calcInfoPtr->calcEntryType == PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
        {
            /* if we are in HA process and we get here it means that the entry was already replayed
               so need to return GT_OK */
            calcInfoPtr->selectedBankId = chosenBankId;
            calcInfoPtr->lastBankIdChecked = chosenBankId;
            return GT_OK;
        }

        calcInfoPtr->selectedBankId = chosenBankId;
        calcInfoPtr->lastBankIdChecked = chosenBankId;
    }
    else
    {
        for( bankId = 0 ;
             (bankId < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes)&&(bankId<calcInfoPtr->numberOfElemInCrcMultiHashArr);
             bankId += calcInfoPtr->bankStep)
        {

            rc = prvCpssDxChExactMatchManagerBankCalcInfoGet(exactMatchManagerPtr, bankId, entryPtr, addParamsPtr, calcInfoPtr);
            if(rc != GT_OK)
            {
                return rc;
            }

            if (calcInfoPtr->calcEntryType == PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CALC_ENTRY_TYPE_FOUND_EXISTS_E)
            {
                return GT_OK;
            }
        }

        calcInfoPtr->selectedBankId = GT_NA;
        calcInfoPtr->lastBankIdChecked = bankId - calcInfoPtr->bankStep;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerRehashBankEntryGet function
* @endinternal
*
* @brief   This function retrieves Exact Match entry from
*          calcInfo per bank from HASH calculated in
*          prvCpssDxChExactMatchManagerDbHashCalculate.
*
* @param[in] exactMatchManagerPtr   - (pointer to) the Exact Match manager
* @param[in] bankId                 - bank ID to get rehashing calc info
* @param[in] calcInfoPtr            - (pointer to) the calculated hash and lookup results
* @param[out]entryPtr               - (pointer to) the current Exact Match entry that need to be rehashed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
GT_STATUS  prvCpssDxChExactMatchManagerRehashBankEntryGet
(
    IN      PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC      *exactMatchManagerPtr,
    IN      GT_U32                                                      bankId,
    IN      PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC    *calcInfoPtr,
    OUT     CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                     *entryPtr
)
{
    GT_STATUS   rc;

    rc = prvCpssDxChExactMatchManagerBankCalcInfoGet(exactMatchManagerPtr, bankId, NULL, NULL, calcInfoPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (calcInfoPtr->dbEntryPtr)
    {
        /* convert DB format to application format*/
        rc = prvCpssDxChExactMatchManagerRestoreEntryFormatParamsFromDb(calcInfoPtr->dbEntryPtr, entryPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        entryPtr = NULL;
    }

    return GT_OK;
}
/**
* @internal prvCpssDxChExactMatchManagerDbApplicationIteratorEntryGetNext function
* @endinternal
*
* @brief   The function return the first/next (pointer to) entry.
*          it is based on 'internally managed last entry' that the manager
*          returned 'last time' to the application.
*
*          NOTE: this function is to be used only for APPLICATION specific 'get next'
*                and not for iterators inside the manager.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] getFirst              - indication to get First (GT_TRUE)  or the next (GT_FALSE)
* @param[inout] lastGetNextInfoPtr - Indicates the last DB node fetched by this scan
*                                    OR - DB pointer from where to start the next search.
* @param[out] dbEntryPtrPtr        - (pointer to)  the '(pointer to)first/next entry'
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PARAM             - the iterator is not valid for the 'get next' operation
*                                    the logic found coherency issue.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbApplicationIteratorEntryGetNext
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr,
    IN GT_BOOL                                                  getFirst,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC    *lastGetNextInfoPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC  **dbEntryPtrPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    GT_U32                                                  dbIndex;
    GT_U32                                                  i,step=0;

    if(getFirst == GT_FALSE &&
       !lastGetNextInfoPtr->isValid_entryPointer)
    {
        /* the caller invalidated the 'get next' that was the first entry on previous call */
        getFirst = GT_TRUE;
    }

    if(getFirst == GT_TRUE)
    {
        *lastGetNextInfoPtr = exactMatchManagerPtr->headOfUsedList;
        if(!lastGetNextInfoPtr->isValid_entryPointer)
        {
            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }
    }

    dbIndex = lastGetNextInfoPtr->entryPointer;
    /* protect access to array out of range */
    EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex,exactMatchManagerPtr->capacityInfo.maxTotalEntries);

    dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
    /* key size 0 Exact Match key size 5 Bytes   --> step=1
       key size 1 Exact Match key size 19 Bytes  --> step=2
       key size 2 Exact Match key size 33 Bytes  --> step=3
       key size 3 Exact Match key size 47 Bytes  --> step=4 */
    step =  dbEntryPtr->hwExactMatchKeySize+1;

    if(getFirst != GT_TRUE)
    {
        /* check that we have valid 'next' to jump to it */
        /* first we need to jump all entries related to the
           current entry and only then we need to check */
        for (i=1;i<step;i++) /*start from 1 since we already have the first instance in dbEntryPtr */
        {
            /*dbIndex = dbEntryPtr->nextEntryPointer;*/
            EM_GET_nextEntryPointer_MAC(dbEntryPtr,dbIndex);

            /* protect access to array out of range */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex,exactMatchManagerPtr->capacityInfo.maxTotalEntries);

            dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        }
        if(!dbEntryPtr->isValid_nextEntryPointer)
        {
            /* not updating lastGetNextInfoPtr ! */

            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }

        /* jump to next entry */
       EM_GET_nextEntryPointer_MAC(dbEntryPtr,dbIndex);

        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex,exactMatchManagerPtr->capacityInfo.maxTotalEntries);

        dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];

        /* update the 'getNext' last info */
        lastGetNextInfoPtr->isValid_entryPointer = 1;
        lastGetNextInfoPtr->entryPointer         = dbIndex;
    }

    *dbEntryPtrPtr = dbEntryPtr;

    return GT_OK;
}
/*
* @internal prvCpssDxChExactMatchManagerDbCountersGet function
* @endinternal
*
* @brief The function fetches Exact Match Manager counters.
*
*
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] countersPtr          - (pointer to) Exact Match Manager Counters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
void prvCpssDxChExactMatchManagerDbCountersGet
(
    IN GT_U32                                           exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC      *countersPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                        counterClientType;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                      counterKeySize;
    GT_U32                                                  *counterValuePtr;
    GT_U32                                                  usedEntries;
    GT_U32                                                  bankId;

    /* we have valid new manager pointer */
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    cpssOsMemSet(countersPtr, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC));

    usedEntries = 0;
    counterClientType = CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
    while (counterClientType < CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E)
    {
        counterKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        while (counterKeySize < CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E)
        {
            switch (counterClientType)
            {
            case CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E:
                counterValuePtr = &countersPtr->ttiClientKeySizeEntriesArray[counterKeySize];
                break;
            case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E:
                counterValuePtr = &countersPtr->ipcl0ClientKeySizeEntriesArray[counterKeySize];
                break;
            case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E:
                counterValuePtr = &countersPtr->ipcl1ClientKeySizeEntriesArray[counterKeySize];
                break;
            case CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E:
                counterValuePtr = &countersPtr->ipcl2ClientKeySizeEntriesArray[counterKeySize];
                break;
            case CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E:
                counterValuePtr = &countersPtr->epclClientKeySizeEntriesArray[counterKeySize];
                break;
            default:
                return;
            }

            if (counterValuePtr)
            {
                *counterValuePtr = exactMatchManagerPtr->typeCountersArr[counterClientType][counterKeySize].entryTypePopulation;

                 /* each entry hold diferrent number of indexes
                    5  Bytes key size = 1 index
                    19 Bytes key size = 2 index
                    33 Bytes key size = 3 index
                    47 Bytes key size = 4 index */
                usedEntries += ((*counterValuePtr) * (counterKeySize+1));
            }
            counterKeySize++;
        }
        counterClientType++;
    }

    /* number of used entries

       Summary of all of the above
       each above entry hold different number of index according to the keySize
       5 Bytes key size  = 1 index
       19 Bytes key size = 2 index
       33 Bytes key size = 3 index
       47 Bytes key size = 4 index */
    countersPtr->usedEntriesIndexes = usedEntries;

    countersPtr->freeEntriesIndexes = exactMatchManagerPtr->capacityInfo.maxTotalEntries - usedEntries;

    for (bankId = 0; bankId < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS; bankId++)
    {
        /* counters per bank */
        countersPtr->bankCounters[bankId] = exactMatchManagerPtr->bankInfoArr[bankId].bankPopulation;
    }
}

/**
* @internal prvCpssDxChExactMatchManagerDbStatisticsGet function
* @endinternal
*
* @brief The function fetches Exact Match Manager statistics.
*
*
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] statisticsPtr        - (pointer to) Exact Match Manager Statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
void prvCpssDxChExactMatchManagerDbStatisticsGet
(
    IN GT_U32                                           exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC    *statisticsPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC      *exactMatchManagerPtr;
    GT_U32                                                      statisticType;
    GT_U32                                                      *statisticValuePtr;

    /* we have valid new manager pointer */
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    cpssOsMemSet(statisticsPtr, 0, sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_STATISTICS_STC));

    /* add statistics */
    statisticType = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_INPUT_INVALID_E;
    while (statisticType < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_BAD_STATE_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorBadState;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_ENTRY_EXIST_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorEntryExist;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_TABLE_FULL_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorTableFull;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_REPLAY_ENTRY_NOT_FOUND_E:
                statisticValuePtr = &statisticsPtr->entryAddErrorReplayEntryNotFound;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_HW_UPDATE_E:
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_ERROR_OTHER_E:
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_0_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStage0;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_1_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStage1;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_2_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStage2;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_3_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStage3;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_4_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStage4;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_API_STATISTICS_OK_REHASHING_STAGE_5_AND_ABOVE_E:
                statisticValuePtr = &statisticsPtr->entryAddOkRehashingStageMoreThanFour;
                break;
            default:
                break;
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = exactMatchManagerPtr->apiEntryAddStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* delete statistics */
    statisticType = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E;
    while (statisticType < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->entryDeleteErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_BAD_STATE_E:
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_ENTRY_NOT_FOUND_E:
                statisticValuePtr = &statisticsPtr->entryDeleteErrorNotfound;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_ERROR_HW_UPDATE_E:
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_DELETE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->entryDeleteOk;
                break;
            default:
                break;
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = exactMatchManagerPtr->apiEntryDeleteStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* update statistics */
    statisticType = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_INPUT_INVALID_E;
    while (statisticType < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->entryUpdateErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_BAD_STATE_E:
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_NOT_FOUND_E:
                statisticValuePtr = &statisticsPtr->entryUpdateErrorNotFound;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_NOT_ALLOWED_E:
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_ERROR_HW_UPDATE_E:
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_UPDATE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->entryUpdateOk;
                break;
            default:
                continue;
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = exactMatchManagerPtr->apiEntryUpdateStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* aging scan scan statistics */
    statisticType = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_OK_E;
    while (statisticType < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->scanAgingOk;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->scanAgingErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_E:
                statisticValuePtr = &statisticsPtr->scanAgingTotalAgedOutEntries;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_AGE_API_STATISTICS_TOTAL_AGEDOUT_DELETED_E:
                statisticValuePtr = &statisticsPtr->scanAgingTotalAgedOutDeleteEntries;
                break;
            default:
                continue;
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = exactMatchManagerPtr->apiAgingScanStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* Delete scan scan statistics */
    statisticType = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_OK_E;
    while (statisticType < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->scanDeleteOk;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->scanDeleteErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_OK_NO_MORE_E:
                statisticValuePtr = &statisticsPtr->scanDeleteOkNoMore;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_SCAN_DELETE_API_STATISTICS_TOTAL_DELETED_E:
                statisticValuePtr = &statisticsPtr->scanDeleteTotalDeletedEntries;
                break;
            default:
                continue;
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = exactMatchManagerPtr->apiDeleteScanStatisticsArr[statisticType];
        }
        statisticType++;
    }

    /* Entries rewrite statistics */
    statisticType = PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_OK_E;
    while (statisticType < PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS___LAST___E)
    {
        statisticValuePtr = NULL;
        switch (statisticType)
        {
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_OK_E:
                statisticValuePtr = &statisticsPtr->entriesRewriteOk;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_ERROR_INPUT_INVALID_E:
                statisticValuePtr = &statisticsPtr->entriesRewriteErrorInputInvalid;
                break;
            case PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRIES_REWRITE_API_STATISTICS_TOTAL_REWRITE_E:
                statisticValuePtr = &statisticsPtr->entriesRewriteTotalRewrite;
                break;
            default:
                break;
        }

        if (statisticValuePtr)
        {
            *statisticValuePtr = exactMatchManagerPtr->apiEntriesRewriteStatisticsArr[statisticType];
        }
        statisticType++;
    }
}

/**
* @internal prvCpssDxChExactMatchManagerDbStatisticsClear function
* @endinternal
*
* @brief The function clears Exact Match Manager statistics.
*
*
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
*/
void prvCpssDxChExactMatchManagerDbStatisticsClear
(
    IN GT_U32       exactMatchManagerId
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr;

    /* we have valid new manager pointer */
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /* clear add statistics */
    cpssOsMemSet(exactMatchManagerPtr->apiEntryAddStatisticsArr, 0, sizeof(exactMatchManagerPtr->apiEntryAddStatisticsArr));
    /* clear delete statistics */
    cpssOsMemSet(exactMatchManagerPtr->apiEntryDeleteStatisticsArr, 0, sizeof(exactMatchManagerPtr->apiEntryDeleteStatisticsArr));
    /* clear update statistics */
    cpssOsMemSet(exactMatchManagerPtr->apiEntryUpdateStatisticsArr, 0, sizeof(exactMatchManagerPtr->apiEntryUpdateStatisticsArr));
    /* clear Aging statistics */
    cpssOsMemSet(exactMatchManagerPtr->apiAgingScanStatisticsArr, 0, sizeof(exactMatchManagerPtr->apiAgingScanStatisticsArr));
    /* clear Delete statistics */
    cpssOsMemSet(exactMatchManagerPtr->apiDeleteScanStatisticsArr, 0, sizeof(exactMatchManagerPtr->apiDeleteScanStatisticsArr));
    /* clear rewrite statistics */
    cpssOsMemSet(exactMatchManagerPtr->apiEntriesRewriteStatisticsArr, 0, sizeof(exactMatchManagerPtr->apiEntriesRewriteStatisticsArr));
}

/**
* @internal prvCpssDxChExactMatchManagerDbAgeBinEntryGetNext function
* @endinternal
*
* @brief   The function return the first/next (pointer to) age-bin entry.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[in] getFirst              - indication to get First (GT_TRUE) or the next (GT_FALSE)
* @param[in] ageBinID              - The age-bin ID to scan for entries.
* @param[inout] lastGetNextInfoPtr - Indicates the last DB node in the ageBin fetched by this agingscan
*                                    OR - DB pointer from where to start the next search in the ageBin.
* @param[out] dbEntryPtrPtr        - (pointer to)  the '(pointer to)first/next age-bin entry'
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - if no more entries to retrieve
* @retval GT_BAD_PARAM             - the iterator is not valid for the 'get next' operation
*                                    the logic found coherency issue.
*
* @note
*   NONE
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbAgeBinEntryGetNext
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    IN GT_BOOL                                                  getFirst,
    IN GT_U32                                                   ageBinID,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC     *lastGetNextInfoPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   **dbEntryPtrPtr
)
{
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    GT_U32                                                  dbIndex;
    GT_U32                                                  hwIndex=0;

    CPSS_NULL_PTR_CHECK_MAC(lastGetNextInfoPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbEntryPtrPtr);

    if((getFirst == GT_FALSE) && (!lastGetNextInfoPtr->isValid_entryPointer))
    {
        /* the caller invalidated the 'get next' that was the first entry on previous call */
        getFirst = GT_TRUE;
    }

    if(getFirst == GT_TRUE)
    {
        if (ageBinID >= exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        *lastGetNextInfoPtr = exactMatchManagerPtr->agingBinInfo.ageBinListPtr[ageBinID].headOfAgeBinEntryList;
        if(!lastGetNextInfoPtr->isValid_entryPointer)
        {
            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }

        dbIndex = lastGetNextInfoPtr->entryPointer;

        dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];

        /* if this is a first index indication then we can work with this
           if not we need to find the first index coresponding to the index we got */
        hwIndex = dbEntryPtr->hwIndex;
        while(dbEntryPtr->isFirst==GT_FALSE)
        {
            /* go reverse untill first index is found */
            if (hwIndex!=0)
            {
                 hwIndex--;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "FAIL: unexpected case,hwIndex is 0 and and can not be deducted \n");
            }

            dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
            dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        }
        /* update the 'getNext' last info */
        lastGetNextInfoPtr->isValid_entryPointer = 1;
        lastGetNextInfoPtr->entryPointer         = dbIndex;
    }
    else
    {
        dbIndex = lastGetNextInfoPtr->entryPointer;

        dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];

        /* check that we have valid 'next' to jump to it */
        if(!dbEntryPtr->isValid_age_nextEntryPointer)
        {
            /* not updating lastGetNextInfoPtr ! */

            return /* NOT to register as error to LOG !!! */ GT_NO_MORE;
        }

        /* jump to next entry */
        dbIndex = dbEntryPtr->age_nextEntryPointer;
        dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];

        if((dbEntryPtr->ageBinIndex != ageBinID) || (!exactMatchManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer))
        {
           *dbEntryPtrPtr = dbEntryPtr; /* Updating entry info to intimate the caller */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /* if this is a first index indication then we can work with this
           if not we need to find the first index coresponding to the index we got */
        hwIndex = dbEntryPtr->hwIndex;
        while(dbEntryPtr->isFirst==GT_FALSE)
        {
            /* go reverse untill first index is found */
            if (hwIndex!=0)
            {
                 hwIndex--;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "FAIL: unexpected case,hwIndex is 0 and and can not be deducted \n");
            }

            dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
            dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];

            if((dbEntryPtr->ageBinIndex != ageBinID) || (!exactMatchManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer))
            {
               *dbEntryPtrPtr = dbEntryPtr; /* Updating entry info to intimate the caller */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
        /* update the 'getNext' last info */
        lastGetNextInfoPtr->isValid_entryPointer = 1;
        lastGetNextInfoPtr->entryPointer         = dbIndex;
    }

    *dbEntryPtrPtr = dbEntryPtr;

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerDbCheck_global function
* @endinternal
*
* @brief This function verifies Exact Match manager global parameters.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbCheck_global
(
    IN  GT_U32                                              exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT  resultArray[],
    OUT GT_U32                                              *errorNumPtr
)
{
    GT_STATUS                                               rc = GT_OK;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC      entryAttr;
    GT_U32                                                  i;

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    /**** Verify global default params ****/
    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;i++)
    {
        cpssOsMemCpy(&entryAttr.expandedArray[i],
                     &exactMatchManagerPtr->exactMatchExpanderArray[i].exactMatchExpandedEntry,
                     sizeof(CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_SET_PARAMS_STC));
    }

    rc = validateNewManagerParams(exactMatchManagerId,
                                  &exactMatchManagerPtr->capacityInfo,
                                  &exactMatchManagerPtr->lookupInfo,
                                  &entryAttr,
                                  &exactMatchManagerPtr->agingInfo,
                                  GT_TRUE/* EXACT_MATCH_MANAGER_DB_EXIST */);
    if(rc != GT_OK)
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                             CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E,
                                                             errorNumPtr);
        return GT_OK;
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerDbCheck_counter_dbInfoGet function
* @endinternal
*
* @brief This function gets the counter details from DB..
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[out] typeCountersArr      - Counters bases on entry type
* @param[out] bankCountersArr      - Counters bases on bank number
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbCheck_counter_dbInfoGet
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC      *exactMatchManagerPtr,
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_TYPE_COUNTERS_STC   typeCountersArr[CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E][CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E],
    OUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_BANK_INFO_STC             bankCountersArr[],
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT          resultArray[],
    OUT GT_U32                                                      *errorNumPtr
)
{
    GT_STATUS                                                   rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC       *dbEntryPtr;
    GT_U32                                                      i,step;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                            counterClientType;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                          counterKeySize;

    rc = prvCpssDxChExactMatchManagerDbApplicationIteratorEntryGetNext(exactMatchManagerPtr,
                                                                        GT_TRUE,
                                                                        &exactMatchManagerPtr->lastGetNextInfo,
                                                                        &dbEntryPtr);
    while(rc == GT_OK)
    {
         counterClientType = exactMatchManagerPtr->lookupInfo.lookupsArray[dbEntryPtr->hwExactMatchLookupNum].lookupClient;
        /* sanity check */
        switch (dbEntryPtr->hwExactMatchEntryType)
        {
        case 0:/* CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E */
            if (counterClientType!=CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
            {
                PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                 CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E,
                                                                 errorNumPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "TTI mismatch actionType[%d] and clientType[%d] \n",dbEntryPtr->hwExactMatchEntryType,counterClientType);
            }
            break;
        case 1:/* CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E */
            if ((counterClientType!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E)&&
                (counterClientType!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_1_E)&&
                (counterClientType!=CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_2_E))
            {
                PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                 CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E,
                                                 errorNumPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "IPCL mismatch actionType[%d] and clientType[%d] \n",dbEntryPtr->hwExactMatchEntryType,counterClientType);
            }
            break;
        case 2:/* CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E */
            if (counterClientType!=CPSS_DXCH_EXACT_MATCH_CLIENT_EPCL_E)
            {
                PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                 CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E,
                                                 errorNumPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "EPCL mismatch actionType[%d] and clientType[%d] \n",dbEntryPtr->hwExactMatchEntryType,counterClientType);
            }
            break;
        default:
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                 CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E,
                                                 errorNumPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE,"error in prvCpssDxChExactMatchManagerDbCheck_counter_dbInfoGet."
                                           " illegal dbEntryPtr->hwExactMatchEntryType[%d]",dbEntryPtr->hwExactMatchEntryType);
            break;
        }

        switch (dbEntryPtr->hwExactMatchKeySize)
        {
            case 0:
                counterKeySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
                step=1;
                break;
            case 1:
                counterKeySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E;
                step=2;
                break;
            case 2:
                counterKeySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E;
                step=3;
                break;
            case 3:
                counterKeySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E;
                step=4;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE,"error in prvCpssDxChExactMatchManagerDbCheck_counter_dbInfoGet."
                                                           " illegal dbEntryPtr->hwExactMatchKeySize[%d]",dbEntryPtr->hwExactMatchKeySize);
        }

        typeCountersArr[counterClientType][counterKeySize].entryTypePopulation++;

        /* each entry hold diferrent number of indexes and
           are spread over different number of banks
            5  Bytes key size = 1 index
            19 Bytes key size = 2 index
            33 Bytes key size = 3 index
            47 Bytes key size = 4 index */
        for (i=0;i<step;i++)
        {
            /* Bank counter update */
            bankCountersArr[((dbEntryPtr->hwIndex+i) % exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes)].bankPopulation++;
        }

        rc = prvCpssDxChExactMatchManagerDbApplicationIteratorEntryGetNext(exactMatchManagerPtr,
                                                                        GT_FALSE,
                                                                        &exactMatchManagerPtr->lastGetNextInfo,
                                                                        &dbEntryPtr);
    }
    if(rc != GT_NO_MORE)
    {
        return rc;
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerDbCheck_counters function
* @endinternal
*
* @brief The function verifies the Exact Match manager counters.
*
* @param[in] exactMatchManagerId   - the Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbCheck_counters
(
    IN  GT_U32                                              exactMatchManagerId,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT  resultArray[],
    OUT GT_U32                                              *errorNumPtr
)
{
    GT_STATUS                                                   rc = GT_OK;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                            counterClientType;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                          counterKeySize;
    GT_U32                                                      bankNum,usedEntriesIndexes=0;
    CPSS_DXCH_EXACT_MATCH_MANAGER_COUNTERS_STC                  counters;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_TYPE_COUNTERS_STC   typeCountersArray[CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E][CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E];
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_BANK_INFO_STC             bankCountersArray[CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_NUM_BANKS_CNS];
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC      *exactMatchManagerPtr;
    PRV_CPSS_DXCH_CUCKOO_DB_ST_BANK_STC                         *cuckooDbBankStPtr;
    GT_U32                                                      *cuckooDbSizeStPtr;
    PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_ENT                           i;

    cpssOsMemSet(typeCountersArray, 0, sizeof(typeCountersArray));
    cpssOsMemSet(bankCountersArray, 0, sizeof(bankCountersArray));

    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);
    prvCpssDxChExactMatchManagerDbCountersGet(exactMatchManagerId, &counters);
    rc = prvCpssDxChExactMatchManagerDbCheck_counter_dbInfoGet(exactMatchManagerPtr, typeCountersArray, bankCountersArray,
                                                               resultArray,errorNumPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Verify actionType+keySize counters */
    counterClientType=CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
    for(;counterClientType < CPSS_DXCH_EXACT_MATCH_CLIENT_LAST_E; counterClientType++)
    {
        counterKeySize=CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
        for (;counterKeySize < CPSS_DXCH_EXACT_MATCH_KEY_SIZE_LAST_E; counterKeySize++)
        {
            if (typeCountersArray[counterClientType][counterKeySize].entryTypePopulation !=
                exactMatchManagerPtr->typeCountersArr[counterClientType][counterKeySize].entryTypePopulation)
            {
                PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E,
                                                                    errorNumPtr);
                CPSS_LOG_ERROR_MAC("mismatch for counterActionType[%d]+counterKeySize[%d] : entryTypePopulation[%d] != db.entryTypePopulation[%d]",
                    counterClientType,counterKeySize,
                    typeCountersArray[counterClientType][counterKeySize].entryTypePopulation,
                    exactMatchManagerPtr->typeCountersArr[counterClientType][counterKeySize].entryTypePopulation);

                break;
            }
        }
    }

    /* Verify Bank counters */
    for(bankNum = 0; bankNum<exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes; bankNum++)
    {
        if(bankCountersArray[bankNum].bankPopulation != exactMatchManagerPtr->bankInfoArr[bankNum].bankPopulation)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                 CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E,
                                                                 errorNumPtr);
            CPSS_LOG_ERROR_MAC("mismatch for bankNum[%d] : bankPopulation[%d] != db.bankPopulation[%d]",
                bankNum,
                bankCountersArray[bankNum].bankPopulation,
                exactMatchManagerPtr->bankInfoArr[bankNum].bankPopulation);
            break;
        }
    }

    /* verify Counters for cuckoo DB */
    rc = prvCpssDxChCuckooDbStatisticsPtrGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,&cuckooDbBankStPtr,&cuckooDbSizeStPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (i=0; i<=PRV_CPSS_DXCH_CUCKOO_KEY_SIZE_47B_E; i++)
        usedEntriesIndexes += cuckooDbSizeStPtr[i];

    if (counters.usedEntriesIndexes != usedEntriesIndexes)
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                             CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_BANK_COUNTERS_MISMATCH_E,
                                                             errorNumPtr);
        CPSS_LOG_ERROR_MAC("mismatch for usedEntriesIndexes : counters.usedEntriesIndexes[%d] != cuckooDB.usedEntriesIndexes[%d]",
            counters.usedEntriesIndexes,
            usedEntriesIndexes);
        /* we already got CUCKOO_BANK_COUNTERS_MISMATCH no need to continue to next check*/
        return GT_OK;
    }

    for(bankNum = 0; bankNum<exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes; bankNum++)
    {
        if(counters.bankCounters[bankNum] != cuckooDbBankStPtr[bankNum].count)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                 CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_BANK_COUNTERS_MISMATCH_E,
                                                                 errorNumPtr);
            CPSS_LOG_ERROR_MAC("mismatch for bankNum[%d] : bankPopulation[%d] != db.bankPopulation[%d]",
                bankNum,
                counters.bankCounters[bankNum],
                exactMatchManagerPtr->bankInfoArr[bankNum].bankPopulation);
            break;
        }
    }

    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerDbCheck_dbFreeList function
* @endinternal
*
* @brief The function verifies the Exact Match manager free list in the DB.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbCheck_dbFreeList
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[],
    OUT GT_U32                                                  *errorNumPtr
)
{
    GT_U32                                              dbIndex;
    GT_U32                                              freeCnt = 0;

    /* Check head of free list, Head is expect to be invalid in case of Exact Match is complete FULL */
    if((!exactMatchManagerPtr->headOfFreeList.isValid_entryPointer) &&
            (exactMatchManagerPtr->capacityInfo.maxTotalEntries != exactMatchManagerPtr->totalPopulation))
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                            CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E,
                                                            errorNumPtr);
    }

    /* Traverse till end of free list (Head is verified, start with next)*/
    for(dbIndex = 0; dbIndex < exactMatchManagerPtr->capacityInfo.maxTotalEntries; dbIndex++)
    {
        /* NOTE - nextEntryPointer is used for pointing to the next used entry
         *        Traversing the index array, using "isUsedEntry" to get the free count */
        if(!exactMatchManagerPtr->entryPoolPtr[dbIndex].isUsedEntry)
        {
            freeCnt += 1;
        }
    }

    /* Match SW counters with Traverse counter */
    if(freeCnt != (exactMatchManagerPtr->capacityInfo.maxTotalEntries - exactMatchManagerPtr->totalPopulation))
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                            CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E,
                                                            errorNumPtr);
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerDbCheck_dbUsedList function
* @endinternal
*
* @brief The function verifies the Exact Match manager used list in the DB.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbCheck_dbUsedList
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[],
    OUT GT_U32                                                  *errorNumPtr
)
{
    GT_U32                                                      dbIndex;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INDEX_STC             *dbIndexPtr;
    GT_U32                                                      usedCnt = 0;
    GT_BOOL                                                     getNextErrorDetected = GT_FALSE, getPrevErrorDetected = GT_FALSE;

    /* Check head of used list:(Below Expected)
     *      entryPointer        == 1
     *      prevEntryPointer    == 0
     */
    if(!exactMatchManagerPtr->headOfUsedList.isValid_entryPointer)
    {
        /* Head of list not valid, skip the list check
         * Scenario - Exact Match Manager created with at least one device but no entry populated */
        return GT_OK;
    }
    else
    {
        /* Initialize the iterator with HEAD node, updated counter by 1*/
        dbIndex     = exactMatchManagerPtr->headOfUsedList.entryPointer;
        dbEntryPtr  = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        if(!dbEntryPtr->isUsedEntry)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E,
                                                                errorNumPtr);
            /* As used list Head is corrupted, should not check for other error */
            return GT_OK;
        }
        /* Head node should not have any previous valid node */
        if(dbEntryPtr->isValid_prevEntryPointer)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E,
                                                                errorNumPtr);
            /* As used list Head is corrupted, should not check for other error */
            return GT_OK;
        }
        usedCnt += 1;
    }

    /* Traverse till end of Used list (Head is verified, start with next)*/
    while(dbEntryPtr->isValid_nextEntryPointer)
    {
        /* jump to next entry */
        /*dbIndex = dbEntryPtr->nextEntryPointer;*/
        EM_GET_nextEntryPointer_MAC(dbEntryPtr, dbIndex);

        dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        dbIndexPtr = &exactMatchManagerPtr->indexArr[dbEntryPtr->hwIndex];

        /* Check elements used list:(Below Expected)
         *      isValid_entryPointer        == 1
         *      isValid_prevEntryPointer    == 1
         */
        if(!dbIndexPtr->isValid_entryPointer && !getNextErrorDetected)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E,
                                                                errorNumPtr);
            if(getPrevErrorDetected)
            {
                break;
            }
            getNextErrorDetected = GT_TRUE;
        }
        if(!dbEntryPtr->isValid_prevEntryPointer && !getPrevErrorDetected)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E,
                                                                errorNumPtr);
            if(getNextErrorDetected)
            {
                break;
            }
            getPrevErrorDetected = GT_TRUE;
        }
        usedCnt += 1;
    }

    /* Match SW counters with Traverse counter */
    if(usedCnt != exactMatchManagerPtr->totalPopulation)
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                            CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E,
                                                            errorNumPtr);
    }

    if(exactMatchManagerPtr->lastGetNextInfo.isValid_entryPointer)
    {
        dbIndex    = exactMatchManagerPtr->lastGetNextInfo.entryPointer;
        dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        if(!dbEntryPtr->isUsedEntry)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E,
                                                                errorNumPtr);
        }
    }

    if(exactMatchManagerPtr->lastDeleteInfo.isValid_entryPointer)
    {
        dbIndex    = exactMatchManagerPtr->lastDeleteInfo.entryPointer;
        dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
        if(!dbEntryPtr->isUsedEntry)
        {
            PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E,
                                                                errorNumPtr);
        }
    }
    return GT_OK;
}

/*
* @internal prvCpssDxChExactMatchManagerDbCheck_indexPointer function
* @endinternal
*
* @brief The function verifies the Exact Match manager index pointers.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbCheck_indexPointer
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[],
    OUT GT_U32                                                  *errorNumPtr
)
{

    GT_STATUS       rc = GT_OK;
    GT_U32          hwIndex;
    GT_U32          dbIndex;
    GT_BOOL         indexPointInvalidEn = GT_FALSE, indexInvalidEn = GT_FALSE;

    for(hwIndex = 0 ; hwIndex < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; hwIndex++)
    {
        dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;

        /* protect access to array out of range */
        EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex,exactMatchManagerPtr->capacityInfo.maxTotalEntries);

        /* Valid Index should point to not NULL entry,
         * Invalid Index should point to NULL Entry
         */
        if(exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            /* Valid entry but isUsedEntry flag not set. (Add to result array  once) */
            if(!exactMatchManagerPtr->entryPoolPtr[dbIndex].isUsedEntry && !indexPointInvalidEn)
            {
                PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E,
                                                                    errorNumPtr);
                if(indexInvalidEn)
                {
                    break;
                }
                indexPointInvalidEn = GT_TRUE;
            }
            /* Valid entry but hwIndex is not matching. (Add to result array  once) */
            if((exactMatchManagerPtr->entryPoolPtr[dbIndex].hwIndex != hwIndex) && !indexInvalidEn)
            {
                PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E,
                                                                    errorNumPtr);
                CPSS_LOG_ERROR_MAC("mismatch for dbIndex[%d] : exactMatchManagerPtr->entryPoolPtr[dbIndex].hwIndex[%d] != hwIndex[%d]",
                                    dbIndex,
                                    exactMatchManagerPtr->entryPoolPtr[dbIndex].hwIndex,
                                    hwIndex);
                if(indexPointInvalidEn)
                {
                    break;
                }
                indexInvalidEn = GT_TRUE;
            }
        }
        /* else - In case of invalid entry dbIndex will not be valid to check */
    }
    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerDbCheck_agingBin function
* @endinternal
*
* @brief The function verifies the Exact Match manager aging bin.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the ExactMatch manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbCheck_agingBin
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[],
    OUT GT_U32                                                  *errorNumPtr
)
{
    GT_STATUS                                               rc = GT_OK;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC   *dbEntryPtr;
    GT_U32                                                  curAgeBinId;
    GT_U32                                                  curAgeBinCnt = 0;
    GT_U32                                                  totalAgeBinCnt = 0;
    GT_BOOL                                                 scanPtrInvalid = GT_FALSE;
    GT_BOOL                                                 entryNumMismatch = GT_FALSE;
    GT_BOOL                                                 ageBinIndexInvalid = GT_FALSE;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC       lastGetNextInfo;

    for(curAgeBinId = 0; curAgeBinId<exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated; curAgeBinId++)
    {
        rc = prvCpssDxChExactMatchManagerDbAgeBinEntryGetNext(exactMatchManagerPtr, GT_TRUE, curAgeBinId, &lastGetNextInfo, &dbEntryPtr);
        while(rc == GT_OK)
        {
            curAgeBinCnt += (dbEntryPtr->hwExactMatchKeySize+1);
            rc = prvCpssDxChExactMatchManagerDbAgeBinEntryGetNext(exactMatchManagerPtr, GT_FALSE, curAgeBinId, &lastGetNextInfo, &dbEntryPtr);
        }

        /* Age-bin Get next failed */
        if(rc != GT_NO_MORE)
        {
            if(dbEntryPtr->ageBinIndex != curAgeBinId)
            {
                ageBinIndexInvalid = GT_TRUE;
            }
            scanPtrInvalid = GT_TRUE;
        }

        /* Validate age-bin's entry count */
        if(curAgeBinCnt != exactMatchManagerPtr->agingBinInfo.ageBinListPtr[curAgeBinId].totalUsedEntriesCnt)
        {
            entryNumMismatch = GT_TRUE;
        }
        totalAgeBinCnt += curAgeBinCnt;
        curAgeBinCnt = 0;
        if(ageBinIndexInvalid && scanPtrInvalid && entryNumMismatch)
        {
            break;
        }
    }

    /* Set the error list */
    if(entryNumMismatch)
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                            CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E,
                                                            errorNumPtr);
    }
    if(totalAgeBinCnt != exactMatchManagerPtr->totalPopulation)
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                            CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E,
                                                            errorNumPtr);
        CPSS_LOG_ERROR_MAC("mismatch for totalAgeBinCnt[%d] != totalPopulation[%d]",
            totalAgeBinCnt,
            exactMatchManagerPtr->totalPopulation);
    }
    if(ageBinIndexInvalid)
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                            CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E,
                                                            errorNumPtr);
    }
    if(scanPtrInvalid)
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                            CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E,
                                                            errorNumPtr);
    }
    return GT_OK;
}
/*
* @internal prvCpssDxChExactMatchManagerDbCheck_ageBinUsageMatrix function
* @endinternal
*
* @brief The function verifies the Exact Match manager aging bin usage matrix.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbCheck_ageBinUsageMatrix
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC   *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT       resultArray[],
    OUT GT_U32                                                  *errorNumPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  curAgeBinId;
    GT_BOOL                                 l2BitEnabled = GT_TRUE;
    GT_BOOL                                 l1ErrorEn = GT_FALSE, l2ErrorEn = GT_FALSE;

    for(curAgeBinId = 0; curAgeBinId < exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated; curAgeBinId++)
    {
        /* Level-1 Verification */
        if(exactMatchManagerPtr->agingBinInfo.ageBinListPtr[curAgeBinId].totalUsedEntriesCnt != exactMatchManagerPtr->capacityInfo.maxEntriesPerAgingScan)
        {
            /* Level-1 usage bit should not be set */
            if((EM_IS_BMP_SET_MAC(exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr, curAgeBinId)) && !l1ErrorEn)
            {
                PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E,
                                                                    errorNumPtr);
                if(l2ErrorEn)
                {
                    break;
                }
                l1ErrorEn = GT_TRUE;
            }
            l2BitEnabled = GT_FALSE;
        }
        else
        {
            /* Level-1 usage bit should be set */
            if((!EM_IS_BMP_SET_MAC(exactMatchManagerPtr->agingBinInfo.level1UsageBitmapPtr, curAgeBinId)) && !l1ErrorEn)
            {
                PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E,
                                                                    errorNumPtr);
                if(l2ErrorEn)
                {
                    break;
                }
                l1ErrorEn = GT_TRUE;
            }
        }

        /* Level-2 Verification */
        if((curAgeBinId != 0) && (((curAgeBinId+1) % 32) == 0))
        {
            if(l2BitEnabled)
            {
                if((!EM_IS_BMP_SET_MAC(&exactMatchManagerPtr->agingBinInfo.level2UsageBitmap, curAgeBinId>>5)) && !l2ErrorEn)
                {
                    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                        CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E,
                                                                        errorNumPtr);
                    if(l1ErrorEn)
                    {
                        break;
                    }
                    l2ErrorEn = GT_TRUE;
                }
            }
            else
            {
                if((EM_IS_BMP_SET_MAC(&exactMatchManagerPtr->agingBinInfo.level2UsageBitmap, curAgeBinId>>5)) && !l2ErrorEn)
                {
                    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                                        CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E,
                                                                        errorNumPtr);
                    if(l1ErrorEn)
                    {
                        break;
                    }
                    l2ErrorEn = GT_TRUE;
                }
            }
            /* Reset L2 Bit, to check next 32 age-bin */
            l2BitEnabled = GT_TRUE;
        }
    }
    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerDbCheck_hwList function
* @endinternal
*
* @brief The function verifies the Exact Match manager used/Free list in HW.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - there is error in DB management.
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbCheck_hwList
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[],
    OUT GT_U32                                                  *errorNumPtr
)
{
    GT_STATUS                                                   rc = GT_OK;
    GT_U32                                                      hwIndex,step,i;
    GT_U32                                                      dbIndex;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT          result;

    /* check all exact match entries defined in the DB */
    for(hwIndex = 0 ; hwIndex < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; /*hwIndex updated according to entry keySize */)
    {
        result = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E;
        /* Validate used list content */
        if(exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;
            /* protect access to array out of range */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

            if (exactMatchManagerPtr->entryPoolPtr[dbIndex].isFirst==GT_TRUE)
            {
                 rc = prvCpssDxChExactMatchManagerHwEntryMatchWithSwEntry_allDevice(exactMatchManagerPtr,
                                                                                   hwIndex,
                                                                                   &exactMatchManagerPtr->entryPoolPtr[dbIndex],
                                                                                   &result);
                if(rc != GT_OK)
                {
                    return rc;
                }
                if(result == CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E)
                {
                    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                        CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E,
                                                        errorNumPtr);
                    break;
                }

                /* jump to the next hwIndex to check */
                step = exactMatchManagerPtr->entryPoolPtr[dbIndex].hwExactMatchKeySize + 1;
                hwIndex = hwIndex+step;
            }
            else
            {
                hwIndex++;
            }
        }
        else
        {
            /* else - In case of invalid entry dbIndex will not be valid to check */
            hwIndex++;
        }
    }
    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }

    /* check lookup parameters set during manager create */
    result=0;/* reset result */
    rc = prvCpssDxChExactMatchManagerHwLookupConfigMatchWithSwLookupConfig_allDevice(exactMatchManagerPtr,&result);
    if ((result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E)||
        (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E)||
        (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_KEY_PARAMS_INVALID_E)||
        (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_ENABLE_INVALID_E)||
        (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_INVALID_E))
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                            result,
                                                            errorNumPtr);
    }
    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }

    /* check expanded array set during manager create */

    /* first validate the expanded array kept in the DB */
    for (i=0;i<CPSS_DXCH_EXACT_MATCH_MANAGER_EXPANDED_ACTION_ENTRIES_NUM_CNS;i++)
    {
        rc = validateExactMatchExpandedActionParams(i, &(exactMatchManagerPtr->exactMatchExpanderArray[i].exactMatchExpandedEntry));
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in validation of exactMatchManagerPtr->exactMatchExpanderArray[%d]",i);
        }
    }

    rc = prvCpssDxChExactMatchManagerHwExpandedArrayMatchWithSwExpandedArray_allDevice(exactMatchManagerPtr,&result);
    if (result==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_EXPANDER_CONFIG_INVALID_E)
    {
        PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_EXPANDER_CONFIG_INVALID_E,
                                                errorNumPtr);
    }

    if(rc == GT_NO_MORE)
    {
        rc = GT_OK;
    }

    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerDbCheck_cuckooList function
* @endinternal
*
* @brief The function verifies the Cuckoo DB compare to manager used List.
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - there is error in DB management.
*
*/
static GT_STATUS prvCpssDxChExactMatchManagerDbCheck_cuckooList
(
    IN  PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT      resultArray[],
    OUT GT_U32                                                  *errorNumPtr
)
{
    GT_STATUS                                                   rc = GT_OK;
    GT_U32                                                      hwIndex,step,ii;
    GT_U32                                                      dbIndex;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT          result;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC       *entryPtr;
    GT_U32                                                      sizeInBytes,bank,line,exactMatchManagerId;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC                           cuckooEntry;

    /* check all exact match entries defined in the DB */
    for(hwIndex = 0 ; hwIndex < exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHwIndexes; /*hwIndex updated according to entry keySize */)
    {
        result = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E;
        cpssOsMemSet(&cuckooEntry,0,sizeof(PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC));

        /* Validate used list content */
        if(exactMatchManagerPtr->indexArr[hwIndex].isValid_entryPointer)
        {
            dbIndex = exactMatchManagerPtr->indexArr[hwIndex].entryPointer;

            /* protect access to array out of range */
            EM_CHECK_X_NOT_EQUAL_OR_MORE_THAN_Y_MAC(dbIndex, exactMatchManagerPtr->capacityInfo.maxTotalEntries);

            if (exactMatchManagerPtr->entryPoolPtr[dbIndex].isFirst==GT_TRUE)
            {

                entryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];

                /* convert Exact Match key enum to size in bytes */
                EM_CONV_KEY_ENUM_TO_SIZE_IN_BYTES_MAC(entryPtr->hwExactMatchKeySize,sizeInBytes);

                /* calc line from hwIndex */
                EM_LINE_FROM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,line);

                /* calc bank from hwIndex */
                EM_BANK_FROM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,hwIndex,bank);

                /* get Excat Match manager id */
                EM_MANAGER_ID_GET_MAC(exactMatchManagerPtr,exactMatchManagerId);

                /* get entry from cuckoo DB */
                rc = prvCpssDxChCuckooDbEntryGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,bank,line,&cuckooEntry);
                if (rc != GT_OK)
                {
                    return rc;
                }

                /* compare the entries */
                if (cuckooEntry.isFree || (cuckooEntry.sizeInBytes != sizeInBytes) ||
                    (cuckooEntry.isFirst != GT_TRUE) || (cuckooEntry.isLock) ||
                    (cpssOsMemCmp(entryPtr->pattern,cuckooEntry.data,sizeInBytes) != 0))
                {
                    result = CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_ENTRY_INVALID_E;
                }

                if(result == CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_ENTRY_INVALID_E)
                {
                    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_ERROR_ASSIGN(resultArray,
                                                        CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_ENTRY_INVALID_E,
                                                        errorNumPtr);
                    break;
                }

                /* jump to the next hwIndex to check */
                step = exactMatchManagerPtr->entryPoolPtr[dbIndex].hwExactMatchKeySize + 1;
                hwIndex = hwIndex+step;
            }
            else
            {
                /* else - In case of invalid entry dbIndex will not be valid to check */
                hwIndex++;
            }
        }
        else
        {
            /* else - In case of invalid entry dbIndex will not be valid to check */
            hwIndex++;
        }
    }

    return rc;
}

/*
* @internal prvCpssDxChExactMatchManagerDbCheck function
* @endinternal
*
* @brief The function verifies the Exact Match manager DB.
*
* @param[in]  exactMatchManagerId  - The Exact Match Manager id.
*                                    (APPLICABLE RANGES : 0..31)
* @param[in]  checksPtr            - (Pointer to) DB check attributes
* @param[out] resultArray          - (pointer to) Test result array.
* @param[out] errorNumPtr          - (pointer to) Number of error types filled in resultArray
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDbCheck
(
    IN  GT_U32                                              exactMatchManagerId,
    IN  CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC         *checksPtr,
    OUT CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT  resultArray[],
    OUT GT_U32                                              *errorNumPtr
)
{
    GT_STATUS                                               rc;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    *errorNumPtr = 0;
    if(checksPtr->globalCheckEnable)                /* globalCheckEnable */
    {
        rc = prvCpssDxChExactMatchManagerDbCheck_global(exactMatchManagerId, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->countersCheckEnable)              /* countersCheckEnable */
    {
        rc = prvCpssDxChExactMatchManagerDbCheck_counters(exactMatchManagerId, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->dbFreeListCheckEnable)            /* dbFreeListCheckEnable */
    {
        rc = prvCpssDxChExactMatchManagerDbCheck_dbFreeList(exactMatchManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->dbUsedListCheckEnable)            /* dbUsedListCheckEnable */
    {
        rc = prvCpssDxChExactMatchManagerDbCheck_dbUsedList(exactMatchManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->dbIndexPointerCheckEnable)        /* dbIndexPointerCheckEnable */
    {
        rc = prvCpssDxChExactMatchManagerDbCheck_indexPointer(exactMatchManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->dbAgingBinCheckEnable)            /* dbAgingBinCheckEnable */
    {
        rc = prvCpssDxChExactMatchManagerDbCheck_agingBin(exactMatchManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->dbAgingBinUsageMatrixCheckEnable) /* dbAgingBinUsageMatrixCheckEnable */
    {
        rc = prvCpssDxChExactMatchManagerDbCheck_ageBinUsageMatrix(exactMatchManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    if(checksPtr->hwUsedListCheckEnable) /* hwUsedListCheckEnable */
    {
        rc = prvCpssDxChExactMatchManagerDbCheck_hwList(exactMatchManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

    }
     if(checksPtr->cuckooDbCheckEnable) /* cuckooDbCheckEnable */
    {
        /* compare cuckoo DB against manager DB */
        rc = prvCpssDxChExactMatchManagerDbCheck_cuckooList(exactMatchManagerPtr, resultArray, errorNumPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}
/*
* @internal prvCpssDxChExactMatchManagerDebugSelfTest function
* @endinternal
*
* @brief The function injects/corrects error in Exact Match manager DB, as mentioned in expResultArray
*
* @param[in] exactMatchManagerId  - The Exact Match Manager id.
*                                   (APPLICABLE RANGES : 0..31)
* @param[in] expResultArray       - (pointer to) Test error types array.
* @param[in] errorNumPtr          - (pointer to) Number of error types filled in expResultArray
* @param[in] errorEnable          - Status value indicates error/correct state
*                                   GT_TRUE  - Error need to injected according to expResultArray and expErrorNum
*                                   GT_FALSE - Error need to be corrected according to expResultArray and expErrorNum
*
* @retval GT_OK                   - on success
* @retval GT_FAIL                 - on error
*
*/
GT_STATUS prvCpssDxChExactMatchManagerDebugSelfTest
(
    IN GT_U32                                               exactMatchManagerId,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_RESULT_ENT   expResultArray[CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E],
    IN GT_U32                                               expErrorNum,
    IN GT_BOOL                                              errorEnable
)
{
    GT_STATUS                                                   rc = GT_OK;
    GT_U32                                                      ii, tmpCnt;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC     *exactMatchManagerPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DEBUG_RESTORE_INFO_UNT   *restoreInfoPtr;
    GT_U32                                                      bankId = 2;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                            counterClientType =CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT                          counterKeySize = CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E;
    GT_U32                                                      dbIndex;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_FORMAT_STC      *dbEntryPtr;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_NODE_PTR_STC          *dbNodePtr;  /* For delete scan */
    /* used for cuckoo check */
    GT_U32                              line, bank;
    PRV_CPSS_DXCH_CUCKOO_DB_ENTRY_STC   cuckooEntry;

    EM_MANAGER_ID_CHECK(exactMatchManagerId);
    exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

    for(ii=0; ii<expErrorNum; ii++)
    {
        /* Validate if error is already injected/corrected */
        if((errorEnable && EM_IS_BMP_SET_MAC(&exactMatchManagerPtr->selfTestBitmap, expResultArray[ii])) ||
           (!errorEnable && !EM_IS_BMP_SET_MAC(&exactMatchManagerPtr->selfTestBitmap, expResultArray[ii])))
        {
            continue;
        }
        restoreInfoPtr = &exactMatchManagerPtr->selfTestRestoreInfo[expResultArray[ii]];
        switch(expResultArray[ii])
        {
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->counter = exactMatchManagerPtr->capacityInfo.maxTotalEntries;
                    exactMatchManagerPtr->capacityInfo.maxTotalEntries +=1;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    exactMatchManagerPtr->capacityInfo.maxTotalEntries = restoreInfoPtr->counter;
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_GLOBAL_PARAM_HW_INVALID_E);
                }
                break;

            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->counter = exactMatchManagerPtr->typeCountersArr[counterClientType][counterKeySize].entryTypePopulation;
                    exactMatchManagerPtr->typeCountersArr[counterClientType][counterKeySize].entryTypePopulation +=1;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    exactMatchManagerPtr->typeCountersArr[counterClientType][counterKeySize].entryTypePopulation = restoreInfoPtr->counter;
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_TYPE_COUNTERS_INVALID_E);
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E:
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_BANK_COUNTERS_MISMATCH_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->counter = exactMatchManagerPtr->bankInfoArr[bankId].bankPopulation;
                    exactMatchManagerPtr->bankInfoArr[bankId].bankPopulation += 1;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E);
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_BANK_COUNTERS_MISMATCH_E);
                }
                else
                {/* Correct last injected error*/
                    exactMatchManagerPtr->bankInfoArr[bankId].bankPopulation = restoreInfoPtr->counter;
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_BANK_COUNTERS_INVALID_E);
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_BANK_COUNTERS_MISMATCH_E);
                }
                break;

            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->entryStatus = BIT2BOOL_MAC(exactMatchManagerPtr->headOfFreeList.isValid_entryPointer);
                    exactMatchManagerPtr->headOfFreeList.isValid_entryPointer = 0;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    exactMatchManagerPtr->headOfFreeList.isValid_entryPointer = BOOL2BIT_MAC(restoreInfoPtr->entryStatus);
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_1ST_PTR_INVALID_E);
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E:
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E:
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->counter = exactMatchManagerPtr->totalPopulation;
                    exactMatchManagerPtr->totalPopulation += 1;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E);
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E);
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E);
                }
                else
                {/* Correct last injected error*/
                    exactMatchManagerPtr->totalPopulation = restoreInfoPtr->counter;
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_FREE_LIST_ENTRIES_NUM_MISMATCH_E);
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRIES_NUM_MISMATCH_E);
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_TOTAL_NUM_MISMATCH_E);
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E:
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E:
                dbIndex     = exactMatchManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
                EM_GET_nextEntryPointer_MAC(dbEntryPtr, dbIndex);              /* Invalidate the entry next to head */
                dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
                if(errorEnable)
                {/* Inject Error */
                    if(!exactMatchManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer)
                    {
                        rc = GT_FAIL;
                        break;
                    }
                    restoreInfoPtr->entryStatus = BIT2BOOL_MAC(exactMatchManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer);
                    exactMatchManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer = 0;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E);
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    exactMatchManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer = BOOL2BIT_MAC(restoreInfoPtr->entryStatus);
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_GETNEXT_PTR_INVALID_E);
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_SCAN_PTR_INVALID_E);
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E:
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E:
                if(expResultArray[ii] == CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E)
                {
                    dbNodePtr = &exactMatchManagerPtr->lastDeleteInfo;
                }
                else
                {
                    dbNodePtr = &exactMatchManagerPtr->lastGetNextInfo;
                }

                if(errorEnable)
                {/* Inject Error */
                    /* In case of scan not used at all - assign last scan as head of free list */
                    if(!dbNodePtr->isValid_entryPointer)
                    {
                        dbNodePtr->entryPointer          = exactMatchManagerPtr->headOfFreeList.entryPointer;
                    }
                    restoreInfoPtr->nodeInfo.entryPointer = dbNodePtr->entryPointer;
                    dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[dbNodePtr->entryPointer];

                    restoreInfoPtr->nodeInfo.isValid_entryPointer   = BIT2BOOL_MAC(dbNodePtr->isValid_entryPointer);
                    dbNodePtr->isValid_entryPointer                 = 1;
                    restoreInfoPtr->nodeInfo.isUsedEntry            = BIT2BOOL_MAC(dbEntryPtr->isUsedEntry);
                    dbEntryPtr->isUsedEntry                         = 0;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E);
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    dbEntryPtr = &exactMatchManagerPtr->entryPoolPtr[restoreInfoPtr->nodeInfo.entryPointer];
                    dbEntryPtr->isUsedEntry                 = BOOL2BIT_MAC(restoreInfoPtr->nodeInfo.isUsedEntry);
                    dbNodePtr->isValid_entryPointer         = BOOL2BIT_MAC(restoreInfoPtr->nodeInfo.isValid_entryPointer);
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_DELETE_SCAN_PTR_INVALID_E);
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_LAST_PTR_INVALID_E);
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E:
                dbIndex     = exactMatchManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->entryStatus = BIT2BOOL_MAC(dbEntryPtr->isValid_prevEntryPointer);
                    dbEntryPtr->isValid_prevEntryPointer = 1;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    dbEntryPtr->isValid_prevEntryPointer = BOOL2BIT_MAC(restoreInfoPtr->entryStatus);
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_ENTRY_PREV_PTR_INVALID_E);
                }
                break;

            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E:
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E:
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E:
                dbIndex     = exactMatchManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
                if(errorEnable)
                {/* Inject Error */
                    if(!exactMatchManagerPtr->indexArr[dbEntryPtr->hwIndex].isValid_entryPointer)
                    {
                        rc = GT_FAIL;
                        break;
                    }
                    if(expResultArray[ii] == CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E)
                    {
                        restoreInfoPtr->hwIndex = exactMatchManagerPtr->entryPoolPtr[dbIndex].hwIndex;
                        exactMatchManagerPtr->entryPoolPtr[dbIndex].hwIndex +=1;
                        EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E);
                    }
                    else
                    {
                        restoreInfoPtr->entryStatus = BIT2BOOL_MAC(exactMatchManagerPtr->entryPoolPtr[dbIndex].isUsedEntry);
                        exactMatchManagerPtr->entryPoolPtr[dbIndex].isUsedEntry = 0;
                        EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E);
                        EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E);
                    }
                }
                else
                {/* Correct last injected error*/
                    if(expResultArray[ii] == CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E)
                    {
                        exactMatchManagerPtr->entryPoolPtr[dbIndex].hwIndex = restoreInfoPtr->hwIndex;
                        EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ENTRY_INDEX_INVALID_E);
                    }
                    else
                    {
                        exactMatchManagerPtr->entryPoolPtr[dbIndex].isUsedEntry = BOOL2BIT_MAC(restoreInfoPtr->entryStatus);
                        EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_INDEX_POINT_INVALID_ENTRY_E);
                        EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_USED_LIST_1ST_PTR_INVALID_E);
                    }
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E:
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->counter = exactMatchManagerPtr->agingBinInfo.ageBinListPtr[0].totalUsedEntriesCnt;
                    if(exactMatchManagerPtr->agingBinInfo.ageBinListPtr[0].totalUsedEntriesCnt < exactMatchManagerPtr->capacityInfo.maxEntriesPerAgingScan)
                    {
                        exactMatchManagerPtr->agingBinInfo.ageBinListPtr[0].totalUsedEntriesCnt = exactMatchManagerPtr->capacityInfo.maxEntriesPerAgingScan;
                    }
                    else
                    {
                        exactMatchManagerPtr->agingBinInfo.ageBinListPtr[0].totalUsedEntriesCnt -=1;
                    }
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E);
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    exactMatchManagerPtr->agingBinInfo.ageBinListPtr[0].totalUsedEntriesCnt = restoreInfoPtr->counter;
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_BIN_ENTRIES_NUM_MISMATCH_E);
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L1_INVALID_E);
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E:
                dbIndex     = exactMatchManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
                for(tmpCnt = 0; exactMatchManagerPtr->capacityInfo.maxTotalEntries; tmpCnt++)
                {
                    dbEntryPtr  = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
                    EM_GET_nextEntryPointer_MAC(dbEntryPtr, dbIndex);
                    /*  Get the 3rd Element */
                    if(tmpCnt == 3)
                    {
                        break;
                    }
                }
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->ageBinIndex = dbEntryPtr->ageBinIndex;
                    dbEntryPtr->ageBinIndex = (dbEntryPtr->ageBinIndex+1)%(exactMatchManagerPtr->agingBinInfo.totalAgeBinAllocated-1);
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    dbEntryPtr->ageBinIndex = restoreInfoPtr->ageBinIndex;
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_ENTRY_BIN_INDEX_INVALID_E);
                }
                break;

            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E:
                tmpCnt = 1;
                if(errorEnable)
                {/* Inject Error */
                    if(!EM_IS_BMP_SET_MAC(&exactMatchManagerPtr->agingBinInfo.level2UsageBitmap, tmpCnt))
                    {
                        EM_BMP_SET_MAC(&exactMatchManagerPtr->agingBinInfo.level2UsageBitmap, tmpCnt);
                        restoreInfoPtr->l2BitStatus = GT_FALSE;
                    }
                    else
                    {
                        EM_BMP_CLEAR_MAC(&exactMatchManagerPtr->agingBinInfo.level2UsageBitmap, tmpCnt);
                        restoreInfoPtr->l2BitStatus = GT_TRUE;
                    }
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    if(restoreInfoPtr->l2BitStatus)
                    {
                        EM_BMP_SET_MAC(&exactMatchManagerPtr->agingBinInfo.level2UsageBitmap, tmpCnt);
                    }
                    else
                    {
                        EM_BMP_CLEAR_MAC(&exactMatchManagerPtr->agingBinInfo.level2UsageBitmap, tmpCnt);
                    }
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_AGING_USAGE_MATRIX_L2_INVALID_E);
                }
                break;

            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E:
                dbIndex     = exactMatchManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
                for(tmpCnt = 0; exactMatchManagerPtr->capacityInfo.maxTotalEntries; tmpCnt++)
                {
                    EM_GET_nextEntryPointer_MAC(dbEntryPtr, dbIndex);
                    dbEntryPtr  = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
                    /* break on First TTI entry */
                    if((dbEntryPtr->isFirst==GT_TRUE)&&
                       (dbEntryPtr->hwExactMatchEntryType == PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_TTI_E))
                    {
                        break;
                    }
                }
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->pattern_0 = dbEntryPtr->pattern[0];
                    dbEntryPtr->pattern[0]+=1;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    dbEntryPtr->pattern[0] = restoreInfoPtr->pattern_0;
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_ENTRY_INVALID_E);
                }
                break;

            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E:
                if (expErrorNum==CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS___LAST___E)
                {
                    /* in case of CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_ALL_E we need to skip
                       LOOKUP_CLIENT_INVALID since it will return GT_AD_PARAM and it will break
                       the loop on the rest of the expected fails*/
                    break;
                }
                if (errorEnable)
                {
                    if (exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupEnable==GT_TRUE)
                    {
                        restoreInfoPtr->lookupClient = exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClient;
                        exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClient += 1;
                        EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E);
                    }
                    else
                    {
                        if (exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupEnable==GT_TRUE)
                        {
                            restoreInfoPtr->lookupClient = exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClient;
                            exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClient += 1;
                            EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E);
                        }
                    }
                }
                else
                {
                    if (exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupEnable==GT_TRUE)
                    {
                        exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClient = restoreInfoPtr->lookupClient;
                        EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E);
                    }
                    else
                    {
                        if (exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupEnable==GT_TRUE)
                        {
                            exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClient = restoreInfoPtr->lookupClient;
                            EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_CLIENT_INVALID_E);
                        }
                    }
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E:
                if(errorEnable)
                {
                    if (exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupEnable==GT_TRUE)
                    {
                        if(exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClientMappingsNum>0)
                        {
                            if (exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
                            {
                                restoreInfoPtr->profileId = exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClientMappingsArray[0].ttiMappingElem.profileId;
                                exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClientMappingsArray[0].ttiMappingElem.profileId++;
                            }
                            else
                            {
                                restoreInfoPtr->profileId = exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClientMappingsArray[0].pclMappingElem.profileId;
                                exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClientMappingsArray[0].pclMappingElem.profileId++;
                            }
                            EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E);
                        }
                    }
                    else
                    {
                        if (exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupEnable==GT_TRUE)
                        {
                            if(exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClientMappingsNum>0)
                            {
                                if (exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
                                {
                                    restoreInfoPtr->profileId = exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClientMappingsArray[0].ttiMappingElem.profileId;
                                    exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClientMappingsArray[0].ttiMappingElem.profileId++;
                                }
                                else
                                {
                                    restoreInfoPtr->profileId = exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClientMappingsArray[0].pclMappingElem.profileId;
                                    exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClientMappingsArray[0].pclMappingElem.profileId++;
                                }
                                EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E);
                            }
                        }
                    }
                }
                else
                {
                    if (exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupEnable==GT_TRUE)
                    {
                        if(exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClientMappingsNum>0)
                        {
                            if (exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
                            {
                               exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClientMappingsArray[0].ttiMappingElem.profileId = restoreInfoPtr->profileId;
                            }
                            else
                            {
                               exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClientMappingsArray[0].pclMappingElem.profileId = restoreInfoPtr->profileId;
                            }
                            EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E);
                        }
                    }
                    else
                    {
                        if (exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupEnable==GT_TRUE)
                        {
                             if(exactMatchManagerPtr->lookupInfo.lookupsArray[0].lookupClientMappingsNum>0)
                             {
                                if (exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClient==CPSS_DXCH_EXACT_MATCH_CLIENT_TTI_E)
                                {
                                    exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClientMappingsArray[0].ttiMappingElem.profileId = restoreInfoPtr->profileId;
                                }
                                else
                                {
                                    exactMatchManagerPtr->lookupInfo.lookupsArray[1].lookupClientMappingsArray[0].pclMappingElem.profileId = restoreInfoPtr->profileId;
                                }
                                EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_PROFILE_ID_INVALID_E);
                             }
                        }
                    }
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_KEY_PARAMS_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->keyParams.keySize = exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].keyParams.keySize;
                    restoreInfoPtr->keyParams.keyStart = exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].keyParams.keyStart;

                    exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].keyParams.keySize++;
                    exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].keyParams.keyStart++;

                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_KEY_PARAMS_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].keyParams.keySize=restoreInfoPtr->keyParams.keySize;
                    exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].keyParams.keyStart=restoreInfoPtr->keyParams.keyStart;

                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_KEY_PARAMS_INVALID_E);
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_ENABLE_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    restoreInfoPtr->defaultActionEn = exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultActionEn;

                    exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultActionEn =
                                    (exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultActionEn==GT_TRUE)? GT_FALSE:GT_TRUE;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_ENABLE_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultActionEn = restoreInfoPtr->defaultActionEn;
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_ENABLE_INVALID_E);
                }
                break;
            case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    if (exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultActionType==CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
                    {
                        cpssOsMemCpy(&restoreInfoPtr->defaultAction.ttiAction,
                               &exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultAction.ttiAction,
                               sizeof(restoreInfoPtr->defaultAction.ttiAction));

                        exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultAction.ttiAction.redirectCommand=CPSS_DXCH_TTI_NO_REDIRECT_E;
                    }
                    else
                    {
                        cpssOsMemCpy(&restoreInfoPtr->defaultAction.pclAction,
                               &exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultAction.pclAction,
                               sizeof(restoreInfoPtr->defaultAction.pclAction));

                        exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultAction.pclAction.pktCmd=CPSS_PACKET_CMD_DROP_HARD_E;
                    }

                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    if (exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultActionType==CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
                    {
                        cpssOsMemCpy(&exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultAction.ttiAction,
                                     &restoreInfoPtr->defaultAction.ttiAction,
                                     sizeof(restoreInfoPtr->defaultAction.ttiAction));
                    }
                    else
                    {
                        cpssOsMemCpy(&exactMatchManagerPtr->lookupInfo.profileEntryParamsArray[1].defaultAction.pclAction,
                                     &restoreInfoPtr->defaultAction.pclAction,
                                     sizeof(restoreInfoPtr->defaultAction.pclAction));
                    }
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_LOOKUP_DEFAULT_ACTION_INVALID_E);
                }
                break;
        case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_EXPANDER_CONFIG_INVALID_E:
                if(errorEnable)
                {/* Inject Error */
                    cpssOsMemCpy(&restoreInfoPtr->expandedAction,
                               &exactMatchManagerPtr->exactMatchExpanderArray[3].exactMatchExpandedEntry.expandedAction,
                               sizeof(restoreInfoPtr->expandedAction));
                    exactMatchManagerPtr->exactMatchExpanderArray[3].exactMatchExpandedEntry.expandedAction.ttiAction.redirectCommand=0;
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_EXPANDER_CONFIG_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    cpssOsMemCpy(&exactMatchManagerPtr->exactMatchExpanderArray[3].exactMatchExpandedEntry.expandedAction,
                                 &restoreInfoPtr->expandedAction,
                                 sizeof(restoreInfoPtr->expandedAction));

                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_HW_EXPANDER_CONFIG_INVALID_E);
                }
                break;
        case CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_ENTRY_INVALID_E:
                dbIndex     = exactMatchManagerPtr->headOfUsedList.entryPointer;
                dbEntryPtr  = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
                for(tmpCnt = 0; exactMatchManagerPtr->capacityInfo.maxTotalEntries; tmpCnt++)
                {
                    EM_GET_nextEntryPointer_MAC(dbEntryPtr, dbIndex);
                    dbEntryPtr  = &exactMatchManagerPtr->entryPoolPtr[dbIndex];
                    /* break on First TTI entry */
                    if((dbEntryPtr->isFirst==GT_TRUE)&&
                       (dbEntryPtr->hwExactMatchEntryType == PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HW_ENTRY_TYPE_TTI_E))
                    {
                        break;
                    }
                }

                EM_LINE_FROM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,dbEntryPtr->hwIndex,line);
                EM_BANK_FROM_HW_INDEX_GET_MAC(exactMatchManagerPtr->capacityInfo.hwCapacity.numOfHashes,dbEntryPtr->hwIndex,bank);

                rc = prvCpssDxChCuckooDbEntryGet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,bank,line,&cuckooEntry);
                if (rc != GT_OK)
                {
                    rc = GT_FAIL;
                    break;
                }

                if(errorEnable)
                {/* Inject Error */

                    restoreInfoPtr->cuckooEntry_sizeInBytes = cuckooEntry.sizeInBytes;
                    cuckooEntry.sizeInBytes++;
                    rc = prvCpssDxChCuckooDbEntrySet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,bank,line,&cuckooEntry);
                    if (rc != GT_OK)
                    {
                        rc = GT_FAIL;
                        break;
                    }
                    EM_SELF_TEST_BMP_SET_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_ENTRY_INVALID_E);
                }
                else
                {/* Correct last injected error*/
                    cuckooEntry.sizeInBytes=restoreInfoPtr->cuckooEntry_sizeInBytes;
                    rc = prvCpssDxChCuckooDbEntrySet(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId,bank,line,&cuckooEntry);
                    if (rc != GT_OK)
                    {
                        rc = GT_FAIL;
                        break;
                    }
                    EM_SELF_TEST_BMP_CLR_MAC(exactMatchManagerPtr, CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_CUCKOO_ENTRY_INVALID_E);
                }
                break;
            default:
                break;
        }
    }
    return rc;
}


static GT_STATUS haCompareSwDataAndHwData
(
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC              *entryPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_HA_DB_ENTRY_STC    *exactMatchEntryPtr,
    IN GT_BOOL                                              *foundValidEntryPtr
)
{
    GT_U32 rc=GT_OK;
    GT_U32 sizeInBytes;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_SPECIFIC_FORMAT  specificFormatHw;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_SPECIFIC_FORMAT  specificFormatSw;

    cpssOsMemSet(&specificFormatHw,0,sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_SPECIFIC_FORMAT));
    cpssOsMemSet(&specificFormatSw,0,sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_SPECIFIC_FORMAT));

    /* compare key parameters */
    if (entryPtr->exactMatchEntry.key.keySize!=exactMatchEntryPtr->exactMatchEntry.key.keySize)
    {
        *foundValidEntryPtr=GT_FALSE;
        return GT_OK;
    }

    switch (entryPtr->exactMatchEntry.key.keySize)
    {
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
            sizeInBytes = 5;
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
            sizeInBytes = 19;
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
            sizeInBytes = 33;
            break;
        case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
            sizeInBytes = 47;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (cpssOsMemCmp(&entryPtr->exactMatchEntry.key.pattern,
                    &exactMatchEntryPtr->exactMatchEntry.key.pattern,
                    sizeInBytes * sizeof(GT_U8))!=0)
    {
        *foundValidEntryPtr=GT_FALSE;
        return GT_OK;
    }

    if (entryPtr->exactMatchEntry.lookupNum!=exactMatchEntryPtr->exactMatchEntry.lookupNum)
    {
        *foundValidEntryPtr=GT_FALSE;
        return GT_OK;
    }

    /* compare action parameters */
    if (entryPtr->expandedActionIndex!=exactMatchEntryPtr->expandedActionIndex)
    {
        *foundValidEntryPtr=GT_FALSE;
        return GT_OK;
    }

    if (entryPtr->exactMatchActionType!=exactMatchEntryPtr->exactMatchActionType)
    {
        *foundValidEntryPtr=GT_FALSE;
        return GT_OK;
    }
    if (entryPtr->exactMatchActionType==CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E)
    {
        rc = saveEntryFormatParamsToDb_exactMatchEntryTtiFormat(&entryPtr->exactMatchAction.ttiAction,
                                                               &specificFormatSw.prvTtiEntryFormat);
        if (rc!=GT_OK)
        {
            *foundValidEntryPtr=GT_FALSE;
            return rc;
        }
        rc = saveEntryFormatParamsToDb_exactMatchEntryTtiFormat(&exactMatchEntryPtr->exactMatchAction.ttiAction,
                                                               &specificFormatHw.prvTtiEntryFormat);
        if (rc!=GT_OK)
        {
            *foundValidEntryPtr=GT_FALSE;
            return rc;
        }

        if(cpssOsMemCmp(&specificFormatSw.prvTtiEntryFormat,
                        &specificFormatHw.prvTtiEntryFormat,
                        sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_TTI_FORMAT_STC))!=0)
        {
            *foundValidEntryPtr=GT_FALSE;
            return GT_OK;
        }
    }
    else
    {
        /* if command==forward need to reset the mirror.cpuCode. it is not relevant in this case */
        if (exactMatchEntryPtr->exactMatchAction.pclAction.pktCmd==CPSS_PACKET_CMD_FORWARD_E)
        {
            exactMatchEntryPtr->exactMatchAction.pclAction.mirror.cpuCode = CPSS_NET_UNDEFINED_CPU_CODE_E;
        }

        if (entryPtr->exactMatchActionType==CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E)
        {
            rc = saveEntryFormatParamsToDb_exactMatchEntryPclFormat(&entryPtr->exactMatchAction.pclAction,
                                                                    &specificFormatSw.prvPclEntryFormat);
            if (rc!=GT_OK)
            {
                *foundValidEntryPtr=GT_FALSE;
                return rc;
            }
            rc = saveEntryFormatParamsToDb_exactMatchEntryPclFormat(&exactMatchEntryPtr->exactMatchAction.pclAction,
                                                                   &specificFormatHw.prvPclEntryFormat);
            if (rc!=GT_OK)
            {
                *foundValidEntryPtr=GT_FALSE;
                return rc;
            }

            if(cpssOsMemCmp(&specificFormatSw.prvPclEntryFormat,
                            &specificFormatHw.prvPclEntryFormat,
                            sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_PCL_FORMAT_STC))!=0)
            {
                *foundValidEntryPtr=GT_FALSE;
                return GT_OK;
            }
        }
        else
        {
            rc = saveEntryFormatParamsToDb_exactMatchEntryEpclFormat(&entryPtr->exactMatchAction.pclAction,
                                                                    &specificFormatSw.prvEpclEntryFormat);
            if (rc!=GT_OK)
            {
                *foundValidEntryPtr=GT_FALSE;
                return rc;
            }
            rc = saveEntryFormatParamsToDb_exactMatchEntryEpclFormat(&exactMatchEntryPtr->exactMatchAction.pclAction,
                                                                   &specificFormatHw.prvEpclEntryFormat);
            if (rc!=GT_OK)
            {
                *foundValidEntryPtr=GT_FALSE;
                return rc;
            }

            if(cpssOsMemCmp(&specificFormatSw.prvEpclEntryFormat,
                            &specificFormatHw.prvEpclEntryFormat,
                            sizeof(PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_ENTRY_EPCL_FORMAT_STC))!=0)
            {
                *foundValidEntryPtr=GT_FALSE;
                return GT_OK;
            }
        }
    }

    /* if we reach until here then all fields are equal and we found a match */
    *foundValidEntryPtr=GT_TRUE;
    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerDbFindValidIndexInHaTempDb function
* @endinternal
*
* @brief   This function uses result from function find if a given index exist in the HW.
*          The lookup is done in the HW tempDb, read when the device was added to the
*          manager, in the HA process.
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] entryPtr              - (pointer to) the entry (application format)*
* @param[in] bankId                - the bankId were we can locate the index we are looking for
* @param[inout] calcInfoPtr        - (pointer to) the calculated hash and lookup results
* @param[out]foundValidEntryPtr    - (pointer to) GT_TRUE: a valid HW entry was found
*                                                 GT_FALSE: no valid HW entry was found
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbFindValidIndexInHaTempDb
(
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC                      *entryPtr,
    IN GT_U32                                                       bankId,
    INOUT PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC  *calcInfoPtr,
    OUT GT_BOOL                                                     *foundValidEntryPtr
)
{
    GT_U32      rc = GT_OK;
    GT_U32      hwIndex;

    /* check that the tempDb was initialized - if not then this is an error and we should return a fail */
    if(exactMatchManagerPtr->haTempDbArr==NULL)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "haTempDbArr [%d] was not initialized ");
    }
    /* Look if the index is found in the HA temporary DB */
    hwIndex = calcInfoPtr->crcMultiHashArr[bankId];
    if (exactMatchManagerPtr->haTempDbArr[hwIndex].isValid==GT_TRUE)
    {
        if (exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr==NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"FAIL - the haTempDb is incorrect. can not have a index retrieve "
                                                  "from hash calculation with a isValid=true but with no legal "
                                                  "exactMatchEntryPtr");
        }
        /* check if the SW entry is the same as HW entry */
        rc = haCompareSwDataAndHwData(entryPtr,
                                      exactMatchManagerPtr->haTempDbArr[hwIndex].exactMatchEntryPtr,
                                      foundValidEntryPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    else
    {
        /* in this case we have a replay of an entry that is not in the HW */
        *foundValidEntryPtr=GT_FALSE;
    }

    return rc;
}

/**
* @internal prvCpssDxChExactMatchManagerDbSetIndexInHwTempDbAsTaken function
* @endinternal
*
* @brief   This function set a given index in the HW tempDb as taken
*
*        NOTE: the function doing only DB operations (no HW operations)
*
* @param[in] exactMatchManagerPtr  - (pointer to) the Exact Match manager.
* @param[in] calcInfoPtr           - (pointer to) calcInfo hold all information
*                                    regarding the Index value taken from the HW tempDb
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
*
*/
GT_STATUS   prvCpssDxChExactMatchManagerDbSetIndexInHwTempDbAsTaken
(

    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC       *exactMatchManagerPtr,
    IN PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_HASH_CALC_ENTRY_STC     *calcInfoPtr
)
{
    GT_U32      i=0;

    if(calcInfoPtr->selectedBankId==GT_NA)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error selectedBankId is not valid - fail in prvCpssDxChExactMatchManagerSetIndexInHwTempDbAsTaken \n");
    }

    /* set index as taken for all indexes of the entry
       if the entry is 5Bytes  - 1 index is taken
       if the entry is 19Bytes - 2 index is taken
       if the entry is 33Bytes - 3 index is taken
       if the entry is 47Bytes - 4 index is taken*/
    for (i=0;i<calcInfoPtr->bankStep;i++)
    {
        /* only a valid entry can be taken */
        if (exactMatchManagerPtr->haTempDbArr[calcInfoPtr->crcMultiHashArr[(calcInfoPtr->selectedBankId + i)]].isValid==GT_TRUE)
        {
            exactMatchManagerPtr->haTempDbArr[calcInfoPtr->crcMultiHashArr[(calcInfoPtr->selectedBankId + i)]].entryWasTaken = GT_TRUE;
        }
        else
        {
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL,"Error only a valid entry can be taken from the haTempDb-fail in prvCpssDxChExactMatchManagerDbSetIndexInHwTempDbAsTaken");
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChExactMatchManagerCompletionForHa function
* @endinternal
*
* @brief  this function goes over all the managers defined in
*         the system, and for each one, clean all the entries
*         that were not marked as used in the HA temporary DB
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
* @retval GT_FAIL                  - otherwise.
*
*
*/
GT_STATUS prvCpssDxChExactMatchManagerCompletionForHa
(
    GT_VOID
)
{
    GT_STATUS                                               rc=GT_OK;
    GT_U32                                                  exactMatchManagerId;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC  *exactMatchManagerPtr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DB_CHECKS_STC             dbChecks;

    for (exactMatchManagerId=0; exactMatchManagerId<CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS ; exactMatchManagerId++)
    {
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);
        if (exactMatchManagerPtr!=NULL)
        {
            if (exactMatchManagerPtr->haTempDbArr!=NULL)
            {
                /*Invalidate non taken entries*/
                rc = prvCpssDxChExactMatchManagerHwInvalidateNonTakenIndexFromHaTempDb(exactMatchManagerId);
                if (rc!=GT_OK)
                {
                    return rc;
                }
                /* delete the haTempDb and free all allocations done */
                rc=prvCpssDxChExactMatchManagerHwHaTempDbDelete(exactMatchManagerId);
                if (rc!=GT_OK)
                {
                    return rc;
                }
            }
            /* check all Exact Match SW entries are Sync with HW */
            cpssOsMemSet(&dbChecks,0,sizeof(dbChecks));
            dbChecks.hwUsedListCheckEnable = GT_TRUE;
            rc = prvCpssDxChExactMatchManagerHwDbValidityCheck(exactMatchManagerId,&dbChecks);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChExactMatchManagerLibraryShutdown function
* @endinternal
*
* @brief   Shutdown Exact Match Manager Library.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_FOUND             - on Exact Match Manager not found
*/
GT_STATUS prvCpssDxChExactMatchManagerLibraryShutdown
(
    GT_VOID
)
{
    GT_STATUS                                                   rc=GT_OK;
    GT_U32                                                      exactMatchManagerId;
    PRV_CPSS_DXCH_EXACT_MATCH_MANAGER_DB_INSTANCE_INFO_STC      *exactMatchManagerPtr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC       pairListArr[PRV_CPSS_MAX_PP_DEVICES_CNS];
    GT_U32                                                      numOfPairs;
    GT_U8                                                       devNum = 0;
    GT_PORT_GROUPS_BMP                                          portGroupBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                                      i=0;

    for (exactMatchManagerId = 0; (exactMatchManagerId < CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_INSTANCE_NUM_CNS); exactMatchManagerId++)
    {
        exactMatchManagerPtr = EM_MANAGER_GET_MAC(exactMatchManagerId);

        if (exactMatchManagerPtr != NULL)
        {
            /* build parameter to pass to the delete devices function */
            numOfPairs = exactMatchManagerPtr->numOfDevices;
            rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_TRUE, &devNum, &portGroupBmp);
            if(rc!=GT_OK)
            {
                return rc;
            }
            for (i=0;i<numOfPairs;i++)
            {
                pairListArr[i].devNum = devNum;
                pairListArr[i].portGroupsBmp = portGroupBmp;
                rc = prvCpssDxChExactMatchManagerDbDeviceIteratorGetNext(exactMatchManagerPtr, GT_FALSE, &devNum, &portGroupBmp);
                if(rc!=GT_OK)
                {
                    if(rc == GT_NO_MORE)
                    {
                        rc = GT_OK;
                        break;
                    }
                    return rc;
                }
            }

            /************************************************/
            /* remove devices from DB (+ DB validity check) */
            /************************************************/
            rc = prvCpssDxChExactMatchManagerDbDevListRemove(exactMatchManagerId,pairListArr,numOfPairs);
            if(rc != GT_OK)
            {
                return rc;
            }

            /************************************************/
            /* remove the manager                           */
            /************************************************/
            rc = prvCpssDxChExactMatchManagerDbDelete(exactMatchManagerId);
            if(rc != GT_OK)
            {
                return rc;
            }

            /************************************************/
            /* delete cuckoo DB                             */
            /************************************************/
            rc = prvCpssDxChCuckooDbDelete(PRV_CPSS_DXCH_CUCKOO_CLIENTS_EXACT_MATCH_E,exactMatchManagerId);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return rc;
}
