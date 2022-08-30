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
* @file cpssDxChPhaUT.c
*
* @brief Unit tests for cpssDxChPha.h
*
* @version   31
********************************************************************************
*/
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* includes */
/* #include <cpss/common/cpssTypes.h> */
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/pha/private/prvCpssDxChPha.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/* check if the SW CPU codes are the same (lead to the same HW value) */
static GT_VOID check_cpuCode(
    IN GT_U8                dev,
    IN  CPSS_NET_RX_CPU_CODE_ENT    cpuCode1,
    IN  CPSS_NET_RX_CPU_CODE_ENT    cpuCode2
)
{
    GT_STATUS            st;
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode_1,dsaCpuCode_2;

    if(cpuCode1 == cpuCode2)
    {
        /* already the same value */
        return;
    }

    /* the CPU code can be converted from 'HW' format to other 'SW format'*/
    /* convert SW cpu code to HW cpu code */
    st = prvCpssDxChNetIfCpuToDsaCode(cpuCode1,&dsaCpuCode_1);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    st = prvCpssDxChNetIfCpuToDsaCode(cpuCode2,&dsaCpuCode_2);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    /* those values MUST be the same !!! */
    UTF_VERIFY_EQUAL1_PARAM_MAC(dsaCpuCode_1,dsaCpuCode_2, dev);
}


/* activate the 'cpssDxChPhaThreadIdEntrySet' and call 'Get' and check it is ok */
static GT_VOID check_cpssDxChPhaThreadIdEntrySet_andGet(
    IN GT_U8                dev,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr,
    IN GT_STATUS            rc_expected,
    IN GT_BOOL              use_rc_not_expected
)
{
    GT_STATUS            st;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC commonInfoGet;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT    extTypeGet;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfoGet;

    st = cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr);
    if(use_rc_not_expected == GT_FALSE)
    {
        UTF_VERIFY_EQUAL1_PARAM_MAC(rc_expected, st, dev);
    }
    else
    {
        UTF_VERIFY_NOT_EQUAL1_PARAM_MAC(rc_expected, st, dev);
    }

    if(st != GT_OK)
    {
        return;
    }

    /********************************************************/
    /* check that the Get return the same values as the Set */
    /********************************************************/

    st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfoGet , &extTypeGet , &extInfoGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    UTF_VERIFY_EQUAL1_PARAM_MAC(extType, extTypeGet, dev);

    UTF_VERIFY_EQUAL1_PARAM_MAC(commonInfoPtr->statisticalProcessingFactor, commonInfoGet.statisticalProcessingFactor, dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(commonInfoPtr->busyStallMode,               commonInfoGet.busyStallMode, dev);
    /* the CPU code can be converted from 'HW' format to other 'SW format'*/
    /* convert SW cpu code to HW cpu code */
    check_cpuCode(dev,commonInfoPtr->stallDropCode,commonInfoGet.stallDropCode);

    if(cpssOsMemCmp(extInfoPtr,&extInfoGet,sizeof(extInfoGet)))
    {
        st = GT_BAD_STATE;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
}

/* test common info */
static GT_VOID check_threadCommonInfo(
    IN GT_U8                dev,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
)
{
    GT_STATUS            st;
    GT_U32               phaThreadId;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    origCommonInfo = *commonInfoPtr;

    /* should be OK */
    phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OK , GT_FALSE);

    /* check bad parameters */
    phaThreadId++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_BAD_PARAM , GT_FALSE);

    phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC - 1;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_BAD_PARAM , GT_FALSE);

    phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;

    /* good parameters */
    commonInfoPtr->statisticalProcessingFactor = BIT_8 - 1;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    /* check out of range valid parameters */
    commonInfoPtr->statisticalProcessingFactor++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OUT_OF_RANGE , GT_FALSE);
    commonInfoPtr->statisticalProcessingFactor--;


    /* good parameters */
    commonInfoPtr->busyStallMode        = CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    commonInfoPtr->busyStallMode        = CPSS_DXCH_PHA_BUSY_STALL_MODE_DROP_E;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    commonInfoPtr->busyStallMode        = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    /* check bad parameters */
    commonInfoPtr->busyStallMode        ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_TRUE);

    /*  check bad ENUM values */
    UTF_ENUMS_CHECK_MAC(cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr),
        commonInfoPtr->busyStallMode);

    commonInfoPtr->busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_PUSH_BACK_E;

    for(commonInfoPtr->stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E ;
        commonInfoPtr->stallDropCode <= CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E ;
        commonInfoPtr->stallDropCode += 17)/* iterate on several values */
    {
        check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OK , GT_FALSE);
    }

    /*  check bad ENUM values */
    UTF_ENUMS_CHECK_MAC(cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr),
        commonInfoPtr->stallDropCode);

    commonInfoPtr->stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    *commonInfoPtr = origCommonInfo;
}

/* test info based on : CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC */
static GT_VOID check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC(
    IN GT_U8                dev,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC   *infoPtr
)
{
    infoPtr->IOAM_Trace_Type = BIT_16-1;/* 16 bits value */
    infoPtr->Maximum_Length  =  BIT_8-1;/*  8 bits value */
    infoPtr->Flags           =  BIT_8-1;/*  8 bits value */
    infoPtr->Hop_Lim         =  BIT_8-1;/*  8 bits value */
    infoPtr->node_id         = BIT_24-1;/* 24 bits value */
    infoPtr->Type1           =  BIT_8-1;/*  8 bits value , 'IOAM' */
    infoPtr->IOAM_HDR_len1   =  BIT_8-1;/*  8 bits value */
    infoPtr->Reserved1       =  BIT_8-1;/*  8 bits value */
    infoPtr->Next_Protocol1  =  BIT_8-1;/*  8 bits value */
    infoPtr->Type2           =  BIT_8-1;/*  8 bits value , 'IOAM_E2E' */
    infoPtr->IOAM_HDR_len2   =  BIT_8-1;/*  8 bits value */
    infoPtr->Reserved2       =  BIT_8-1;/*  8 bits value */
    infoPtr->Next_Protocol2  =  BIT_8-1;/*  8 bits value */

    /* check valid parameters */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OK , GT_FALSE);

    /* check common info */
    check_threadCommonInfo(dev, commonInfoPtr , extType , extInfoPtr);

    /* check out of range valid parameters */

    infoPtr->IOAM_Trace_Type++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->IOAM_Trace_Type--;
    infoPtr->Maximum_Length ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Maximum_Length --;
    infoPtr->Flags          ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Flags          --;
    infoPtr->Hop_Lim        ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Hop_Lim        --;
    infoPtr->node_id        ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->node_id        --;
    infoPtr->Type1          ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Type1          --;
    infoPtr->IOAM_HDR_len1  ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->IOAM_HDR_len1  --;
    infoPtr->Reserved1      ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Reserved1      --;
    infoPtr->Next_Protocol1 ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Next_Protocol1 --;
    infoPtr->Type2          ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Type2          --;
    infoPtr->IOAM_HDR_len2  ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->IOAM_HDR_len2  --;
    infoPtr->Reserved2      ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Reserved2      --;
    infoPtr->Next_Protocol2 ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->Next_Protocol2 --;

    /* we must be ok now ! otherwise we may fail cases on other ranges */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr, GT_OK , GT_FALSE);

}

/* test info based on : CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC */
static GT_VOID check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC(
    IN GT_U8                dev,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC   *infoPtr
)
{

    infoPtr->node_id         = BIT_24-1;/* 24 bits value */

    /* check valid parameters */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    /* check common info */
    check_threadCommonInfo(dev, commonInfoPtr , extType , extInfoPtr);

    /* check out of range valid parameters */

    infoPtr->node_id        ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->node_id        --;

    /* we must be ok now ! otherwise we may fail cases on other ranges */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);
}


/* test info based on : CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC */
static GT_VOID check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC(
    IN GT_U8                dev,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC   *infoPtr
)
{
    infoPtr->source_node_id = BIT_12-1;/* 12 bits value */

    /* check valid parameters */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    /* check common info */
    check_threadCommonInfo(dev, commonInfoPtr , extType , extInfoPtr);

    /* check out of range valid parameters */

    infoPtr->source_node_id ++;
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OUT_OF_RANGE , GT_FALSE);

    infoPtr->source_node_id --;

    /* we must be ok now ! otherwise we may fail cases on other ranges */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);
}


/* test info based on : CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC */
static GT_VOID check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC(
    IN GT_U8                dev,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC   *infoPtr
    )
{
    GT_U32 i;

    /* Set IPv6 address */
    for (i = 0; i < 16; i++)
    {
        infoPtr->srcAddr.arIP[i] = BIT_8-1;
    }

    /* check valid parameters */
    check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , commonInfoPtr , extType , extInfoPtr , GT_OK , GT_FALSE);

    /* check common info */
    check_threadCommonInfo(dev, commonInfoPtr , extType , extInfoPtr);
}

extern GT_U32 utfFamilyTypeGet(IN GT_U8 devNum);
/* add additional macro to treat devices like Ironman that not hold dedicated
   family in UTF_PP_FAMILY_BIT_ENT , and uses the UTF_AC5X_E */
#define PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC                             \
    if(PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.phaInfo.numOfPpg == 0) \
    {                                                                    \
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,utfFamilyTypeGet(prvTgfDevNum));         \
    }

/*
GT_STATUS cpssDxChPhaThreadIdEntrySet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    IN CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    IN CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType,
    IN CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaThreadIdEntrySet)
{
    GT_STATUS            st;
    GT_U8                dev;
    GT_U32               phaThreadId;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC commonInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfo;

    GT_U32                          iter;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT   threadType[5] = {CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E,
                                                     CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E,
                                                     CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E,
                                                     CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E,
                                                     CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E};

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
        cpssOsMemSet(&commonInfo,0,sizeof(commonInfo));
        /* must set valid 'drop code' otherwise function will fail! */
        commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
        cpssOsMemSet(&extInfo,0,sizeof(extInfo));

        extType = CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E;
        extInfo.notNeeded = 0x12345678;/* dummy ... not used ... don't care */

        /* check NULL pointers */
        st = cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , NULL , extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , &commonInfo , extType , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* check valid parameters */
        check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId , &commonInfo , extType , &extInfo , GT_OK , GT_FALSE);

        cpssOsMemSet(&extInfo,0,sizeof(extInfo));

        /* check type : IOAM ingress switch ipv4 (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E, &phaThreadId))
        {
           check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC(dev,
                phaThreadId , &commonInfo ,
                CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV4_E , &extInfo ,
                &extInfo.ioamIngressSwitchIpv4);
        }

        /* check type : IOAM ingress switch ipv6 (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E, &phaThreadId))
        {
            check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_INGRESS_SWITCH_STC(dev,
                phaThreadId , &commonInfo ,
                CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E , &extInfo ,
                &extInfo.ioamIngressSwitchIpv6);

            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
        }

        /* check type : IOAM transit switch ipv4 (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_IOAM_INGRESS_SWITCH_IPV6_E, &phaThreadId))
        {
            check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC(dev,
                phaThreadId , &commonInfo ,
                CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV4_E , &extInfo ,
                &extInfo.ioamTransitSwitchIpv4);
        }

        /* check type : IOAM transit switch ipv6 (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E, &phaThreadId))
        {
            check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_IOAM_TRANSIT_SWITCH_STC(dev,
                phaThreadId , &commonInfo ,
                CPSS_DXCH_PHA_THREAD_TYPE_IOAM_TRANSIT_SWITCH_IPV6_E , &extInfo ,
                &extInfo.ioamTransitSwitchIpv6);

            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
        }

        /* check type : Classifier NSH over VXLAN-GPE (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_E, &phaThreadId))
        {
            check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_STC(dev,
                phaThreadId , &commonInfo ,
                CPSS_DXCH_PHA_THREAD_TYPE_CLASSIFIER_NSH_OVER_VXLAN_GPE_E , &extInfo ,
                &extInfo.classifierNshOverVxlanGpe);
        }

        /* check type : SRV6 SOURCE NODE (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E, &phaThreadId))
        {
            check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC(dev,
                phaThreadId , &commonInfo ,
                CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_SEGMENT_E , &extInfo ,
                &extInfo.srv6SrcNode);

            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
        }

        /* check type : PTP 1 Step (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_PTP_PHY_1_STEP_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);

            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
        }

        /* check type : SGT Net Add MSB (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E, &phaThreadId))
        {
            extInfo.sgtNetwork.etherType    = cpssOsRand() & 0xFFFF;    /* 16 Bits */
            extInfo.sgtNetwork.version      = cpssOsRand() & 0xFF;      /* 8  Bits */
            extInfo.sgtNetwork.length       = cpssOsRand() & 0xFFF;     /* 12 Bits */
            extInfo.sgtNetwork.optionType   = cpssOsRand() & 0xFFF;     /* 12 Bits */
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_ADD_MSB_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }

        /* check type : SGT Net Fix (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_FIX_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);

            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
        }

        /* check type : SGT Net Remove (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_REMOVE_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_NETWORK_REMOVE_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }

        /* check type : SGT eDSA Fix */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_FIX_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_FIX_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }

        /* check type : SGT eDSA Remove (run it only if thread is supported)  */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_REMOVE_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_EDSA_REMOVE_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }

        /* check type : SGT GBP Fix IPv4 (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV4_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV4_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }

        /* check type : SGT GBP Fix IPv6 (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV6_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_FIX_IPV6_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }

        /* check type : SGT GBP Remove IPv4 (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV4_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV4_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }

        /* check type : SGT GBP Remove IPv6 (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV6_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SGT_GBP_REMOVE_IPV6_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }

        iter = 0;
        /* check below CC ERSPAN threads types (run it only if thread is supported)
           - CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E
           - CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E
           - CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV4_E
           - CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_DIRECT_IPV6_E
           - CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E
        */
        while((iter < 5) && (GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, threadType[iter], &phaThreadId)))
        {
            cpssOsMemSet(&extInfo,0,sizeof(extInfo));

            extType = threadType[iter];

            /* check common info */
            check_threadCommonInfo(dev, &commonInfo , extType , &extInfo);

            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
            iter++;
        }
        /* check type : VXLAN GBP (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_GBP_SOURCE_GROUP_POLICY_ID_E;

            cpssOsMemSet(&extInfo,0,sizeof(extInfo));

            extInfo.vxlanGbpSourceGroupPolicyId.copyReservedLsb      = 0x12;
            extInfo.vxlanGbpSourceGroupPolicyId.copyReservedMsb      = 0x13;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OUT_OF_RANGE , GT_FALSE);

            extInfo.vxlanGbpSourceGroupPolicyId.copyReservedLsb      = 0x10;
            extInfo.vxlanGbpSourceGroupPolicyId.copyReservedMsb      = PRV_CPSS_SIP_6_10_CHECK_MAC(dev) ? 0x13: 0x12;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OUT_OF_RANGE , GT_FALSE);

            extInfo.vxlanGbpSourceGroupPolicyId.copyReservedLsb      = 0x11;
            extInfo.vxlanGbpSourceGroupPolicyId.copyReservedMsb      = 0x10;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_BAD_PARAM , GT_FALSE);

            extInfo.vxlanGbpSourceGroupPolicyId.copyReservedLsb      = 0x06;
            extInfo.vxlanGbpSourceGroupPolicyId.copyReservedMsb      = 0x11;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
        /* check type : SRv6 Best Effort Tunnel (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SRV6_BEST_EFFORT_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
        /* check type : SRv6 SRH Single Pass With 1 Container (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E, &phaThreadId))
        {
            check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC(dev,
                phaThreadId , &commonInfo ,
                CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_1_CONTAINER_E , &extInfo ,
                &extInfo.srv6SrcNode);

            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
        }
        /* check type : SRv6 SRH Pass 1 With 2/3 Containers (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E, &phaThreadId))
        {
            check_CPSS_DXCH_PHA_THREAD_INFO_TYPE_SRV6_SRC_NODE_STC(dev,
                phaThreadId , &commonInfo ,
                CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E , &extInfo ,
                &extInfo.srv6SrcNode);

            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
        }
        /* check type : SRv6 SRH Pass 2 With 2 Containers (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
        /* check type : SRv6 SRH Pass 2 With 3 Containers (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E;
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
        /* check type : SRv6 SRH With G-SID Containers (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SRV6_END_NODE_COC32_GSID_E;

            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
            extInfo.srv6Coc32GsidCommonPrefix.dipCommonPrefixLength      = 0x40;

            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
        /* check type : IPv4 TTL increment (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_IPV4_TTL_INCREMENT_E;
            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
        /* check type : IPv6 Hop Limit increment (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_IPV6_HOP_LIMIT_INCREMENT_E;
            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
        /* check type : Clear Outgoing Mtag Command (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_CLEAR_OUTGOING_MTAG_COMMAND_E;
            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
        /* check type : Sflow V5 IPv4(run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV4_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV4_E;
            cpssOsMemSet(&extInfo,0,sizeof(extInfo));

            /*agent IP = 41.42.43.44*/
            /*enterprise type = 1, fmt = 4*/
            extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[0] = 41;
            extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[1] = 42;
            extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[2] = 43;
            extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[3] = 44;
            extInfo.sflowV5Mirror.sflowDataFormat = 1<<12 | 4 ;

            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
        /* check type : Sflow V5 IPv6(run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV6_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_IPV6_E;
            cpssOsMemSet(&extInfo,0,sizeof(extInfo));

            /*agent IP = 61.62.63.64*/
            /*enterprise type = 1, fmt = 6*/
            extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[0] = 61;
            extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[1] = 62;
            extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[2] = 63;
            extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[3] = 64;
            extInfo.sflowV5Mirror.sflowDataFormat = 1<<12 | 6 ;

            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
        /* check type : SLS testing (run it only if thread is supported) */
        if(GT_OK == prvCpssDxChPhaThreadValidityCheck(dev, CPSS_DXCH_PHA_THREAD_TYPE_SLS_E, &phaThreadId))
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_SLS_E;
            cpssOsMemSet(&extInfo,0,sizeof(extInfo));
            check_cpssDxChPhaThreadIdEntrySet_andGet(dev, phaThreadId, &commonInfo,
                    extType , &extInfo , GT_OK , GT_FALSE);
        }
    }

    phaThreadId = 1;
    cpssOsMemSet(&commonInfo,0,sizeof(commonInfo));
    /* must set valid 'drop code' otherwise function will fail! */
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    extType = CPSS_DXCH_PHA_THREAD_TYPE_UNUSED_E;
    extInfo.notNeeded = 0;/* dummy ... not used ... don't care */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , &commonInfo , extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaThreadIdEntrySet(dev, phaThreadId , &commonInfo , extType , &extInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*
GT_STATUS cpssDxChPhaThreadIdEntryGet
(
    IN GT_U8                devNum,
    IN GT_U32               phaThreadId,
    OUT CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC *commonInfoPtr,
    OUT CPSS_DXCH_PHA_THREAD_TYPE_ENT    *extTypePtr,
    OUT CPSS_DXCH_PHA_THREAD_INFO_UNT    *extInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaThreadIdEntryGet)
{
    GT_STATUS            st;
    GT_U8                dev;
    GT_U32               phaThreadId;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC commonInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT    extType;
    CPSS_DXCH_PHA_THREAD_INFO_UNT    extInfo;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;

        /* check NULL pointers */
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , NULL , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , NULL , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* check valid parameters */
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check invalid parameters */
        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC + 1;
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC - 1;
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    }

    phaThreadId = 1;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaThreadIdEntryGet(dev, phaThreadId , &commonInfo , &extType , &extInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*
GT_STATUS cpssDxChPhaPortThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    IN GT_BOOL      enable,
    IN GT_U32       phaThreadId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaPortThreadIdSet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable,enableGet;
    GT_U32           phaThreadId,phaThreadIdGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            enable = GT_FALSE;

            phaThreadId = 0xFFFFFFFF;/* ignored due to enable = GT_FALSE */
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaPortThreadIdGet(dev,port,&enableGet,&phaThreadIdGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);

            enable = GT_TRUE;

            phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC - 1;
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

            phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC + 1;
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

            /* min */
            phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaPortThreadIdGet(dev,port,&enableGet,&phaThreadIdGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(phaThreadId, phaThreadIdGet);

            /* max */
            phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaPortThreadIdGet(dev,port,&enableGet,&phaThreadIdGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(enable, enableGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(phaThreadId, phaThreadIdGet);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            enable = GT_TRUE;
            phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
            st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    port = 0;
    enable = GT_FALSE;
    phaThreadId = 1;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaPortThreadIdSet(dev,port,enable,phaThreadId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaPortThreadIdGet
(
    IN GT_U8        devNum,
    IN GT_PORT_NUM  portNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *phaThreadIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaPortThreadIdGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PORT_NUM      port;
    GT_BOOL          enable;
    GT_U32           phaThreadId;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPhaPortThreadIdGet(dev,port,&enable,&phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* check NULL pointer */
            st = cpssDxChPhaPortThreadIdGet(dev,port,NULL,&phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            st = cpssDxChPhaPortThreadIdGet(dev,port,&enable,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
        }

        st = prvUtfNextVirtPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextVirtPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPhaPortThreadIdGet(dev,port,&enable,&phaThreadId);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    port = 0;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaPortThreadIdGet(dev,port,&enable,&phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaPortThreadIdGet(dev,port,&enable,&phaThreadId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
GT_STATUS cpssDxChPhaSourcePortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaSourcePortEntrySet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PHYSICAL_PORT_NUM      port;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT infoType,infoTypeGet;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT portInfo,portInfoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E;
            portInfo.rawFormat = 15;/* dummy */

            st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoTypeGet,&portInfoGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(infoType, infoTypeGet);
            st = cpssOsMemCmp(&portInfo, &portInfoGet, sizeof(portInfoGet));
            UTF_VERIFY_EQUAL0_PARAM_MAC(0, st);
        
            /*valid port value*/
            infoType = CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E;
            portInfo.erspanSameDevMirror.erspanIndex = 0xFFFF;

            st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoTypeGet,&portInfoGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(infoType, infoTypeGet);
            st = cpssOsMemCmp(&portInfo, &portInfoGet, sizeof(portInfoGet));
            UTF_VERIFY_EQUAL0_PARAM_MAC(0, st);

            /*invalid port id*/
            /*CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E*/
            infoType = CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E;
            portInfo.erspanSameDevMirror.erspanIndex = 0x1FFFF;

            st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, st);
            /* check NULL pointer */
            st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            infoType = CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E;
            portInfo.rawFormat = 15;/* dummy */
            st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    port = 0;
    infoType = CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNUSED_E;
    portInfo.rawFormat = 15;/* dummy */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,&portInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaSourcePortEntrySet(dev,port,infoType,&portInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaSourcePortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaSourcePortEntryGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PHYSICAL_PORT_NUM      port;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT infoType;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT portInfo;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* check NULL pointer */
            st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoType,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            st = cpssDxChPhaSourcePortEntryGet(dev,port,NULL,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    port = 0;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoType,&portInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaSourcePortEntryGet(dev,port,&infoType,&portInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaTargetPortEntrySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  infoType,
    IN CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaTargetPortEntrySet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PHYSICAL_PORT_NUM      port;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT infoType,infoTypeGet;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT portInfo,portInfoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            infoType = CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E;
            portInfo.rawFormat = 15;/* dummy */

            st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoTypeGet,&portInfoGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(infoType, infoTypeGet);
            st = cpssOsMemCmp(&portInfo, &portInfoGet, sizeof(portInfoGet));
            UTF_VERIFY_EQUAL0_PARAM_MAC(0, st);

            /*valid values*/
            infoType = CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E;
            portInfo.erspanSameDevMirror.erspanIndex = 0xFFFF;

            st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoTypeGet,&portInfoGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            UTF_VERIFY_EQUAL0_PARAM_MAC(infoType, infoTypeGet);
            st = cpssOsMemCmp(&portInfo, &portInfoGet, sizeof(portInfoGet));
            UTF_VERIFY_EQUAL0_PARAM_MAC(0, st);
            
            /*invalid port id*/
            /*CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E*/
            infoType = CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E;
            portInfo.erspanSameDevMirror.erspanIndex = 0x1FFFF;

            st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, st);

            /* check NULL pointer */
            st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            infoType = CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E;
            portInfo.rawFormat = 15;/* dummy */
            st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    port = 0;
    infoType = CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNUSED_E;
    portInfo.rawFormat = 15;/* dummy */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,&portInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaTargetPortEntrySet(dev,port,infoType,&portInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaTargetPortEntryGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT  *infoTypePtr,
    OUT CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT  *portInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaTargetPortEntryGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_PHYSICAL_PORT_NUM      port;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT infoType;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT portInfo;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* check NULL pointer */
            st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoType,NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            st = cpssDxChPhaTargetPortEntryGet(dev,port,NULL,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
        }

        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available ePorts. */
        while (GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoType,&portInfo);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        }
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    port = 0;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoType,&portInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaTargetPortEntryGet(dev,port,&infoType,&portInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT    dropCode,
    IN  CPSS_PACKET_CMD_ENT         packetCommand
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaHeaderModificationViolationInfoSet)
{
    GT_STATUS        st;
    GT_U8            dev;
    CPSS_NET_RX_CPU_CODE_ENT    dropCode,dropCodeGet;
    CPSS_PACKET_CMD_ENT         packetCommand,packetCommandGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        dropCode      = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
        packetCommand = CPSS_PACKET_CMD_DROP_HARD_E;

        st = cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCodeGet,&packetCommandGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* the CPU code can be converted from 'HW' format to other 'SW format'*/
        /* convert SW cpu code to HW cpu code */
        check_cpuCode(dev,dropCode,dropCodeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(packetCommand, packetCommandGet, dev);


        for(dropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E ;
            dropCode <= CPSS_NET_LAST_UNKNOWN_HW_CPU_CODE_E ;
            dropCode += 19)/* iterate on several values */
        {
            st = cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*  check bad ENUM values */
        UTF_ENUMS_CHECK_MAC(cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand),
            dropCode);

        dropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 111;

        for(packetCommand = CPSS_PACKET_CMD_FORWARD_E ;
            packetCommand <= CPSS_PACKET_CMD_DEFAULT_ROUTE_ENTRY_E;
            packetCommand++)
        {
            st = cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand);
            if(packetCommand == CPSS_PACKET_CMD_FORWARD_E ||
               packetCommand == CPSS_PACKET_CMD_DROP_HARD_E)
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCodeGet,&packetCommandGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
                /* the CPU code can be converted from 'HW' format to other 'SW format'*/
                /* convert SW cpu code to HW cpu code */
                check_cpuCode(dev,dropCode,dropCodeGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(packetCommand, packetCommandGet, dev);
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            }
        }
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    dropCode      = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    packetCommand = CPSS_PACKET_CMD_FORWARD_E;
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaHeaderModificationViolationInfoSet(dev,dropCode,packetCommand);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaHeaderModificationViolationInfoGet
(
    IN  GT_U8                       devNum,
    OUT CPSS_NET_RX_CPU_CODE_ENT    *dropCodePtr,
    OUT CPSS_PACKET_CMD_ENT         *packetCommandPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaHeaderModificationViolationInfoGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    CPSS_NET_RX_CPU_CODE_ENT    dropCode;
    CPSS_PACKET_CMD_ENT         packetCommand;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCode,&packetCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check NULL pointer */
        st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCode,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,NULL,&packetCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCode,&packetCommand);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaHeaderModificationViolationInfoGet(dev,&dropCode,&packetCommand);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaHeaderModificationViolationCapturedGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                     *capturedThreadIdPtr,
    OUT CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  *violationTypePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaHeaderModificationViolationCapturedGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_U32                      capturedThreadId;
    CPSS_DXCH_PHA_HEADER_MODIFICATION_VIOLATION_TYPE_ENT  violationType;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        do{
            st = cpssDxChPhaHeaderModificationViolationCapturedGet(dev,&capturedThreadId,&violationType);
            if(st == GT_NO_MORE)
            {
                /* this is valid value ! ... there was no violation to report about */
                break;
            }
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }while(st == GT_OK);

        /* check NULL pointer */
        st = cpssDxChPhaHeaderModificationViolationCapturedGet(dev,&capturedThreadId,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaHeaderModificationViolationCapturedGet(dev,NULL,&violationType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaHeaderModificationViolationCapturedGet(dev,&capturedThreadId,&violationType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaHeaderModificationViolationCapturedGet(dev,&capturedThreadId,&violationType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdSet
(
    IN GT_U8        devNum,
    IN GT_U32       phaThreadId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaStatisticalProcessingCounterThreadIdSet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_U32           phaThreadId,phaThreadIdGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(phaThreadId, phaThreadIdGet, dev);

        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC;
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(phaThreadId, phaThreadIdGet, dev);

        phaThreadId = 0;
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadIdGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(phaThreadId, phaThreadIdGet, dev);

        phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MAX_MAC + 1;
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    phaThreadId = PRV_CPSS_DXCH_PHA_THREAD_ID_MIN_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaStatisticalProcessingCounterThreadIdSet(dev,phaThreadId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaStatisticalProcessingCounterThreadIdGet
(
    IN  GT_U8        devNum,
    OUT GT_U32       *phaThreadIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaStatisticalProcessingCounterThreadIdGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_U32           phaThreadId;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check NULL pointer */
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaStatisticalProcessingCounterThreadIdGet(dev,&phaThreadId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaStatisticalProcessingCounterGet
(
    IN  GT_U8        devNum,
    OUT GT_U64       *counterPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaStatisticalProcessingCounterGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_U64           counter;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPhaStatisticalProcessingCounterGet(dev,&counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check NULL pointer */
        st = cpssDxChPhaStatisticalProcessingCounterGet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaStatisticalProcessingCounterGet(dev,&counter);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaStatisticalProcessingCounterGet(dev,&counter);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaInit
(
    IN  GT_U8                           devNum,
    IN GT_BOOL                          packetOrderChangeEnable
    IN CPSS_DXCH_PHA_FW_IMAGE_ID_ENT    phaFwImageId
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaInit)
{
    GT_STATUS          st;
    GT_U8              dev;
    GT_BOOL            packetOrderChangeEnable = GT_TRUE;
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT phaFwImageId = CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Initialized PHA unit */
        st = cpssDxChPhaInit(dev,packetOrderChangeEnable,phaFwImageId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaInit(dev,packetOrderChangeEnable,phaFwImageId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaInit(dev,packetOrderChangeEnable,phaFwImageId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*
GT_STATUS cpssDxChPhaFwImageIdGet
(
    IN GT_U8                           devNum,
    OUT CPSS_DXCH_PHA_FW_IMAGE_ID_ENT  *phaFwImageIdPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaFwImageIdGet)
{
    GT_STATUS          st;
    GT_U8              dev;
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT phaFwImageId;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Get PHA fw image ID */
        st = cpssDxChPhaFwImageIdGet(dev, &phaFwImageId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Check PHA fw image is default */
        if (phaFwImageId != CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E)
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaFwImageIdGet(dev, &phaFwImageId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaFwImageIdGet(dev, &phaFwImageId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaSharedMemoryErspanGlobalConfigSet
(
    IN GT_U8        devNum,
    IN GT_U8        erspanDevId,
    IN GT_BOOL      isVoQ
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaSharedMemoryErspanGlobalConfigSet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_U16           erspanDevId,erspanDevIdGet;
    GT_BOOL          isVoQ, isVoQGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        erspanDevId = 0;
        isVoQ = GT_TRUE;
        st = cpssDxChPhaSharedMemoryErspanGlobalConfigSet(dev,erspanDevId,isVoQ);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaSharedMemoryErspanGlobalConfigGet(dev,&erspanDevIdGet,&isVoQGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(erspanDevId, erspanDevIdGet, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(isVoQ, isVoQGet, dev);

        erspanDevId = 7;
        isVoQ = GT_FALSE;
        st = cpssDxChPhaSharedMemoryErspanGlobalConfigSet(dev,erspanDevId,isVoQ);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaSharedMemoryErspanGlobalConfigGet(dev,&erspanDevIdGet,&isVoQGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(erspanDevId, erspanDevIdGet, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(isVoQ, isVoQGet, dev);

        erspanDevId = 3;
        st = cpssDxChPhaSharedMemoryErspanGlobalConfigSet(dev,erspanDevId,isVoQ);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaSharedMemoryErspanGlobalConfigGet(dev,&erspanDevIdGet,&isVoQGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(erspanDevId, erspanDevIdGet, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(isVoQ, isVoQGet, dev);

        erspanDevId = 8;
        st = cpssDxChPhaSharedMemoryErspanGlobalConfigSet(dev,erspanDevId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    erspanDevId = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaSharedMemoryErspanGlobalConfigSet(dev,erspanDevId,GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaSharedMemoryErspanGlobalConfigSet(dev,erspanDevId,GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaSharedMemoryErspanGlobalConfigGet
(
    IN  GT_U8        devNum,
    OUT GT_U8       *erspanDevIdPtr,
    OUT GT_BOOL     *isVoQPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaSharedMemoryErspanGlobalConfigGet)
{
    GT_STATUS        st;
    GT_U8            dev;
    GT_U16           erspanDevId;
    GT_BOOL          isVoQ;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChPhaSharedMemoryErspanGlobalConfigGet(dev,&erspanDevId,&isVoQ);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* check NULL pointer */
        st = cpssDxChPhaSharedMemoryErspanGlobalConfigGet(dev,NULL,&isVoQ);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChPhaSharedMemoryErspanGlobalConfigGet(dev,&erspanDevId,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaSharedMemoryErspanGlobalConfigGet(dev,&erspanDevId,&isVoQ);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaSharedMemoryErspanGlobalConfigGet(dev,&erspanDevId,&isVoQ);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           analyzerIndex,
    IN CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet)
{
    GT_STATUS                                        st;
    GT_U8                                            dev;
    GT_U32                                           analyzerIndex;
    CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC entryInfo;
    CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC entryInfoGet;
    GT_U8 macDa[6] = {1,2,3,4,5,6};
    GT_U8 macSa[6] = {6,5,4,3,2,1};
    GT_U8 ipv6SipAddr[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    GT_U8 ipv6DipAddr[16] = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        analyzerIndex = 0;
        cpssOsMemSet(&entryInfo,0,sizeof(entryInfo));
        cpssOsMemSet(&entryInfoGet,0,sizeof(entryInfoGet));

        /* check NULL pointer */
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* check analyzerIndex validity */
        analyzerIndex = 7;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* check protocol stack validity */
        analyzerIndex = 0;
        entryInfo.protocol = CPSS_IP_PROTOCOL_IPV4V6_E;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* check I/E session id validity */
        entryInfo.protocol = CPSS_IP_PROTOCOL_IPV4_E;
        entryInfo.ingressSessionId = BIT_10;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        entryInfo.ingressSessionId = BIT_10-1;
        entryInfo.egressSessionId = BIT_10;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
        entryInfo.egressSessionId = BIT_10-1;

        /* check ipv4 parameters validity */
        entryInfo.protocol = CPSS_IP_PROTOCOL_IPV4_E;

        /* ipv4 dscp validation */
        entryInfo.ipInfo.ipv4.dscp  = BIT_6;
        entryInfo.ipInfo.ipv4.flags = BIT_0;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* ipv4 flags validation */
        entryInfo.ipInfo.ipv4.dscp  = BIT_6-1;
        entryInfo.ipInfo.ipv4.flags = BIT_3;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        entryInfo.ipInfo.ipv4.flags = BIT_0;
        /* check l2 parameters validity */
        entryInfo.l2Info.up = BIT_3;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        entryInfo.l2Info.up  = BIT_3 - 1;
        entryInfo.l2Info.cfi = BIT_1;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        entryInfo.l2Info.cfi = BIT_1 - 1;
        entryInfo.l2Info.vid = BIT_12;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        entryInfo.l2Info.vid = BIT_12 - 1;

        /* check ipv6 parameters validity */
        entryInfo.protocol = CPSS_IP_PROTOCOL_IPV6_E;
        entryInfo.ipInfo.ipv6.flowLabel = BIT_20;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* check api */
        cpssOsMemCpy(&entryInfo.l2Info.macDa, &macDa[0], sizeof(GT_U8)*6);
        cpssOsMemCpy(&entryInfo.l2Info.macSa, &macSa[0], sizeof(GT_U8)*6);
        entryInfo.l2Info.tpid                   = BIT_16 - 1;
        entryInfo.l2Info.up                     = BIT_3 - 1;
        entryInfo.l2Info.cfi                    = BIT_1 - 1;
        entryInfo.l2Info.vid                    = BIT_12 - 1;
        entryInfo.protocol                      = CPSS_IP_PROTOCOL_IPV4_E;
        entryInfo.ipInfo.ipv4.flags             = BIT_0;
        entryInfo.ipInfo.ipv4.dscp              = BIT_6-1;
        entryInfo.ipInfo.ipv4.ttl               = BIT_8-1;
        entryInfo.ipInfo.ipv4.sipAddr.arIP[0]   = BIT_0;
        entryInfo.ipInfo.ipv4.sipAddr.arIP[1]   = BIT_1;
        entryInfo.ipInfo.ipv4.sipAddr.arIP[2]   = BIT_2;
        entryInfo.ipInfo.ipv4.sipAddr.arIP[3]   = BIT_3;
        entryInfo.ipInfo.ipv4.dipAddr.arIP[0]   = BIT_4;
        entryInfo.ipInfo.ipv4.dipAddr.arIP[1]   = BIT_5;
        entryInfo.ipInfo.ipv4.dipAddr.arIP[2]   = BIT_6;
        entryInfo.ipInfo.ipv4.dipAddr.arIP[3]   = BIT_7;

        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet(dev, analyzerIndex, &entryInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssOsMemCmp(&entryInfo,&entryInfoGet,sizeof(entryInfo)) == 0 ? GT_OK : GT_BAD_STATE;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        cpssOsMemSet(&entryInfo,0,sizeof(entryInfo));
        cpssOsMemSet(&entryInfoGet,0,sizeof(entryInfoGet));

        cpssOsMemCpy(&entryInfo.l2Info.macDa, &macDa[0], sizeof(GT_U8)*6);
        cpssOsMemCpy(&entryInfo.l2Info.macSa, &macSa[0], sizeof(GT_U8)*6);
        entryInfo.l2Info.tpid                   = BIT_16 - 1;
        entryInfo.l2Info.up                     = BIT_3 - 1;
        entryInfo.l2Info.cfi                    = BIT_1 - 1;
        entryInfo.l2Info.vid                    = BIT_12 - 1;
        entryInfo.protocol                      = CPSS_IP_PROTOCOL_IPV6_E;
        entryInfo.ipInfo.ipv6.tc                = BIT_8-1;
        entryInfo.ipInfo.ipv6.flowLabel         = BIT_20-1;
        entryInfo.ipInfo.ipv6.hopLimit          = BIT_8-1;
        cpssOsMemCpy(&entryInfo.ipInfo.ipv6.sipAddr.arIP[0], &ipv6SipAddr[0], sizeof(GT_U8)*16);
        cpssOsMemCpy(&entryInfo.ipInfo.ipv6.dipAddr.arIP[0], &ipv6DipAddr[0], sizeof(GT_U8)*16);

        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet(dev, analyzerIndex, &entryInfoGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssOsMemCmp(&entryInfo,&entryInfoGet,sizeof(entryInfo)) == 0 ? GT_OK : GT_BAD_STATE;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    analyzerIndex = 0;
    cpssOsMemSet(&entryInfo,0,sizeof(entryInfo));

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex , &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex , &entryInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*
GT_STATUS cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet
(
    IN GT_U8                                             devNum,
    IN GT_U32                                            analyzerIndex,
    OUT CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC *entryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet)
{
    GT_STATUS                                        st;
    GT_U8                                            dev;
    GT_U32                                           analyzerIndex;
    CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC entryInfo;
    CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC entryInfoGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        analyzerIndex = 0;
        cpssOsMemSet(&entryInfo,0,sizeof(entryInfo));
        cpssOsMemSet(&entryInfoGet,0,sizeof(entryInfoGet));

        /* check NULL pointer */
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet(dev, analyzerIndex, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /* check analyzerIndex validity */
        analyzerIndex = 7;
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(dev, analyzerIndex, &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    analyzerIndex = 0;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet(dev, analyzerIndex , &entryInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet(dev, analyzerIndex, &entryInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*
GT_STATUS cpssDxChPhaErrorsConfigSet
(
    IN  GT_U8                                dev,
    IN  CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC  *errorCode
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhaErrorsConfigSet)
{
/*
    ITERATE_DEVICES(Falcon and above)
    1.1. Call with errorCode[(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1) /
                           CPSS_NET_UN_KNOWN_UC_E /
                           CPSS_NET_UN_REGISTERD_MC_E /
                           CPSS_NET_CONTROL_BPDU_E /
                           CPSS_NET_CONTROL_DEST_MAC_TRAP_E],
    Expected: GT_OK.
    1.2. Call cpssDxChPhaErrorsConfigGet
         with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call API with wrong errorCode [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.4 Call API with NULL pointer
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                st = GT_OK;
    GT_U8                    dev;
    CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC errorCode = {(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1), (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)};
    CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC errorCodeGet = {(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1), (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)};

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with errorCode[(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1) /
                                   CPSS_NET_UN_KNOWN_UC_E /
                                   CPSS_NET_UN_REGISTERD_MC_E /
                                   CPSS_NET_CONTROL_BPDU_E /
                                   CPSS_NET_CONTROL_DEST_MAC_TRAP_E],
            Expected: GT_OK.
        */
        /* call with errorCode[(CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1)][CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E] */
        errorCode.tablesReadErrorDropCode = (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1);
        errorCode.ppaClockDownErrorDropCode = CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E;

        st = cpssDxChPhaErrorsConfigSet(dev, &errorCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPhaErrorsConfigGet
                 with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChPhaErrorsConfigGet(dev, &errorCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChPhaErrorsConfigGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(errorCode.tablesReadErrorDropCode, errorCodeGet.tablesReadErrorDropCode,
                       "got another errorCode then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(errorCode.ppaClockDownErrorDropCode, errorCodeGet.ppaClockDownErrorDropCode,
                       "got another errorCode then was set: %d", dev);

        /* call with errorCode[CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E]i[CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E] */
        errorCode.tablesReadErrorDropCode = CPSS_NET_IEEE_RSRVD_MULTICAST_ADDR_1_E;
        errorCode.ppaClockDownErrorDropCode = CPSS_NET_EGRESS_MIRRORED_TO_ANLYZER_E;

        st = cpssDxChPhaErrorsConfigSet(dev, &errorCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPhaErrorsConfigGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChPhaErrorsConfigGet(dev, &errorCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChPhaErrorsConfigGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(errorCode.tablesReadErrorDropCode, errorCodeGet.tablesReadErrorDropCode,
                       "got another errorCode then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(errorCode.ppaClockDownErrorDropCode, errorCodeGet.ppaClockDownErrorDropCode,
                       "got another errorCode then was set: %d", dev);

        /* call with errorCode[CPSS_NET_IPV6_HOP_BY_HOP_E][CPSS_NET_ROUTE_ENTRY_TRAP_E] */
        errorCode.tablesReadErrorDropCode = CPSS_NET_IPV6_HOP_BY_HOP_E;
        errorCode.ppaClockDownErrorDropCode = CPSS_NET_ROUTE_ENTRY_TRAP_E;

        st = cpssDxChPhaErrorsConfigSet(dev, &errorCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPhaErrorsConfigGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChPhaErrorsConfigGet(dev, &errorCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChPhaErrorsConfigGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(errorCode.tablesReadErrorDropCode, errorCodeGet.tablesReadErrorDropCode,
                       "got another errorCode then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(errorCode.ppaClockDownErrorDropCode, errorCodeGet.ppaClockDownErrorDropCode,
                       "got another errorCode then was set: %d", dev);

        /* call with errorCode[CPSS_NET_CONTROL_BPDU_E] */
        errorCode.tablesReadErrorDropCode = CPSS_NET_CONTROL_BPDU_E;
        errorCode.ppaClockDownErrorDropCode = CPSS_NET_CONTROL_BPDU_E;

        st = cpssDxChPhaErrorsConfigSet(dev, &errorCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPhaErrorsConfigGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChPhaErrorsConfigGet(dev, &errorCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChPhaErrorsConfigGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(errorCode.tablesReadErrorDropCode, errorCodeGet.tablesReadErrorDropCode,
                       "got another errorCode then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(errorCode.ppaClockDownErrorDropCode, errorCodeGet.ppaClockDownErrorDropCode,
                       "got another errorCode then was set: %d", dev);

        /* call with errorCode[CPSS_NET_CLASS_KEY_MIRROR_E] */
        errorCode.tablesReadErrorDropCode = CPSS_NET_CLASS_KEY_MIRROR_E;
        errorCode.ppaClockDownErrorDropCode = CPSS_NET_CLASS_KEY_MIRROR_E;

        st = cpssDxChPhaErrorsConfigSet(dev, &errorCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChPhaErrorsConfigGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChPhaErrorsConfigGet(dev, &errorCodeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChPhaErrorsConfigGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(errorCode.tablesReadErrorDropCode, errorCodeGet.tablesReadErrorDropCode,
                       "got another errorCode then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(errorCode.ppaClockDownErrorDropCode, errorCodeGet.ppaClockDownErrorDropCode,
                       "got another errorCode then was set: %d", dev);

        /*
            1.3. Call api with wrong errorCode [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChPhaErrorsConfigSet
                            (dev, &errorCode),
                             errorCode.tablesReadErrorDropCode);

        UTF_ENUMS_CHECK_MAC(cpssDxChPhaErrorsConfigSet
                            (dev, &errorCode),
                             errorCode.ppaClockDownErrorDropCode);
        /*
            1.3. Call api with NULL pointer.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPhaErrorsConfigSet(dev, &errorCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* restore values */
    errorCode.tablesReadErrorDropCode = (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1);
    errorCode.ppaClockDownErrorDropCode = (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 1);

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaErrorsConfigSet(dev, &errorCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaErrorsConfigSet(dev, &errorCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaErrorsConfigGet
(
    IN  GT_U8                     dev,
    OUT CPSS_NET_RX_CPU_CODE_ENT  *errorCodePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPhaErrorsConfigGet)
{
/*
    ITERATE_DEVICES(Falcon and above)
    1.1. Call with not null errorCodePtr.
    Expected: GT_OK.
    1.2. Call api with wrong errorCodePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                                dev;
    CPSS_DXCH_PHA_ERROR_CODE_CONFIG_STC  errorCode;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null errorCodePtr.
            Expected: GT_OK.
        */
        st = cpssDxChPhaErrorsConfigGet(dev, &errorCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong errorCodePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPhaErrorsConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaErrorsConfigGet(dev, &errorCode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaErrorsConfigGet(dev, &errorCode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaFwImageInfoGet
(
    IN GT_U8                             devNum,
    OUT CPSS_DXCH_PHA_FW_IMAGE_INFO_STC  *fwImageInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaFwImageInfoGet)
{
/*
    1.1. Call API with valid parameters.
    Expected: GT_OK                             .
    1.2. Verify fw image ID is the default
         Verify fw version is not zero                                   .
         Verify fw version array is not zero1.2. Call api with wrong errorCodePtr [NULL].
    1.3. Call API with NULL pointer to fwVerInfo structure.
    Expected: GT_BAD_PTR.
    2. For not-active devices and devices from non-applicable family
       check that function returns GT_BAD_PARAM.
    3. Call function with out of bound value for device id.
*/
    GT_STATUS          st;
    GT_U8              dev;
    GT_U32             i;
    CPSS_DXCH_PHA_FW_IMAGE_INFO_STC fwVerInfo;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Clear fwVerInfo structure */
        cpssOsMemSet(&fwVerInfo, 0, sizeof(fwVerInfo));

        /*
            1.1. Call API with valid parameters.
            Expected: GT_OK.
        */
        /* Get PHA fw image information */
        st = cpssDxChPhaFwImageInfoGet(dev, &fwVerInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Verify fw image ID is the default       .
                 Verify fw version is not zero                                             .
                 Verify fw version array is not zero                                                                          .
        */
        /* Check PHA fw image ID is default */
        if (fwVerInfo.fwImageId != CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E)
        {
            st = GT_FAIL;
        }

        /* Check PHA fw image version is not zero */
        if (fwVerInfo.fwVersion == 0)
        {
            st = GT_FAIL;
        }

        /* Check PHA fw image version array is not zero */
        for (i=0;i<CPSS_DXCH_PHA_FW_NUM_OF_SUPPORTED_VERSIONS_PER_IMAGE_CNS;i++)
        {
            if (fwVerInfo.fwVersionsArr[i] == 0)
            {
                st = GT_FAIL;
            }
        }

        /*
            1.3. Call API with NULL pointer to fwVerInfo structure.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPhaFwImageInfoGet(dev,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaFwImageInfoGet(dev, &fwVerInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaFwImageInfoGet(dev, &fwVerInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChPhaFwImageUpgrade
(
    IN GT_U8                             devNum,
    IN  GT_BOOL                          packetOrderChangeEnable,
    OUT CPSS_DXCH_PHA_FW_IMAGE_INFO_STC  *fwImageInfoPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChPhaFwImageUpgrade)
{
/*
    1.1. Call API with valid parameters.
    Expected: GT_OK                             .
    1.2. Load invalid fw version (fw version that doesn't exist)      .
    Expected: GT_BAD_PARAM
    1.3. Call API with NULL pointer to fwVerInfo structure.
    Expected: GT_BAD_PTR.
    2. For not-active devices and devices from non-applicable family
       check that function returns GT_BAD_PARAM.
    3. Call function with out of bound value for device id.
*/
    GT_STATUS          st;
    GT_U8              dev;
    CPSS_DXCH_PHA_FW_IMAGE_INFO_STC fwVerInfo;


    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Clear fwVerInfo structure */
        cpssOsMemSet(&fwVerInfo, 0, sizeof(fwVerInfo));

        /*
            1.1. Call API with valid parameters.
            Expected: GT_OK.
        */
        /* Get PHA fw image information */
        st = cpssDxChPhaFwImageInfoGet(dev, &fwVerInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Load the same default version that was loaded at PHA init */
        st = cpssDxChPhaFwImageUpgrade(dev, GT_FALSE, &fwVerInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Load fw image ID 1 */
        st = cpssDxChPhaInit(dev, GT_FALSE, CPSS_DXCH_PHA_FW_IMAGE_ID_01_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Clear fwVerInfo structure */
        cpssOsMemSet(&fwVerInfo, 0, sizeof(fwVerInfo));

        /* Get PHA fw image information */
        st = cpssDxChPhaFwImageInfoGet(dev, &fwVerInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Select to load the second version from the versions array */
        fwVerInfo.fwVersion = fwVerInfo.fwVersionsArr[1];

        st = cpssDxChPhaFwImageUpgrade(dev, GT_FALSE, &fwVerInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* Load back default fw image */
        st = cpssDxChPhaInit(dev, GT_FALSE, CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);       

        /*
            1.2. Load invalid fw version (fw version that doesn't exist)      .
            Expected: GT_BAD_PARAM
        */
        fwVerInfo.fwImageId = 0;
        fwVerInfo.fwVersion = 0xffff;
        st = cpssDxChPhaFwImageUpgrade(dev, GT_FALSE, &fwVerInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /*
            1.3. Call API with NULL pointer to fwVerInfo structure.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChPhaFwImageUpgrade(dev, GT_FALSE, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }
    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_CNS);
    PRV_TGF_NON_PHA_APPLIC_DEV_RESET_MAC;

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPhaFwImageUpgrade(dev, GT_FALSE, &fwVerInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPhaFwImageUpgrade(dev, GT_FALSE, &fwVerInfo);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}




/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPha suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPha)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaThreadIdEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaThreadIdEntryGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaPortThreadIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaPortThreadIdGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaSourcePortEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaSourcePortEntryGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaTargetPortEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaTargetPortEntryGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaHeaderModificationViolationInfoSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaHeaderModificationViolationInfoGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaHeaderModificationViolationCapturedGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaStatisticalProcessingCounterThreadIdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaStatisticalProcessingCounterThreadIdGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaStatisticalProcessingCounterGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaInit)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaFwImageIdGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaSharedMemoryErspanGlobalConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaSharedMemoryErspanGlobalConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaSharedMemoryErspanIpTemplateEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaErrorsConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaErrorsConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaFwImageInfoGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPhaFwImageUpgrade)
UTF_SUIT_END_TESTS_MAC(cpssDxChPha)

