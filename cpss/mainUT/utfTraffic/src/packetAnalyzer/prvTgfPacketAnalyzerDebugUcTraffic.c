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
* @file prvTgfPacketAnalyzerDebugUcTraffic.c
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
#include <packetAnalyzer/prvTgfPacketAnalyzerDebugUcTraffic.h>

#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzer.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_TGF_MANAGER_ID_CNS  1
#define PRV_TGF_KEY_ID_CNS      1
#define PRV_TGF_GROUP_ID_CNS    1
#define PRV_TGF_ACTION_ID_CNS   1
#define PRV_TGF_RULE_ID_CNS     1

#define PRV_TGF_SEND_PORT_INDEX_CNS 2
#define PRV_TGF_DEF_VLANID_FOR_PA__CNS        1
#define PRV_TGF_VLANID_TEST_CNS      0x131
/*#define IGNORE_HITCOUNT*/

/*#define     IGNORE_FIELDS_RESULTS*/

/*#define     DUMP_MATCH_RESULTS*/

#define TGF_PA_FUNCTION_CALL(_rc,_func)\
    do\
    {\
        if(GT_TRUE==prvTgfPacketAnalyzerDebugModeEnableGet())\
        {\
            PRV_UTF_LOG1_MAC("\n\n\n>>>>>>>>>Call function %s\n\n\n",#_func);\
        }\
        _rc =_func;\
        if(GT_TRUE==prvTgfPacketAnalyzerDebugModeEnableGet())\
        {\
             cpssOsTimerWkAfter(100);\
        }\
    }while(0);

#define LENGTH_TO_NUM_WORDS_MAC(_len)     ((_len+31) >> 5)

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x55}                 /* saMac */
};

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

/* packet's ipv4 ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* DATA of packet */
static GT_U8 prvTgfPacketPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
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

/* packet to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,              /* totalLen */
    (sizeof(prvTgfPacketPartArray)
        / sizeof(prvTgfPacketPartArray[0])), /* numOfParts */
    prvTgfPacketPartArray                    /* partsArray */
};

static GT_BOOL     paDebugMode = GT_FALSE;
static GT_U32      globalManagerId = 0 ;

GT_VOID prvTgfPacketAnalyzerGeneralSwitchMuxStages
(
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageOut,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageIn
)
{
    GT_STATUS  rc;

    PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(globalManagerId);
    /*disable sampling and group activation */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId,PRV_TGF_ACTION_ID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionSamplingEnableSet action: ",PRV_TGF_ACTION_ID_CNS);
    rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId,PRV_TGF_GROUP_ID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet group: ",PRV_TGF_GROUP_ID_CNS);


    /*unbind default stage that mux and bind other mux stage*/
    PRV_UTF_LOG1_MAC("\nUnbind stage [%s]",paStageStrArr[stageOut]);
    rc = cpssDxChPacketAnalyzerMuxStageUnbind(globalManagerId,stageOut);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerMuxStageUnbind group: ",PRV_TGF_GROUP_ID_CNS);

    PRV_UTF_LOG1_MAC("\nbind stage [%s]",paStageStrArr[stageIn]);
    rc = cpssDxChPacketAnalyzerMuxStageBind(globalManagerId,stageIn);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerMuxStageBind group: ",PRV_TGF_GROUP_ID_CNS);

    /*enable sampling and group activation */
    rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId, PRV_TGF_GROUP_ID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet group: ",PRV_TGF_GROUP_ID_CNS);
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId, PRV_TGF_ACTION_ID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionSamplingEnableSet action: ",PRV_TGF_ACTION_ID_CNS);
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerUnknownUcRestore
*           function
* @endinternal
*
* @brief   Restore Packet Analyzer test Configurations
*/
GT_VOID prvTgfPacketAnalyzerUnknownUcRestore
(
    GT_BOOL restoreVlan
)
{
    GT_STATUS                                               rc;

    PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(globalManagerId);

    if(GT_TRUE==prvTgfPacketAnalyzerDebugModeEnableGet())
    {
            prvTgfPacketAnalyzerDebugModeEnableSet(GT_FALSE);
            prvWrAppTraceHwAccessEnable(prvTgfDevNum,CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_BOTH_E,GT_FALSE);
    }

    /* AUTODOC: enable group activation */
    rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId, PRV_TGF_GROUP_ID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet group: ",PRV_TGF_GROUP_ID_CNS);

    /* AUTODOC: Delete packet analyzer group */
    rc = cpssDxChPacketAnalyzerGroupDelete(globalManagerId, PRV_TGF_GROUP_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupDelete group: ",PRV_TGF_GROUP_ID_CNS);

    /* AUTODOC: Delete packet analyzer action */
    rc = cpssDxChPacketAnalyzerActionDelete(globalManagerId, PRV_TGF_ACTION_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionDelete action: %d",PRV_TGF_ACTION_ID_CNS);

    /* AUTODOC: Remove device from Packet manager */
    rc = cpssDxChPacketAnalyzerManagerDeviceRemove(globalManagerId,prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerManagerDeviceRemove dev: %d" ,prvTgfDevNum);

    /* AUTODOC: Delete packet analyzer manager */
    rc = cpssDxChPacketAnalyzerManagerDelete(globalManagerId);
    UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerManagerDelete manager: %d",globalManagerId);

    if(restoreVlan)
    {
        rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_DEF_VLANID_FOR_PA__CNS);
        UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_DEF_VLANID_FOR_PA__CNS);
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_NO_RETURN_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    globalManagerId = 0;
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerUnknownUcTrafficGenerate
*           function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfPacketAnalyzerUnknownUcTrafficGenerate
(
     GT_BOOL invalidateVlan,
     GT_BOOL useCustomPort,
     GT_U32  customPortIndex
)
{
    GT_STATUS  rc  = GT_OK;
    GT_U32     sendPortIndex;
    GT_BOOL    restoreDebugMode = GT_FALSE;

    if(GT_TRUE==prvTgfPacketAnalyzerDebugModeEnableGet())
    {
        restoreDebugMode=GT_TRUE;
        prvWrAppTraceHwAccessEnable(prvTgfDevNum,CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_BOTH_E,GT_FALSE);
    }


    if(GT_FALSE == useCustomPort)
    {
        sendPortIndex = PRV_TGF_SEND_PORT_INDEX_CNS;
    }
    else
    {
        sendPortIndex = customPortIndex;
    }

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG1_MAC("\nSending unkown uc packet from port [%d]\n\n",
                     prvTgfPortsArray[sendPortIndex]);

    if(invalidateVlan)
    {
        rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_DEF_VLANID_FOR_PA__CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                    "prvTgfBrgVlanEntryInvalidate: %d", prvTgfDevNum);
    }

    /* setup Packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, 1 /*burstCount*/, 0 /*numVfd*/, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");



    /* Send packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[sendPortIndex]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");

    if(GT_TRUE==restoreDebugMode)
    {
        prvTgfPacketAnalyzerDebugModeEnableSet(GT_TRUE);
        prvWrAppTraceHwAccessEnable(prvTgfDevNum,CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_BOTH_E,GT_TRUE);
    }

}

GT_VOID prvTgfPacketAnalyzerGeneralConfigSet
(
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArr,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     *fieldsArr,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *fieldsValueArr,
    GT_U32                                                   *numOfStagesPtr,
    GT_U32                                                   numOfFields,
    GT_CHAR_PTR                                              ruleName
)
{
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC            keyAttr;
    CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC          groupAttr;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC           ruleAttr;
    GT_STATUS                                               rc = GT_OK;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             firstStage;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT     fieldMode;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                    action;

    GT_U32                                                   ruleNameSize;
    GT_U32                                                   ii,jj,numOfStagesOut,numOfFieldsOut;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                      tmpfieldsArr[10];

    if (stagesArr == NULL || fieldsArr == NULL || fieldsValueArr == NULL||ruleName==NULL)
    {
        rc = GT_BAD_PARAM;
        goto exit_cleanly_lbl;
    }

    PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(globalManagerId);

    /* AUTODOC: create manager */
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerManagerCreate(globalManagerId))
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerManagerCreate",rc);
        goto exit_cleanly_lbl;
    }


    /* AUTODOC: add device to manager */
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerManagerDeviceAdd(globalManagerId, prvTgfDevNum))
    if (rc != GT_OK)
    {   PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerManagerDeviceAdd",rc);
        goto exit_cleanly_lbl;
    }


    /* AUTODOC: enable PA on all devices added to manager */
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerManagerEnableSet(globalManagerId, GT_TRUE))
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerManagerEnableSet",rc);
        goto exit_cleanly_lbl;
    }

    ruleNameSize = cpssOsStrlen(ruleName);

    cpssOsMemSet(&keyAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
    cpssOsMemCpy(keyAttr.keyNameArr,ruleName,sizeof(GT_CHAR)*ruleNameSize);

    firstStage = stagesArr[0];
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerLogicalKeyCreate(globalManagerId, PRV_TGF_KEY_ID_CNS,&keyAttr, *numOfStagesPtr, stagesArr,
                                                CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E,
                                                numOfFields, fieldsArr))
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerLogicalKeyCreate",rc);
        goto exit_cleanly_lbl;
    }


    /* AUTODOC: check that logical key creation is according to definition */
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerLogicalKeyInfoGet(globalManagerId, PRV_TGF_KEY_ID_CNS, &keyAttr, &fieldMode))
    if (rc != GT_OK)
    {   PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerLogicalKeyInfoGet",rc);
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: check number of stages created in logical key */
    numOfStagesOut = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerLogicalKeyStagesGet(globalManagerId, PRV_TGF_KEY_ID_CNS, &numOfStagesOut, stagesArr))
    if (rc != GT_OK)
    {
        PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerLogicalKeyStagesGet",rc);
        goto exit_cleanly_lbl;
    }

    if ((firstStage == CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E) ||
        (firstStage == CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E) ||
        (firstStage == CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_STAGES_E))
    {
        *numOfStagesPtr=numOfStagesOut;
    }

    if (*numOfStagesPtr < numOfStagesOut)
    {
        PRV_UTF_LOG2_MAC("Error: Logical Key Creation Failed; numOfStages [%d] stage [%s]",numOfStagesOut,paStageStrArr[stagesArr[0]]);
        rc = GT_FAIL;
        goto exit_cleanly_lbl;
    }
    else
    {
        PRV_UTF_LOG2_MAC("Info: set [%d/%d] Stages in key",numOfStagesOut,*numOfStagesPtr);
        *numOfStagesPtr=numOfStagesOut;
    }

    /* AUTODOC: create group */
    cpssOsMemSet(&groupAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC));
    cpssOsMemCpy(groupAttr.groupNameArr,"group id 1",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerGroupCreate(globalManagerId,PRV_TGF_GROUP_ID_CNS, &groupAttr))
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: create default action */
    cpssOsMemSet(&action,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));
    action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E ;
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerActionCreate(globalManagerId, PRV_TGF_ACTION_ID_CNS, &action))
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }


    /* AUTODOC: create rule in group bounded to ingress logical key */
    cpssOsMemSet(&ruleAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
    cpssOsMemCpy(ruleAttr.ruleNameArr,"rule id 1",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);

    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerGroupRuleAdd(globalManagerId, PRV_TGF_KEY_ID_CNS, PRV_TGF_GROUP_ID_CNS, PRV_TGF_RULE_ID_CNS, &ruleAttr,
                                            numOfFields, fieldsValueArr, PRV_TGF_ACTION_ID_CNS))
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    for (ii=0;ii<*numOfStagesPtr;ii++)
    {
        PRV_UTF_LOG1_MAC("\nstage [%s] set ,",paStageStrArr[stagesArr[ii]]);
        numOfFieldsOut = numOfFields ;
        rc = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(globalManagerId,PRV_TGF_KEY_ID_CNS,stagesArr[ii],&numOfFieldsOut,tmpfieldsArr);
        /*rc = cpssDxChPacketAnalyzerStageFieldsGet(globalManagerId,stagesArr[ii],&numOfFieldsOut,tmpfieldsArr);*/
        PRV_UTF_LOG1_MAC("  numOfFields set %d  ",numOfFieldsOut);
        for (jj=0 ; jj<numOfFieldsOut ; jj++ )
        {
            PRV_UTF_LOG2_MAC("\n  %d. field [%s]",jj,paFieldStrArr[tmpfieldsArr[jj]]);
        }
    }

    /* AUTODOC: clear sampling data and counters for rule in group */
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerSampledDataCountersClear(globalManagerId, PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS))
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable group activation */
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId, PRV_TGF_GROUP_ID_CNS, GT_TRUE))
    if (rc != GT_OK)
    {
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: enable sampling on all rules bounded to action */
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId, PRV_TGF_ACTION_ID_CNS, GT_TRUE))
    if (rc != GT_OK) {
        goto exit_cleanly_lbl;
    }

exit_cleanly_lbl:

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerGeneralConfigSet");
}

GT_VOID prvTgfPacketAnalyzerCreateLogicalStagesArrWithFields
(
    OUT GT_U32                                                  *numOfStagesOut,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArrOut
)
{
    GT_STATUS                                               rc = GT_OK;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArr=NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     *fieldsArr=NULL;
    GT_U32                                                  i,numOfStages,numOfFields;

    PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(globalManagerId);
    /* array allocation */
    stagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    if (stagesArr == NULL)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_OUT_OF_CPU_MEM, "prvCreateLogicalStagesArrWithFields");
    }

    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
    if (fieldsArr == NULL)
    {
        cpssOsFree(stagesArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_OUT_OF_CPU_MEM, "prvCreateLogicalStagesArrWithFields");
    }

    /* AUTODOC: get logical key stages array */
    numOfStages=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerLogicalKeyStagesGet(globalManagerId,PRV_TGF_KEY_ID_CNS,&numOfStages,stagesArr))
    if (rc != GT_OK)
    {
        cpssOsFree(stagesArr);
        cpssOsFree(fieldsArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCreateLogicalStagesArrWithFields");
    }

    *numOfStagesOut = 0;
    /* AUTODOC: get logical key number of fields array per stage */
    for (i=0; i<numOfStages; i++)
    {
        numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
        TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(globalManagerId,PRV_TGF_KEY_ID_CNS,stagesArr[i],&numOfFields,fieldsArr))
        if (rc != GT_OK)
        {
            cpssOsFree(stagesArr);
            cpssOsFree(fieldsArr);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCreateLogicalStagesArrWithFields");
        }
        if (numOfFields)
        {
            stagesArrOut[*numOfStagesOut] = stagesArr[i];
            (*numOfStagesOut)++;
        }
    }

    cpssOsFree(stagesArr);
    cpssOsFree(fieldsArr);
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerUnknownUcResultsGet
*           function
* @endinternal
*
* @brief   Check Packet Analyzer test Results
*/
GT_VOID prvTgfPacketAnalyzerGeneralResultsGet
(

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              *matchedStagesArr,
    GT_U32                                                   numOfMatchedStages,
    GT_U32                                                   numOfFields,
    GT_U32                                                   *expectedNumOfHits,
    GT_U32                                                   *expectedNumOfSampleFields,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC                *expectedSampleFieldsValueArr[]
)
{
    GT_STATUS                                               rc = GT_OK;
    GT_U32                                                  numOfMatchedStagesOut;
    GT_U32                                                  numOfHitsOut = 0;
    GT_U32                                                  numOfSampleFieldsOut = 0;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *sampleFieldsValueArrOut=NULL;
    GT_U32                                                  i,j,k,length,sampleFieldsIterator;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *matchedStagesArrOut=NULL;
    GT_BOOL                                                 rcMatch =GT_TRUE;

    /* array allocation */
    matchedStagesArrOut = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    if (matchedStagesArrOut == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: disable sampling on all rules bounded to action */
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId,PRV_TGF_ACTION_ID_CNS, GT_FALSE))
    if (rc != GT_OK)
    {
        cpssOsFree(matchedStagesArrOut);
        matchedStagesArrOut = NULL;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerUnknownUcResultsGet");
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: check on which stages there was a match for rule in group */
    prvTgfPacketAnalyzerCreateLogicalStagesArrWithFields(&numOfMatchedStagesOut,matchedStagesArrOut);

    if (numOfMatchedStages <numOfMatchedStagesOut)
    {
        PRV_UTF_LOG4_MAC("Error: Match on ruleId [%d] in groupId [%d] Failed; numOfMatchStages [%d] numOfMatchedStages [%d]",
                         PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS,numOfMatchedStagesOut,numOfMatchedStages);

        rc = GT_FAIL;
        cpssOsFree(matchedStagesArrOut);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerUnknownUcResultsGet");
    }
    else
    {
        PRV_UTF_LOG2_MAC("Expected num of matched stages [%d] is met ! Actually got %d\n",numOfMatchedStages,numOfMatchedStagesOut);
        numOfMatchedStages=numOfMatchedStagesOut;
    }



    for (i=0; i<numOfMatchedStages; i++)
    {
       if(matchedStagesArr[i] != matchedStagesArrOut[i])
       {
            PRV_UTF_LOG4_MAC("Error: Match on stage  [%d] in groupId [%d] Failed; matchedStagesArr [%d] expected [%0x%x]\n",
                             i,PRV_TGF_GROUP_ID_CNS,matchedStagesArrOut[i],matchedStagesArr[i] );

            PRV_UTF_LOG1_MAC("Expected  stage [%s]\n",paStageStrArr[matchedStagesArr[i]]);
            PRV_UTF_LOG1_MAC("Received  stage [%s]\n",paStageStrArr[matchedStagesArrOut[i]]);
            rc = GT_FAIL;
            cpssOsFree(matchedStagesArrOut);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerUnknownUcResultsGet");
        }
        else
       {
        PRV_UTF_LOG2_MAC("Expected  stage %d [%s] is met !\n",i,paStageStrArr[matchedStagesArr[i]]);
        }
    }

    /* allocate match data info array for single field CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E */
    sampleFieldsValueArrOut = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numOfFields);
    if (sampleFieldsValueArrOut == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        cpssOsFree(matchedStagesArrOut);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerUnknownUcResultsGet");
    }


    for (i=0; i<numOfMatchedStagesOut; i++)
    {
        PRV_UTF_LOG2_MAC("Match data index  %d for stage [%s] \n",i,paStageStrArr[matchedStagesArrOut[i]]);

        numOfSampleFieldsOut = expectedNumOfSampleFields[i];

        PRV_UTF_LOG2_MAC("IN :expectedNumOfHits  [%d]  expectedNumOfSampleFields [%d] \n",expectedNumOfHits[i],expectedNumOfSampleFields[i]);

        if(sampleFieldsValueArrOut==NULL)
        {
            PRV_UTF_LOG0_MAC("Error  sampleFieldsValueArrOut is NULL");
                    goto exit_cleanly_lbl;
        }

        for(sampleFieldsIterator=0;sampleFieldsIterator<numOfSampleFieldsOut;sampleFieldsIterator++)
        {
            sampleFieldsValueArrOut[sampleFieldsIterator].fieldName = expectedSampleFieldsValueArr[i][sampleFieldsIterator].fieldName;
        }

        PRV_UTF_LOG0_MAC("Read HW in order to get results\n");

        TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerStageMatchDataGet(globalManagerId,PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS,
                                                    matchedStagesArr[i],&numOfHitsOut,&numOfSampleFieldsOut, sampleFieldsValueArrOut))
        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerRuleMatchDataGet",rc);
            goto exit_cleanly_lbl;
        }

        PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerRuleMatchDataGet\n",rc);
        PRV_UTF_LOG2_MAC("OUT :numOfHitsOut  [%d]  numOfSampleFieldsOut [%d] \n",numOfHitsOut,numOfSampleFieldsOut);
        rc = prvCpssDxChPacketAnalyzerDumpSwCounters(globalManagerId,prvTgfDevNum,matchedStagesArr[i]);

        if (rc != GT_OK)
        {
            PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerRuleMatchDataGet",rc);
            goto exit_cleanly_lbl;
        }

#ifndef IGNORE_HITCOUNT
        /* AUTODOC: expect 1 hits for field CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E for each stage; with data CPSS_PACKET_CMD_FORWARD_E */
        if (numOfHitsOut != expectedNumOfHits[i])
        {
            PRV_UTF_LOG2_MAC("Error: Expected numOfHits [%d] received  [%d]\n",
                             expectedNumOfHits[i],numOfHitsOut);
            rcMatch = GT_FALSE;
        }
        else
#endif
        {
            PRV_UTF_LOG1_MAC("Expected  number of hits[%d] is met !\n",numOfHitsOut);


            if(numOfHitsOut!=0)
            {

                for(k=0;k<numOfSampleFieldsOut;k++)
                {
#ifndef IGNORE_FIELDS_RESULTS
                if (sampleFieldsValueArrOut[k].data[0] != expectedSampleFieldsValueArr[i][k].data[0])
            {
                    PRV_UTF_LOG3_MAC("Error: Expected sampleFieldsValueArr[%d].data[0] [%d] received  [%d]\n ",k,
                                     expectedSampleFieldsValueArr[i][k].data[0],
                                     sampleFieldsValueArrOut[k].data[0]);
                rcMatch = GT_FALSE;
            }
            else
            {
#endif
                    PRV_UTF_LOG2_MAC("Expected  sampleFieldsValueArr[%d].data[0] [%d] is met !\n",k,sampleFieldsValueArrOut[k].data[0]);
#ifndef IGNORE_FIELDS_RESULTS
                    }
#endif

                }
            }

        }

        if ( (rcMatch==GT_FALSE) ||
            (sampleFieldsValueArrOut[0].fieldName != expectedSampleFieldsValueArr[i][0].fieldName))
        {
            PRV_UTF_LOG6_MAC("Error: Sample data on ruleId [%d] in groupId [%d] Failed; numOfSampleFields [%d] sampleField [%s] numOfHits [%d] data [0x%x]",
                             PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS,
                             expectedNumOfSampleFields[i],paFieldStrArr[expectedSampleFieldsValueArr[i][0].fieldName],
                             expectedNumOfHits[i],expectedSampleFieldsValueArr[i][0].data[0]);
            rc = GT_FAIL;
            goto exit_cleanly_lbl;
        }

        PRV_UTF_LOG3_MAC("\nSample data on ruleId [%d] in groupId [%d] numOfSampleFields [%d]:",PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS,expectedNumOfSampleFields[i]);

        if(numOfHitsOut!=0)
        {
        for (k=0; k<numOfSampleFieldsOut; k++)
        {
           PRV_UTF_LOG3_MAC("\n%d. sampleField [%s] numOfHits [%d] ",k+1,paFieldStrArr[sampleFieldsValueArrOut[k].fieldName],numOfHitsOut);
           TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerFieldSizeGet(globalManagerId,sampleFieldsValueArrOut[k].fieldName,&length))
            if (rc != GT_OK)
            {   PRV_UTF_LOG1_MAC("%d = cpssDxChPacketAnalyzerRuleMatchDataGet",rc);
                goto exit_cleanly_lbl;
            }

            for (j=0; j<LENGTH_TO_NUM_WORDS_MAC(length); j++)
            {
                PRV_UTF_LOG2_MAC("data%d[0x%x] ",j,sampleFieldsValueArrOut[k].data[j]);
            }

        }
         }

        PRV_UTF_LOG0_MAC("\n\n");
    }


exit_cleanly_lbl:
    if(sampleFieldsValueArrOut)cpssOsFree(sampleFieldsValueArrOut);
    if(matchedStagesArrOut)cpssOsFree(matchedStagesArrOut);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerUnknownUcResultsGet");

}


GT_VOID  prvTgfPacketAnalyzerLogicalKeyFieldsPerStageGet
(
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId,
    INOUT GT_U32                                            *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT               fieldsArr[]
)
{
    GT_STATUS rc;
    TGF_PA_FUNCTION_CALL(rc,cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(globalManagerId, PRV_TGF_KEY_ID_CNS,stageId,
        numOfFieldsPtr,fieldsArr))
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet");
}





GT_STATUS  prvTgfPacketAnalyzerTestAddExpectedResultStage
(
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT *matchedStagesArr,
    IN GT_U32 index,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stageId,
    IN GT_U32 totalStages
)
{
    if(index>=totalStages)
    {
        PRV_UTF_LOG2_MAC("(index[%d]>=totalStages[%d])",index,totalStages);
        return GT_OUT_OF_RANGE;
    }

   matchedStagesArr[index] = stageId;
    return GT_OK;
}


GT_STATUS  prvTgfPacketAnalyzerTestAddExpectedResultFieldsToStage
(
    IN GT_U32 *numOfHits,
    IN GT_U32 *numOfSampleFields,
    IN GT_U32 index,
    IN GT_U32 expectedHits,
    IN GT_U32 expectedHitFieldsAmount,
    IN GT_U32 totalStages
)
{
    if(index>=totalStages)
    {
        PRV_UTF_LOG2_MAC("(index[%d]>=totalStages[%d])",index,totalStages);
        return GT_OUT_OF_RANGE;
    }

    numOfHits[index] = expectedHits;
    numOfSampleFields[index] = expectedHitFieldsAmount;

    return GT_OK;
}





GT_VOID  prvTgfPacketAnalyzerEgressAggregatorCatchPort
(
    GT_U32 targetPortIndex
)
{
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArr = NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     *fieldsArr = NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *fieldsValueArr = NULL;
    GT_U32                                                   numberOfStages;
    GT_U32                                                   numberOfFields;
    GT_CHAR_PTR                                              ruleName="Egress Q";
    /*expected result*/
    GT_U32                                                   numOfMatchedStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              *matchedStages=NULL;
    GT_U32                                                   *numOfHitsArr=NULL;
    GT_U32                                                   *numOfSampleFieldsArr=NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC                **sampleFieldsValueArr=NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC                *sampleFieldsValue=NULL;
    GT_U32                                                   i;

    GT_U32                                                   stageIterator=0;
    GT_U32                                                   fieldIterator=0;
    GT_U32                                                   expectedStageIterator=0;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                      perStageFields[10];
    GT_U32                                                   perStageNumOfFields;
    GT_U32                                                   perStageNumOfHits;
    GT_STATUS                                                rc;
    GT_U32                                                   targetPort;
    GT_U32                                                   byteCount = 94;


    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             requiredStages[] =
        {
            CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E
        };

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     requiredFileds[] =
        {   CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E
        };

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN_E |UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5X_E | UTF_AC5P_E | UTF_HARRIER_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_ALDRIN_E |UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5X_E | UTF_AC5P_E | UTF_HARRIER_E)) )


    targetPort = prvTgfPortsArray[targetPortIndex];

    numberOfFields = sizeof(requiredFileds)/sizeof(requiredFileds[0]);
    numberOfStages = sizeof(requiredStages)/sizeof(requiredStages[0]);

    /*Each stage should match*/
    numOfMatchedStages = numberOfStages;

    PRV_UTF_LOG2_MAC("Starting test for %d stages and %d fields\n",numberOfStages,numberOfFields);



    /* array allocation */
    stagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*numberOfStages);
    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*numberOfFields);
    fieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numberOfFields);

    /*Each stage should match for one time*/
    matchedStages =(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*numberOfStages);
    numOfHitsArr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*numberOfStages);
    numOfSampleFieldsArr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*numberOfStages);
    sampleFieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC**)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)*numberOfStages);
    for (i=0; i<numberOfStages; i++)
    {
        sampleFieldsValueArr[i] = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numberOfFields);
    }
    sampleFieldsValue = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numberOfFields*numberOfStages);


    if(stagesArr&&fieldsArr&&fieldsValueArr&&matchedStages&&numOfHitsArr&&numOfSampleFieldsArr&&sampleFieldsValueArr&&sampleFieldsValue)
    {
        /*input*/
        for(i=0;i<numberOfStages;i++)
        {
            stagesArr[i] = requiredStages[i];
        }

        for(i=0;i<numberOfFields;i++)
        {
             fieldsArr[i] = requiredFileds[i];
             fieldsValueArr[i].fieldName = requiredFileds[i];
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            byteCount = byteCount - 4 ;/*CRC is not included in byte count in SIP_6*/
        }
        PRV_UTF_LOG2_MAC("\nTry to catch target port %d , byte count  %d \n", targetPort,byteCount);

        /*CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E*/
        fieldsValueArr[0].data[0] = byteCount;
        fieldsValueArr[0].msk[0] =  0xFFFF;

        fieldsValueArr[1].data[0] = targetPort;
        fieldsValueArr[1].msk[0] =  0xFFFF;

        /* Set Packet Analyzer test configuration */
        prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValueArr,&numberOfStages,numberOfFields,ruleName);

        /* Generate traffic*/
        for(i=0;i<4;i++)
        {
            prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE,GT_TRUE,i);
        }

        sampleFieldsValue[0].data[0] = byteCount;
        sampleFieldsValue[0].fieldName = requiredFileds[0];

        sampleFieldsValue[1].data[0] = targetPort;
        sampleFieldsValue[1].fieldName = requiredFileds[1];

         /*output*/
         for(stageIterator=0,expectedStageIterator=0;stageIterator<numberOfStages;stageIterator++)
         {
             PRV_UTF_LOG2_MAC("\nInfo Stage %d = %s\n", stageIterator,paStageStrArr[stagesArr[stageIterator]]);

             perStageNumOfFields=numberOfFields;

             prvTgfPacketAnalyzerLogicalKeyFieldsPerStageGet(stagesArr[stageIterator],&perStageNumOfFields,perStageFields);

             for(fieldIterator=0;fieldIterator<perStageNumOfFields;fieldIterator++)
             {
                if (perStageFields[fieldIterator] > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E)
                {
                    rc = GT_FAIL ;
                    /* exit cleanly */
                    PRV_UTF_LOG1_MAC("unvalid field value got  %d \n",perStageFields[fieldIterator]);
                    goto exit_cleanly_lbl;
                }

                 PRV_UTF_LOG2_MAC("  Info Expect hit for field %d  %s\n", fieldIterator, paFieldStrArr[perStageFields[fieldIterator]]);
             }

             if(perStageNumOfFields==0)
             {
                 PRV_UTF_LOG0_MAC("  Info Expect no hit\n");
                 continue;
             }

             sampleFieldsValueArr[expectedStageIterator][0] = sampleFieldsValue[0];
             sampleFieldsValueArr[expectedStageIterator][1] = sampleFieldsValue[1];
             perStageNumOfHits=3;
#ifdef ASIC_SIMULATION
             perStageNumOfHits=0;
#endif

             rc = prvTgfPacketAnalyzerTestAddExpectedResultStage(matchedStages,
                 expectedStageIterator,stagesArr[stageIterator],numberOfStages);
              UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                 "prvTgfPacketAnalyzerTestAddExpectedResultStage: %d", prvTgfDevNum);

              rc = prvTgfPacketAnalyzerTestAddExpectedResultFieldsToStage(numOfHitsArr,numOfSampleFieldsArr,
                expectedStageIterator,perStageNumOfHits,perStageNumOfFields,numberOfStages);
              UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                 "prvTgfPacketAnalyzerTestAddExpectedResultStage: %d", prvTgfDevNum);

              expectedStageIterator++;
         }

        /* Check Packet Analyzer test Results */
        prvTgfPacketAnalyzerGeneralResultsGet(matchedStages,
                                             numOfMatchedStages,
                                             numberOfFields,
                                             numOfHitsArr,
                                             numOfSampleFieldsArr,
                                             sampleFieldsValueArr);

         /* Restore Packet Analyzer test Configurations */
         prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);

exit_cleanly_lbl:

         cpssOsFree(stagesArr);
         cpssOsFree(fieldsArr);
         cpssOsFree(fieldsValueArr);
         cpssOsFree(matchedStages);
         cpssOsFree(numOfHitsArr);
         cpssOsFree(numOfSampleFieldsArr);
         for (i=0; i<numberOfStages; i++)
            cpssOsFree(sampleFieldsValueArr[i]);
         cpssOsFree(sampleFieldsValueArr);
         cpssOsFree(sampleFieldsValue);

     }
     else
     {
        PRV_UTF_LOG0_MAC("Memory allocation failed\n");
        if(stagesArr)cpssOsFree(stagesArr);
        if(fieldsArr)cpssOsFree(fieldsArr);
        if(fieldsValueArr)cpssOsFree(fieldsValueArr);
        if(matchedStages)cpssOsFree(matchedStages);
        if(numOfHitsArr)cpssOsFree(numOfHitsArr);
        if(numOfSampleFieldsArr)cpssOsFree(numOfSampleFieldsArr);
        for (i=0; i<numberOfStages; i++)
            if(sampleFieldsValueArr[i])cpssOsFree(sampleFieldsValueArr[i]);
        if(sampleFieldsValueArr)cpssOsFree(sampleFieldsValueArr);
        if(sampleFieldsValue)cpssOsFree(sampleFieldsValue);
     }


}




GT_VOID  prvTgfPacketAnalyzerIngressAggregatorCatchPort
(
    GT_U32 sendPortIndex
)
{
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArr = NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     *fieldsArr = NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *fieldsValueArr = NULL;
    GT_U32                                                   numberOfStages;
    GT_U32                                                   numberOfFields;
    GT_CHAR_PTR                                              ruleName="Egress Q";
    /*expected result*/
    GT_U32                                                   numOfMatchedStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              *matchedStages=NULL;
    GT_U32                                                   *numOfHitsArr=NULL;
    GT_U32                                                   *numOfSampleFieldsArr=NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC                **sampleFieldsValueArr=NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC                *sampleFieldsValue=NULL;
    GT_U32                                                   i;

    GT_U32                                                   stageIterator=0;
    GT_U32                                                   fieldIterator=0;
    GT_U32                                                   expectedStageIterator=0;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                      perStageFields[10];
    GT_U32                                                   perStageNumOfFields;
    GT_U32                                                   perStageNumOfHits;
    GT_STATUS                                                rc;
    GT_U32                                                   byteCount = 94;


    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             requiredStages[] =
        {
            CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E
        };

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     requiredFileds[] =
        {   CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
        CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E

        };

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN_E |UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5X_E | UTF_AC5P_E | UTF_HARRIER_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_ALDRIN_E |UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5X_E | UTF_AC5P_E | UTF_HARRIER_E)) )

    numberOfFields = sizeof(requiredFileds)/sizeof(requiredFileds[0]);
    numberOfStages = sizeof(requiredStages)/sizeof(requiredStages[0]);

    /*Each stage should match*/
    numOfMatchedStages = numberOfStages;

    PRV_UTF_LOG2_MAC("Starting test for %d stages and %d fields\n",numberOfStages,numberOfFields);



    /* array allocation */
    stagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*numberOfStages);
    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*numberOfFields);
    fieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numberOfFields);

    /*Each stage should match for one time*/
    matchedStages =(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*numberOfStages);
    numOfHitsArr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*numberOfStages);
    numOfSampleFieldsArr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*numberOfStages);
    sampleFieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC**)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)*numberOfStages);
    for (i=0; i<numberOfStages; i++)
    {
        sampleFieldsValueArr[i] = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numberOfFields);
    }
    sampleFieldsValue = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numberOfFields*numberOfStages);


    if(stagesArr&&fieldsArr&&fieldsValueArr&&matchedStages&&numOfHitsArr&&numOfSampleFieldsArr&&sampleFieldsValueArr&&sampleFieldsValue)
    {
        /*input*/
        for(i=0;i<numberOfStages;i++)
        {
            stagesArr[i] = requiredStages[i];
        }

        for(i=0;i<numberOfFields;i++)
        {
             fieldsArr[i] = requiredFileds[i];
             fieldsValueArr[i].fieldName = requiredFileds[i];
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            byteCount = byteCount - 4 ;/*CRC is not included in byte count in SIP_6*/
        }
        PRV_UTF_LOG2_MAC("\nTry to catch ingress port index  %d , byte count  %d \n", sendPortIndex,byteCount);

        /*CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E*/
        fieldsValueArr[0].data[0] = byteCount;
        fieldsValueArr[0].msk[0] =  0xFFFF;

        fieldsValueArr[1].data[0] = prvTgfPortsArray[sendPortIndex];
        fieldsValueArr[1].msk[0] =  0xFFFF;

        /* Set Packet Analyzer test configuration */
        prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValueArr,&numberOfStages,numberOfFields,ruleName);


        prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE,GT_TRUE,sendPortIndex);


        sampleFieldsValue[0].data[0] = byteCount;
        sampleFieldsValue[0].fieldName = requiredFileds[0];

        sampleFieldsValue[1].data[0] = prvTgfPortsArray[sendPortIndex];
        sampleFieldsValue[1].fieldName = requiredFileds[1];
         /*output*/
         for(stageIterator=0,expectedStageIterator=0;stageIterator<numberOfStages;stageIterator++)
         {
             PRV_UTF_LOG2_MAC("\nInfo Stage %d = %s\n", stageIterator,paStageStrArr[stagesArr[stageIterator]]);

             perStageNumOfFields=numberOfFields;

             prvTgfPacketAnalyzerLogicalKeyFieldsPerStageGet(stagesArr[stageIterator],&perStageNumOfFields,perStageFields);

             for(fieldIterator=0;fieldIterator<perStageNumOfFields;fieldIterator++)
             {
                if (perStageFields[fieldIterator] > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_REGULAR_LAST_E)
                {
                    rc = GT_FAIL ;
                    /* exit cleanly */
                    PRV_UTF_LOG1_MAC("unvalid field value got  %d \n",perStageFields[fieldIterator]);
                    goto exit_cleanly_lbl;
                }
                 PRV_UTF_LOG2_MAC("  Info Expect hit for field %d  %s\n", fieldIterator,paFieldStrArr[perStageFields[fieldIterator]]);
             }

             if(perStageNumOfFields==0)
             {
                 PRV_UTF_LOG0_MAC("  Info Expect no hit\n");
                 continue;
             }

             sampleFieldsValueArr[expectedStageIterator][0] = sampleFieldsValue[0];
             sampleFieldsValueArr[expectedStageIterator][1] = sampleFieldsValue[1];
             perStageNumOfHits=1;
             if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
                 perStageNumOfHits=2;/*should be 1 - CPSS-9753  */
#ifdef ASIC_SIMULATION
             perStageNumOfHits=0;
#endif

             rc = prvTgfPacketAnalyzerTestAddExpectedResultStage(matchedStages,
                 expectedStageIterator,stagesArr[stageIterator],numberOfStages);

              UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                 "prvTgfPacketAnalyzerTestAddExpectedResultStage: %d", prvTgfDevNum);


              rc = prvTgfPacketAnalyzerTestAddExpectedResultFieldsToStage(numOfHitsArr,numOfSampleFieldsArr,
                expectedStageIterator,perStageNumOfHits,perStageNumOfFields,numberOfStages);
              UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                 "prvTgfPacketAnalyzerTestAddExpectedResultStage: %d", prvTgfDevNum);

              expectedStageIterator++;
         }

        /* Check Packet Analyzer test Results */
        prvTgfPacketAnalyzerGeneralResultsGet(matchedStages,
                                             numOfMatchedStages,
                                             numberOfFields,
                                             numOfHitsArr,
                                             numOfSampleFieldsArr,
                                             sampleFieldsValueArr);



#ifdef DUMP_MATCH_RESULTS
        prvCpssDxChIdebugPortGroupInterfaceSampleListDump(prvTgfDevNum,ia2ctrl_desc);
#endif
         /* Restore Packet Analyzer test Configurations */
         prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);

exit_cleanly_lbl:
         cpssOsFree(stagesArr);
         cpssOsFree(fieldsArr);
         cpssOsFree(fieldsValueArr);
         cpssOsFree(matchedStages);
         cpssOsFree(numOfHitsArr);
         cpssOsFree(numOfSampleFieldsArr);
         for (i=0; i<numberOfStages; i++)
            cpssOsFree(sampleFieldsValueArr[i]);
         cpssOsFree(sampleFieldsValueArr);
         cpssOsFree(sampleFieldsValue);

     }
     else
     {
        PRV_UTF_LOG0_MAC("Memory allocation failed\n");
        if(stagesArr)cpssOsFree(stagesArr);
        if(fieldsArr)cpssOsFree(fieldsArr);
        if(fieldsValueArr)cpssOsFree(fieldsValueArr);
        if(matchedStages)cpssOsFree(matchedStages);
        if(numOfHitsArr)cpssOsFree(numOfHitsArr);
        if(numOfSampleFieldsArr)cpssOsFree(numOfSampleFieldsArr);
        for (i=0; i<numberOfStages; i++)
            if(sampleFieldsValueArr[i])cpssOsFree(sampleFieldsValueArr[i]);
        if(sampleFieldsValueArr)cpssOsFree(sampleFieldsValueArr);
        if(sampleFieldsValue)cpssOsFree(sampleFieldsValue);
     }



}

static GT_VOID  prvTgfPacketAnalyzerSingleStageSingleFieldTest
(
    GT_U32 expectedValue,
    GT_U32 expectedNumOfHits,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT stage,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         field,
    GT_BOOL                                     invalidateVlan,
    GT_U32                                      customPortIndex,
    GT_BOOL                                     sendTwice
)
{

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *stagesArr = NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     *fieldsArr = NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *fieldsValueArr = NULL;
    GT_U32                                                   numberOfStages;
    GT_U32                                                   numberOfFields;
    GT_CHAR_PTR                                              ruleName="Egress Q";
    /*expected result*/
    GT_U32                                                   numOfMatchedStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT              *matchedStages=NULL;
    GT_U32                                                   *numOfHitsArr=NULL;
    GT_U32                                                   *numOfSampleFieldsArr=NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC                **sampleFieldsValueArr=NULL;
    GT_U32                                                   numberOfFieldsExpectedToMatchPerStage=1;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC                *sampleFieldsValue=NULL;
    GT_U32                                                   i;



    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             requiredStages[] =
        {CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E};

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     requiredFileds[] =
        {CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E};


    numberOfFields = sizeof(requiredFileds)/sizeof(requiredFileds[0]);
    numberOfStages = sizeof(requiredStages)/sizeof(requiredStages[0]);

    /*Each stage should match*/
    numOfMatchedStages = numberOfStages;

    expectedNumOfHits=expectedNumOfHits; /*warning*/
    PRV_UTF_LOG2_MAC("Starting test for %d stages and %d fields\n",numberOfStages,numberOfFields);

    if(stage<CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E)
    {
        requiredStages[0]= stage;
    }


    if(field<CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E)
    {
        requiredFileds[0]= field;
    }



    /* array allocation */
    stagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*numberOfStages);
    fieldsArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*numberOfFields);
    fieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numberOfFields);

    /*Each stage should match for one time*/
    matchedStages =(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*numberOfStages);
    numOfHitsArr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*numberOfStages);
    numOfSampleFieldsArr = (GT_U32*)cpssOsMalloc(sizeof(GT_U32)*numberOfStages);
    sampleFieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC**)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)*numberOfStages);
    for (i=0; i<numberOfStages; i++)
    {
        sampleFieldsValueArr[i] = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numberOfFields);
    }
    sampleFieldsValue = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*numberOfFields);


    if(stagesArr&&fieldsArr&&fieldsValueArr&&matchedStages&&numOfHitsArr&&numOfSampleFieldsArr&&sampleFieldsValueArr&&sampleFieldsValue)
    {
        /*input*/
        for(i=0;i<numberOfStages;i++)
        {
            stagesArr[i] = requiredStages[i];
        }

        for(i=0;i<numberOfFields;i++)
        {
             fieldsArr[i] = requiredFileds[i];
             fieldsValueArr[i].fieldName = requiredFileds[i];
        }

        /*CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E*/
        fieldsValueArr[0].data[0] = expectedValue;
        fieldsValueArr[0].msk[0] = 0xFFFFFFFF;


        /* Set Packet Analyzer test configuration */
        prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValueArr,&numberOfStages,numberOfFields,ruleName);

        switch(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            /*tested stage in mux and needs to be selected */
            if(requiredStages[0]==CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E)
                prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
                                                           CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            /*tested stage in mux and needs to be selected */
            if(requiredStages[0]==CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E)
                prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
                                                           CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E);
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            if(requiredStages[0]==CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E)
                {
                    /*tested stage in mux and needs to be selected */
                    prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
                                                               CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E);
                }
                else if (requiredStages[0]==CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E)
                {
                    /*tested stage in mux and needs to be selected */
                    prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
                                                               CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E);
                }
                else if (requiredStages[0]==CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E)
                {
                    /*tested stage in mux and needs to be selected */
                    prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
                                                               CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E);
                }
                else if (requiredStages[0]==CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E)
                {
                    /*tested stage in mux and needs to be selected */
                    prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
                                                               CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E);
                }
            break;
        default:
            break;
        }

        /* Generate traffic */
        prvTgfPacketAnalyzerUnknownUcTrafficGenerate(invalidateVlan,GT_TRUE,customPortIndex/*3*/);

        if(sendTwice==GT_TRUE)
        {
            prvTgfPacketAnalyzerUnknownUcTrafficGenerate(invalidateVlan,GT_TRUE,3-customPortIndex/*0*/);
        }

        /*output*/


        matchedStages[0]= requiredStages[0];

#ifndef ASIC_SIMULATION
      numOfHitsArr[0] = expectedNumOfHits;
#else
      numOfHitsArr[0] = 0;
#endif
        numOfSampleFieldsArr[0]=1;

        sampleFieldsValueArr[0][0] = sampleFieldsValue[0];
        sampleFieldsValueArr[0][0].data[0] = expectedValue;
        sampleFieldsValueArr[0][0].fieldName = requiredFileds[0];


  /* Check Packet Analyzer test Results */
       prvTgfPacketAnalyzerGeneralResultsGet(matchedStages,
                                             numOfMatchedStages,
                                             numberOfFieldsExpectedToMatchPerStage,
                                             numOfHitsArr,
                                             numOfSampleFieldsArr,
                                             sampleFieldsValueArr);

        /* Restore Packet Analyzer test Configurations */
        prvTgfPacketAnalyzerUnknownUcRestore(GT_TRUE);

        cpssOsFree(stagesArr);
         cpssOsFree(fieldsArr);
         cpssOsFree(fieldsValueArr);
         cpssOsFree(matchedStages);
         cpssOsFree(numOfHitsArr);
         cpssOsFree(numOfSampleFieldsArr);
         for (i=0; i<numberOfStages; i++)
            cpssOsFree(sampleFieldsValueArr[i]);
         cpssOsFree(sampleFieldsValueArr);
         cpssOsFree(sampleFieldsValue);

     }
     else
     {
        PRV_UTF_LOG0_MAC("Memory allocation failed\n");
        if(stagesArr)cpssOsFree(stagesArr);
        if(fieldsArr)cpssOsFree(fieldsArr);
        if(fieldsValueArr)cpssOsFree(fieldsValueArr);
        if(matchedStages)cpssOsFree(matchedStages);
        if(numOfHitsArr)cpssOsFree(numOfHitsArr);
        if(numOfSampleFieldsArr)cpssOsFree(numOfSampleFieldsArr);
        for (i=0; i<numberOfStages; i++)
            if(sampleFieldsValueArr[i])cpssOsFree(sampleFieldsValueArr[i]);
        if(sampleFieldsValueArr)cpssOsFree(sampleFieldsValueArr);
        if(sampleFieldsValue)cpssOsFree(sampleFieldsValue);

     }
}

GT_VOID  prvTgfPacketAnalyzerDebugModeEnableSet
(
    GT_BOOL enable
)
{
    paDebugMode = enable;

}

GT_BOOL  prvTgfPacketAnalyzerDebugModeEnableGet
(
    GT_VOID
)
{
    return paDebugMode;
}

GT_VOID  prvTgfPacketAnalyzerPreQTest
(
    GT_U32 byteCount,
    GT_U32 expectedNumOfHits
)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN_E |UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5X_E | UTF_AC5P_E | UTF_HARRIER_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN_E |UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5X_E | UTF_AC5P_E | UTF_HARRIER_E)))

    prvTgfPacketAnalyzerSingleStageSingleFieldTest(byteCount,expectedNumOfHits,
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
    CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,GT_TRUE,3,GT_FALSE);
}


GT_VOID  prvTgfPacketAnalyzerSip6SanityTest
(
    GT_U32 byteCount,
    GT_U32 srcDevNum,
    GT_U32 queueOffset,
    GT_U32 expectedNumOfHits
)
{
    GT_U32 i,valueToCatch;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             requiredStages[] =
    {   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
         /*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E,*/
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
         CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E
    };

     CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     requiredFileds[] =
        {
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PRIORITY_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
             /*CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PRIORITY_E,*/
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_DEV_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_DEV_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
             CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E
        };



    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E)

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_FALCON_E)))

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        requiredFileds[4]=CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_OFFSET_E ;/*QUEUE_OFFSET replace QUEUE_PRIORITY in  SIP_6*/
    }

    for(i=0;i<sizeof(requiredStages)/sizeof(requiredStages[0]);i++)
    {
        PRV_UTF_LOG3_MAC("\nStarting test for stage %s.Expect  %d hits for field %s\n",
            paStageStrArr[requiredStages[i]],expectedNumOfHits,paFieldStrArr[requiredFileds[i]]);

        switch (requiredStages[i])
        {
            case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E:
            case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E:
                valueToCatch = srcDevNum;
                break;
            case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E:
            case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E:
                valueToCatch = queueOffset;
                break;
            default:
                valueToCatch = byteCount;
                break;
        }

        if(expectedNumOfHits)
        {
             switch (requiredStages[i])
             {
                 case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E:
                 case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E:
                 case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E:
                 case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E:
                 case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E:
                 case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E:
                     expectedNumOfHits = 3;
                     break;
                 default:
                    expectedNumOfHits=1;
                     break;
             }
        }

       if(GT_TRUE==prvTgfPacketAnalyzerDebugModeEnableGet())
       {
        prvWrAppTraceHwAccessEnable(prvTgfDevNum,CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_BOTH_E,GT_TRUE);
       }

        prvTgfPacketAnalyzerSingleStageSingleFieldTest(valueToCatch,expectedNumOfHits,requiredStages[i],
            requiredFileds[i],GT_FALSE,3,GT_FALSE);

        prvTgfPacketAnalyzerSingleStageSingleFieldTest(valueToCatch,expectedNumOfHits,requiredStages[i],
            requiredFileds[i],GT_FALSE,0,GT_FALSE);
    }

}

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralResults2Get
*           function
* @endinternal
*
* @brief   Check Packet Analyzer test Results
*/
GT_VOID prvTgfPacketAnalyzerGeneralResults2Get
(
   IN GT_U32                                                numOfStages,
   IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stagesArr[],
   IN GT_U32                                                numOfHitPerStageArr[],
   IN GT_U32                                                numOfFieldPerStage[],
   IN CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC             *fieldsValuePerStageArr[]
)
{
    GT_STATUS                                               rc = GT_OK;
    GT_U32                                                  offset;
    GT_U32                                                  numOfMatchedStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *matchedStagesArr = NULL;
    GT_U32                                                  numOfHits=0;
    GT_U32                                                  numOfSampleFieldsExp,numOfSampleFieldsGet;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *sampleFieldsValueArr = NULL;
    GT_U32                                                  ii,jj,kk,length;

    PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(globalManagerId);

    PRV_UTF_LOG1_MAC("\n test %d stages:\n " ,numOfStages);
    for (ii=0;ii<numOfStages;ii++)
    {
        PRV_UTF_LOG3_MAC("%d.[%s],\texpect [%d] hit \n " ,
                         ii,paStageStrArr[stagesArr[ii]],numOfHitPerStageArr[ii]);
        if (numOfHitPerStageArr[ii]>0)
        {
            PRV_UTF_LOG1_MAC("%d fields:\n " ,numOfFieldPerStage[ii]);
            for (jj=0;jj<numOfFieldPerStage[ii];jj++)
            {
                if ((fieldsValuePerStageArr[ii][jj].fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E)&&
                    (fieldsValuePerStageArr[ii][jj].fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E )  )
                {
                    PRV_UTF_LOG1_MAC("\tudf field [%d] ,",
                                (fieldsValuePerStageArr[ii][jj].fieldName - CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E ));
                }
                else
                {
                    PRV_UTF_LOG1_MAC("\t[%s] ,",paFieldStrArr[fieldsValuePerStageArr[ii][jj].fieldName]);
                }
                rc = cpssDxChPacketAnalyzerFieldSizeGet(globalManagerId, fieldsValuePerStageArr[ii][jj].fieldName, &length);
                if (rc != GT_OK) {
                    PRV_UTF_LOG1_MAC("\nError : cpssDxChPacketAnalyzerFieldSizeGet rc=%d ",rc);
                    goto exit_cleanly_lbl;
                }
                PRV_UTF_LOG0_MAC("expect ");
                for (kk = 0; kk < LENGTH_TO_NUM_WORDS_MAC(length); kk++) {
                    PRV_UTF_LOG2_MAC("data%d[0x%x] ", kk, fieldsValuePerStageArr[ii][jj].data[kk]);
                }
                PRV_UTF_LOG0_MAC("\n ");
            }
        }
    }
    /* array allocation */
    matchedStagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    if (matchedStagesArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerGeneralResults2Get");
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: disable sampling on all rules bounded to action */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId,PRV_TGF_ACTION_ID_CNS, GT_FALSE);
    if (rc != GT_OK)
    {
        if(matchedStagesArr)cpssOsFree(matchedStagesArr);
        matchedStagesArr=NULL;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR : cpssDxChPacketAnalyzerActionSamplingEnableSet");
        goto exit_cleanly_lbl;
    }

    numOfMatchedStages = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
    rc = cpssDxChPacketAnalyzerRuleMatchStagesGet(globalManagerId,PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS,&numOfMatchedStages,matchedStagesArr);
    if (rc != GT_OK)
    {
        if(matchedStagesArr)cpssOsFree(matchedStagesArr);
        matchedStagesArr=NULL;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerRuleMatchSubStagesGet");
        goto exit_cleanly_lbl;
    }

    offset=0;
    /*check for num of stages got hit*/
    if ((numOfMatchedStages != numOfStages))
    {
        for (ii=0;ii<numOfStages;ii++)
        {
            if (numOfHitPerStageArr[ii]==0)
                offset++;
        }

        if ((numOfStages-offset) != numOfMatchedStages )
        {
            PRV_UTF_LOG2_MAC("\nError: Match on ruleId [%d] in groupId [%d] Failed;",
                         PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS);
            PRV_UTF_LOG0_MAC("\n ERROR :number of Matched Stages not as expected ");
            PRV_UTF_LOG3_MAC("\n(numOfStages-offset) [(%d -%d)] != numOfMatchedStages [%d] ",numOfStages,offset,numOfMatchedStages);
            PRV_UTF_LOG0_MAC("\n matched stages :  ");
            for (ii=0;ii<numOfMatchedStages;ii++)
            {
                PRV_UTF_LOG2_MAC("\n%d. %s ",ii,paStageStrArr[matchedStagesArr[ii]]);
            }
            rc = GT_FAIL;
            goto exit_cleanly_lbl;
        }
    }
    else
        PRV_UTF_LOG1_MAC("\n num Of hitted Stages is as expected [%d] " ,numOfMatchedStages);

    offset=0;
    for (ii = 0; ii < numOfStages; ii++)
    {
        /*expect no hit in stage */
        if (numOfHitPerStageArr[ii]==0)
        {
            offset++;
            PRV_UTF_LOG1_MAC("\nstage [%s]:0 hits, as expected", paStageStrArr[stagesArr[ii]]);
            continue ;
        }
        if (stagesArr[ii]!= (matchedStagesArr[ii-offset]))
        {
            PRV_UTF_LOG1_MAC("\nERROR: expected:  stage %s ",paStageStrArr[stagesArr[ii]]);
            PRV_UTF_LOG1_MAC("\nERROR: received:  stage %s ",paStageStrArr[matchedStagesArr[ii-offset]]);
            rc = GT_FAIL;
            goto exit_cleanly_lbl;
        }

        numOfSampleFieldsExp = numOfFieldPerStage[ii];
        sampleFieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC *)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC) * numOfSampleFieldsExp + 1);
        if (sampleFieldsValueArr == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            PRV_UTF_LOG1_MAC("\nError : sampleFieldsValueArr Malloc failed rc=%d ",rc);
            goto exit_cleanly_lbl;
        }
        cpssOsMemSet(sampleFieldsValueArr, 0, sizeof(sampleFieldsValueArr));
        for (kk=0;kk<numOfSampleFieldsExp;kk++)
        {
            sampleFieldsValueArr[kk].fieldName = fieldsValuePerStageArr[ii][kk].fieldName;
        }
        numOfSampleFieldsGet = numOfSampleFieldsExp;
        rc = cpssDxChPacketAnalyzerStageMatchDataGet(globalManagerId, PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS,
                                                     stagesArr[ii], &numOfHits, &numOfSampleFieldsGet, sampleFieldsValueArr);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("\nError : cpssDxChPacketAnalyzerStageMatchDataGet rc=%d  stage:%s",rc,
                             paStageStrArr[stagesArr[ii]]);
            goto exit_cleanly_lbl;
        }

        if ((numOfSampleFieldsGet != numOfSampleFieldsExp) ||
            (numOfHits != numOfHitPerStageArr[ii])) {
            PRV_UTF_LOG7_MAC("\n Error: Sample data on ruleId [%d] in groupId [%d] Failed;"
                             "\n stage [%s]"
                             "\n GOT numOfSampleFields [%d] numOfHits [%d]"
                             "\n EXP numOfSampleFields [%d] numOfHits [%d]",
                             PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS,
                             paStageStrArr[stagesArr[ii]],
                             numOfSampleFieldsGet, numOfHits,numOfSampleFieldsExp,numOfHitPerStageArr[ii]);
            rc = GT_FAIL;
            goto exit_cleanly_lbl;
        }

        if ((numOfSampleFieldsGet == 0 )&& (numOfHits >0))
        {
            PRV_UTF_LOG3_MAC("\nstage [%s] numOfSampleFieldsGet [%d]: numOfHits [%d] \t\tas expected ",
                             paStageStrArr[stagesArr[ii]], numOfSampleFieldsGet,numOfHits);
            if(sampleFieldsValueArr)cpssOsFree(sampleFieldsValueArr);
            sampleFieldsValueArr=NULL;
            continue;
        }
        /*check data sampled*/
        for (kk=0;kk<numOfSampleFieldsGet;kk++)
        {
            if ((sampleFieldsValueArr[kk].data[0] != fieldsValuePerStageArr[ii][kk].data[0] )||
                (sampleFieldsValueArr[kk].data[1] != fieldsValuePerStageArr[ii][kk].data[1] )||
                (sampleFieldsValueArr[kk].data[2] != fieldsValuePerStageArr[ii][kk].data[2] )||
                (sampleFieldsValueArr[kk].data[3] != fieldsValuePerStageArr[ii][kk].data[3] )   )
            {
                if ((sampleFieldsValueArr[kk].fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) &&
                    (sampleFieldsValueArr[kk].fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E )   )
                {
                    PRV_UTF_LOG1_MAC("\nError: field UDF %d" ,(sampleFieldsValueArr[kk].fieldName-CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E));
                }
                else
                {
                    PRV_UTF_LOG1_MAC("\nError: field %s" ,paFieldStrArr[sampleFieldsValueArr[kk].fieldName]);
                }
                PRV_UTF_LOG0_MAC("\nError sample data not as expected");
                for (jj=0; jj<4 ;jj++)
                {
                    PRV_UTF_LOG3_MAC("\nError data%d got 0x%x , expected 0x%x",jj,
                                 sampleFieldsValueArr[kk].data[jj], fieldsValuePerStageArr[ii][kk].data[jj]);
                }
                rc = GT_FAIL;
                goto exit_cleanly_lbl;
            }
        }

        /*print fields value*/
        PRV_UTF_LOG2_MAC("\nstage [%s] numOfSampleFieldsGet [%d]:",paStageStrArr[stagesArr[ii]], numOfSampleFieldsGet);

        for (jj = 0; jj < numOfSampleFieldsGet; jj++)
        {
            if ((sampleFieldsValueArr[jj].fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) &&
                (sampleFieldsValueArr[jj].fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E )   )
                PRV_UTF_LOG3_MAC("\n%d. sampleField UDF [%d] numOfHits [%d] ", jj + 1, (sampleFieldsValueArr[jj].fieldName-CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E), numOfHits);
            else
                PRV_UTF_LOG3_MAC("\n%d. sampleField [%s] numOfHits [%d] ", jj + 1, paFieldStrArr[sampleFieldsValueArr[jj].fieldName], numOfHits);

            rc = cpssDxChPacketAnalyzerFieldSizeGet(globalManagerId, sampleFieldsValueArr[jj].fieldName, &length);
            if (rc != GT_OK) {
                PRV_UTF_LOG1_MAC("\nError : cpssDxChPacketAnalyzerFieldSizeGet rc=%d ",rc);
                goto exit_cleanly_lbl;
            }

            for (kk = 0; kk < LENGTH_TO_NUM_WORDS_MAC(length); kk++) {
                PRV_UTF_LOG2_MAC("data%d[0x%x] ", kk, sampleFieldsValueArr[jj].data[kk]);
            }
        }
        PRV_UTF_LOG0_MAC("\n");
        if(sampleFieldsValueArr)cpssOsFree(sampleFieldsValueArr);
        sampleFieldsValueArr=NULL;
    }


exit_cleanly_lbl:

    if(matchedStagesArr)cpssOsFree(matchedStagesArr);
    if(sampleFieldsValueArr)cpssOsFree(sampleFieldsValueArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerGeneralResults2Get");
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralResultsByAttributeGet
*           function
* @endinternal
*
* @brief   Check Packet Analyzer test Results by search Attribute for Falcon
*/
GT_VOID prvTgfPacketAnalyzerGeneralResultsByAttributeGet
(
   IN GT_U32                                                numOfStages,
   IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stagesArr[],
   CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC           *searchAttributePtr,
   IN GT_U32                                                numOfHitPerStageArr[],
   IN GT_U32                                                numOfFieldPerStage[],
   IN CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC             *fieldsValuePerStageArr[]
)
{
    GT_STATUS                                               rc = GT_OK;
    GT_U32                                                  offset;
    GT_U32                                                  numOfMatchedStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             *matchedStagesArr = NULL;
    GT_U32                                                  numOfHits=0;
    GT_U32                                                  numOfSampleFieldsExp,numOfSampleFieldsGet;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *sampleFieldsValueArr = NULL;
    GT_U32                                                  ii,jj,kk,uu,length;

    PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(globalManagerId);

    PRV_UTF_LOG1_MAC("\n test %d stages:\n " ,numOfStages);
    for (ii=0;ii<numOfStages;ii++)
    {
        PRV_UTF_LOG3_MAC("%d.[%s],\texpect [%d] hit \n " ,
                         ii,paStageStrArr[stagesArr[ii]],numOfHitPerStageArr[ii]);
        if (numOfHitPerStageArr[ii]>0)
        {
            PRV_UTF_LOG1_MAC("%d fields:\n " ,numOfFieldPerStage[ii]);
            for (jj=0;jj<numOfFieldPerStage[ii];jj++)
            {
                if ((fieldsValuePerStageArr[ii][jj].fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E)&&
                    (fieldsValuePerStageArr[ii][jj].fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E )  )
                {
                    PRV_UTF_LOG1_MAC("\tudf field [%d] ,",
                                (fieldsValuePerStageArr[ii][jj].fieldName - CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E ));
                }
                else
                {
                    PRV_UTF_LOG1_MAC("\t[%s] ,",paFieldStrArr[fieldsValuePerStageArr[ii][jj].fieldName]);
                }
                rc = cpssDxChPacketAnalyzerFieldSizeGet(globalManagerId, fieldsValuePerStageArr[ii][jj].fieldName, &length);
                if (rc != GT_OK) {
                    PRV_UTF_LOG1_MAC("\nError : cpssDxChPacketAnalyzerFieldSizeGet rc=%d ",rc);
                    goto exit_cleanly_lbl;
                }
                PRV_UTF_LOG0_MAC("expect ");
                for (kk = 0; kk < LENGTH_TO_NUM_WORDS_MAC(length); kk++) {
                    PRV_UTF_LOG2_MAC("data%d[0x%x] ", kk, fieldsValuePerStageArr[ii][jj].data[kk]);
                }
                PRV_UTF_LOG0_MAC("\n ");
            }
        }
    }
    /* array allocation */
    matchedStagesArr = (CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT)*CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
    if (matchedStagesArr == NULL)
    {
        rc = GT_OUT_OF_CPU_MEM;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerGeneralResults2Get");
        goto exit_cleanly_lbl;
    }

    /* AUTODOC: disable sampling on all rules bounded to action */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId,PRV_TGF_ACTION_ID_CNS, GT_FALSE);
    if (rc != GT_OK)
    {
        if(matchedStagesArr)cpssOsFree(matchedStagesArr);
        matchedStagesArr=NULL;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR : cpssDxChPacketAnalyzerActionSamplingEnableSet");
        goto exit_cleanly_lbl;
    }

    numOfMatchedStages = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
    rc = cpssDxChPacketAnalyzerRuleMatchStagesGet(globalManagerId,PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS,&numOfMatchedStages,matchedStagesArr);
    if (rc != GT_OK)
    {
        if(matchedStagesArr)cpssOsFree(matchedStagesArr);
        matchedStagesArr=NULL;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerRuleMatchSubStagesGet");
        goto exit_cleanly_lbl;
    }

    offset=0;
    /*check for num of stages got hit*/
    if ((numOfMatchedStages != numOfStages))
    {
        for (ii=0;ii<numOfStages;ii++)
        {
            if (numOfHitPerStageArr[ii]==0)
                offset++;
        }

        if ((numOfStages-offset) != numOfMatchedStages )
        {
            PRV_UTF_LOG2_MAC("\nError: Match on ruleId [%d] in groupId [%d] Failed;",
                         PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS);
            PRV_UTF_LOG0_MAC("\n ERROR :number of Matched Stages not as expected ");
            PRV_UTF_LOG3_MAC("\n(numOfStages-offset) [(%d -%d)] != numOfMatchedStages [%d] ",numOfStages,offset,numOfMatchedStages);
            PRV_UTF_LOG0_MAC("\n matched stages :  ");
            for (ii=0;ii<numOfMatchedStages;ii++)
            {
                PRV_UTF_LOG2_MAC("\n%d. %s ",ii,paStageStrArr[matchedStagesArr[ii]]);
            }
            rc = GT_FAIL;
            goto exit_cleanly_lbl;
        }
    }
    else
        PRV_UTF_LOG1_MAC("\n num Of hitted Stages is as expected [%d] " ,numOfMatchedStages);

    offset=0;
    for (ii = 0; ii < numOfStages; ii++)
    {
        /*expect no hit in stage */
        if (numOfHitPerStageArr[ii]==0)
        {
            offset++;
            PRV_UTF_LOG1_MAC("\nstage [%s]:0 hits, as expected", paStageStrArr[stagesArr[ii]]);
            continue ;
        }
        if (stagesArr[ii]!= (matchedStagesArr[ii-offset]))
        {
            PRV_UTF_LOG1_MAC("\nERROR: expected:  stage %s ",paStageStrArr[stagesArr[ii]]);
            PRV_UTF_LOG1_MAC("\nERROR: received:  stage %s ",paStageStrArr[matchedStagesArr[ii-offset]]);
            rc = GT_FAIL;
            goto exit_cleanly_lbl;
        }

        numOfSampleFieldsExp = numOfFieldPerStage[ii];
        sampleFieldsValueArr = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC *)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
        if (sampleFieldsValueArr == NULL)
        {
            rc = GT_OUT_OF_CPU_MEM;
            PRV_UTF_LOG1_MAC("\nError : sampleFieldsValueArr Malloc failed rc=%d ",rc);
            goto exit_cleanly_lbl;
        }

        numOfSampleFieldsGet = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
        rc = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(globalManagerId,PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS,
                                                                        stagesArr[ii],searchAttributePtr,&numOfHits,&numOfSampleFieldsGet,sampleFieldsValueArr);
        if (rc != GT_OK)
        {
            PRV_UTF_LOG2_MAC("\nError : cpssDxChPacketAnalyzerStageMatchDataGet rc=%d  stage:%s",rc,
                             paStageStrArr[stagesArr[ii]]);
            goto exit_cleanly_lbl;
        }

        if (numOfSampleFieldsExp > numOfSampleFieldsGet)
        {
            PRV_UTF_LOG3_MAC("\n ERROR : stage [%s]"
                             "\n numOfSampleFieldsExp [%d] > numOfSampleFieldsGet [%d] ",
                             paStageStrArr[stagesArr[ii]],
                             numOfSampleFieldsExp, numOfSampleFieldsGet);
            rc = GT_FAIL;
            goto exit_cleanly_lbl;
        }

        /*check only the fields that we want to check*/
        for (jj=0;jj<numOfSampleFieldsExp;jj++)
        {
            for (kk=0;kk<numOfSampleFieldsGet;kk++)
            {
                if (sampleFieldsValueArr[kk].fieldName != fieldsValuePerStageArr[ii][jj].fieldName)
                    continue ;

                if ((sampleFieldsValueArr[kk].data[0] != fieldsValuePerStageArr[ii][jj].data[0] )&&
                    (sampleFieldsValueArr[kk].data[1] != fieldsValuePerStageArr[ii][jj].data[1] )&&
                    (sampleFieldsValueArr[kk].data[2] != fieldsValuePerStageArr[ii][jj].data[2] )&&
                    (sampleFieldsValueArr[kk].data[3] != fieldsValuePerStageArr[ii][jj].data[3] )   )
                {
                    if ((sampleFieldsValueArr[kk].fieldName > PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) &&
                        (sampleFieldsValueArr[kk].fieldName < PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E )   )
                        PRV_UTF_LOG1_MAC("\nError: field UDF %d" ,sampleFieldsValueArr[kk].fieldName);
                    else
                    {
                        PRV_UTF_LOG1_MAC("\nError: field %s" ,paFieldStrArr[sampleFieldsValueArr[kk].fieldName]);
                        PRV_UTF_LOG0_MAC("\nError sample data not as expected");
                        PRV_UTF_LOG2_MAC("\nError data got 0x%x , expected 0x%x",
                                         sampleFieldsValueArr[kk].data[0], fieldsValuePerStageArr[ii][jj].data[0]);
                        rc = GT_FAIL;
                        goto exit_cleanly_lbl;
                    }
                }
                /*print fields value*/
                PRV_UTF_LOG1_MAC("\nstage [%s] :",paStageStrArr[stagesArr[ii]]);

                if ((sampleFieldsValueArr[kk].fieldName >= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E) &&
                    (sampleFieldsValueArr[kk].fieldName <= PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_LAST_E )   )
                    PRV_UTF_LOG3_MAC("\n%d. sampleField UDF [%d] numOfHits [%d] ", jj + 1,
                                     (sampleFieldsValueArr[kk].fieldName-CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E), numOfHits);
                else
                    PRV_UTF_LOG3_MAC("\n%d. sampleField [%s] numOfHits [%d] ", jj + 1, paFieldStrArr[sampleFieldsValueArr[kk].fieldName], numOfHits);

                rc = cpssDxChPacketAnalyzerFieldSizeGet(globalManagerId, sampleFieldsValueArr[kk].fieldName, &length);
                if (rc != GT_OK) {
                    PRV_UTF_LOG1_MAC("\nError : cpssDxChPacketAnalyzerFieldSizeGet rc=%d ",rc);
                    goto exit_cleanly_lbl;
                }

                for (uu = 0; uu < LENGTH_TO_NUM_WORDS_MAC(length); uu++) {
                    PRV_UTF_LOG2_MAC("data%d[0x%x] ", uu, sampleFieldsValueArr[kk].data[uu]);
                }
            }
        }
        PRV_UTF_LOG0_MAC("\n");
        if(sampleFieldsValueArr)cpssOsFree(sampleFieldsValueArr);
        sampleFieldsValueArr=NULL;
    }

exit_cleanly_lbl:

    if(matchedStagesArr)cpssOsFree(matchedStagesArr);
    if(sampleFieldsValueArr)cpssOsFree(sampleFieldsValueArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketAnalyzerGeneralResults2Get");
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralCountersClear
*           function
* @endinternal
*
* @brief   Clear Sampling Data and Counters for packet analyzer rule.
*/
GT_VOID prvTgfPacketAnalyzerGeneralCountersClear
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;

    PRV_CPSS_PACKET_ANALYZER_GET_MANAGER_ID_MAC(globalManagerId);
    /* AUTODOC: clear sampling data and counters for rule in group */
    rc = cpssDxChPacketAnalyzerSampledDataCountersClear(globalManagerId,PRV_TGF_RULE_ID_CNS, PRV_TGF_GROUP_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerSampledDataCountersClear");
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralSetVlan
*           function
* @endinternal
*
* @brief   Replace Vlan content of packet analyzer rule.
*/
GT_VOID prvTgfPacketAnalyzerGeneralSetVlan
(
   GT_U32                 vlan ,
   GT_BOOL                setCustomVlan,
   GT_BOOL                setVlan2system
)
{
    GT_STATUS                                   rc = GT_OK;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC   fieldsValueArr[1];
    if (setVlan2system == GT_FALSE)
    {
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E;
        if (setCustomVlan == GT_TRUE)
            fieldsValueArr[0].data[0] = vlan;
        else
            fieldsValueArr[0].data[0] = PRV_TGF_VLANID_TEST_CNS;

        fieldsValueArr[0].msk[0] = 0xFFFF;

         /* AUTODOC: disable group activation */
        rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId,PRV_TGF_GROUP_ID_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet");

        rc = cpssDxChPacketAnalyzerGroupRuleUpdate(globalManagerId,PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS,1,fieldsValueArr,PRV_TGF_ACTION_ID_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupRuleUpdate");

         /* AUTODOC: enable group activation */
        rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId,PRV_TGF_GROUP_ID_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet");

        /* AUTODOC: enable sampling on all rules bounded to action */
        rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId,PRV_TGF_ACTION_ID_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionSamplingEnableSet");
    }
    else /*config system*/
    {
         /* AUTODOC: create VLAN  */
        rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_TEST_CNS);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
    }
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralChangeBindStatus
*           function
* @endinternal
*
* @brief   Replace bind state for stage.
*/
GT_VOID prvTgfPacketAnalyzerGeneralChangeBindStatus
(
   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT           stage,
   GT_BOOL                                               toBind
)
{
    GT_STATUS                                   rc = GT_OK;

    /* AUTODOC: disable sampling on all rules bounded to action */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId, PRV_TGF_ACTION_ID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionSamplingEnableSet");

     /* AUTODOC: disable group activation */
    rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId, PRV_TGF_GROUP_ID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet");

    if (toBind)
    {
        PRV_UTF_LOG1_MAC("\nBind stage [%s]",paStageStrArr[stage]);
        rc = cpssDxChPacketAnalyzerMuxStageBind(globalManagerId, stage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerMuxStageBind");
    }
    else
    {
        PRV_UTF_LOG1_MAC("\nUnbind stage [%s]",paStageStrArr[stage]);
        rc = cpssDxChPacketAnalyzerMuxStageUnbind(globalManagerId, stage);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerMuxStageUnbind");
    }

    /* AUTODOC: enable group activation */
    rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId, PRV_TGF_GROUP_ID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet");

    /* AUTODOC: enable sampling on all rules bounded to action */
    rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId, PRV_TGF_ACTION_ID_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionSamplingEnableSet");
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralUpdateRule
*           function
* @endinternal
*
* @brief   change field configuration in rule
*/
GT_VOID prvTgfPacketAnalyzerGeneralUpdateRule
(
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               *fieldsValueArr
)
{
    GT_STATUS                                   rc = GT_OK;

         /* AUTODOC: disable group activation */
        rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId,PRV_TGF_GROUP_ID_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet");

        rc = cpssDxChPacketAnalyzerGroupRuleUpdate(globalManagerId,PRV_TGF_RULE_ID_CNS,PRV_TGF_GROUP_ID_CNS,1,fieldsValueArr,PRV_TGF_ACTION_ID_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupRuleUpdate");

         /* AUTODOC: enable group activation */
        rc = cpssDxChPacketAnalyzerGroupActivateEnableSet(globalManagerId,PRV_TGF_GROUP_ID_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerGroupActivateEnableSet");

        /* AUTODOC: enable sampling on all rules bounded to action */
        rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId,PRV_TGF_ACTION_ID_CNS, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionSamplingEnableSet");
}

/**
* @internal GT_VOID prvTgfPacketAnalyzerGeneralEnableSampling
*           function
* @endinternal
*
* @brief   enable/disable sampling
*/
GT_VOID prvTgfPacketAnalyzerGeneralEnableSampling
(
    GT_BOOL           enable
)
{
    GT_STATUS                                   rc = GT_OK;

        /* AUTODOC: enable sampling on all rules bounded to action */
        rc = cpssDxChPacketAnalyzerActionSamplingEnableSet(globalManagerId,PRV_TGF_ACTION_ID_CNS, enable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPacketAnalyzerActionSamplingEnableSet");
}

