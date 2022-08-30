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
* @file cpssDxChPtpManagerUT.c
*
* @brief Unit tests for cpssDxChPtpManager.c/h, that provides
* 'Exact MatchPTP manager' CPSS DxCh facility implementation.
*
* @version   1
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManager.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* Defines */

/* Default valid value for port/trunk id */
#define TTI_VALID_PORT_TRUNK_CNS        20

/* Default valid value for vlan id */
#define TTI_VALID_VLAN_ID_CNS           100

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPtpInit(            \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerPtpInit(devNum,&outputInterfaceConf,                 \
           &ptpOverEthernetConf,&ptpOverIpUdpConf,&ptpTsTagGlobalConf,cpuCode); \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerPtpInit: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChPtpManagerPtpInit(                                \
    paramPtr /*one of : ptr1,ptr2,ptr3,ptr4*/,                                  \
    ptr1,ptr2,ptr3,ptr4                                                         \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChPtpManagerPtpInit(devNum,ptr1,ptr2,ptr3,ptr4,cpuCode);         \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChPtpManagerPtpInit: %s is NULL pointer expected GT_BAD_PTR",    \
        #paramPtr);                                                             \
}

/**
* @internal prvCpssDxChPtpManagerPtpInitDefaultValues function
* @endinternal
*
* @brief   This routine set default values to global PTP-related settings.
*
* @param[in] outputInterfaceConfPtr   - (pointer to) clock output interface configuration.
* @param[in] ptpOverEthernetConfPtr   - (pointer to) PTP over Ethernet configuration.
* @param[in] ptpOverIpUdpConfPtr      - (pointer to) PTP over IP/UDP configuration.
* @param[in] ptpTsTagGlobalConfPtr    - (pointer to) PTP timestamp tag global configuration.
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets.
*/
GT_VOID prvCpssDxChPtpManagerPtpInitDefaultValues
(
    IN CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC   *outputInterfaceConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC      *ptpOverEthernetConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC        *ptpOverIpUdpConfPtr,
    IN CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC              *ptpTsTagGlobalConfPtr,
    IN CPSS_NET_RX_CPU_CODE_ENT                         *cpuCodePtr
)
{
    cpssOsBzero((GT_VOID*) outputInterfaceConfPtr, sizeof(CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC));
    cpssOsBzero((GT_VOID*) ptpOverEthernetConfPtr, sizeof(CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC));
    cpssOsBzero((GT_VOID*) ptpOverIpUdpConfPtr, sizeof(CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC));
    cpssOsBzero((GT_VOID*) ptpTsTagGlobalConfPtr, sizeof(CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC));
    *cpuCodePtr = CPSS_NET_FIRST_USER_DEFINED_E;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerPtpInit
(
    IN GT_U8                                            devNum,
    IN CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC   *outputInterfaceConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC      *ptpOverEthernetConfPtr,
    IN CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC        *ptpOverIpUdpConfPtr,
    IN CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC              *ptpTsTagGlobalConfPtr,
    IN CPSS_NET_RX_CPU_CODE_ENT                         cpuCode
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerPtpInit)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC   *outputInterfaceConfPtr;
    CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC   outputInterfaceConf;
    CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC      *ptpOverEthernetConfPtr;
    CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC      ptpOverEthernetConf;
    CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC        *ptpOverIpUdpConfPtr;
    CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC        ptpOverIpUdpConf;
    CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC              *ptpTsTagGlobalConfPtr;
    CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC              ptpTsTagGlobalConf;
    CPSS_NET_RX_CPU_CODE_ENT                         cpuCode;

    outputInterfaceConfPtr  = &outputInterfaceConf;
    ptpOverEthernetConfPtr  = &ptpOverEthernetConf;
    ptpOverIpUdpConfPtr     = &ptpOverIpUdpConf;
    ptpTsTagGlobalConfPtr   = &ptpTsTagGlobalConf;

    cpssOsMemSet(&outputInterfaceConf,0, sizeof(outputInterfaceConf));
    cpssOsMemSet(&ptpOverEthernetConf,0, sizeof(ptpOverEthernetConf));
    cpssOsMemSet(&ptpOverIpUdpConf,0, sizeof(ptpOverIpUdpConf));
    cpssOsMemSet(&ptpTsTagGlobalConf,0, sizeof(ptpTsTagGlobalConf));

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /************************/
        /* create valid manager */
        /* proof of good parameters */
        /************************/

        /* set default setting */
        prvCpssDxChPtpManagerPtpInitDefaultValues(outputInterfaceConfPtr,ptpOverEthernetConfPtr,ptpOverIpUdpConfPtr,ptpTsTagGlobalConfPtr,&cpuCode);

        st = cpssDxChPtpManagerPtpInit(devNum,outputInterfaceConfPtr,ptpOverEthernetConfPtr,ptpOverIpUdpConfPtr,ptpTsTagGlobalConfPtr,cpuCode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerPtpInit");

        /* output Interface Configuration */
        outputInterfaceConfPtr->outputInterfaceMode = CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PCLK_E;
        outputInterfaceConfPtr->taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        outputInterfaceConfPtr->nanoSeconds = 3;

        /* ptp Over Ethernet Configuration */
        ptpOverEthernetConfPtr->ptpOverEthernetEnable = GT_TRUE;
        ptpOverEthernetConfPtr->etherType0value = 0x8847;
        ptpOverEthernetConfPtr->etherType1value = 0x8848;

        /* ptp Over Udp Configuration */
        ptpOverIpUdpConfPtr->ptpOverIpUdpEnable = GT_TRUE;
        ptpOverIpUdpConfPtr->udpPort0value = 0x8765;
        ptpOverIpUdpConfPtr->udpPort1value = 0x1234;
        cpuCode = CPSS_NET_FIRST_USER_DEFINED_E;

        /* PTP timestamp tag global configuration */
        ptpTsTagGlobalConfPtr->tsTagParseEnable = GT_TRUE;
        ptpTsTagGlobalConfPtr->tsTagEtherType = 0xaabb;

        st = cpssDxChPtpManagerPtpInit(devNum,outputInterfaceConfPtr,ptpOverEthernetConfPtr,ptpOverIpUdpConfPtr,ptpTsTagGlobalConfPtr,cpuCode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerPtpInit");

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPtpInit(
            outputInterfaceConf.taiNumber,
            CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPtpInit(
            outputInterfaceConf.outputInterfaceMode,
            CPSS_DXCH_PTP_TAI_OUTPUT_INTERFACE_MODE_PPS_MASTER_E+1);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPtpInit(
            outputInterfaceConf.nanoSeconds,
            0xFFFFFFFF);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPtpInit(
            ptpOverEthernetConf.etherType0value,
            0xFFFFF);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPtpInit(
            ptpOverEthernetConf.etherType1value,
            0xFFFFF);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPtpInit(
            ptpOverIpUdpConf.udpPort0value,
            0xFFFFF);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPtpInit(
            ptpOverIpUdpConf.udpPort1value,
            0xFFFFF);


        /* check NULL pointers */
        FAIL_NULL_PTR_cpssDxChPtpManagerPtpInit(outputInterfaceConfPtr/*checked for NULL*/,
        outputInterfaceConfPtr,ptpOverEthernetConfPtr,ptpOverIpUdpConfPtr,ptpTsTagGlobalConfPtr);

        FAIL_NULL_PTR_cpssDxChPtpManagerPtpInit(ptpOverEthernetConfPtr/*checked for NULL*/,
        outputInterfaceConfPtr,ptpOverEthernetConfPtr,ptpOverIpUdpConfPtr,ptpTsTagGlobalConfPtr);

        FAIL_NULL_PTR_cpssDxChPtpManagerPtpInit(ptpOverIpUdpConfPtr/*checked for NULL*/,
        outputInterfaceConfPtr,ptpOverEthernetConfPtr,ptpOverIpUdpConfPtr,ptpTsTagGlobalConfPtr);

        FAIL_NULL_PTR_cpssDxChPtpManagerPtpInit(ptpTsTagGlobalConfPtr/*checked for NULL*/,
        outputInterfaceConfPtr,ptpOverEthernetConfPtr,ptpOverIpUdpConfPtr,ptpTsTagGlobalConfPtr);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        /* set default setting */
        prvCpssDxChPtpManagerPtpInitDefaultValues(outputInterfaceConfPtr,ptpOverEthernetConfPtr,ptpOverIpUdpConfPtr,ptpTsTagGlobalConfPtr,&cpuCode);

        st = cpssDxChPtpManagerPtpInit(devNum,outputInterfaceConfPtr,ptpOverEthernetConfPtr,ptpOverIpUdpConfPtr,ptpTsTagGlobalConfPtr,cpuCode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerPtpInit");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerInputPulseSet(      \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerInputPulseSet(devNum,taiNumber,inputMode);           \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerInputPulseSet: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerInputPulseSet
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT   inputMode
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerInputPulseSet)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber;
    CPSS_DXCH_PTP_TAI_PULSE_IN_MODE_ENT              inputMode;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        inputMode = CPSS_DXCH_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E;
        st = cpssDxChPtpManagerInputPulseSet(devNum,taiNumber,inputMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerInputPulseSet");

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerInputPulseSet(
            taiNumber,
            CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerInputPulseSet(
            inputMode,
            CPSS_DXCH_PTP_TAI_PULSE_IN_DISABLED_E);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerInputPulseSet(devNum,taiNumber,inputMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerInputPulseSet");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerMasterPpsActivate(  \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerMasterPpsActivate(devNum,taiNumber,phaseValue,       \
                                             seconds,nanoSeconds);              \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerMasterPpsActivate: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerMasterPpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerMasterPpsActivate)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber;
    GT_U32                                           phaseValue;
    GT_U32                                           seconds;
    GT_U32                                           nanoSeconds;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        phaseValue = 0x12345678;
        seconds = 3;
        nanoSeconds = 0xFFFFFF;
        st = cpssDxChPtpManagerMasterPpsActivate(devNum,taiNumber,phaseValue,seconds,nanoSeconds);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerMasterPpsActivate");

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerMasterPpsActivate(
            taiNumber,
            CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerMasterPpsActivate(
            seconds,
            4);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerMasterPpsActivate(
            nanoSeconds,
            0xFFFFFFFF);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerMasterPpsActivate(devNum,taiNumber,phaseValue,seconds,nanoSeconds);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerMasterPpsActivate");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerSlavePpsActivate(   \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerSlavePpsActivate(devNum,taiNumber,phaseValue,        \
                                             seconds,nanoSeconds);              \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerSlavePpsActivate: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerSlavePpsActivate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  GT_U32                                phaseValue,
    IN  GT_U32                                seconds,
    IN  GT_U32                                nanoSeconds
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerSlavePpsActivate)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber;
    GT_U32                                           phaseValue;
    GT_U32                                           seconds;
    GT_U32                                           nanoSeconds;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        phaseValue = 0x12345678;
        seconds = 3;
        nanoSeconds = 0xFFFFFF;
        st = cpssDxChPtpManagerSlavePpsActivate(devNum,taiNumber,phaseValue,seconds,nanoSeconds);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerSlavePpsActivate");

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerSlavePpsActivate(
            taiNumber,
            CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerSlavePpsActivate(
            seconds,
            4);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerSlavePpsActivate(
            nanoSeconds,
            0xFFFFFFFF);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerSlavePpsActivate(devNum,taiNumber,phaseValue,seconds,nanoSeconds);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerSlavePpsActivate");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortAllocateTai(   \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerPortAllocateTai(devNum,portNum,taiNumber,            \
                                           egressPortBindMode);                 \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerPortAllocateTai: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerPortAllocateTai
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT               taiNumber,
    IN  CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT    egressPortBindMode
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerPortAllocateTai)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    GT_PHYSICAL_PORT_NUM                             portNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber;
    CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_ENT          egressPortBindMode;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        portNum = 12;
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        egressPortBindMode = CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_INGRESS_E;
        st = cpssDxChPtpManagerPortAllocateTai(devNum,portNum,taiNumber,egressPortBindMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerPortAllocateTai");

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortAllocateTai(
            portNum,
            0xFFFFFFFF);

        if (!(PRV_CPSS_DXCH_IS_IRONMAN_L_MAC(devNum)))
        {
            FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortAllocateTai(
                taiNumber,
                CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);

            FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortAllocateTai(
                egressPortBindMode,
                CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E+1);
        }
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerPortAllocateTai(devNum,portNum,taiNumber,egressPortBindMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerPortAllocateTai");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerCaptureTodValueSet( \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerCaptureTodValueSet(devNum,taiNumber);                \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerCaptureTodValueSet: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerCaptureTodValueSet
(
    IN GT_U8                                        devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT                 taiNumber
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerCaptureTodValueSet)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber,taiNumberEnd;
    CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC      todValueEntry0;
    CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC      todValueEntry1;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        /* read to invalidate privious entries (COR) */
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
        st = cpssDxChPtpManagerTodValueGet(devNum,taiNumber,&todValueEntry0,&todValueEntry1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerTodValueGet");

        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_1_E;
        st = cpssDxChPtpManagerTodValueGet(devNum,taiNumber,&todValueEntry0,&todValueEntry1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerTodValueGet");

        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        st = cpssDxChPtpManagerCaptureTodValueSet(devNum,taiNumber); /* capture to entry0 */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerCaptureTodValueSet");
        st = cpssDxChPtpManagerCaptureTodValueSet(devNum,taiNumber); /* capture to entry1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerCaptureTodValueSet");

        taiNumberEnd = (PRV_CPSS_DXCH_IS_IRONMAN_L_MAC(devNum)) ? CPSS_DXCH_PTP_TAI_NUMBER_4_E : CPSS_DXCH_PTP_TAI_NUMBER_1_E;
        for (taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E; taiNumber <= taiNumberEnd; taiNumber++)
        {
            st = cpssDxChPtpManagerTodValueGet(devNum,taiNumber,&todValueEntry0,&todValueEntry1);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerTodValueGet");
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,todValueEntry0.todValueIsValid);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,todValueEntry1.todValueIsValid);
        }

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerCaptureTodValueSet(
            taiNumber,
            CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);
    }

    taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerCaptureTodValueSet(devNum,taiNumber);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerCaptureTodValueSet");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTodValueSet(        \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerTodValueSet(devNum,taiNumber,todValuePtr,            \
                                       capturePrevTodValueEnable);              \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerTodValueSet: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChPtpManagerTodValueSet(                            \
    paramPtr, ptr1                                                              \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChPtpManagerTodValueSet(devNum,taiNumber,ptr1,                   \
                                       capturePrevTodValueEnable);              \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChPtpManagerTodValueSet: %s is NULL pointer expected GT_BAD_PTR",\
        #paramPtr);                                                             \
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerTodValueSet
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN CPSS_DXCH_PTP_TOD_COUNT_STC            *todValuePtr,
    IN GT_BOOL                                capturePrevTodValueEnable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerTodValueSet)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber,taiNumberEnd;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      todValue;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      *todValuePtr;
    GT_BOOL                                          capturePrevTodValueEnable;
    CPSS_DXCH_PTP_TAI_ID_STC                         taiId;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      todValueGet;

    todValuePtr  = &todValue;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        todValue.seconds.l[0] = 2;
        todValue.seconds.l[1] = 3;
        todValue.nanoSeconds = 456;
        todValue.fracNanoSeconds = 789;
        capturePrevTodValueEnable = GT_TRUE;
        st = cpssDxChPtpManagerTodValueSet(devNum,taiNumber,&todValue,capturePrevTodValueEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerTodValueSet");

        taiNumberEnd = (PRV_CPSS_DXCH_IS_IRONMAN_L_MAC(devNum)) ? CPSS_DXCH_PTP_TAI_NUMBER_4_E : CPSS_DXCH_PTP_TAI_NUMBER_1_E;
        for (taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E; taiNumber <= taiNumberEnd; taiNumber++)
        {
            taiId.taiNumber = taiNumber;
            st = cpssDxChPtpTaiTodGet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,&todValueGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpTaiTodGet");

            UTF_VERIFY_EQUAL0_PARAM_MAC(todValue.seconds.l[0],todValueGet.seconds.l[0]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(todValue.seconds.l[1],todValueGet.seconds.l[1]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(todValue.nanoSeconds,todValueGet.nanoSeconds);
            UTF_VERIFY_EQUAL0_PARAM_MAC(todValue.fracNanoSeconds,todValueGet.fracNanoSeconds);
        }

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTodValueSet(
            taiNumber,
            CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);

        /* check NULL pointers */
        FAIL_NULL_PTR_cpssDxChPtpManagerTodValueSet(todValuePtr/*checked for NULL*/,
        todValuePtr);
    }

    taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerTodValueSet(devNum,taiNumber,&todValue,capturePrevTodValueEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerTodValueSet");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTodValueFrequencyUpdate( \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerTodValueFrequencyUpdate(devNum,taiNumber,updateValue,\
                                       capturePrevTodValueEnable);              \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerTodValueFrequencyUpdate: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerTodValueFrequencyUpdate
(
    IN GT_U8                                  devNum,
    IN CPSS_DXCH_PTP_TAI_NUMBER_ENT           taiNumber,
    IN GT_U32                                 updateValue,
    IN GT_BOOL                                capturePrevTodValueEnable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerTodValueFrequencyUpdate)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber,taiNumberEnd;
    GT_U32                                           updateValue;
    GT_BOOL                                          capturePrevTodValueEnable;
    CPSS_DXCH_PTP_TAI_ID_STC                         taiId;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      todValueGet;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        taiNumberEnd = (PRV_CPSS_DXCH_IS_IRONMAN_L_MAC(devNum)) ? CPSS_DXCH_PTP_TAI_NUMBER_4_E : CPSS_DXCH_PTP_TAI_NUMBER_1_E;

        /*** set valid values ***/

        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        updateValue = 123;
        capturePrevTodValueEnable = GT_TRUE;
        st = cpssDxChPtpManagerTodValueFrequencyUpdate(devNum,taiNumber,updateValue,capturePrevTodValueEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerTodValueFrequencyUpdate");

        /* trigger update*/
        for (taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E; taiNumber <= taiNumberEnd; taiNumber++)
        {
            taiId.taiNumber = taiNumber;
            st = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpTodCounterFunctionTriggerSet");
        }

        for (taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E; taiNumber <= taiNumberEnd; taiNumber++)
        {
            taiId.taiNumber = taiNumber;
            st = cpssDxChPtpTaiTodGet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,&todValueGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpTaiTodGet");

            UTF_VERIFY_EQUAL0_PARAM_MAC(updateValue,todValueGet.fracNanoSeconds);
        }

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTodValueFrequencyUpdate(
            taiNumber,
            CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);
    }

    taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerTodValueFrequencyUpdate(devNum,taiNumber,updateValue,capturePrevTodValueEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerTodValueFrequencyUpdate");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTodValueUpdate(     \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerTodValueUpdate(devNum,taiNumber,updateValuePtr,      \
                                         function,gracefulStep,capturePrevTodValueEnable); \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerTodValueUpdate: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChPtpManagerTodValueUpdate(                         \
    paramPtr, ptr1                                                              \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChPtpManagerTodValueUpdate(devNum,taiNumber,ptr1,                \
                                          function,gracefulStep,capturePrevTodValueEnable);  \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChPtpManagerTodValueUpdate: %s is NULL pointer expected GT_BAD_PTR",\
        #paramPtr);                                                             \
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerTodValueUpdate
(
    IN  GT_U8                                 devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT          taiNumber,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC           *updateValuePtr,
    IN  CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT    function,
    IN  GT_U32                                gracefulStep,
    IN  GT_BOOL                               capturePrevTodValueEnable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerTodValueUpdate)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber,taiNumberEnd;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      updateValue;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      *updateValuePtr;
    CPSS_DXCH_PTP_TOD_COUNTER_FUNC_ENT               function;
    GT_U32                                           gracefulStep;
    GT_BOOL                                          capturePrevTodValueEnable;
    CPSS_DXCH_PTP_TAI_ID_STC                         taiId;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      todValueGet;

    updateValuePtr  = &updateValue;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        taiNumberEnd = (PRV_CPSS_DXCH_IS_IRONMAN_L_MAC(devNum)) ? CPSS_DXCH_PTP_TAI_NUMBER_4_E : CPSS_DXCH_PTP_TAI_NUMBER_1_E;

        /*** set valid values ***/

        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        updateValue.seconds.l[0] = 12;
        updateValue.seconds.l[1] = 13;
        updateValue.nanoSeconds = 456000;
        updateValue.fracNanoSeconds = 789111;
        capturePrevTodValueEnable = GT_TRUE;
        function = CPSS_DXCH_PTP_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E;
        gracefulStep = 1;
        st = cpssDxChPtpManagerTodValueUpdate(devNum,taiNumber,&updateValue,function,gracefulStep,capturePrevTodValueEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerTodValueUpdate");

        /* trigger update*/
        for (taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E; taiNumber <= taiNumberEnd; taiNumber++)
        {
            taiId.taiNumber = taiNumber;
            st = cpssDxChPtpTodCounterFunctionTriggerSet(devNum,0 /* not relevant */,&taiId);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpTodCounterFunctionTriggerSet");
        }

        for (taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E; taiNumber <= taiNumberEnd; taiNumber++)
        {
            taiId.taiNumber = taiNumber;
            st = cpssDxChPtpTaiTodGet(devNum,&taiId,CPSS_DXCH_PTP_TAI_TOD_TYPE_LOAD_VALUE_E,&todValueGet);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpTaiTodGet");

            UTF_VERIFY_EQUAL0_PARAM_MAC(updateValue.seconds.l[0],todValueGet.seconds.l[0]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(updateValue.seconds.l[1],todValueGet.seconds.l[1]);
            UTF_VERIFY_EQUAL0_PARAM_MAC(updateValue.nanoSeconds,todValueGet.nanoSeconds);
            UTF_VERIFY_EQUAL0_PARAM_MAC(updateValue.fracNanoSeconds,todValueGet.fracNanoSeconds);
        }

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTodValueUpdate(
            taiNumber,
            CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTodValueUpdate(
            function,
            CPSS_DXCH_PTP_TOD_COUNTER_FUNC_UPDATE_E);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTodValueUpdate(
            gracefulStep,
            0xFFFF);

        /* check NULL pointers */
        FAIL_NULL_PTR_cpssDxChPtpManagerTodValueUpdate(updateValuePtr/*checked for NULL*/,
        updateValuePtr);

    }

    taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerTodValueUpdate(devNum,taiNumber,&updateValue,function,gracefulStep,capturePrevTodValueEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerTodValueUpdate");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTodValueGet(        \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerTodValueGet(devNum,taiNumber,todValueEntry0Ptr,      \
                                       todValueEntry1Ptr);                      \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerTodValueGet: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChPtpManagerTodValueGet(                            \
    paramPtr, ptr1,ptr2                                                         \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChPtpManagerTodValueGet(devNum,taiNumber,ptr1,ptr2);             \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChPtpManagerTodValueGet: %s is NULL pointer expected GT_BAD_PTR",\
        #paramPtr);                                                             \
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerTodValueGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry0Ptr,
    OUT CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC *todValueEntry1Ptr
);
*/

UTF_TEST_CASE_MAC(cpssDxChPtpManagerTodValueGet)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber,taiNumberEnd;
    CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC      todValueEntry0;
    CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC      *todValueEntry0Ptr;
    CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC      todValueEntry1;
    CPSS_DXCH_PTP_MANAGER_CAPTURE_TOD_VALUE_STC      *todValueEntry1Ptr;

    todValueEntry0Ptr  = &todValueEntry0;
    todValueEntry1Ptr  = &todValueEntry1;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        taiNumberEnd = (PRV_CPSS_DXCH_IS_IRONMAN_L_MAC(devNum)) ? CPSS_DXCH_PTP_TAI_NUMBER_4_E : CPSS_DXCH_PTP_TAI_NUMBER_1_E;

        /*** set valid values ***/

        /* read twice to invalidate privious entries (COR) */
        for (taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E; taiNumber <= taiNumberEnd; taiNumber++)
        {
            taiNumber = taiNumber;
            st = cpssDxChPtpManagerTodValueGet(devNum,taiNumber,&todValueEntry0,&todValueEntry1);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerTodValueGet");
            st = cpssDxChPtpManagerTodValueGet(devNum,taiNumber,&todValueEntry0,&todValueEntry1);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerTodValueGet");
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, todValueEntry0.todValueIsValid);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, todValueEntry1.todValueIsValid);
        }

        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        st = cpssDxChPtpManagerCaptureTodValueSet(devNum,taiNumber); /* capture to entry0 */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerCaptureTodValueSet");
        st = cpssDxChPtpManagerCaptureTodValueSet(devNum,taiNumber); /* capture to entry1 */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerCaptureTodValueSet");

        for (taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E; taiNumber <= taiNumberEnd; taiNumber++)
        {
            taiNumber = taiNumber;
            st = cpssDxChPtpManagerTodValueGet(devNum,taiNumber,&todValueEntry0,&todValueEntry1);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerTodValueGet");
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,todValueEntry0.todValueIsValid);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,todValueEntry1.todValueIsValid);
        }

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTodValueGet(
            taiNumber,
            CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);

        /* check NULL pointers */
        FAIL_NULL_PTR_cpssDxChPtpManagerTodValueGet(todValueEntry0Ptr/*checked for NULL*/,
        todValueEntry0Ptr,todValueEntry1Ptr)

        /* check NULL pointers */
        FAIL_NULL_PTR_cpssDxChPtpManagerTodValueGet(todValueEntry1Ptr/*checked for NULL*/,
        todValueEntry0Ptr,todValueEntry1Ptr)
    }

    taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerTodValueGet(devNum,taiNumber,&todValueEntry0,&todValueEntry1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerTodValueGet");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTriggerGenerate(    \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerTriggerGenerate(devNum,taiNumber,extPulseWidth,      \
                                       triggerTimePtr,maskEnable,maskValuePtr); \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerTriggerGenerate: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChPtpManagerTriggerGenerate(                        \
    paramPtr, ptr1,ptr2                                                         \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChPtpManagerTriggerGenerate(devNum,taiNumber,extPulseWidth,      \
                                           ptr1,maskEnable,ptr2);               \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChPtpManagerTriggerGenerate: %s is NULL pointer expected GT_BAD_PTR",\
        #paramPtr);                                                             \
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerTriggerGenerate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  extPulseWidth,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *triggerTimePtr,
    IN  GT_BOOL                                 maskEnable,
    IN  CPSS_DXCH_PTP_TOD_COUNT_STC             *maskValuePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerTriggerGenerate)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber;
    GT_U32                                           extPulseWidth;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      triggerTime;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      *triggerTimePtr;
    GT_BOOL                                          maskEnable;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      maskValue;
    CPSS_DXCH_PTP_TOD_COUNT_STC                      *maskValuePtr;

    triggerTimePtr  = &triggerTime;
    maskValuePtr    = &maskValue;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        extPulseWidth = 0xFFFF;
        triggerTime.nanoSeconds = 22;
        triggerTime.seconds.l[0] = 1;
        triggerTime.seconds.l[0] = 2;
        triggerTime.fracNanoSeconds = 333;
        maskEnable = GT_TRUE;
        maskValue.nanoSeconds = 0xFF;
        maskValue.seconds.l[0] = 0xF;
        maskValue.seconds.l[1] = 0xF;
        maskValue.fracNanoSeconds = 0xFFFF;
        st = cpssDxChPtpManagerTriggerGenerate(devNum,taiNumber,extPulseWidth,&triggerTime,maskEnable,&maskValue);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerTriggerGenerate");

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTriggerGenerate(
            taiNumber,
            CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);

        /* check NULL pointers */
        FAIL_NULL_PTR_cpssDxChPtpManagerTriggerGenerate(triggerTimePtr/*checked for NULL*/,
        triggerTimePtr,maskValuePtr);

        /* check NULL pointers */
        FAIL_NULL_PTR_cpssDxChPtpManagerTriggerGenerate(maskValuePtr/*checked for NULL*/,
        triggerTimePtr,maskValuePtr);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerTriggerGenerate(devNum,taiNumber,extPulseWidth,&triggerTime,maskEnable,&maskValue);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerTriggerGenerate");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerGlobalPtpDomainSet( \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerGlobalPtpDomainSet(devNum,domainConfPtr);            \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerGlobalPtpDomainSet: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChPtpManagerGlobalPtpDomainSet(                     \
    paramPtr, ptr1                                                              \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChPtpManagerGlobalPtpDomainSet(devNum,ptr1);                     \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChPtpManagerGlobalPtpDomainSet: %s is NULL pointer expected GT_BAD_PTR",\
        #paramPtr);                                                             \
}
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerGlobalPtpDomainSet
(
    IN GT_U8                                           devNum,
    IN CPSS_DXCH_PTP_MANAGER_DOMAIN_STC                *domainConfPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerGlobalPtpDomainSet)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC                 domainConf;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC                 *domainConfPtr;

    domainConfPtr  = &domainConf;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        domainConf.domainIndex = 1;
        domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
        domainConf.v2DomainId = 2;
        st = cpssDxChPtpManagerGlobalPtpDomainSet(devNum,&domainConf);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerGlobalPtpDomainSet");

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerGlobalPtpDomainSet(
            domainConf.domainIndex,
            4);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerGlobalPtpDomainSet(
            domainConf.domainMode,
            CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_DISABLE_E);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerGlobalPtpDomainSet(
            domainConf.v2DomainId,
            0xFFFFFFFF);

        /* check NULL pointers */
        FAIL_NULL_PTR_cpssDxChPtpManagerGlobalPtpDomainSet(domainConfPtr/*checked for NULL*/,
        domainConfPtr);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerGlobalPtpDomainSet(devNum,&domainConf);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerGlobalPtpDomainSet");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortPacketActionsConfig( \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerPortPacketActionsConfig(devNum,portNum,taiNumber,domainIndex, \
                                            domainEntryPtr,messageType,action); \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerPortPacketActionsConfig: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/* check NULL pointer param , expected to fail */
#define FAIL_NULL_PTR_cpssDxChPtpManagerPortPacketActionsConfig(                \
    paramPtr, ptr1                                                              \
)                                                                               \
{                                                                               \
    GT_VOID*  origPtr = (GT_VOID*)paramPtr;                                     \
                                                                                \
    paramPtr = NULL;                                                            \
                                                                                \
    st = cpssDxChPtpManagerPortPacketActionsConfig(devNum,portNum,taiNumber,    \
                                          domainIndex,ptr1,messageType,action); \
    /* restore orig value */                                                    \
    paramPtr = origPtr;                                                         \
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,                                \
        "cpssDxChPtpManagerPortPacketActionsConfig: %s is NULL pointer expected GT_BAD_PTR",\
        #paramPtr);                                                             \
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerPortPacketActionsConfig
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber,
    IN  GT_U32                                  domainIndex,
    IN  CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   *domainEntryPtr,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT  messageType,
    IN  CPSS_DXCH_PTP_TS_ACTION_ENT             action
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerPortPacketActionsConfig)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    GT_PHYSICAL_PORT_NUM                             portNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT                     taiNumber;
    GT_U32                                           domainIndex;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC            domainEntry;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC            *domainEntryPtr;
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT           messageType;
    CPSS_DXCH_PTP_TS_ACTION_ENT                      action;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    domainEntryPtr  = &domainEntry;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        portNum = 10;
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
        domainIndex = 3;
        cpssOsMemSet(&domainEntry,0, sizeof(domainEntry));
        messageType = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E;
        action = CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E;
        st = cpssDxChPtpManagerPortPacketActionsConfig(devNum,portNum,taiNumber,domainIndex,&domainEntry,messageType,action);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerPortPacketActionsConfig");

        /*** check not valid values ***/
        if (!(PRV_CPSS_DXCH_IS_IRONMAN_L_MAC(devNum)))
        {
            FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortPacketActionsConfig(
                taiNumber,
                CPSS_DXCH_PTP_TAI_NUMBER_ALL_E+1);
        }

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortPacketActionsConfig(
            domainIndex,
            5);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortPacketActionsConfig(
            messageType,
            CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_LAST_E);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortPacketActionsConfig(
            action,
            CPSS_DXCH_PTP_TS_ACTION_ALL_E+1);

        /* check NULL pointers */
        FAIL_NULL_PTR_cpssDxChPtpManagerPortPacketActionsConfig(domainEntryPtr/*checked for NULL*/,
        domainEntryPtr);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerPortPacketActionsConfig(devNum,portNum,taiNumber,domainIndex,&domainEntry,messageType,action);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerPortCommandAssignment");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTsQueueConfig( \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerTsQueueConfig(devNum,messageType,queueNum);          \
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerTsQueueConfig: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerTsQueueConfig
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT      messageType,
    IN  GT_U32                                      queueNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerTsQueueConfig)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT           messageType;
    GT_U32                                           queueNum;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        messageType = CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E;
        queueNum = 1;
        st = cpssDxChPtpManagerTsQueueConfig(devNum,messageType,queueNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerPortPacketActionsConfig");

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTsQueueConfig(
            messageType,
            CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_LAST_E);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerTsQueueConfig(
            queueNum,
            2);
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerTsQueueConfig(devNum,messageType,queueNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerTsQueueConfig");
    }
}

/* check specific value of numeric field , and high values (31,32 bits value) :
expected to fail */
#define FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortTimestampingModeConfig(   \
    fieldName,                                                                  \
    testedField_Value                                                           \
)                                                                               \
{                                                                               \
    GT_U32  origValue = fieldName;                                              \
                                                                                \
    fieldName = (testedField_Value);                                            \
    st = cpssDxChPtpManagerPortTimestampingModeConfig(devNum,portNum,tsMode,operationMode);\
    /* restore orig value */                                                    \
    fieldName = origValue;                                                      \
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,                                 \
        "cpssDxChPtpManagerPortTimestampingModeConfig: %s expected NOT to be GT_OK on value [%d]", \
        #fieldName,                                                             \
        (testedField_Value));                                                   \
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManagerPortTimestampingModeConfig
(
    IN  GT_U8                                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                            portNum,
    IN  CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT    tsMode,
    IN  CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT        operationMode
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManagerPortTimestampingModeConfig)
{

    GT_STATUS                                        st = GT_OK;
    GT_U8                                            devNum;
    GT_PHYSICAL_PORT_NUM                             portNum;
    CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT     tsMode;
    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT         operationMode;

    /* this feature is on eArch devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        /*** set valid values ***/

        portNum = 2;
        tsMode = CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E;
        operationMode = CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_STEP_IN_SWITCH_E;
        st = cpssDxChPtpManagerPortTimestampingModeConfig(devNum,portNum,tsMode,operationMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st,"cpssDxChPtpManagerPortTimestampingModeConfig");

        /*** check not valid values ***/

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortTimestampingModeConfig(
            portNum,
            0xFFFFFFFF);

        FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortTimestampingModeConfig(
            tsMode,
            CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_AM_E+1);

        /*FAIL_SPECIFIC_VALUE_NUMERIC_FIELD_cpssDxChPtpManagerPortTimestampingModeConfig(
            operationMode,
            CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_IN_MAC_E+1);*/ /* roni TBD */
    }

    /* 2. For not active devices check that function returns non GT_NOT_APPLICABLE_DEVICE.*/
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_CPSS_PP_ALL_SIP6_10_CNS ) ;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManagerPortTimestampingModeConfig(devNum,portNum,tsMode,operationMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,"cpssDxChPtpManagerPortTimestampingModeConfig");
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManager8021AsMsgSelectionEnableSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_BOOL                                     followUpEnable,
    IN  GT_BOOL                                     syncEnable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManager8021AsMsgSelectionEnableSet)
{
    GT_STATUS st;
    GT_U8 devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber;
    GT_BOOL                                 followUpEnable,followUpEnableGet;
    GT_BOOL                                 syncEnable,syncEnableGet;

     /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_IRONMAN_L_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for(taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E; taiNumber < CPSS_DXCH_PTP_TAI_NUMBER_4_E; taiNumber++)
        {
             /*
                1.1. For all active devices go over all tais.
                Expected: GT_OK.
            */
            followUpEnable = GT_TRUE;
            syncEnable = GT_TRUE;
            st = cpssDxChPtpManager8021AsMsgSelectionEnableSet(devNum,taiNumber,followUpEnable,syncEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssDxChPtpManager8021AsMsgSelectionEnableGet with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChPtpManager8021AsMsgSelectionEnableGet(devNum,taiNumber,&followUpEnableGet,&syncEnableGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(followUpEnable,followUpEnableGet,
                           "got another followUpEnable then was set: %d", devNum);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(syncEnable,syncEnableGet,
                           "got another syncEnable then was set: %d", devNum);
        }

         /*
            1.3. Call api with wrong taiNumber.
            Expected: GT_BAD_PARAM.
        */
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_3_E;
        st = cpssDxChPtpManager8021AsMsgSelectionEnableSet(devNum,taiNumber+1,followUpEnable,syncEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum);
    }

    followUpEnable = GT_TRUE;
    syncEnable = GT_TRUE;
    taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_3_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_IRONMAN_L_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManager8021AsMsgSelectionEnableSet(devNum,taiNumber,followUpEnable,syncEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPtpManager8021AsMsgSelectionEnableSet(devNum,taiNumber,followUpEnable,syncEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManager8021AsMsgSelectionEnableGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT GT_BOOL                                     *followUpEnablePtr,
    OUT GT_BOOL                                     *syncEnablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManager8021AsMsgSelectionEnableGet)
{
    GT_STATUS st;
    GT_U8 devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber;
    GT_BOOL                                 followUpEnable;
    GT_BOOL                                 syncEnable;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_IRONMAN_L_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

        /*
            1.1 Call with non-null followUpEnablePtr,syncEnablePtr
            Expected: GT_OK.
        */
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
        st = cpssDxChPtpManager8021AsMsgSelectionEnableGet(devNum,taiNumber,&followUpEnable,&syncEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call api with wrong followUpEnablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPtpManager8021AsMsgSelectionEnableGet(devNum,taiNumber,NULL,&syncEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

        /*
            1.2 Call api with wrong syncEnablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPtpManager8021AsMsgSelectionEnableGet(devNum,taiNumber,&followUpEnable,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);
    }

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_IRONMAN_L_E );

    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManager8021AsMsgSelectionEnableGet(devNum,taiNumber,&followUpEnable,&syncEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPtpManager8021AsMsgSelectionEnableGet(devNum,taiNumber,&followUpEnable,&syncEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    IN  GT_U32                                      offset
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet)
{
    GT_STATUS st;
    GT_U8 devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber;
    GT_U32                                  offset,offsetGet;

     /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_IRONMAN_L_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {
        for(taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E; taiNumber < CPSS_DXCH_PTP_TAI_NUMBER_4_E; taiNumber++)
        {
             /*
                1.1. For all active devices go over all tais.
                Expected: GT_OK.
            */
            offset = 0x123;
            st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet(devNum,taiNumber,offset);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet(devNum,taiNumber,&offsetGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(offset,offsetGet,
                           "got another offset then was set: %d", devNum);

            /*
                1.1. For all active devices go over all tais.
                Expected: GT_OK.
            */
            offset = 0xFFFFFFFF;
            st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet(devNum,taiNumber,offset);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /*
                1.2. Call cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet(devNum,taiNumber,&offsetGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(offset,offsetGet,
                           "got another offset then was set: %d", devNum);

        }

         /*
            1.3. Call api with wrong taiNumber.
            Expected: GT_BAD_PARAM.
        */
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_3_E;
        st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet(devNum,taiNumber+1,offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, devNum);
    }

    offset = 0x123;
    taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_3_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_IRONMAN_L_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet(devNum,taiNumber,offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet(devNum,taiNumber,offset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PTP_TAI_NUMBER_ENT                taiNumber,
    OUT GT_U32                                      *offsetPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet)
{
    GT_STATUS st;
    GT_U8 devNum;
    CPSS_DXCH_PTP_TAI_NUMBER_ENT            taiNumber;
    GT_U32                                  offset;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_IRONMAN_L_E );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_TRUE))
    {

        /*
            1.1 Call with non-null offsetPtr
            Expected: GT_OK.
        */
        taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_0_E;
        st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet(devNum,taiNumber,&offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, devNum);

        /*
            1.2 Call api with wrong followUpEnablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet(devNum,taiNumber,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, devNum);

    }

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&devNum, UTF_IRONMAN_L_E );

    /* 2. Go over all non-applicable devices.*/
    while(GT_OK == prvUtfNextDeviceGet(&devNum, GT_FALSE))
    {
        st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet(devNum,taiNumber,&offset);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, devNum);
    }

    /* 3. Call function with out of bound value for device id.*/
    devNum = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet(devNum,taiNumber,&offset);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, devNum);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPtpManager suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPtpManager)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerPtpInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerInputPulseSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerMasterPpsActivate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerSlavePpsActivate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerPortAllocateTai)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerCaptureTodValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerTodValueSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerTodValueFrequencyUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerTodValueUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerTodValueGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerTriggerGenerate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerGlobalPtpDomainSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerPortPacketActionsConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerTsQueueConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManagerPortTimestampingModeConfig)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManager8021AsMsgSelectionEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManager8021AsMsgSelectionEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPtpManager8021AsCumulativeScaledRateOffsetGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPtpManager)


