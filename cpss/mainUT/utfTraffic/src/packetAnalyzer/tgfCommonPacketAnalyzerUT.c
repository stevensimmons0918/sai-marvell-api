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
* @file tgfCommonPacketAnalyzer.c
*
* @brief Enhanced UTs for CPSS Packet Analyzer
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <trafficEngine/tgfTrafficGenerator.h>

#include <packetAnalyzer/prvTgfPacketAnalyzerDebugUcTraffic.h>
#include <packetAnalyzer/prvTgfPacketAnalyzerDebugManagerReset.h>
#include <packetAnalyzer/prvTgfPacketAnalyzerDebugUdsUdf.h>
#include <packetAnalyzer/prvTgfPacketAnalyzerDebugOverlappingFields.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzer.h>

#include <common/tgfBridgeGen.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PA_ALL_STAGES           CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E

#define PA_EGRESS_STAGES        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,\
                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E


#define PA_EGRESS_STAGES_START_FROM_TXQ   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,\
                                          CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,\
                                          CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,\
                                          CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,\
                                          CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,\
                                          CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,\
                                          CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E

#define TGF_PA_CHECK_XML \
    do\
    {\
        if(prvCpssDxChPacketAnalyzerIsXmlPresent(prvTgfDevNum)==GT_FALSE)\
        {\
            PRV_UTF_LOG0_MAC("XML is not found.Test skipped\n");\
            SKIP_TEST_MAC\
        }\
    }while(0);

#define TGF_ARRAY_ALLOCATION            \
    do {                                \
        /*arr allocation*/              \
        fieldsValuePerStageArr =(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC**)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)*maxNumOfStage);                \
        if (fieldsValuePerStageArr){                                                                                                                                        \
            for (ii = 0; ii < maxNumOfStage; ii++){                                                                                                                         \
                fieldsValuePerStageArr[ii] = (CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC*)cpssOsMalloc(sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*MaxNumberOfFields+1); \
                if (fieldsValuePerStageArr[ii]==NULL){                                                                                  \
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_OUT_OF_CPU_MEM,"\nError: fieldsValuePerStageArr[ii] allocation failed");     \
                    goto exit_cleanly_lbl;                                                                                              \
                }                                                                                                                       \
            }                                                                                                                           \
        }                                                                                                                               \
                                                                                                                                        \
        for (ii = 0; ii < maxNumOfStage; ii++){                                                                                         \
            for (jj = 0; jj < MaxNumberOfFields; jj++)                                                                                  \
            {                                                                                                                           \
                cpssOsMemSet(fieldsValuePerStageArr[ii][jj].data, 0, sizeof(fieldsValuePerStageArr[ii][jj].data));                      \
                cpssOsMemSet(fieldsValuePerStageArr[ii][jj].msk, 0, sizeof(fieldsValuePerStageArr[ii][jj].msk));                        \
            }                                                                                                                           \
        }                                                                                                                               \
    } while (0);

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugUcTraffic)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer basic configuration:
            - create manager
            - add device to manager
            - enable PA on all devices added to manager
            - create ingress logical key with single stage INGRESS_PRE_BRIDGE and single field PACKET_CMD; field mode PARTIAL
            - create group
            - create default action
            - create rule in group bounded to ingress logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. Generate traffic
        3. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect 2 hits for field CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E for stage INGRESS_PRE_BRIDGE;
              with data CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E.
              first hit is for packet transmitted from CPU to SEND_PORT;
              second hit is for packet transmitted from SEND PORT.
        4. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */
    GT_U32                                        maxNumOfStage = 1,MaxNumberOfFields = 1 ;
    GT_U32                                        jj,ii,numOfStages ;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[1];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[1];
    GT_U32                                        numOfHitPerStageArr[1];
    GT_U32                                        numOfFieldPerStage[1];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;
    GT_CHAR_PTR                                   ruleName="packet cmd rule";

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    numOfStages = maxNumOfStage;
    stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E;
    fieldsArr[0]=CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;

    for (ii = 0;ii< numOfStages;ii++ )
    {
        numOfFieldPerStage[ii] = MaxNumberOfFields;
        fieldsValuePerStageArr[ii][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
        fieldsValuePerStageArr[ii][0].data[0] = CPSS_PACKET_CMD_FORWARD_E;
        fieldsValuePerStageArr[ii][0].msk[0] = 0xFFFF;
    }

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValuePerStageArr[0],&maxNumOfStage,MaxNumberOfFields,ruleName);

    /* Generate traffic */
    prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_TRUE,GT_FALSE,0);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=1;
#else
      numOfHitPerStageArr[0]=0;
#endif

    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_TRUE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugUcTrafficTwoStages)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer basic configuration:
            - create manager
            - add device to manager
            - enable PA on all devices added to manager
            - create egress logical key with two stages  - EGRESS_PRE_POLICER
                                                         - EGRESS_PRE_FILTER
                                          and two fields - PACKET_CMD;
                                                         - BYTE_COUNT_E
                                              field mode - ASSIGN_ALL
            - create group
            - create default action
            - create rule in group bounded to egress logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. Generate traffic
        3. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect 1 hit for fields CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E and CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E  ;
        4. clear counters and field validity
        5. set rule with Vlan diffrent from what set in packet
        6. Generate traffic
        7. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect no hits
        8. clear counters and field validity
        9. restore Vlan value as set for packet
        10. Generate traffic
        11. Analyze the results:
             - disable sampling on all rules bounded to action
             - expect 1 hit for fields CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E and CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E  ;
        12. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */

    GT_U32                                        maxNumOfStage = 2,MaxNumberOfFields = 2 ;
    GT_U32                                        jj,ii,numOfStages ;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[2];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[2];
    GT_U32                                        numOfHitPerStageArr[2];
    GT_U32                                        numOfFieldPerStage[2];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;
    GT_CHAR_PTR                                   ruleName="vlan rule";
    GT_U32                                        vlan = 0x131;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( (UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E) )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    numOfStages =maxNumOfStage;
    stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E;
    stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
    fieldsArr[0]= CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E;
    fieldsArr[1]= CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;

    for (ii = 0;ii< numOfStages ;ii++ )
    {
        numOfFieldPerStage[ii] = MaxNumberOfFields;
        fieldsValuePerStageArr[ii][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E;
        fieldsValuePerStageArr[ii][0].data[0] = vlan;
        fieldsValuePerStageArr[ii][0].msk[0] = 0xFFFF;
        fieldsValuePerStageArr[ii][1].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsValuePerStageArr[ii][1].data[0] = 84;
        fieldsValuePerStageArr[ii][1].msk[0] = 0xFFFF;

        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            fieldsValuePerStageArr[ii][1].data[0]= fieldsValuePerStageArr[ii][1].data[0] - 4 ;/*CRC is not included in byte count in SIP_6*/
        }
    }
    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValuePerStageArr[0],&maxNumOfStage,MaxNumberOfFields,ruleName);

    /*set Vlan*/
    prvTgfPacketAnalyzerGeneralSetVlan(0, GT_FALSE, GT_TRUE);

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) {
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E);
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E);
    }

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(4);

#ifndef ASIC_SIMULATION
    numOfHitPerStageArr[0] = 1;
    numOfHitPerStageArr[1] = 3;
#else
    numOfHitPerStageArr[0] = 0;
    numOfHitPerStageArr[1] = 0;
#endif

    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /*set diffrent Vlan in PA rule*/
    prvTgfPacketAnalyzerGeneralSetVlan(1, GT_TRUE, GT_FALSE);

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(4);

    numOfHitPerStageArr[0] = 0;
    numOfHitPerStageArr[1] = 0;

    /* Check Packet Analyzer test Results -no hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /*restore default Vlan in PA rule*/
    prvTgfPacketAnalyzerGeneralSetVlan(0, GT_FALSE, GT_FALSE);

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(4);

#ifndef ASIC_SIMULATION
    numOfHitPerStageArr[0] = 1;
    numOfHitPerStageArr[1] = 3;
#else
    numOfHitPerStageArr[0] = 0;
    numOfHitPerStageArr[1] = 0;
#endif

    /* Check Packet Analyzer test Results -no hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    prvTgfBrgDefVlanEntryInvalidate((GT_U16)vlan);
    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_TRUE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

/*
       1. Set Packet Analyzer basic configuration:
           - create manager
           - add device to manager
           - enable PA on all devices added to manager
           - create egress  logical key with two  stages PRE_TRANSMIT_QUEUE and  PRE_OAM ,
                    two  field LOCAL_DEV_TRG_PHY_PORT and TRG_EPORT; field mode PARTIAL
           - create group
           - create default action
           - create rule in group bounded to egress  logical key
           - clear sampling data and counters for rule in group
           - enable group activation
           - enable sampling on all rules bounded to action
       2. Generate traffic
       3. Analyze the results:
           - disable sampling on all rules bounded to action

           - expect 1 hits for field CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E
                    for stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E;
                    with data 0.


             expect 1 hits for field CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_EPORT_E
                                for stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E;
                                with data 0.

             both hits is for packet transmitted from CPU to SEND_PORT (port 36);

       4. Restore Packet Analyzer basic configuration:
           - delete group
           - delete action
           - remove device from manager
           - delete manager
   */


UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugEgress)
{
#ifndef GM_USED

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
    GT_U32                                                   matchedPortNum = prvTgfPortsArray[2];
    GT_U32                                                   matchedEgressQueueNum=7/*From cpu*/;
    GT_U32                                                   stageIterator=0;
    GT_U32                                                   fieldIterator=0;
    GT_U32                                                   expectedStageIterator=0;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                      perStageFields[10];
    GT_U32                                                   perStageNumOfFields;
    GT_U32                                                   perStageNumOfHits;
    GT_STATUS                                                rc;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             requiredStages[] =
        {PA_EGRESS_STAGES};

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     requiredFileds[] =
        {   CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_EPORT_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PRIORITY_E
        };

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E | UTF_AC5P_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E )))

    TGF_PA_CHECK_XML

    numberOfFields = sizeof(requiredFileds)/sizeof(requiredFileds[0]);
    numberOfStages = sizeof(requiredStages)/sizeof(requiredStages[0]);

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E )
    {
        requiredStages[3] = requiredStages[4];
        requiredStages[4] = requiredStages[7];
        numberOfStages -= 3;
    }


    /*remove E_OAM (mux)and MAC (no support) from list */
    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E )
    {
        requiredStages[3] = requiredStages[4];
        requiredStages[4] = requiredStages[5];
        requiredStages[5] = requiredStages[6];
        numberOfStages -= 2;
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        requiredFileds[2]=CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_OFFSET_E ;/*QUEUE_OFFSET replace QUEUE_PRIORITY in  SIP_6*/
    }

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

        /*CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E*/
        fieldsValueArr[0].data[0] = matchedPortNum;
        fieldsValueArr[0].msk[0] = 0xFFFF;

        /*CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_EPORT_E*/
        fieldsValueArr[1].data[0] = matchedPortNum;
        fieldsValueArr[1].msk[0] = 0xFFFF;

        /*CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_PRIORITY_E*/
        fieldsValueArr[2].data[0] = matchedEgressQueueNum;
        fieldsValueArr[2].msk[0] = 0xFFFF;

        /* Set Packet Analyzer test configuration */
        prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValueArr,&numberOfStages,numberOfFields,ruleName);

        prvTgfBrgFdbNaStormPreventAllSet(GT_TRUE);

        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E )
        {
            prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
                                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E);
        }

        /* Generate traffic*/
        prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE,GT_FALSE,0);


        sampleFieldsValue[0].data[0] = matchedPortNum;
        sampleFieldsValue[0].fieldName = requiredFileds[0];

        sampleFieldsValue[1].data[0] = matchedPortNum;
        sampleFieldsValue[1].fieldName = requiredFileds[1];

        sampleFieldsValue[2].data[0] = matchedEgressQueueNum;
        sampleFieldsValue[2].fieldName = requiredFileds[2];

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
             sampleFieldsValueArr[expectedStageIterator][2] = sampleFieldsValue[2];
#ifndef ASIC_SIMULATION
             perStageNumOfHits = 1;
#else
             perStageNumOfHits = 0;
#endif

            switch(stagesArr[stageIterator])
            {
                /*muxed so no hit*/
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E:
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E:
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E:
                    if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5X_E ) &&
                       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5P_E ) &&
                       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_HARRIER_E)   )
                    perStageNumOfHits=0;
                    break;
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E:
                    if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E )
                    {
                        perStageNumOfHits=0;
                    }
                    /*ac5x have only CPSS_DXCH_PACKET_ANALYZER_FIELD_QUEUE_OFFSET_E under CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E  */
                    if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E )||
                       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E )||
                       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E ) )
                    {
                        sampleFieldsValueArr[expectedStageIterator][0] = sampleFieldsValue[2];
                    }
                    break;
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E:
                    sampleFieldsValueArr[expectedStageIterator][0] = sampleFieldsValue[1];
                    sampleFieldsValueArr[expectedStageIterator][1] = sampleFieldsValue[2];
                    break;
                default:

                    break;
            }

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


exit_cleanly_lbl:
         /* Restore Packet Analyzer test Configurations */
         prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);

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

#else /* ASIC_SIMULATION */
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

/*
       1. Set Packet Analyzer basic configuration:
           - create manager
           - add device to manager
           - enable PA on all devices added to manager
           - create ingress   logical key with one  stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E  ,
                    one  field CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E ; field mode PARTIAL
           - create group
           - create default action
           - create rule in group bounded to egress  logical key
           - clear sampling data and counters for rule in group
           - enable group activation
           - enable sampling on all rules bounded to action
       2. Generate traffic
       3. Analyze the results:
           - disable sampling on all rules bounded to action

           - expect 2  hits for field CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E
                    for stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;
                    with data  equal to test packet mac da. Two packets are due to loopback.

             both hits is for packet transmitted from CPU to SEND_PORT (port 36);

       4. Restore Packet Analyzer basic configuration:
           - delete group
           - delete action
           - remove device from manager
           - delete manager
   */

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugIpcl)
{
#ifndef GM_USED

    GT_U32                                        maxNumOfStage = 1,MaxNumberOfFields = 2 ;
    GT_U32                                        jj,ii,numOfStages ;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[1];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[2];
    GT_U32                                        numOfHitPerStageArr[1];
    GT_U32                                        numOfFieldPerStage[2];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;
    GT_CHAR_PTR                                   ruleName="Ipcl rule";
    GT_U32                                        matchedMacDa = 0x3402;
    GT_U32                                        matchedDip = 0x01010000;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( (UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E) )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    numOfStages =maxNumOfStage;
    stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;
    fieldsArr[0]= CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
    fieldsArr[1]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E;

    for (ii = 0;ii< numOfStages;ii++ )
    {
        numOfFieldPerStage[ii] = MaxNumberOfFields;
        fieldsValuePerStageArr[ii][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        fieldsValuePerStageArr[ii][0].data[0] = matchedMacDa;
        fieldsValuePerStageArr[ii][0].data[1] = 0;
        fieldsValuePerStageArr[ii][0].msk[0] = 0xFFFFFFFF;
        fieldsValuePerStageArr[ii][0].msk[1] = 0xFFFF;
        fieldsValuePerStageArr[ii][1].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E;
        fieldsValuePerStageArr[ii][1].data[0] = matchedDip;
        fieldsValuePerStageArr[ii][1].msk[0] = 0xFFFFFFFF;
    }

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValuePerStageArr[0],&maxNumOfStage,MaxNumberOfFields,ruleName);


    /* Generate traffic */
    prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_TRUE,GT_FALSE,0);

    /*output*/


#ifndef ASIC_SIMULATION
    numOfHitPerStageArr[0] = 2;
#else
    numOfHitPerStageArr[0] = 0;
#endif

    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_TRUE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}


/*
       1. Set Packet Analyzer basic configuration:
           - create manager
           - add device to manager
           - enable PA on all devices added to manager
           - create ingress   logical key with 4  stages
           - create group
           - create default action
           - create rule in group bounded to ingress  logical key
           - clear sampling data and counters for rule in group
           - enable group activation
           - enable sampling on all rules bounded to action
       2. Generate traffic
       3. Analyze the results:
           - disable sampling on all rules bounded to action

           - expect 1  hits for field CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E
                    and CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E

             both hits is for packet transmitted from CPU to SEND_PORT (port 36);

       4. Restore Packet Analyzer basic configuration:
           - delete group
           - delete action
           - remove device from manager
           - delete manager
   */


UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugPacketCommand)
{
#ifndef GM_USED
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
    GT_U32                                                   origSrcPort=prvTgfPortsArray[2]; /* 12 - ALDRIN ,36 ALDRIN2*/
    GT_U32                                                   stageIterator=0;
    GT_U32                                                   fieldIterator=0;
    GT_STATUS                                                rc;
    GT_U32                                                   expectedStageIterator=0;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             requiredStages[] =
        {
            PA_ALL_STAGES
        };

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     requiredFileds[] =
        {
            CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_DEV_E,
            CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E,
             CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E
        };

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     perStageFields[10];
    GT_U32                                                  perStageNumOfFields;
    GT_U32                                                  perStageNumOfHits = 0;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E)) )

    TGF_PA_CHECK_XML

    numberOfFields = sizeof(requiredFileds)/sizeof(requiredFileds[0]);
    numberOfStages = sizeof(requiredStages)/sizeof(requiredStages[0]);

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E )
    {
        requiredStages[2] = requiredStages[3];
        requiredStages[3] = requiredStages[4];
        requiredStages[4] = requiredStages[8];
        requiredStages[5] = requiredStages[9];
        requiredStages[6] = requiredStages[11];
        requiredStages[7] = requiredStages[14];
        numberOfStages -= 7;
    }

    /*remove E_OAM (mux)and MAC (no support) from list */
    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E )
    {
        requiredStages[10] = requiredStages[11];
        requiredStages[11] = requiredStages[12];
        requiredStages[12] = requiredStages[13];
        numberOfStages -= 2;
    }

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

        fieldsValueArr[0].data[0] = 16;
        fieldsValueArr[0].msk[0] = 0xFFFF;

        /*CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_PHY_PORT_OR_TRUNK_ID_E*/
        fieldsValueArr[1].data[0] = origSrcPort;
        fieldsValueArr[1].msk[0] = 0xFFFF;

        /*CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E*/
        fieldsValueArr[2].data[0] = CPSS_PACKET_CMD_DROP_SOFT_E;
        fieldsValueArr[2].msk[0] = 0x3;/*Forward or Soft Drop*/


        /* Set Packet Analyzer test configuration */
        prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValueArr,&numberOfStages,numberOfFields,ruleName);

        /* Generate traffic */
        prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_TRUE,GT_FALSE,0);

        /*Expected result for forward*/

        sampleFieldsValue[0].data[0] = 16;
        sampleFieldsValue[0].fieldName = requiredFileds[0];

        sampleFieldsValue[1].data[0] = origSrcPort;
        sampleFieldsValue[1].fieldName = requiredFileds[1];

        sampleFieldsValue[2].data[0] = CPSS_PACKET_CMD_FORWARD_E;
        sampleFieldsValue[2].fieldName = requiredFileds[2];




        /*Expected result forsoft drop*/

        sampleFieldsValue[3].data[0] = 16;
        sampleFieldsValue[3].fieldName = requiredFileds[0];

        sampleFieldsValue[4].data[0] = origSrcPort;
        sampleFieldsValue[4].fieldName = requiredFileds[1];

        sampleFieldsValue[5].data[0] = CPSS_PACKET_CMD_DROP_SOFT_E;
        sampleFieldsValue[5].fieldName = requiredFileds[2];


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
            sampleFieldsValueArr[expectedStageIterator][2] = sampleFieldsValue[2];
            perStageNumOfHits=0;

            switch(stagesArr[stageIterator])
            {
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E:

                if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E) ||
                    (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ||
                    (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ||
                    (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ||
                    (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) )
                {
                    sampleFieldsValueArr[expectedStageIterator][0] = sampleFieldsValue[3];
                    sampleFieldsValueArr[expectedStageIterator][1] = sampleFieldsValue[4];
                    sampleFieldsValueArr[expectedStageIterator][2] = sampleFieldsValue[5];
                    perStageNumOfHits = 1;
                }
                    break;
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E:
                    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
                    {
                        sampleFieldsValueArr[expectedStageIterator][0] = sampleFieldsValue[3];
                        sampleFieldsValueArr[expectedStageIterator][1] = sampleFieldsValue[4];
                        sampleFieldsValueArr[expectedStageIterator][2] = sampleFieldsValue[5];
                        perStageNumOfHits=1;
                    }
                    break;
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E:
                    if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ||
                       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ||
                       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) )
                    {
                        sampleFieldsValueArr[expectedStageIterator][0] = sampleFieldsValue[3];
                        sampleFieldsValueArr[expectedStageIterator][1] = sampleFieldsValue[4];
                        sampleFieldsValueArr[expectedStageIterator][2] = sampleFieldsValue[5];
                        perStageNumOfHits = 1;
                    }
                    else
                    {
                        perStageNumOfHits=0;
                    }
                    break;
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E:
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E:
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E:
                    perStageNumOfHits=0;
                    break;
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E:
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E:
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E:
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E:
                    if((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ||
                       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ||
                       (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) )
                    {
                        sampleFieldsValueArr[expectedStageIterator][0] = sampleFieldsValue[3];
                        sampleFieldsValueArr[expectedStageIterator][1] = sampleFieldsValue[4];
                        sampleFieldsValueArr[expectedStageIterator][2] = sampleFieldsValue[5];
                        perStageNumOfHits = 1;
                    }
                    else
                    {
                        perStageNumOfHits=0;
                    }
                    break;
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E:
                    perStageNumOfHits=0;
                    break;
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E:
                case CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E:
                    sampleFieldsValueArr[expectedStageIterator][0] = sampleFieldsValue[3];
                    sampleFieldsValueArr[expectedStageIterator][1] = sampleFieldsValue[4];
                    sampleFieldsValueArr[expectedStageIterator][2] = sampleFieldsValue[5];
                    perStageNumOfHits=1;
                    break;
                default:
                    perStageNumOfHits=1;
                    break;
            }

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
exit_cleanly_lbl:
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

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}


/*
       1. Set Packet Analyzer basic configuration:
           - create manager
           - add device to manager
           - enable PA on all devices added to manager
           - create ingress   logical key with 4  stages
           - create group
           - create default action
           - create rule in group bounded to ingress  logical key
           - clear sampling data and counters for rule in group
           - enable group activation
           - enable sampling on all rules bounded to action
             Set rule to catch CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E and CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E

       2. Generate traffic from 4 different ports (each one is a different iteration) ,each time try to catch different egress port
       3. Analyze the results:
           - disable sampling on all rules bounded to action

           - expect 3  hits for field CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E =94
                    and CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E

             3 hit s because from CPU packet is bigger then 94 due to from cpu overhead

       4. Restore Packet Analyzer basic configuration:
           - delete group
           - delete action
           - remove device from manager
           - delete manager
   */

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugEgressAggregation)
{
#ifndef GM_USED


    GT_U32 i;

    TGF_PA_CHECK_XML


    for(i=0;i<4;i++)
    {
        prvTgfPacketAnalyzerEgressAggregatorCatchPort(i);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}


/*
       1. Set Packet Analyzer basic configuration:
           - create manager
           - add device to manager
           - enable PA on all devices added to manager
           - create ingress   logical key with 4  stages
           - create group
           - create default action
           - create rule in group bounded to ingress  logical key
           - clear sampling data and counters for rule in group
           - enable group activation
           - enable sampling on all rules bounded to action
             Set rule to catch CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E and CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E

       2. Generate traffic from  one port (repeat 4 times) ,each time try to catch different ingress port
       3. Analyze the results:
           - disable sampling on all rules bounded to action

           - expect 1  hits for field CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E =94
                    and CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E


       4. Restore Packet Analyzer basic configuration:
           - delete group
           - delete action
           - remove device from manager
           - delete manager
   */

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugIngressAggregation)
{


#ifndef GM_USED

    GT_U32 i;

    TGF_PA_CHECK_XML

    for(i=0;i<4;i++)
    {
        prvTgfPacketAnalyzerIngressAggregatorCatchPort(i);
    }
#else
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

/*
       1. Set Packet Analyzer basic configuration:
           - create manager
           - add device to manager
           - enable PA on all devices added to manager
           - create ingress   logical key with one  stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E  ,
                    one  field CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E ; field mode PARTIAL
           - create group
           - create default action
           - create rule in group bounded to egress  logical key (once byte count=94 and  once byte count=13)
           - clear sampling data and counters for rule in group
           - enable group activation
           - enable sampling on all rules bounded to action
       2. Generate traffic
       3. Analyze the results:
           - disable sampling on all rules bounded to action

           - expect 1  hits for field CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E
                    for stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E;
                    with data  equal to 94
        expect 0  hits for field CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E
                    for stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E;
                   with data  equal to 13

             both hits is for packet transmitted from CPU to SEND_PORT (port 36);

       4. Restore Packet Analyzer basic configuration:
           - delete group
           - delete action
           - remove device from manager
           - delete manager
   */

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugPreQ)
{
#ifndef GM_USED

    GT_U32  byteCount = 94;

    TGF_PA_CHECK_XML

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        byteCount -= 4;/*CRC is not included in SIP_6*/
    }

    prvTgfPacketAnalyzerPreQTest(byteCount,1);
    prvTgfPacketAnalyzerPreQTest(13,0);
#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}


UTF_TEST_CASE_MAC(tgfPacketAnalyzerSip6Sanity)
{
#ifndef GM_USED
    TGF_PA_CHECK_XML

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_FALCON_E); /*test run ~540 sec */

    prvTgfPacketAnalyzerSip6SanityTest(90,16,7,1);
    prvTgfPacketAnalyzerSip6SanityTest(13,17,3,0);
#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugBindUnbind)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer configuration:
            - create manager
            - add device to manager
            - enable PA on all devices added to manager
            - create key with two stages  - INGRESS_PRE_ROUTER_E ,INGRESS_PRE_OAM_E
                              and 1 field - BYTE_COUNT_E
                              field mode  - ASSIGN_ALL
            - create group
            - create default action
            - create rule in group bounded to logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. Generate traffic
        3. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for stage router and no hit in oam (muxed)  ;
        4. clear counters and field validity
        5. unbind router
        6. Generate traffic
        7. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect no hits in router and oam
        8. clear counters and field validity
        9. bind router
        10. Generate traffic
        11. Analyze the results:
             - disable sampling on all rules bounded to action
             - expect hit for stage router and no hit in oam (muxed)  ;
        12. clear counters and field validity
        13. unbind router
        14. bind oam
        15. Generate traffic
        16. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hits oam and not hits in router
        17. clear counters and field validity
        18. unbind oam
        19. bind router
        20. Generate traffic
        21. Analyze the results:
             - disable sampling on all rules bounded to action
             - expect hit for stage router and no hit in oam (muxed)  ;

        22. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */

    GT_U32                                        maxNumOfStage = 2,MaxNumberOfFields = 1 ;
    GT_U32                                        jj,ii,numOfStages ,expectedHitNum;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[2];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[1];
    GT_U32                                        numOfHitPerStageArr[2];
    GT_U32                                        numOfFieldPerStage[2];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;
    GT_CHAR_PTR                                   ruleName="bind unbind rule";

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( (UTF_ALDRIN_E |UTF_ALDRIN2_E | UTF_FALCON_E |UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_ALDRIN_E |UTF_ALDRIN2_E | UTF_FALCON_E |UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E )) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    switch(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily)
    {
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E;
        stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
        break;
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
        stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E;
        break;
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E;
        stagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E;
        break;
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E;
        stagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E;
        break;
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
        stagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E;
        break;
    default:
        break;
    }

    expectedHitNum = 2;
    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E ||
        PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E ||
        PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E   )
    {
        expectedHitNum = 4;
    }
    expectedHitNum = expectedHitNum; /*in case of simulation ,remove warnings */
    fieldsArr[0]= CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E ;

    numOfStages = maxNumOfStage;
    for (ii = 0;ii< numOfStages;ii++ )
    {
        numOfFieldPerStage[ii] = 1;
        fieldsValuePerStageArr[ii][0].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsValuePerStageArr[ii][0].data[0] = 80;
        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            fieldsValuePerStageArr[ii][0].data[0]= fieldsValuePerStageArr[ii][0].data[0] - 4 ;/*CRC is not included in byte count in SIP_6*/
        }
    }
    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValuePerStageArr[0],&maxNumOfStage,MaxNumberOfFields,ruleName);

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

    PRV_UTF_LOG0_MAC("\n stages status:");
    PRV_UTF_LOG2_MAC("\n stage %s bound (default)"
                     "\n stage %s not bound (default)",paStageStrArr[stagesArr[0]],paStageStrArr[stagesArr[1]]);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=expectedHitNum;
      numOfHitPerStageArr[1]=0;
#else
      numOfHitPerStageArr[0]=0;
      numOfHitPerStageArr[1]=0;
#endif

    /* Check Packet Analyzer test Results - hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /*unbind router stage */
    prvTgfPacketAnalyzerGeneralChangeBindStatus(stagesArr[0], GT_FALSE);

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

    PRV_UTF_LOG0_MAC("\n stages status:");
    PRV_UTF_LOG2_MAC("\n stage %s not bound"
                     "\n stage %s not bound (default)",paStageStrArr[stagesArr[0]],paStageStrArr[stagesArr[1]]);

    numOfHitPerStageArr[0] = 0;
    numOfHitPerStageArr[1] = 0;

    /* Check Packet Analyzer test Results -no hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /*bind router stage*/
    prvTgfPacketAnalyzerGeneralChangeBindStatus(stagesArr[0], GT_TRUE);

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

    PRV_UTF_LOG0_MAC("\n stages status:");
    PRV_UTF_LOG2_MAC("\n stage %s bound"
                     "\n stage %s not bound (default)",paStageStrArr[stagesArr[0]],paStageStrArr[stagesArr[1]]);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=expectedHitNum;
      numOfHitPerStageArr[1]=0;
#else
      numOfHitPerStageArr[0]=0;
      numOfHitPerStageArr[1]=0;
#endif

    /* Check Packet Analyzer test Results - hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /*unbind router stage , bind oam stage */
    prvTgfPacketAnalyzerGeneralSwitchMuxStages(stagesArr[0],stagesArr[1]);

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

    PRV_UTF_LOG0_MAC("\n stages status:");
    PRV_UTF_LOG2_MAC("\n stage %s not bound"
                     "\n stage %s bound ",paStageStrArr[stagesArr[0]],paStageStrArr[stagesArr[1]]);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=0;
      numOfHitPerStageArr[1]=expectedHitNum;
#else
      numOfHitPerStageArr[0]=0;
      numOfHitPerStageArr[1]=0;
#endif

    /* Check Packet Analyzer test Results - hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /*unbind oam stage bind router stage*/
    prvTgfPacketAnalyzerGeneralSwitchMuxStages(stagesArr[1],stagesArr[0]);

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

    PRV_UTF_LOG0_MAC("\n stages status:");
    PRV_UTF_LOG2_MAC("\n stage %s bound"
                     "\n stage %s not bound",paStageStrArr[stagesArr[0]],paStageStrArr[stagesArr[1]]);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=expectedHitNum;
      numOfHitPerStageArr[1]=0;
#else
      numOfHitPerStageArr[0]=0;
      numOfHitPerStageArr[1]=0;
#endif

    /* Check Packet Analyzer test Results - hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugManagerReset)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer configuration:
            - create manager
            - add device to manager
            - enable PA on all devices added to manager
            - create key with one stage  - INGRESS_PRE_ROUTER_E
                              and 3 field - BYTE_COUNT_E ,LOCAL_DEV_SRC_PORT,MAC_SA
                              field mode  - ASSIGN_ALL
            - create group
            - create default action
            - create rule in group bounded to logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. Generate traffic with packet 1
        3. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for stage router ;
        4. restore manager to default
        5. Generate traffic with packet 2
        6. Set Packet Analyzer configuration:
            - add device to manager
            - enable PA on all devices added to manager
            - create key with one stage  - INGRESS_PRE_ROUTER_E
                              and 3 field - BYTE_COUNT_E ,LOCAL_DEV_SRC_PORT,MAC_SA
                              field mode  - ASSIGN_ALL
            - create group
            - create default action
            - create rule in group bounded to logical key
            - enable group activation
            - enable sampling on all rules bounded to action
        7. Generate traffic with packet 1
        8. Analyze the results:
             - disable sampling on all rules bounded to action
             - expect hit 1 for stage router;

        9. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */

    GT_U32                                        maxNumOfStage = 1,MaxNumberOfFields = 3 ;
    GT_U32                                        jj,ii,numOfStages ;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[1];
    GT_U32                                        numOfHitPerStageArr[1];
    GT_U32                                        numOfFieldPerStage[1];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( (UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E) )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerDebugManagerResetConfigSet(GT_FALSE);

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E )
    {
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
    }

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugManagerResetTrafficGenerate(1);

    numOfStages = maxNumOfStage;
    stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
    for (ii = 0;ii< numOfStages;ii++ )
    {
        numOfFieldPerStage[ii] = 3;
        fieldsValuePerStageArr[ii][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E;
        fieldsValuePerStageArr[ii][0].data[0] = prvTgfPortsArray[0]; /* 0 - ALDRIN and ALDRIN2*/
        fieldsValuePerStageArr[ii][1].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsValuePerStageArr[ii][1].data[0] = 80;
        fieldsValuePerStageArr[ii][2].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        fieldsValuePerStageArr[ii][2].data[0] = 0x55;
        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            fieldsValuePerStageArr[ii][1].data[0]= fieldsValuePerStageArr[ii][1].data[0] - 4 ;/*CRC is not included in byte count in SIP_6*/
        }
    }

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=1;
#else
      numOfHitPerStageArr[0]=0;
#endif

    /* Check Packet Analyzer test Results - hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    PRV_UTF_LOG0_MAC("\n Manager Reset To Default...");

    prvTgfPacketAnalyzerDebugManagerResetToDefaults();

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugManagerResetTrafficGenerate(2);

    PRV_UTF_LOG0_MAC("\n Set Packet Analyzer configuration...");

    /* Set Packet Analyzer test configuration (skip manager create and counter reset */
    prvTgfPacketAnalyzerDebugManagerResetConfigSet(GT_TRUE);

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E )
    {
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
    }

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugManagerResetTrafficGenerate(1);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=1;
#else
      numOfHitPerStageArr[0]=0;
#endif

    /* Check Packet Analyzer test Results - hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugInterfaceReset)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer configuration:
            - create manager
            - add device to manager
            - enable PA on all devices added to manager
            - create key with one stage  - INGRESS_PRE_ROUTER_E
                              and 3 field - BYTE_COUNT_E ,LOCAL_DEV_SRC_PORT,MAC_SA
                              field mode  - ASSIGN_ALL
            - create group
            - create default action
            - create rule in group bounded to logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. Generate traffic with packet 1
        3. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for stage router ;
        4. Generate traffic with packet 2
        5. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect not hit;
        6. reset router interface
        7. Generate traffic with packet 2
        8. Analyze the results:
             - disable sampling on all rules bounded to action
             - expect hit 1 for stage router;

        9. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */

    GT_U32                                        maxNumOfStage = 1,MaxNumberOfFields = 3 ;
    GT_U32                                        jj,ii,numOfStages ;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[1];
    GT_U32                                        numOfHitPerStageArr[1];
    GT_U32                                        numOfFieldPerStage[1];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( (UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerDebugManagerResetConfigSet(GT_FALSE);

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E )
    {
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
    }

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugManagerResetTrafficGenerate(1);

    numOfStages = maxNumOfStage;
    stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
    for (ii = 0;ii< numOfStages;ii++ )
    {
        numOfFieldPerStage[ii] = 3;
        fieldsValuePerStageArr[ii][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E;
        fieldsValuePerStageArr[ii][0].data[0] = prvTgfPortsArray[0]; /* 0 - ALDRIN and ALDRIN2*/
        fieldsValuePerStageArr[ii][1].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsValuePerStageArr[ii][1].data[0] = 80;
        fieldsValuePerStageArr[ii][2].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        fieldsValuePerStageArr[ii][2].data[0] = 0x55;
        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
        {
            fieldsValuePerStageArr[ii][1].data[0]= fieldsValuePerStageArr[ii][1].data[0] - 4 ;/*CRC is not included in byte count in SIP_6*/
        }
    }

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=1;
#else
      numOfHitPerStageArr[0]=0;
#endif

    /* Check Packet Analyzer test Results - hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    prvTgfPacketAnalyzerDebugInterfaceReset(GT_FALSE);

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugManagerResetTrafficGenerate(2);

    numOfHitPerStageArr[0]=0;

    /* Check Packet Analyzer test Results -no hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    PRV_UTF_LOG0_MAC("\n Reset Interface");

    prvTgfPacketAnalyzerDebugInterfaceReset(GT_TRUE);

    /* Generate traffic */
    prvTgfPacketAnalyzerDebugManagerResetTrafficGenerate(2);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=2;
#else
      numOfHitPerStageArr[0]=0;
#endif

    fieldsValuePerStageArr[0][2].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
    fieldsValuePerStageArr[0][2].data[0] = 0x66;
    /* Check Packet Analyzer test Results - hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugUds)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer configuration:
            - create manager
            - add device to manager
            - create uds and udf
            - enable PA on all devices added to manager
            - create key with one stage   - UDS_0 - l2i2mt_na_bus :Interface between L2I to FDB
                              and 1 field - MAC_SA
                              field mode  - ASSIGN_ALL
            - unbind mux stage and bind uds
            - create group
            - create default action
            - create rule in group bounded to logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. Generate traffic with packet 1
        3. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for uds 0 ;
        4. clear sampling data and counters for rule in group
        5. enable sampling on all rules bounded to action

        6. Generate traffic with packet 1
        7. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect no hit for uds 0  ;
        8. clear sampling data and counters for rule in group
        9. change rule to mac sa 0x1173 and enable sampling
       10. Generate traffic with packet 2
       11. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for stage uds 0 ;
       12. clear sampling data and counters for rule in group
       13. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */

    GT_U32                                        maxNumOfStage = 1,MaxNumberOfFields = 1 ;
    GT_U32                                        jj,ii,numOfStages ;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[1];
    GT_U32                                        numOfHitPerStageArr[1];
    GT_U32                                        numOfFieldPerStage[1];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( (UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerDebugUdsConfigSet(GT_FALSE);

    /* Generate traffic 1*/
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=1;
#else
      numOfHitPerStageArr[0]=0;
#endif
    numOfStages = maxNumOfStage;
    numOfFieldPerStage[0] = 1;
    stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E;
    fieldsValuePerStageArr[0][0].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
    fieldsValuePerStageArr[0][0].data[0] = 0x2112;

    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    prvTgfPacketAnalyzerGeneralCountersClear();

    /*only enable sampling*/
    prvTgfPacketAnalyzerDebugUdsUdfUpdateRule(1,fieldsValuePerStageArr[0],GT_FALSE);

    /* Generate traffic 2 */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

    numOfHitPerStageArr[0]=0;

    /* Check Packet Analyzer test Results -no hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    prvTgfPacketAnalyzerGeneralCountersClear();

    fieldsValuePerStageArr[0][0].data[0]=0x1173;

    /*change rule to mac sa 0x1173 and enable sampling*/
    prvTgfPacketAnalyzerDebugUdsUdfUpdateRule(1,fieldsValuePerStageArr[0],GT_TRUE);

    /* Generate traffic 1*/
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(2);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=1;
#else
      numOfHitPerStageArr[0]=0;
#endif

    /* Check Packet Analyzer test Results - hit*/
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    prvTgfPacketAnalyzerGeneralCountersClear();

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugConcatinatedUds)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer configuration:
            - create manager
            - add device to manager
            - create uds and udf
            - enable PA on all devices added to manager
            - create key with one stage   - UDS_0 - l2i2mt_na_bus :Interface between L2I to FDB
                              and 1 field - MAC_SA
                              field mode  - ASSIGN_ALL
            - unbind mux stage and bind uds
            - create group
            - create default action
            - create rule in group bounded to logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. Generate traffic with packet 1
        3. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for uds 0 ;
        4. clear sampling data and counters for rule in group
        5. enable sampling on all rules bounded to action

        6. Generate traffic with packet 1
        7. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect no hit for uds 0  ;
        8. clear sampling data and counters for rule in group
        9. change rule to mac sa 0x1173 and enable sampling
       10. Generate traffic with packet 2
       11. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for stage uds 0 ;
       12. clear sampling data and counters for rule in group
       13. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */

    GT_U32                                       numOfHitArr[2]  ;
    GT_U32                                       expSample[]   = {0x2112};

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E)

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E)))

    TGF_PA_CHECK_XML

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerDebugUdsConfigSet(GT_TRUE);

    /* Generate traffic 1*/
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

#ifndef ASIC_SIMULATION
      numOfHitArr[0]=1;
#else
      numOfHitArr[0]=0;
#endif

    /* Check Packet Analyzer test Results - hit  */
    prvTgfPacketAnalyzerDebugUdsResultsGet(numOfHitArr,expSample,GT_TRUE);

    prvTgfPacketAnalyzerGeneralCountersClear();

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);


#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugUdf)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer configuration:
            - create manager
            - add device to manager
            - create uds and udf
            - enable PA on all devices added to manager
            - create key with one stage   - CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E
                              and 3 fields - MAC_DA
                                           - UDF 0 -is_bc
                                           - UDF 1 -ttl
            - create group
            - create default action
            - create rule in group bounded to logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. Generate traffic with packet 3
        3. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for all fields ;
        4. clear sampling data and counters for rule in group
        5. update rule for diffrent mac da - "is bc" field is set only when mac da is all 1
        6. Generate traffic with packet 1
        7. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for all fields  ;
        8. clear sampling data and counters for rule in group
        9. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */

    GT_U32                                        maxNumOfStage = 1,MaxNumberOfFields = 3 ;
    GT_U32                                        jj,ii,numOfStages ;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[1];
    GT_U32                                        numOfHitPerStageArr[1];
    GT_U32                                        numOfFieldPerStage[1];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;


    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( (UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerDebugUdfConfigSet();

    /* Generate traffic 3 ipv4 + mac da all 1*/
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(3);

    numOfFieldPerStage[0] = MaxNumberOfFields;
    numOfStages = maxNumOfStage;
    stagesArr[0]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0] = 2 ;
#else
      numOfHitPerStageArr[0] = 0 ;
#endif

    fieldsValuePerStageArr[0][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
    fieldsValuePerStageArr[0][0].data[0] = 0xffffffff;
    fieldsValuePerStageArr[0][0].data[1] = 0xffff;
    fieldsValuePerStageArr[0][1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E;
    fieldsValuePerStageArr[0][1].data[0] = 1;
    fieldsValuePerStageArr[0][2].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_1_E;
    fieldsValuePerStageArr[0][2].data[0] = 0x40;

    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    prvTgfPacketAnalyzerGeneralCountersClear();

    fieldsValuePerStageArr[0][0].data[0] = 0x3402;
    fieldsValuePerStageArr[0][0].data[1] = 0x0;
    fieldsValuePerStageArr[0][1].data[0] = 0;
    fieldsValuePerStageArr[0][2].data[0] = 0;

    prvTgfPacketAnalyzerDebugUdsUdfUpdateRule(3,fieldsValuePerStageArr[0], GT_TRUE);

    /* Generate traffic send uc packet 1 */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0] = 2 ;
#else
      numOfHitPerStageArr[0] = 0 ;
#endif

    /* Check Packet Analyzer test Results*/
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    prvTgfPacketAnalyzerGeneralCountersClear();

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);


#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugOverlappingFields)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer configuration:
            - create manager
            - add device to manager
            - create udf (overlapping ipv4/ipv6 fields)
            - enable PA on all devices added to manager
            - create key with one stage   - CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E
                              and 3 fields - CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E
                                           - CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E
                                           - UDF 0 -arp_sip
            - create group
            - create default action
            - try to create rule in group bounded to logical key with the 3 overlapping fields - fail
            - create rule in group bounded to logical key with the field CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. Generate traffic with packet 1 (ipv4)
        3. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for all fields ;
        4. clear sampling data and counters for rule in group
        5. delete old rule and set rule with field ipv6
        6. Generate traffic with packet 2 (ipv6)
        7. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for all fields  ;
        8. clear sampling data and counters for rule in group
        9. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */

    GT_U32                                        maxNumOfStage = 1,MaxNumberOfFields = 1 ;
    GT_U32                                        jj,ii,numOfStages ;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[1];
    GT_U32                                        numOfHitPerStageArr[1];
    GT_U32                                        numOfFieldPerStage[1];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;


    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( (UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerDebugOverlappingFieldsConfigSet();

    /* Generate traffic ipv4*/
    prvTgfPacketAnalyzerDebugOverlappingFieldsTrafficGenerate(1);

    numOfFieldPerStage[0] = 1;
    numOfStages = maxNumOfStage;
    stagesArr[0]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E;

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0] = 2 ;
#else
      numOfHitPerStageArr[0] = 0 ;
#endif

    fieldsValuePerStageArr[0][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;
    fieldsValuePerStageArr[0][0].data[0] = 0x01030204;

    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    prvTgfPacketAnalyzerGeneralCountersClear();

    cpssOsMemSet(fieldsValuePerStageArr[0][0].data, 0, sizeof(fieldsValuePerStageArr[0][0].data));
    cpssOsMemSet(fieldsValuePerStageArr[0][0].msk, 0, sizeof(fieldsValuePerStageArr[0][0].msk));

    fieldsValuePerStageArr[0][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E;
    fieldsValuePerStageArr[0][0].data[0] = 0x10112222;
    fieldsValuePerStageArr[0][0].data[3] = 0x22223303;
    fieldsValuePerStageArr[0][0].msk[0] = 0xffffffff;
    fieldsValuePerStageArr[0][0].msk[3] = 0xffffffff;

    prvTgfPacketAnalyzerDebugOverlappingFieldsChangeField(1,fieldsValuePerStageArr[0]);

    /* Generate traffic ipv6*/
    prvTgfPacketAnalyzerDebugOverlappingFieldsTrafficGenerate(2);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0] = 2 ;
#else
      numOfHitPerStageArr[0] = 0 ;
#endif

    /* Check Packet Analyzer test Results*/
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    prvTgfPacketAnalyzerGeneralCountersClear();

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);


#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugEgressMuxStagesCheck)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer basic configuration:
            - create manager
            - add device to manager
            - enable PA on all devices added to manager
            - create egrss logical key with 5 muxed stages and 2 fields LOCAL_DEV_SRC_PORT and LOCAL_DEV_TRG_PHY_PORT
              field mode PARTIAL
            - create group
            - create default action
            - create rule in group bounded to ingress logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. bind stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E;
            - enable group activation
            - enable sampling on all rules bounded to action
        3. Generate traffic
        4. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect 1 hit in each stage
        5. bind stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
            - enable group activation
            - enable sampling on all rules bounded to action
        6. Generate traffic
        7. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect 1 hit in each stage
        8. bind stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
            - enable group activation
            - enable sampling on all rules bounded to action
        9. Generate traffic
       10. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect 1 hit in each stage

       11.  bind stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E;
            - enable group activation
            - enable sampling on all rules bounded to action
       12. Generate traffic
       13. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect 1 hit in each stage

       14. bind stage CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E;
            - enable group activation
            - enable sampling on all rules bounded to action
       15. Generate traffic
       16. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect 1 hit in each stage
       17. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */
    GT_U32                                        maxNumOfStage = 6,MaxNumberOfFields = 2 ;
    GT_U32                                        jj,ii,numOfStages,numOfFields=2,stageIndex=0;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[6];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[4];

    GT_U32                                        numOfHitPerStageArr[6]={0};
    GT_U32                                        numOfFieldPerStage[6];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;
    GT_CHAR_PTR                                   ruleName="egress mux rule";

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E ))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
        {
            numOfStages = 4 ;
        }
        else
        {
            numOfStages = maxNumOfStage;
        }
    }
    else
        numOfStages = 5;
    stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E;
    stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
    stagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
    stagesArr[3]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E;
    stagesArr[4]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E;
    stagesArr[5]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E; /*only sip6*/

    fieldsArr[0]=CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E;
    fieldsArr[1]=CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E;

    stageIndex=stageIndex; /*remove warnings */
    for (ii = 0;ii< numOfStages;ii++ )
    {
        numOfFieldPerStage[ii] = numOfFields;
        fieldsValuePerStageArr[ii][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E;
        fieldsValuePerStageArr[ii][0].data[0] = prvTgfPortsArray[1]; /*12 - ALDRIN2 ,8 - ALDRIN */
        fieldsValuePerStageArr[ii][0].msk[0] = 0xFFFF;
        fieldsValuePerStageArr[ii][1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E;
        fieldsValuePerStageArr[ii][1].data[0] = prvTgfPortsArray[2]; /*36 - ALDRIN2 , 12 - ALDRIN*/
        fieldsValuePerStageArr[ii][1].msk[0] = 0xFFFF;
    }

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValuePerStageArr[0],&numOfStages,numOfFields,ruleName);

    switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily) {
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E);
        break;
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E);
        break;
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E);
        numOfFieldPerStage[4] = 0;
        break;
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
        numOfFieldPerStage[4] = 0;
    default:
        break;
    }

    /* Generate traffic */
    prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE,GT_TRUE,1);

#ifndef ASIC_SIMULATION
    switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily) {
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        numOfHitPerStageArr[0] = 1;
        numOfHitPerStageArr[1] = 0;
        numOfHitPerStageArr[2] = 0;
        numOfHitPerStageArr[3] = 0;
        numOfHitPerStageArr[4] = 0;
        break;
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        numOfHitPerStageArr[0] = 1;
        numOfHitPerStageArr[1] = 1; /*not mux*/
        numOfHitPerStageArr[2] = 0;
        numOfHitPerStageArr[3] = 0;
        numOfHitPerStageArr[4] = 0;
        break;
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
        numOfHitPerStageArr[0] = 1;
        numOfHitPerStageArr[1] = 1; /*not mux*/
        numOfHitPerStageArr[2] = 0;
        numOfHitPerStageArr[3] = 0;
        numOfHitPerStageArr[4] = 4; /*not mux*/
        numOfHitPerStageArr[5] = 0;
        break;
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
        numOfHitPerStageArr[0] = 1;
        numOfHitPerStageArr[1] = 1; /*not mux*/
        numOfHitPerStageArr[2] = 1; /*not mux*/
        numOfHitPerStageArr[3] = 1; /*not mux*/
        numOfHitPerStageArr[4] = 4; /*not mux*/
        numOfHitPerStageArr[5] = 0;
        break;
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        numOfHitPerStageArr[0] = 1;
        numOfHitPerStageArr[1] = 0;
        numOfHitPerStageArr[2] = 1; /*not mux*/
        numOfHitPerStageArr[3] = 1; /*not mux*/
        break;
    default:
        break;
    }
#else
    for (ii = 0; ii < numOfStages; ii++) numOfHitPerStageArr[ii] = 0;
#endif

    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily) {
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E);
        break;
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E);
        break;
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
        prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
                                                   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E);
        break;
    default:
        break;
    }

    /* Generate traffic */
    prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE, GT_TRUE, 1);

#ifndef ASIC_SIMULATION
        numOfHitPerStageArr[stageIndex] = 0;
        switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily) {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            stageIndex++;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            stageIndex += 2;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            stageIndex += 5;
            break;
        default:
            break;
        }
        numOfHitPerStageArr[stageIndex] = 1;
#endif

        /* Check Packet Analyzer test Results- hit */
        prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

        /* Clear counter */
        prvTgfPacketAnalyzerGeneralCountersClear();

        switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily) {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
                                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
                                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E);
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            goto finish_test_lbl;
            break;
        default:
            break;
        }

        /* Generate traffic */
        prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE, GT_TRUE, 1);

#ifndef ASIC_SIMULATION
        numOfHitPerStageArr[stageIndex] = 0;
        stageIndex++;
        numOfHitPerStageArr[stageIndex] = 1;
#endif

        /* Check Packet Analyzer test Results- hit */
        prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);
        /* Clear counter */
        prvTgfPacketAnalyzerGeneralCountersClear();

        switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily) {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
                                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
                                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E);
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
                                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E);
            break;
        default:
            break;
        }

        /* Generate traffic */
        prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE, GT_TRUE, 1);


#ifndef ASIC_SIMULATION
    numOfHitPerStageArr[stageIndex] = 0;
    switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily) {
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        stageIndex++;
        break;
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
        stageIndex += 2;
        break;
    default:
        break;
    }
    numOfHitPerStageArr[stageIndex] = 1;
#endif


        /* Check Packet Analyzer test Results- hit */
        prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

        /* Clear counter */
        prvTgfPacketAnalyzerGeneralCountersClear();

        switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily) {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            prvTgfPacketAnalyzerGeneralSwitchMuxStages(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
                                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            goto finish_test_lbl;
            break;
        default:
            break;
        }

        /* Generate traffic */
        prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE, GT_TRUE, 1);

#ifndef ASIC_SIMULATION
        numOfHitPerStageArr[stageIndex] = 0;
        stageIndex++;
        numOfHitPerStageArr[stageIndex] = 2;  /*should be 1 - CPSS-9753  */
#endif

        /* Check Packet Analyzer test Results- hit */
        prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

finish_test_lbl:
    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_TRUE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugNoFields)
{
#ifndef GM_USED

    GT_U32                                        maxNumOfStage = 17,MaxNumberOfFields = 0 ;
    GT_U32                                        jj,ii,numOfStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[17];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[1];

    GT_U32                                        numOfHitPerStageArr[17]={0};
    GT_U32                                        numOfFieldPerStage[17];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;
    GT_CHAR_PTR                                   ruleName="no fields rule";


    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC((UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_ALDRIN_E|UTF_ALDRIN2_E|UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    numOfStages = maxNumOfStage ;
    /*set default valid stages*/
    switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily) {
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:

        stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E;
        stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;
        stagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E;
        stagesArr[3]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E;
        stagesArr[4]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;
        stagesArr[5]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E;
        stagesArr[6]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E;
        stagesArr[7]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
        numOfStages = 8;
        break;
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E;
        stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;
        stagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E;
        stagesArr[3]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
        stagesArr[4]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;
        stagesArr[5]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E;
        stagesArr[6]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E;
        stagesArr[7]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E;
        stagesArr[8]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
        stagesArr[9]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
        numOfStages = 10;
        break;
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
        stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E;
        stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;
        stagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E;
        stagesArr[3]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
        stagesArr[4]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E;
        stagesArr[5]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;
        stagesArr[6]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E;
        stagesArr[7]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E;
        stagesArr[8]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E;
        stagesArr[9]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
        stagesArr[10]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
        stagesArr[11]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E;
        stagesArr[12]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E;
        numOfStages = 13;
        break;
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
        stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E;
        stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;
        stagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E;
        stagesArr[3]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
        stagesArr[4]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E;
        stagesArr[5]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;
        stagesArr[6]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E;
        stagesArr[7]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E;
        stagesArr[8]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E;
        stagesArr[9]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E;
        stagesArr[10]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E;
        stagesArr[11]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
        stagesArr[12]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
        stagesArr[13]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E;
        stagesArr[14]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E;
        stagesArr[15]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E;
        stagesArr[16]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E;
        numOfStages = 17;
        break;
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E;
        stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;
        stagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E;
        stagesArr[3]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
        stagesArr[4]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E;
        stagesArr[5]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;
        stagesArr[6]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E;
        stagesArr[7]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E;
        stagesArr[8]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E;
        stagesArr[9]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E;
        stagesArr[10]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
        stagesArr[11]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
        stagesArr[12]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E;
        stagesArr[13]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E;
        stagesArr[14]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E;
        numOfStages = 15;
        break;
    default:
        break;
    }

    for (ii = 0;ii< numOfStages;ii++ )
    {
        numOfFieldPerStage[ii] = MaxNumberOfFields; /*0*/
    }

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValuePerStageArr[0],&numOfStages,MaxNumberOfFields,ruleName);


    /* Generate traffic */
    prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE, GT_TRUE, 1);

#ifndef ASIC_SIMULATION

    switch (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily) {
    case CPSS_PP_FAMILY_DXCH_ALDRIN_E:

        for (ii = 0; ii < 5; ii++) {
            numOfHitPerStageArr[ii] = 2;
        }
        for (ii = 5; ii < numOfStages; ii++) {
            numOfHitPerStageArr[ii] = 4;
        }

        numOfHitPerStageArr[0] = 4;/*should be 2 - CPSS-9753   */
        break;
    case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        for (ii = 0; ii < 6; ii++) {
            numOfHitPerStageArr[ii] = 2;
        }
        for (ii = 6; ii < numOfStages; ii++) {
            numOfHitPerStageArr[ii] = 4;
        }
        break;
    case CPSS_PP_FAMILY_DXCH_FALCON_E:
        for (ii = 0; ii < 8; ii++) {
            numOfHitPerStageArr[ii] = 2;
        }
        for (ii = 8; ii < numOfStages; ii++) {
            numOfHitPerStageArr[ii] = 4;
        }
        break;
    case CPSS_PP_FAMILY_DXCH_AC5P_E:
    case CPSS_PP_FAMILY_DXCH_AC5X_E:
    case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        for (ii = 0; ii < 9; ii++) {
            numOfHitPerStageArr[ii] = 2;
        }
        for (ii = 9; ii < numOfStages; ii++) {
            numOfHitPerStageArr[ii] = 4;
        }
        break;
    default:
        break;
    }
#else
    for (ii = 0; ii < numOfStages; ii++) numOfHitPerStageArr[ii] = 0;
#endif

    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();


    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_TRUE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugPacketTrace)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer basic configuration:
            - create manager
            - add device to manager
            - enable PA on all devices added to manager
            - create logical key with 11 stages and 2 fields MAC_SA and PACKET_TRACE(macth to 0)
              field mode PARTIAL
            - create group
            - create default action
            - create rule in group bounded to ingress logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        3. Generate traffic with mac sa not match to rule
        4. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect no hits in stages with mac sa field and hit on all other
        5. update rule for packet track to match 1
        6. Generate traffic with mac sa not match to rule
        7. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect 0 hits in all stages
        9. Generate traffic with mac sa match to rule
       10. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect 1 hit in each stage
       11. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */
    GT_U32                                        maxNumOfStage = 11,MaxNumberOfFields = 2 ;
    GT_U32                                        jj,ii,numOfStages,numOfFields=2;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[11];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[2];

    GT_U32                                        numOfHitPerStageArr[11]={0};
    GT_U32                                        numOfFieldPerStage[11];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;
    GT_CHAR_PTR                                   ruleName="egress mux rule";

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E)

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_FALCON_E|UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E)))

    TGF_PA_CHECK_XML

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {
        maxNumOfStage = numOfStages = 10;
    }

    TGF_ARRAY_ALLOCATION

    numOfStages = maxNumOfStage;
    stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;
    stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E;
    stagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
    stagesArr[3]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E;
    stagesArr[4]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;
    stagesArr[5]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E;
    stagesArr[6]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E;
    stagesArr[7]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E;
    stagesArr[8]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
    stagesArr[9]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
    stagesArr[10]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E;

    fieldsArr[0]=CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_TRACE_E;
    fieldsArr[1]=CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;

    for (ii = 0;ii< numOfStages;ii++ )
    {
        numOfFieldPerStage[ii] = 1;
        fieldsValuePerStageArr[ii][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_TRACE_E;
        fieldsValuePerStageArr[ii][0].data[0] = 0;
        fieldsValuePerStageArr[ii][0].msk[0] = 0x0;
        fieldsValuePerStageArr[ii][1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        fieldsValuePerStageArr[ii][1].data[0] = 0x2112;
        fieldsValuePerStageArr[ii][1].msk[0] = 0xFFFF;
    }

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValuePerStageArr[0],&maxNumOfStage,numOfFields,ruleName);

    numOfFieldPerStage[0] = 1;
    fieldsValuePerStageArr[0][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
    fieldsValuePerStageArr[0][0].data[0] = 0x2112;
    fieldsValuePerStageArr[0][0].msk[0] = 0xFFFF;

    PRV_UTF_LOG0_MAC("\n Sending a packet 1 with mac sa that doesn't match the rule  ");
    /* Generate traffic packet 2  */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(2);

#ifndef ASIC_SIMULATION

    numOfHitPerStageArr[0]=0;
    numOfHitPerStageArr[1]=0;
    numOfHitPerStageArr[2]=0;

    for (ii = 3; ii < 7; ii++)            numOfHitPerStageArr[ii] = 2;
    for (ii = 7; ii < numOfStages; ii++)  numOfHitPerStageArr[ii] = 4;
#else
    for (ii = 0; ii < numOfStages; ii++) numOfHitPerStageArr[ii] = 0;
#endif



    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    PRV_UTF_LOG0_MAC("\n enable packet trace");
    for (ii = 1; ii < numOfStages; ii++)
    {
        fieldsValuePerStageArr[ii][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_TRACE_E;
        fieldsValuePerStageArr[ii][0].data[0] = 1;
        fieldsValuePerStageArr[ii][0].msk[0] = 0xf;
    }

    /*update rule*/
    prvTgfPacketAnalyzerGeneralUpdateRule(fieldsValuePerStageArr[1]);

    PRV_UTF_LOG0_MAC("\n Sending a packet 1 with mac sa that doesn't match the rule  ");
    /* Generate traffic packet 2 */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(2);

    for (ii = 0; ii < numOfStages; ii++) numOfHitPerStageArr[ii] = 0;

    /* Check Packet Analyzer test Results- no hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    prvTgfPacketAnalyzerGeneralEnableSampling(GT_TRUE);

    PRV_UTF_LOG0_MAC("\n Sending a packet 1 with mac sa that match the rule  ");
    /* Generate traffic packet 1  */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

#ifndef ASIC_SIMULATION

    for (ii = 0; ii < 7; ii++)
    {
        numOfFieldPerStage[ii] = 1;
        numOfHitPerStageArr[ii] = 2;
    }
    for (ii = 7; ii < numOfStages; ii++)
    {
        numOfFieldPerStage[ii] = 1;
        numOfHitPerStageArr[ii] = 4;
    }
#endif

    numOfFieldPerStage[1] = 2;
    numOfFieldPerStage[2] = 2;

    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_TRUE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugHitByAttribute)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer basic configuration:
            - create manager
            - add device to manager
            - enable PA on all devices added to manager
            - create logical key with 3 stage and 2 fields MAC_SA (mask 0) and BYTE_COUNT (mask 0)
              field mode PARTIAL
            - create group
            - create default action
            - create rule in group bounded to ingress logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        3. Generate traffic
        4. Analyze the results:
            - disable sampling on all rules bounded to action
            - check hit by search attribute (port)
            - expect hit , src port 63 and bytecount 110
        6. Generate traffic
        7. Analyze the results:
            - disable sampling on all rules bounded to action
            - check hit by search attribute (port)
            - expect hit src port 111 and bytecount 90
        8.repeat 3-7 with search attribute (core)
        9. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */
    GT_U32                                        maxNumOfStage = 3,MaxNumberOfFields = 2 ;
    GT_U32                                        jj,ii,numOfStages,numOfFields=2;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[3];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[3];

    GT_U32                                              numOfHitPerStageArr[3]={0};
    GT_U32                                              numOfFieldPerStage[3];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC           **fieldsValuePerStageArr=NULL;
    GT_CHAR_PTR                                         ruleName="egress mux rule";
    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC      searchAttribute;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E | UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~(UTF_FALCON_E | UTF_AC5X_E | UTF_HARRIER_E|UTF_AC5P_E) ))

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    cpssOsMemSet(&searchAttribute, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC));
    numOfStages = maxNumOfStage;

    stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E;
    stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
    stagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E;

    fieldsArr[0]=CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E;
    fieldsArr[1]=CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;

    for (ii = 0;ii< numOfStages;ii++ )
    {
        numOfFieldPerStage[ii] = 2;
        fieldsValuePerStageArr[ii][0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E;
        fieldsValuePerStageArr[ii][0].data[0] = 0;
        fieldsValuePerStageArr[ii][0].msk[0] = 0x0;
        fieldsValuePerStageArr[ii][1].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsValuePerStageArr[ii][1].data[0] = 0;
        fieldsValuePerStageArr[ii][1].msk[0] = 0x0;
    }

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerGeneralConfigSet(stagesArr,fieldsArr,fieldsValuePerStageArr[0],&maxNumOfStage,numOfFields,ruleName);

    /* Generate traffic */
    prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE,GT_TRUE,2);

#ifndef ASIC_SIMULATION

    for (ii = 0; ii < numOfStages; ii++) numOfHitPerStageArr[ii] = 1;
#else
    for (ii = 0; ii < numOfStages; ii++) numOfHitPerStageArr[ii] = 0;
#endif

    searchAttribute.type = CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_PORT_E;
    searchAttribute.portNum = 63;
    for (ii = 0; ii < numOfStages; ii++){
        fieldsValuePerStageArr[ii][0].data[0] = 63;
        fieldsValuePerStageArr[ii][1].data[0] = 110;
    }

    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResultsByAttributeGet(numOfStages, stagesArr,&searchAttribute ,
                                                     numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();
    /* enable sampling */
    prvTgfPacketAnalyzerGeneralEnableSampling(GT_TRUE);
    /* Generate traffic */
    prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE,GT_TRUE,2);

    searchAttribute.portNum = prvTgfPortsArray[2];
    for (ii = 0; ii < numOfStages; ii++){
        fieldsValuePerStageArr[ii][0].data[0] = prvTgfPortsArray[2];
        fieldsValuePerStageArr[ii][1].data[0] = 90;
    }

    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResultsByAttributeGet(numOfStages, stagesArr,&searchAttribute ,
                                                     numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);


    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E )
    {
        searchAttribute.type = CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_CORE_E;
        searchAttribute.tile = 0;
        searchAttribute.pipe = 1;
        searchAttribute.dp = 0;
        for (ii = 0; ii < numOfStages; ii++){
            fieldsValuePerStageArr[ii][0].data[0] = 63;
            fieldsValuePerStageArr[ii][1].data[0] = 110;
        }

        /* Clear counter */
        prvTgfPacketAnalyzerGeneralCountersClear();
        /* enable sampling */
        prvTgfPacketAnalyzerGeneralEnableSampling(GT_TRUE);
        /* Generate traffic */
        prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE,GT_TRUE,2);

        /* Check Packet Analyzer test Results- hit */
        prvTgfPacketAnalyzerGeneralResultsByAttributeGet(numOfStages, stagesArr,&searchAttribute ,
                                                         numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

        for (ii = 0; ii < numOfStages; ii++){
            fieldsValuePerStageArr[ii][0].data[0] = prvTgfPortsArray[2];
            fieldsValuePerStageArr[ii][1].data[0] = 90;
        }
        searchAttribute.tile = 1;
        searchAttribute.pipe = 0;
        searchAttribute.dp = 2;

        /* Clear counter */
        prvTgfPacketAnalyzerGeneralCountersClear();
        /* enable sampling */
        prvTgfPacketAnalyzerGeneralEnableSampling(GT_TRUE);
        /* Generate traffic */
        prvTgfPacketAnalyzerUnknownUcTrafficGenerate(GT_FALSE,GT_TRUE,2);

        /* Check Packet Analyzer test Results- hit */
        prvTgfPacketAnalyzerGeneralResultsByAttributeGet(numOfStages, stagesArr,&searchAttribute ,
                                                         numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    }
    /* Clear counter */
    prvTgfPacketAnalyzerGeneralCountersClear();

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_TRUE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

UTF_TEST_CASE_MAC(tgfPacketAnalyzerDebugAsymmetricalInstances)
{
#ifndef GM_USED
      /*
        1. Set Packet Analyzer configuration:
            - create manager
            - add device to manager
            - create uds and udf
            - enable PA on all devices added to manager
            - create key with one stage   - UDS_0 - PDS_2_QFC_DEQ :Interface that connected to 2
              Asymmetrical Instances (tile0_pipe1_txqs_macro_i3_pipe0_1,tile0_pipe1_txqs_macro_i3_pipe1_0)
                              and 1 field - src_port
                              field mode  - ASSIGN_ALL
            - unbind mux stage and bind uds
            - create group
            - create default action
            - create rule in group bounded to logical key
            - clear sampling data and counters for rule in group
            - enable group activation
            - enable sampling on all rules bounded to action
        2. Generate traffic with packet 1
        3. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect hit for uds ;
        4. clear sampling data and counters for rule in group
        5. update the rule not to match the packet
        6. enable sampling on all rules bounded to action

        7. Generate traffic with packet 1
        8. Analyze the results:
            - disable sampling on all rules bounded to action
            - expect no hit for uds  ;
        8. clear sampling data and counters for rule in group
        9. Restore Packet Analyzer basic configuration:
            - delete group
            - delete action
            - remove device from manager
            - delete manager
    */

    GT_U32                                        maxNumOfStage = 1,MaxNumberOfFields = 1 ;
    GT_U32                                        jj,ii,numOfStages ;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stagesArr[1];
    GT_U32                                        numOfHitPerStageArr[1];
    GT_U32                                        numOfFieldPerStage[1];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     **fieldsValuePerStageArr=NULL;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( (UTF_FALCON_E))

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~(UTF_FALCON_E)) )

    TGF_PA_CHECK_XML

    TGF_ARRAY_ALLOCATION

    /* Set Packet Analyzer test configuration */
    prvTgfPacketAnalyzerDebugAsymmetricalInstancesConfigSet();

    /* Generate traffic 1*/
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

#ifndef ASIC_SIMULATION
      numOfHitPerStageArr[0]=1;
#else
      numOfHitPerStageArr[0]=0;
#endif

    numOfStages = maxNumOfStage;
    numOfFieldPerStage[0] = 1;
    stagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E;
    fieldsValuePerStageArr[0][0].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E;
    fieldsValuePerStageArr[0][0].data[0] = 63;
    fieldsValuePerStageArr[0][0].msk[0] = 0xffff;

    /* Check Packet Analyzer test Results- hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages,stagesArr,numOfHitPerStageArr,numOfFieldPerStage,fieldsValuePerStageArr);

    prvTgfPacketAnalyzerGeneralCountersClear();

    fieldsValuePerStageArr[0][0].data[0] = 0x3;

    /*only enable sampling*/
    prvTgfPacketAnalyzerDebugUdsUdfUpdateRule(1, fieldsValuePerStageArr[0], GT_TRUE);

    /* Generate traffic 2 */
    prvTgfPacketAnalyzerDebugUdsUdfTrafficGenerate(1);

    numOfHitPerStageArr[0] = 0;

    /* Check Packet Analyzer test Results -no hit */
    prvTgfPacketAnalyzerGeneralResults2Get(numOfStages, stagesArr, numOfHitPerStageArr, numOfFieldPerStage, fieldsValuePerStageArr);

    prvTgfPacketAnalyzerGeneralCountersClear();

    /* Restore Packet Analyzer test Configurations */
    prvTgfPacketAnalyzerUnknownUcRestore(GT_FALSE);

exit_cleanly_lbl:

    for (ii=0; ii<maxNumOfStage; ii++)
        if(fieldsValuePerStageArr[ii])cpssOsFree(fieldsValuePerStageArr[ii]);
    if(fieldsValuePerStageArr)cpssOsFree(fieldsValuePerStageArr);

#else /* ASIC_SIMULATION */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

/*
 * Configuration of tgfPacketAnalyzer suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfPacketAnalyzer)

    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugIpcl)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugUcTraffic)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugUcTrafficTwoStages)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugEgress)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugPacketCommand)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugEgressAggregation)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugIngressAggregation)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugPreQ)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerSip6Sanity)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugBindUnbind)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugManagerReset)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugInterfaceReset)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugUds)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugConcatinatedUds)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugUdf)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugOverlappingFields)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugEgressMuxStagesCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugNoFields)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugPacketTrace)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugHitByAttribute)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPacketAnalyzerDebugAsymmetricalInstances)

UTF_SUIT_END_TESTS_MAC(tgfPacketAnalyzer)

