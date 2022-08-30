/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, msk WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssDxChPacketAnalzerUT.c
*
* @brief Unit tests for cpssDxChPacketAnalyzer,
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/packetAnalyzer/cpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/packetAnalyzer/private/prvCpssDxChPacketAnalyzer.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Defines */
#ifdef ASIC_SIMULATION

#ifndef GM_USED
#define UTF_PA_E (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E ) /*WM*/
#else /*GM*/
#define UTF_PA_E (UTF_ALDRIN2_E )
#endif

#else /*BM*/
#define UTF_PA_E (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E )
#endif

#define SIZE_OF_TEST_ARR         27

#define BAD_MANAGER_ID_VALUE     PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_MANAGERS_NUM_CNS +1
#define BAD_KEY_ID_VALUE         PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_LOGICAL_KEY_NUM_CNS +1
#define BAD_GROUP_ID_VALUE       PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_GROUPS_NUM_CNS+1
#define BAD_RULE_ID_VALUE        PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_RULE_NUM_CNS+1
#define BAD_ACTION_ID_VALUE      PRV_CPSS_DXCH_PACKET_ANALYZER_MAX_ACTIONS_NUM_CNS +1

#define GENERIC_MANAGER_ID       9

#define PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(_manager)\
        PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.idebugDir.packetAnalyzerSrc.paMngDB[_manager])

#define UT_PA_CHECK_XML\
    do\
    {\
        if(prvCpssDxChPacketAnalyzerIsXmlPresent(prvTgfDevNum)==GT_FALSE)\
        {\
            PRV_UTF_LOG0_MAC("XML is not found.Test skipped\n");\
            SKIP_TEST_MAC\
        }\
    }while(0);

/*Some tests will be skipped for Falcon until support is ready*/
#define FALCON_SKIPPED_TEST\
    do\
    {\
        if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)==GT_TRUE)\
        {\
            SKIP_TEST_MAC\
        }\
    }while(0);

#ifndef GM_USED

    static CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT utPaAldrinDefaultValidStages[]={
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E
    };

    static CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT utPaAldrin2DefaultValidStages[]={
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E
    };

    static CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT utPaFalconDefaultValidStages[]={
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E
    };

    static CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT utPaHawkDefaultValidStages[]={
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E

    };

    static CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT utPaPhoenixDefaultValidStages[]={
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E

    };

    static CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT utPaHarrierDefaultValidStages[]={
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E,
        CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E
    };
#endif

GT_STATUS genericManagerAndDeviceInit
(
    IN     GT_U8        devNum,
    INOUT  GT_U32      *managerId
)
{
    GT_STATUS                                            rc =GT_OK;

    rc = cpssDxChPacketAnalyzerManagerCreate(*managerId);
    if (rc == GT_OK)
    {
        rc = cpssDxChPacketAnalyzerManagerDeviceAdd(*managerId, devNum);
        if (rc != GT_OK)
        {
            return rc ;
        }

        /*Manager enable*/
        rc = cpssDxChPacketAnalyzerManagerEnableSet(*managerId,GT_TRUE);
    }
    return rc;
}

GT_STATUS genericKeyInit
(
    IN  GT_U32                                                  managerId,
    IN GT_U32                                                   keyId,
    OUT CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC           *keyAttr,
    OUT GT_U32                                                 *numOfStages,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             stagesArr[],
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT    *fieldMode,
    OUT GT_U32                                                 *numOfFields,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     fieldsArr[]
)
{
    GT_STATUS rc = GT_OK ;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT                 udsId;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC               udsInterfaceAttributes ;
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC                udsAttr ;
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC                udfAttr ;
    GT_BOOL                                                     valid ;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                       paMngDbPtr=NULL;

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                         udfId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC                    fieldName;

    rc = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
    if (rc != GT_OK)
    {
        return rc ;
    }
    if (*numOfStages == 0) /*function called from test and not from genericKeyInitSimple */
    {
        cpssOsMemCpy(keyAttr->keyNameArr, "test run : genericKeyInit", sizeof("test run : genericKeyInit"));
    }

    cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
    rc = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&udsInterfaceAttributes);
    if (rc != GT_OK)
    {
        return rc ;
    }
    if (valid == GT_FALSE)
    {
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "har_tx_macro_i0_cell_req0");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"tx2pds_fifo_state");
            udsInterfaceAttributes.interfaceIndex = 0;
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_tx_0_macro_cell_req0");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"tx2pds_fifo_state");
            udsInterfaceAttributes.interfaceIndex = 0;
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "pnx_txd_macro_cell_req0");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"tx2pds_fifo_state");
            udsInterfaceAttributes.interfaceIndex = 0;
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_pipe0_eagle_l2i_ipvx_0_macro_mt_or_shm");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"L2I_2_MT_LEARN");
            udsInterfaceAttributes.interfaceIndex = 1;
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"l2i_ipvx_desc_or_debug_buses_0_8");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"l2i2mt_na_bus");
            udsInterfaceAttributes.interfaceIndex = 2;
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"txq_queue_flow");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"txq_ll2eq_desc");
            udsInterfaceAttributes.interfaceIndex = 1;
            break;
        default:
            return GT_NOT_SUPPORTED;
            break;

        }
        rc = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
        if (rc != GT_OK)
        {
            return rc ;
        }
    }

    cpssOsStrCpy(fieldName.fieldNameArr,"qos_profile");
    cpssOsStrCpy(udfAttr.udfNameArr,"udf 0");
    rc = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfId,&valid,&fieldName);
    if (rc != GT_OK)
    {
        return rc ;
    }
    if (valid == GT_FALSE)
    {
        rc = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfId);
        if (rc != GT_OK)
            return rc ;
    }

    *numOfStages = 5 ;
    stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E ;
    stagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E ;
    stagesArr[2] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
    stagesArr[3] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E ;
    stagesArr[4] = udsId;
    *fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
    *numOfFields = 5 ;
    fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
    fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
    fieldsArr[2] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
    fieldsArr[3] = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
    fieldsArr[4] = udfId;

    rc = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,keyAttr,*numOfStages,stagesArr,*fieldMode,*numOfFields,fieldsArr);

    return rc ;
}

GT_STATUS genericKeyInitSimple
(
    IN  GT_U32                                                  managerId,
    IN GT_U32                                                   keyId
)
{
    GT_STATUS rc = GT_OK ;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC           keyAttr;
    GT_U32                                                 numOfStages=0;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT            stagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT    fieldMode=0;
    GT_U32                                                 numOfFields=0;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                    fieldsArr[SIZE_OF_TEST_ARR];

    cpssOsMemCpy(keyAttr.keyNameArr, "test run : genericKeyInitSimple", sizeof("test run : genericKeyInitSimple"));
    rc = genericKeyInit(managerId,keyId,&keyAttr,&numOfStages,stagesArr,&fieldMode,&numOfFields,fieldsArr);

    return rc ;
}

GT_STATUS genericGroupInit
(
    IN GT_U32             managerId,
    IN GT_U32             groupId
)
{
    GT_STATUS                                        rc =GT_OK;
    CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC   groupAttr;


    cpssOsMemCpy(groupAttr.groupNameArr,"genericGroupInit " ,sizeof("genericGroupInit"));
    rc = cpssDxChPacketAnalyzerGroupCreate(managerId,groupId,&groupAttr);

    return rc;
}

GT_STATUS genericActionInit
(
    IN GT_U32          managerId,
    IN GT_U32          actionId
)
{
    GT_STATUS                               rc =GT_OK;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC    action;


    action.samplingEnable = GT_FALSE ;
    action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E ;
    action.matchCounterEnable = GT_TRUE ;
    action.matchCounterIntThresh.lsb = 0x4;
    action.matchCounterIntThresh.middle =0x1;
    action.matchCounterIntThresh.msb = 0xa;
    action.interruptSamplingEnable = GT_FALSE ;
    action.interruptMatchCounterEnable = GT_FALSE ;
    action.inverseEnable = GT_TRUE ;
    rc = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);

    return rc;
}

GT_STATUS genericRuleInit
(
    IN GT_U32              managerId,
    IN GT_U32              keyId,
    IN GT_U32              groupId,
    IN GT_U32              actionId,
    IN GT_U32              ruleId
)
{

    GT_STATUS                                            rc = GT_OK;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC        ruleAttr;
    GT_U32                                               ruleNumOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC            fieldsValueArr[SIZE_OF_TEST_ARR];

    cpssOsMemCpy(ruleAttr.ruleNameArr,"genericRuleInit" ,sizeof("genericRuleInit"));
    ruleNumOfFields = 4;
    fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
    cpssOsMemSet(fieldsValueArr[0].data, 0, sizeof(fieldsValueArr[0].data));
    cpssOsMemSet(fieldsValueArr[0].msk, 1, sizeof(fieldsValueArr[0].msk));

    fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
    cpssOsMemSet(fieldsValueArr[1].data, 0, sizeof(fieldsValueArr[1].data));
    cpssOsMemSet(fieldsValueArr[1].msk, 1, sizeof(fieldsValueArr[1].msk));

    fieldsValueArr[2].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
    cpssOsMemSet(fieldsValueArr[2].data, 0, sizeof(fieldsValueArr[2].data));
    cpssOsMemSet(fieldsValueArr[2].msk, 1, sizeof(fieldsValueArr[2].msk));

    fieldsValueArr[3].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
    cpssOsMemSet(fieldsValueArr[3].data, 0, sizeof(fieldsValueArr[3].data));
    cpssOsMemSet(fieldsValueArr[3].msk, 1, sizeof(fieldsValueArr[3].msk));

    fieldsValueArr[4].fieldName = PRV_CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_FIRST_E;
    cpssOsMemSet(fieldsValueArr[4].data, 0, sizeof(fieldsValueArr[4].data));
    cpssOsMemSet(fieldsValueArr[4].msk, 1, sizeof(fieldsValueArr[4].msk));

    rc = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);

    return rc;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerManagerCreate
(
    IN  GT_U32                                               managerId
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerManagerCreate)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with managerId [1 / 10 ]
                            managerAttr[CPSS_DXCH_PACKET_ANALYZER_SYSTEM_MODE_OUT_OF_SERVICE_E /
                                        CPSS_DXCH_PACKET_ANALYZER_SYSTEM_MODE_IN_SERVICE_E       ]
    Expected: GT_OK.
    1.2. Call function with managerId [BAD_MANAGER_ID_VALUE].
    Expected: GT_BAD_PARAM
    1.3  Call function with managerId already initialized
    Expected: GT_ALREADY_EXIST
    1.4. Call with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
           1.1.1  Call function with managerId [1]
           Expected: GT_OK.
        */
        managerId = 1 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);

        /*
           1.1.2  Call function with managerId [10]
           Expected: GT_OK.
        */
        managerId = 10 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);

        /*
           1.2  Call function with managerId [BAD_MANAGER_ID_VALUE].
           Expected: GT_BAD_PARAM
        */
        managerId = BAD_MANAGER_ID_VALUE ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, managerId);

        /*
            1.3  Call function with managerId already initialized
            Expected: GT_ALREADY_EXIST
        */
        managerId = 10 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_ALREADY_EXIST, st, devNum, managerId);

        /* Restore system. */
        managerId=1;
        st =cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        managerId=10;
        st =cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }
#else

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerManagerDelete
(
    IN  GT_U32                      managerId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerManagerDelete)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerManagerCreate with managerId [1]
         and delete it
    Expected: GT_OK.
    1.2. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function managerId [5].
    Expected: GT_NOT_FOUND.
    2. fill memory with more than 1 manager and delete all
    Expected: GT_OK.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function cpssDxChPacketAnalyzerManagerCreate with managerId [1]
                 and delete it
            Expected: GT_OK.
        */
        managerId = 1 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);

        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /*
            1.2. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        managerId = BAD_MANAGER_ID_VALUE ;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, managerId);

        /*
            1.3. Call function managerId [5].
            Expected: GT_NOT_FOUND.
        */
        managerId = 5 ;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, managerId);

        /*
            2. fill memory with more than 1 manager and delete all
            Expected: GT_OK.
        */
        managerId = 3 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);
        managerId = 6 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);
        managerId = 2 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);
        managerId = 1 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);

        managerId=6;
        st =cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        managerId=2;
        st =cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        managerId=1;
        st =cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        managerId=3;
        st =cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerManagerDeviceAdd
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerManagerDeviceAdd)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with initialized managerId [1] and applic devNum
    Expected: GT_OK.
    1.2. Call function with not initialized managerId [2] and applic devNum
    Expected: GT_NOT_FOUND.
    1.3. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId */
        managerId = 1 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);

        /*
            1.1. Call function with initialized managerId [1] and applic devNum [0]
            Expected: GT_OK.
        */
        st = cpssDxChPacketAnalyzerManagerDeviceAdd(managerId,devNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, managerId, devNum );

        /*
            1.2. Call function with not initialized managerId [2] and applic devNum [0]
            Expected: GT_NOT_FOUND.
        */
        managerId = 2 ;
        st = cpssDxChPacketAnalyzerManagerDeviceAdd(managerId,devNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, managerId);

        /*
            1.3. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        managerId = BAD_MANAGER_ID_VALUE ;
        st = cpssDxChPacketAnalyzerManagerDeviceAdd(managerId,devNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, managerId);

        /* Restore system. */
        managerId = 1 ;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        /* initialized managerId */
        managerId = 1 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);

        st = cpssDxChPacketAnalyzerManagerDeviceAdd(managerId,devNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, managerId);

        st =cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerManagerDeviceRemove
(

    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerManagerDeviceRemove)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerManagerCreate and
                       cpssDxChPacketAnalyzerManagerDeviceAdd with managerId [1]
                       and applic devNum and delete it
    Expected: GT_OK.
    1.2. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function managerId [5].
    Expected: GT_NOT_FOUND.
    1.4. Call function devNum that not added .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");

        /*
            1.1. Call function cpssDxChPacketAnalyzerManagerCreate and
                               cpssDxChPacketAnalyzerManagerDeviceAdd with managerId [1]
                               and applic devNum and delete it
            Expected: GT_OK.
        */
        st = cpssDxChPacketAnalyzerManagerDeviceRemove(managerId,devNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, managerId, devNum );

        /*
            1.2. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        managerId = BAD_MANAGER_ID_VALUE ;
        st = cpssDxChPacketAnalyzerManagerDeviceRemove(managerId,devNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, managerId, devNum );

        /*
            1.3. Call function managerId [5].
            Expected: GT_NOT_FOUND.
        */
        managerId = 5 ;
        st = cpssDxChPacketAnalyzerManagerDeviceRemove(managerId,devNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, managerId, devNum );
        /*
            1.4. Call function devNum that not added .
            Expected: GT_BAD_PARAM.
        */
        managerId = GENERIC_MANAGER_ID ;
        st = cpssDxChPacketAnalyzerManagerDeviceRemove(managerId,devNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, managerId, devNum );

        /* Restore system. */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        /* initialized managerId */
        managerId = 1 ;
        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);

        PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->numOfDevices++;
        st = cpssDxChPacketAnalyzerManagerDeviceRemove(managerId,devNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, managerId, devNum );

        PRV_SHARED_IDEBUG_DIR_PACKET_ANALYZER_SRC_DB_MANGER_PTR(managerId)->numOfDevices--;
        st =cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, managerId);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerManagerEnableSet
(
    IN  GT_U32                                  managerId,
    IN  GT_BOOL                                 enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerManagerEnableSet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerManagerCreate and
                       cpssDxChPacketAnalyzerManagerDeviceAdd with managerId [1]
                       and applic devNum and enable it
    Expected: GT_OK.
    1.2. Call function cpssDxChPacketAnalyzerManagerCreate and
                       cpssDxChPacketAnalyzerManagerDeviceAdd with managerId [1]
                       and applic devNum and disable it
    Expected: GT_OK.
    1.3. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.4. Call function managerId not created [5].
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*init manager and device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /*
            1.1. Call function cpssDxChPacketAnalyzerManagerCreate and
                 cpssDxChPacketAnalyzerManagerDeviceAdd with managerId [1]
                 and applic devNum and enable it
           Expected: GT_OK.
        */
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /*
            1.2. Call function cpssDxChPacketAnalyzerManagerCreate and
                 cpssDxChPacketAnalyzerManagerDeviceAdd with managerId [1]
                 and applic devNum and disable it
            Expected: GT_OK.
        */
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        /*
            1.3. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        managerId = BAD_MANAGER_ID_VALUE ;
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, managerId);

        /*
            1.4. Call function managerId not created [5].
            Expected: GT_NOT_FOUND.
        */
        managerId = 5 ;
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, managerId);

        /* Restore system. */
        managerId = GENERIC_MANAGER_ID ;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerManagerDevicesGet
(
    IN    GT_U32                                            managerId,
    OUT   GT_BOOL                                           *deviceStatePtr,
    INOUT GT_U32                                            *numOfDevicesPtr,
    OUT   GT_U8                                             devicesArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerManagerDevicesGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerManagerCreate
                       cpssDxChPacketAnalyzerManagerDeviceAdd
                       cpssDxChPacketAnalyzerManagerEnableSet
                       with valid values
    Expected: GT_OK and same action as written
    1.2. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function managerId [5].
    Expected: GT_NOT_FOUND.
    1.4. Call function numOfDevicesGet less than needed  [0].
    Expected: GT_BAD_SIZE.
    1.5. Call with NULL managerAttrPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call with NULL deviceStatePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.7. Call with NULL numOfDevicesPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.8. Call with NULL devicesArr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;
    GT_U32                                              ii;
    GT_BOOL                                             deviceState;
    GT_U32                                              numOfDevices;
    GT_U8                                               devicesArr[SIZE_OF_TEST_ARR];

    GT_BOOL                                             deviceStateGet;
    GT_U32                                              numOfDevicesGet;
    GT_U8                                               devicesArrGet[SIZE_OF_TEST_ARR];

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    cpssOsMemSet(devicesArrGet, 0, sizeof(devicesArr));
    cpssOsMemSet(devicesArr, 0, sizeof(devicesArrGet));

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device and enable manager*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");

        deviceState = GT_TRUE;

        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,deviceState);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        /*
            1.1. Call function cpssDxChPacketAnalyzerManagerCreate
                               cpssDxChPacketAnalyzerManagerDeviceAdd
                               cpssDxChPacketAnalyzerManagerEnableSet
                               with valid values
            Expected: GT_OK and same action as written
        */
        devicesArr[0] = devNum ;
        numOfDevices = numOfDevicesGet = 1 ;
        st = cpssDxChPacketAnalyzerManagerDevicesGet(managerId,&deviceStateGet,&numOfDevicesGet ,devicesArrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChPacketAnalyzerManagerDevicesGet: %d", managerId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(deviceState, deviceStateGet,
                       "got other deviceStateGet than was set: %d", managerId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfDevices, numOfDevicesGet,
                       "got other numOfDevicesGet than was set: %d", managerId);
        for (ii=0 ; ii<numOfDevicesGet; ii++ )
        {
            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(devicesArr[ii], devicesArrGet[ii],
                           "got other devicesArrGet than was set at index: %d ,mangerId : %d ",ii ,managerId);
        }
        /*
            1.2. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        managerId = BAD_MANAGER_ID_VALUE ;
        st = cpssDxChPacketAnalyzerManagerDevicesGet(managerId,&deviceStateGet,&numOfDevicesGet ,devicesArrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,"cpssDxChPacketAnalyzerManagerDevicesGet: %d", managerId);
        /*
            1.3. Call function managerId [5].
            Expected: GT_NOT_FOUND.
        */
        managerId = 5 ;
        st = cpssDxChPacketAnalyzerManagerDevicesGet(managerId,&deviceStateGet,&numOfDevicesGet ,devicesArrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"cpssDxChPacketAnalyzerManagerDevicesGet: %d", managerId);

        /*
            1.4. Call function numOfDevicesGet less than needed  [0].
            Expected: GT_BAD_SIZE.
        */
        managerId = GENERIC_MANAGER_ID ;
        numOfDevicesGet = 0;
        st = cpssDxChPacketAnalyzerManagerDevicesGet(managerId,&deviceStateGet,&numOfDevicesGet ,devicesArrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_SIZE, st,"cpssDxChPacketAnalyzerManagerDevicesGet: %d", managerId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(1, numOfDevicesGet,
                       "got other numOfDevicesGet than was set: %d", numOfDevicesGet);

        /*
            1.6. Call with NULL deviceStatePtr and other valid params
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPacketAnalyzerManagerDevicesGet(managerId,NULL,&numOfDevicesGet ,devicesArrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,"cpssDxChPacketAnalyzerManagerDevicesGet: %d", managerId);
        /*
            1.7. Call with NULL numOfDevicesPtr and other valid params
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPacketAnalyzerManagerDevicesGet(managerId,&deviceStateGet,NULL ,devicesArrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,"cpssDxChPacketAnalyzerManagerDevicesGet: %d", managerId);
        /*
            1.8. Call with NULL devicesArr and other valid params.
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPacketAnalyzerManagerDevicesGet(managerId,&deviceStateGet,&numOfDevicesGet ,NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,"cpssDxChPacketAnalyzerManagerDevicesGet: %d", managerId);

        /* Restore system. */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerManagerResetToDefaults
(
    IN  GT_U32                                  managerId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerManagerResetToDefaults)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerManagerCreate
                       cpssDxChPacketAnalyzerManagerDeviceAdd
                       cpssDxChPacketAnalyzerManagerEnableSet
                       with valid values
    Expected: GT_OK and same values as written
    1.2  call cpssDxChPacketAnalyzerManagerDevicesGet
    Expected: GT_OK default configuration
    1.3. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.4. Call function managerId [5].
    Expected: GT_NOT_FOUND.

*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;
    GT_U32                                              ii;
    GT_BOOL                                             deviceState;
    GT_U32                                              numOfDevices;
    GT_U8                                               devicesArr[SIZE_OF_TEST_ARR];

    GT_BOOL                                             deviceStateGet;
    GT_U32                                              numOfDevicesGet;
    GT_U8                                               devicesArrGet[SIZE_OF_TEST_ARR];

    cpssOsMemSet(devicesArr, 0, sizeof(devicesArr));
    cpssOsMemSet(devicesArrGet, 0, sizeof(devicesArrGet));

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device and enable manager*/
        managerId = GENERIC_MANAGER_ID;

        st = cpssDxChPacketAnalyzerManagerCreate(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        st = cpssDxChPacketAnalyzerManagerDeviceAdd(managerId, devNum);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, managerId,devNum);

        deviceState = GT_TRUE ;
        devicesArr[0] = devNum ;

        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,deviceState);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        /*
            1.1. Call function cpssDxChPacketAnalyzerManagerCreate
                               cpssDxChPacketAnalyzerManagerDeviceAdd
                               cpssDxChPacketAnalyzerManagerEnableSet
                               with valid values
            Expected: GT_OK and same values as written
        */
        numOfDevices = numOfDevicesGet = 1 ;
        st = cpssDxChPacketAnalyzerManagerDevicesGet(managerId,&deviceStateGet,&numOfDevicesGet ,devicesArrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChPacketAnalyzerManagerDevicesGet: %d", managerId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(deviceState, deviceStateGet,
                       "got other deviceStateGet than was set: %d", managerId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfDevices, numOfDevicesGet,
                       "got other numOfDevicesGet than was set: %d", managerId);
        for (ii=0 ; ii<numOfDevicesGet; ii++ )
        {
            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(devicesArr[ii], devicesArrGet[ii],
                           "got other devicesArrGet than was set at index: %d ,mangerId : %d ", ii,managerId);
        }
        /*
            1.2  call cpssDxChPacketAnalyzerManagerDevicesGet
            Expected: GT_OK default configuration
        */
        managerId = GENERIC_MANAGER_ID ;
        st = cpssDxChPacketAnalyzerManagerResetToDefaults(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId );

        /*set expected values */
        numOfDevicesGet = 1 ;
        numOfDevices = 0 ;
        devicesArr[0]= 0 ;
        deviceState = GT_FALSE ;
        st = cpssDxChPacketAnalyzerManagerDevicesGet(managerId,&deviceStateGet,&numOfDevicesGet ,devicesArrGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"cpssDxChPacketAnalyzerManagerDevicesGet: %d", managerId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(deviceState, deviceStateGet,
                       "got other deviceStateGet than was set: %d", managerId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfDevices, numOfDevicesGet,
                       "got other numOfDevicesGet than was set: %d", managerId);
        for (ii=0 ; ii<numOfDevicesGet; ii++ )
        {
            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(devicesArr[ii], devicesArrGet[ii],
                           "got other devicesArrGet than was set at index: %d ,mangerId : %d ", ii,managerId);
        }
        /*
            1.2. Call function with wrong value managerId [BAD_MANAGER_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        managerId = BAD_MANAGER_ID_VALUE ;
        st = cpssDxChPacketAnalyzerManagerResetToDefaults(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, managerId );
        /*
            1.3. Call function managerId [5].
            Expected: GT_NOT_FOUND.
        */
        managerId = 5 ;
        st = cpssDxChPacketAnalyzerManagerResetToDefaults(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, managerId );

        /*  Restore system. */
        managerId = GENERIC_MANAGER_ID ;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerFieldSizeGet
(
    IN  GT_U32                                      managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT         fieldName,
    OUT GT_U32
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerFieldSizeGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E     /
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E      /
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E /
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E /
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E ]
    Expected: GT_OK with Expected value .
    1.2. call function with UDF with known field
    Expected: GT_OK with Expected value.
    1.3. Call function with wrong value fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E]
    Expected: GT_BAD_PARAM.
    1.4. Call function null lengthPtr .
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                 fieldName;
    GT_U32                                              length;
    GT_U32                                              lengthGet;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                 udfId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC            udfFieldName;
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC        udfAttr;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");

        /*
            1.1.1 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E]
            Expected: GT_OK with Expected value .
        */
        fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E ;
        length = 48 ;
        st = cpssDxChPacketAnalyzerFieldSizeGet(managerId,fieldName,&lengthGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum );

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
            1.1.2 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E]
            Expected: GT_OK with Expected value .
        */
        fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E ;
        length = 1 ;
        st = cpssDxChPacketAnalyzerFieldSizeGet(managerId,fieldName,&lengthGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum );

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
            1.1.3 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E]
            Expected: GT_OK with Expected value .
        */
        fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E ;
        length = 14 ;
        st = cpssDxChPacketAnalyzerFieldSizeGet(managerId,fieldName,&lengthGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum );

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
            1.1.4 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E]
            Expected: GT_OK with Expected value .
        */
        fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E ;
        length = 3 ;
        st = cpssDxChPacketAnalyzerFieldSizeGet(managerId,fieldName,&lengthGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum );

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
            1.1.5 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E ]
            Expected: GT_OK with Expected value .
        */
        fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E ;
        length = 32 ;
        st = cpssDxChPacketAnalyzerFieldSizeGet(managerId,fieldName,&lengthGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum );

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
             1.2 call function with UDF with known field
             Expected: GT_OK.
        */
        cpssOsStrCpy(udfFieldName.fieldNameArr,"routed");
        cpssOsStrCpy(udfAttr.udfNameArr,"routed");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&udfFieldName,&udfId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfId);

        fieldName = udfId ;
        length = 1 ;
        st = cpssDxChPacketAnalyzerFieldSizeGet(managerId,fieldName,&lengthGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum );

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
            1.3. Call function with wrong value fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E]
            Expected: GT_BAD_PARAM.
        */
        fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E ;
        st = cpssDxChPacketAnalyzerFieldSizeGet(managerId,fieldName,&lengthGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM,  st,
                       "got other lengthGet than expected : %d", lengthGet);

        /*
            1.4. Call function null lengthPtr .
            Expected: GT_BAD_PTR.
        */
        fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E ;
        st= cpssDxChPacketAnalyzerFieldSizeGet(managerId,fieldName,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, managerId, devNum);

        /* Restore system. */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyCreate
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    IN  CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC        *keyAttrPtr,
    IN  GT_U32                                              numOfStages,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stagesArr[],
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT fieldMode,
    IN  GT_U32                                              numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                 fieldsArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerLogicalKeyCreate)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with keyId [1/1024/5]
                            keyAttr["test run : Logical Key Create"]
                            numOfStages[3/1/2]
                            stagesArr [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E,
                                       /
                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E
                                       /
                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
                                       CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E]
                            fieldMode [CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E
                                       /
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E
                                       /
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E]
                            numOfFields[3/4/1];
                            fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
                                      /
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E
                                      /
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E]
    Expected: GT_OK.
    1.1.4 call function with 0 fields and NULL fieldArr
    Expected: GT_OK.
    1.2. Call function with keyId [1]
                            keyAttr["test run : Logical Key Create"]
                            numOfStages[2]
                            stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E,
                                      CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_ENGRESS_STAGES_E]
                            fieldMode[CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E]
                            numOfFields[4];
                            fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E]
    Expected: GT_BAD_PARAM.
    1.3. Call function with keyId [1]
                            keyAttr["test run : Logical Key Create"]
                            numOfStages[2]
                            stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_ENGRESS_STAGES_E,
                                      CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E]
                            fieldMode[CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E]
                            numOfFields[4];
                            fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E]
    Expected: GT_BAD_PARAM.
    1.4. Call function with keyId [1]
                            keyAttr["test run : Logical Key Create"]
                            numOfStages[1]
                            stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E]
                            fieldMode[CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E]
                            numOfFields[1];
                            fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E]
    Expected: GT_NOT_FOUND .
    field CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E is not in stage
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E
    1.5. Call function with keyId [1]
                            numOfStages[1]
                            stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E]
                            fieldMode[CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E]
                            numOfFields[1];
                            fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E]
    Expected: GT_BAD_PARAM - UDS_0 is not valid
    1.6. Call function with keyId [1]
                            numOfStages[1]
                            stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E]
                            fieldMode[CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E]
                            numOfFields[1];
                            fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E]
    Expected: GT_NOT_INITIALIZED - UDF_0 is not valid
    1.7. Call function with keyId [BAD_KEY_ID_VALUE] and all other valid value
    Expected: GT_BAD_PARAM.
    1.8. Call with NULL keyAttrPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.9. Call with NULL stagesArr and other valid params.
    Expected: GT_BAD_PTR.
    1.10. Call with NULL fieldsArr and other valid params.
    Expected: GT_BAD_PTR.
    1.11. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;
    GT_U32                                               keyId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC         keyAttr;
    GT_U32                                               numOfStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT  fieldMode;
    GT_U32                                               numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsArr[SIZE_OF_TEST_ARR];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, managerId, devNum);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        /*
            1.1.1 Call function with keyId [1]
                                     keyAttr["test run : Logical Key Create"]
                                     numOfStages[3]
                                     stagesArr [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E,
                                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E,
                                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E]
                                     fieldMode [CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E]
                                     numOfFields[3];
                                     fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,
                                               CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,
                                               CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E]
            Expected: GT_OK.
        */
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 1 ;
        cpssOsMemCpy(keyAttr.keyNameArr,"test run : Logical Key Create" ,sizeof("test run : Logical Key Create"));
        numOfStages = 3 ;
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E ;
        stagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        stagesArr[2] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
        numOfFields = 3 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        fieldsArr[2] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, managerId, devNum);

        /*
            1.1.2 Call function with keyId [1024]
                                     keyAttr["test run : Logical Key Create"]
                                     numOfStages[1]
                                     stagesArr [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E]
                                     fieldMode [CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E]
                                     numOfFields[4];
                                     fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E,
                                               CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
                                               CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,
                                               CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E ]
            Expected: GT_OK.
        */
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 1024 ;
        numOfStages = 1 ;
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
        numOfFields = 4 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsArr[2] = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
        fieldsArr[3] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, managerId, devNum);

        /*
            1.1.3 Call function with keyId [5]
                                     keyAttr["test run : Logical Key Create"]
                                     numOfStages[2]
                                     stagesArr [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E,
                                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E,
                                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E,
                                                CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E]
                                     fieldMode [CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E]
                                     numOfFields[1];
                                     fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E]
            Expected: GT_OK.
        */
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 5 ;
        numOfStages = 4 ;
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E ;
        stagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E ;
        stagesArr[2] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E ;
        stagesArr[3] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E;
        numOfFields = 1 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, managerId, devNum);

        /*
            1.1.4 call function with 0 fields and NULL fieldArr
            Expected: GT_OK.
        */
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 2 ;
        numOfStages = 1 ;
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
        numOfFields = 0 ;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, managerId, devNum);

        /*delete Keys*/
        keyId = 1;
        st = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId,keyId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum , keyId );
        keyId = 1024 ;
        st = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId,keyId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum ,keyId );
        keyId = 5 ;
        st = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId,keyId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum ,keyId );
        keyId = 2;
        st = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId,keyId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum , keyId );


        /*
            1.2. Call function with keyId [1]
                                    keyAttr["test run : Logical Key Create"]
                                    numOfStages[2]
                                    stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E,
                                              CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E]
                                    fieldMode[CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E]
                                    numOfFields[4];
                                    fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E,
                                              CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
                                              CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,
                                              CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E ]
            Expected: GT_BAD_PARAM ,
            can't be both CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E
            and CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_ENGRESS_STAGES_E .
        */
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 1 ;
        numOfStages = 2 ;
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E ;
        stagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
        numOfFields = 4 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsArr[2] = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
        fieldsArr[3] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, managerId, devNum);

        /*
            1.2.1 Call function with valid parms and not applicable stage
            Expected: GT_BAD_STATE
        */
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E ;

        keyId = 6 ;
        numOfStages = 1 ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E;
        numOfFields = 1 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_STATE, st, managerId, devNum);
        /*
            1.3. Call function with keyId [1]
                                    keyAttr["test run : Logical Key Create"]
                                    numOfStages[2]
                                    stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_ENGRESS_STAGES_E,
                                              CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E]
                                    fieldMode[CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E]
                                    numOfFields[4];
                                    fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E,
                                              CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,
                                              CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,
                                              CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E ]
            Expected: GT_BAD_PARAM.
            can't be both CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_ENGRESS_STAGES_E
            and CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E .
        */
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 1 ;
        numOfStages = 2 ;
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E ;
        stagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
        numOfFields = 4 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsArr[2] = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
        fieldsArr[3] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, managerId, devNum);

        /*
            1.4. Call function with keyId [1]
                                    keyAttr["test run : Logical Key Create"]
                                    numOfStages[1]
                                    stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E]
                                    fieldMode[CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E]
                                    numOfFields[1];
                                    fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E]
            Expected: GT_NOT_FOUND .
            field CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E is not in stage
            CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E
        */
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 1 ;
        numOfStages = 1 ;
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E;
        numOfFields = 1 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E;

        /*harrier doesn't has pre mac stage */
        if (paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
        {
            stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E ;
        }

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_FAIL, st, managerId, devNum);

        /*check that key was not created */
        st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId,keyId,&numOfStages,stagesArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum,keyId);

        /*
            1.5. Call function with keyId [1]
                                    numOfStages[1]
                                    stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E]
                                    fieldMode[CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E]
                                    numOfFields[1];
                                    fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E]
            Expected: GT_BAD_PARAM - UDS_0 is not valid
        */
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 1 ;
        numOfStages = 1 ;
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_0_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
        numOfFields = 1 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(devNum,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, managerId, devNum);

       /*
            1.6. Call function with keyId [1]
                                numOfStages[1]
                                stagesArr[CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E]
                                fieldMode[CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E]
                                numOfFields[1];
                                fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E]
            Expected: GT_BAD_PARAM - UDF_0 is not valid
        */
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 1 ;
        numOfStages = 1 ;
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
        numOfFields = 1 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(devNum,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, managerId, devNum);

        /*
            1.7. Call function with keyId [BAD_KEY_ID_VALUE] and all other valid value
            Expected: GT_BAD_PARAM.
        */
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = BAD_KEY_ID_VALUE ;
        numOfStages = 2 ;
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E ;
        stagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E;
        numOfFields = 2 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, managerId, devNum);
        /*
            1.8. Call with NULL keyAttrPtr and other valid params
            Expected: GT_BAD_PTR.
        */
        keyId = 1 ;
        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,NULL,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, managerId, devNum);
        /*
            1.9. Call with NULL stagesArr and other valid params
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,NULL,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, managerId, devNum);
        /*
            1.10. Call with NULL fieldsArr and other valid params
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, managerId, devNum);
        /*
            1.11. Call function when manager is not initialized
            Expected: GT_BAD_PARAM
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,numOfStages,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, managerId, devNum);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyFieldsAdd
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId,
    IN  GT_U32                                  numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT     fieldsArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerLogicalKeyFieldsAdd)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with keyId match to created key
                            numOfFields[2];
                            fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_1_E]
    Expected: GT_OK.
    1.2. Call function with keyId match to created key
                            numOfFields[2];
                            fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_2_E]
    Expected: GT_BAD_PARAM - UDF_1 is not valid
    1.3. Call function with keyId [BAD_KEY_ID_VALUE] and all other valid value
    Expected: GT_BAD_PARAM.
    1.4. Call with NULL fieldsArr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call function with keyId [2] that not created and all other valid value
    Expected: GT_NOT_FOUND.
    1.6. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_U32                                               keyId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC         keyAttr;
    GT_U32                                               numOfStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT  fieldMode;
    GT_U32                                               numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  udfId;
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC         udfAttr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC             udfFieldName;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=1;
        st = genericKeyInit(managerId,keyId,&keyAttr,&numOfStages,stagesArr,&fieldMode,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericKeyInit ");

        cpssOsStrCpy(udfFieldName.fieldNameArr, "packet_hash");
        cpssOsStrCpy(udfAttr.udfNameArr, "packet_hash");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&udfFieldName, &udfId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, managerId, udfId);

        /*
            1.1. Call function with keyId match to created key
                                    numOfFields[2];
                                    fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,udf_1]
            Expected: GT_OK.
        */
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 1 ;
        numOfFields = 2 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
        fieldsArr[1] = udfId;

        st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /*
            1.2. Call function with keyId match to created key
                                    numOfFields[2];
                                    fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_2_E]
            Expected: GT_BAD_PARAM - UDF_2 is not valid
        */
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 1 ;
        numOfFields = 2 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_2_E;

        st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId);

        /*
            1.3. Call function with keyId [BAD_KEY_ID_VALUE] and all other valid value
            Expected: GT_BAD_PARAM.
        */
        keyId = BAD_KEY_ID_VALUE ;

        st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId);

        /*
            1.4. Call with NULL fieldsArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        keyId = 1 ;

        st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId,keyId,numOfFields,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId);

        /*
            1.5. Call function with keyId [2] that not created and all other valid value
            Expected: GT_NOT_FOUND.
        */
        keyId = 2 ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);

        /*
            1.6. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        keyId = 1 ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyFieldsRemove
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId,
    IN  GT_U32                                  numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT     fieldsArr[]
)
;*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerLogicalKeyFieldsRemove)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with keyId match to created key
                            numOfFields[3];
                            fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,
                                      CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,
                                      UDF_0]
    Expected: GT_OK.
    1.2. Call function with keyId [BAD_KEY_ID_VALUE] and all other valid value
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL fieldsArr and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call function with keyId [2] that not created and all other valid value
    Expected: GT_NOT_FOUND.
    1.5. Call function with field that bounded to rule
    Expected: GT_FAIL.
    1.6. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum=0;
    GT_U32                                               managerId;
    GT_U32                                               keyId,groupId,actionId,ruleId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC         keyAttr;
    GT_U32                                               numOfStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT  fieldMode;
    GT_U32                                               numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsArr[SIZE_OF_TEST_ARR];

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=1;
        st = genericKeyInit(managerId,keyId,&keyAttr,&numOfStages,stagesArr,&fieldMode,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /* initialized group and action*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");
        actionId = 1;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericActionInit ");

        /*
            1.1. Call function with keyId match to created key
                                    numOfFields[3];
                                    fieldsArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,
                                              CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E
                                              UDF_0]
        */

        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        keyId = 1 ;
        numOfFields = 3 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E;
        fieldsArr[2] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;

        st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /*
            1.2. Call function with keyId [BAD_KEY_ID_VALUE] and all other valid value
            Expected: GT_BAD_PARAM.
        */
        keyId = BAD_KEY_ID_VALUE ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId);

        /*
            1.3. Call with NULL fieldsArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        keyId = 1 ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,numOfFields,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId);

        /*
            1.4. Call function with keyId [2] that not created and all other valid value
            Expected: GT_NOT_FOUND.
        */
        keyId = 2 ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);

        /*
            1.5. Call function with field that bounded to rule
            Expected: GT_FAIL.
        */

        keyId = 1 ;
        numOfFields = 3 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        fieldsArr[2] = CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /* initialized rule*/
        ruleId = 1;
        st = genericRuleInit(managerId, keyId, groupId, actionId, ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericRuleInit ");

        /*try to delete fields */
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_FAIL, st, devNum, keyId);

        /*delete bounded rule*/
        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId, ruleId, groupId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId);

        st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /* Restore system. */
        st = cpssDxChPacketAnalyzerGroupDelete(managerId, groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        st = cpssDxChPacketAnalyzerActionDelete(managerId, actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.6. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        keyId = 1 ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);

    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyDelete
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  keyId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerLogicalKeyDelete)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerLogicalKeyDelete applicable devNum
            and valid keyId [1]
    Expected: GT_OK.
    1.2. Call function with wrong value keyId [BAD_KEY_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong value keyId [2]
    Expected: GT_NOT_FOUND.
    1.4. Call Call function when manager is not initialized
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;
    GT_U32                                               keyId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=1;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");

        /*
            1.1. Call function cpssDxChPacketAnalyzerLogicalKeyDelete applicable devNum
                    and valid keyId [1]
            Expected: GT_OK.
        */
        keyId = 1 ;
        st = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId,keyId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /*
            1.2. Call function with wrong value keyId [BAD_KEY_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        keyId = BAD_KEY_ID_VALUE ;
        st = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId,keyId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId);

        /*
            1.3. Call function with wrong value keyId [2]
            Expected: GT_NOT_FOUND.
        */
        keyId = 2 ;
        st = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId,keyId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);

        /*
            1.4. Call Call function when manager is not initialized
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId,keyId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyInfoGet
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    OUT CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC        *keyAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT *fieldModePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerLogicalKeyInfoGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerLogicalKeyInfoGet with
         valid values
    Expected: GT_OK and same values as written at init stage
    1.2. Call function with wrong value keyId [BAD_KEY_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong value keyId [2]
    Expected: GT_NOT_FOUND.
    1.4. Call with NULL keyAttrPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL fieldModePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call Call function when manager is not initialized
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;
    GT_U32                                               keyId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC         keyAttr;
    GT_U32                                               numOfStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT  fieldMode;
    GT_U32                                               numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsArr[SIZE_OF_TEST_ARR];

    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC         keyAttrGet;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT  fieldModeGet;
    GT_U32                                               strCmpAns;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=1;
        st = genericKeyInit(managerId,keyId,&keyAttr,&numOfStages,stagesArr,&fieldMode,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /*
            1.1. Call function cpssDxChPacketAnalyzerLogicalKeyInfoGet with
                 valid values
            Expected: GT_OK and same values as written
        */
        st = cpssDxChPacketAnalyzerLogicalKeyInfoGet(managerId,keyId,&keyAttrGet,&fieldModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        strCmpAns = cpssOsMemCmp((GT_VOID*) &keyAttr.keyNameArr,(GT_VOID*) keyAttrGet.keyNameArr,
                                 sizeof(keyAttr.keyNameArr));
        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC(0,strCmpAns,
                       "got other keyAttrGet than was set");
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(fieldMode, fieldModeGet,
                       "got other fieldModeGet than was set: %d", fieldModeGet);

        /*
            1.2. Call function with wrong value keyId [BAD_KEY_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        keyId = BAD_KEY_ID_VALUE ;
        st = cpssDxChPacketAnalyzerLogicalKeyInfoGet(managerId,keyId,&keyAttrGet,&fieldModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId);

        /*
            1.3. Call function with wrong value keyId [2]
            Expected: GT_NOT_FOUND.
        */
        keyId = 2 ;
        st = cpssDxChPacketAnalyzerLogicalKeyInfoGet(managerId,keyId,&keyAttrGet,&fieldModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);
        /*
            1.4. Call with NULL keyAttrPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        keyId = 1 ;
        st = cpssDxChPacketAnalyzerLogicalKeyInfoGet(managerId,keyId,NULL,&fieldModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId);
        /*
            1.5. Call with NULL fieldModePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerLogicalKeyInfoGet(managerId,keyId,&keyAttrGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId);
        /*
            1.6. Call Call function when manager is not initialized
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerLogicalKeyInfoGet(managerId,keyId,&keyAttrGet,&fieldModeGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyStagesGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U32                                            keyId,
    INOUT GT_U32                                            *numOfStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stagesArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerLogicalKeyStagesGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerLogicalKeyStagesGet with
         valid values
    Expected: GT_OK and same values as written at init stage
    1.2. Call function with numOfStages less than init number [1]
    Expected: GT_BAD_SIZE.
    1.3. Call function with wrong value keyId [BAD_KEY_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong value keyId [2]
    Expected: GT_NOT_FOUND.
    1.5. Call with NULL numOfStagesPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call with NULL stagesArr and other valid params.
    Expected: GT_BAD_PTR.
    1.7. Call Call function when manager is not initialized
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;
    GT_U32                                               keyId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC         keyAttr;
    GT_U32                                               numOfStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT  fieldMode;
    GT_U32                                               numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsArr[SIZE_OF_TEST_ARR];
    GT_U32                                               ii;

    GT_U32                                               numOfStagesGet;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stagesArrGet[SIZE_OF_TEST_ARR];

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=1;
        st = genericKeyInit(managerId,keyId,&keyAttr,&numOfStages,stagesArr,&fieldMode,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");

        /*
            1.1. Call function cpssDxChPacketAnalyzerLogicalKeyStagesGet with
                 valid values
            Expected: GT_OK and same values as written at init stage
        */
        numOfStagesGet = SIZE_OF_TEST_ARR ; /* more than what init*/
        cpssOsMemSet(stagesArrGet, 0, sizeof(stagesArrGet));

        st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId,keyId,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfStages, numOfStagesGet,
                       "got other numOfStagesGet than was set :", numOfStagesGet);

        for (ii=0 ; ii<numOfStagesGet ; ii++)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(stagesArr[ii], stagesArrGet[ii],
                           "got other stagesArrGet than was set: %d index %d ", stagesArr[ii] , ii);
        }
        /*
            1.2. Call function with numOfStages less than init number [1]
            Expected: GT_BAD_SIZE.
        */
        numOfStagesGet = 1 ; /* less than what init*/
        cpssOsMemSet(stagesArrGet, 0, sizeof(stagesArrGet));

        st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId,keyId,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum, keyId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(5, numOfStagesGet,
                       "got other numOfStagesGet than was set :", numOfStagesGet);

        /*
            1.3. Call function with wrong value keyId [BAD_KEY_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        keyId = BAD_KEY_ID_VALUE ;
        numOfStagesGet = 5;
        st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId,keyId,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId);

        /*
            1.4. Call function with wrong value keyId [2]
            Expected: GT_NOT_FOUND.
        */
        keyId = 2 ;
        st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId,keyId,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);
        /*
            1.5. Call with NULL keyAttrPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        keyId = 1 ;
        st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId,keyId,NULL,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId);
        /*
            1.6. Call with NULL fieldModePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId,keyId,&numOfStagesGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId);
        /*
            1.7. Call Call function when manager is not initialized
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId,keyId,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet
(
    IN  GT_U32                                              managerId,
    IN  GT_U32                                              keyId,
    IN  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId,
    INOUT GT_U32                                            *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT               fieldsArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet with
         valid values
    Expected: GT_OK and same values as written at init stage
    1.2. Call function with numOfFields less than init number [1]
    Expected: GT_BAD_SIZE.
    1.3. Call function with wrong stageId
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong value keyId [BAD_KEY_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.5. Call function with wrong value keyId [2]
    Expected: GT_NOT_FOUND.
    1.6. Call with NULL numOfFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.7. Call with NULL fieldsArr and other valid params.
    Expected: GT_BAD_PTR.
    1.8. Call Call function when manager is not initialized
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;
    GT_U32                                               keyId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC         keyAttr;
    GT_U32                                               numOfStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT  fieldMode;
    GT_U32                                               numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsArr[SIZE_OF_TEST_ARR];
    GT_U32                                               ii;

    GT_U32                                               numOfFieldsGet;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsArrGet[SIZE_OF_TEST_ARR];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsMemSet(stagesArr, 0, sizeof(stagesArr));
        cpssOsMemSet(fieldsArr, 0, sizeof(fieldsArr));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /* initialized Key*/
        keyId=1;
        st = genericKeyInit(managerId,keyId,&keyAttr,&numOfStages,stagesArr,&fieldMode,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");

        /*
            1.1. Call function cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet with
                 valid values
            Expected: GT_OK and same values as written at init stage
        */
        numOfFieldsGet = SIZE_OF_TEST_ARR ; /* more than what init*/
        cpssOsMemSet(fieldsArrGet, 0, sizeof(fieldsArrGet));

        st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,stagesArr[0],&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /* verifying values */
        numOfFields=3;  /*Expected*/
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfFields, numOfFieldsGet,
                       "got other numOfFieldsGet than was set :", numOfFieldsGet);

        fieldsArr[0]=CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;  /*Expected*/
        fieldsArr[1]=CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;  /*Expected*/
        fieldsArr[2]=CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E;       /*Expected*/
        for (ii=0 ; ii<numOfFieldsGet ; ii++)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(fieldsArr[ii], fieldsArrGet[ii],
                           "got other fieldsArrGet than was set: %d index %d ", fieldsArrGet[ii] , ii);
        }

        numOfFieldsGet = 5 ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,stagesArr[1],&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /* verifying values */
        numOfFields=5;
        UTF_VERIFY_EQUAL0_STRING_MAC(numOfFields, numOfFieldsGet,
                       "got other numOfFieldsGet than was set ");

        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;     /*Expected*/
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;     /*Expected*/
        fieldsArr[2] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E; /*Expected*/
        fieldsArr[3] = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E; /*Expected*/
        fieldsArr[4] = CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E;      /*Expected*/

        for (ii=0 ; ii<numOfFieldsGet ; ii++)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(fieldsArr[ii], fieldsArrGet[ii],
                           "got other fieldsArrGet than was set: %d index %d ", fieldsArrGet[ii] , ii);
        }

        /*
            1.2. Call function with numOfFields less than init number [1]
            Expected: GT_BAD_SIZE.
        */
        numOfFieldsGet = 2 ; /* less than what init*/
        cpssOsMemSet(fieldsArrGet, 0, sizeof(fieldsArrGet));

        st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,stagesArr[0],&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum, keyId);

        /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(3, numOfFieldsGet,
                           "got other numOfFieldsGet than was set: %d ", numOfFieldsGet);
        /*
            1.3. Call function with wrong stageId
            Expected: GT_OK with numOfFields 0 and default value in first arr .
        */
        numOfFieldsGet=5 ;
        stagesArr[5] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,
                                                               stagesArr[5],
                                                               &numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /* verifying values */
        numOfFields=0;  /*Expected*/
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfFields, numOfFieldsGet,
                       "got other numOfFieldsGet than was set :", numOfFieldsGet);

        fieldsArr[0]=CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_IS_TRUNK_E;  /*Expected*/
        for (ii=0 ; ii<numOfFieldsGet ; ii++)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(fieldsArr[ii], fieldsArrGet[ii],
                           "got other fieldsArrGet than was set: %d index %d ", fieldsArrGet[ii] , ii);
        }

        /*
            1.4. Call function with wrong value keyId [BAD_KEY_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        keyId = BAD_KEY_ID_VALUE ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,stagesArr[0],&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId);

        /*
            1.5. Call function with wrong value keyId [2]
            Expected: GT_NOT_FOUND.
        */
        keyId = 2 ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,stagesArr[0],&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);
        /*                                                       .
            1.6. Call with NULL numOfFieldsPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        keyId = 1 ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,stagesArr[0],NULL,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId);
        /*
            1.7. Call with NULL fieldsArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,stagesArr[0],&numOfFieldsGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId);
        /*
            1.8. Call Call function when manager is not initialized
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,stagesArr[0],&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerStagesGet
(
    IN    GT_U32                                         managerId,
    INOUT GT_U32                                         *numOfApplicStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    applicStagesListArr[],
    INOUT GT_U32                                         *numOfValidStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    validStagesListArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerStagesGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1.1 Call function with no selected stages
    Expected: GT_OK and number of valid stages [10] and valid stages as expected.
    1.1.2 Call function after select stage that is muxed
    Expected: GT_OK and number of valid stages [10] and valid stages as expected.
    1.1.3 Call function after adding uds and after binding it
    Expected: GT_OK and number of valid stages [11] and valid stages as expected.
    1.1.4 Call function after unbinding 3 stages
    Expected: GT_OK and number of valid stages [8] and valid stages as expected.
    1.2. Call function numOfApplicStagesPtr with less than needed [15]  .
    Expected: GT_BAD_SIZE.
    1.3. Call function numOfValidStagesPtr with less than needed [15]  .
    Expected: GT_BAD_SIZE.
    1.4. Call with NULL numOfApplicStagesPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL applicStagesListArr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call with NULL numOfValidStagesPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.7. Call with NULL validStagesListArr and other valid params.
    Expected: GT_BAD_PTR.
    1.8. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                               st = GT_OK;
    GT_U8                                                   devNum;
    GT_U32                                                  managerId;
    GT_U32                                                  ii,numOfApplicStagesGet=26 ,expNumOfApplicStages = 0;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             applicStagesListArr[SIZE_OF_TEST_ARR];
    GT_U32                                                  expNumOfValidStages = 0 ,numOfValidStagesGet=26;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             validStagesListArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             expValidStages[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC           udsInterfaceAttributes;
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC            udsAttr;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             udsId;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                   paMngDbPtr=NULL;

     /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    UT_PA_CHECK_XML

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1. Set Packet Analyzer basic configuration: */

        /*set default values */
        managerId = 1;

        /* initialized managerId with device*/
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");

        /* AUTODOC: enable PA on all devices added to manager */
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerManagerEnableSet ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*avoid warnings*/
        cpssOsMemCpy(expValidStages, utPaPhoenixDefaultValidStages, sizeof(utPaPhoenixDefaultValidStages));
        cpssOsMemCpy(expValidStages, utPaHawkDefaultValidStages, sizeof(utPaHawkDefaultValidStages));
        cpssOsMemCpy(expValidStages, utPaFalconDefaultValidStages, sizeof(utPaFalconDefaultValidStages));
        cpssOsMemCpy(expValidStages, utPaAldrin2DefaultValidStages, sizeof(utPaAldrin2DefaultValidStages));
        cpssOsMemCpy(expValidStages, utPaAldrinDefaultValidStages, sizeof(utPaAldrinDefaultValidStages));
        cpssOsMemCpy(expValidStages, utPaHarrierDefaultValidStages, sizeof(utPaHarrierDefaultValidStages));
        cpssOsMemSet(expValidStages, 0, sizeof(expValidStages));

        /*
            1.1.1 Call function with no selected stages
            Expected: GT_OK and number of valid stages [10] and valid stages as expected.
        */
        expNumOfApplicStages=16;
        numOfApplicStagesGet=16;
        switch(paMngDbPtr->devFamily)
        {

        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsMemCpy(expValidStages, utPaHarrierDefaultValidStages, sizeof(utPaHarrierDefaultValidStages));
            expNumOfApplicStages=16;
            numOfApplicStagesGet=16;
            numOfValidStagesGet=15;
            expNumOfValidStages=15;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsMemCpy(expValidStages, utPaPhoenixDefaultValidStages, sizeof(utPaPhoenixDefaultValidStages));
            expNumOfApplicStages=18;
            numOfApplicStagesGet=18;
            numOfValidStagesGet=17;
            expNumOfValidStages=17;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsMemCpy(expValidStages, utPaHawkDefaultValidStages, sizeof(utPaHawkDefaultValidStages));
            expNumOfApplicStages=18;
            numOfApplicStagesGet=18;
            numOfValidStagesGet=17;
            expNumOfValidStages=17;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsMemCpy(expValidStages, utPaFalconDefaultValidStages, sizeof(utPaFalconDefaultValidStages));
            expNumOfApplicStages=18;
            numOfApplicStagesGet=18;
            numOfValidStagesGet=13;
            expNumOfValidStages=13;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            numOfValidStagesGet=10;
            expNumOfValidStages=10;
            cpssOsMemCpy(expValidStages, utPaAldrin2DefaultValidStages, sizeof(utPaAldrin2DefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            numOfValidStagesGet=8;
            expNumOfValidStages=8;
            cpssOsMemCpy(expValidStages, utPaAldrinDefaultValidStages, sizeof(utPaAldrinDefaultValidStages));
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                     "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                     paStageStrArr[validStagesListArr[ii]],
                                     paStageStrArr[expValidStages[ii]]);
        }

        /*
            1.1.2 Call function after select stage that is muxed
            Expected: GT_OK and number of valid stages [10] and valid stages as expected.
        */

        /*unbind default stage that mux and bind other mux stage*/
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageUnbind");
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageBind" );
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageUnbind");
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageBind" );
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageUnbind");
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageBind" );
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageUnbind");
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageBind" );
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageUnbind");
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageBind" );
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        numOfApplicStagesGet=18;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            numOfValidStagesGet=15;
            expNumOfValidStages=15;
            expValidStages[10] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            numOfValidStagesGet=17;
            expNumOfValidStages=17;
            for (ii = 10; ii < 13; ii++)
                expValidStages[ii] = expValidStages[ii+1];
            expValidStages[13] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E;
            expValidStages[14] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E;
            expValidStages[15] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E;
            expValidStages[16] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            numOfValidStagesGet=13;
            expNumOfValidStages=13;
            expValidStages[6] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            numOfValidStagesGet=10;
            expNumOfValidStages=10;
            expValidStages[3]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            numOfValidStagesGet=8;
            expNumOfValidStages=8;
            expValidStages[2] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                     "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                     paStageStrArr[validStagesListArr[ii]],
                                     paStageStrArr[expValidStages[ii]]);
        }

        /*
            1.1.3 Call function after adding uds and after binding it
            Expected: GT_OK and number of valid stages [11] and valid stages as expected.
        */
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"har_epi_mac_macro_i0_mif2mti_app");
            udsInterfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"har_txqp_macro_sdq");
            udsInterfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"SH_FIFO_iDEBUG");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"har_pca_163e_macro_i1_sff_lmu_egr");
            udsInterfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"pnx_epi_100g_macro_mif2mti_app");
            udsInterfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"pnx_txq_macro_sdq0");
            udsInterfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"SH_FIFO_iDEBUG");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"pnx_pca_eip163_macro_sff_lmu_egr");
            udsInterfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"tile0_hawk_epi_400g_mac_macro_i2_mif2mti_app");
            udsInterfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"tile0_hawk_txqs_0_macro_sdq0");
            udsInterfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"SH_FIFO_iDEBUG");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"tile0_hawk_pca_macro_i0_sff_lmu_ctsu");
            udsInterfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"tile0_pipe1_txqs_macro_i3_pipe0_0");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"pds2tx_desc");
            udsInterfaceAttributes.interfaceIndex = 1;

            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_pipe1_eagle_ia_1_macro_erep2eq");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"erep2hbuegr_desc");
            udsInterfaceAttributes.interfaceIndex = 2;

            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_eagle_pdxc_macro_pdx2pdx0");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"PDX_2_SDQ_QCN_MSG");
            udsInterfaceAttributes.interfaceIndex = 5;

            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"iplr_ioam_desc_0_9");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"ioam2iplr0_desc");
            udsInterfaceAttributes.interfaceIndex = 0;

            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "l2i_ipvx_desc_or_debug_buses_0_8");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"l2i2mt_na_bus");
            udsInterfaceAttributes.interfaceIndex = 2;

            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "mbus_x_u11_buses_1_21");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"xbar2u_rd");
            udsInterfaceAttributes.interfaceIndex = 1;

            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"txfifo2_desc_push_7");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"DEBUG_BUS");
            udsInterfaceAttributes.interfaceIndex = 6;

            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "txdma0_hdr");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"TXDMA_2_HA_HEADER");
            udsInterfaceAttributes.interfaceIndex = 0;

                        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "rxdma0_desc_in_1");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"PORT2MEM");
            udsInterfaceAttributes.interfaceIndex = 5;

            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        numOfApplicStagesGet=19;
        expNumOfApplicStages=19;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            numOfApplicStagesGet=19;
            expNumOfApplicStages=19;
            numOfValidStagesGet=15;
            expNumOfValidStages=15;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            numOfApplicStagesGet=21;
            expNumOfApplicStages=21;
            numOfValidStagesGet=17;
            expNumOfValidStages=17;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            numOfApplicStagesGet=21;
            expNumOfApplicStages=21;
            numOfValidStagesGet=13;
            expNumOfValidStages=13;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            numOfValidStagesGet=10;
            expNumOfValidStages=10;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            numOfValidStagesGet=8;
            expNumOfValidStages=8;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        /*bind the last uds */
        st = cpssDxChPacketAnalyzerMuxStageBind(managerId,udsId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageBind");

        numOfApplicStagesGet=19;
        expNumOfApplicStages=19;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            numOfValidStagesGet=16;
            expNumOfValidStages=16;
            expValidStages[15]= udsId;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            numOfApplicStagesGet=21;
            expNumOfApplicStages=21;
            numOfValidStagesGet=18;
            expNumOfValidStages=18;
            expValidStages[17]= udsId;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            numOfApplicStagesGet=21;
            expNumOfApplicStages=21;
            numOfValidStagesGet=14;
            expNumOfValidStages=14;
            expValidStages[13]= udsId;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            numOfValidStagesGet=11;
            expNumOfValidStages=11;
            expValidStages[10]= udsId;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            numOfValidStagesGet=9;
            expNumOfValidStages=9;
            expValidStages[8]= udsId;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                     "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                     paStageStrArr[validStagesListArr[ii]],
                                     paStageStrArr[expValidStages[ii]]);
        }

        /*
            1.1.4 Call function after unbinding 3 stages
            Expected: GT_OK and number of valid stages [8] and valid stages as expected.
        */
        st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageUnbind");
        st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageUnbind");
        st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_HEADER_ALTERATION_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageUnbind");

        numOfApplicStagesGet = 20;
        expNumOfApplicStages=19;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            numOfApplicStagesGet=19;
            expNumOfApplicStages=19;
            numOfValidStagesGet=14;
            expNumOfValidStages=14;
            expValidStages[9]= expValidStages[10];
            expValidStages[10]= expValidStages[12];
            expValidStages[11]= expValidStages[13];
            expValidStages[12]= expValidStages[14];
            expValidStages[13]= udsId;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            numOfApplicStagesGet=21;
            expNumOfApplicStages=21;
            numOfValidStagesGet=15;
            expNumOfValidStages=15;
            expValidStages[9]= expValidStages[12];
            expValidStages[10]= expValidStages[13];
            expValidStages[11]= expValidStages[14];
            expValidStages[12]= expValidStages[15];
            expValidStages[13]= expValidStages[16];
            expValidStages[14]= udsId;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            numOfApplicStagesGet=21;
            expNumOfApplicStages=21;
            numOfValidStagesGet=11;
            expNumOfValidStages=11;
            expValidStages[8]= expValidStages[11];
            expValidStages[9]= expValidStages[12];
            expValidStages[10]= udsId;

            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            numOfValidStagesGet=8;
            expNumOfValidStages=8;
            expValidStages[7]= udsId;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            numOfValidStagesGet=7;
            expNumOfValidStages=7;
            expValidStages[6]= udsId;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                         "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                         paStageStrArr[validStagesListArr[ii]],
                                         paStageStrArr[expValidStages[ii]]);
        }

        /*
            1.2. Call function numOfApplicStagesPtr with less than needed .
            Expected: GT_BAD_SIZE.
        */
        numOfApplicStagesGet =18;
        expNumOfApplicStages=19;
        if ((paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) || (paMngDbPtr->devFamily ==  CPSS_PP_FAMILY_DXCH_AC5P_E) || (paMngDbPtr->devFamily ==  CPSS_PP_FAMILY_DXCH_AC5X_E))
            expNumOfApplicStages = 21;
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_SIZE, st);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                             "wrong value, got [%d] while expected [%d]  " ,
                             numOfApplicStagesGet,expNumOfApplicStages);
        /*
            1.3. Call function numOfValidStagesPtr with less than needed .
            Expected: GT_BAD_SIZE.
        */
        numOfApplicStagesGet =22;
        numOfValidStagesGet  =5;
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_SIZE, st);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                             "wrong value, got [%d] while expected [%d]  " ,
                             numOfValidStagesGet,expNumOfValidStages);

        /*
            1.4. Call with NULL numOfApplicStagesPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfValidStagesGet=20;
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             NULL,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.5. Call with NULL applicStagesListArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,NULL,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.6. Call with NULL numOfValidStagesPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             NULL,validStagesListArr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.7. Call with NULL validStagesListArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.8. Call function when manager is not initialized
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_FOUND, st);

    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerGroupCreate
(
    IN GT_U32                                               managerId,
    IN GT_U32                                               groupId,
    IN CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC       *groupAttrPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerGroupCreate)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with groupId [1 / 1024 ]
                            groupAttr["group 1 " ]
    Expected: GT_OK.
    1.2. Call function with groupId [BAD_GROUP_ID_VALUE].
    Expected: GT_BAD_PARAM
    1.3  Call function with groupId already initialized
    Expected: GT_ALREADY_EXIST
    1.4. Call with NULL groupAttrPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              groupId;
    CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC      groupAttr;
    GT_U32                                              managerId = GENERIC_MANAGER_ID;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");

        /*
            1.1.1 Call function with groupId [1 ]
                                    groupAttr["Group Create" ]
            Expected: GT_OK.
        */
        groupId = 1 ;
        cpssOsMemCpy(groupAttr.groupNameArr,"group 1" ,sizeof("group 1"));
        st = cpssDxChPacketAnalyzerGroupCreate(managerId,groupId,&groupAttr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        /*
            1.1.2 Call function with groupId [1024 ]
                                    groupAttr["Group Create" ]
            Expected: GT_OK.
        */
        groupId = 1024 ;
        cpssOsMemCpy(groupAttr.groupNameArr,"group 1024" ,sizeof("group 1024"));
        st = cpssDxChPacketAnalyzerGroupCreate(managerId,groupId,&groupAttr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        /*
            1.2. Call function with groupId [BAD_GROUP_ID_VALUE].
            Expected: GT_BAD_PARAM
        */
        groupId = BAD_GROUP_ID_VALUE ;
        cpssOsMemCpy(groupAttr.groupNameArr,"group 1025" ,sizeof("group 1025"));
        st = cpssDxChPacketAnalyzerGroupCreate(managerId,groupId,&groupAttr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, groupId);

        /*
            1.3  Call function with groupId already initialized
            Expected: GT_ALREADY_EXIST
        */
        groupId = 1024 ;
        cpssOsMemCpy(groupAttr.groupNameArr,"group 1024" ,sizeof("group 1024"));
        st = cpssDxChPacketAnalyzerGroupCreate(managerId,groupId,&groupAttr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_ALREADY_EXIST, st, groupId);

        /*
            1.4. Call with NULL groupAttrPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        groupId = 5 ;
        st = cpssDxChPacketAnalyzerGroupCreate(managerId,groupId,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, groupId);

        /* Restore system. */
        groupId=1;
        st =cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);
        groupId=1024;
        st =cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

    }

#else/* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerGroupDelete
(
    IN GT_U32                                   managerId,
    IN GT_U32                                   groupId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerGroupDelete)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerGroupCreate with groupId [1]
         and delete it
    Expected: GT_OK.
    1.2. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function groupId [2].
    Expected: GT_NOT_FOUND.
    2. fill memory with more than 1 group and delete all
    Expected: GT_OK.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              groupId;
    CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC      groupAttr;
    GT_U32                                              managerId = GENERIC_MANAGER_ID;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function cpssDxChPacketAnalyzerGroupCreate with groupId [1]
                 and delete it
            Expected: GT_OK.
        */
        groupId = 1 ;
        cpssOsMemCpy(groupAttr.groupNameArr,"group 1 to delete" ,sizeof("group 1 to delete"));
        st = cpssDxChPacketAnalyzerGroupCreate(managerId,groupId,&groupAttr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        /*
            1.2. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        groupId = BAD_GROUP_ID_VALUE ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, groupId);

        /*
            1.3. Call function groupId [2].
            Expected: GT_NOT_FOUND.
        */
        groupId = 2 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, groupId);

        /*
            2. fill memory with more than 1 group and delete all
            Expected: GT_OK.
        */
        groupId = 10 ;
        st = cpssDxChPacketAnalyzerGroupCreate(managerId,groupId,&groupAttr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);
        groupId = 4 ;
        st = cpssDxChPacketAnalyzerGroupCreate(managerId,groupId,&groupAttr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);
        groupId = 7 ;
        st = cpssDxChPacketAnalyzerGroupCreate(managerId,groupId,&groupAttr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        groupId = 4 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);
        groupId = 10 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);
        groupId = 7 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerActionCreate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC        *actionPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerActionCreate)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with actionId [1 / 128 ]
                       and valid action
    Expected: GT_OK.
    1.2. Call function with actionId [BAD_ACTION_ID_VALUE].
    Expected: GT_BAD_PARAM
    1.3. Call with action.matchCounterIntThresh.lsb more than 4 bits
    Expected: GT_BAD_PARAM
    1.4. Call with action.matchCounterIntThresh.msb more than 4 bits
    Expected: GT_BAD_PARAM
    1.5. Call with action.matchCounterIntThresh.middle more than 1 bit
    Expected: GT_BAD_PARAM
    1.6. Call with action.interruptSamplingEnable = GT_TRUE
    Expected: GT_NOT_SUPPORTED
    1.7. Call with action.interruptMatchCounterEnable = GT_TRUE
    Expected: GT_NOT_SUPPORTED
    1.8  Call function with actionId already initialized
    Expected: GT_ALREADY_EXIST
    1.9. Call with NULL action and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              actionId;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                action;
    GT_U32                                              managerId = GENERIC_MANAGER_ID;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1.1 Call function with actionId [1]
                               and valid action
            Expected: GT_OK.
        */
        actionId = 1 ;
        action.samplingEnable = GT_TRUE ;
        action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E ;
        action.matchCounterEnable = GT_TRUE ;
        action.matchCounterIntThresh.lsb = 0x4;
        action.matchCounterIntThresh.middle =0x1;
        action.matchCounterIntThresh.msb = 0xa;
        action.interruptSamplingEnable = GT_FALSE ;
        action.interruptMatchCounterEnable = GT_FALSE ;
        action.inverseEnable = GT_TRUE ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.1.1 Call function with actionId [128]
                               and valid action
            Expected: GT_OK
        */
        actionId = 128 ;
        action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E ;
        action.matchCounterIntThresh.lsb = 0xb;
        action.matchCounterIntThresh.middle =0x0;
        action.matchCounterIntThresh.msb = 0x7;
        action.inverseEnable = GT_FALSE ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.2. Call function with actionId [BAD_ACTION_ID_VALUE].
            Expected: GT_BAD_PARAM
        */
        actionId = BAD_ACTION_ID_VALUE ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
            1.3. Call with action.matchCounterIntThresh.lsb more than 4 bits
            Expected: GT_BAD_PARAM
        */
        actionId = 2 ;
        action.matchCounterIntThresh.lsb = 0x20;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
        1.4. Call with action.matchCounterIntThresh.msb more than 4 bits
        Expected: GT_BAD_PARAM
        */
        action.matchCounterIntThresh.lsb = 0x3;
        action.matchCounterIntThresh.msb = 0x17;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
        1.5. Call with action.matchCounterIntThresh.middle more than 1 bit
        Expected: GT_BAD_PARAM
        */
        action.matchCounterIntThresh.msb = 0x6;
        action.matchCounterIntThresh.middle =0x2;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
            1.6. Call with action.interruptSamplingEnable = GT_TRUE
            Expected: GT_NOT_SUPPORTED
        */
        action.matchCounterIntThresh.middle =0x1;
        action.interruptSamplingEnable = GT_TRUE;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, actionId);

        /*
            1.7. Call with action.interruptMatchCounterEnable = GT_TRUE.
            Expected: GT_NOT_SUPPORTED
        */
        action.interruptSamplingEnable = GT_FALSE;
        action.interruptMatchCounterEnable = GT_TRUE;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, actionId);

        /*
            1.8  Call function with groupId already initialized
            Expected: GT_ALREADY_EXIST
        */
        action.interruptMatchCounterEnable = GT_FALSE;
        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_ALREADY_EXIST, st, actionId);

        /*
            1.9. Call with NULL groupAttrPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        actionId = 2 ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, actionId);

        /* Restore system. */

        actionId=1;
        st =cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        actionId=128;
        st =cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerActionGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    OUT  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC       *actionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerActionGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with actionId [3]
    Expected: GT_OK and value as set .
    1.2. Call function with actionId [BAD_ACTION_ID_VALUE].
    Expected: GT_BAD_PARAM
    1.3. Call function with actionId not initialized [3].
    Expected: GT_NOT_FOUND
    1.4. Call with NULL action and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              actionId;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                action;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                actionGet;
    GT_U32                                              managerId = GENERIC_MANAGER_ID;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*set action*/
        actionId = 3 ;
        action.samplingEnable = GT_TRUE ;
        action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E ;
        action.matchCounterEnable = GT_TRUE ;
        action.matchCounterIntThresh.lsb = 0x2;
        action.matchCounterIntThresh.middle =0x1;
        action.matchCounterIntThresh.msb = 0xa;
        action.interruptSamplingEnable = GT_FALSE ;
        action.interruptMatchCounterEnable = GT_FALSE ;
        action.inverseEnable = GT_TRUE ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);
        /*
            1.1 Call function with actionId [3]
            Expected: GT_OK and value as set .
        */
        st = cpssDxChPacketAnalyzerActionGet(managerId,actionId,&actionGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.samplingEnable, actionGet.samplingEnable,
                       "got other action.samplingEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.samplingMode, actionGet.samplingMode,
                       "got other action.samplingMode than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterEnable, actionGet.matchCounterEnable,
                       "got other action.matchCounterEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterIntThresh.lsb, actionGet.matchCounterIntThresh.lsb,
                       "got other action.matchCounterIntThresh.lsb than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterIntThresh.middle, actionGet.matchCounterIntThresh.middle,
                       "got other action.matchCounterIntThresh.middle than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterIntThresh.msb, actionGet.matchCounterIntThresh.msb,
                       "got other action.matchCounterIntThresh.msb than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.interruptSamplingEnable, actionGet.interruptSamplingEnable,
                       "got other action.interruptSamplingEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.interruptMatchCounterEnable, actionGet.interruptMatchCounterEnable,
                       "got other action.interruptMatchCounterEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.inverseEnable, actionGet.inverseEnable,
                       "got other action.inverseEnable than was set: %d", actionId);

        /*
            1.2. Call function with actionId [BAD_ACTION_ID_VALUE].
            Expected: GT_BAD_PARAM
        */
        actionId = BAD_ACTION_ID_VALUE ;
        st = cpssDxChPacketAnalyzerActionGet(managerId,actionId,&actionGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
            1.3. Call function with actionId not initialized [3].
            Expected: GT_NOT_FOUND
        */
        actionId = 7 ;
        st = cpssDxChPacketAnalyzerActionGet(managerId,actionId,&actionGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, actionId);

        /*
            1.4. Call with NULL action and other valid params.
            Expected: GT_BAD_PTR.
        */
        actionId = 3 ;
        st = cpssDxChPacketAnalyzerActionGet(managerId,actionId,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, actionId);

        /* Restore system. */

        actionId=3;
        st =cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerActionUpdate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC        *actionPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerActionUpdate)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with actionId exist and change all parms
    Expected: GT_OK and value as set at update .
    1.2. Call function with actionId [BAD_ACTION_ID_VALUE].
    Expected: GT_BAD_PARAM
    1.3. Call with action.matchCounterIntThresh.lsb more than 4 bits
    Expected: GT_BAD_PARAM
    1.4. Call with action.matchCounterIntThresh.msb more than 4 bits
    Expected: GT_BAD_PARAM
    1.5. Call with action.matchCounterIntThresh.middle more than 1 bit
    Expected: GT_BAD_PARAM
    1.6. Call with action.interruptSamplingEnable = GT_TRUE
    Expected: GT_NOT_SUPPORTED
    1.7. Call with action.interruptMatchCounterEnable = GT_TRUE
    Expected: GT_NOT_SUPPORTED
    1.8  Call function with actionId not initialized
    Expected: GT_NOT_FOUND
    1.9. Call with NULL action and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              actionId;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                action;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                actionGet;
    GT_U32                                              managerId = GENERIC_MANAGER_ID;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function with actionId exist and change all parms
            Expected: GT_OK and value as set at update .
        */

        /*set action */
        actionId = 1 ;
        action.samplingEnable = GT_TRUE ;
        action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E ;
        action.matchCounterEnable = GT_TRUE ;
        action.matchCounterIntThresh.lsb = 0x4;
        action.matchCounterIntThresh.middle =0x1;
        action.matchCounterIntThresh.msb = 0xa;
        action.interruptSamplingEnable = GT_FALSE ;
        action.interruptMatchCounterEnable = GT_FALSE ;
        action.inverseEnable = GT_TRUE ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        st = cpssDxChPacketAnalyzerActionGet(managerId,actionId,&actionGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.samplingEnable, actionGet.samplingEnable,
                       "got other action.samplingEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.samplingMode, actionGet.samplingMode,
                       "got other action.samplingMode than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterEnable, actionGet.matchCounterEnable,
                       "got other action.matchCounterEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterIntThresh.lsb, actionGet.matchCounterIntThresh.lsb,
                       "got other action.matchCounterIntThresh.lsb than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterIntThresh.middle, actionGet.matchCounterIntThresh.middle,
                       "got other action.matchCounterIntThresh.middle than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterIntThresh.msb, actionGet.matchCounterIntThresh.msb,
                       "got other action.matchCounterIntThresh.msb than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.interruptSamplingEnable, actionGet.interruptSamplingEnable,
                       "got other action.interruptSamplingEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.interruptMatchCounterEnable, actionGet.interruptMatchCounterEnable,
                       "got other action.interruptMatchCounterEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.inverseEnable, actionGet.inverseEnable,
                       "got other action.inverseEnable than was set: %d", actionId);

        /*update action */

        actionId = 1 ;
        action.samplingEnable = GT_FALSE ;
        action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E ;
        action.matchCounterEnable = GT_FALSE ;
        action.matchCounterIntThresh.lsb = 0x2;
        action.matchCounterIntThresh.middle =0x0;
        action.matchCounterIntThresh.msb = 0x9;
        action.interruptSamplingEnable = GT_FALSE ;
        action.interruptMatchCounterEnable = GT_FALSE ;
        action.inverseEnable = GT_FALSE ;
        st = cpssDxChPacketAnalyzerActionUpdate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        st = cpssDxChPacketAnalyzerActionGet(managerId,actionId,&actionGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.samplingEnable, actionGet.samplingEnable,
                       "got other action.samplingEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.samplingMode, actionGet.samplingMode,
                       "got other action.samplingMode than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterEnable, actionGet.matchCounterEnable,
                       "got other action.matchCounterEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterIntThresh.lsb, actionGet.matchCounterIntThresh.lsb,
                       "got other action.matchCounterIntThresh.lsb than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterIntThresh.middle, actionGet.matchCounterIntThresh.middle,
                       "got other action.matchCounterIntThresh.middle than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.matchCounterIntThresh.msb, actionGet.matchCounterIntThresh.msb,
                       "got other action.matchCounterIntThresh.msb than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.interruptSamplingEnable, actionGet.interruptSamplingEnable,
                       "got other action.interruptSamplingEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.interruptMatchCounterEnable, actionGet.interruptMatchCounterEnable,
                       "got other action.interruptMatchCounterEnable than was set: %d", actionId);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(action.inverseEnable, actionGet.inverseEnable,
                       "got other action.inverseEnable than was set: %d", actionId);
        /*
            1.2. Call function with actionId [BAD_ACTION_ID_VALUE].
            Expected: GT_BAD_PARAM
        */
        actionId = BAD_ACTION_ID_VALUE ;
        st = cpssDxChPacketAnalyzerActionUpdate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
            1.3. Call with action.matchCounterIntThresh.lsb more than 4 bits
            Expected: GT_BAD_PARAM
        */
        actionId = 1 ;
        action.matchCounterIntThresh.lsb = 0x20;
        st = cpssDxChPacketAnalyzerActionUpdate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
        1.4. Call with action.matchCounterIntThresh.msb more than 4 bits
        Expected: GT_BAD_PARAM
        */
        action.matchCounterIntThresh.lsb = 0x3;
        action.matchCounterIntThresh.msb = 0x17;
        st = cpssDxChPacketAnalyzerActionUpdate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
        1.5. Call with action.matchCounterIntThresh.middle more than 1 bit
        Expected: GT_BAD_PARAM
        */
        action.matchCounterIntThresh.msb = 0x6;
        action.matchCounterIntThresh.middle =0x2;
        st = cpssDxChPacketAnalyzerActionUpdate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
            1.6. Call with action.interruptSamplingEnable = GT_TRUE
            Expected: GT_NOT_SUPPORTED
        */
        action.matchCounterIntThresh.middle =0x1;
        action.interruptSamplingEnable = GT_TRUE;
        st = cpssDxChPacketAnalyzerActionUpdate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, actionId);

        /*
            1.7. Call with action.interruptMatchCounterEnable = GT_TRUE.
            Expected: GT_NOT_SUPPORTED
        */
        action.interruptSamplingEnable = GT_FALSE;
        action.interruptMatchCounterEnable = GT_TRUE;
        st = cpssDxChPacketAnalyzerActionUpdate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, actionId);

        /*
            1.8  Call function with actionId not initialized
            Expected: GT_NOT_FOUND
        */
        action.interruptMatchCounterEnable = GT_FALSE;
        actionId = 5 ;
        st = cpssDxChPacketAnalyzerActionUpdate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, actionId);

        /*
            1.9. Call with NULL groupAttrPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionUpdate(managerId,actionId,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, actionId);

        /* Restore system. */

        st =cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerActionDelete
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  actionId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerActionDelete)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerActionCreate with groupId [1]
         and delete it
    Expected: GT_OK.
    1.2. Call function with wrong value actionId [BAD_ACTION_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function actionId [2].
    Expected: GT_NOT_FOUND.
    2. fill memory with more than 1 action and delete all
    Expected: GT_OK.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              actionId;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                action;
    GT_U32                                              managerId = GENERIC_MANAGER_ID;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function cpssDxChPacketAnalyzerActionCreate with groupId [1]
                 and delete it
            Expected: GT_OK.
        */
        /*make actionId*/
        actionId = 1 ;
        action.samplingEnable = GT_TRUE ;
        action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E ;
        action.matchCounterEnable = GT_TRUE ;
        action.matchCounterIntThresh.lsb = 0x4;
        action.matchCounterIntThresh.middle =0x1;
        action.matchCounterIntThresh.msb = 0xa;
        action.interruptSamplingEnable = GT_FALSE ;
        action.interruptMatchCounterEnable = GT_FALSE ;
        action.inverseEnable = GT_TRUE ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.2. Call function with wrong value actionId [BAD_ACTION_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        actionId = BAD_ACTION_ID_VALUE ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
            1.3. Call function actionId [2].
            Expected: GT_NOT_FOUND.
        */
        actionId = 2 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, actionId);

        /*
            2. fill memory with more than 1 action and delete all
            Expected: GT_OK.
        */
        actionId = 2 ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);
        actionId = 6 ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);
        actionId = 9 ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        actionId = 6 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);
        actionId = 2 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);
        actionId = 9 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerGroupActivateEnableSet
(
    IN  GT_U32                                  managerId,
    IN  GT_U32                                  groupId,
    IN  GT_BOOL                                 enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerGroupActivateEnableSet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerGroupCreate
                       and enable it
    Expected: GT_OK.
    1.2. Call function cpssDxChPacketAnalyzerGroupCreate
                       and disable it
    Expected: GT_OK.
    1.3. Call function on group that already enabled
    Expected: GT_BAD_STATE.
    1.4. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.5. Call function groupId not created [2].
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;
    GT_U32                                              keyId;

    GT_U32                                              groupId;
    GT_U32                                              actionId;
    GT_U32                                              ruleId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

       /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId = 1;
        st = genericKeyInitSimple(managerId, keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /* initialized group ,action and rule*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericGroupInit ");
        actionId = 1;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericActionInit ");
        ruleId = 1 ;
        st = genericRuleInit(managerId,keyId,groupId,actionId,ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericRuleInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /*
            1.1. Call function cpssDxChPacketAnalyzerGroupCreate
                               and enable it
            Expected: GT_OK.
        */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        /*
            1.2. Call function cpssDxChPacketAnalyzerGroupCreate
                               and disable it
            Expected: GT_OK.
        */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId,GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        /*
            1.3. Call function on group that already enabled
            Expected: GT_BAD_STATE.
        */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, groupId);

        /*
            1.4. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        groupId = BAD_GROUP_ID_VALUE ;
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, groupId);

        /*
            1.5. Call function groupId not created [2].
            Expected: GT_NOT_FOUND.
        */
        groupId = 2 ;
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, groupId);

        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /* Restore system. */
        managerId = GENERIC_MANAGER_ID ;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerActionSamplingEnableSet
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      actionId,
    IN  GT_BOOL                                     enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerActionSamplingEnableSet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerActionCreate
                       and enable it
    Expected: GT_OK.
    1.2. Call function cpssDxChPacketAnalyzerActionCreate
                       and disable it
    Expected: GT_OK.
    1.3. Call function with wrong value actionId [BAD_ACTION_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.4. Call function actionId not created [2].
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              actionId;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                action;
    GT_U32                                              managerId = GENERIC_MANAGER_ID;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1. Call function cpssDxChPacketAnalyzerActionCreate
                               and enable it
            Expected: GT_OK.
        */
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");

        actionId = 1 ;
        cpssOsMemSet(&action, 0, sizeof(action));
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId,&action);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId,actionId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.2. Call function cpssDxChPacketAnalyzerActionCreate
                               and disable it
        */
        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId,actionId,GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);
        /*
            1.3. Call function with wrong value actionId [BAD_ACTION_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        actionId = BAD_ACTION_ID_VALUE ;
        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId,actionId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, actionId);

        /*
            1.4. Call function actionId not created [2].
            Expected: GT_NOT_FOUND.
        */
        actionId = 2 ;
        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId,actionId,GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, actionId);

        /* Restore system. */
        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerGroupRuleAdd
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          keyId,
    IN  GT_U32                                          groupId,
    IN  GT_U32                                          ruleId,
    IN  CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC   *ruleAttrPtr,
    IN  GT_U32                                          numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC       fieldsValueArr[],
    IN  GT_U32                                          actionId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerGroupRuleAdd)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function after initialized manager,device,key,group and action
            with keyId [1/2]
            ruleId [1/2048]
            ruleAttr ["rule add "]
            ruleNumOfFields[1/3]
            fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,0,0 /
                            CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,1,1
                            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,1,1,
                            CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E,1,1]
    Expected: GT_OK.
    1.2. Call function after initialized manager,device,key,group and action
            keyid [1]
            ruleId [2040]
            ruleAttr ["rule add "]
            ruleNumOfFields[2]
            fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,0,0
                            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,0,0]
    Expected: GT_BAD_PARAM - cant do 2 rules on same key (rule 1 and rule 2040)
    1.3. Call function after initialized manager,device,key,group and action
            with ruleId [2]
            ruleAttr ["rule add "]
            ruleNumOfFields[1]
            fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_DSCP_E,0,0]
    Expected: GT_FAIL - field that doesn't initialized at key.
    1.4. Call function with keyId [BAD_KEY_ID_VALUE] and all other valid value
    Expected: GT_BAD_PARAM.
    1.5. Call function with groupId [BAD_GROUP_ID_VALUE] and all other valid value
    Expected: GT_BAD_PARAM.
    1.6. Call function with ruleId [BAD_RULE_ID_VALUE] and all other valid value
    Expected: GT_BAD_PARAM.
    1.7. Call function with actionId [BAD_ACTION_ID_VALUE] and all other valid value
    Expected: GT_BAD_PARAM.
    1.8. Call with NULL ruleAttr and other valid params.
    Expected: GT_BAD_PTR.
    1.9. Call with NULL fieldsValueArr and other valid params.
    Expected: GT_BAD_PTR.
    1.10. Call function groupId that not initialized .
    Expected: GT_NOT_FOUND.
    1.11. Call function actionId that not initialized .
    Expected: GT_NOT_FOUND.
    1.12. Call function with keyId [2] that not created and all other valid value
    Expected: GT_FAIL.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_U32                                               keyId;

    GT_U32                                               groupId;
    GT_U32                                               actionId;
    GT_U32                                               ruleId;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC        ruleAttr;
    GT_U32                                               ruleNumOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC            fieldsValueArr[SIZE_OF_TEST_ARR];


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsMemSet(fieldsValueArr, 0, sizeof(fieldsValueArr));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=1;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /* initialized Key2*/
        keyId=2;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /* initialized group and action*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");
        groupId = 3;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");

        actionId = 1;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericActionInit ");
        /*
            1.1.1 Call function after initialized manager,device,key,group and action
                      with ruleId [1]
                      ruleAttr ["rule add "]
                      ruleNumOfFields[1]
                      fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,0,0]
            Expected: GT_OK.
        */
        groupId = 1;
        keyId=1;
        ruleId = 1 ;
        cpssOsMemCpy(ruleAttr.ruleNameArr,"rule 1" ,sizeof("rule 1"));
        ruleNumOfFields = 1;

        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        cpssOsMemSet(fieldsValueArr[0].data, 0, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[0].msk, 0, sizeof(fieldsValueArr[0].msk));

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, keyId,groupId,ruleId);


        /*
            1.1.2 Call function after initialized manager,device,key,group and action
                    with ruleId [2048]
                    ruleAttr ["rule add "]
                    ruleNumOfFields[2]
                    fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,1,1
                                    CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,1,1,
                                    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E,1,1]
            Expected: GT_OK.
        */
        groupId = 3;
        keyId = 2 ;
        ruleId = 2048 ;
        cpssOsMemCpy(ruleAttr.ruleNameArr,"rule 2048" ,sizeof("rule 2048"));
        ruleNumOfFields = 3;

        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        cpssOsMemSet(fieldsValueArr[0].data, 1, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[0].msk, 1, sizeof(fieldsValueArr[0].msk));

        fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        cpssOsMemSet(fieldsValueArr[1].data, 1, sizeof(fieldsValueArr[1].data));
        cpssOsMemSet(fieldsValueArr[1].msk, 1, sizeof(fieldsValueArr[1].msk));

        fieldsValueArr[2].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E;
        cpssOsMemSet(fieldsValueArr[2].data, 1, sizeof(fieldsValueArr[2].data));
        cpssOsMemSet(fieldsValueArr[2].msk, 1, sizeof(fieldsValueArr[2].msk));

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, keyId,groupId,ruleId);

        /*clear rule 2048 */
        ruleId = 2048;
        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st,ruleId,groupId);
        /*
            1.2. Call function after initialized manager,device,key,group and action
            keyid [1]
            ruleId [2040]
            ruleAttr ["rule add "]
            ruleNumOfFields[2]
            fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,0,0
                            CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,0,0]
            Expected: GT_BAD_PARAM - cant do 2 rules on same key (rule 1 and rule 2040)
        */
        groupId = 1;
        keyId = 1 ;
        ruleId = 2040 ;
        cpssOsMemCpy(ruleAttr.ruleNameArr,"rule 2040" ,sizeof("rule 2040"));
        ruleNumOfFields = 2;
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        cpssOsMemSet(fieldsValueArr[0].data, 1, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[0].msk, 1, sizeof(fieldsValueArr[0].msk));

        fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        cpssOsMemSet(fieldsValueArr[1].data, 1, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[1].msk, 1, sizeof(fieldsValueArr[0].msk));

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId,groupId,ruleId);

        /*
            1.3. Call function after initialized manager,device,key,group and action
                    with ruleId [2]
                    ruleAttr ["rule add "]
                    ruleNumOfFields[1]
                    fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_DSCP_E,0,0]
            Expected: GT_BAD_PARAM - field that doesn't initialized at key.
        */
        keyId = 2 ;
        ruleId = 2 ;
        cpssOsMemCpy(ruleAttr.ruleNameArr,"rule 2" ,sizeof("rule 2"));
        ruleNumOfFields = 1;
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_DSCP_E;
        cpssOsMemSet(fieldsValueArr[0].data, 0, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[0].msk, 0, sizeof(fieldsValueArr[0].msk));

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId,groupId,ruleId);

        /*
            1.4. Call function with keyId [BAD_KEY_ID_VALUE] and all other valid value
            Expected: GT_BAD_PARAM.
        */
        ruleNumOfFields = 4;
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        keyId = BAD_KEY_ID_VALUE;
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId,groupId,ruleId);

        /*
            1.5. Call function with groupId [BAD_GROUP_ID_VALUE] and all other valid value
            Expected: GT_BAD_PARAM.
        */
        keyId = 2;
        groupId = BAD_GROUP_ID_VALUE;
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId,groupId,ruleId);

        /*
            1.6. Call function with ruleId [BAD_RULE_ID_VALUE] and all other valid value
            Expected: GT_BAD_PARAM.
        */
        groupId = 1;
        ruleId = BAD_RULE_ID_VALUE;
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId, groupId, ruleId, &ruleAttr, ruleNumOfFields, fieldsValueArr, actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId, groupId, ruleId);

        /*
            1.7. Call function with actionId [BAD_ACTION_ID_VALUE] and all other valid value
            Expected: GT_BAD_PARAM.
        */
        ruleId = 2;
        actionId = BAD_ACTION_ID_VALUE;
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId,groupId,ruleId);

        /*
            1.8. Call with NULL ruleAttr and other valid params.
            Expected: GT_BAD_PTR.
        */
        actionId = 1;
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,NULL,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId,groupId,ruleId);

        /*
            1.9. Call with NULL fieldsValueArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,NULL,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId,groupId,ruleId);

        /*
            1.10. Call function groupId that not initialized .
            Expected: GT_NOT_FOUND.
        */
        groupId = 10 ;
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId,groupId,ruleId);

        /*
            1.11. Call function actionId that not initialized .
            Expected: GT_NOT_FOUND.
        */
        groupId = 1 ;
        actionId = 10 ;
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId,groupId,ruleId);

        /*
            1.12. Call function with keyId [2] that not created and all other valid value
            Expected: GT_NOT_FOUND.
        */
        actionId = 1 ;
        keyId = 3 ;
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId,groupId,ruleId,&ruleAttr,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId,groupId,ruleId);

        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        groupId = 3 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /* Restore system. */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerGroupRuleUpdate
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId,
    IN  GT_U32                                      numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC   fieldsValueArr[],
    IN  GT_U32                                      actionId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerGroupRuleUpdate)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1.1 Call function after initialized manager,device,key,group, action
                       and rule with ruleId [1]
                                     groupId [1]
                                     actionId [1]
                                     ruleNumOfFields[1/2]
            fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E/
                            CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E
                            CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,
                            CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E]
    Expected: GT_OK.
    1.2 Call function with ruleId [1]
                      groupId [1]
                      ruleNumOfFields[1]
                      actionId[1]
            fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_DSCP_E,0,0xFFFFFFFF]
    Expected: GT_BAD_PARAM - field that doesn't initialized at ruleId.

    1.3. Call function with groupId [BAD_GROUP_ID_VALUE] and all other valid value
    Expected: GT_BAD_PARAM.
    1.4. Call function with ruleId [BAD_RULE_ID_VALUE] and all other valid value
    Expected: GT_BAD_PARAM.
    1.5. Call function with actionId [BAD_ACTION_ID_VALUE] and all other valid value
    Expected: GT_BAD_PARAM.
    1.6. Call with NULL fieldsValueArr and other valid params.
    Expected: GT_BAD_PTR.
    1.7. Call function groupId that not initialized .
    Expected: GT_NOT_FOUND.
    1.8. Call function actionId that not initialized .
    Expected: GT_NOT_FOUND.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_U32                                               keyId;

    GT_U32                                               groupId;
    GT_U32                                               actionId;
    GT_U32                                               ruleId;
    GT_U32                                               ruleNumOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC            fieldsValueArr[SIZE_OF_TEST_ARR];


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsMemSet(fieldsValueArr, 0, sizeof(fieldsValueArr));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=1;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /* initialized group and action*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");
        actionId = 1;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericActionInit ");
        /* initialized rule*/
        ruleId = 1;
        st = genericRuleInit(managerId,keyId, groupId, actionId, ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericRuleInit ");

        /*
            1.1.1 Call function after initialized manager,device,key,group, action
                               and rule with ruleId [1]
                                             groupId [1]
                                             actionId [1]
                                             ruleNumOfFields[1]
                    fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,0xABABABAB,0xABABABAB]
            Expected: GT_OK.
        */

        ruleNumOfFields = 1;
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        cpssOsMemSet(fieldsValueArr[0].data, 0xAB, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[0].msk, 0xAB, sizeof(fieldsValueArr[0].msk));

        st = cpssDxChPacketAnalyzerGroupRuleUpdate(managerId,ruleId,groupId,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, keyId,groupId,ruleId);

        /*
            1.1.2 Call function after initialized manager,device,key,group, action
                               and rule with ruleId [1]
                                             groupId [1]
                                             actionId [1]
                                             ruleNumOfFields[2]
                    fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E
                                    CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,
                                    CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E]
            Expected: GT_OK.
        */
        ruleNumOfFields = 3;
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        fieldsValueArr[2].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_0_E;
        cpssOsMemSet(fieldsValueArr[0].data, 0, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[0].msk, 0, sizeof(fieldsValueArr[0].msk));
        cpssOsMemSet(fieldsValueArr[1].data, 1, sizeof(fieldsValueArr[1].data));
        cpssOsMemSet(fieldsValueArr[1].msk, 1, sizeof(fieldsValueArr[1].msk));
        cpssOsMemSet(fieldsValueArr[2].data, 0xAB, sizeof(fieldsValueArr[2].data));
        cpssOsMemSet(fieldsValueArr[2].msk, 0xAB, sizeof(fieldsValueArr[2].msk));

        st = cpssDxChPacketAnalyzerGroupRuleUpdate(managerId,ruleId,groupId,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, keyId,groupId,ruleId);
        /*
            1.2 Call function with ruleId [1]
                              groupId [1]
                              ruleNumOfFields[1]
                              actionId[1]
                    fieldsValueArr [CPSS_DXCH_PACKET_ANALYZER_FIELD_DSCP_E,0,0xFFFFFFFF]
            Expected: GT_BAD_PARAM - field that doesn't initialized at ruleId.
        */
        ruleNumOfFields = 1;
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_DSCP_E;
        cpssOsMemSet(fieldsValueArr[0].data, 0, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[0].msk, 1, sizeof(fieldsValueArr[0].msk));

        st = cpssDxChPacketAnalyzerGroupRuleUpdate(managerId,ruleId,groupId,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId,groupId,ruleId);

        /*
            1.3. Call function with groupId [BAD_GROUP_ID_VALUE] and all other valid value
            Expected: GT_BAD_PARAM.
        */
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        groupId = BAD_GROUP_ID_VALUE;
        st = cpssDxChPacketAnalyzerGroupRuleUpdate(managerId,ruleId,groupId,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId,groupId,ruleId);

        /*
            1.4. Call function with ruleId [BAD_RULE_ID_VALUE] and all other valid value
            Expected: GT_BAD_PARAM.
        */
        groupId = 1;
        ruleId = BAD_RULE_ID_VALUE;
        st = cpssDxChPacketAnalyzerGroupRuleUpdate(managerId,ruleId, groupId, ruleNumOfFields, fieldsValueArr, actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId, groupId, ruleId);

        /*
            1.5. Call function with actionId [BAD_ACTION_ID_VALUE] and all other valid value
            Expected: GT_BAD_PARAM.
        */
        ruleId = 1;
        actionId = BAD_ACTION_ID_VALUE;
        st = cpssDxChPacketAnalyzerGroupRuleUpdate(managerId,ruleId,groupId,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PARAM, st, devNum, keyId,groupId,ruleId);

        /*restore*/
        ruleNumOfFields = 1;
        /*
            1.6. Call with NULL fieldsValueArr and other valid params.
            Expected: GT_BAD_PTR.                                     .
        */
        st = cpssDxChPacketAnalyzerGroupRuleUpdate(managerId,ruleId,groupId,ruleNumOfFields,NULL,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_BAD_PTR, st, devNum, keyId,groupId,ruleId);

        /*
            1.7. Call function groupId that not initialized .
            Expected: GT_NOT_FOUND.
        */
        groupId = 10 ;
        st = cpssDxChPacketAnalyzerGroupRuleUpdate(managerId,ruleId,groupId,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId,groupId,ruleId);
        /*
            1.8. Call function actionId that not initialized .
            Expected: GT_NOT_FOUND.
        */
        groupId = 1 ;
        actionId = 10 ;
        st = cpssDxChPacketAnalyzerGroupRuleUpdate(managerId,ruleId,groupId,ruleNumOfFields,fieldsValueArr,actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_NOT_FOUND, st, devNum, keyId,groupId,ruleId);

        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /* Restore system. */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

    }
#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerGroupRuleDelete
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerGroupRuleDelete)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerGroupRuleAdd with ruleId [1]
         and delete it
    Expected: GT_OK.
    1.2. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function groupId [2].
    Expected: GT_NOT_FOUND.

    2. fill memory with more than 1 rule and delete all
    Expected: GT_OK.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_U32                                               keyId;

    GT_U32                                               groupId;
    GT_U32                                               actionId;
    GT_U32                                               ruleId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=1;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /* initialized group and action*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");
        actionId = 1;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericActionInit ");
        /* initialized rule*/
        ruleId = 1;
        st = genericRuleInit(managerId,keyId, groupId, actionId, ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericRuleInit ");


        /*
            1.1. Call function cpssDxChPacketAnalyzerGroupRuleAdd with ruleId [1]
                 and delete it
            Expected: GT_OK.
        */
        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, ruleId,groupId);

        /*
            1.2. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = BAD_RULE_ID_VALUE;
        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,ruleId, groupId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.3. Call function groupId [2].
            Expected: GT_NOT_FOUND.
        */
        ruleId = 1;
        groupId = 2 ;
        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId,groupId);

        /*
            2. fill memory with more than 1 rule and delete all
            Expected: GT_OK.
        */
        groupId = 1 ;
        ruleId = 10;
        st = genericRuleInit(managerId,keyId, groupId, actionId, ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericRuleInit ");
        /* initialized Keys and rules*/
        keyId=3;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        groupId = 3;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");
        ruleId = 3;
        st = genericRuleInit(managerId,keyId, groupId, actionId, ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericRuleInit ");
        keyId=5;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        groupId = 9;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");
        ruleId = 1;
        st = genericRuleInit(managerId,keyId, groupId, actionId, ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericRuleInit ");

        /*delete rules*/
        ruleId = 10 ;
        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, ruleId,groupId);
        ruleId = 1 ;
        groupId = 9 ;
        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, ruleId,groupId);
        ruleId = 3 ;
        groupId = 3 ;
        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, ruleId,groupId);

         groupId = 3 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);
        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);
        groupId = 9 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*restore system*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerSampledDataCountersClear
(
    IN  GT_U32                                      managerId,
    IN  GT_U32                                      ruleId,
    IN  GT_U32                                      groupId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerSampledDataCountersClear)
{
#ifndef GM_USED
/*

    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function after initialized manager,device,key,group and action
                        with valid ruleId[1]
                                   groupId[1]
    Expected: GT_OK.
    1.2. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.4. Call function with not initialized ruleId [2]
    Expected: GT_NOT_FOUND.
    1.5. Call function with not initialized groupId [2]
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_U32                                               keyId;

    GT_U32                                               groupId;
    GT_U32                                               actionId;
    GT_U32                                               ruleId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId = 1;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /* initialized group action and rule*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");
        actionId = 1 ;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericActionInit ");
        ruleId = 1 ;
        st = genericRuleInit(managerId,keyId, groupId, actionId, ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericRuleInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /*Action sampling disable*/
        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId,actionId,GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.1. Call function after initialized manager,device,key,group and action
                                with valid ruleId[1]
                                           groupId[1]
            Expected: GT_OK.
        */
        st = cpssDxChPacketAnalyzerSampledDataCountersClear(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, managerId,groupId ,ruleId);

        /*
            1.2. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = BAD_RULE_ID_VALUE;
        st = cpssDxChPacketAnalyzerSampledDataCountersClear(managerId,ruleId, groupId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, managerId, groupId, ruleId);

        /*
            1.3. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = 1;
        groupId = BAD_GROUP_ID_VALUE;
        st = cpssDxChPacketAnalyzerSampledDataCountersClear(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, managerId,groupId ,ruleId);
        /*
            1.4. Call function with not initialized ruleId [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 2;
        groupId = 1 ;
        st = cpssDxChPacketAnalyzerSampledDataCountersClear(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_FOUND, st, managerId,groupId ,ruleId);
        /*
            1.5. Call function with not initialized groupId [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 1;
        groupId = 2;
        st = cpssDxChPacketAnalyzerSampledDataCountersClear(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_FOUND, st, managerId,groupId ,ruleId);

        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /* Restore system. */
        managerId = GENERIC_MANAGER_ID ;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerGroupRuleGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    OUT   GT_U32                                        *keyIdPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC *ruleAttrPtr,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     fieldsValueArr[],
    OUT   GT_U32                                        *actionIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerGroupRuleGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerGroupRuleGet with
         valid values
    Expected: GT_OK and same values as written at init stage
    1.2. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong value ruleId [2]
    Expected: GT_NOT_FOUND.
    1.5. Call function with wrong value groupId [2]
    Expected: GT_NOT_FOUND.
    1.6. Call function with numOfFields less than init number [1]
    Expected: GT_BAD_SIZE.
    1.7. Call with NULL keyIdPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.8. Call with NULL numOfFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.9. Call with NULL fieldsValueArr and other valid params.
    Expected: GT_BAD_PTR.
    1.10. Call with NULL actionIdPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.11. Call Call function when manager is not initialized
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_U32                                               keyId;

    GT_U32                                               groupId;
    GT_U32                                               actionId;
    GT_U32                                               ruleId;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC        ruleAttr;
    GT_U32                                               ruleNumOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC            fieldsValueArr[SIZE_OF_TEST_ARR];

    GT_U32                                               ii,jj;
    GT_U32                                               keyIdGet;
    GT_U32                                               actionIdGet;
    GT_U32                                               ruleNumOfFieldsGet;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC            fieldsValueArrGet[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC        ruleAttrGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        /* initialized managerId with device*/
        cpssOsMemSet(fieldsValueArr, 0, sizeof(fieldsValueArr));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId = 1;
        st = genericKeyInitSimple(managerId, keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericKeyInit ");
        /* initialized group and action*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericGroupInit ");
        actionId = 1;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericActionInit ");

        ruleId = 1 ;
        cpssOsMemCpy(ruleAttr.ruleNameArr,"rule 1" ,sizeof("rule 1"));
        ruleNumOfFields = 4;
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        cpssOsMemSet(fieldsValueArr[0].data, 1, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[0].msk, 1, sizeof(fieldsValueArr[0].msk));

        fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        cpssOsMemSet(fieldsValueArr[1].data, 0, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[1].msk, 0, sizeof(fieldsValueArr[0].msk));

        fieldsValueArr[2].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        cpssOsMemSet(fieldsValueArr[2].data, 0xC, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[2].msk, 1, sizeof(fieldsValueArr[0].msk));

        fieldsValueArr[3].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
        cpssOsMemSet(fieldsValueArr[3].data, 0x12, sizeof(fieldsValueArr[0].data));
        cpssOsMemSet(fieldsValueArr[3].msk, 0xFAFA, sizeof(fieldsValueArr[0].msk));

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId, groupId, ruleId, &ruleAttr, ruleNumOfFields, fieldsValueArr, actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, keyId, groupId, ruleId);

        /*
            1.1. Call function cpssDxChPacketAnalyzerGroupRuleGet with
                 valid values
            Expected: GT_OK and same values as written
        */
        ruleNumOfFieldsGet=6;
        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,&keyIdGet,&ruleAttrGet,&ruleNumOfFieldsGet,fieldsValueArrGet,&actionIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(keyId, keyIdGet,
                                     "got other keyIdGet than was set: %d", keyIdGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actionId, actionIdGet,
                                     "got other actionIdGet than was set: %d", actionIdGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(ruleNumOfFields, ruleNumOfFieldsGet,
                                     "got other ruleNumOfFieldsGet than was set: %d", ruleNumOfFieldsGet);
        for (ii=0 ; ii<ruleNumOfFieldsGet ; ii++)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(fieldsValueArr[ii].fieldName, fieldsValueArrGet[ii].fieldName,
                                         "got other fieldsValueArrGet than was set: %d index %d ",
                                         fieldsValueArrGet[ii].fieldName , ii);
            for (jj=0 ; jj<CPSS_DXCH_PACKET_ANALYZER_MAX_FIELD_LENGTH_CNS ; jj++)
            {
                /* verifying values */
                UTF_VERIFY_EQUAL3_STRING_MAC(fieldsValueArr[ii].msk[jj], fieldsValueArrGet[ii].msk[jj],
                                             "got other fieldsValueArrGet[ii].msk[jj] than was set: %d index ii %d index jj %d ",
                                             fieldsValueArrGet[ii].msk[jj] , ii, jj);
                /* verifying values */
                UTF_VERIFY_EQUAL3_STRING_MAC(fieldsValueArr[ii].data[jj], fieldsValueArrGet[ii].data[jj],
                                             "got other fieldsValueArrGet[ii].data[jj] than was set: %d index ii %d index jj %d ",
                                             fieldsValueArrGet[ii].data[jj] , ii, jj);
            }
        }

        /*
            1.2. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = BAD_RULE_ID_VALUE;
        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,&keyIdGet,&ruleAttrGet,&ruleNumOfFieldsGet, fieldsValueArrGet, &actionIdGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId, keyId);

        /*
            1.3. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = 1;
        groupId = BAD_GROUP_ID_VALUE;
        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,&keyIdGet,&ruleAttrGet,&ruleNumOfFieldsGet,fieldsValueArrGet,&actionIdGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId,keyId);

        /*
            1.4. Call function with wrong value ruleId [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 2;
        groupId = 1;
        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,&keyIdGet,&ruleAttrGet,&ruleNumOfFieldsGet,fieldsValueArrGet,&actionIdGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId,keyId);

        /*
            1.5. Call function with wrong value groupId [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 1;
        groupId = 2;
        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,&keyIdGet,&ruleAttrGet,&ruleNumOfFieldsGet,fieldsValueArrGet,&actionIdGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId,keyId);


        /*
            1.6. Call function with ruleNumOfFieldsGet less than init number [1]
            Expected: GT_BAD_SIZE.
        */
        groupId = 1;
        ruleNumOfFieldsGet = 1;
        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,&keyIdGet,&ruleAttrGet,&ruleNumOfFieldsGet,fieldsValueArrGet,&actionIdGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_SIZE, st, ruleId, groupId,keyId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(4, ruleNumOfFieldsGet,
                                     "got other ruleNumOfFieldsGet than expected : %d", ruleNumOfFieldsGet);

        /*
            1.7. Call with NULL keyIdPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        ruleNumOfFieldsGet = 4;
        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,NULL,&ruleAttrGet,&ruleNumOfFieldsGet,fieldsValueArrGet,&actionIdGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId,keyId);
        /*
            1.8. Call with NULL numOfFieldsPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        ruleNumOfFieldsGet = 4;
        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,&keyIdGet,&ruleAttrGet,NULL,fieldsValueArrGet,&actionIdGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId,keyId);

        /*
            1.9. Call with NULL fieldsValueArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        ruleNumOfFieldsGet = 4;
        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,&keyIdGet,&ruleAttrGet,&ruleNumOfFieldsGet,NULL,&actionIdGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId,keyId);

        /*
            1.10. Call with NULL actionIdPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        ruleNumOfFieldsGet = 4;
        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,&keyIdGet,&ruleAttrGet,&ruleNumOfFieldsGet,fieldsValueArrGet,NULL);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId,keyId);

        /* Restore system. */

        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.11. Call function when manager is not initialized
            Expected: GT_NOT_FOUND.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId,groupId,&keyIdGet,&ruleAttrGet,&ruleNumOfFieldsGet,fieldsValueArrGet,&actionIdGet);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId,keyId);

    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}



/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerRuleMatchStagesGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U32                                            ruleId,
    IN    GT_U32                                            groupId,
    INOUT GT_U32                                            *numOfMatchedStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       matchedStagesArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerRuleMatchStagesGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    this UT doesnt check values
    1.1. Call function cpssDxChPacketAnalyzerRuleMatchStagesGet with
         valid values
    Expected: GT_OK and 0 hits (doesnt work without traffic)
    1.2. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong value ruleId [2]
    Expected: GT_NOT_FOUND.
    1.5. Call function with wrong value groupId [2]
    Expected: GT_NOT_FOUND.
    1.6. Call with NULL numOfMatchedSubStagesPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.7. Call with NULL matchedstagesArr and other valid params.
    Expected: GT_BAD_PTR.
    1.8. Call Call function when manager is not initialized
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_U32                                               keyId;

    GT_U32                                               groupId;
    GT_U32                                               actionId;
    GT_U32                                               ruleId;

    GT_U32                                               numOfMatchedStagesExp;
    GT_U32                                               numOfMatchedStagesGet;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          matchedStagesArrExp[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          matchedStagesArrGet[SIZE_OF_TEST_ARR];

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        /* initialized managerId with device*/
        cpssOsMemSet(matchedStagesArrExp, 0, sizeof(matchedStagesArrExp));
        cpssOsMemSet(matchedStagesArrGet, 0, sizeof(matchedStagesArrGet));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId = 1;
        st = genericKeyInitSimple(managerId, keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericKeyInit ");
        /* initialized group ,action and rule*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericGroupInit ");
        actionId = 1;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericActionInit ");
        ruleId = 1 ;
        genericRuleInit(managerId,keyId,groupId,actionId,ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericRuleInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /* AUTODOC: clear sampling data and counters for rule in group */
        st = cpssDxChPacketAnalyzerSampledDataCountersClear(managerId, ruleId, groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /* AUTODOC: enable group activation */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId, groupId, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /*
            1.1. Call function cpssDxChPacketAnalyzerRuleMatchStagesGet with
                 valid values
            Expected: GT_OK and same values as written at init stage
        */
        numOfMatchedStagesExp = 0;
        numOfMatchedStagesGet = numOfMatchedStagesExp; /*warning*/
        numOfMatchedStagesGet = 20;

        st = cpssDxChPacketAnalyzerRuleMatchStagesGet(managerId, ruleId, groupId, &numOfMatchedStagesGet, matchedStagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, ruleId, groupId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfMatchedStagesExp, numOfMatchedStagesGet,
                                     "got other numOfMatchedStagesGet than expected: %d", numOfMatchedStagesGet);

        /*
            1.2. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = BAD_RULE_ID_VALUE;
        st = cpssDxChPacketAnalyzerRuleMatchStagesGet(managerId,ruleId, groupId, &numOfMatchedStagesGet, matchedStagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.3. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = 1;
        groupId = BAD_GROUP_ID_VALUE;
        st = cpssDxChPacketAnalyzerRuleMatchStagesGet(managerId,ruleId,groupId,&numOfMatchedStagesGet,matchedStagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.4. Call function with wrong value ruleId [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 2;
        groupId = 1;
        st = cpssDxChPacketAnalyzerRuleMatchStagesGet(managerId,ruleId,groupId,&numOfMatchedStagesGet,matchedStagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId);

        /*
            1.5. Call function with wrong value groupId [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 1;
        groupId = 2;
        st = cpssDxChPacketAnalyzerRuleMatchStagesGet(managerId,ruleId,groupId,&numOfMatchedStagesGet,matchedStagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId);

        /*
            1.6. Call with NULL numOfMatchedSubStagesPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        groupId = 1;
        numOfMatchedStagesGet = 3;
        st = cpssDxChPacketAnalyzerRuleMatchStagesGet(managerId,ruleId,groupId,NULL,matchedStagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /*
            1.7. Call with NULL matchedstagesArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerRuleMatchStagesGet(managerId,ruleId,groupId,&numOfMatchedStagesGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /* Restore system. */

        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.8. Call function when manager is not initialized
            Expected: GT_NOT_FOUND.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerRuleMatchStagesGet(managerId,ruleId,groupId,&numOfMatchedStagesGet,matchedStagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerStageMatchDataGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    OUT   GT_U32                                        *numOfHitsPtr,
    INOUT GT_U32                                        *numOfSampleFieldsPtr,
    INOUT CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     sampleFieldsValueArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerStageMatchDataGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    this UT doesnt check values
    1.1. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.2. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong value subStageId [BAD_SUBSTAGE_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.4. Call function with stageId not exist [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E]
    Expected: GT_BAD_PARAM.
    1.5. Call function with ruleId not exist [2]
    Expected: GT_NOT_FOUND.
    1.6. Call function with groupId not exist [2]
    Expected: GT_NOT_FOUND.
    1.7. Call with NULL numOfHitsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.8. Call with NULL numOfSampleFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.9. Call with NULL sampleFieldsValueArr and other valid params.
    Expected: GT_BAD_PTR.
    1.9. Call Call function when manager is not initialized
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_U32                                               keyId;

    GT_U32                                               groupId;
    GT_U32                                               actionId;
    GT_U32                                               ruleId;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId;
    GT_U32                                              numOfHits;
    GT_U32                                              numOfSampleFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC           sampleFieldsValueArr[4];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR               paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        /* initialized managerId with device*/
        cpssOsMemSet(&sampleFieldsValueArr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*4);
        numOfSampleFields = 4 ;

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /* initialized Key*/
        keyId = 1;
        st = genericKeyInitSimple(managerId, keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericKeyInit ");
        /* initialized group ,action and rule*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericGroupInit ");
        actionId = 1;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericActionInit ");
        ruleId = 1 ;
        genericRuleInit(managerId,keyId,groupId,actionId,ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericRuleInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;

        /*
            1.1. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = BAD_RULE_ID_VALUE;
        st = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.2. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */

        ruleId = 1;
        groupId = BAD_GROUP_ID_VALUE;
        st = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId, groupId, stageId, &numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.4. Call function with stageId not exist [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E]
            Expected: GT_BAD_PARAM.
        */
        groupId = 1;
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
        st = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.5. Call function with ruleId not exist [2]
            Expected: GT_NOT_FOUND.
        */
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;
        ruleId = 2;
        st = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId);

        /*
            1.6. Call function with groupId not exist [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 1;
        groupId = 2;
        st = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId);

        /*
            1.7. Call with NULL numOfHits and other valid params.
            Expected: GT_BAD_PTR.
        */
        groupId = 1;
        st = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId,groupId,stageId,NULL,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /*
            1.8. Call with NULL numOfSampleFields and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId,groupId,stageId,&numOfHits,NULL,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /*
            1.9. Call with NULL sampleFieldsValueArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /* Restore system. */

        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.9. Call Call function when manager is not initialized
            Expected: GT_NOT_FOUND.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId);

    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U32                                        ruleId,
    IN    GT_U32                                        groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    OUT   GT_U32                                        *numOfHitsPtr,
    INOUT GT_U32                                        *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC     sampleFieldsValueArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    this UT doesnt check values
    1.1. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.2. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.4. Call function with ruleId not exist [2]
    Expected: GT_NOT_FOUND.
    1.5. Call function with groupId not exist [2]
    Expected: GT_NOT_FOUND.
    1.6. Call with NULL numOfHitsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.7. Call with NULL numOfSampleFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.8. Call with NULL sampleFieldsValueArr and other valid params.
    Expected: GT_BAD_PTR.
    1.9. Call with numOfSampleFields less than max fields number per stage.
    Expected: GT_BAD_SIZE.
    1.10. Call function with stageId not exist [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E]
    Expected: GT_BAD_PARAM.
    1.11 Call function with stageId not exist only for aldrin
    Expected: GT_BAD_STATE for ALDRIN GT_OK for others .
    1.12. Call Call function when manager is not initialized
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_U32                                               keyId;

    GT_U32                                               groupId;
    GT_U32                                               actionId;
    GT_U32                                               ruleId;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId;
    GT_U32                                              numOfHits;
    GT_U32                                              numOfSampleFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC           *sampleFieldsValueArr=NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR               paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /* initialized Key*/
        keyId = 1;
        st = genericKeyInitSimple(managerId, keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericKeyInit ");
        /* initialized group ,action and rule*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericGroupInit ");
        actionId = 1;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericActionInit ");
        ruleId = 1 ;
        genericRuleInit(managerId,keyId,groupId,actionId,ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericRuleInit ");

        /* initialized managerId with device*/
        sampleFieldsValueArr = cpssOsMalloc(CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E*sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));
        if (sampleFieldsValueArr == NULL)
        {
            st = GT_OUT_OF_CPU_MEM;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at sampleFieldsValueArr cpssOsMalloc ");
        }
        cpssOsMemSet(sampleFieldsValueArr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;

        /*
            1. Call function with vaid params
            Expected: GT_OK.
        */
        numOfSampleFields=CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
        st = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, ruleId, groupId);

        /*
            1.1. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = BAD_RULE_ID_VALUE;
        numOfSampleFields=10;
        st = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.2. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = 1;
        groupId = BAD_GROUP_ID_VALUE;
        st = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.4. Call function with ruleId not exist [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 2;
        st = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.5. Call function with groupId not exist [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 1;
        groupId = 2;
        st = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId);

        /*
            1.6. Call with NULL numOfHits and other valid params.
            Expected: GT_BAD_PTR.
        */
        groupId=1;
        numOfSampleFields=10;
        st = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,groupId,stageId,NULL,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /*
            1.7. Call with NULL numOfSampleFields and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,groupId,stageId,&numOfHits,NULL,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /*
            1.8. Call with NULL sampleFieldsValueArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /*
            1.9. Call with numOfSampleFields less than max fields number per stage.
            Expected: GT_BAD_SIZE.
        */
        groupId = 1;
        ruleId = 1;
        stageId = PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E;
        numOfSampleFields = 0;
        st = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, ruleId, groupId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(5, numOfSampleFields,
                       "got other numOfSampleFields then Expected %d ", numOfSampleFields );
        /*
            1.10. Call function with stageId not exist [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E]
            Expected: GT_BAD_PARAM.
        */
        numOfSampleFields = 10;
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
        st = cpssDxChPacketAnalyzerStageMatchDataGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;
        /* Restore system. */
        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.12. Call Call function when manager is not initialized
            Expected: GT_NOT_FOUND.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet(managerId,ruleId,groupId,stageId,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId);

        if (sampleFieldsValueArr != NULL)
        {
            cpssOsFree(sampleFieldsValueArr);
        }
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerStageFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerStageFieldsGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E/
                                     CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E/
                                     CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E/
                                     CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E]
                             subStage [1]
                             numOfFields[SIZE_OF_TEST_ARR*10]
    Expected: GT_OK.
    1.2. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_FIRST_E]
                            subStage [1]
                            numOfFields[SIZE_OF_TEST_ARR*10]
    Expected: GT_BAD_PARAM.
    1.3. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E]
                            subStage [1]
                            numOfFields[0]
    Expected: GT_BAD_SIZE.
    1.4. Call with NULL numOfFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL fieldsArr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stage ;
    GT_U32                                               numOfFieldsGet;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsArrGet[SIZE_OF_TEST_ARR*10];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        cpssOsMemSet(fieldsArrGet, 0, sizeof(fieldsArrGet));
        /*
            1.1.1 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E]
                                     subStage [1]
                                     numOfFields[SIZE_OF_TEST_ARR*10]
            Expected: GT_OK.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(5, numOfFieldsGet,
                       "numOfFieldsGet got value : %d Expected 5 ", numOfFieldsGet );

        /*
            1.1.2 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E]
                                     subStage [1]
                                     numOfFields[SIZE_OF_TEST_ARR*10]
            Expected: GT_OK.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, numOfFieldsGet,"numOfFieldsGet got 0 value ");

        /*
            1.1.3 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E]
                                     subStage [1]
                                     numOfFields[SIZE_OF_TEST_ARR*10]
            Expected: GT_OK.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, numOfFieldsGet,"numOfFieldsGet got 0 value ");
        /*
            1.1.4 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E]
                                     subStage [1]
                                     numOfFields[SIZE_OF_TEST_ARR*10]
            Expected: GT_OK.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, numOfFieldsGet, "numOfFieldsGet got 0 value ");
        /*
            1.2. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_FIRST_E]
                                    subStage [1]
                                    numOfFields[SIZE_OF_TEST_ARR*10]
            Expected: GT_BAD_PARAM.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum,stage);

        /*
            1.3. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E]
                                    numOfFields[0]
            Expected: GT_BAD_SIZE.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E ;
        numOfFieldsGet = 0;
        st = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum,stage);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, numOfFieldsGet, "numOfFieldsGet got 0 value ");

        /*
            1.4. Call with NULL numOfFieldsPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stage,NULL,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,stage);
        /*                                                         .
            1.5. Call with NULL fieldsArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stage,&numOfFieldsGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,stage);
        /*                                                    .
            1.6. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerStageFieldsGet(managerId,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum,stage);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerFieldStagesGet
(
    IN    GT_U32                                            managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT               fieldName,
    INOUT GT_U32                                            *numOfStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stagesArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerFieldStagesGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E\
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E\
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E\
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E\
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E\
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E\
                                       CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E]
                    numOfStagesPtr [20]
    Expected: GT_OK.
    1.2. Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E]
                            numOfStagesPtr [20]
    Expected: GT_BAD_PARAM.
    1.3. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E]
                            numOfStagesPtr [2]
    Expected: GT_BAD_SIZE.
    1.4. Call with NULL numOfStagesPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL stagesArr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;

    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  field ;
    GT_U32                                               numOfStagesGet;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stagesArrGet[SIZE_OF_TEST_ARR];

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        cpssOsMemSet(stagesArrGet, 0, sizeof(stagesArrGet));
        /*
            1.1.1 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E]
                            numOfStagesPtr [20]
            Expected: GT_OK.
        */
        field = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E ;
        numOfStagesGet = 20;
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,field);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, numOfStagesGet, "numOfStagesGet got 0 value ");

        /*
            1.1.2 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E]
                            numOfStagesPtr [20]
            Expected: GT_OK.
        */
        field = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E ;
        numOfStagesGet = 20;
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,field);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, numOfStagesGet, "numOfStagesGet got 0 value ");

        /*
            1.1.3 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E]
                            numOfStagesPtr [20]
            Expected: GT_OK.
        */
        field = CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IP_E ;
        numOfStagesGet = 20;
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,field);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, numOfStagesGet, "numOfStagesGet got 0 value ");

        /*
            1.1.4 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E]
                            numOfStagesPtr [23]
            Expected: GT_OK.
        */
        field = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E ;
        numOfStagesGet = 20;
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,field);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, numOfStagesGet, "numOfStagesGet got 0 value ");

        /*
            1.1.5 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E]
                            numOfStagesPtr [20]
            Expected: GT_OK.
        */
        field = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E ;
        numOfStagesGet = 20;
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,field);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0, numOfStagesGet,
                       "stagesArrGet got 0 value : %d", numOfStagesGet);

        /*
            1.1.6 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E]
                            numOfStagesPtr [20]
            Expected: GT_OK.
        */
        field = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E ;
        numOfStagesGet = 20;
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,field);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL1_STRING_MAC(0, numOfStagesGet,
                       "stagesArrGet got 0 value : %d", numOfStagesGet);

        /*
            1.1.7 Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E]
                            numOfStagesPtr [20]
            Expected: GT_OK.
        */
        field = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E ;
        numOfStagesGet = 20;
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,field);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0, numOfStagesGet, "stagesArrGet got 0 value");

        /*
            1.2. Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E]
                                numOfStagesPtr [20]
            Expected: GT_BAD_PARAM.
        */
        numOfStagesGet = 20;
        field = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E ;
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum,field);

        /*
            1.3. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E]
                                numOfStagesPtr [2]
            Expected: GT_BAD_SIZE.
        */
        field = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E ;
        numOfStagesGet = 2;
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum,field);

        /* verifying values */
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(2, numOfStagesGet, "stagesArrGet got value :2 ");

        /*
            1.4. Call with NULL numOfStagesPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        field = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E ;
        numOfStagesGet = 20;
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,NULL,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,field);

        /*
            1.5. Call with NULL stagesArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,field);

        /*
            1.6. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerFieldStagesGet(managerId,field,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum,field);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerDbDeviceGet
(
    IN  GT_U32                                      managerId,
    IN  GT_U8                                       devNum,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR       *paDevDbPtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerDbDeviceGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function prvCpssDxChPacketAnalyzerDbDeviceGet after
                       genericManagerAndDeviceInit
                       genericKeyInitSimple
    Expected: GT_OK and ptr to dev with same value
    1.2. Call function with wrong value devNum [2]
    Expected: GT_BAD_PARAM.
    1.3. Call function when manager is not initialized.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum,savedDevNum;
    GT_U32                                              managerId;
    GT_U32                                              keyId;

    PRV_CPSS_DXCH_PACKET_ANALYZER_DEV_PTR               paDevDb=NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR               paMngDb=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        savedDevNum=devNum;
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=1;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");

        /* initialized Key*/
        keyId=4;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /*
            1.1. Call function prvCpssDxChPacketAnalyzerDbDeviceGet after
                               genericManagerAndDeviceInit
                               genericKeyInitSimple
            Expected: GT_OK and ptr to dev with same value
        */
        st =prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPacketAnalyzerDbDeviceGet: %d", devNum);

        st =prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPacketAnalyzerDbManagerGet: %d", devNum);

        if (paDevDb && paMngDb)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC( 2 , paMngDb->numOfKeys,
                                "got other paDevDb->numOfKeys than expected: %d", paMngDb->numOfKeys);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC( savedDevNum , paDevDb->paDevId,
                                         "got other paDevDb->paDevId than expected: %d", paDevDb->paDevId);
            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC( 4 , paMngDb->paKeyPtrArr[4]->paKeyId,
                                         "got other paDevDb->paKeyPtrArr[4] than expected: %d", paMngDb->paKeyPtrArr[4]->paKeyId);
        }
        /*
            1.2. Call function with wrong value devNum [2]
            Expected: GT_BAD_PARAM.
        */
        st =prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,2,&paDevDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,"prvCpssDxChPacketAnalyzerDbDeviceGet: %d", devNum);

        /*
            1.3. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st =prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"prvCpssDxChPacketAnalyzerDbDeviceGet: %d", devNum);
    }
    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        managerId = GENERIC_MANAGER_ID ;
        st =prvCpssDxChPacketAnalyzerDbDeviceGet(managerId,devNum,&paDevDb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerDbStageGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U8                                             devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stageId,
    OUT   PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC          **stagePtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerDbStageGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function prvCpssDxChPacketAnalyzerDbStageGet after
                       genericManagerAndDeviceInit
                       genericKeyInitSimple
                       with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E\
                                     CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E ]
    Expected: GT_OK and ptr to ptr of sub stage
    1.2. Call function with wrong value devNum [2]
    Expected: GT_BAD_PARAM.
    1.3. Call function when manager is not initialized.
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;
    GT_U32                                              keyId;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGES_STC            *stagePtr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR               paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /* initialized Key*/
        keyId=5;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /*
            1.1.1 Call function prvCpssDxChPacketAnalyzerDbStageGet after
                               genericManagerAndDeviceInit
                               genericKeyInitSimple
                               with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E]
            Expected: GT_OK and ptr to ptr of sub stage
        */
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        st =prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stageId,&stagePtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPacketAnalyzerDbStageGet: %d", devNum);

        /*
            1.1.2 Call function prvCpssDxChPacketAnalyzerDbStageGet after
                               genericManagerAndDeviceInit
                               genericKeyInitSimple
                               with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E ]
            Expected: GT_OK and ptr to ptr of sub stage
        */
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E ;
        st =prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stageId,&stagePtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPacketAnalyzerDbStageGet: %d", devNum);

        /*
            1.2. Call function with wrong value devNum [2]
            Expected: GT_BAD_PARAM.
        */
        st =prvCpssDxChPacketAnalyzerDbStageGet(managerId,2,stageId,&stagePtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,"prvCpssDxChPacketAnalyzerDbStageGet: %d", devNum);

        /*
            1.3. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st =prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum,stageId,&stagePtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"prvCpssDxChPacketAnalyzerDbStageGet: %d", devNum);
    }
    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        managerId = GENERIC_MANAGER_ID ;
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E;
        st = prvCpssDxChPacketAnalyzerDbStageGet(managerId,devNum, stageId, &stagePtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerDbKeyGet
(
    IN  GT_U32                                          managerId,
    IN  GT_U8                                           devNum,
    IN  GT_U32                                          keyId,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR           *paKeyDbPtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerDbKeyGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function prvCpssDxChPacketAnalyzerDbKeyGet after
                       genericManagerAndDeviceInit
                       genericKeyInitSimple
                       keyId[6/8]
    Expected: GT_OK and ptr to dev with same value
    1.2. Call function with wrong value devNum [2]
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong value keyId  [2]
    Expected: GT_NOT_FOUND.
    1.4. Call function with wrong value keyId  [BAD_KEY_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.5. Call function when manager is not initialized.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;
    GT_U32                                              keyId;

    PRV_CPSS_DXCH_PACKET_ANALYZER_KEY_PTR               paKeyDb=NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR               paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /* initialized Key*/
        keyId=6;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");

        /* initialized Key*/
        keyId=8;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /*
            1.1.1 Call function prvCpssDxChPacketAnalyzerDbDeviceGet after
                               genericManagerAndDeviceInit
                               genericKeyInitSimple
                               keyId [6]
            Expected: GT_OK and ptr to dev with same value
        */
        keyId=6;
        st =prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPacketAnalyzerDbKeyGet: %d", devNum);

        if (paKeyDb)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC( 5 , paKeyDb->numOfFields,
                            "got other paKeyDb->numOfFields than expected: %d", paKeyDb->numOfFields);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC( 6 , paKeyDb->paKeyId,
                                     "got other paKeyDb->paKeyId than expected: %d", paKeyDb->paKeyId);
            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC( CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E , paKeyDb->stagesArr[0],
                                     "got other paKeyDb->stagesArr[0] than expected: %d", paKeyDb->stagesArr[0]);
        }

        /*
            1.1.2 Call function prvCpssDxChPacketAnalyzerDbDeviceGet after
                               genericManagerAndDeviceInit
                               genericKeyInitSimple
            Expected: GT_OK and ptr to dev with same value
        */
        keyId=8;
        st =prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPacketAnalyzerDbKeyGet: %d", devNum);

        if (paKeyDb)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(5, paKeyDb->numOfFields,
                                         "got other paKeyDb->numOfFields than expected: %d", paKeyDb->numOfFields);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(8, paKeyDb->paKeyId,
                                         "got other paKeyDb->paKeyId than expected: %d", paKeyDb->paKeyId);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E, paKeyDb->stagesArr[1],
                                     "got other paKeyDb->stagesArr[1] than expected: %d", paKeyDb->stagesArr[1]);
        }

        /*
            1.2. Call function with wrong value devNum [2]
            Expected: GT_BAD_PARAM.
        */
        st =prvCpssDxChPacketAnalyzerDbKeyGet(managerId,2,keyId,&paKeyDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,"prvCpssDxChPacketAnalyzerDbKeyGet: %d", devNum);

        /*
            1.3. Call function with wrong value keyId  [2]
            Expected: GT_NOT_FOUND.
        */
        keyId = 2 ;
        st =prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"prvCpssDxChPacketAnalyzerDbKeyGet: %d", devNum);

        /*
            1.4. Call function with wrong value keyId  [BAD_KEY_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        keyId = BAD_KEY_ID_VALUE ;
        st =prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,"prvCpssDxChPacketAnalyzerDbKeyGet: %d", devNum);

        /*
            1.5. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        keyId = 1;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st =prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"prvCpssDxChPacketAnalyzerDbKeyGet: %d", devNum);
    }
    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        managerId = GENERIC_MANAGER_ID ;
        keyId = 1 ;
        st =prvCpssDxChPacketAnalyzerDbKeyGet(managerId,devNum,keyId,&paKeyDb);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerDbRuleGet
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          ruleId,
    IN  GT_U32                                          groupId,
    OUT PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR          *paRuleDbPtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerDbRuleGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function prvCpssDxChPacketAnalyzerDbRuleGet after
                       genericManagerAndDeviceInit
                       genericKeyInitSimple
                       genericGroupInit
                       genericActionInit
                       genericRuleInit
    Expected: GT_OK and ptr to rule with same value
    1.2. Call function with wrong value ruleId [2]
    Expected: GT_NOT_FOUND.
    1.3. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.4. Call function with wrong value groupId  [2]
    Expected: GT_NOT_FOUND.
    1.5. Call function with wrong value groupId  [BAD_GROUP_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.6. Call function ruleId when manager is not initialized .
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;
    GT_U32                                              keyId;
    GT_U32                                              groupId;
    GT_U32                                              actionId;
    GT_U32                                              ruleId;

    PRV_CPSS_DXCH_PACKET_ANALYZER_RULE_PTR               paRuleDb=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=4;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /* initialized group and action*/
        groupId = 4;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");
        actionId = 4;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericActionInit ");
        /* initialized rule*/
        ruleId = 4;
        st = genericRuleInit(managerId, keyId, groupId, actionId, ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericRuleInit ");
        /*
            1.1. Call function prvCpssDxChPacketAnalyzerDbRuleGet after
                               genericManagerAndDeviceInit
                               genericKeyInitSimple
                               genericGroupInit
                               genericActionInit
                               genericRuleInit
            Expected: GT_OK and ptr to rule with same value
        */
        st =prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPacketAnalyzerDbRuleGet: %d", ruleId);

        if (paRuleDb)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(4, paRuleDb->paKeyId,
                                         "got other paRuleDb->paKeyId than expected: %d", paRuleDb->paKeyId);

            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(4,  paRuleDb->paRuleId,
                                         "got other paRuleDb->paRuleId than expected: %d", paRuleDb->paRuleId);
            /* verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,  paRuleDb->fieldsValueArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E].fieldName,
                                         "got other paRuleDb->fieldsValueArr[0].fieldName "
                                         "than expected: %d", paRuleDb->fieldsValueArr[0].fieldName);

            /* verifying values (default value expected*/
            UTF_VERIFY_EQUAL1_STRING_MAC(CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_IS_TRUNK_E,  paRuleDb->fieldsValueArr[CPSS_DXCH_PACKET_ANALYZER_FIELD_TRG_PHY_PORT_E].fieldName,
                                         "got other paRuleDb->fieldsValueArr[0].fieldName "
                                         "than expected: %d", paRuleDb->fieldsValueArr[0].fieldName);
        }

        /*
            1.2. Call function with wrong value ruleId [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId=2;
        st =prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"prvCpssDxChPacketAnalyzerDbRuleGet: %d", ruleId);

        /*
            1.3. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = BAD_RULE_ID_VALUE ;
        st =prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,"prvCpssDxChPacketAnalyzerDbRuleGet: %d", ruleId);


        /*
            1.4. Call function with wrong value groupId  [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 4;
        groupId = 2;
        st =prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"prvCpssDxChPacketAnalyzerDbRuleGet: %d", ruleId);

        /*
            1.5. Call function with wrong value groupId  [BAD_GROUP_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        groupId = BAD_GROUP_ID_VALUE;
        st =prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,"prvCpssDxChPacketAnalyzerDbRuleGet: %d", ruleId);

        /* Restore system. */
        groupId = 4 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 4 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.6. Call function when manager is not initialized .
            Expected: GT_NOT_FOUND.
        */
        groupId = 4;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st =prvCpssDxChPacketAnalyzerDbRuleGet(managerId,ruleId,groupId,&paRuleDb);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"prvCpssDxChPacketAnalyzerDbRuleGet: %d", ruleId);

    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerDbDevGet
(
    IN  GT_U32                               managerId,
    IN  GT_U32                               keyId,
    OUT  GT_U8                               *devNumPtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerDbDevGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function prvCpssDxChPacketAnalyzerDbDevGet after
                       genericManagerAndDeviceInit
                       genericKeyInitSimple
                       keyId[6]
    Expected: GT_OK and ptr to dev number with same value
    1.2. Call function with wrong value keyId  [2]
    Expected: GT_NOT_FOUND.
    1.3. Call function with wrong value keyId  [BAD_KEY_ID_VALUE]
    Expected: GT_NOT_FOUND.
    1.4. Call function when manager is not initialized.
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;
    GT_U32                                              keyId;

    GT_U8                                               devNumGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        /* initialized Key*/
        keyId=6;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");
        /*
            1.1 Call function prvCpssDxChPacketAnalyzerDbDeviceGet after
                               genericManagerAndDeviceInit
                               genericKeyInitSimple
                               keyId [6]
            Expected: GT_OK and ptr to dev with same value
        */
        st =prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNumGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPacketAnalyzerDbDevGet: %d", keyId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( devNum , devNumGet,
                            "got other devNumGet than expected: %d", devNumGet);

        /*
            1.2. Call function with wrong value keyId  [2]
            Expected: GT_NOT_FOUND.
        */
        keyId = 2 ;
        st =prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNumGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"prvCpssDxChPacketAnalyzerDbDevGet: %d", keyId);

        /*
            1.3. Call function with wrong value keyId  [BAD_KEY_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        keyId = BAD_KEY_ID_VALUE ;
        st =prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNumGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,"prvCpssDxChPacketAnalyzerDbDevGet: %d", keyId);

        /*
            1.4. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        keyId = 6;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerDbDevGet(managerId,keyId,&devNumGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st,"prvCpssDxChPacketAnalyzerDbDevGet: %d", keyId);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerDbManagerValidCheck
(
    IN  GT_U32                               managerId
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerDbManagerValidCheck)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function prvCpssDxChPacketAnalyzerDbManagerValidCheck after
                       genericManagerAndDeviceInit
                       genericKeyInitSimple
    Expected: GT_OK
    1.2. Call function with wrong value managerId [2]
    Expected: GT_NOT_INITIALIZED.
    1.3. call function when there is no manager initialized
    Expected: GT_NOT_INITIALIZED.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E) ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");

        /*
            1.1. Call function prvCpssDxChPacketAnalyzerDbManagerValidCheck after
                               genericManagerAndDeviceInit
                               genericKeyInitSimple
            Expected: GT_OK
        */
        st =prvCpssDxChPacketAnalyzerDbManagerValidCheck(managerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"prvCpssDxChPacketAnalyzerDbManagerValidCheck: %d", managerId);

        /*
            1.2. Call function with wrong value managerId [2]
            Expected: GT_NOT_INITIALIZED.
        */
        managerId = 2 ;
        st =prvCpssDxChPacketAnalyzerDbManagerValidCheck(managerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,"prvCpssDxChPacketAnalyzerDbManagerValidCheck: %d", managerId);

        /*
            1.3. call function when there is no manager initialized
            Expected: GT_NOT_INITIALIZED.
        */
        managerId = GENERIC_MANAGER_ID ;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st =prvCpssDxChPacketAnalyzerDbManagerValidCheck(managerId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,"prvCpssDxChPacketAnalyzerDbDeviceGet: %d", devNum);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet
(
    IN  GT_U8                                       devNum,
    IN  GT_BOOL                                     enable
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with supported device and enable [GT_FALSE /GT_TRUE]
    Expected: GT_OK
    1.2. call function when there is no manager initialized
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_BOOL                                              enable;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        /*
             1.1.1 Call function with supported device and enable [GT_TRUE]
             Expected: GT_OK
        */
        enable = GT_TRUE ;
        st = prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet(devNum,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,enable);

        /*
             1.1.2 Call function with supported device and enable [GT_FALSE]
             Expected: GT_OK
        */
        enable = GT_FALSE ;
        st = prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet(devNum,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,enable);

        /*
            1.3. call function when there is no manager initialized
            Expected: GT_BAD_PARAM.
        */
        managerId = GENERIC_MANAGER_ID ;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet(devNum,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,enable);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        enable = GT_TRUE ;
        st = prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet(devNum,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum,enable);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldAdd
(
    IN GT_U32                                   managerId,
    IN GT_CHAR                                  *fieldName,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT     *udfIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerUserDefinedFieldAdd)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with fieldName [key,key_size,key_type]
    Expected: GT_OK.
    1.2. Call function with wrong fieldName [PRV_CPSS_DXCH_IDEBUG_FIELD_LAST_E] and all other valid value
    Expected: GT_BAD_PARAM.
    1.3. Call function with fieldName predefine
    Expected: GT_ALREADY_EXIST
    1.4. Call function with fieldName already set
    Expected: GT_ALREADY_EXIST
    1.5. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  udfIdArr[5];
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC         udfAttr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC             fieldName;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        /*
             1.1.1 Call function with fieldName [key]
             Expected: GT_OK.
        */
        cpssOsStrCpy(fieldName.fieldNameArr,"key");
        cpssOsStrCpy(udfAttr.udfNameArr,"key");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfIdArr[0]);

        /*
             1.1.2 Call function with fieldName [key_size]
             Expected: GT_OK.
        */
        cpssOsStrCpy(fieldName.fieldNameArr,"key_size");
        cpssOsStrCpy(udfAttr.udfNameArr,"key_size");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[1]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfIdArr[1]);

        /*
             1.1.3 Call function with fieldName [key_type]
             Expected: GT_OK.
        */
        cpssOsStrCpy(fieldName.fieldNameArr,"key_type");
        cpssOsStrCpy(udfAttr.udfNameArr,"key_type");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[2]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfIdArr[2]);

        /*
            1.2. Call function with wrong fieldName "" and all other valid value
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(fieldName.fieldNameArr,"");
        cpssOsStrCpy(udfAttr.udfNameArr,"");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[3]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, udfIdArr[3]);

        /*
            1.3. Call function with fieldName predefine
            Expected: GT_ALREADY_EXIST
        */
        cpssOsStrCpy(fieldName.fieldNameArr,"byte_count");
        cpssOsStrCpy(udfAttr.udfNameArr,"byte_count");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[4]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_ALREADY_EXIST, st, devNum, udfIdArr[4]);

        /*
            1.4. Call function with fieldName already set
            Expected: GT_ALREADY_EXIST
        */
        cpssOsStrCpy(fieldName.fieldNameArr,"key");
        cpssOsStrCpy(udfAttr.udfNameArr,"key");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[4]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_ALREADY_EXIST, st, devNum, udfIdArr[4]);

        /*
            1.5. Call function when manager is not initialized . .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[4]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, udfIdArr[4]);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldDelete
(
    IN GT_U32                               managerId,
    IN GT_CHAR                              *fieldName
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerUserDefinedFieldDelete)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerUserDefinedFieldAdd
                            fieldName [key,key_size,key_type]
              call cpssDxChPacketAnalyzerUserDefinedFieldGet and check valid == GT_TRUE
              delete all and check valid == GT_FALSE
    Expected: GT_OK.
    1.2. Call function with wrong fieldName and all other valid value
    Expected: GT_BAD_PARAM.
    1.3. Call function with predefine field [ipv4_sip] and all other valid value
    Expected: GT_BAD_PARAM.
    1.3. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  udfIdArr[5];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  udfId;
    GT_BOOL                                              valid;
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC         udfAttr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC             fieldName;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        /*
            1.1. Call function cpssDxChPacketAnalyzerUserDefinedFieldAdd
                                fieldName [key,key_size,key_type]
                  call cpssDxChPacketAnalyzerUserDefinedFieldGet and check valid == GT_TRUE
                  delete all and check valid == GT_FALSE
            Expected: GT_OK.
        */
        cpssOsStrCpy(fieldName.fieldNameArr,"key");
        cpssOsStrCpy(udfAttr.udfNameArr,"key");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfIdArr[0]);

        cpssOsStrCpy(fieldName.fieldNameArr,"key_size");
        cpssOsStrCpy(udfAttr.udfNameArr,"key_size");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[1]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfIdArr[1]);

        cpssOsStrCpy(fieldName.fieldNameArr,"key_type");
        cpssOsStrCpy(udfAttr.udfNameArr,"key_type");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[2]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfIdArr[2]);

        /*check that all udfs are valid */
        cpssOsStrCpy(udfAttr.udfNameArr,"key");
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfId,&valid,&fieldName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfId);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_TRUE , valid,
                            "got other valid than expected: %d", valid);
        cpssOsStrCpy(udfAttr.udfNameArr,"key_size");
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfId,&valid,&fieldName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfId);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_TRUE , valid,
                            "got other valid than expected: %d", valid);
        cpssOsStrCpy(udfAttr.udfNameArr,"key_type");
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfId,&valid,&fieldName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfId);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_TRUE , valid,
                            "got other valid than expected: %d", valid);

        /*delete all udfs*/
        cpssOsStrCpy(udfAttr.udfNameArr,"key");
        st = cpssDxChPacketAnalyzerUserDefinedFieldDelete(managerId,&udfAttr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfAttr.udfNameArr);
        cpssOsStrCpy(udfAttr.udfNameArr,"key_size");
        st = cpssDxChPacketAnalyzerUserDefinedFieldDelete(managerId,&udfAttr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfAttr.udfNameArr);
        cpssOsStrCpy(udfAttr.udfNameArr,"key_type");
        st = cpssDxChPacketAnalyzerUserDefinedFieldDelete(managerId,&udfAttr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfAttr.udfNameArr);

        /*check that all udfs are not  valid */
        cpssOsStrCpy(udfAttr.udfNameArr,"key");
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfId,&valid,&fieldName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfAttr.udfNameArr);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_FALSE , valid,
                            "got other valid than expected: %d", valid);
        cpssOsStrCpy(udfAttr.udfNameArr,"key_size");
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfId,&valid,&fieldName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfAttr.udfNameArr);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_FALSE , valid,
                            "got other valid than expected: %d", valid);
        cpssOsStrCpy(udfAttr.udfNameArr,"key_type");
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfId,&valid,&fieldName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfAttr.udfNameArr);
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_FALSE , valid,
                            "got other valid than expected: %d", valid);
        /*
            1.2. Call function with wrong fieldName and all other valid value
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(udfAttr.udfNameArr,"");
        st = cpssDxChPacketAnalyzerUserDefinedFieldDelete(managerId,&udfAttr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, udfAttr.udfNameArr);

        /*
            1.4. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerUserDefinedFieldDelete(managerId,&udfAttr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, udfAttr.udfNameArr);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerUserDefinedFieldGet
(
    IN  GT_U32                               managerId,
    IN  GT_CHAR                              *fieldName,
    OUT CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT  *udfIdPtr,
    OUT GT_BOOL                              *validPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerUserDefinedFieldGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with fieldName "key"
    Expected: GT_OK and same value as set,validPtr == GT_TRUE .
    1.2. Call function with fieldName "key_size" that not exist
    Expected: GT_OK  and valid ==GT_FALSE.
    1.3. Call function with wrong fieldName [""] and all other valid value
    Expected: GT_BAD_PARAM.
    1.4. Call function with NULL fieldNamePtr and all other valid value
    Expected: GT_BAD_PARAM.
    1.5. Call function with NULL validPtr and all other valid value
    Expected: GT_BAD_PTR.
    1.6. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  udfId;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  udfIdGet;
    GT_BOOL                                              valid;
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC         udfAttr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC             fieldName;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC             fieldNameGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        /*init udf*/
        cpssOsStrCpy(fieldName.fieldNameArr,"key");
        cpssOsStrCpy(udfAttr.udfNameArr,"key");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, fieldName.fieldNameArr);

        /*
            1.1. Call function with fieldName "key"
            Expected: GT_OK.
        */
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfIdGet,&valid,&fieldNameGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, fieldName.fieldNameArr);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_TRUE , valid,
                            "got other valid than expected: %d", valid);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( udfId , udfIdGet,
                            "got other udfIdGet than expected: %d", udfIdGet);

        /*
            1.2. Call function with fieldName "key_size" that not exist
            Expected: GT_OK  and valid ==GT_FALSE.
        */
        cpssOsStrCpy(udfAttr.udfNameArr,"key_size");
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfIdGet,&valid,&fieldNameGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, fieldName.fieldNameArr);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, valid,
                                     "got other valid than expected: %d", valid);

        /*
            1.3. Call function with wrong fieldName "" and all other valid value
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(udfAttr.udfNameArr,"");
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfIdGet,&valid,&fieldNameGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, fieldName.fieldNameArr);

        /*
            1.5. Call function with NULL fieldName and all other valid value
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,NULL,&udfIdGet,&valid,&fieldNameGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, fieldName.fieldNameArr);

        /*
            1.6. Call function with NULL validPtr and all other valid value
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(udfAttr.udfNameArr,"key");
        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfIdGet,NULL,&fieldNameGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, fieldName.fieldNameArr);

        /*
            1.7. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerUserDefinedFieldGet(managerId,&udfAttr,&udfIdGet,&valid,&fieldNameGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, fieldName.fieldNameArr);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageAdd
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC     *udsAttrPtr,
    IN CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC    *interfaceAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *udsIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerUserDefinedStageAdd)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with udsId [0-9]
                            and valid interfaceAttributes
    Expected: GT_OK.
    1.3.Call function with udsId [7]
    And not valid interfaceAttributes (desc that is in predefine stage)
    Expected: GT_BAD_PARAM.
    1.4. Call function with uds name already set
    Expected: GT_ALREADY_EXIST
    1.5. Call function with uds interface already set
    Expected: GT_ALREADY_EXIST
    1.6. Call function with uds interface name not exist in xml
    Expected: GT_BAD_PARAM
    1.7. Call function with uds instance name not exist in xml
    Expected: GT_BAD_PARAM
    1.7. Call function with NULL udsAttrPtr
    Expected: GT_BAD_PTR
    1.8. Call function with NULL interfaceAttrPtr
    Expected: GT_BAD_PTR
    1.9. Call function with NULL udsIdPtr
    Expected: GT_BAD_PTR
    1.10. Call function with empty udsIdPtr
    Expected: GT_BAD_PARAM
    1.11. Call function with empty interfaceAttrPtr
    Expected: GT_BAD_PARA
    1.12. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          udsId;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC        interfaceAttributes;
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC         udsAttr;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");

        cpssOsMemSet(&interfaceAttributes,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*
            1.1.1 Call function with udsId [0-9]
                                    and valid interfaceAttributes
            Expected: GT_OK.
        */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_epi_mac_macro_i0_mif2mti_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PFC_MSG");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_txqp_macro_qfc_app");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"crdt_bkwd_tx");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_epi_mac_macro_i0_qfc_app");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 3");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"SDB_PCH_TX");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_pca_brg_macro_i1_pca_brg_app");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 4");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PCL_2_TCAM_LOOKUP");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_pcl_macro_tcam_lookup");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_epi_100g_macro_mif2mti_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PFC_MSG");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_txq_macro_qfc0");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"crdt_bkwd_tx");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_pca_eip163_macro_eip163hi");
            interfaceAttributes.interfaceIndex = 4;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 3");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"SDB_PCH_TX");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_pca_eip163_macro_sff_lmu_egr");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 4");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PCL_2_TCAM_LOOKUP");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_pcl_macro_tcam_lookup");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_hawk_epi_400g_mac_macro_i2_mif2mti_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PFC_MSG");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_hawk_epi_400g_mac_macro_i2_qfc_app");
            interfaceAttributes.interfaceIndex = 2;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"crdt_bkwd_tx");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_hawk_epi_usx_mac_macro_i0_pca2mif_tx_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 3");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"SDB_PCH_TX");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_hawk_pca_macro_i0_sff_lmu_ctsu");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 4");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PCL_2_TCAM_LOOKUP");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pcl_macro_tcam_lookup");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"rxing2iaing_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe0_rx_macro_i0_ia_desc1");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"L2I_2_MT_LEARN");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe0_eagle_l2i_ipvx_0_macro_mt_or_shm");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"iaegr2rxegr_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe0_eagle_ia_0_macro_rx_desc3");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 3");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"eft2qag_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe0_eagle_egf_0_macro_eft");
            interfaceAttributes.interfaceIndex = 4;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 4");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PCL_2_TCAM_LOOKUP");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe1_eagle_pcl_1_macro_tcam_lookup");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"xbar2u_rd");
            cpssOsStrCpy(interfaceAttributes.instanceId,"mbus_x_u10_buses_1_21");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"xbar2u");
            cpssOsStrCpy(interfaceAttributes.instanceId,"mbus_x_u10_buses_1_21");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"u2xbar");
            cpssOsStrCpy(interfaceAttributes.instanceId,"mbus_x_u10_buses_1_21");
            interfaceAttributes.interfaceIndex = 2;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 3");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"u2xbar_rd");
            cpssOsStrCpy(interfaceAttributes.instanceId,"mbus_x_u10_buses_1_21");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 4");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"txq_q2txq_dq_q_main_rd_add");
            cpssOsStrCpy(interfaceAttributes.instanceId,"queue2dq_q_main_read_buses_2_7");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"TXDMA_2_HA_HEADER");
            cpssOsStrCpy(interfaceAttributes.instanceId, "txdma0_hdr");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PORT2MEM");
            cpssOsStrCpy(interfaceAttributes.instanceId, "rxdma0_desc_in_1");
            interfaceAttributes.interfaceIndex = 5;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"ERMRK_2_TXFIFO_HEADER_CTRL_BUS");
            cpssOsStrCpy(interfaceAttributes.instanceId, "egress_hdr");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 3");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"HA_2_ERMRK_HEADER_CTRL_BUS");
            cpssOsStrCpy(interfaceAttributes.instanceId, "egress_hdr");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 4");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"OAM_2_EQ_LOC_UPDATE");
            cpssOsStrCpy(interfaceAttributes.instanceId, "egress_hdr");
            interfaceAttributes.interfaceIndex = 2;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.3.Call function with udsId [7]
            And not valid interfaceAttributes (desc that is in predefine stage)
            Expected: GT_BAD_PARAM.
        */

        /*remove uds 3 first */
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 3");
        st = cpssDxChPacketAnalyzerUserDefinedStageDelete(managerId,&udsAttr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, udsAttr.udsNameArr);

        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"eq2eft_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_upper_ing_macro_eft_desc");
            interfaceAttributes.interfaceIndex = 2;
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"eq2eft_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_upper_ing_macro_eft_desc");
            interfaceAttributes.interfaceIndex = 2;
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"eq2eft_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_upper_ing_macro_eft_desc");
            interfaceAttributes.interfaceIndex = 2;
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"eq2eft_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe0_eagle_upper_ing_0_macro_eft_desc");
            interfaceAttributes.interfaceIndex = 0;
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"ipvx_2_ioam_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"l2i_ipvx_desc_or_debug_buses_0_8");
            interfaceAttributes.interfaceIndex = 3;
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"iplr02iplr1_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId, "upper_ingress_desc_push");
            interfaceAttributes.interfaceIndex = 2;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_ALREADY_EXIST, st, devNum, udsId);

        /*
            1.4. Call function with uds id already set
            Expected: GT_ALREADY_EXIST
        */
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 4");
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"CLIENT_2_CNC_UPDATE");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_tti_macro_erep_fc_msg");
            interfaceAttributes.interfaceIndex = 0;
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"CLIENT_2_CNC_UPDATE");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_tti_macro_erep_fc_msg");
            interfaceAttributes.interfaceIndex = 0;
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"CLIENT_2_CNC_UPDATE");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_tti_macro_erep_fc_msg");
            interfaceAttributes.interfaceIndex = 0;
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"LPM_2_SHM_AGING_RQ");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe1_eagle_l2i_ipvx_1_macro_mt_or_shm");
            interfaceAttributes.interfaceIndex = 3;
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"cpu2mem_rx_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"mg_mem_buses_1_21");
            interfaceAttributes.interfaceIndex = 0;
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"TXDMA_2_TXQ_CREDITS");
            cpssOsStrCpy(interfaceAttributes.instanceId, "txdma2_buses");
            interfaceAttributes.interfaceIndex = 2;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_ALREADY_EXIST, st, devNum, udsId);

        /*
            1.5. Call function with uds interface already set
            Expected: GT_ALREADY_EXIST
        */
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 3");
        switch (paMngDbPtr->devFamily) {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_txqp_macro_sdq");
            interfaceAttributes.interfaceIndex = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_txq_macro_sdq0");
            interfaceAttributes.interfaceIndex = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_hawk_txqs_0_macro_sdq0");
            interfaceAttributes.interfaceIndex = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId, "rxing2iaing_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId, "tile0_pipe0_rx_macro_i0_ia_desc1");
            interfaceAttributes.interfaceIndex = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"u2xbar");
            cpssOsStrCpy(interfaceAttributes.instanceId,"mbus_x_u10_buses_1_21");
            interfaceAttributes.interfaceIndex = 2;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PORT2MEM");
            cpssOsStrCpy(interfaceAttributes.instanceId, "rxdma0_desc_in_1");
            interfaceAttributes.interfaceIndex = 5;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_ALREADY_EXIST, st, devNum, udsId);

        /*
            1.6.Call function with uds interface name not exist in xml
            Expected: GT_BAD_PARAM
        */
            cpssOsStrCpy(udsAttr.udsNameArr,"uds bad interfaceId");
            cpssOsStrCpy(interfaceAttributes.interfaceId, "badName");
            cpssOsStrCpy(interfaceAttributes.instanceId, "rxdma1_ctrl_pipe_and_nextct");
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, udsId);
        /*
            1.7. Call function with uds instance name not exist in xml
            Expected: GT_BAD_PARAM
        */

            cpssOsStrCpy(udsAttr.udsNameArr,"uds bad instanceId");
            cpssOsStrCpy(interfaceAttributes.interfaceId, "EGF_2_TXQ_MC_CLEAR_BUS");
            cpssOsStrCpy(interfaceAttributes.instanceId, "BadName");
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, udsId);

        /*
            1.8. Call function with NULL udsAttrPtr
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,NULL,&interfaceAttributes,&udsId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, udsId);
        /*
            1.9. Call function with NULL interfaceAttrPtr
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,NULL,&udsId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, udsId);
        /*
            1.10. Call function with NULL udsIdPtr
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, udsId);

        /*
            1.11. Call function with empty udsIdPtr
            Expected: GT_BAD_PARAM
        */
        cpssOsMemSet(&udsAttr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC));
        cpssOsStrCpy(interfaceAttributes.interfaceId, "txq_txq2mg_cpu_ptx_pend");
        cpssOsStrCpy(interfaceAttributes.instanceId, "dq2mg_or_debug_buses_2_15");
        interfaceAttributes.interfaceIndex = 0;
        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId, &udsAttr, &interfaceAttributes, &udsId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, udsId);

        /*
            1.12. Call function with empty interfaceAttrPtr
            Expected: GT_BAD_PARA
        */
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 3");
        cpssOsMemSet(&interfaceAttributes,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, udsId);

        /*
            1.13. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, udsId);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageDelete
(
    IN GT_U32                                           managerId,
    IN CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC     udsAttr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerUserDefinedStageDelete)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerUserDefinedStageAdd
                            with udsId [0-2]
                            and valid interfaceAttributes
              call cpssDxChPacketAnalyzerUserDefinedStageGet and check valid == GT_TRUE
              delete all and check valid == GT_FALSE
    Expected: GT_OK.
    1.2. Call function with NULL udsAttr
    Expected: GT_BAD_PTR.
    1.3. Call function with empty udsAttr
    Expected: GT_BAD_PARAM.
    1.4. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          udsId;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC        interfaceAttributes;
    GT_BOOL                                              valid;
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC         udsAttr;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");

        cpssOsMemSet(&interfaceAttributes,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /*
            1.1.1 Call function with udsId [0/15/10]
                                    and valid interfaceAttributes
            Expected: GT_OK.
        */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_epi_mac_macro_i0_mif2mti_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PFC_MSG");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_txqp_macro_qfc_app");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"crdt_bkwd_tx");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_epi_mac_macro_i0_qfc_app");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_epi_100g_macro_mif2mti_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PFC_MSG");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_txq_macro_qfc0");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"crdt_bkwd_tx");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_pca_eip163_macro_eip163hi");
            interfaceAttributes.interfaceIndex = 4;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_hawk_epi_400g_mac_macro_i2_mif2mti_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PFC_MSG");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_hawk_epi_400g_mac_macro_i2_qfc_app");
            interfaceAttributes.interfaceIndex = 2;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"crdt_bkwd_tx");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_hawk_epi_usx_mac_macro_i0_pca2mif_tx_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"rxing2iaing_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe0_rx_macro_i0_ia_desc1");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"L2I_2_MT_LEARN");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe0_eagle_l2i_ipvx_0_macro_mt_or_shm");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"iaegr2rxegr_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe0_eagle_ia_0_macro_rx_desc3");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"mem2p_512_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"txfifo_desc_push_0_0_0_0");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"mem2p_64_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"txfifo_desc_push_2_0_0_0");
            interfaceAttributes.interfaceIndex = 7;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"txfifo_payload_cntr_dec");
            cpssOsStrCpy(interfaceAttributes.instanceId,"txfifo_credits_dec_bus_0_0_0");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.instanceId, "txdma0_hdr");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"TXDMA_2_HA_HEADER");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.instanceId, "rxdma0_desc_in_1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PORT2MEM");
            interfaceAttributes.interfaceIndex = 5;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
            cpssOsStrCpy(interfaceAttributes.instanceId, "egress_hdr");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"ERMRK_2_TXFIFO_HEADER_CTRL_BUS");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*check that all udses are valid */
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&interfaceAttributes);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
        UTF_VERIFY_EQUAL2_STRING_MAC( GT_TRUE , valid,
                            "got other valid than expected: %d for udsID : %d", valid ,udsId);
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&interfaceAttributes);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
        UTF_VERIFY_EQUAL2_STRING_MAC( GT_TRUE , valid,
                            "got other valid than expected: %d for udsID : %d", valid ,udsId);
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&interfaceAttributes);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
        UTF_VERIFY_EQUAL2_STRING_MAC( GT_TRUE , valid,
                            "got other valid than expected: %d for udsID : %d", valid ,udsId);

        /*delete all udses*/
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
        st = cpssDxChPacketAnalyzerUserDefinedStageDelete(managerId,&udsAttr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
        st = cpssDxChPacketAnalyzerUserDefinedStageDelete(managerId,&udsAttr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
        st = cpssDxChPacketAnalyzerUserDefinedStageDelete(managerId,&udsAttr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

        /*check that all udses are not  valid */
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 2");
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&interfaceAttributes);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
        UTF_VERIFY_EQUAL2_STRING_MAC( GT_FALSE , valid,
                    "got other valid than expected: %d for udsID : %d", valid ,udsId);
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&interfaceAttributes);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
        UTF_VERIFY_EQUAL2_STRING_MAC( GT_FALSE , valid,
                    "got other valid than expected: %d for udsID : %d", valid ,udsId);
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&interfaceAttributes);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
        UTF_VERIFY_EQUAL2_STRING_MAC( GT_FALSE , valid,
                    "got other valid than expected: %d for udsID : %d", valid ,udsId);

        /*
            1.2. Call function with NULL udsAttr
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerUserDefinedStageDelete(managerId,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
            1.3. Call function with empty udsAttr
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(udsAttr.udsNameArr,"");
        st = cpssDxChPacketAnalyzerUserDefinedStageDelete(managerId,&udsAttr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /*
            1.4. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerUserDefinedStageDelete(managerId,&udsAttr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, udsId);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerUserDefinedStageGet
(
    IN  GT_U32                                          managerId,
    IN  CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC    *udsAttrPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT     *udsIdPtr,
    OUT GT_BOOL                                         *validPtr,
    OUT CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC   *interfaceAttrPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerUserDefinedStageGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with udsId [0/1]
    Expected: GT_OK and same value as set with valid == GT_TRUE .
    1.2. Call function with udsId [4] that not exist
    Expected: GT_OK with valid == GT_FALSE  .
    1.3. Call function with empty udsAttrPtr and all other valid value
    Expected: GT_BAD_PARAM.
    1.4. Call function with NULL udsAttrPtr and all other valid value
    Expected: GT_BAD_PTR.
    1.5. Call function with NULL udsIdPtr and all other valid value
    Expected: GT_BAD_PTR.
    1.6. Call function with NULL validPtr and all other valid value
    Expected: GT_BAD_PTR.
    1.7. Call function with NULL interfaceAttrPtr and all other valid value
    Expected: GT_BAD_PTR.
    1.8. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          udsId;
    GT_BOOL                                              valid;
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC         udsAttr;

    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC        interfaceAttributes;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC        interfaceAttributesGet;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        /*init uds*/
        cpssOsMemSet(&interfaceAttributes,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
        cpssOsMemSet(&interfaceAttributesGet,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        switch(paMngDbPtr->devFamily)
        {
          case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_epi_mac_macro_i0_mif2mti_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            cpssOsStrCpy(interfaceAttributes.instanceId,"har_txqp_macro_sdq");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_epi_100g_macro_mif2mti_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            cpssOsStrCpy(interfaceAttributes.instanceId,"pnx_txq_macro_sdq0");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_hawk_epi_400g_mac_macro_i2_mif2mti_app");
            interfaceAttributes.interfaceIndex = 3;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_hawk_txqs_0_macro_sdq0");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"rxing2iaing_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe0_rx_macro_i0_ia_desc1");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"L2I_2_MT_LEARN");
            cpssOsStrCpy(interfaceAttributes.instanceId,"tile0_pipe0_eagle_l2i_ipvx_0_macro_mt_or_shm");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"mem2p_512_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId,"txfifo_desc_push_0_0_0_0");
            interfaceAttributes.interfaceIndex = 0;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"txfifo_payload_cntr_dec");
            cpssOsStrCpy(interfaceAttributes.instanceId,"txfifo_credits_dec_bus_0_0_0");
            interfaceAttributes.interfaceIndex = 1;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
            cpssOsStrCpy(interfaceAttributes.instanceId, "rxdma0_desc_in_1");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"PORT2MEM");
            interfaceAttributes.interfaceIndex = 5;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
            cpssOsStrCpy(interfaceAttributes.instanceId, "rxdma1_ctrl_pipe_and_nextct");
            cpssOsStrCpy(interfaceAttributes.interfaceId,"RXDMA_NEXT_CT");
            interfaceAttributes.interfaceIndex = 4;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&interfaceAttributes,&udsId);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.1 Call function with udsId [1]
            Expected: GT_OK and same value as set
        */
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&interfaceAttributesGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_TRUE , valid,
                            "got other valid than expected: %d", valid);
        UTF_VERIFY_EQUAL1_STRING_MAC( interfaceAttributes.interfaceIndex , interfaceAttributesGet.interfaceIndex,
                                      "got other interfaceIndex than expected: %d", interfaceAttributesGet.interfaceIndex);
        /*
            1.1.2 Call function with udsId [0]
            Expected: GT_OK and same value as set
        */
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&interfaceAttributesGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( GT_TRUE , valid,
                            "got other valid than expected: %d", valid);

        /*
            1.2. Call function with udsId [4] that not exist
            Expected: GT_OK with valid == GT_FALSE  .
        */
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 4");
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId, &udsAttr, &udsId, &valid, &interfaceAttributesGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_FALSE, valid,
                                     "got other valid than expected: %d", valid);

        /*
            1.3. Call function with empty udsAttrPtr and all other valid value
            Expected: GT_BAD_PARAM.
        */
        cpssOsMemSet(&udsAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC));
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&interfaceAttributesGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, udsId);

        /*
            1.4. Call function with NULL udsAttrPtr and all other valid value
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,NULL,&udsId,&valid,&interfaceAttributesGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, udsId);

        /*
            1.5. Call function with NULL udsIdPtr and all other valid value
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,NULL,&valid,&interfaceAttributesGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, udsId);

        /*
            1.6. Call function with NULL udsAttrPtr and all other valid value
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,NULL,&interfaceAttributesGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, udsId);

        /*
            1.7. Call function with NULL interfaceAttributesPtr and all other valid value
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, udsId);

        /*
            1.8. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerUserDefinedStageGet(managerId,&udsAttr,&udsId,&valid,&interfaceAttributesGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, udsId);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerStageInterfacesSet
(
    IN GT_U32                                           managerId,
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT      stageId,
    IN CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC    boundInterface
);
*/

UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerStageInterfacesSet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E]
    Expected: GT_OK.
    1.2. Call function with wrong stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E] .
    Expected: GT_BAD_PARAM.
    1.4. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stageId;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC    boundInterface;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        cpssOsMemSet(&boundInterface,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*
            1.1.1 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E]
                                    subStageId [1]
                                    numOfBoundInterfaces [1]
            Expected: GT_OK.
        */
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        st = prvCpssDxChPacketAnalyzerStageInterfacesSet(managerId,devNum,stageId,boundInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stageId);

        /*
            1.2. Call function with wrong stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E ;
        st = prvCpssDxChPacketAnalyzerStageInterfacesSet(managerId,devNum,stageId,boundInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, stageId);

        /*
            1.3. Call function with wrong stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E] .
            Expected: GT_BAD_PARAM.
        */
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E ;
        st = prvCpssDxChPacketAnalyzerStageInterfacesSet(managerId,devNum,stageId,boundInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, stageId);

        /*
            1.4. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerStageInterfacesSet(managerId,devNum,stageId,boundInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, stageId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        managerId = GENERIC_MANAGER_ID ;
        stageId = 0;
        cpssOsMemSet(&boundInterface,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
        st = prvCpssDxChPacketAnalyzerStageInterfacesSet(managerId,devNum,stageId,boundInterface);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, stageId);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerStageInterfacesGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U8                                             devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stageId,
    OUT   CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC     *boundInterfacePtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerStageInterfacesGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E]
    Expected: GT_OK and same values as set.
    1.2. Call function with wrong stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E] .
    Expected: GT_BAD_PARAM.
    1.4. Call with NULL boundInterfacePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stageId;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC    boundInterfaceArrExp;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC    boundInterfaceArrGet;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        cpssOsMemSet(&boundInterfaceArrExp,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
        cpssOsMemSet(&boundInterfaceArrGet,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        st = prvCpssDxChPacketAnalyzerStageInterfacesSet(managerId,devNum,stageId,boundInterfaceArrExp);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stageId);
        /*
            1.1. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E]
                                    subStageId [1]
                                    numOfApplicInterfaces[20]
                                    numOfBoundInterfacesPtr [7]
            Expected: GT_OK and same values as set.
        */
        st = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&boundInterfaceArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stageId);

        /*
            1.2. Call function with wrong stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E ;
        st = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&boundInterfaceArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, stageId);

        /*
            1.3. Call function with wrong stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E] .
            Expected: GT_BAD_PARAM.
        */
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E ;
        st = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&boundInterfaceArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, stageId);

        /*
            1.4. Call with NULL boundInterfacePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        st = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, stageId);

        /*
            1.5. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&boundInterfaceArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, stageId);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerStageFieldsGet
(
    IN    GT_U32                                            managerId,
    IN    GT_U8                                             devNum,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT       stageId,
    INOUT GT_U32                                            *numOfFieldsPtr,
    OUT   PRV_CPSS_DXCH_IDEBUG_FIELD_STC                    fieldsArr[]
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerStageFieldsGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E/
                                     CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E/
                                     CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E/
                                     CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E]
                             subStage [1]
                             numOfFields[SIZE_OF_TEST_ARR*10]
    Expected: GT_OK.
    1.2. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_FIRST_E]
                            subStage [1]
                            numOfFields[SIZE_OF_TEST_ARR*10]
    Expected: GT_BAD_PARAM.
    1.3. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E]
                            subStage [1]
                            numOfFields[0]
    Expected: GT_BAD_SIZE.
    1.4. Call with NULL numOfFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL fieldsArr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stage ;
    GT_U32                                               numOfFieldsGet;
    PRV_CPSS_DXCH_IDEBUG_FIELD_STC                       fieldsArrGet[SIZE_OF_TEST_ARR*10];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        cpssOsMemSet(fieldsArrGet, 0, sizeof(fieldsArrGet));
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /*
            1.1.1 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E]
                                     subStage [1]
                                     numOfFields[SIZE_OF_TEST_ARR*10]
            Expected: GT_OK.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(11, numOfFieldsGet,
                           "got other numOfFieldsGet than expected: %d", numOfFieldsGet );
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(17, numOfFieldsGet,
                           "got other numOfFieldsGet than expected: %d", numOfFieldsGet );
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(16, numOfFieldsGet,
                           "got other numOfFieldsGet than expected: %d", numOfFieldsGet );
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.2 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E]
                                     subStage [1]
                                     numOfFields[SIZE_OF_TEST_ARR*10]
            Expected: GT_OK.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(183, numOfFieldsGet,
                           "got other numOfFieldsGet than expected: %d", numOfFieldsGet );
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(181, numOfFieldsGet,
                           "got other numOfFieldsGet than expected: %d", numOfFieldsGet );
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(180, numOfFieldsGet,
                           "got other numOfFieldsGet than expected: %d", numOfFieldsGet);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(176, numOfFieldsGet,
                           "got other numOfFieldsGet than expected: %d", numOfFieldsGet);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.3 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E]
                                     subStage [1]
                                     numOfFields[SIZE_OF_TEST_ARR*10]
            Expected: GT_OK.
        */

        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(136, numOfFieldsGet,
                           "got other numOfFieldsGet than expected: %d", numOfFieldsGet );
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(132, numOfFieldsGet,
                                         "got other numOfFieldsGet than expected: %d", numOfFieldsGet );
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(133, numOfFieldsGet,
                                         "got other numOfFieldsGet than expected: %d", numOfFieldsGet);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(130, numOfFieldsGet,
                                         "got other numOfFieldsGet than expected: %d", numOfFieldsGet);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.4 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E]
                                     subStage [1]
                                     numOfFields[SIZE_OF_TEST_ARR*10]
            Expected: GT_OK.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TRANSMIT_QUEUE_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(21, numOfFieldsGet,
                           "got other numOfFieldsGet than expected: %d", numOfFieldsGet );
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(21, numOfFieldsGet,
                                         "got other numOfFieldsGet than expected: %d", numOfFieldsGet );
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(134, numOfFieldsGet,
                                         "got other numOfFieldsGet than expected: %d", numOfFieldsGet);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(131, numOfFieldsGet,
                                         "got other numOfFieldsGet than expected: %d", numOfFieldsGet);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.2. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_FIRST_E]
                                    subStage [1]
                                    numOfFields[SIZE_OF_TEST_ARR*10]
            Expected: GT_BAD_PARAM.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum,stage);

        /*
            1.3. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E]
                                    subStage [1]
                                    numOfFields[0]
            Expected: GT_BAD_SIZE.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E ;
        numOfFieldsGet = 0;
        st = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum,stage);

        /*
            1.4. Call with NULL numOfFieldsPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stage,NULL,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,stage);

        /*                                                         .
            1.5. Call with NULL fieldsArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stage,&numOfFieldsGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,stage);
        /*                                                    .
            1.6. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum,stage);


    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = prvCpssDxChPacketAnalyzerStageFieldsGet(managerId,devNum,stage,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum,stage);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerInstanceInterfacesGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U8                                         devNum,
    IN    GT_CHAR_PTR                                   instanceId,
    INOUT GT_U32                                        *numOfInterfacesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC interfacesArr[]
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerInstanceInterfacesGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with instanceId [egress_desc_or_hdr_1_3]
                            numOfInterfaces[20]
    Expected: GT_OK and same values as set.
    1.2. Call function with wrong instanceId [PRV_CPSS_DXCH_IDEBUG_INSTANCE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call function with too low numOfInterfaces value [2] .
    Expected: GT_BAD_SIZE.
    1.4. Call with NULL numOfInterfaces and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL interfacesArr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_CHAR                                              instanceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                               numOfInterfaces;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC        interfacesArrGet[SIZE_OF_TEST_ARR];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        cpssOsMemSet(interfacesArrGet,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC)*SIZE_OF_TEST_ARR);

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /*
            1.1.1 Call function with instanceId [egress_desc_or_hdr_1_3]
                                    numOfInterfaces[20]
            Expected: GT_OK and same values as set.
        */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(instanceId,"har_upper_ing_macro_iplr0_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(instanceId,"pnx_upper_ing_macro_iplr0_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(instanceId,"tile0_upper_ing_macro_iplr0_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(instanceId,"tile0_pipe0_eagle_epcl_0_macro_epcl_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
             cpssOsStrCpy(instanceId,"egress_desc_or_hdr_1_3");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
             cpssOsStrCpy(instanceId,"egress_desc");
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        numOfInterfaces = 25 ;
        st = prvCpssDxChPacketAnalyzerInstanceInterfacesGet(managerId,devNum,instanceId,&numOfInterfaces,interfacesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, instanceId);

        /*
            1.2. Call function with wrong instanceId [PRV_CPSS_DXCH_IDEBUG_INSTANCE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(instanceId,"");
        numOfInterfaces = 20 ;
        st = prvCpssDxChPacketAnalyzerInstanceInterfacesGet(managerId,devNum,instanceId,&numOfInterfaces,interfacesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, instanceId);

        /*
            1.3. Call function with too low numOfInterfaces value [2] .
            Expected: GT_BAD_SIZE.
        */
        switch(paMngDbPtr->devFamily)
        {
          case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(instanceId,"har_upper_ing_macro_mll_eq_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(instanceId,"pnx_upper_ing_macro_mll_eq_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(instanceId,"tile0_upper_ing_macro_mll_eq_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(instanceId,"tile0_pipe0_eagle_l2i_ipvx_0_macro_ipvx_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(instanceId,"txq2txdma_or_sniff_2_8");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
             cpssOsStrCpy(instanceId,"txfifo2_desc_push_7");
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        numOfInterfaces = 2 ;
        st = prvCpssDxChPacketAnalyzerInstanceInterfacesGet(managerId,devNum,instanceId,&numOfInterfaces,interfacesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum, instanceId);

        /*
            1.4. Call with NULL numOfInterfaces and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfInterfaces = 20 ;
        st = prvCpssDxChPacketAnalyzerInstanceInterfacesGet(managerId,devNum,instanceId,NULL,interfacesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, instanceId);

        /*
            1.5. Call with NULL interfacesArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfInterfaces = 20 ;
        st = prvCpssDxChPacketAnalyzerInstanceInterfacesGet(managerId,devNum,instanceId,&numOfInterfaces,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, instanceId);

        /*
            1.6. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        numOfInterfaces = 20 ;
        st = prvCpssDxChPacketAnalyzerInstanceInterfacesGet(managerId,devNum,instanceId,&numOfInterfaces,interfacesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, instanceId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        numOfInterfaces = 20 ;
        st = prvCpssDxChPacketAnalyzerInstanceInterfacesGet(managerId,devNum,instanceId,&numOfInterfaces,interfacesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, instanceId);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*
GT_STATUS prvCpssDxChPacketAnalyzerFieldInterfacesGet
(
    IN    GT_U32                                  managerId,
    IN    GT_U8                                   devNum,
    IN    GT_CHAR_PTR                             fieldName,
    INOUT GT_U32                                  *numOfInterfacesPtr,
    OUT   GT_CHAR                                 *interfacesArr[]
)
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerFieldInterfacesGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with fieldName      [packet_cmd\
                                            evlan\
                                            is_ip\
                                            mac_da\
                                            tm_queue_id]
    Expected: GT_OK
    1.2. Call function with wrong fieldName [PRV_CPSS_DXCH_IDEBUG_FIELD_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call with numOfInterfaces less than needed .
    Expected: GT_BAD_SIZE.
    1.4. Call with NULL numOfInterfacesPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL interfacesArr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId,i;

    GT_CHAR                                              fieldsName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                               numOfInterfaces;
    GT_CHAR_PTR                                          *interfaceArr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        interfaceArr  = (GT_CHAR_PTR*)cpssOsMalloc(sizeof(GT_CHAR_PTR)*SIZE_OF_TEST_ARR*2);
        if (interfaceArr == NULL)
        {
            st = GT_OUT_OF_CPU_MEM;
            goto exit_cleanly_lbl;
        }

        cpssOsMemSet(interfaceArr,0,sizeof(GT_CHAR_PTR)*SIZE_OF_TEST_ARR*2);
        for (i=0; i<SIZE_OF_TEST_ARR*2; i++)
        {
            interfaceArr[i] = (GT_CHAR_PTR)cpssOsMalloc(sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS);
            if (interfaceArr[i] == NULL)
            {
                st = GT_OUT_OF_CPU_MEM;
                goto exit_cleanly_lbl;
            }
            cpssOsMemSet(interfaceArr[i],0,sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS);
        }
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*
            1.1.1 Call function with fieldName [packet_cmd]
            Expected: GT_OK
        */
        cpssOsStrCpy(fieldsName,"packet_cmd");
        numOfInterfaces = SIZE_OF_TEST_ARR*2 ;
        st = prvCpssDxChPacketAnalyzerFieldInterfacesGet(managerId,devNum,fieldsName,&numOfInterfaces,interfaceArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, fieldsName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(11, numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 9 , numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 9 , numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.2 Call function with fieldName [evlan]
            Expected: GT_OK
        */

        cpssOsStrCpy(fieldsName,"evlan");
        numOfInterfaces = SIZE_OF_TEST_ARR*2 ;
        st = prvCpssDxChPacketAnalyzerFieldInterfacesGet(managerId,devNum,fieldsName,&numOfInterfaces,interfaceArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, fieldsName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
             UTF_VERIFY_EQUAL1_STRING_MAC(19, numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(20, numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(22, numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 25 , numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 18 , numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         default:
             UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.3 Call function with fieldName [is_ip]
            Expected: GT_OK
        */
        cpssOsStrCpy(fieldsName,"is_ip");
        numOfInterfaces = SIZE_OF_TEST_ARR*2 ;
        st = prvCpssDxChPacketAnalyzerFieldInterfacesGet(managerId,devNum,fieldsName,&numOfInterfaces,interfaceArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, fieldsName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(8, numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 9 , numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 8 , numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.4 Call function with fieldName [mac_da]
            Expected: GT_OK
        */
        cpssOsStrCpy(fieldsName,"mac_da");
        numOfInterfaces = SIZE_OF_TEST_ARR*2 ;
        st = prvCpssDxChPacketAnalyzerFieldInterfacesGet(managerId,devNum,fieldsName,&numOfInterfaces,interfaceArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, fieldsName);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( 2 , numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);

        /*
            1.2. Call function with wrong fieldName [PRV_CPSS_DXCH_IDEBUG_FIELD_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(fieldsName,"");
        numOfInterfaces = SIZE_OF_TEST_ARR*2 ;
        st = prvCpssDxChPacketAnalyzerFieldInterfacesGet(managerId,devNum,fieldsName,&numOfInterfaces,interfaceArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, fieldsName);

        /*
            1.3. Call with numOfInterfaces less than needed .
            Expected: GT_BAD_SIZE.
        */
        cpssOsStrCpy(fieldsName,"mac_da");
        numOfInterfaces = 1 ;
        st = prvCpssDxChPacketAnalyzerFieldInterfacesGet(managerId,devNum,fieldsName,&numOfInterfaces,interfaceArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum, fieldsName);

        /*
            1.4. Call with NULL numOfInterfaces and other valid params.
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(fieldsName,"mac_da");
        numOfInterfaces = SIZE_OF_TEST_ARR*2 ;
        st = prvCpssDxChPacketAnalyzerFieldInterfacesGet(managerId,devNum,fieldsName,NULL,interfaceArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, fieldsName);

        /*
            1.5. Call with NULL interfaceArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = prvCpssDxChPacketAnalyzerFieldInterfacesGet(managerId,devNum,fieldsName,&numOfInterfaces,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, fieldsName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

    }

exit_cleanly_lbl:

    if (interfaceArr)
    {
        for (i=0; i<SIZE_OF_TEST_ARR*2; i++)
            if (interfaceArr[i])
                cpssOsFree(interfaceArr[i]);
        cpssOsFree(interfaceArr);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at memory allocation ");
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerInterfacesGet
(
    IN    GT_U32                                    managerId,
    IN    GT_U8                                     devNum,
    INOUT GT_U32                                    *numOfInterfacesPtr,
    OUT   GT_CHAR                                   **interfacesArr[]
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerInterfacesGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with numOfInterfaces[20]
    Expected: GT_OK and same values as set.
    1.2. Call function with too low numOfInterfaces value [2] .
    Expected: GT_BAD_SIZE.
    1.3. Call with NULL numOfInterfacesPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL interfacesArr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_U32                                               numOfInterfaces;
    GT_CHAR_PTR                                          *interfacesArrGet = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /*
            1.1. Call function with numOfInterfaces[210]
            Expected: GT_OK and same values as set.
        */
        numOfInterfaces = SIZE_OF_TEST_ARR * 10;
        st = prvCpssDxChPacketAnalyzerInterfacesGet(managerId, devNum, &numOfInterfaces, &interfacesArrGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
             UTF_VERIFY_EQUAL1_STRING_MAC(216, numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
             break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(213, numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(218, numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(251, numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC(119, numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 84 , numOfInterfaces,
                                     "got other numOfInterfaces than expected: %d", numOfInterfaces);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.2. Call function with too low numOfInterfaces value [2] .
            Expected: GT_BAD_SIZE.
        */
        numOfInterfaces = 2 ;
        st = prvCpssDxChPacketAnalyzerInterfacesGet(managerId, devNum,&numOfInterfaces,&interfacesArrGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_SIZE, st, devNum);

        /*
            1.4. Call with NULL numOfInterfaces and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfInterfaces = SIZE_OF_TEST_ARR*10 ;
        st = prvCpssDxChPacketAnalyzerInterfacesGet(managerId, devNum,NULL,&interfacesArrGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
            1.4. Call with NULL interfacesArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfInterfaces = SIZE_OF_TEST_ARR*10 ;
        st = prvCpssDxChPacketAnalyzerInterfacesGet(managerId, devNum,&numOfInterfaces,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
            1.5. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        numOfInterfaces = SIZE_OF_TEST_ARR * 10;
        st = prvCpssDxChPacketAnalyzerInterfacesGet(managerId, devNum, &numOfInterfaces, &interfacesArrGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, devNum);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        managerId = GENERIC_MANAGER_ID ;
        numOfInterfaces = SIZE_OF_TEST_ARR*10 ;
        st = prvCpssDxChPacketAnalyzerInterfacesGet(managerId, devNum,&numOfInterfaces,&interfacesArrGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerInterfaceInfoGet
(
    IN    GT_U32                                        managerId,
    IN    GT_U8                                         devNum,
    IN    GT_CHAR_PTR                                   interfaceName,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   *stageIdPtr,
    OUT   GT_CHAR                                       *instanceIdPtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerInterfaceInfoGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with interfaceName  [eoam2eplr_desc\
                                            mll2eq_desc\
                                            eq2eft_desc\
                                            pcl2l2i_desc]
    Expected: GT_OK and same values as set.
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL stageIdPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call with NULL subStageIdPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL instanceIdPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call function when manager is not initialized .
    Expected: GT_BAD_STATE.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stageIdGet;
    GT_CHAR                                              instanceIdGet[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_CHAR                                              expInstanceId[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /*
            1.1.1 Call function with interfaceName [eoam2eplr_desc]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        st = prvCpssDxChPacketAnalyzerInterfaceInfoGet(managerId, devNum,interfaceName,&stageIdGet,instanceIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(expInstanceId,"har_epcl_macro_epcl_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(expInstanceId,"pnx_epcl_macro_epcl_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(expInstanceId,"tile0_epcl_ha_macro_epcl_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(expInstanceId,"tile0_pipe1_eagle_epcl_1_macro_epcl_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(expInstanceId,"egress_desc_or_hdr_1_3");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(expInstanceId,"egress_desc");
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        UTF_VERIFY_EQUAL1_STRING_CMP_MAC( expInstanceId , instanceIdGet,
                                     "got other instanceIdGet than expected: %s", instanceIdGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E, stageIdGet,
                                     "got other stageIdGet than expected: %d", stageIdGet);

        /*
            1.1.2 Call function with interfaceName [mll2eq_desc]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        st = prvCpssDxChPacketAnalyzerInterfaceInfoGet(managerId, devNum,interfaceName,&stageIdGet,instanceIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
             cpssOsStrCpy(expInstanceId,"har_upper_ing_macro_mll_eq_desc");
             break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(expInstanceId,"pnx_upper_ing_macro_mll_eq_desc");
             break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(expInstanceId,"tile0_upper_ing_macro_mll_eq_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(expInstanceId,"tile0_pipe1_eagle_upper_ing_1_macro_mll_eq_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(expInstanceId,"mll_eq_desc_0_10");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(expInstanceId,"upper_ingress_desc_push");
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_CMP_MAC( expInstanceId , instanceIdGet,
                                     "got other instanceIdGet than expected: %s", instanceIdGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E, stageIdGet,
                                     "got other stageIdGet than expected: %d", stageIdGet);

        /*
            1.1.3 Call function with interfaceName [eq2eft_desc/eq2egf_desc]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"eq2eft_desc");
        if( paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
           cpssOsStrCpy(interfaceName,"eq2egf_desc");

        st = prvCpssDxChPacketAnalyzerInterfaceInfoGet(managerId, devNum,interfaceName,&stageIdGet,instanceIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(expInstanceId,"har_upper_ing_macro_eft_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(expInstanceId,"pnx_upper_ing_macro_eft_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(expInstanceId,"tile0_upper_ing_macro_eft_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(expInstanceId,"tile0_pipe1_eagle_upper_ing_1_macro_eft_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
             cpssOsStrCpy(expInstanceId,"mll_eq_desc_0_10");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
             cpssOsStrCpy(expInstanceId,"upper_ingress_desc_push");
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_CMP_MAC( expInstanceId , instanceIdGet,
                                     "got other instanceIdGet than expected: %s", instanceIdGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E, stageIdGet,
                                     "got other stageIdGet than expected: %d", stageIdGet);
        /*
            1.1.4 Call function with interfaceName [pcl2l2i_desc]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        st = prvCpssDxChPacketAnalyzerInterfaceInfoGet(managerId, devNum,interfaceName,&stageIdGet,instanceIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        switch(paMngDbPtr->devFamily)
        {
          case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(expInstanceId,"har_pcl_macro_pcl_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
           cpssOsStrCpy(expInstanceId,"pnx_pcl_macro_pcl_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(expInstanceId,"tile0_pcl_macro_pcl_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(expInstanceId,"tile0_pipe1_eagle_pcl_1_macro_pcl_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
             cpssOsStrCpy(expInstanceId,"pcl_desc_0_19");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
             cpssOsStrCpy(expInstanceId,"pcl_l2i_desc");
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_CMP_MAC( expInstanceId , instanceIdGet,
                                     "got other instanceIdGet than expected: %s", instanceIdGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E, stageIdGet,
                                     "got other stageIdGet than expected: %d", stageIdGet);

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        st = prvCpssDxChPacketAnalyzerInterfaceInfoGet(managerId, devNum,interfaceName,&stageIdGet,instanceIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.3. Call with NULL stageIdPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(interfaceName,"l2i2ipvx_desc");
        st = prvCpssDxChPacketAnalyzerInterfaceInfoGet(managerId, devNum,interfaceName,NULL,instanceIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*
            1.5. Call with NULL instanceIdPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = prvCpssDxChPacketAnalyzerInterfaceInfoGet(managerId, devNum,interfaceName,&stageIdGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*
            1.6. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerInterfaceInfoGet(managerId, devNum,interfaceName,&stageIdGet,instanceIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, interfaceName);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"l2i2ipvx_desc");
        st = prvCpssDxChPacketAnalyzerInterfaceInfoGet(managerId, devNum,interfaceName,&stageIdGet,instanceIdGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChPacketAnalyzerFieldSizeGet
(
    IN  GT_U32                                  managerId,
    IN  GT_U8                                   devNum,
    IN  GT_CHAR_PTR                             fieldName,
    OUT GT_U32                                  *lengthPtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerFieldSizeGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with fieldName [mac_da     /
                                       is_ip,      /
                                       byte_count /
                                       packet_cmd /
                                       ipv4_sip ]
    Expected: GT_OK with Expected value .
    1.2. Call function with wrong value fieldName [PRV_CPSS_DXCH_IDEBUG_FIELD_LAST_E]
    Expected: GT_BAD_PARAM.
    1.3. Call function null lengthPtr .
    Expected: GT_BAD_PTR.
    1.4. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                           st = GT_OK;
    GT_U8                                               devNum;
    GT_U32                                              managerId;
    GT_CHAR                                             fieldName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                              length;
    GT_U32                                              lengthGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");

        /*
            1.1.1 Call function with fieldName [mac_da]
            Expected: GT_OK with Expected value .
        */
        cpssOsStrCpy(fieldName,"mac_da");
        length = 48 ;
        st = prvCpssDxChPacketAnalyzerFieldSizeGet(managerId,devNum,fieldName,&lengthGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
            1.1.2 Call function with fieldName [is_ip]
            Expected: GT_OK with Expected value .
        */
        cpssOsStrCpy(fieldName,"is_ip");
        length = 1 ;
        st = prvCpssDxChPacketAnalyzerFieldSizeGet(managerId,devNum,fieldName,&lengthGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
            1.1.3 Call function with fieldName [byte_count]
            Expected: GT_OK with Expected value .
        */
        cpssOsStrCpy(fieldName,"byte_count");
        length = 14 ;
        prvCpssDxChPacketAnalyzerFieldSizeGet(managerId,devNum,fieldName,&lengthGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
            1.1.4 Call function with fieldName [packet_cmd]
            Expected: GT_OK with Expected value .
        */
        cpssOsStrCpy(fieldName,"packet_cmd");
        length = 3 ;
        st = prvCpssDxChPacketAnalyzerFieldSizeGet(managerId,devNum,fieldName,&lengthGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
            1.1.5 Call function with fieldName [ipv4_sip ]
            Expected: GT_OK with Expected value .
        */
        cpssOsStrCpy(fieldName,"ipv4_sip");
        length = 32 ;
        st = prvCpssDxChPacketAnalyzerFieldSizeGet(managerId,devNum,fieldName,&lengthGet);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(length, lengthGet,
                       "got other lengthGet than was set: %d", managerId);

        /*
            1.2. Call function with wrong value fieldName [PRV_CPSS_DXCH_IDEBUG_FIELD_LAST_E]
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(fieldName,"");
        st = prvCpssDxChPacketAnalyzerFieldSizeGet(managerId,devNum,fieldName,&lengthGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, managerId, devNum);

        /*
            1.3. Call function null lengthPtr .
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(fieldName,"packet_cmd");
        st = prvCpssDxChPacketAnalyzerFieldSizeGet(managerId,devNum,fieldName,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, managerId, devNum);

        /*
            1.4. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerFieldSizeGet(managerId,devNum,fieldName,&lengthGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, managerId, devNum);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(fieldName,"ipv4_sip");
        st = prvCpssDxChPacketAnalyzerFieldSizeGet(managerId,devNum,fieldName,&lengthGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugInterfaceNumFieldsGet
(
    IN  GT_U8                                   devNum,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_ENT      interfaceName,
    OUT GT_U32                                  *numOfFieldsPtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugInterfaceNumFieldsGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with interfaceName  [eoam2eplr_desc\
                                            mll2eq_desc\
                                            pcl2l2i_desc]
    Expected: GT_OK
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL numOfFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                               numOfFields;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*
            1.1.1 Call function with interfaceName [eoam2eplr_desc]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        st = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceName,&numOfFields);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
             UTF_VERIFY_EQUAL1_STRING_MAC( 177 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 172 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 92 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 91 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.2 Call function with interfaceName [mll2eq_desc]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        st = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceName,&numOfFields);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 136 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 134 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 132 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 130 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.4 Call function with interfaceName [pcl2l2i_desc]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        st = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceName,&numOfFields);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 183 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 181 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 180 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 176 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        st = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceName,&numOfFields);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.3. Call with NULL numOfFieldsPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        st = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceName,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        st = prvCpssDxChIdebugInterfaceNumFieldsGet(devNum,interfaceName,&numOfFields);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugInterfaceFieldsGet
(
    IN    GT_U8                                   devNum,
    IN    GT_CHAR_PTR                             interfaceName,
    INOUT GT_U32                                  *numOfFieldsPtr,
    OUT   PRV_CPSS_DXCH_IDEBUG_FIELD_STC          fieldsArr[]
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugInterfaceFieldsGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with interfaceName  [eoam2eplr_desc\
                                            mll2eq_desc\
                                            pcl2l2i_desc]
    Expected: GT_OK
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call with numOfFieldsPtr less than needed .
    Expected: GT_BAD_SIZE.
    1.4. Call with NULL numOfFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL fieldsArr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                               numOfFields;
    PRV_CPSS_DXCH_IDEBUG_FIELD_STC                       fieldsArr[SIZE_OF_TEST_ARR*10 +10];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*
            1.1.1 Call function with interfaceName [eoam2eplr_desc]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        numOfFields = SIZE_OF_TEST_ARR*10 ;
        cpssOsMemSet(&fieldsArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC));
        st = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceName,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 177 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
           break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 172 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
           break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 92 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
           break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 91 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
           break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.2 Call function with interfaceName [mll2eq_desc]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        numOfFields = SIZE_OF_TEST_ARR*10 ;
        cpssOsMemSet(&fieldsArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC));
        st = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceName,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 136 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
           break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 134 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
           break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 132 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
           break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 130 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
           break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.3 Call function with interfaceName [pcl2l2i_desc]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        numOfFields = SIZE_OF_TEST_ARR*10 ;
        cpssOsMemSet(&fieldsArr,0,sizeof(PRV_CPSS_DXCH_IDEBUG_FIELD_STC));
        st = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceName,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 183 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
           break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 181 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 180 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 176 , numOfFields,
                                         "got other numOfFields than expected: %d", numOfFields);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INSTANCE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        numOfFields = SIZE_OF_TEST_ARR ;
        st = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceName,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.3. Call with numOfFieldsPtr less than needed .
            Expected: GT_BAD_SIZE.
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        numOfFields = 1 ;
        st = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceName,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum, interfaceName);

        /*
            1.4. Call with NULL numOfFieldsPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        numOfFields = SIZE_OF_TEST_ARR ;
        st = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceName,NULL,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*
            1.5. Call with NULL fieldsArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceName,&numOfFields,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        numOfFields = SIZE_OF_TEST_ARR ;
        st = prvCpssDxChIdebugInterfaceFieldsGet(devNum,interfaceName,&numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSet
(
    IN  GT_U8                                     devNum,
    IN  GT_PORT_GROUPS_BMP                        portGroupsBmp,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_ENT        interfaceName,
    IN  GT_U32                                    numOfFields,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC  fieldsValueArr[]
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugPortGroupInterfaceSet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                            interfaceName  [eoam2eplr_desc\
                                            ermrk2td_desc\
                                            ioam2iplr0_desc\
                                            mem2p_256_desc\
                                            pcl2l2i_desc\
                                            txq2qcn_desc]
                            numOfFields     [2\3\1\2\2\4];
                            fieldsValueArr  [byte_count,evlan\
                                             local_dev_src_port,cut_through_id,valid_bytes\
                                             dec_ttl\
                                             egress_byte_count,udp_checksum_offset\
                                             byte_count,ipv4_dip\
                                             egress_filter_registered,byte_count,evlan,queue_buff_cntr]
    Expected: GT_OK
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong fieldsValue [PRV_CPSS_DXCH_IDEBUG_FIELD_LAST_E] .
    Expected: GT_BAD_STATE.
    1.4. Call with NULL fieldsValueArr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_PORT_GROUPS_BMP                                   portGroupsBmp;
    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                               numOfFields;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC             fieldsValueArr[SIZE_OF_TEST_ARR];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsMemSet(&fieldsValueArr, 0, sizeof(PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC));
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS ;

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*
            1.1.1 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [eoam2eplr_desc]
                                     numOfFields     [2];
                                     fieldsValueArr  [byte_count,evlan]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        numOfFields = 2;
        cpssOsStrCpy(fieldsValueArr[0].fieldName,"byte_count");
        cpssOsStrCpy(fieldsValueArr[1].fieldName,"evlan");
        st = prvCpssDxChIdebugPortGroupInterfaceSet(devNum,portGroupsBmp,interfaceName,numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.1.2 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [rxdma2ctrl_pipe_desc / hbuing2tti_desc ]
                                     numOfFields     [3];
                                     fieldsValueArr  [timestamp,cpu_code,first_buffer]
            Expected: GT_OK
        */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(interfaceName,"hbuing2tti_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(interfaceName,"rxdma2ctrl_pipe_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(interfaceName,"RXDMA_2_CTRL_PIPE");
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        numOfFields = 3;
        cpssOsStrCpy(fieldsValueArr[0].fieldName,"timestamp");
        cpssOsStrCpy(fieldsValueArr[1].fieldName,"cpu_code");
        cpssOsStrCpy(fieldsValueArr[2].fieldName,"first_buffer");
        st = prvCpssDxChIdebugPortGroupInterfaceSet(devNum,portGroupsBmp,interfaceName,numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        st = prvCpssDxChIdebugPortGroupInterfaceSet(devNum,portGroupsBmp,interfaceName,numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.3. Call function with wrong fieldsValue [PRV_CPSS_DXCH_IDEBUG_FIELD_LAST_E] .
            Expected: GT_OUT_OF_RANGE.
        */
        cpssOsStrCpy(interfaceName,"iplr02iplr1_desc");
        numOfFields = 1;
        cpssOsStrCpy(fieldsValueArr[0].fieldName,"");
        st = prvCpssDxChIdebugPortGroupInterfaceSet(devNum,portGroupsBmp,interfaceName,numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum, interfaceName);

        /*
            1.4. Call with NULL fieldsValueArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(interfaceName,"iplr02iplr1_desc");
        cpssOsStrCpy(fieldsValueArr[0].fieldName,"packet_cmd");
        st = prvCpssDxChIdebugPortGroupInterfaceSet(devNum,portGroupsBmp,interfaceName,numOfFields,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        numOfFields = 1;
        st = prvCpssDxChIdebugPortGroupInterfaceSet(devNum,portGroupsBmp,interfaceName,numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceReset
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_ENT      interfaceName
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugPortGroupInterfaceReset)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                            and valid interfaces
    Expected: GT_OK
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_PORT_GROUPS_BMP                                   portGroupsBmp;
    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {


        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*
            1.1.1 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [hbuing2tti_desc/eoam2eplr_desc/TTI_2_PCL_UPPER_DESC]
            Expected: GT_OK
        */
        switch(paMngDbPtr->devFamily)
        {
         case CPSS_PP_FAMILY_DXCH_HARRIER_E:
         case CPSS_PP_FAMILY_DXCH_AC5X_E:
         case CPSS_PP_FAMILY_DXCH_AC5P_E:
         case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(interfaceName,"hbuing2tti_desc");
            break;
         case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
         case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
            break;

        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        st = prvCpssDxChIdebugPortGroupInterfaceReset(devNum,portGroupsBmp,interfaceName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.1.2 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [ipvx2ioam_desc/ermrk2td_desc/iplr02iplr1_desc]
            Expected: GT_OK
        */
        switch (paMngDbPtr->devFamily) {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(interfaceName, "ipvx2ioam_desc");
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(interfaceName,"iplr02iplr1_desc");
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        st = prvCpssDxChIdebugPortGroupInterfaceReset(devNum,portGroupsBmp,interfaceName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.1.3 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName  [mll2eq_desc/ioam2iplr0_desc/RXDMA_2_CTRL_PIPE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceReset(devNum,portGroupsBmp,interfaceName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        st = prvCpssDxChIdebugPortGroupInterfaceReset(devNum,portGroupsBmp,interfaceName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceReset(devNum,portGroupsBmp,interfaceName);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceResetAll
(
    IN  GT_U8                                   devNum,
    IN  GT_PORT_GROUPS_BMP                      portGroupsBmp
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugPortGroupInterfaceResetAll)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
    Expected: GT_OK
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_PORT_GROUPS_BMP                                   portGroupsBmp;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /*reset all interfaces in falcon takes to long - cause time out  */
    FALCON_SKIPPED_TEST

    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        /*
            1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
            Expected: GT_OK
        */
        st = prvCpssDxChIdebugPortGroupInterfaceResetAll(devNum,portGroupsBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = prvCpssDxChIdebugPortGroupInterfaceResetAll(devNum,portGroupsBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceActionSet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_ENT              interfaceName,
    IN  CPSS_DXCH_PACKET_ANALYZER_ACTION_STC            *actionPtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugPortGroupInterfaceActionSet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                            interfaceName  [eoam2eplr_desc\
                                            mll2eq_desc\
                                            pcl2l2i_desc]
                            action          [0]
    Expected: GT_OK
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL actionPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_PORT_GROUPS_BMP                                   portGroupsBmp;
    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                 action;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS ;
        cpssOsMemSet(&action, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /*
            1.1.1 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [eoam2eplr_desc]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceActionSet(devNum,portGroupsBmp,interfaceName,&action);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.1.2 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName  [mll2eq_desc]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceActionSet(devNum,portGroupsBmp,interfaceName,&action);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.1.3 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName  [pcl2l2i_desc]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceActionSet(devNum,portGroupsBmp,interfaceName,&action);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        st = prvCpssDxChIdebugPortGroupInterfaceActionSet(devNum,portGroupsBmp,interfaceName,&action);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.3. Call with NULL actionPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceActionSet(devNum,portGroupsBmp,interfaceName,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceActionSet(devNum,portGroupsBmp,interfaceName,&action);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceActionGet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_ENT              interfaceName,
    OUT CPSS_DXCH_PACKET_ANALYZER_ACTION_STC            *actionPtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugPortGroupInterfaceActionGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                            interfaceName  [eoam2eplr_desc\
                                            mll2eq_desc\
                                            pcl2l2i_desc]
    Expected: GT_OK and action as set
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL actionPtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_PORT_GROUPS_BMP                                   portGroupsBmp;
    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];

    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                 actionSet;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                 actionGet;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

        cpssOsMemSet(&actionSet, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));
        cpssOsMemSet(&actionGet, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        /*
            1.1.1 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [eoam2eplr_desc]
            Expected: GT_OK and action as set
        */
        cpssOsStrCpy(interfaceName, "eoam2eplr_desc");

        st = prvCpssDxChIdebugPortGroupInterfaceActionSet(devNum, portGroupsBmp, interfaceName, &actionSet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        st = prvCpssDxChIdebugPortGroupInterfaceActionGet(devNum, portGroupsBmp, interfaceName, &actionGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actionSet.interruptMatchCounterEnable, actionGet.interruptMatchCounterEnable,
                                     "got other actionGet.interruptMatchCounterEnable"
                                     " than expected: %d",  actionGet.interruptMatchCounterEnable);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actionSet.samplingMode, actionGet.samplingMode,
                                     "got other actionGet.samplingMode"
                                     " than expected: %d", actionGet.samplingMode);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(actionSet.inverseEnable, actionGet.inverseEnable,
                                     "got other actionGet.inverseEnable"
                                     " than expected: %d", actionGet.inverseEnable);

        /*
            1.1.2 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [mll2eq_desc]
            Expected: GT_OK and action as set
        */
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceActionSet(devNum,portGroupsBmp,interfaceName,&actionSet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        st = prvCpssDxChIdebugPortGroupInterfaceActionGet(devNum,portGroupsBmp,interfaceName,&actionGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( actionSet.interruptMatchCounterEnable , actionGet.interruptMatchCounterEnable,
                                     "got other  actionGet.interruptMatchCounterEnable"
                                     " than expected: %d",  actionGet.interruptMatchCounterEnable);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( actionSet.samplingMode , actionGet.samplingMode,
                                     "got other actionGet.samplingMode"
                                     " than expected: %d", actionGet.samplingMode);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( actionSet.inverseEnable , actionGet.inverseEnable,
                                     "got other actionGet.inverseEnable"
                                     " than expected: %d", actionGet.inverseEnable);

        /*
            1.1.3 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName  [pcl2l2i_desc]
            Expected: GT_OK and action as set
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceActionSet(devNum,portGroupsBmp,interfaceName,&actionSet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        st = prvCpssDxChIdebugPortGroupInterfaceActionGet(devNum,portGroupsBmp,interfaceName,&actionGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( actionSet.interruptMatchCounterEnable , actionGet.interruptMatchCounterEnable,
                                     "got other  actionGet.interruptMatchCounterEnable"
                                     " than expected: %d",  actionGet.interruptMatchCounterEnable);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( actionSet.samplingMode , actionGet.samplingMode,
                                     "got other actionGet.samplingMode"
                                     " than expected: %d", actionGet.samplingMode);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( actionSet.inverseEnable , actionGet.inverseEnable,
                                     "got other actionGet.inverseEnable"
                                     " than expected: %d", actionGet.inverseEnable);

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        st = prvCpssDxChIdebugPortGroupInterfaceActionGet(devNum,portGroupsBmp,interfaceName,&actionGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.3. Call with NULL actionPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceActionGet(devNum,portGroupsBmp,interfaceName,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceActionGet(devNum,portGroupsBmp,interfaceName,&actionGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet
(
    IN  GT_U8                                           devNum,
    IN  GT_PORT_GROUPS_BMP                              portGroupsBmp,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_ENT              interfaceName,
    IN  GT_BOOL                                         clearOnReadEnable,
    OUT GT_U32                                          *valuePtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    this UT doesnt check values
    1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                            interfaceName  [eoam2eplr_desc\
                                            pcl2l2i_desc\
                                            l2i2ipvx_desc]
                            clearOnReadEnable [GT_TRUE \GT_FALSE]
    Expected: GT_OK
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL valuePtr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_BOOL                                              clearOnReadEnable;
    GT_U32                                               valueGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /*
            1.1.1 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [eoam2eplr_desc]
                                     clearOnReadEnable [GT_TRUE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        clearOnReadEnable = GT_TRUE ;
        st = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,META_DATA_ALL_CNS,interfaceName,clearOnReadEnable,&valueGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( 0 , valueGet,
                                     "got other  valueGet than expected: %d",  valueGet);

        /*
            1.1.2 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [pcl2l2i_desc]
                                     clearOnReadEnable [GT_FALSE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        clearOnReadEnable = GT_FALSE ;
        st = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,META_DATA_ALL_CNS,interfaceName,clearOnReadEnable,&valueGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);


        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( 0 , valueGet,
                                     "got other  valueGet than expected: %d",  valueGet);

        /*
            1.1.3 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName  [l2i2ipvx_desc]
                                     clearOnReadEnable = GT_TRUE ;
            and call one more time to check counters 0
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"l2i2ipvx_desc");
        clearOnReadEnable = GT_TRUE ;
        st = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,META_DATA_ALL_CNS,interfaceName,clearOnReadEnable,&valueGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( 0 , valueGet,
                                     "got other  valueGet than expected: %d",  valueGet);

        clearOnReadEnable = GT_FALSE;
        st = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,META_DATA_ALL_CNS,interfaceName,clearOnReadEnable,&valueGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( 0 , valueGet,
                                     "got other  valueGet than expected: %d",  valueGet);

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        st = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,META_DATA_ALL_CNS,interfaceName,clearOnReadEnable,&valueGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.3. Call with NULL valuePtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(interfaceName,"l2i2ipvx_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,META_DATA_ALL_CNS,interfaceName,clearOnReadEnable,NULL,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"eq2eft_desc");
        clearOnReadEnable = GT_FALSE ;
        st = prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet(devNum,META_DATA_ALL_CNS,interfaceName,clearOnReadEnable,&valueGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet
(
    IN    GT_U8                                         devNum,
    IN    GT_PORT_GROUPS_BMP                            portGroupsBmp,
    IN    GT_CHAR_PTR                                   interfaceName,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC      fieldsValueArr[]
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                            interfaceName  [eoam2eplr_desc\
                                            mll2eq_desc\
                                            eq2eft_desc]
                            numOfFields     [SIZE_OF_TEST_ARR*10]
    Expected: GT_OK
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call with numOfFieldsPtr less than needed .
    Expected: GT_BAD_SIZE.
    1.4. Call with NULL numOfFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL fieldsValueArr and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_PORT_GROUPS_BMP                                   portGroupsBmp;
    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                               numOfFields;
    static    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC   fieldsValueArr[SIZE_OF_TEST_ARR*10];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS ;

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*
            1.1.1 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [eoam2eplr_desc]
                                     clearOnReadEnable [GT_TRUE]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        numOfFields = SIZE_OF_TEST_ARR*10 ;
        st = prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet(devNum,portGroupsBmp,interfaceName,
                                                                  &numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 177 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 172 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 92 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 91 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.2 Call function with portGroupsBmp     [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName     [mll2eq_desc]
                                     clearOnReadEnable [GT_FALSE]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        numOfFields = SIZE_OF_TEST_ARR*10 ;
        st = prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet(devNum,portGroupsBmp,interfaceName,
                                                                  &numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 136 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 134 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 132 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 130 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.1.3 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName  [pcl2l2i_desc]
                                     clearOnReadEnable [GT_TRUE]
            Expected: GT_OK and same values as set.
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        numOfFields = SIZE_OF_TEST_ARR*10 ;
        st = prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet(devNum,portGroupsBmp,interfaceName,
                                                                  &numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 183 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 181 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 180 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            UTF_VERIFY_EQUAL1_STRING_MAC( 176 , numOfFields,
                             "got other numOfFields than expected: %d", numOfFields);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        numOfFields = SIZE_OF_TEST_ARR*10 ;
        st = prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet(devNum,portGroupsBmp,interfaceName,
                                                                  &numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.3. Call with numOfFieldsPtr less than needed .
            Expected: GT_BAD_SIZE.
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        numOfFields = 1 ;
        st = prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet(devNum,portGroupsBmp,interfaceName,
                                                                  &numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum, interfaceName);

        /*
            1.4. Call with NULL numOfFieldsPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfFields = SIZE_OF_TEST_ARR*10 ;
        st = prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet(devNum,portGroupsBmp,interfaceName,
                                                                NULL,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*
            1.5. Call with NULL fieldsValueArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfFields = SIZE_OF_TEST_ARR*10 ;
        st = prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet(devNum,portGroupsBmp,interfaceName,
                                                                  &numOfFields,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        numOfFields = SIZE_OF_TEST_ARR*10 ;
        st = prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet(devNum,portGroupsBmp,interfaceName,
                                                                 &numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet
(
    IN    GT_U8                                         devNum,
    IN    GT_PORT_GROUPS_BMP                            portGroupsBmp,
    IN    PRV_CPSS_DXCH_IDEBUG_INTERFACE_ENT            interfaceName,
    IN    GT_BOOL                                       clearOnReadEnable,
    IN    GT_U32                                        numOfFields,
    INOUT PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC      fieldsValueArr[]
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    this UT doesnt check values
    1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                            interfaceName  [eoam2eplr_desc\
                                            mll2eq_desc\
                                            pcl2l2i_desc]
                            clearOnReadEnable [GT_TRUE \GT_FALSE]
                            numOfFields     [2\3\1\2\2\4];
                            fieldsValueArr  [byte_count,evlan\
                                             local_dev_src_port,cut_through_id,valid_bytes\
                                             dec_ttl\
                                             egress_byte_count,udp_checksum_offset\
                                             byte_count,ipv4_dip\
                                             egress_filter_registered,byte_count,evlan,queue_buff_cntr]
    Expected: GT_OK
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call function with wrong fieldsValue [PRV_CPSS_DXCH_IDEBUG_FIELD_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.4. Call with NULL fieldsValueArr and other valid params.
    Expected: GT_BAD_PTR.

    NOTE : this UT can't check value ,valid values can be check only after traffic
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_PORT_GROUPS_BMP                                   portGroupsBmp;
    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_U32                                               numOfFields;
    PRV_CPSS_DXCH_IDEBUG_INTERFACE_FIELD_STC             fieldsValueArr[SIZE_OF_TEST_ARR];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /*
            1.1.1 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                    interfaceName  [eoam2eplr_desc]
                                    clearOnReadEnable [GT_TRUE]
                                    numOfFields     [2];
                                    fieldsValueArr  [byte_count,evlan]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        numOfFields = 2;
        cpssOsStrCpy(fieldsValueArr[0].fieldName,"byte_count");
        cpssOsStrCpy(fieldsValueArr[1].fieldName,"evlan");
        st = prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet(devNum, portGroupsBmp, interfaceName,
                                                                 numOfFields, fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.1.2 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                    interfaceName  [mll2eq_desc]
                                    clearOnReadEnable [GT_TRUE]
                                    numOfFields     [3];
                                    fieldsValueArr  [local_dev_src_port,first_buffer,marvell_tagged]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        numOfFields = 3;
        cpssOsStrCpy(fieldsValueArr[0].fieldName,"local_dev_src_port");
        cpssOsStrCpy(fieldsValueArr[1].fieldName,"first_buffer");
        cpssOsStrCpy(fieldsValueArr[2].fieldName,"marvell_tagged");
        st = prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet(devNum, portGroupsBmp, interfaceName,
                                                                  numOfFields, fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.1.3 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                    interfaceName  [pcl2l2i_desc]
                                    clearOnReadEnable [GT_FALSE]
                                    numOfFields     [1];
                                    fieldsValueArr  [recalc_crc]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        numOfFields = 1;
        cpssOsStrCpy(fieldsValueArr[0].fieldName,"recalc_crc");
        st = prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet(devNum, portGroupsBmp, interfaceName,
                                                                  numOfFields, fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        st = prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet(devNum, portGroupsBmp, interfaceName,
                                                                  numOfFields, fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.3. Call function with wrong fieldsValue [PRV_CPSS_DXCH_IDEBUG_FIELD_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        numOfFields = 1 ;
        cpssOsStrCpy(fieldsValueArr[0].fieldName,"");
        st = prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet(devNum, portGroupsBmp, interfaceName,
                                                                  numOfFields, fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.4. Call with NULL fieldsValueArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        cpssOsStrCpy(fieldsValueArr[0].fieldName,"marvell_tagged");
        st = prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet(devNum, portGroupsBmp, interfaceName,
                                                                  numOfFields, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);


        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"mll2eq_desc");
        numOfFields = 1;
        cpssOsStrCpy(fieldsValueArr[0].fieldName,"marvell_tagged");
        st = prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet(devNum, portGroupsBmp, interfaceName,
                                                                  numOfFields, fieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet
(
    IN GT_U8                                            devNum,
    IN GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN PRV_CPSS_DXCH_IDEBUG_INTERFACE_ENT               interfaceName,
    IN GT_BOOL                                          enable
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    this UT doesnt check values
    1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                            interfaceName  [eoam2eplr_desc\
                                            pcl2l2i_desc\
                                            l2i2ipvx_desc\
                                            pcl2l2i_desc]
                            enable         [GT_TRUE\GT_FALSE]
    Expected: GT_OK
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_PORT_GROUPS_BMP                                   portGroupsBmp;
    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_BOOL                                              enable;
    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS ;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {


        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        /*
            1.1.1 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [eoam2eplr_desc]
                                     enable          [GT_TRUE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        enable = GT_TRUE ;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.1.2 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [pcl2l2i_desc]
                                     enable          [GT_TRUE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        enable = GT_TRUE ;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.1.3 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName  [l2i2ipvx_desc]
                                     enable         [GT_FALSE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"l2i2ipvx_desc");
        enable = GT_FALSE ;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.1.4 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName  [pcl2l2i_desc]
                                     enable         [GT_FALSE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        enable = GT_FALSE ;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        enable = GT_TRUE ;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        enable = GT_TRUE ;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet
(
    IN  GT_U8                                            devNum,
    IN  GT_PORT_GROUPS_BMP                               portGroupsBmp,
    IN  PRV_CPSS_DXCH_IDEBUG_INTERFACE_ENT               interfaceName,
    OUT GT_BOOL                                          *enablePtr
);
*/
UTF_TEST_CASE_MAC(prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                            interfaceName  [eoam2eplr_desc\
                                            pcl2l2i_desc\
                                            l2i2ipvx_desc\
                                            pcl2l2i_desc]

    Expected: GT_OK and value as set
    1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call with NULL interfaceName and other valid params.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_PORT_GROUPS_BMP                                   portGroupsBmp;
    GT_CHAR                                              interfaceName[CPSS_DXCH_PACKET_ANALYZER_MAX_STR_LEN_CNS];
    GT_BOOL                                              enable;
    GT_BOOL                                              enableGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS ;

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        /*Manager enable*/
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /*
            1.1.1 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [eoam2eplr_desc]
                                     enable          [GT_TRUE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"eoam2eplr_desc");
        enable = GT_TRUE;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet(devNum,portGroupsBmp,interfaceName,&enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( enable , enableGet,
                                     "got other enableGet than expected: %d", enableGet);
        /*
            1.1.2 Call function with portGroupsBmp   [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName   [pcl2l2i_desc]
                                     enable          [GT_TRUE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        enable = GT_TRUE;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet(devNum,portGroupsBmp,interfaceName,&enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( enable , enableGet,
                                     "got other enableGet than expected: %d", enableGet);

        /*
            1.1.3 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName  [l2i2ipvx_desc]
                                     enable         [GT_FALSE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"l2i2ipvx_desc");
        enable = GT_FALSE;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet(devNum,portGroupsBmp,interfaceName,&enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( enable , enableGet,
                                     "got other enableGet than expected: %d", enableGet);

        /*
            1.1.4 Call function with portGroupsBmp  [CPSS_PORT_GROUP_UNAWARE_MODE_CNS]
                                     interfaceName  [pcl2l2i_desc]
                                     enable         [GT_FALSE]
            Expected: GT_OK
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        enable = GT_FALSE;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet(devNum,portGroupsBmp,interfaceName,&enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfaceName);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC( enable , enableGet,
                                     "got other enableGet than expected: %d", enableGet);

        /*
            1.2. Call function with wrong interfaceName [PRV_CPSS_DXCH_IDEBUG_INTERFACE_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        cpssOsStrCpy(interfaceName,"");
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet(devNum,portGroupsBmp,interfaceName,&enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, interfaceName);

        /*
            1.3. Call with NULL interfaceName and other valid params.
            Expected: GT_BAD_PTR
        */
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet(devNum,portGroupsBmp,interfaceName,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, interfaceName);

        /*restore*/
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    /* Go over all non active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        cpssOsStrCpy(interfaceName,"pcl2l2i_desc");
        enable = GT_TRUE ;
        st = prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet(devNum,portGroupsBmp,interfaceName,enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum, interfaceName);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
                                 /*DB CHECK*/
/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(PacketAnalyzerDbTest1)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1 fill DB with manager [4]
                     device  [0]
                     key     [2,7]
                     group   [4]
                     action  [6]
                     rule    [7,5]

        check that all values are set (values that cant be check by apis)
*/

    GT_STATUS                                            st = GT_OK;
    GT_U32                                               ii;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_U32                                               groupId;
    GT_U32                                               actionId;

    GT_U32                                               keyId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC         keyAttr;
    GT_U32                                               keyNumOfStages;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          keyStagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT  keyFieldMode;
    GT_U32                                               keyNumOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  keyFieldsArr[SIZE_OF_TEST_ARR];

    GT_U32                                               keyTestNumOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  keyTestFieldsArr[SIZE_OF_TEST_ARR];

    GT_U32                                               ruleId;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC        ruleAttr;
    GT_U32                                               ruleNumOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC            ruleFieldsValueArr[SIZE_OF_TEST_ARR];
    GT_U32                                               expectedValuesArr[3];

    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR                paGrpDbPtr=NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_ACT_PTR                paActDbPtr=NULL;


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*
            1.1 fill DB with manager [4]
                         device  [0]
                         key     [2,7]
                         group   [4]
                         action  [6]
                         rule    [7,5]

           check that all values are set (values that cant be check by apis)
        */

        /* initialized managerId with device*/
        managerId = 4 ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        /* initialized Keys */
        keyId = 786 ;
        cpssOsMemCpy(keyAttr.keyNameArr,"key 2" ,sizeof("key 2"));
        keyNumOfStages = 3 ;
        keyStagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E ;
        keyStagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        keyStagesArr[2] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E ;
        keyFieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
        keyNumOfFields = 3 ;
        keyFieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        keyFieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        keyFieldsArr[2] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,keyNumOfStages,
                                                    keyStagesArr,keyFieldMode,keyNumOfFields,keyFieldsArr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, managerId, devNum,keyId);

        expectedValuesArr[0]=3;
        expectedValuesArr[1]=3;
        expectedValuesArr[2]=1;

        for (ii=0 ; ii < keyNumOfStages ; ii++)
        {
            keyTestNumOfFields = SIZE_OF_TEST_ARR;
            st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,keyStagesArr[ii],&keyTestNumOfFields,keyTestFieldsArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);
            /* verifying values */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedValuesArr[ii],keyTestNumOfFields,
                                         "got other keyTestNumOfFields than was set");
        }

        keyId = 257;
        cpssOsMemCpy(keyAttr.keyNameArr, "key 7", sizeof("key 7"));
        keyNumOfStages = 2;
        keyStagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
        keyStagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
        keyFieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E;
        keyNumOfFields = 2;
        keyFieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        keyFieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId, keyId, &keyAttr, keyNumOfStages,
                                                    keyStagesArr, keyFieldMode, keyNumOfFields, keyFieldsArr);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, managerId, devNum, keyId);

        expectedValuesArr[0]=2;
        expectedValuesArr[1]=2;
        for (ii=0 ; ii < keyNumOfStages ; ii++)
        {
            keyTestNumOfFields = SIZE_OF_TEST_ARR;
            st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId,keyId,keyStagesArr[ii],&keyTestNumOfFields,keyTestFieldsArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);
            /* verifying values */
            UTF_VERIFY_EQUAL0_STRING_MAC(expectedValuesArr[ii],keyTestNumOfFields,
                                         "got other keyTestNumOfFields than was set");
        }

        /* initialized group and action*/
        groupId = 4;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");

        actionId = 6;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericActionInit ");


        /* initialized rules*/
        ruleId = 7;
        groupId = 4;
        keyId = 786;
        cpssOsMemCpy(ruleAttr.ruleNameArr, "rule 7", sizeof("rule 7"));
        ruleNumOfFields = 3;
        ruleFieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        cpssOsMemSet(ruleFieldsValueArr[0].data, 0, sizeof(ruleFieldsValueArr[0].data));
        cpssOsMemSet(ruleFieldsValueArr[0].msk, 0, sizeof(ruleFieldsValueArr[0].msk));
        ruleFieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        cpssOsMemSet(ruleFieldsValueArr[1].data, 0, sizeof(ruleFieldsValueArr[0].data));
        cpssOsMemSet(ruleFieldsValueArr[1].msk, 0, sizeof(ruleFieldsValueArr[0].msk));
        ruleFieldsValueArr[2].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        cpssOsMemSet(ruleFieldsValueArr[2].data, 0, sizeof(ruleFieldsValueArr[0].data));
        cpssOsMemSet(ruleFieldsValueArr[2].msk, 0, sizeof(ruleFieldsValueArr[0].msk));

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId, keyId, groupId, ruleId, &ruleAttr, ruleNumOfFields, ruleFieldsValueArr, actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, keyId, groupId, ruleId);

        ruleId = 5;
        groupId = 4;
        keyId = 257;
        cpssOsMemCpy(ruleAttr.ruleNameArr, "rule 5", sizeof("rule 5"));
        ruleNumOfFields = 2;
        ruleFieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        cpssOsMemSet(ruleFieldsValueArr[0].data, 1, sizeof(ruleFieldsValueArr[0].data));
        cpssOsMemSet(ruleFieldsValueArr[0].msk, 1, sizeof(ruleFieldsValueArr[0].msk));
        ruleFieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E;
        cpssOsMemSet(ruleFieldsValueArr[1].data, 1, sizeof(ruleFieldsValueArr[0].data));
        cpssOsMemSet(ruleFieldsValueArr[1].msk, 1, sizeof(ruleFieldsValueArr[0].msk));

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId, keyId, groupId, ruleId, &ruleAttr, ruleNumOfFields, ruleFieldsValueArr, actionId);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, devNum, keyId, groupId, ruleId);

        keyId = 1024 ;
        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId, keyId, &keyAttr, keyNumOfStages,
                                            keyStagesArr, keyFieldMode, keyNumOfFields, keyFieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, managerId, devNum);


        /*verifying values of key bitmap */
        UTF_VERIFY_EQUAL0_STRING_MAC(0x2, paMngDbPtr->keysBitmap[8],
                                     "keysBitmap is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(0x40000, paMngDbPtr->keysBitmap[24],
                                     "keysBitmap is wrong");

        UTF_VERIFY_EQUAL0_STRING_MAC(0x1, paMngDbPtr->keysBitmap[32],
                                     "keysBitmap is wrong");

        /* verifying values of group and rule inside  */
        paGrpDbPtr = paMngDbPtr->paGrpPtrArr[groupId];
        if (paGrpDbPtr) {
            UTF_VERIFY_EQUAL0_STRING_MAC(4, paGrpDbPtr->paGrpId,
                                         "got other paGrpDbPtr->paGrpId than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(2, paGrpDbPtr->numOfRules,
                                         "got other paGrpDbPtr->numOfRules than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, paGrpDbPtr->activateEnable,
                                         "got other paGrpDbPtr->activateEnable than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(5, paGrpDbPtr->numOfStages,
                                         "got other paGrpDbPtr->numOfStages than was set");
            if(paGrpDbPtr->numOfRules!= 0)
            {
                UTF_VERIFY_EQUAL0_STRING_MAC(7, paGrpDbPtr->paRulePtrArr[1]->paRuleId,
                                             "got other paRulePtrArr[1]->paRuleId than was set");
                UTF_VERIFY_EQUAL0_STRING_MAC(786, paGrpDbPtr->paRulePtrArr[1]->paKeyId,
                                             "got other paRulePtrArr[1]->paKeyId than was set");
                UTF_VERIFY_EQUAL0_STRING_MAC(6, paGrpDbPtr->paRulePtrArr[1]->paActId,
                                             "got other paRulePtrArr[1]->paActId than was set");


                UTF_VERIFY_EQUAL0_STRING_MAC(5, paGrpDbPtr->paRulePtrArr[2]->paRuleId,
                                             "got other paRulePtrArr[2]->paRuleId than was set");
                UTF_VERIFY_EQUAL0_STRING_MAC(257, paGrpDbPtr->paRulePtrArr[2]->paKeyId,
                                             "got other paRulePtrArr[2]->paKeyId than was set");
                UTF_VERIFY_EQUAL0_STRING_MAC(6, paGrpDbPtr->paRulePtrArr[2]->paActId,
                                             "got other paRulePtrArr[2]->paActId than was set");
            }
        }

        /* verifying values of action  */
        st = prvCpssDxChPacketAnalyzerDbActionGet(managerId, actionId, &paActDbPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        if (paActDbPtr)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(6, paActDbPtr->paActId,
                                         "got other paActDbPtr->paActId than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, paActDbPtr->action.samplingEnable,
                                         "got other paActDbPtr->action.samplingEnable than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_FIRST_MATCH_E,
                                         paActDbPtr->action.samplingMode, "got other paActDbPtr->action.samplingMode than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, paActDbPtr->action.matchCounterEnable,
                                         "got other paActDbPtr->action.matchCounterEnable than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(0x4, paActDbPtr->action.matchCounterIntThresh.lsb,
                                         "got other paActDbPtr->action.matchCounterIntThresh.lsb than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(0x1, paActDbPtr->action.matchCounterIntThresh.middle,
                                         "got other paActDbPtr->action.matchCounterIntThresh.middle than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(0xa, paActDbPtr->action.matchCounterIntThresh.msb,
                                         "got other paActDbPtr->action.matchCounterIntThresh.msb than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, paActDbPtr->action.interruptSamplingEnable,
                                         "got other paActDbPtr->action.interruptSamplingEnable than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, paActDbPtr->action.interruptMatchCounterEnable,
                                         "got other paActDbPtr->action.interruptMatchCounterEnable than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, paActDbPtr->action.inverseEnable,
                                         "got other paActDbPtr->action.inverseEnable than was set");
            UTF_VERIFY_EQUAL0_STRING_MAC(1, paActDbPtr->numOfGrps, "got other paActDbPtr->numOfGrps than was set");
        }

        /* Restore system. */

        groupId = 4 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 6 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        managerId = 4;
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }
#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

UTF_TEST_CASE_MAC(PacketAnalyzerDbTest2)
{
    /*
     1. Set Packet Analyzer basic configuration:
            1.1 create manager
            1.2 create action #1  - action.samplingMode = LAST_MATCH_E
            1.3 add device to manager
            1.4 enable PA on all devices added to manager
            1.5 create egress logical key #1
            1.6 create egress logical key #2
            1.7 create group #1
            1.8 create rule #1 in group #1 bounded to ALL_INGRESS logical key #1 with no fields
            1.9 create rule #2 in group #1 bounded to ALL_EGRESS  logical key #2 with no fields

            1.10 add to key #1  field      - EVLAN_E ,field mode - ASSIGN_ALL
            1.11 check rule #1 data field empty
            1.12 delete rule #1 from Group #1
            1.13 create rule #1 in group #1 bounded to logical key #1 with
                                                 field EVLAN_E ,data - 5,mask - 0xFFFF

            1.14 add to key #1  field  - BYTE_COUNT_E , field mode - ASSIGN_ALL
            1.15 check rule #1 data field - EVLAN_E ,data - 5, mask - 0xFFFF
            1.16 delete rule #1 from Group #1
            1.17 create rule #1 in group #1 bounded to logical key #1
                            with EVLAN_E      field data - 5  mask - 0xFFFF
                            BYTE_COUNT_E field data - 84 mask - 0xFFFF

            1.18 repeat 1.10 - 1.17 for key #2 rule #2

            1.19 enable group activation
            1.20 enable sampling on all rules bounded to action
                */


    GT_STATUS                                               st = GT_OK;
    GT_U8                                                   devNum;
    GT_U32                                                  managerId,keyId1,groupId,actionId,ruleId1;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC            keyAttr;
    CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC          groupAttr;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC           ruleAttr;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             stagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     fieldsArr[SIZE_OF_TEST_ARR];
    GT_U32                                                  ii,jj,numOfStages,numOfFields ,expNumOfStages[]={8,8};
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                    action;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               fieldsValueArr[SIZE_OF_TEST_ARR];

    GT_U32                                                  keyIdGet,actionIdGet;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC           ruleAttrGet;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                   paMngDbPtr=NULL;

     /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*set default values */
        managerId = 1;
        keyId1 = 1;
        groupId =1 ;
        actionId =1;
        ruleId1 = 1;

        cpssOsMemSet(stagesArr,0,sizeof(stagesArr));
        cpssOsMemSet(fieldsArr,0,sizeof(fieldsArr));
        cpssOsMemSet(fieldsValueArr,0,sizeof(fieldsValueArr));

        /* initialized managerId with device*/
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /* AUTODOC: enable PA on all devices added to manager */
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerManagerEnableSet ");

        /* AUTODOC: create action #1 */
        cpssOsMemSet(&action,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));
        action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId,actionId, &action);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerActionCreate ");

        /* AUTODOC: create egress logical key #1 with no  fields */
        cpssOsMemSet(&keyAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
        cpssOsMemCpy(keyAttr.keyNameArr,"ingress key id 1 ",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId, keyId1,&keyAttr, 1, stagesArr,
                                                    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E,
                                                    0, fieldsArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyCreate keyId %d " , keyId1);

        /* AUTODOC: create egress logical key #2 with no stages and fields */
        cpssOsMemSet(&keyAttr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
        cpssOsMemCpy(keyAttr.keyNameArr, "egress key id 2 ", sizeof(GT_CHAR) * CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId, keyId1+1, &keyAttr, 1, stagesArr,
                                                    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E,
                                                    0, fieldsArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyCreate keyId %d " , keyId1);

        if ((paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) || (paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) || (paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) )
            expNumOfStages[1] = 10;

        for (ii=0;ii<2;ii++)
        {
            /* AUTODOC: check number of stages created in logical key - expect 0 */
            numOfStages = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
            st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId, keyId1+ii, &numOfStages, stagesArr);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyStagesGet keyId %d " , keyId1+ii);

            UTF_VERIFY_EQUAL1_STRING_MAC(expNumOfStages[ii], numOfStages,"got other numOfStages than was set [%d]" , expNumOfStages[ii]);

             /* AUTODOC: read number of fields for each stage in logical key */
            for (jj=0; jj<numOfStages; jj++)
            {
                numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E; /* max number of fields */
                cpssOsMemSet(fieldsArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*SIZE_OF_TEST_ARR);
                st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId, keyId1+ii,
                                                                       stagesArr[jj], &numOfFields, fieldsArr);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyStagesGet keyId %d " , keyId1+ii);

                /* AUTODOC: expect 0 fields in stage */
                UTF_VERIFY_EQUAL1_STRING_MAC(0, numOfFields,"got other numOfStages than was set [%d]" , numOfFields);
            }
        }

        /* AUTODOC: create group */
        cpssOsMemSet(&groupAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC));
        cpssOsMemCpy(groupAttr.groupNameArr,"group id 1",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
        st = cpssDxChPacketAnalyzerGroupCreate(managerId, groupId, &groupAttr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyStagesGet keyId %d " , keyId1+ii);

        /*
           AUTODOC: create rule1 in group1 bounded logical key1
                    create rule2 in group1 bounded logical key2
        */
        for (ii=0;ii<2;ii++)
        {
            /* AUTODOC: create rule1 in group1 bounded logical key1 */
            cpssOsMemSet(&ruleAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
            cpssOsMemCpy(ruleAttr.ruleNameArr,"rule id 1 ",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
            st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId1+ii, groupId, ruleId1+ii, &ruleAttr,
                                                0, fieldsValueArr, actionId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleAdd ruleId %d " , ruleId1+ii);
        }

        for (ii = 0; ii < 2; ii++)
        {
            /* AUTODOC: add field evlan to key check that rule is empty , delete and create new rule with field evlan  */
            fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E;
            st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId, keyId1 + ii, 1, fieldsArr);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyFieldsAdd keyId %d " , keyId1+ii);

            numOfFields = 10;
            st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId1 + ii, groupId, &keyIdGet,
                                                    &ruleAttrGet, &numOfFields, fieldsValueArr, &actionIdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleGet ruleId %d " , ruleId1+ii);

            /* AUTODOC: expect 0 fields */
            UTF_VERIFY_EQUAL1_STRING_MAC(0, numOfFields,"got other numOfFields than was set [%d]" , numOfFields);

            st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,ruleId1 + ii, groupId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId1+ii);

            /* AUTODOC: create rule in group1 bounded logical key */
            cpssOsMemSet(&ruleAttr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
            cpssOsMemCpy(ruleAttr.ruleNameArr, "rule with 1 field ", sizeof(GT_CHAR) * CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
            fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E;
            fieldsValueArr[0].data[0] = 5;
            fieldsValueArr[0].msk[0] = 0xFFFF;

            st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId1 + ii, groupId, ruleId1 + ii, &ruleAttr,
                                                    1, fieldsValueArr, actionId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleAdd ruleId %d " , ruleId1+ii);

            /* AUTODOC: add field byte count to key check that rule have 1 field , delete and create new rule with 2 field evlan and byte count  */
            fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
            st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId, keyId1 + ii, 1, fieldsArr);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyFieldsAdd keyId %d " , keyId1+ii);

            numOfFields = 10;
            st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId1 + ii, groupId, &keyIdGet,
                                                    &ruleAttrGet, &numOfFields, fieldsValueArr, &actionIdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleGet ruleId %d " , ruleId1+ii);

            /* AUTODOC: expect 1 field name EVLAN with vlan 5 and mask FFFF  */
            UTF_VERIFY_EQUAL0_STRING_MAC(1, numOfFields,
                                         "got other numOfFields than expected [%d]" );
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E, fieldsValueArr[0].fieldName,
                                         "got other fieldsValueArr[0].fieldName than expected" );
            UTF_VERIFY_EQUAL0_STRING_MAC(5, fieldsValueArr[0].data[0],
                                         "got other fieldsValueArr[0].data[0] than expected" );
            UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFF, fieldsValueArr[0].msk[0],
                                         "got other fieldsValueArr[0].msk[0] than expected" );

            st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,ruleId1 + ii, groupId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId1+ii);

            /* AUTODOC: create rule in group1 bounded logical key */
            cpssOsMemSet(&ruleAttr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
            cpssOsMemCpy(ruleAttr.ruleNameArr, "rule with 2 fields ", sizeof(GT_CHAR) * CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
            fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E;
            fieldsValueArr[0].data[0] = 5;
            fieldsValueArr[0].msk[0] = 0xffff;
            fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
            fieldsValueArr[1].data[0] = 84;
            fieldsValueArr[1].msk[0] = 0xffff;
            st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId1 + ii, groupId, ruleId1 + ii, &ruleAttr,
                                                    2, fieldsValueArr, actionId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleAdd ruleId %d " , ruleId1+ii);

            numOfFields = 10;
            cpssOsMemSet(&ruleAttrGet, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
            st = cpssDxChPacketAnalyzerGroupRuleGet(managerId,ruleId1 + ii, groupId, &keyIdGet,
                                                    &ruleAttrGet, &numOfFields, fieldsValueArr, &actionIdGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleGet ruleId %d " , ruleId1+ii);

            /* AUTODOC: expect 2 field name EVLAN  with vlan 5  mask FFFF
               and BYTE_COUNT with 84 byte mask FFFF*/
            UTF_VERIFY_EQUAL0_STRING_MAC(2, numOfFields,
                                         "got other numOfFields than expected [%d]" );
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_PACKET_ANALYZER_FIELD_EVLAN_E, fieldsValueArr[0].fieldName,
                                         "got other fieldsValueArr[0].fieldName than expected" );
            UTF_VERIFY_EQUAL0_STRING_MAC(5, fieldsValueArr[0].data[0],
                                         "got other fieldsValueArr[0].data[0] than expected" );
            UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFF, fieldsValueArr[0].msk[0],
                                         "got other fieldsValueArr[0].msk[0] than expected" );
            UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E, fieldsValueArr[1].fieldName,
                                         "got other fieldsValueArr[0].fieldName than expected" );
            UTF_VERIFY_EQUAL0_STRING_MAC(84, fieldsValueArr[1].data[0],
                                         "got other fieldsValueArr[0].data[0] than expected" );
            UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFF, fieldsValueArr[1].msk[0],
                                         "got other fieldsValueArr[0].msk[0] than expected " );
        }

        /* AUTODOC: enable group activation */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId, groupId, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupActivateEnableSet ruleId %d " , groupId);

        /* AUTODOC: enable sampling on all rules bounded to action */
        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId, actionId, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerActionSamplingEnableSet ruleId %d " , actionId);

        /* AUTODOC: enable sampling on all rules bounded to action */
        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId, actionId, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerActionSamplingEnableSet ruleId %d " , actionId);

        /* AUTODOC: enable group activation */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId, groupId, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupActivateEnableSet ruleId %d " , groupId);

        /* Restore system. */
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

}


UTF_TEST_CASE_MAC(PacketAnalyzerDbTest3)
{
    /*
     1. Set Packet Analyzer basic configuration:
            1.1 create manager
            1.2 create action #1  - action.samplingMode = LAST_MATCH_E
            1.3 add device to manager
            1.4 enable PA on all devices added to manager
            1.5 create group #1

     2. test key add and remove fields
            2.1 create ingress logical key #1 with fields  CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
                                                           CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
                                                           CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
                                                           CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
                                                           CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E;



            2.2 add and remove fields from key step by step
            2.3 check stages and field of key after change
            2.4 create rule #1 in group #1 bounded to ALL_INGRESS logical key #1 with fields CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
                                                                                             CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
                                                                                             CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
            2.5 enable group activation
            2.6 enable sampling on all rules bounded to action
            2.7 disable sampling on all rules bounded to action
            2.8 disable enable group activation

     3. test groupe add and remove rules
            3.1 create 8 rules and keys for group 1
            3.2 check stagesArr and ruleArr for right configuration
            3.3 add and remove rules from group step by step
            3.4 check stagesArr and ruleArr after the change
            3.5 enable group activation
            3.6 enable sampling on all rules bounded to action
            3.7 disable sampling on all rules bounded to action
            3.8 disable enable group activation
    */

#ifndef GM_USED

    GT_STATUS                                               st = GT_OK;
    GT_U8                                                   devNum;
    GT_U32                                                  managerId,keyId,groupId,actionId,ruleId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC            keyAttr;
    CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC          groupAttr;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC           ruleAttr;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             stagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     fieldsArr[SIZE_OF_TEST_ARR];
    GT_U32                                                  ii,jj,kk,numOfStages,numOfStagesExp=8,numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_ACTION_STC                    action;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               fieldsValueArr[SIZE_OF_TEST_ARR];

    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                   paMngDbPtr;
    PRV_CPSS_DXCH_PACKET_ANALYZER_GRP_PTR                   paGrpDbPtr;

    /*
        CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E  24
        CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E      12
        CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E      13
        CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E  32
        CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E     26
    */
    GT_U32          expKeyStageFieldValues1 [8][5] =    {{CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E}};

    GT_U32          expKeyStageFieldValues2 [8][3] =    {{CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E},
                                                         {CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E}};
    GT_U32          expRuleIdStageValues3 [3] =         {CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_EPORT_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_TRG_PHY_PORT_E,CPSS_DXCH_PACKET_ANALYZER_FIELD_ORIG_SRC_DEV_E};

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);
    UT_PA_CHECK_XML

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1. Set Packet Analyzer basic configuration: */

        /*set default values */
        managerId = 1;
        keyId = 1;
        groupId =1 ;
        actionId =1;
        ruleId = 1;

        cpssOsMemSet(stagesArr,0,sizeof(stagesArr));
        cpssOsMemSet(fieldsArr,0,sizeof(fieldsArr));
        cpssOsMemSet(fieldsValueArr,0,sizeof(fieldsValueArr));

        /* initialized managerId with device*/
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at prvCpssDxChPacketAnalyzerDbManagerGet managerId %d " , managerId);

        /* AUTODOC: enable PA on all devices added to manager */
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerManagerEnableSet ");

        /* AUTODOC: create action #1 */
        cpssOsMemSet(&action,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_ACTION_STC));
        action.samplingMode = CPSS_DXCH_PACKET_ANALYZER_SAMPLING_MODE_LAST_MATCH_E ;
        st = cpssDxChPacketAnalyzerActionCreate(managerId, actionId, &action);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerActionCreate ");

        /* AUTODOC: create group */
        cpssOsMemSet(&groupAttr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_GROUP_ATTRIBUTES_STC));
        cpssOsMemCpy(groupAttr.groupNameArr, "group id 1", sizeof(GT_CHAR) * CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
        st = cpssDxChPacketAnalyzerGroupCreate(managerId, groupId, &groupAttr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerGroupCreate groupId %d ", groupId);

        /* 2. test key add and remove fields */

        /* AUTODOC: create egress logical key #1 with 5 fields */
        cpssOsMemSet(&keyAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
        cpssOsMemCpy(keyAttr.keyNameArr,"ingress key id 1 ",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        fieldsArr[2] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        fieldsArr[3] = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
        fieldsArr[4] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId, keyId,&keyAttr, 1, stagesArr,
                                                    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E,
                                                    5, fieldsArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyCreate keyId %d " , keyId);

        numOfStages = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
        st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId, keyId, &numOfStages, stagesArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyStagesGet keyId %d " , keyId);

        /* AUTODOC: expect 8 stages */
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfStagesExp, numOfStages,"got other numOfStages than was set [%d]" , numOfStages);

         /* AUTODOC: read number of fields for each stage in logical key */
        for (jj=0; jj<numOfStages; jj++)
        {
            numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E; /* max number of fields */
            cpssOsMemSet(fieldsArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*SIZE_OF_TEST_ARR);

            st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId, keyId,
                                                                   stagesArr[jj], &numOfFields, fieldsArr);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyStagesGet keyId %d " , keyId);

            for (kk=0;kk<numOfFields;kk++)
            {
                if (fieldsArr[kk] > CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E ||
                    expKeyStageFieldValues1[jj][kk] > CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E )
                    break;

                UTF_VERIFY_EQUAL2_STRING_MAC(expKeyStageFieldValues1[jj][kk], fieldsArr[kk],
                                             "wrong value, got [%s] while expected [%s] ",
                                             paFieldStrArr[fieldsArr[kk]],paFieldStrArr[expKeyStageFieldValues1[jj][kk]]);
            }
        }

        /*delete the first field in the key*/
        numOfFields = 1 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /*delete the last field in the key*/
        numOfFields = 1 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IS_IPV4_E;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /*add field */
        numOfFields = 1 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /*delete the middle field in the key*/
        numOfFields = 1 ;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsRemove(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        numOfStages = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
        st = cpssDxChPacketAnalyzerLogicalKeyStagesGet(managerId, keyId, &numOfStages, stagesArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyStagesGet keyId %d " , keyId);

        /* AUTODOC: expect 8/7 stages */
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfStagesExp, numOfStages,"got other numOfStages than was set [%d]" , numOfStages);

         /* AUTODOC: read number of fields for each stage in logical key */
        for (jj=0; jj<numOfStages; jj++)
        {
            numOfFields = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E; /* max number of fields */
            cpssOsMemSet(fieldsArr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT)*SIZE_OF_TEST_ARR);
            st = cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet(managerId, keyId,
                                                                   stagesArr[jj], &numOfFields, fieldsArr);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyStagesGet keyId %d " , keyId);
            for (kk=0;kk<numOfFields;kk++)
            {
                if (fieldsArr[kk] > CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E ||
                    expKeyStageFieldValues2[jj][kk] > CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E )
                    break;

                UTF_VERIFY_EQUAL2_STRING_MAC(expKeyStageFieldValues2[jj][kk], fieldsArr[kk],
                         "wrong value, got [%s] while expected [%s] ",
                         paFieldStrArr[fieldsArr[kk]],paFieldStrArr[expKeyStageFieldValues2[jj][kk]]);
            }
        }

        /* AUTODOC: create rule1 in group1 bounded logical key1 */
        cpssOsMemSet(&ruleAttr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
        cpssOsMemCpy(ruleAttr.ruleNameArr, "rule id 1 ", sizeof(GT_CHAR) * CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_SA_E;
        fieldsValueArr[2].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_PACKET_CMD_E;
        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,keyId, groupId, ruleId, &ruleAttr,
                                                3, fieldsValueArr, actionId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerGroupRuleAdd ruleId %d ", ruleId);

        /* AUTODOC: enable group activation */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId, groupId, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupActivateEnableSet ruleId %d " , groupId);

        /* AUTODOC: enable sampling on all rules bounded to action */
        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId, actionId, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerActionSamplingEnableSet ruleId %d " , actionId);

        /* AUTODOC: enable sampling on all rules bounded to action */
        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId, actionId, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerActionSamplingEnableSet ruleId %d " , actionId);

        /* AUTODOC: enable group activation */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId, groupId, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupActivateEnableSet ruleId %d " , groupId);


        /*restore */
        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,ruleId,groupId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, ruleId);

        st = cpssDxChPacketAnalyzerLogicalKeyDelete(managerId,keyId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        /* 3. test groupe add and remove rules */
        /* AUTODOC: create 8 keys and 8 rules in 1 group  */
        cpssOsMemSet(&keyAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
        cpssOsMemCpy(keyAttr.keyNameArr,"simple key  ",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);

        cpssOsMemSet(&ruleAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
        cpssOsMemCpy(ruleAttr.ruleNameArr,"simple rule ",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        for (ii=0;ii<8;ii++)
        {
            stagesArr[0] = ii;
            st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId, 1+ii,&keyAttr, 1, stagesArr,
                                                        CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E,
                                                        1, fieldsArr);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerLogicalKeyCreate keyId %d " , 1+ii);

            st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId, 1+ii, groupId, 1+ii, &ruleAttr,
                                                1, fieldsValueArr, actionId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleAdd ruleId %d " , 1+ii);
        }

        paGrpDbPtr = paMngDbPtr->paGrpPtrArr[groupId];

        for (ii=0; ii<paGrpDbPtr->numOfRules ;ii++)
        {
            stagesArr[0] = ii;
            UTF_VERIFY_EQUAL3_STRING_MAC(stagesArr[0], paGrpDbPtr->stagesArr[ii],"wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                         paStageStrArr[paGrpDbPtr->stagesArr[ii]],paStageStrArr[stagesArr[0]]);
            UTF_VERIFY_EQUAL2_STRING_MAC(ii+1, paGrpDbPtr->paRulePtrArr[ii+1]->paRuleId," wrong ruleId, got [%d] while expected [%d] " ,
                                         paGrpDbPtr->paRulePtrArr[ii+1]->paRuleId,ii+1);
        }

        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,1, groupId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId);

        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,8, groupId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId);

        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,5, groupId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId);

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId, 1, groupId, 1, &ruleAttr,
                                                1, fieldsValueArr, actionId);

        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,2, groupId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId);

        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,7, groupId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId);

        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,1, groupId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId);

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,5, groupId, 5, &ruleAttr,
                                                1, fieldsValueArr, actionId);

        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,3, groupId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId);

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId,7, groupId, 7, &ruleAttr,
                                                1, fieldsValueArr, actionId);

        st = cpssDxChPacketAnalyzerGroupRuleDelete(managerId,6, groupId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupRuleDelete ruleId %d " , ruleId);


        UTF_VERIFY_EQUAL1_STRING_MAC(3, paGrpDbPtr->numOfRules," wrong value, got [%s] while expected 7 " ,
                                      paGrpDbPtr->numOfRules );

        for (ii=0; ii<paGrpDbPtr->numOfRules ;ii++)
        {
            UTF_VERIFY_EQUAL2_STRING_MAC(expRuleIdStageValues3[ii], paGrpDbPtr->stagesArr[ii]," wrong value, got [%d] while expected [%d]  " ,
                                         paStageStrArr[paGrpDbPtr->stagesArr[ii]],expRuleIdStageValues3[ii]);
            UTF_VERIFY_EQUAL2_STRING_MAC(expRuleIdStageValues3[ii]+1, paGrpDbPtr->paRulePtrArr[ii+1]->paRuleId," wrong ruleId, got [%d] while expected [%d] " ,
                                         paGrpDbPtr->paRulePtrArr[ii+1]->paRuleId,expRuleIdStageValues3[ii]+1);
        }

        /* AUTODOC: enable group activation */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupActivateEnableSet ruleId %d " , groupId);

        /* AUTODOC: enable sampling on all rules bounded to action */
        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId,actionId, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerActionSamplingEnableSet ruleId %d " , actionId);

        /* AUTODOC: enable sampling on all rules bounded to action */
        st = cpssDxChPacketAnalyzerActionSamplingEnableSet(managerId,actionId, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerActionSamplingEnableSet ruleId %d " , actionId);

        /* AUTODOC: enable group activation */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerGroupActivateEnableSet ruleId %d " , groupId);


        /* Restore system. */
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerMuxStageBind
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerMuxStageBind)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1.1 Call function with uds stages that are muxed
    Expected: GT_OK for first uds GT_BAD_STATE for the other
    1.1.2 Call function after select stage that is muxed
    Expected: GT_OK and number of valid stages [11] and valid stages as expected.
    1.2. Call function when stage still bounded .
    Expected: GT_BAD_STATE.
    1.3. Call function when stage not valid.
    Expected: GT_BAD_PARAM.
    1.4. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                               st = GT_OK;
    GT_U8                                                   devNum;
    GT_U32                                                  managerId,keyId,groupId,actionId,ruleId;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC            keyAttr;
    CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC           ruleAttr;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             stagesArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                     fieldsArr[SIZE_OF_TEST_ARR];
    GT_U32                                                  numOfStages,numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC               fieldsValueArr[SIZE_OF_TEST_ARR];

    GT_U32                                                  ii,numOfApplicStagesGet,expNumOfApplicStages = 0;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             applicStagesListArr[SIZE_OF_TEST_ARR];
    GT_U32                                                  expNumOfValidStages = 0 ,numOfValidStagesGet;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             validStagesListArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT             expValidStages[SIZE_OF_TEST_ARR];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                   paMngDbPtr=NULL;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          udsId[2];
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC        interfaceAttributes;
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC         udsAttr[2];

     /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    UT_PA_CHECK_XML

    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* 1. Set Packet Analyzer basic configuration: */

        /*set default values */
        managerId = 1;
        keyId = 1;
        groupId =1 ;
        actionId =1;
        ruleId = 1;

        cpssOsMemSet(stagesArr,0,sizeof(stagesArr));
        cpssOsMemSet(fieldsArr,0,sizeof(fieldsArr));
        cpssOsMemSet(fieldsValueArr,0,sizeof(fieldsValueArr));

        /* initialized managerId with device*/
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /* AUTODOC: enable PA on all devices added to manager */
        st = cpssDxChPacketAnalyzerManagerEnableSet(managerId, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at cpssDxChPacketAnalyzerManagerEnableSet ");

        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericGroupInit ");
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericActionInit ");

        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsMemCpy(expValidStages, utPaHarrierDefaultValidStages, sizeof(utPaHarrierDefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsMemCpy(expValidStages, utPaPhoenixDefaultValidStages, sizeof(utPaPhoenixDefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsMemCpy(expValidStages, utPaHawkDefaultValidStages, sizeof(utPaHawkDefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsMemCpy(expValidStages, utPaFalconDefaultValidStages, sizeof(utPaFalconDefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsMemCpy(expValidStages, utPaAldrin2DefaultValidStages, sizeof(utPaAldrin2DefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsMemCpy(expValidStages, utPaAldrinDefaultValidStages, sizeof(utPaAldrinDefaultValidStages));
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }


        /*
            1.1.1 Call function with uds stages that are muxed
            Expected: GT_OK for first uds GT_BAD_STATE for the other
        */
        if (paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            cpssOsStrCpy(udsAttr[0].udsNameArr, "PDS_2_QFC_DEQ");
            cpssOsStrCpy(interfaceAttributes.instanceId, "tile0_pipe1_txqs_macro_i3_pipe0_1");
            cpssOsStrCpy(interfaceAttributes.interfaceId, "PDS_2_QFC_DEQ");
            interfaceAttributes.interfaceIndex = 2;
            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr[0],&interfaceAttributes,&udsId[0]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            cpssOsStrCpy(udsAttr[1].udsNameArr, "pds2tx_desc");
            cpssOsStrCpy(interfaceAttributes.instanceId, "tile0_pipe1_txqs_macro_i3_pipe0_0");
            cpssOsStrCpy(interfaceAttributes.interfaceId, "pds2tx_desc");
            interfaceAttributes.interfaceIndex = 1;

            st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr[1],&interfaceAttributes,&udsId[1]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId);

            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,udsId[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,udsId[1]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,udsId[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,udsId[1]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*remove Uds*/
            st = cpssDxChPacketAnalyzerUserDefinedStageDelete(managerId,&udsAttr[0]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            st = cpssDxChPacketAnalyzerUserDefinedStageDelete(managerId,&udsAttr[1]);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }

        /*
            1.1.2 Call function after select stage that is muxed
            Expected: GT_OK and number of valid stages [10] and valid stages as expected.
        */

        /* AUTODOC: create key and rule in 1 group  */
        cpssOsMemSet(&keyAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
        cpssOsMemCpy(keyAttr.keyNameArr,"simple key  ",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);

        cpssOsMemSet(&ruleAttr,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_RULE_ATTRIBUTES_STC));
        cpssOsMemCpy(ruleAttr.ruleNameArr,"simple rule ",sizeof(GT_CHAR)*CPSS_DXCH_PACKET_ANALYZER_MAX_NAME_LENGTH_CNS);

        numOfStages=5;
        numOfFields=1;
        stagesArr[0]=  CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
        stagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E;

        stagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E;
        stagesArr[3]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E ;
        stagesArr[4]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E ;

        fieldsArr[0]= CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;

        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId, keyId, &keyAttr, numOfStages, stagesArr,
                                                    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_PARTIAL_E,
                                                    numOfFields, fieldsArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerLogicalKeyCreate keyId %d ", keyId);

        fieldsValueArr[0].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E;
        fieldsValueArr[1].fieldName=CPSS_DXCH_PACKET_ANALYZER_FIELD_LOCAL_DEV_SRC_PORT_E;

        st = cpssDxChPacketAnalyzerGroupRuleAdd(managerId, keyId , groupId, ruleId, &ruleAttr,
                                                numOfFields, fieldsValueArr, actionId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerGroupRuleAdd ruleId %d ",keyId);

        /*write rule to HW  */
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId,GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        numOfApplicStagesGet=27;
        numOfValidStagesGet=27;
        expNumOfApplicStages = 16;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            expNumOfApplicStages = 16;
            expNumOfValidStages=15;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            expNumOfApplicStages = 18;
            expNumOfValidStages=17;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            expNumOfApplicStages = 18;
            expNumOfValidStages=13;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            expNumOfValidStages=10;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            expNumOfValidStages = 8;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                     "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                     paStageStrArr[validStagesListArr[ii]],
                                     paStageStrArr[expValidStages[ii]]);
        }

        if (paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E )
        {
            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }
        else
        {
            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId, CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
        }

        numOfApplicStagesGet=27;
        numOfValidStagesGet=27;
        expNumOfApplicStages=16;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            expNumOfApplicStages = 16;
            expNumOfValidStages=13;
            for (ii = 3; ii < 10; ii++)
            {
                expValidStages[ii] = expValidStages[ii+1];
            }
            for (ii = 10; ii < expNumOfValidStages; ii++)
            {
                expValidStages[ii] = expValidStages[ii+2];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            expNumOfApplicStages=18;
            expNumOfValidStages=15;
            for (ii = 3; ii < 11; ii++)
            {
                expValidStages[ii] = expValidStages[ii+1];
            }
            for (ii = 11; ii < expNumOfValidStages; ii++)
            {
                expValidStages[ii] = expValidStages[ii+2];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            expNumOfApplicStages=18;
            expNumOfValidStages=12;
            /*update expected values*/
            for (ii = 3; ii < expNumOfValidStages; ii++)
            {
                expValidStages[ii] = expValidStages[ii+1];
            }
            expValidStages[9]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            expNumOfValidStages=10;
            expValidStages[3]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E;
            expValidStages[9]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            expNumOfValidStages=8;
            expValidStages[2]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E;
            expValidStages[7]=CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);


        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                     "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                     paStageStrArr[validStagesListArr[ii]],
                                     paStageStrArr[expValidStages[ii]]);
        }

         /*write rule to HW*/
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);
        st = cpssDxChPacketAnalyzerGroupActivateEnableSet(managerId,groupId,GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        /*check again that correct mux was chosen*/

        numOfApplicStagesGet=27;
        numOfValidStagesGet=27;
        expNumOfApplicStages=16;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            expNumOfApplicStages=16;
            expNumOfValidStages=13;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            expNumOfApplicStages=18;
            expNumOfValidStages=15;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            expNumOfApplicStages=18;
            expNumOfValidStages=12;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            expNumOfValidStages=10;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            expNumOfValidStages=8;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                     "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                     paStageStrArr[validStagesListArr[ii]],
                                     paStageStrArr[expValidStages[ii]]);
        }

        /*
            1.2. Call function when stage still bounded .
            Expected: GT_BAD_STATE.
        */
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_STATE, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /*
            1.3. Call function when stage not valid.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);

        /* Restore system. */
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.4. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerMuxStageBind(managerId,CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, devNum);

    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPacketAnalyzerMuxStageUnbind
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerMuxStageUnbind)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E/
                                     CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E/
                                     CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E]
    Expected: GT_OK and values as expected
    1.2. Call function with wrong stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E] .
    Expected: GT_BAD_PARAM.
    1.3. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stageId;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC    boundInterfaceArrExp;
    PRV_CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC    boundInterfaceArrGet;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          applicStagesListArr[SIZE_OF_TEST_ARR];
    GT_U32                                               numOfApplicStagesGet=16 ,expNumOfApplicStages =16;
    GT_U32                                               expNumOfValidStages = 0,numOfValidStagesGet=16;
    GT_U32                                               ii;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          validStagesListArr[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          expValidStages[SIZE_OF_TEST_ARR];
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC        udsInterfaceAttributes;
    CPSS_DXCH_PACKET_ANALYZER_UDS_ATTRIBUTES_STC         udsAttr;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          udsId;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        cpssOsMemSet(&boundInterfaceArrExp,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));
        cpssOsMemSet(&boundInterfaceArrGet,0,sizeof(CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        /*
            1.1.1 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E]
            Expected: GT_OK and False in valid bit.
        */
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsMemCpy(expValidStages, utPaHarrierDefaultValidStages, sizeof(utPaHarrierDefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsMemCpy(expValidStages, utPaPhoenixDefaultValidStages, sizeof(utPaPhoenixDefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsMemCpy(expValidStages, utPaHawkDefaultValidStages, sizeof(utPaHawkDefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsMemCpy(expValidStages, utPaFalconDefaultValidStages, sizeof(utPaFalconDefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsMemCpy(expValidStages, utPaAldrin2DefaultValidStages, sizeof(utPaAldrin2DefaultValidStages));
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsMemCpy(expValidStages, utPaAldrinDefaultValidStages, sizeof(utPaAldrinDefaultValidStages));
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E ;
        boundInterfaceArrExp.isValid = GT_TRUE;

        st = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&boundInterfaceArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stageId);

        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC( boundInterfaceArrExp.isValid ,
                                      boundInterfaceArrGet.isValid ,
                                     "got other isValid than expected");

        st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,stageId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stageId);

        boundInterfaceArrExp.isValid = GT_FALSE;
        st = prvCpssDxChPacketAnalyzerStageInterfacesGet(managerId,devNum,stageId,&boundInterfaceArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stageId);

        /* verifying values */
        UTF_VERIFY_EQUAL0_STRING_MAC( boundInterfaceArrExp.isValid ,
                                      boundInterfaceArrGet.isValid ,
                                     "got other isValid than expected");


        numOfApplicStagesGet=27;
        numOfValidStagesGet=27;
        expNumOfApplicStages = 16;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            expNumOfApplicStages = 16;
            expNumOfValidStages=14;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            expNumOfApplicStages = 18;
            expNumOfValidStages=16;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            expNumOfApplicStages = 18;
            expNumOfValidStages=12;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            expNumOfValidStages=9;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            expNumOfValidStages=7;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        /*update expected values*/
        for (ii = 0; ii < numOfValidStagesGet; ii++)
        {
            expValidStages[ii] = expValidStages[ii+1];
        }

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                     "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                     paStageStrArr[validStagesListArr[ii]],
                                     paStageStrArr[expValidStages[ii]]);
        }

        /*
            1.1.2 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E]
            Expected: GT_OK and 8/10 valid stages.
        */

        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E ;
        if (paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
            stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;

        st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,stageId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stageId);

        numOfApplicStagesGet=27;
        numOfValidStagesGet=27;
        expNumOfApplicStages=16;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            expNumOfApplicStages = 16;
            expNumOfValidStages=13;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            expNumOfApplicStages=18;
            expNumOfValidStages=15;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            expNumOfApplicStages=18;
            expNumOfValidStages=11;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            expNumOfValidStages=8;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            expNumOfValidStages=6;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);


        /*update expected values*/
        for (ii = 2; ii < numOfValidStagesGet; ii++)
        {
            expValidStages[ii] = expValidStages[ii+1];
        }

        if (paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E)
        {
            expValidStages[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_OAM_E ;
            expValidStages[2] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E ;
        }

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                     "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                     paStageStrArr[validStagesListArr[ii]],
                                     paStageStrArr[expValidStages[ii]]);
        }


        /*
            1.1.3 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E]
            Expected: GT_OK and [12] applic stages.
        */

        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E ;
        if (paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E )
            stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E ;

        st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,stageId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stageId);

        numOfApplicStagesGet=27;
        numOfValidStagesGet=27;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            expNumOfValidStages=12;
            for (ii=9;ii<expNumOfValidStages;ii++)
            {
                expValidStages[ii]=expValidStages[ii+1];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            expNumOfValidStages=14;
            for (ii=10;ii<expNumOfValidStages;ii++)
            {
                expValidStages[ii]=expValidStages[ii+1];
            }
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            expNumOfValidStages=10;
            expValidStages[8] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_REPLICATION_E ;
            expValidStages[9] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E ;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            expNumOfValidStages=7;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            expNumOfValidStages=5;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                     "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                     paStageStrArr[validStagesListArr[ii]],
                                     paStageStrArr[expValidStages[ii]]);
        }

        /*
            1.1.4 Call function with stageId [uds0/uds1]
            Expected: GT_OK .
        */
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 0");
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"har_epi_mac_macro_i0_mif2mti_app");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            udsInterfaceAttributes.interfaceIndex = 3;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"pnx_epi_100g_macro_mif2mti_app");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            udsInterfaceAttributes.interfaceIndex = 3;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"tile0_hawk_epi_400g_mac_macro_i2_mif2mti_app");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"MIF_2_MTI_TX_PKT_32B");
            udsInterfaceAttributes.interfaceIndex = 3;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_pipe1_eagle_ia_1_macro_erep2eq");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"erep2hbuegr_desc");
            udsInterfaceAttributes.interfaceIndex = 2;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "mbus_x_u11_buses_1_21");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"xbar2u_rd");
            udsInterfaceAttributes.interfaceIndex = 1;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "rxdma0_desc_in_1");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"PORT2MEM");
            udsInterfaceAttributes.interfaceIndex = 3;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        st = cpssDxChPacketAnalyzerMuxStageBind(managerId,udsId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageBind udsId %d ",
                                     udsId - PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E);

        expValidStages[expNumOfValidStages] = udsId ;
        cpssOsStrCpy(udsAttr.udsNameArr,"uds 1");
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"har_txqp_macro_sdq");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            udsInterfaceAttributes.interfaceIndex = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"pnx_txq_macro_sdq0");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            udsInterfaceAttributes.interfaceIndex = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId,"tile0_hawk_txqs_0_macro_sdq0");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"SDQ_2_PSI_PORT_BP");
            udsInterfaceAttributes.interfaceIndex = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "tile0_pipe1_txqs_macro_i3_pipe0_1");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"PDS_2_QFC_DEQ");
            udsInterfaceAttributes.interfaceIndex = 2;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "l2i_ipvx_desc_or_debug_buses_0_8");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"l2i2mt_na_bus");
            udsInterfaceAttributes.interfaceIndex = 2;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            cpssOsStrCpy(udsInterfaceAttributes.instanceId, "egress_hdr");
            cpssOsStrCpy(udsInterfaceAttributes.interfaceId,"ERMRK_2_TXFIFO_HEADER_CTRL_BUS");
            udsInterfaceAttributes.interfaceIndex = 1;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerUserDefinedStageAdd(managerId,&udsAttr,&udsInterfaceAttributes,&udsId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        st = cpssDxChPacketAnalyzerMuxStageBind(managerId,udsId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, " error at cpssDxChPacketAnalyzerMuxStageBind uds %d ",
                                     udsId - PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E);
        expNumOfValidStages++;
        expValidStages[expNumOfValidStages] = udsId ;

        numOfApplicStagesGet=27;
        numOfValidStagesGet=27;
        expNumOfApplicStages=18;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            expNumOfApplicStages=18;
            expNumOfValidStages=14;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            expNumOfApplicStages=20;
            expNumOfValidStages=16;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            expNumOfApplicStages=20;
            expNumOfValidStages=12;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            expNumOfValidStages=9;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            expNumOfValidStages=7;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);

        for (ii=0;ii<numOfValidStagesGet;ii++)
        {
            if (validStagesListArr[ii]>PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_LAST_E)
                break;

            UTF_VERIFY_EQUAL3_STRING_MAC(expValidStages[ii], validStagesListArr[ii],
                                     "wrong value in iteration %d, got [%s] while expected [%s]  " ,ii,
                                     paStageStrArr[validStagesListArr[ii]],
                                     paStageStrArr[expValidStages[ii]]);
        }

        st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,udsId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udsId - PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_UDS_FIRST_E);

        numOfApplicStagesGet=27;
        numOfValidStagesGet=27;
        expNumOfValidStages--;
        st = cpssDxChPacketAnalyzerStagesGet(managerId,
                                             &numOfApplicStagesGet,applicStagesListArr,
                                             &numOfValidStagesGet,validStagesListArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfApplicStages, numOfApplicStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfApplicStagesGet,expNumOfApplicStages);

        UTF_VERIFY_EQUAL2_STRING_MAC(expNumOfValidStages, numOfValidStagesGet,
                                     "wrong value, got [%d] while expected [%d]  " ,
                                     numOfValidStagesGet,expNumOfValidStages);


        /*
            1.2. Call function with wrong stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E] .
            Expected: GT_BAD_PARAM.
        */
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E ;
        st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,stageId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, stageId);

        /*
            1.3. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E ;
        st = cpssDxChPacketAnalyzerMuxStageUnbind(managerId,stageId);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, stageId);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

/*
GT_STATUS cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet
(
    IN    GT_U32                                        managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT   stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldName,
    INOUT GT_U32                                        *numOfFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT           fieldsArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1.1 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E]
                             fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E/
                                        CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E /
                                        CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E]
    Expected: GT_OK.
    1.2. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_FIRST_E]
    Expected: GT_BAD_PARAM.
    1.3. Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E]
    Expected: GT_BAD_PARAM.
    1.4. Call function with numOfFields[0]
    Expected: GT_BAD_SIZE.
    1.5. Call with NULL numOfFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call with NULL fieldsArr and other valid params.
    Expected: GT_BAD_PTR.
    1.7. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stage ;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  testedField;
    GT_U32                                               ii, numOfFieldsExp = 0,numOfFieldsGet;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsArrGet[SIZE_OF_TEST_ARR*10];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  udfIdArr[2];
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC         udfAttr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC             fieldName;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsExp[3];

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        cpssOsMemSet(fieldsArrGet, 0, sizeof(fieldsArrGet));
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /*
            1.1.1 Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E]
                                     fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E/
                                                CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E /
                                                CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E]
            Expected: GT_OK.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        testedField = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId,stage,testedField,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        UTF_VERIFY_EQUAL1_STRING_MAC(0, numOfFieldsGet,
                        "got other numOfFieldsGet than expected: %d", numOfFieldsGet );

        testedField = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId,stage,testedField,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            numOfFieldsExp = 2;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            numOfFieldsExp = 1;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        fieldsExp[0]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;
        fieldsExp[1]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E;

        UTF_VERIFY_EQUAL1_STRING_MAC(numOfFieldsExp, numOfFieldsGet,
            "got other numOfFieldsGet than expected: %d", numOfFieldsGet );
        for(ii=0;ii<numOfFieldsGet ; ii++)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(fieldsExp[ii], fieldsArrGet[ii],
                           "got other fieldsArrGet than expected " );
        }

        testedField = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId,stage,testedField,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);
        numOfFieldsExp = 1;
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfFieldsExp, numOfFieldsGet,
                        "got other numOfFieldsGet than expected: %d", numOfFieldsGet );

        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
                fieldsExp[0]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                fieldsExp[0]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        for(ii=0;ii<numOfFieldsGet ; ii++)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(fieldsExp[ii], fieldsArrGet[ii],
                           "got other fieldsArrGet than expected " );
        }

        /*
            1.1.2 add 2 udf with fields that overlap
                  Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E]
                                     fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E]
            Expected: GT_OK.
        */
        cpssOsStrCpy(fieldName.fieldNameArr,"arp_sip");
        cpssOsStrCpy(udfAttr.udfNameArr,"arp_sip");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfIdArr[0]);

        cpssOsStrCpy(fieldName.fieldNameArr,"fcoe_s_id");
        cpssOsStrCpy(udfAttr.udfNameArr,"fcoe_s_id");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[1]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfIdArr[1]);

        testedField = udfIdArr[0] ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId,stage,testedField,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum,stage);

        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
                numOfFieldsExp=2;
                fieldsExp[0]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E;
                fieldsExp[1]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                numOfFieldsExp = 3;
                fieldsExp[0]= CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_1_E;
                fieldsExp[1]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;
                fieldsExp[2]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
                numOfFieldsExp = 3;
                fieldsExp[0]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E;
                fieldsExp[1]= CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_SIP_E;
                fieldsExp[2]= CPSS_DXCH_PACKET_ANALYZER_FIELD_UDF_1_E;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(numOfFieldsExp, numOfFieldsGet,
                "got other numOfFieldsGet than expected: %d", numOfFieldsGet );

        for(ii=0;ii<numOfFieldsGet ; ii++)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(fieldsExp[ii], fieldsArrGet[ii],
                           "got other fieldsArrGet than expected " );
        }

        /*
            1.2. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_EGRESS_STAGES_FIRST_E]
            Expected: GT_BAD_PARAM.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_INGRESS_STAGES_E ;
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId,stage,testedField,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum,stage);
        /*
            1.3. Call function with fieldName [CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E]
            Expected: GT_BAD_PARAM.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E ;
        testedField = CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E ;
        st = cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId,stage,testedField,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum,stage);

        /*
            1.4. Call function with stageId [CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E]
            Expected: GT_BAD_SIZE.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_ROUTER_E ;
        testedField = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E;
        numOfFieldsGet = 0;
        st = cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId,stage,testedField,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum,stage);

        /*
            1.5. Call with NULL numOfFieldsPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfFieldsGet = SIZE_OF_TEST_ARR*10;
        st = cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId,stage,testedField,NULL,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,stage);

        /*                                                         .
            1.6. Call with NULL fieldsArrGet and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId,stage,testedField,&numOfFieldsGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum,stage);
        /*                                                    .
            1.7. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet(managerId,stage,testedField,&numOfFieldsGet,fieldsArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum,stage);

    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

/*
GT_STATUS prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields
(
    IN  GT_U32                                          managerId,
    IN  GT_U32                                          keyId,
    IN  GT_U32                                          numOfFields,
    IN  CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC       fieldsValueArr[]
)
*/
UTF_TEST_CASE_MAC(prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1.1 Call function with valid key and fields [CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E/
                                                   CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E /
                                                   CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E]
    Expected: GT_OK.
    1.2. Call function with unset keyId
    Expected: GT_NOT_FOUND.
    1.3. Call with NULL fieldsValueArr and other valid params.
    Expected: GT_BAD_PTR.
    1.4. Call function when manager is not initialized .
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;

    GT_U32                                               keyId,numOfFields;
    CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC         keyAttr;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stagesArr[2];
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ENT  fieldMode;

    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC            fieldsValueArr[4];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  udfIdArr[2];
    CPSS_DXCH_PACKET_ANALYZER_UDF_ATTRIBUTES_STC         udfAttr;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_NAME_STC             fieldName;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_ENT                  fieldsArr[4];


    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at genericManagerAndDeviceInit ");
        cpssOsMemSet(fieldsValueArr, 0, sizeof(fieldsValueArr));

        /* initialized Key*/
        keyId=1;
        st = genericKeyInitSimple(managerId,keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        numOfFields = 2 ;
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E ;
        fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_MAC_DA_E ;
        st = prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields(managerId,keyId,numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields ");

        cpssOsStrCpy(fieldName.fieldNameArr,"arp_sip");
        cpssOsStrCpy(udfAttr.udfNameArr,"arp_sip");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[0]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfIdArr[0]);

        cpssOsStrCpy(fieldName.fieldNameArr,"fcoe_s_id");
        cpssOsStrCpy(udfAttr.udfNameArr,"fcoe_s_id");
        st = cpssDxChPacketAnalyzerUserDefinedFieldAdd(managerId,&udfAttr,&fieldName,&udfIdArr[1]);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, udfIdArr[1]);

        numOfFields = 4;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E ;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E ;
        fieldsArr[2] = udfIdArr[0] ;
        fieldsArr[3] = udfIdArr[1] ;
        st = cpssDxChPacketAnalyzerLogicalKeyFieldsAdd(managerId,keyId,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, keyId);

        numOfFields = 2;
        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E ;
        fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_BYTE_COUNT_E ;
        st = prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields(managerId,keyId,numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields ");

        fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E ;
        fieldsValueArr[1].fieldName = udfIdArr[0] ;
        st = prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields(managerId,keyId,numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st,"error at prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields ");

        /* initialized Key*/
        keyId=2;
        cpssOsMemSet(&keyAttr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_KEY_ATTRIBUTES_STC));
        stagesArr[0] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_PCL_E ;
        stagesArr[1] = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_BRIDGE_E ;
        fieldMode = CPSS_DXCH_PACKET_ANALYZER_FIELD_ASSIGNMENT_MODE_ASSIGN_ALL_E ;
        numOfFields = 3;
        fieldsArr[0] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E ;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E ;
        fieldsArr[1] = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E ;
        st = cpssDxChPacketAnalyzerLogicalKeyCreate(managerId,keyId,&keyAttr,2,stagesArr,fieldMode,numOfFields,fieldsArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericKeyInit ");

        numOfFields =2 ;
        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E ;
            fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_SIP_E ;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            fieldsValueArr[0].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV4_DIP_E ;
            fieldsValueArr[1].fieldName = CPSS_DXCH_PACKET_ANALYZER_FIELD_IPV6_DIP_E ;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        st = prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields(managerId,keyId,numOfFields,fieldsValueArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st,"error at prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields ");

        /*
            1.2. Call function with unset keyId
            Expected: GT_NOT_FOUND.
        */
        keyId = 12;
        st = prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields(managerId,keyId,0,fieldsValueArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_FOUND, st,"error at prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields ");

        /*
            1.3. Call with NULL fieldsValueArr and other valid params.
            Expected: GT_BAD_PTR.

        */
        keyId = 1;
        st = prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields(managerId,keyId,0,NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PTR, st,"error at prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields ");
        /*                                                    .
            1.4. Call function when manager is not initialized .
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields(managerId,keyId,0,fieldsValueArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_FOUND, st,"error at prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields ");

    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*
GT_STATUS cpssDxChPacketAnalyzerMuxStagesGet
(
    IN    GT_U32                                         managerId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    stageId,
    INOUT GT_U32                                         *numOfMuxStagesPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT    muxStagesListArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerMuxStagesGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E)
    1.1. Call function cpssDxChPacketAnalyzerMuxStagesGet with
         valid stage
    Expected: GT_OK and expected mux stages
    1.2. Call function with not applicable stage
    Expected: GT_BAD_STATE.
    1.3. Call function with numOfMuxStagesPtr less then needed
    Expected: GT_BAD_SIZE.
    1.4. Call with NULL numOfMuxStagesPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.5. Call with NULL muxStagesListArr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call Call function when manager is not initialized
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U32                                               managerId;
    GT_U8                                                devNum;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stage;
    GT_U32                                               expNumOfStages=0;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          expStagesArr[SIZE_OF_TEST_ARR];
    GT_U32                                               ii;

    GT_U32                                               numOfStagesGet;
    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT          stagesArrGet[SIZE_OF_TEST_ARR];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        cpssOsMemSet(expStagesArr, 0, sizeof(expStagesArr));
        cpssOsMemSet(stagesArrGet, 0, sizeof(stagesArrGet));
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum,&managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st," error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /*
            1.1.1 Call function cpssDxChPacketAnalyzerMuxStagesGet with
                 valid stage
            Expected: GT_OK and expected mux stages
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;
        numOfStagesGet = SIZE_OF_TEST_ARR ;
        st = cpssDxChPacketAnalyzerMuxStagesGet(managerId,stage,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stage);

        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            expNumOfStages = 0;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            expStagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E;
            expNumOfStages = 1;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            expStagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_MLL_E;
            expStagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_E_QUEUE_E;
            expStagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_FILTER_E;
            expNumOfStages = 3;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(expNumOfStages, numOfStagesGet,
                       "got other numOfStagesGet than was set :", numOfStagesGet);

        for (ii=0 ; ii<numOfStagesGet ; ii++)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(expStagesArr[ii], stagesArrGet[ii],
                           "got other stagesArrGet than was set: %d index %d ", stagesArrGet[ii] , ii);
        }

        /*
            1.1.2 Call function cpssDxChPacketAnalyzerMuxStagesGet with
                 valid stage
            Expected: GT_OK and expected mux stages
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E;
        numOfStagesGet = SIZE_OF_TEST_ARR ;
        st = cpssDxChPacketAnalyzerMuxStagesGet(managerId,stage,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stage);

        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            expStagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
            expNumOfStages = 1;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            expStagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E;
            expNumOfStages = 1;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            expStagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
            expStagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E;
            expStagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PHA_E;
            expNumOfStages = 3;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            expStagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
            expStagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E;
            expStagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E;
            expNumOfStages = 3;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            expStagesArr[0]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_PCL_E;
            expStagesArr[1]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_POLICER_E;
            expStagesArr[2]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_TIMESTAMP_E;
            expStagesArr[3]= CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_MAC_E;
            expNumOfStages = 4;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(expNumOfStages, numOfStagesGet,
                       "got other numOfStagesGet than was set :", numOfStagesGet);

        for (ii=0 ; ii<numOfStagesGet ; ii++)
        {
            /* verifying values */
            UTF_VERIFY_EQUAL2_STRING_MAC(expStagesArr[ii], stagesArrGet[ii],
                           "got other stagesArrGet than was set: %d index %d ", stagesArrGet[ii] , ii);
        }

        /*
            1.1.2 Call function cpssDxChPacketAnalyzerMuxStagesGet with
                 valid stage
            Expected: GT_OK and expected mux stages
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_TTI_E;
        numOfStagesGet = SIZE_OF_TEST_ARR ;
        st = cpssDxChPacketAnalyzerMuxStagesGet(managerId,stage,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, stage);

        expNumOfStages = 0;
        if (paMngDbPtr->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E )
            expNumOfStages = 1;
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(expNumOfStages, numOfStagesGet,
                       "got other numOfStagesGet than was set %d:", numOfStagesGet);

        /*
            1.2. Call function with not applicable stage
            Expected: GT_BAD_PARAM.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E ;
        numOfStagesGet = SIZE_OF_TEST_ARR;
        st = cpssDxChPacketAnalyzerMuxStagesGet(managerId,stage,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, devNum, stage);

        /*
            1.3. Call function with numOfMuxStagesPtr less then needed
            Expected: GT_BAD_SIZE.
        */
        stage = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_EGRESS_PRE_OAM_E ;
        numOfStagesGet = 0;
        st = cpssDxChPacketAnalyzerMuxStagesGet(managerId,stage,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, devNum, stage);

        switch(paMngDbPtr->devFamily)
        {
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
            expNumOfStages = 1;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
            expNumOfStages = 3;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            expNumOfStages = 3;
            break;
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            expNumOfStages = 4;
            break;
        default:
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);
            break;
        }

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(expNumOfStages, numOfStagesGet,
                       "got other numOfStagesGet than was set :", numOfStagesGet);
        /*
            1.4. Call with NULL numOfMuxStagesPtr and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfStagesGet = SIZE_OF_TEST_ARR;
        st = cpssDxChPacketAnalyzerMuxStagesGet(managerId,stage,NULL,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, stage);
        /*
            1.5. Call with NULL muxStagesListArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        numOfStagesGet = SIZE_OF_TEST_ARR;
        st = cpssDxChPacketAnalyzerMuxStagesGet(managerId,stage,&numOfStagesGet,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, devNum, stage);

        /*
            1.6. Call Call function when manager is not initialized
            Expected: GT_NOT_FOUND.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerMuxStagesGet(managerId,stage,&numOfStagesGet,stagesArrGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, devNum, stage);
    }

#else /* ASIC_SIMULATION */

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}

UTF_TEST_CASE_MAC(xmlDescriptorDuplication)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_ALDRIN2_E | UTF_ALDRIN_E | UTF_FALCON_E )
    checks xml for duplicates interface internal name and index
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;

    GT_U32                                               ii,jj,kk,ww,numOfInterfaces,numOfInstances;
    GT_U32                                               segmentId,SegmentId2;
    GT_CHAR                                              **instancesArr;
    CPSS_DXCH_PACKET_ANALYZER_STAGE_INTERFACE_STC        interfacesArrGet[SIZE_OF_TEST_ARR*4];
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR                paMngDbPtr=NULL;
    GT_BOOL                                              indexArr[8],interfaceToSkipArr[SIZE_OF_TEST_ARR*4],found,testFail;/*max size of mux legs*/

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, (UTF_ALDRIN2_E | UTF_FALCON_E | UTF_ALDRIN_E));

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        cpssOsMemSet(interfacesArrGet,0,sizeof(interfacesArrGet));
        cpssOsMemSet(interfaceToSkipArr,0,sizeof(interfaceToSkipArr));
        cpssOsMemSet(indexArr,0,sizeof(indexArr));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        numOfInstances = 500;
        st = prvCpssDxChPacketAnalyzerInstancesGet(managerId,devNum,&numOfInstances,&instancesArr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");

        testFail = GT_FALSE;
        /*run on all Instances */
        for (ii=0;ii<numOfInstances;ii++)
        {
            cpssOsMemSet(indexArr,0,sizeof(indexArr));
            cpssOsMemSet(interfaceToSkipArr,0,sizeof(interfaceToSkipArr));
            found = GT_FALSE;

            numOfInterfaces = SIZE_OF_TEST_ARR*4;
            st = prvCpssDxChPacketAnalyzerInstanceInterfacesGet(managerId,devNum,instancesArr[ii],&numOfInterfaces,interfacesArrGet);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, devNum, ii,numOfInterfaces);

            /*check for interfaces with same index*/
            for (jj=0;jj<numOfInterfaces-1;jj++)
            {
                if(PRV_CPSS_SIP_6_CHECK_MAC(devNum)==GT_TRUE)
                {
                    if((cpssOsStrStr(interfacesArrGet[jj].interfaceId,"D2D_Payload")!=NULL)  ||
                       (cpssOsStrStr(interfacesArrGet[jj].interfaceId,"RX_2_PB_Payload")!=NULL)  ||
                       (cpssOsStrStr(interfacesArrGet[jj].interfaceId,"D2D_Credit")!=NULL)) /*skip*/
                        continue;
                }
                if(interfaceToSkipArr[jj] == GT_TRUE ) /*skip this interface*/
                    continue;
                found = GT_FALSE;
                for (kk=jj+1;kk<numOfInterfaces;kk++)
                {
                    if(interfacesArrGet[jj].interfaceIndex == interfacesArrGet[kk].interfaceIndex )
                    {
                        st = prvCpssDxChIdebugInterfaceMxmlSegmentGet(devNum, interfacesArrGet[jj].interfaceId, &segmentId);
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfacesArrGet[jj].interfaceId);
                        st = prvCpssDxChIdebugInterfaceMxmlSegmentGet(devNum, interfacesArrGet[kk].interfaceId, &SegmentId2);
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, devNum, interfacesArrGet[kk].interfaceId);
                        if (segmentId == SegmentId2) /*not Concatinated*/
                        {
                            if (found == GT_FALSE) cpssOsPrintf("\nInstance %s interfaces: %s segment %d, ",
                                                                instancesArr[ii], interfacesArrGet[jj].interfaceId, segmentId);
                            found = GT_TRUE;
                            testFail = GT_TRUE;
                            cpssOsPrintf("%s segment %d , ", interfacesArrGet[kk].interfaceId, SegmentId2);
                            interfaceToSkipArr[kk] = GT_TRUE; /*set array index to skip */
                        }
                    }
                }
            }/*numOfInterfaces jj*/

            cpssOsMemSet(interfaceToSkipArr,0,sizeof(interfaceToSkipArr));
            found = GT_FALSE;

            /*check for Interfaces with same internal name  */
            for (jj=0;jj<numOfInterfaces-1;jj++)
            {
                if(cpssOsStrStr(interfacesArrGet[jj].interfaceId,"desc")==NULL) /*checks only descriptors */
                    continue;
                if((cpssOsStrStr(interfacesArrGet[jj].interfaceId,"mem2p")!=NULL)  ||
                   (cpssOsStrStr(interfacesArrGet[jj].interfaceId,"p2mem")!=NULL)  ||
                   (cpssOsStrStr(interfacesArrGet[jj].interfaceId,"cpu2mem")!=NULL)||
                   (cpssOsStrStr(interfacesArrGet[jj].interfaceId,"mem2cpu")!=NULL)||
                   (cpssOsStrStr(interfacesArrGet[jj].interfaceId,"txq_ll2dq_desc")!=NULL)||
                   (cpssOsStrStr(interfacesArrGet[jj].interfaceId,"txq_ll2txdma_desc")!=NULL)  ) /*skip*/
                    continue;

                if(interfaceToSkipArr[jj] == GT_TRUE ) /*skip this interface*/
                    continue;
                for (kk=jj+1;kk<numOfInterfaces;kk++)
                {
                    if((cpssOsStrCmp(interfacesArrGet[jj].interfaceId ,interfacesArrGet[kk].interfaceId) == 0)&&
                       (interfacesArrGet[jj].interfaceIndex !=interfacesArrGet[kk].interfaceIndex ))
                    {
                        found = GT_TRUE ;
                        indexArr[interfacesArrGet[jj].interfaceIndex]= GT_TRUE;
                        indexArr[interfacesArrGet[kk].interfaceIndex]= GT_TRUE;
                        interfaceToSkipArr[kk] = GT_TRUE ; /*set array index to skip */
                    }
                }

                if (found == GT_TRUE )
                {
                    cpssOsPrintf("\nInstance %s interface %s index: ",instancesArr[ii],interfacesArrGet[jj].interfaceId);
                    for (ww=0;ww<8;ww++)
                    {
                        if (indexArr[ww] == GT_TRUE)
                            cpssOsPrintf("%d ",ww );
                    }
                    testFail = GT_TRUE ;
                    found = GT_FALSE;
                    cpssOsMemSet(indexArr,0,sizeof(indexArr));
                }
            }/*numOfInterfaces jj*/
        }/*numOfInstances*/

        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, testFail,"Duplicate interface internal names found in xml !");
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif
}

/*
GT_STATUS cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet
(
    IN    GT_U32                                           managerId,
    IN    GT_U32                                           ruleId,
    IN    GT_U32                                           groupId,
    IN    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT      stageId,
    IN    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC  *searchAttributePtr,
    OUT   GT_U32                                          *numOfHitsPtr,
    INOUT GT_U32                                          *numOfSampleFieldsPtr,
    OUT   CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC        sampleFieldsValueArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet)
{
#ifndef GM_USED
/*
    ITERATE_DEVICES (UTF_FALCON_E)
    this UT doesnt check values
    1.1. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.2. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
    Expected: GT_BAD_PARAM.
    1.3. Call function with ruleId not exist [2]
    Expected: GT_NOT_FOUND.
    1.4. Call function with groupId not exist [2]
    Expected: GT_NOT_FOUND.
    1.5. Call with searchAttributePtr with bad params .
    Expected: GT_BAD_PARAM.
    1.5. Call with NULL searchAttributePtr and other valid params.
    Expected: GT_BAD_PTR.
    1.6. Call with NULL numOfHitsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.7. Call with NULL numOfSampleFieldsPtr and other valid params.
    Expected: GT_BAD_PTR.
    1.8. Call with NULL sampleFieldsValueArr and other valid params.
    Expected: GT_BAD_PTR.
    1.9. Call with numOfSampleFields less than max fields number per stage.
    Expected: GT_BAD_SIZE.
    1.10. Call function with stageId not exist [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E]
    Expected: GT_BAD_PARAM.
    1.11. Call Call function when manager is not initialized
    Expected: GT_NOT_FOUND.
*/
    GT_STATUS                                            st = GT_OK;
    GT_U8                                                devNum;
    GT_U32                                               managerId;
    GT_U32                                               keyId;

    GT_U32                                               groupId;
    GT_U32                                               actionId;
    GT_U32                                               ruleId;

    CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ENT         stageId;
    CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC      searchAttribute;
    GT_U32                                              numOfHits;
    GT_U32                                              numOfSampleFields;
    CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC           *sampleFieldsValueArr = NULL;
    PRV_CPSS_DXCH_PACKET_ANALYZER_MNG_PTR               paMngDbPtr=NULL;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    UT_PA_CHECK_XML
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE)) {
        /* initialized managerId with device*/
        cpssOsMemSet(&searchAttribute, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_STC));

        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");
        st = prvCpssDxChPacketAnalyzerDbManagerGet(managerId,&paMngDbPtr);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"error at prvCpssDxChPacketAnalyzerDbManagerGet ");
        /* initialized Key*/
        keyId = 1;
        st = genericKeyInitSimple(managerId, keyId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericKeyInit ");
        /* initialized group ,action and rule*/
        groupId = 1;
        st = genericGroupInit(managerId,groupId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericGroupInit ");
        actionId = 1;
        st = genericActionInit(managerId,actionId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericActionInit ");
        ruleId = 1 ;
        genericRuleInit(managerId,keyId,groupId,actionId,ruleId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericRuleInit ");

        sampleFieldsValueArr = cpssOsMalloc(CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E*sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));
        if (sampleFieldsValueArr == NULL)
        {
            st = GT_OUT_OF_CPU_MEM;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at sampleFieldsValueArr cpssOsMalloc ");
        }
        cpssOsMemSet(sampleFieldsValueArr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_INGRESS_PRE_POLICER_E;
        /*
            1.1. Call function with wrong value ruleId [BAD_RULE_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = BAD_RULE_ID_VALUE;
        numOfSampleFields=CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.2. Call function with wrong value groupId [BAD_GROUP_ID_VALUE]
            Expected: GT_BAD_PARAM.
        */
        ruleId = 1;
        groupId = BAD_GROUP_ID_VALUE;
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.3. Call function with ruleId not exist [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 2;
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.4. Call function with groupId not exist [2]
            Expected: GT_NOT_FOUND.
        */
        ruleId = 1;
        groupId = 2;
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId);

        /*
            1.5.1 Call with searchAttributePtr with bad params - bad port number.
            Expected: GT_BAD_PARAM.
        */
        groupId=1;
        numOfSampleFields=CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E;
        searchAttribute.type = CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_PORT_E ;
        searchAttribute.portNum = 1025;
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.5.2 Call with searchAttributePtr with bad params.
            Expected: GT_BAD_PARAM.
        */
        searchAttribute.type = CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_CORE_E ;
        searchAttribute.portNum = 0;
        searchAttribute.tile = 5;
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /*
            1.5.3 Call with searchAttributePtr with bad params.
            Expected: GT_OUT_OF_RANGE.
        */
        searchAttribute.type = CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_CORE_E ;
        searchAttribute.tile = 0;
        searchAttribute.dp = 5;
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, ruleId, groupId);

        /*
            1.5.4 Call with searchAttributePtr with bad params.
            Expected: GT_OUT_OF_RANGE.
        */
        searchAttribute.type = CPSS_DXCH_PACKET_ANALYZER_SEARCH_ATTRIBUTE_TYPE_CORE_E ;
        searchAttribute.dp = 0;
        searchAttribute.pipe = 3;
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, st, ruleId, groupId);

        /*
            1.6. Call with NULL searchAttribute and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,NULL,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /*
            1.7. Call with NULL numOfHits and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,NULL,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /*
            1.8. Call with NULL numOfSampleFields and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,NULL,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /*
            1.9. Call with NULL sampleFieldsValueArr and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, ruleId, groupId);

        /*
            1.10. Call with numOfSampleFields less than max fields number per stage.
            Expected: GT_BAD_SIZE.
        */
        groupId = 1;
        ruleId = 1;
        stageId = PRV_CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_ALL_REGULAR_STAGES_FIRST_E;
        numOfSampleFields = 0;
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_SIZE, st, ruleId, groupId);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(5, numOfSampleFields,
                       "got other numOfSampleFields then Expected %d ", numOfSampleFields );
        /*
            1.11. Call function with stageId not exist [CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E]
            Expected: GT_BAD_PARAM.
        */
        numOfSampleFields = 10;
        stageId = CPSS_DXCH_PACKET_ANALYZER_LOOKUP_STAGES_LAST_E;
        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, ruleId, groupId);

        /* Restore system. */
        groupId = 1 ;
        st = cpssDxChPacketAnalyzerGroupDelete(managerId,groupId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, groupId);

        actionId = 1 ;
        st = cpssDxChPacketAnalyzerActionDelete(managerId,actionId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, actionId);

        /*
            1.12. Call Call function when manager is not initialized
            Expected: GT_NOT_FOUND.
        */
        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);

        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_FOUND, st, ruleId, groupId);
    }
    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_PA_E ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        sampleFieldsValueArr = cpssOsMalloc(CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E*sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC));
        if (sampleFieldsValueArr == NULL)
        {
            st = GT_OUT_OF_CPU_MEM;
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at sampleFieldsValueArr cpssOsMalloc ");
        }
        cpssOsMemSet(sampleFieldsValueArr, 0, sizeof(CPSS_DXCH_PACKET_ANALYZER_FIELD_VALUE_STC)*CPSS_DXCH_PACKET_ANALYZER_FIELD_LAST_E);
        /* initialized managerId with device*/
        managerId = GENERIC_MANAGER_ID ;
        st = genericManagerAndDeviceInit(devNum, &managerId);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, " error at genericManagerAndDeviceInit ");

        st = cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet(managerId,ruleId,groupId,stageId,&searchAttribute,&numOfHits,&numOfSampleFields,sampleFieldsValueArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, ruleId, groupId);

        st = cpssDxChPacketAnalyzerManagerDelete(managerId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, managerId);
    }
    if (sampleFieldsValueArr != NULL)
    {
        cpssOsFree(sampleFieldsValueArr);
    }

#else /* ASIC_SIMULATION */

        /* asic simulation not support this --> skip */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
#endif

}
/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPacketAnalyzerManager suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPacketAnalyzer)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerManagerCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerManagerDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerManagerDeviceAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerManagerDeviceRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerManagerEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerManagerDevicesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerManagerResetToDefaults)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerFieldSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerLogicalKeyCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerLogicalKeyFieldsAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerLogicalKeyFieldsRemove)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerLogicalKeyDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerLogicalKeyInfoGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerLogicalKeyStagesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerLogicalKeyFieldsPerStageGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerStagesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerGroupCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerGroupDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerActionCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerActionGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerActionUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerActionDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerGroupActivateEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerActionSamplingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerGroupRuleAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerGroupRuleUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerGroupRuleDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerSampledDataCountersClear)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerGroupRuleGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerRuleMatchStagesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerStageMatchDataGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerStageMatchDataAllFieldsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerStageFieldsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerFieldStagesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerMuxStageBind)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerMuxStageUnbind)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerStageFieldOverlappingFieldsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerMuxStagesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerRuleMatchDataAllFieldsByAttributeGet)

    /*prv*/
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerDbDeviceGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerDbStageGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerDbKeyGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerDbRuleGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerDbDevGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerDbManagerValidCheck)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerDfxPowerSaveEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerUserDefinedFieldAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerUserDefinedFieldDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerUserDefinedFieldGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerUserDefinedStageAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerUserDefinedStageDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPacketAnalyzerUserDefinedStageGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerStageInterfacesSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerStageInterfacesGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerStageFieldsGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerInstanceInterfacesGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerFieldInterfacesGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerInterfacesGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerInterfaceInfoGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerFieldSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPacketAnalyzerGroupRuleCheckOverlappingFields)

    /*Idebug*/
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugInterfaceNumFieldsGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugInterfaceFieldsGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugPortGroupInterfaceSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugPortGroupInterfaceReset)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugPortGroupInterfaceResetAll)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugPortGroupInterfaceActionSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugPortGroupInterfaceActionGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugPortGroupInterfaceMatchCounterGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugPortGroupInterfaceSampleAllFieldsDataGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugPortGroupInterfaceSampleListFieldsDataGet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugPortGroupInterfaceSamplingEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChIdebugPortGroupInterfaceSamplingEnableGet)

    /*DB*/
    UTF_SUIT_DECLARE_TEST_MAC(PacketAnalyzerDbTest1)
    UTF_SUIT_DECLARE_TEST_MAC(PacketAnalyzerDbTest2)
    UTF_SUIT_DECLARE_TEST_MAC(PacketAnalyzerDbTest3)
    UTF_SUIT_DECLARE_TEST_MAC(xmlDescriptorDuplication)

UTF_SUIT_END_TESTS_MAC(cpssDxChPacketAnalyzer)

