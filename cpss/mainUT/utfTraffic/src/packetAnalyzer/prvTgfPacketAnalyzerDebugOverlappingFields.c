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
* @file prvTgfPacketAnalyzerDebugOverlappingFields.c
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

/* packet's ipv4 ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's ipv6 ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};

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
    { 1,  3,  2,  4},   /* srcAddr */
    { 1,  1,  0,  0}    /* dstAddr */
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    0x3b,               /* nextHeader */
    0x40,               /* hopLimit */
    {0x2222, 0x3303, 0, 0, 0, 0, 0x1011, 0x2222}, /* TGF_IPV6_ADDR srcAddr */
    {0xff00, 0, 0, 0, 0, 0, 0x0000, 0x2222}  /* TGF_IPV6_ADDR dstAddr */
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
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray2[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
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

/* PACKET1 to send */
static TGF_PACKET_STC prvTgfPacketInfo2 = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray2)
        / sizeof(prvTgfPacketPartArray2[0])), /* numOfParts */
    prvTgfPacketPartArray2                   /* partsArray */
};

static GT_U32      globalManagerId = 0 ;

/******************************************************************************/

/**
* @internal GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsTrafficGenerate
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
* @internal GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsConfigSet
*           function
* @endinternal
*
* @brief   Set Packet Analyzer test configuration
*/
GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsConfigSet
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

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     udfId;
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

    cpssOsStrCpy(udfFieldName.fieldNameArr,"arp_sip");
    cpssOsStrCpy(udfAttr.udfNameArr,"arp_sip");
    rc = cpssDxChPacketAnalyzerUserDefinedFieldAdd(globalManagerId,&udfAttr,&udfFieldName,&udfId);
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    cpssOsMemSet(&keyAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));

    cpssOsMemCpy(keyAttr.keyNameArr, "OverlappingFields key ", sizeof(GT_CHAR) * CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
    stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E;
    fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;
    fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E;
    fieldsArr[2] = udfId;
    numOfFields = 3 ;

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

    fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;
    fieldsValueArr[0].data[0] = 0x01030204;
    fieldsValueArr[0].msk[0] = 0xffffffff;
    fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E;
    fieldsValueArr[1].data[0] = 0x1;
    fieldsValueArr[1].msk[0] = 0xffff;
    fieldsValueArr[2].fieldName = udfId;
    fieldsValueArr[2].data[0] = 64;
    fieldsValueArr[2].msk[0] = 0xffff;
    numOfFields = 3 ;

    rc = cpssDxChPacketAnalyzerGroupRuleAdd(globalManagerId,PRV_TGF_KEY_ID_CNS, PRV_TGF_GROUP_ID_CNS, PRV_TGF_RULE_ID_CNS, &ruleAttr,
                                                numOfFields , fieldsValueArr, PRV_TGF_ACTION_ID_CNS);
    if (rc != GT_BAD_PARAM) /*should fail*/
    {
        goto exit_cleanly_lbl;
    }
    fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;
    fieldsValueArr[0].data[0] = 0x01030204;
    fieldsValueArr[0].msk[0] = 0xffffffff;
    numOfFields = 1 ;

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
* @internal GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsChangeField
*           function
* @endinternal
*
* @brief   change rule field and set Sampling Enable.
*/
GT_VOID prvTgfPacketAnalyzerDebugOverlappingFieldsChangeField
(
   GT_U32                                     numberOfFields,
   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC  fieldsValueArr[]
)
{
    GT_STATUS                                       rc = GT_OK;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC   ruleAttr;

    /* AUTODOC: disable group activation */
    rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId,PRV_TGF_GROUP_ID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet");

    rc = cpssDxChPacketAnalyzerGroupRuleDelete(globalManagerId , PRV_TGF_RULE_ID_CNS , PRV_TGF_GROUP_ID_CNS );
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupRuleDelete");

    rc = cpssDxChPacketAnalyzerGroupRuleAdd(globalManagerId,PRV_TGF_KEY_ID_CNS, PRV_TGF_GROUP_ID_CNS, PRV_TGF_RULE_ID_CNS, &ruleAttr,
                                            numberOfFields , fieldsValueArr, PRV_TGF_ACTION_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupRuleAdd");

    /* AUTODOC: enable group activation */
    rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId,PRV_TGF_GROUP_ID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet");

    /* AUTODOC: enable sampling on all rules bounded to action */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId,PRV_TGF_ACTION_ID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionSamplingEnableSet");

}
