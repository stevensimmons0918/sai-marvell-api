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
* @file prvTgfPacketAnalyzerDebugUdsUdf.c
*
* @brief Test Packet Analyzer functionality when sending unknown trafffic
*
* @version
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>

#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzer.h>
#include <packetAnalyzer/prvTgfPacketAnalyzerDebugUcTraffic.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_TGF_MANAGER_ID_CNS  1
#define PRV_TGF_KEY_ID_CNS      1
#define PRV_TGF_GROUP_ID_CNS    1
#define PRV_TGF_ACTION_ID_CNS   1
#define PRV_TGF_RULE_ID_CNS     1

#define PRV_TGF_SEND_PORT_INDEX_CNS 0
#define PRV_TGF_VLANID_CNS      0x131

#define LENGTH_TO_NUM_WORDS_MAC(_len)     ((_len+31) >> 5)
/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x21, 0x12}                 /* saMac */
};

/* L2 part of packet2 */
static TGF_PACKET_L2_STC prvTgfPacketL2Part2 = {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x73}                 /* saMac */
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {0x600};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                          /* pri, cfi, VlanId */
};

/* packet's ipv4 ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2E,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xFF,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 2,  2,  2,  4},   /* srcAddr */
    { 1,  1,  0,  0}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPacketPayloadDataArr),                 /* dataLength */
    prvTgfPacketPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet2 */
static TGF_PACKET_PART_STC prvTgfPacketPartArray2[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part2},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray3[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part2},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet for vlan test */
static TGF_PACKET_PART_STC prvTgfPacketPartArray4[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo2 =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray2)
        / sizeof(prvTgfPacketPartArray2[0])), /* numOfParts */
    prvTgfPacketPartArray2                    /* partsArray */
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo3 =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray3)
        / sizeof(prvTgfPacketPartArray3[0])), /* numOfParts */
    prvTgfPacketPartArray3                    /* partsArray */
};

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo4 =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray4)
        / sizeof(prvTgfPacketPartArray4[0])), /* numOfParts */
    prvTgfPacketPartArray4                    /* partsArray */
};

static GT_U32      globalManagerId = 0 ;
/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate
(
     GT_U32         packetToSend
)
{
    GT_STATUS  rc  = GT_OK;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG1_MAC("\nSending unkown uc packet from port [%d]\n\n",
                     prvTgfPortsArray[PRV_TGF_SEND_PORT_INDEX_CNS]);
    /* setup Packet */
    switch (packetToSend) {
        case 1:
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 1 /*burstCount*/, 0 /*numVfd*/, NULL);
            break;
        case 2:
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo2, 1 /*burstCount*/, 0 /*numVfd*/, NULL);
            break;
        case 3:
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo3, 1 /*burstCount*/, 0 /*numVfd*/, NULL);
            break;
        case 4:
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo4, 1 /*burstCount*/, 0 /*numVfd*/, NULL);
            break;
        default:
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 1 /*burstCount*/, 0 /*numVfd*/, NULL);
            break;

    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");
    /* Send packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_INDEX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdsUdfUpdateRule
*           function
* @endinternal
*
* @brief   change rule data and set Sampling Enable.
*/
GT_VOID prvTgfPacketAnalyzerDebugUdsUdfUpdateRule
(
   GT_U32                                     numberOfFields,
   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC  fieldsValueArr[],
   GT_BOOL                                    setField
)
{
    GT_STATUS                                   rc = GT_OK;

    if (setField == GT_TRUE)
    {

        /* AUTODOC: disable group activation */
        rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId,PRV_TGF_GROUP_ID_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet");

        rc = cpssDxChPacketAnalyzerGroupRuleUpdate(globalManagerId,PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS,numberOfFields,fieldsValueArr,PRV_TGF_ACTION_ID_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupRuleUpdate");

        /* AUTODOC: enable group activation */
        rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId,PRV_TGF_GROUP_ID_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet");
    }

    /* AUTODOC: enable sampling on all rules bounded to action */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId,PRV_TGF_ACTION_ID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionSamplingEnableSet");

}


/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdsConfigSet
*           function
* @endinternal
*
* @brief   Set Packet Analyzer test configuration
*/
GT_VOID prvTgfPacketAnalyzerDebugUdsConfigSet
(
    GT_BOOL concatinatedInterface
)
{
    GT_STATUS                                               rc = GT_OK;

    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC            keyAttr;
    CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC          groupAttr;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC           ruleAttr;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     *fieldsArr;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                    action;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *fieldsValueArr;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             udsId;
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC            udsAttr;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC           udsInterfaceAttributes;
    GT_U32                                                  numOfFields;

    /* array allocation */
    stagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    fieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

    if (stagesArr == NULL || fieldsArr == NULL || fieldsValueArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    cpssOsMemSet(stagesArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    cpssOsMemSet(fieldsArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    cpssOsMemSet(fieldsValueArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

    /* AUTODOC: configure manager attributes */
    PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(globalManagerId);

    /* AUTODOC: create manager */
    rc = cpssDxChPacketAnalyzerManagerCreate(globalManagerId);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: add device to manager */
    rc = cpssDxChPacketAnalyzerManagerDeviceAdd(globalManagerId, prvTgfDevNum);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable PA on all devices added to manager */
    rc = cpssDxChPacketAnalyzerManagerEnableSet(globalManagerId, GT_TRUE);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");

    switch(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        cpssOsStrCpy(udsInterfaceAttributes.instanceId,"ipcl_tcam_or_debug_buses");
        cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"L2I_2_MT_NA_BUS");
        udsInterfaceAttributes.interfaceIndex = 4;
        break;
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        cpssOsStrCpy(udsInterfaceAttributes.instanceId,"l2i_ipvx_desc_or_debug_buses_0_8");
        cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"l2i2mt_na_bus");
        udsInterfaceAttributes.interfaceIndex = 2;
        break;
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
        if(GT_FALSE==concatinatedInterface)
        {
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_pipe0_eagle_l2i_ipvx_0_macro_mt_or_shm");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"L2I_2_MT_LEARN");
            udsInterfaceAttributes.interfaceIndex = 1;
        }
        else
        {
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_pipe0_eagle_epcl_0_macro_epcl_desc");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"ERMRK_2_PHA_HEADER");
            udsInterfaceAttributes.interfaceIndex = 4;
        }
        break;
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
        if(GT_FALSE==concatinatedInterface)
        {
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "pnx_l2i_ipvx_macro_mt_or_shm");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"L2I_2_MT_LEARN");
            udsInterfaceAttributes.interfaceIndex = 0;
        }
        else
        {
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "pnx_epcl_macro_ermrk_desc");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"ERMRK_2_PHA_HEADER");
            udsInterfaceAttributes.interfaceIndex = 1;
        }
        break;
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
        if(GT_FALSE==concatinatedInterface)
        {
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_l2i_ipvx_macro_mt_or_shm");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"L2I_2_MT_LEARN");
            udsInterfaceAttributes.interfaceIndex = 0;
        }
        else
        {
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_epcl_ha_macro_ermrk_desc");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"ERMRK_2_PHA_HEADER");
            udsInterfaceAttributes.interfaceIndex = 1;
        }
        break;
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        if(GT_FALSE==concatinatedInterface)
        {
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "har_l2i_ipvx_macro_mt_or_shm");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"L2I_2_MT_LEARN");
            udsInterfaceAttributes.interfaceIndex = 0;
        }
        else
        {
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "har_pha_macro_hbu2tti");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"HBU_2_HA_HEADER");
            udsInterfaceAttributes.interfaceIndex = 3;
        }
        break;
    default:
        rc = GT_NOT_SUPPORTED;
        goto exit_cleanly_lbl;
        break;
    }

    rc = cpssDxChPacketAnalyzerUserDefinedStageAdd(globalManagerId,&udsAttr,&udsInterfaceAttributes,&udsId);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    switch(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        break;
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        /*bind and unbind mux stages */
        rc = cpssDxChPacketAnalyzerMuxStageUnbind(globalManagerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
        if (rc != GT_OK)
        {
            goto exit_cleanly_lbl;
        }
        break;
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
        if(GT_TRUE==concatinatedInterface)
        {
            /*bind and unbind mux stages */
            rc = cpssDxChPacketAnalyzerMuxStageUnbind(globalManagerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E);
            if (rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        }
        break;
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
        if(GT_TRUE==concatinatedInterface)
        {
            /*bind and unbind mux stages */
            rc = cpssDxChPacketAnalyzerMuxStageUnbind(globalManagerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E);
            if (rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        }
        break;
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        if(GT_TRUE==concatinatedInterface)
        {
            /*bind and unbind mux stages */
            rc = cpssDxChPacketAnalyzerMuxStageUnbind(globalManagerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E);
            if (rc != GT_OK)
            {
                goto exit_cleanly_lbl;
            }
        }
        break;
    default:
        rc = GT_NOT_SUPPORTED;
        goto exit_cleanly_lbl;
        break;
    }

    rc = cpssDxChPacketAnalyzerMuxStageBind(globalManagerId,udsId);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    cpssOsMemSet(&keyAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));

    cpssOsMemCpy(keyAttr.keyNameArr, "uds key ", sizeof(GT_CHAR) * CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
    stagesArr[0] = udsId;
    fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
    numOfFields = 1 ;

    rc = cpssDxChPacketAnalyzerLogicalKeyCreate(globalManagerId, PRV_TGF_KEY_ID_CNS,&keyAttr, 1, stagesArr,
                                            CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E,
                                            numOfFields, fieldsArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: create group1 */
    cpssOsMemSet(&groupAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC));
    cpssOsMemCpy(groupAttr.groupNameArr,"group id 1",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
    rc = cpssDxChPacketAnalyzerGroupCreate(globalManagerId,PRV_TGF_GROUP_ID_CNS, &groupAttr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: create default action */
    cpssOsMemSet(&action,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));
    action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E ;
    rc = cpssDxChPacketAnalyzerActionCreate(globalManagerId,PRV_TGF_ACTION_ID_CNS, &action);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: create rule in group bounded logical key */
    cpssOsMemSet(&ruleAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
    cpssOsMemCpy(ruleAttr.ruleNameArr,"rule id 1 ",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);

    fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
    fieldsValueArr[0].data[0] = 0x2112;
    fieldsValueArr[0].msk[0] = 0xFFFF;
    numOfFields = 1 ;

    if(concatinatedInterface)
    {
        numOfFields = 0;
    }

    rc = cpssDxChPacketAnalyzerGroupRuleAdd(globalManagerId,PRV_TGF_KEY_ID_CNS, PRV_TGF_GROUP_ID_CNS, PRV_TGF_RULE_ID_CNS, &ruleAttr,
                                                numOfFields , fieldsValueArr, PRV_TGF_ACTION_ID_CNS);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: clear sampling data and counters for rule in group */
    rc = cpssDxChPacketAnalyzerSampledDataCountersClear(globalManagerId, PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable group activation */
    rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId, PRV_TGF_GROUP_ID_CNS, GT_TRUE);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable sampling on all rules bounded to action */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId, PRV_TGF_ACTION_ID_CNS, GT_TRUE);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

exit_cleanly_lbl:

    cpssOsFree(stagesArr);
    cpssOsFree(fieldsArr);
    cpssOsFree(fieldsValueArr);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerDebugUdsConfigSet");
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdsResultsGet
*           function
* @endinternal
*
* @brief   Check Packet Analyzer test Results
*/
GT_VOID prvTgfPacketAnalyzerDebugUdsResultsGet
(
   IN GT_U32                                                numOfHitArr[],
   IN GT_U32                                                expSample[],
   IN GT_BOOL                                               concatinated
)
{
    GT_STATUS                                               rc = GT_OK;
    GT_U32                                                  numOfFields;
    GT_U32                                                  numOfMatchedStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *matchedStagesArr;
    GT_U32                                                  numOfHits;
    GT_U32                                                  numOfSampleFieldsExp,numOfSampleFieldsGet;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *sampleFieldsValueArr = NULL;
    GT_U32                                                  ii,jj,kk,length;

    /* array allocation */
    matchedStagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    if (matchedStagesArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerDebugUdsResultsGet");
    }

    /* AUTODOC: disable sampling on all rules bounded to action */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId,PRV_TGF_ACTION_ID_CNS, GT_FALSE);
    if (rc != GT_OK)
    {
        if(matchedStagesArr)cpssOsFree(matchedStagesArr);
        matchedStagesArr=NULL;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionSamplingEnableSet");
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: check on which stages there was a match for rule in group */
    prvTgfPacketAnalyzerCreateLogicalStagesArrWithFields(&numOfMatchedStages,matchedStagesArr);

    if(GT_FALSE==concatinated)
    {

        if ((numOfMatchedStages != 1)||(matchedStagesArr[0] != CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E ))
        {
            PRV_UTF_LOG3_MAC("\nError: Match on ruleId [%d] in groupId [%d] Failed;"
                             "\nnumOfMatchStages [%d]" ,
                             PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS ,numOfMatchedStages);
            for (ii=0;ii<numOfMatchedStages;ii++)
            {
                if ( (matchedStagesArr[ii] >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E) &&
                     (matchedStagesArr[ii] <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E ))
                    PRV_UTF_LOG1_MAC("\nUds stage [%d]",
                                 (matchedStagesArr[ii]-CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E ));
                else
                    PRV_UTF_LOG1_MAC("\nstage [%s]",
                                 paStageStrArr[matchedStagesArr[ii]]);
            }
            rc = GT_FAIL;
            if(matchedStagesArr)cpssOsFree(matchedStagesArr);
            matchedStagesArr=NULL;;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerDebugUdsResultsGet");
            goto exit_cleanly_lbl;
        }
     }
     else
     {
        numOfMatchedStages=1;
        matchedStagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E;
     }

    numOfFields = 1;
    sampleFieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numOfFields);

    if (sampleFieldsValueArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: read number of fields for each stage in logical key */
    for (ii=0; ii<numOfMatchedStages; ii++)
    {
        cpssOsMemSet(sampleFieldsValueArr,0,sizeof(sampleFieldsValueArr));
        sampleFieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        numOfSampleFieldsExp=1;
        numOfSampleFieldsGet = numOfSampleFieldsExp;

        if(GT_TRUE==concatinated)
        {
            numOfSampleFieldsGet=0;
        }

        rc = cpssDxChPacketAnalyzerStageMatchDataGet(globalManagerId, PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS,
                                                    matchedStagesArr[ii], &numOfHits,&numOfSampleFieldsGet,sampleFieldsValueArr);
        if (rc != GT_OK)
            goto exit_cleanly_lbl;

        if(GT_FALSE==concatinated)
        {

            if ((numOfSampleFieldsGet != numOfSampleFieldsExp) ||
            (numOfHits != numOfHitArr[ii]))
            {
                if ( (matchedStagesArr[ii] >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E) &&
                     (matchedStagesArr[ii] <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E ))
                    PRV_UTF_LOG5_MAC("\n Error: Sample data on ruleId [%d] in groupId [%d] Failed;"
                                     "\n stage uds [%d]"
                                     "\n numOfSampleFields [%d] numOfHits [%d]",
                                     PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS,
                                     (matchedStagesArr[ii]-CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E ),
                                     numOfSampleFieldsGet,numOfHits);
                else
                    PRV_UTF_LOG5_MAC("\n Error: Sample data on ruleId [%d] in groupId [%d] Failed;"
                                     "\n stage [%s]"
                                     "\n numOfSampleFields [%d] numOfHits [%d]",
                                     PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS,
                                     paStageStrArr[matchedStagesArr[ii]],
                                     numOfSampleFieldsGet,numOfHits);

                rc = GT_FAIL;
                goto exit_cleanly_lbl;
            }

            if (numOfHits > 0)
            {
                if ((sampleFieldsValueArr[0].data[0] != expSample[ii]))
                {
                    PRV_UTF_LOG0_MAC("\n sample data not as expected");
                    PRV_UTF_LOG2_MAC("\n data got 0x%x , expected 0x%x",sampleFieldsValueArr[0].data[0] , expSample[ii]);
                    rc = GT_FAIL;
                    goto exit_cleanly_lbl;
                }
                if ( (matchedStagesArr[ii] >= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E) &&
                     (matchedStagesArr[ii] <= PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E ))
                        PRV_UTF_LOG4_MAC("\nSample data on ruleId [%d] in groupId [%d] numOfSampleFields [%d] \n"
                                 "uds stage [%d]:", PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS, numOfSampleFieldsGet,
                                 (matchedStagesArr[ii]-CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E ));
                else
                    PRV_UTF_LOG4_MAC("\nSample data on ruleId [%d] in groupId [%d] numOfSampleFields [%d] \n"
                                 "stage [%s]:", PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS, numOfSampleFieldsGet,
                                 paStageStrArr[matchedStagesArr[ii]]);
                for (jj = 0; jj < numOfSampleFieldsGet; jj++) {
                    PRV_UTF_LOG3_MAC("\n%d. sampleField [%s] numOfHits [%d] ", jj + 1, paFieldStrArr[sampleFieldsValueArr[jj].fieldName], numOfHits);
                    rc = cpssDxChPacketAnalyzerFieldSizeGet(globalManagerId, sampleFieldsValueArr[jj].fieldName, &length);
                    if (rc != GT_OK) {
                        goto exit_cleanly_lbl;
                    }

                    for (kk = 0; kk < LENGTH_TO_NUM_WORDS_MAC(length); kk++) {
                        PRV_UTF_LOG2_MAC("data%d[0x%x] ", kk, sampleFieldsValueArr[jj].data[kk]);
                    }
                }
                PRV_UTF_LOG0_MAC("\n\n");
            }
            else
            {
                PRV_UTF_LOG1_MAC("\nstage [%s]: no hit , as expected",paStageStrArr[matchedStagesArr[ii]]);
                PRV_UTF_LOG0_MAC("\n\n");
            }
        }
        else
        {

#ifdef ASIC_SIMULATION
        numOfHits =1;
#endif

            if(numOfHits==0)
            {
               PRV_UTF_LOG1_MAC("\nstage [%d]: no hit . NOT as expected\n",matchedStagesArr[ii]);
               goto exit_cleanly_lbl;
            }
            else
            {
                PRV_UTF_LOG2_MAC("\nstage [%d]: [%d] hits \n",matchedStagesArr[ii],numOfHits);
                rc = prvCpssDxChPacketAnalyzerDumpSwCounters(globalManagerId,prvTgfDevNum,matchedStagesArr[ii]);

                if (rc != GT_OK)
                {
                    PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerRuleMatchDataGet",rc);
                    goto exit_cleanly_lbl;
                }

                if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E )
                    rc = prvCpssDxChIdebugPortGroupInterfaceSampleDump(prvTgfDevNum,0xFFFFFFFF,"ERMRK_2_PHA_HEADER","tile0_pipe0_eagle_epcl_0_macro_epcl_desc");
                if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E )
                    rc = prvCpssDxChIdebugPortGroupInterfaceSampleDump(prvTgfDevNum,0xFFFFFFFF,"ERMRK_2_PHA_HEADER","pnx_epcl_macro_ermrk_desc");
                if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E )
                    rc = prvCpssDxChIdebugPortGroupInterfaceSampleDump(prvTgfDevNum,0xFFFFFFFF,"ERMRK_2_PHA_HEADER","tile0_epcl_ha_macro_ermrk_desc");
                if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E )
                    rc = prvCpssDxChIdebugPortGroupInterfaceSampleDump(prvTgfDevNum,0xFFFFFFFF,"HBU_2_HA_HEADER","har_pha_macro_hbu2tti");
                if (rc != GT_OK)
                {
                     goto exit_cleanly_lbl;
                 }
            }
        }
    }

exit_cleanly_lbl:

    if(matchedStagesArr)cpssOsFree(matchedStagesArr);
    if(sampleFieldsValueArr)cpssOsFree(sampleFieldsValueArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerDebugUdsResultsGet");
}
/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugUdfConfigSet
*           function
* @endinternal
*
* @brief   Set Packet Analyzer test configuration
*/
GT_VOID prvTgfPacketAnalyzerDebugUdfConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                               rc = GT_OK;

    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC            keyAttr;
    CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC          groupAttr;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC           ruleAttr;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     *fieldsArr;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                    action;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *fieldsValueArr;

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     udfIdArr[2];
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC            udfAttr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC                udfFieldName;

    GT_U32                                                  numOfFields;

    /* array allocation */
    stagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    fieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

    if (stagesArr == NULL || fieldsArr == NULL || fieldsValueArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    cpssOsMemSet(stagesArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    cpssOsMemSet(fieldsArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    cpssOsMemSet(fieldsValueArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

    /* AUTODOC: configure manager attributes */
    PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(globalManagerId);

    /* AUTODOC: create manager */
    rc = cpssDxChPacketAnalyzerManagerCreate(globalManagerId);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: add device to manager */
    rc = cpssDxChPacketAnalyzerManagerDeviceAdd(globalManagerId, prvTgfDevNum);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable PA on all devices added to manager */
    rc = cpssDxChPacketAnalyzerManagerEnableSet(globalManagerId, GT_TRUE);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    cpssOsStrCpy(udfAttr.udfNameArr,"is_bc");
    cpssOsStrCpy(udfFieldName.fieldNameArr,"is_bc");
    rc = cpssDxChPacketAnalyzerUserDefinedFieldAdd(globalManagerId,&udfAttr,&udfFieldName,&udfIdArr[0]);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }
    cpssOsStrCpy(udfAttr.udfNameArr,"ttl");
    cpssOsStrCpy(udfFieldName.fieldNameArr,"ttl");
    rc = cpssDxChPacketAnalyzerUserDefinedFieldAdd(globalManagerId,&udfAttr,&udfFieldName,&udfIdArr[1]);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    cpssOsMemSet(&keyAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));

    cpssOsMemCpy(keyAttr.keyNameArr, "uds key ", sizeof(GT_CHAR) * CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
    stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;
    fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
    fieldsArr[1] = udfIdArr[0];
    fieldsArr[2] = udfIdArr[1];
    numOfFields = 3 ;

    rc = cpssDxChPacketAnalyzerLogicalKeyCreate(globalManagerId, PRV_TGF_KEY_ID_CNS,&keyAttr, 1, stagesArr,
                                            CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E,
                                            numOfFields, fieldsArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: create group1 */
    cpssOsMemSet(&groupAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC));
    cpssOsMemCpy(groupAttr.groupNameArr,"group id 1",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
    rc = cpssDxChPacketAnalyzerGroupCreate(globalManagerId,PRV_TGF_GROUP_ID_CNS, &groupAttr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: create default action */
    cpssOsMemSet(&action,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));
    action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E ;
    rc = cpssDxChPacketAnalyzerActionCreate(globalManagerId,PRV_TGF_ACTION_ID_CNS, &action);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: create rule in group bounded logical key */
    cpssOsMemSet(&ruleAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
    cpssOsMemCpy(ruleAttr.ruleNameArr,"rule id 1 ",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);

    fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
    fieldsValueArr[0].data[0] = 0xffff;
    fieldsValueArr[0].msk[0] = 0xffff;
    fieldsValueArr[1].fieldName = udfIdArr[0];
    fieldsValueArr[1].data[0] = 0x1; /*is_bc = 1 */
    fieldsValueArr[1].msk[0] = 0xffff;
    fieldsValueArr[2].fieldName = udfIdArr[1];
    fieldsValueArr[2].data[0] = 64;
    fieldsValueArr[2].msk[0] = 0xffff;
    numOfFields = 3 ;

    rc = cpssDxChPacketAnalyzerGroupRuleAdd(globalManagerId,PRV_TGF_KEY_ID_CNS, PRV_TGF_GROUP_ID_CNS, PRV_TGF_RULE_ID_CNS, &ruleAttr,
                                                numOfFields , fieldsValueArr, PRV_TGF_ACTION_ID_CNS);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: clear sampling data and counters for rule in group */
    rc = cpssDxChPacketAnalyzerSampledDataCountersClear(globalManagerId, PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable group activation */
    rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId, PRV_TGF_GROUP_ID_CNS, GT_TRUE);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable sampling on all rules bounded to action */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId, PRV_TGF_ACTION_ID_CNS, GT_TRUE);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

exit_cleanly_lbl:

    cpssOsFree(stagesArr);
    cpssOsFree(fieldsArr);
    cpssOsFree(fieldsValueArr);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerDebugUdfConfigSet");
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugAsymmetricalInstancesConfigSet
*           function
* @endinternal
*
* @brief   Set Packet Analyzer test configuration
*/
GT_VOID prvTgfPacketAnalyzerDebugAsymmetricalInstancesConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                               rc = GT_OK;

    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC            keyAttr;
    CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC          groupAttr;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC           ruleAttr;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     *fieldsArr;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                    action;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *fieldsValueArr;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             udsId;
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC            udsAttr;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC           udsInterfaceAttributes;
    GT_U32                                                  numOfFields;

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     udfId;
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC            udfAttr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC                udfFieldName;

    /* array allocation */
    stagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    fieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

    if (stagesArr == NULL || fieldsArr == NULL || fieldsValueArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    cpssOsMemSet(stagesArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    cpssOsMemSet(fieldsArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    cpssOsMemSet(fieldsValueArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);

    /* AUTODOC: configure manager attributes */
    PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(globalManagerId);

    /* AUTODOC: create manager */
    rc = cpssDxChPacketAnalyzerManagerCreate(globalManagerId);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: add device to manager */
    rc = cpssDxChPacketAnalyzerManagerDeviceAdd(globalManagerId, prvTgfDevNum);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable PA on all devices added to manager */
    rc = cpssDxChPacketAnalyzerManagerEnableSet(globalManagerId, GT_TRUE);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    cpssOsStrCpy(udsAttr.udsNameArr, "PDS_2_QFC_DEQ");
    cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_pipe1_txqs_macro_i3_pipe0_1");
    cpssOsStrCpy(udsInterfaceAttributes.interfaceId, "PDS_2_QFC_DEQ");
    udsInterfaceAttributes.interfaceIndex = 2;

    rc = cpssDxChPacketAnalyzerUserDefinedStageAdd(globalManagerId,&udsAttr,&udsInterfaceAttributes,&udsId);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    rc = cpssDxChPacketAnalyzerMuxStageBind(globalManagerId,udsId);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    cpssOsStrCpy(udfFieldName.fieldNameArr,"source_port");
    cpssOsStrCpy(udfAttr.udfNameArr,"source_port");
    rc = cpssDxChPacketAnalyzerUserDefinedFieldAdd(globalManagerId,&udfAttr,&udfFieldName,&udfId);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    cpssOsMemSet(&keyAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));

    cpssOsMemCpy(keyAttr.keyNameArr, "Asymmetrical key ", sizeof(GT_CHAR) * CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
    stagesArr[0] = udsId;
    fieldsArr[0] = udfId;
    numOfFields = 1 ;

    rc = cpssDxChPacketAnalyzerLogicalKeyCreate(globalManagerId, PRV_TGF_KEY_ID_CNS,&keyAttr, 1, stagesArr,
                                            CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E,
                                            numOfFields, fieldsArr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: create group1 */
    cpssOsMemSet(&groupAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC));
    cpssOsMemCpy(groupAttr.groupNameArr,"group id 1",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
    rc = cpssDxChPacketAnalyzerGroupCreate(globalManagerId,PRV_TGF_GROUP_ID_CNS, &groupAttr);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: create default action */
    cpssOsMemSet(&action,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));
    action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E ;
    rc = cpssDxChPacketAnalyzerActionCreate(globalManagerId,PRV_TGF_ACTION_ID_CNS, &action);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: create rule in group bounded logical key */
    cpssOsMemSet(&ruleAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
    cpssOsMemCpy(ruleAttr.ruleNameArr,"rule id 1 ",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
    fieldsValueArr[0].fieldName = udfId;
    fieldsValueArr[0].data[0] = 63;
    fieldsValueArr[0].msk[0] = 0xffff;
    numOfFields = 1 ;

    rc = cpssDxChPacketAnalyzerGroupRuleAdd(globalManagerId,PRV_TGF_KEY_ID_CNS, PRV_TGF_GROUP_ID_CNS, PRV_TGF_RULE_ID_CNS, &ruleAttr,
                                                numOfFields , fieldsValueArr, PRV_TGF_ACTION_ID_CNS);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable group activation */
    rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId, PRV_TGF_GROUP_ID_CNS, GT_TRUE);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: clear sampling data and counters for rule in group */
    rc = cpssDxChPacketAnalyzerSampledDataCountersClear(globalManagerId, PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable sampling on all rules bounded to action */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId, PRV_TGF_ACTION_ID_CNS, GT_TRUE);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }
exit_cleanly_lbl:

    cpssOsFree(stagesArr);
    cpssOsFree(fieldsArr);
    cpssOsFree(fieldsValueArr);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerDebugAsymmetricalInstancesConfigSet");
}
