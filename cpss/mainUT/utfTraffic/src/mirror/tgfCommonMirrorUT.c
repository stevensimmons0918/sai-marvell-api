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
* @file tgfCommonMirrorUT.c
*
* @brief Enhanced UTs for CPSS Mirror
*
* @version   18
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <mirror/prvTgfMirror.h>
#include <common/tgfCscdGen.h>
#include <mirror/prvTgfIngressMirrorErspanUseCase.h>
#include <mirror/prvTgfEgressMirrorErspanUseCase.h>
#include <mirror/prvTgfIngressMirrorSflow.h>

/*
parameter relevant to next tests:
    prvTgfIngressMirrorHopByHop
    prvTgfIngressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer
    prvTgfIngressMirrorSourceBased
    prvTgfIngressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer
    prvTgfIngressMirrorEndToEnd
    prvTgfIngressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer
    prvTgfEgressMirrorHopByHop
    prvTgfEgressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer
    prvTgfEgressMirrorSourceBased
    prvTgfEgressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer
    prvTgfEgressMirrorEndToEnd
    prvTgfEgressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer
*/
static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
static GT_BOOL                  currentDsaTagEnable = GT_FALSE;
static TGF_DSA_TYPE_ENT         currentDsaType = TGF_DSA_2_WORD_TYPE_E;
static GT_BOOL                   ingressDirection = 0;
static GT_BOOL                   egressDirection = 1;
static GT_BOOL                   lcMirror = 1;
static GT_BOOL                   falconMirror = 0;
static GT_U32                    lcSourceDev = 9;
static GT_U32                    falconSourceDev = 2;
static GT_BOOL                   protocolIpv4 = 0;
static GT_BOOL                   protocolIpv6 = 1;
static GT_BOOL                   withVoQ = 1;
static GT_BOOL                   withoutVoQ = 0;

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress mirroring functionality in hop-by-hop mode:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Rx mirroring for HopByHop mode;
    send BC traffic and verify expected mirrored traffic on analyzer port;
    disable global Rx mirroring;
    send BC traffic and verify expected original flood traffic on all ports;
    enable global Rx mirroring;
    verify that if a packet is mirrored by both the port-based ingress mirroring
    mechanism, and one of the other ingress mirroring mechanisms,
    the selected analyzer is the one with the higher index.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorHopByHop)
{
/********************************************************************
    Test 8.1 - Ingress mirroring functionality in hop-by-hop mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    /* test is oriented to hop-by-hop */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);
    }

    /* Set configuration */
    prvTgfIngressMirrorHopByHopConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorHopByHopTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorHopByHopConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfIngressMirrorHopByHop , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer)
{
    /* test is oriented to hop-by-hop */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;

    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfIngressMirrorHopByHop */
    UTF_TEST_CALL_MAC(prvTgfIngressMirrorHopByHop);

    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfIngressMirrorHopByHop */
        UTF_TEST_CALL_MAC(prvTgfIngressMirrorHopByHop);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress mirroring functionality in source-based mode:
    configure VLAN, FDB entries;
    configure Source-Based forwarding mode for mirroring to analyzer;
    configure 2 analyzer interfaces and enable Rx mirroring;
    send BC traffic to each configured Rx analyzer port;
        verify expected mirrored traffic on analyzer port and flood traffic on all ports;
        disable Rx mirroring;
    send BC traffic to each configured Rx analyzer port;
    verify expected original flood traffic on all ports.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorSourceBased)
{
/********************************************************************
    Test 8.2 - Ingress mirroring functionality in source-based mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E;
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
    }

    /* Set configuration */
    prvTgfIngressMirrorSourceBasedConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorSourceBasedTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorSourceBasedConfigurationRestore();
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfIngressMirrorSourceBased , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer)
{
    /* test is oriented to 'source based' mirroring - related to ingress processing of DSA fields */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E;

    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfIngressMirrorSourceBased */
    UTF_TEST_CALL_MAC(prvTgfIngressMirrorSourceBased);

    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfIngressMirrorSourceBased */
        UTF_TEST_CALL_MAC(prvTgfIngressMirrorSourceBased);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;

}

/*----------------------------------------------------------------------------*/
/*
    prvTgfIngressMirrorEndToEnd
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorEndToEnd)
{
/********************************************************************
    Ingress mirroring functionality in End-To-End mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable, currentDsaType, currentMirrorMode);
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
    }

    /* Set configuration */
    prvTgfIngressMirrorSourceBasedConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorSourceBasedTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorSourceBasedConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfIngressMirrorEndToEnd , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;
#if 0  /* 2 words  DSA not hold parameters of this mode */
    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfIngressMirrorEndToEnd */
    UTF_TEST_CALL_MAC(prvTgfIngressMirrorEndToEnd);
#endif /*0*/
    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfIngressMirrorEndToEnd */
        UTF_TEST_CALL_MAC(prvTgfIngressMirrorEndToEnd);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;

}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test Port source-based mirroring and other ingress mirroring mechanisms:
    configure VLAN, FDB entries;
    configure Source-Based forwarding mode for mirroring to analyzer;
    configure 2 analyzer interfaces and enable Rx mirroring;
    send BC traffic to each configured Rx analyzer port;
        verify expected mirrored traffic on analyzer port and flood traffic on all ports;
        set analyzer interface with lower index and enable Rx mirroring;
        send BC traffic to Rx analyzer port;
    verify expected mirrored traffic on analyzer with higher index.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorHighestIndexSelection)
{
/********************************************************************
    Test 8.3 - Port source-based mirroring and other ingress mirroring mechanisms.
               (Selected analyzer is the one with the higher index.)

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    CPSS_TBD_BOOKMARK_FALCON
    /* the falcon not supports mirroring from FDB entry ..
       so test need to change to use other mirroring cause */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);


    /* Set configuration */
    prvTgfIngressMirrorHighestIndexSelectionConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorHighestIndexSelectionTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorHighestIndexSelectionConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test egress mirroring functionality in hop-by-hop mode:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Tx mirroring for HopByHop mode;
    send BC traffic and verify expected mirrored traffic on analyzer port;
    disable global Tx mirroring;
    send BC traffic and verify expected original flood traffic on all ports;
    enable Tx mirroring on each analyzer;
    send BC traffic to each enabled Tx analyzer port;
        verify expected mirrored traffic on analyzer port and flood traffic on all ports.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorHopByHop)
{
/********************************************************************
    Test 8.4 - Egress mirroring functionality in hop-by-hop mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    /* test is oriented to 'hop-by-hop' mirroring */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);


    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);
    }

    /* Set configuration */
    prvTgfEgressMirrorHopByHopConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorHopByHopTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorHopByHopConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfEgressMirrorHopByHop , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer)
{
    /* test is oriented to 'hop-by-hop' mirroring */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;

    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfEgressMirrorHopByHop */
    UTF_TEST_CALL_MAC(prvTgfEgressMirrorHopByHop);

    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfEgressMirrorHopByHop */
        UTF_TEST_CALL_MAC(prvTgfEgressMirrorHopByHop);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test egress mirroring functionality in source-based mode:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and set Source-Based forwarding mode;
    send traffic and verify expected mirrored traffic on analyzer ports;
    disable Tx mirroring on analyzer ports;
    send traffic and verify no traffic on analyzer ports.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorSourceBased)
{
/********************************************************************
    Test 8.5 - Egress mirroring functionality in source-based mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E;
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
    }

    /* Set configuration */
    prvTgfEgressMirrorSourceBasedConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorSourceBasedTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorSourceBasedConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfEgressMirrorSourceBased , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer)
{
    /* test is oriented to 'src-based' mirroring - behavior of ingress pipe for DSA fields */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_SOURCE_BASED_E;

    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfEgressMirrorSourceBased */
    UTF_TEST_CALL_MAC(prvTgfEgressMirrorSourceBased);

    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfEgressMirrorSourceBased */
        UTF_TEST_CALL_MAC(prvTgfEgressMirrorSourceBased);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}

/*
    prvTgfEgressMirrorEndToEnd
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorEndToEnd)
{
/********************************************************************
    Egress mirroring functionality in End-To-End mode.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    if(currentMirrorMode == PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E)
    {
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;
        prvTgfMirrorConfigParamsSet(currentDsaTagEnable, currentDsaType, currentMirrorMode);
        currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
    }

    /* Set configuration */
    prvTgfEgressMirrorSourceBasedConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorSourceBasedTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorSourceBasedConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfEgressMirrorEndToEnd , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('extended')
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;

#if 0  /* 2 words  DSA not hold parameters of this mode */
    /* state that we run in DSA tag mode - 2 words */
    prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode);

    /* run the test - prvTgfEgressMirrorSourceBased */
    UTF_TEST_CALL_MAC(prvTgfEgressMirrorEndToEnd);
#endif /*0*/

    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    /* state that we run in DSA tag mode - 4 words */
    if(GT_TRUE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        /* run the test - prvTgfEgressMirrorSourceBased */
        UTF_TEST_CALL_MAC(prvTgfEgressMirrorEndToEnd);
    }

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test transforming from HopByHop mode to source-based mode for ingress mirroring:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Rx mirroring for HopByHop mode;
    send traffic and verify expected mirrored traffic on analyzer ports;
    set Hop-by-Hop forwarding mode;
    send traffic and verify expected mirrored traffic on analyzer ports.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorForwardingModeChange)
{
/********************************************************************
    Test 8.6 - Transforming from hop-by-hop mode to source-based mode and back
               to hop-by-hop mode for ingress mirroring.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    /* Set configuration */
    prvTgfIngressMirrorForwardingModeChangeConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorForwardingModeChangeTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorForwardingModeChangeConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test transforming from HopByHop mode to source-based mode for egress mirroring:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Tx mirroring for HopByHop mode;
    send traffic and verify expected mirrored traffic on analyzer ports;
    set source-based forwarding mode;
    send traffic and verify expected mirrored traffic on analyzer ports.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorForwardingModeChange)
{
/********************************************************************
    Test 8.7 - Transforming from hop-by-hop mode to source-based mode and back
               to hop-by-hop mode for egress mirroring.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    /* Set configuration */
    prvTgfEgressMirrorForwardingModeChangeConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorForwardingModeChangeTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorForwardingModeChangeConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN tag removal for ingress mirrored traffic:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Rx mirroring for HopByHop mode;
    enable VLAN tag removal of mirrored traffic;
    send traffic and verify expected mirrored UnTagged traffic;
    disable VLAN tag removal of mirrored traffic;
    send traffic and verify expected mirrored Tagged traffic.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorVlanTagRemoval)
{
/********************************************************************
    Test 8.8 - VLAN Tag Removal of ingress Mirrored Traffic.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E);

    /* Set configuration */
    prvTgfIngressMirrorVlanTagRemovalConfigurationSet();

    /* Generate traffic */
    prvTgfIngressMirrorVlanTagRemovalTrafficGenerate();

    /* Restore configuration */
    prvTgfIngressMirrorVlanTagRemovalConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test VLAN tag removal for egress mirrored traffic:
    configure VLAN, FDB entries;
    configure 2 analyzer interfaces and global Tx mirroring for HopByHop mode;
    enable VLAN tag removal of mirrored traffic;
    send traffic and verify expected mirrored UnTagged traffic;
    disable VLAN tag removal of mirrored traffic;
    send traffic and verify expected mirrored Tagged traffic.
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorVlanTagRemoval)
{
/********************************************************************
    Test 8.9 - VLAN Tag Removal of egress Mirrored Traffic.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E);

    /* Set configuration */
    prvTgfEgressMirrorVlanTagRemovalConfigurationSet();

    /* Generate traffic */
    prvTgfEgressMirrorVlanTagRemovalTrafficGenerate();

    /* Restore configuration */
    prvTgfEgressMirrorVlanTagRemovalConfigurationRestore();
}

/* AUTODOC: Test EPort Vs Physical port ingress mirroring.
    set eport e1 , ePort e2 both associated with physical port pA.
    use tti to classify eVlan = 0x10 from port pA as e1
    use tti to classify eVlan = 0x20 from port pA as e2
    (all others keep the default eport and not get new assignment)


case 1:
    set pA with rx mirror (analyzer 3)
    set e1 with rx mirror (analyzer 4)
    set e2 with rx mirror (analyzer 5)
    --> check:
        traffic from e1 generate mirroring --> to analyzer 4.
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA generate mirroring --> to analyzer 3.

case 2:
    unset e1 from rx mirror
    --> check:
        traffic from e1 generate mirroring --> to analyzer 3(due to physical).
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA generate mirroring --> to analyzer 3.

case 3:
    unset e2 from rx mirror
    --> check:
        traffic from e1 generate mirroring --> to analyzer 3(due to physical).
        traffic from e2 generate mirroring --> to analyzer 3(due to physical).
        other traffic from pA generate mirroring --> to analyzer 3.

case 4:
    set pA with rx mirror (analyzer 6)
    set e1 with rx mirror (analyzer 4)
    set e2 with rx mirror (analyzer 5)
    --> check:
        traffic from e1 generate mirroring --> to analyzer 6(due to physical).
        traffic from e2 generate mirroring --> to analyzer 6(due to physical).
        other traffic from pA generate mirroring --> to analyzer 3.

case 5:
    unset pA from rx mirror
    set e1 with rx mirror (analyzer 4)
    set e2 with rx mirror (analyzer 5)
    --> check:
        traffic from e1 generate mirroring --> to analyzer 4.
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA NOT generate mirroring.

case 6:
    unset e1 from rx mirror
    --> check:
        traffic from e1 NOT generate mirroring.
        traffic from e2 generate mirroring --> to analyzer 5.
        other traffic from pA NOT generate mirroring.

case 7:
    unset e2 from rx mirror
    --> check:
        traffic from e1 NOT generate mirroring.
        traffic from e2 NOT generate mirroring.
        other traffic from pA NOT generate mirroring.

*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorEportVsPhysicalPort)
{
    /* eports relevant only to eArch devices */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    prvTgfIngressMirrorEportVsPhysicalPortTest();
}

/* AUTODOC: Test EPort Vs Physical port egress mirroring.
    Test similar to prvTgfIngressMirrorEportVsPhysicalPort , but for 'egress mirror'
    and not for 'ingress mirror'
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorEportVsPhysicalPort)
{
    /* eports relevant only to eArch devices */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    prvTgfEgressMirrorEportVsPhysicalPortTest();
}

/* AUTODOC: Test eVlan based egress mirroring
    two cases:
    1. UC traffic
    2. Multi-destination traffic e.g. flooding (several ports send mirror replications)
*/
UTF_TEST_CASE_MAC(prvTgfEgressVlanMirroring)
{
    /* egress vlan mirroring supported on Puma3 and eArch , but test not implemented for Puma3 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    prvTgfEgressVlanMirroringTest();
}

/* Test Vlan based ingress mirroring */
UTF_TEST_CASE_MAC(prvTgfIngressVlanMirroring)
{
    prvTgfIngressVlanMirroringTest();
}

/* AUTODOC: Test ingress analyzer that is EPort that is mapped by the E2Phy to physical VIDX.
    With analyzer (egress ports) vidx members in STP blocking/forward .
    test also check that vidx analyzer ports must be in vlan ... otherwise filtered.
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorAnalyzerIsEportIsVidx)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    prvTgfIngressMirrorAnalyzerIsEportIsVidxTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfIngressMirrorAnalyzerIsEportIsVidx , with the 'ingress port'
            that set as (for ingress only) cascade port , and get traffic with DSA tag 'to_analyzer' ('eDSA')
*/
UTF_TEST_CASE_MAC(prvTgfIngressMirrorAnalyzerIsEportIsVidx_fromCascadePortDsaTagToAnalyzer)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* state that we run in DSA tag mode - 4 words */
    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;

    if(GT_FALSE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* run the test - prvTgfEgressMirrorSourceBased */
    UTF_TEST_CALL_MAC(prvTgfIngressMirrorAnalyzerIsEportIsVidx);

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: test permutation of prvTgfEgressMirrorAnalyzerIsEportIsVidx , with the 'ingress port'
            that set as (for ingress and egress) cascade port, and get traffic with DSA tag 'forward' ('eDSA')

            ingress packet from cascade port with FRW DSA to current device , eport
            on that eport the 'egress vlan translation' will modify vid0 from 5 to 300.
            that eport is set for 'egress mirror' to analyzer on other eport (that mapped by e2phy to vidx)
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorAnalyzerIsEportIsVidx_fromCascadePortDsaTagToAnalyzer)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_E_ARCH_CNS));

    /*check if the device support setting cascade on port for ingress different then for the egress */
    if(GT_FALSE == prvTgfCscdIngressPortEnableCheck(prvTgfDevNum))
    {
        prvUtfSkipTestsSet();
        return;
    }

/* Skip this test now for HIR due to indexing issue for mirroring port test */
    if (prvWrAppIsHirApp())
    {
        prvUtfSkipTestsSet();
        return;
    }

#ifdef CPSS_APP_PLATFORM_REFERENCE
        prvUtfSkipTestsSet();
        return;
#endif

    /* state that we run in DSA tag mode - 4 words */
    currentDsaTagEnable = GT_TRUE/*use DSA*/;
    currentDsaType = TGF_DSA_4_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E;

    if(GT_FALSE == prvTgfMirrorConfigParamsSet(currentDsaTagEnable,currentDsaType,currentMirrorMode))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* run the test */
    prvTgfEgressMirrorAnalyzerIsEportIsVidxTest();

    /* restore mode */
    currentDsaTagEnable = GT_FALSE/*do not use DSA*/;
    currentDsaType = TGF_DSA_2_WORD_TYPE_E;
    currentMirrorMode = PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_HOP_BY_HOP_E;
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: This Test will check for sniff outgoing counter.
            Configuration sets the vlan egress mirror to analyzer enabled/disabled.
            Packet is send and verify the sniff outgoing counter in the EREP unit
*/
UTF_TEST_CASE_MAC(prvTgfEgressMirrorToAnalyzerCounter)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));
    prvTgfEgressMirrorToAnalyzerCounterTest();
}

/*----------------------------------------------------------------------------------*/

/* AUTODOC: Test LC Ingress mirroring on Source Falcon using ERSPAN Type II encapsulation

    1.  TO_ANALYZER eDSA mirror packet from LC enters the device(Source Falcon) via ingress port as input.
    2.  Analyzer Target ePort(global ePort) in TO_ANALYZER eDSA triggers L2ECMP to get Target Falcon as output.
    3.  EPCL lookup on Source Falcon based on Word2 Reserved Bit(21) triggers SrcDevMirroring Thread in PHA.
    4.  PHA FW threads adds the Source Falcon Device ID to the outgoing TO_ANALYZER eDSA mirror packet.
    5.  Mirror packet exits the device via physical port#1 which is a cascde link in device map table to Target Falcon.
    6.  The final output packet from source Falcon(#2) is then treated as input to target Falcon (#0).
*/

UTF_TEST_CASE_MAC(prvTgfLcMirrorSourceFalconIngressErspan)
{
    /*
     * LC Ingress ERSPAN mirroring on Source Falcon use case
     */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfLcMirrorSourceFalconIngressErspanConfigSet();

    /* Generate traffic */
    prvTgfIngressMirrorSourceFalconErspanTest(ingressDirection);

    /* Packets output verification */
    prvTgfLcMirrorSourceFalconIngressErspanVerification(lcSourceDev, ingressDirection);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();

}

/*----------------------------------------------------------------------------------*/

/* AUTODOC: Test LC Ingress mirroring on Target Falcon using ERSPAN Type II encapsulation

    1.  TO_ANALYZER eDSA mirror packet from Source Falcon enters the device(Target Falcon) via ingress port.
    2.  Analyzer Target ePort(global ePort) in TO_ANALYZER eDSA triggers L2ECMP.
    3.  HA adds push tag after eDSA tag based on outgoing port push tag configuration.
    4.  EPCL lookup on Target Falcon based on Analyzer TrgDev and ePort triggers TrgDevMirroring IPv4 Thread in PHA.
    5.  PHA FW threads adds the necessary ERSPAN tunnel to the outgoing mirror packet.
    6.  Mirror packet exits the device via analyzer port (physical port#1) with ERSPAN tunnel
        and TO_ANALYZER 8B DSA
*/

UTF_TEST_CASE_MAC(prvTgfLcMirrorTargetFalconIngressErspan)
{
    /*
     * LC Ingress ERSPAN mirroring on Target Falcon use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfLcMirrorTargetFalconIngressErspanConfigSet();

    /* Generate traffic */
    prvTgfIngressMirrorTargetFalconErspanTest(ingressDirection,withVoQ);

    /* Packets output verification */
    prvTgfIngressMirrorErspanVerification(lcMirror,ingressDirection,protocolIpv4,withVoQ);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();

}

/*----------------------------------------------------------------------------------*/

/* AUTODOC: Test LC Ingress mirroring on Target Falcon using ERSPAN Type II IPV6 encapsulation

    1.  TO_ANALYZER eDSA mirror packet from Source Falcon enters the device(Target Falcon) via ingress port.
    2.  Analyzer Target ePort(global ePort) in TO_ANALYZER eDSA triggers L2ECMP.
    3.  HA adds push tag after eDSA tag based on outgoing port push tag configuration.
    4.  EPCL lookup on Target Falcon based on Analyzer TrgDev and ePort triggers TrgDevMirroring IPv6 Thread in PHA.
    5.  PHA FW threads adds the necessary ERSPAN tunnel to the outgoing mirror packet.
    6.  Mirror packet exits the device via analyzer port (physical port#1) with ERSPAN tunnel
        and TO_ANALYZER 8B DSA
*/

UTF_TEST_CASE_MAC(prvTgfLcMirrorTargetFalconIngressErspanIpv6)
{
    /*
     * LC Ingress ERSPAN mirroring on Target Falcon use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfLcMirrorTargetFalconIngressErspanIpv6ConfigSet();

    /* Generate traffic */
    prvTgfIngressMirrorTargetFalconErspanTest(ingressDirection,withVoQ);

    /* Packets output verification */
    prvTgfIngressMirrorErspanVerification(lcMirror,ingressDirection,protocolIpv6,withVoQ);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();

}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test Falcon Ingress mirroring on Source Falcon using ERSPAN Type II encapsulation

    1.  FORWARD eDSA packet enters the device via ingress port.
    2.  CC configuration converts LC source device/port to Falcon device/port for regular processing.
    3.  ePort mirroring is triggered based on ingress port and global ePort is assigned target port for analyzer.
    4.  EPCL lookup on Source Falcon based on Word2 Reserved Bit(21) triggers SrcDevMirroring Thread in PHA.
    5.  PHA FW threads adds the Source Falcon Device ID to the outgoing TO_ANALYZER eDSA mirror packet.
    5.  Mirror packet exits the device via physical port#1 which is a cascde link in device map table to Target Falcon.
    6.  The final output packet from source Falcon(#2) is then treated as input to target Falcon (#0).
*/

UTF_TEST_CASE_MAC(prvTgfFalconMirrorSourceFalconIngressErspan)
{
    /*
     * Falcon Ingress mirroring on Source Falcon use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFalconMirrorSourceFalconIngressErspanConfigSet();

    /* Generate traffic */
    prvTgfFalconIngressMirrorErspanTest();

    /* Packets output verification */
    prvTgfLcMirrorSourceFalconIngressErspanVerification(falconSourceDev,ingressDirection);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();

}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test Falcon Ingress mirroring on Target Falcon using ERSPAN Type II encapsulation

    1.  TO_ANALYZER eDSA mirror packet enters the device via ingress port.
    2.  Analyzer target ePort(global ePort) in TO_ANALYZER eDSA triggers L2ECMP.
    3.  HA adds push tag after eDSA tag based on outgoing port push tag configuration.
    4.  EPCL lookup on Target Falcon based on Analyzer TrgDev and ePort triggers TrgDevMirroring IPv4 Thread in PHA.
    5.  PHA FW threads adds the necessary ERSPAN tunnel to the outgoing mirror packet.
    6.  Mirror packet exits the device via analyzer port (physical port#1) with ERSPAN tunnel
        and TO_ANALYZER 8B DSA
*/

UTF_TEST_CASE_MAC(prvTgfFalconMirrorTargetFalconIngressErspan)
{
    /*
     * Falcon Ingress mirroring on Target Falcon use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFalconMirrorTargetFalconIngressErspanConfigSet();

    /* Generate traffic */
    prvTgfFalconIngressMirrorTargetFalconErspanTest(ingressDirection);

    /* Packets output verification */
    prvTgfIngressMirrorErspanVerification(falconMirror,ingressDirection,protocolIpv4,withVoQ);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test Falcon Ingress mirroring on Target Falcon using ERSPAN Type II encapsulation

    1.  TO_ANALYZER eDSA mirror packet enters the device via ingress port.
    2.  Analyzer target ePort(global ePort) in TO_ANALYZER eDSA triggers L2ECMP.
    3.  HA adds push tag after eDSA tag based on outgoing port push tag configuration.
    4.  EPCL lookup on Target Falcon based on Analyzer TrgDev and ePort triggers TrgDevMirroring IPv6 Thread in PHA.
    5.  PHA FW threads adds the necessary ERSPAN tunnel to the outgoing mirror packet.
    6.  Mirror packet exits the device via analyzer port (physical port#1) with ERSPAN tunnel
        and TO_ANALYZER 8B DSA
*/

UTF_TEST_CASE_MAC(prvTgfFalconMirrorTargetFalconIngressErspanIpv6)
{
    /*
     * Falcon Ingress mirroring on Target Falcon use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFalconMirrorTargetFalconIngressErspanIpv6ConfigSet();

    /* Generate traffic */
    prvTgfFalconIngressMirrorTargetFalconErspanTest(ingressDirection);

    /* Packets output verification */
    prvTgfIngressMirrorErspanVerification(falconMirror,ingressDirection,protocolIpv6,withVoQ);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();
}

/*----------------------------------------------------------------------------------*/

/* AUTODOC: Test LC Egress mirroring on Source Falcon using ERSPAN Type II encapsulation

    1.  TO_ANALYZER eDSA mirror packet from LC enters the device(Source Falcon) via ingress port as input.
    2.  Analyzer Target ePort(global ePort) in TO_ANALYZER eDSA triggers L2ECMP to get Target Falcon as output.
    3.  EPCL lookup on Source Falcon based on Word2 Reserved Bit(21) triggers SrcDevMirroring Thread in PHA.
    4.  PHA FW threads adds the Source Falcon Device ID to the outgoing TO_ANALYZER eDSA mirror packet.
    5.  Mirror packet exits the device via physical port#1 which is a cascde link in device map table to Target Falcon.
    6.  The final output packet from source Falcon(#2) is then treated as input to target Falcon (#0).
*/

UTF_TEST_CASE_MAC(prvTgfLcMirrorSourceFalconEgressErspan)
{
    /*
     * LC Ingress ERSPAN mirroring on Source Falcon use case
     */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfLcMirrorSourceFalconEgressErspanConfigSet();

    /* Generate traffic */
    prvTgfIngressMirrorSourceFalconErspanTest(egressDirection);

    /* Packets output verification */
    prvTgfLcMirrorSourceFalconIngressErspanVerification(lcSourceDev,egressDirection);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();
}

/*----------------------------------------------------------------------------------*/

/* AUTODOC: Test LC Egress mirroring on Target Falcon using ERSPAN Type II encapsulation

    1.  TO_ANALYZER eDSA mirror packet from Source Falcon enters the device(Target Falcon) via ingress port.
    2.  Analyzer Target ePort(global ePort) in TO_ANALYZER eDSA triggers L2ECMP.
    3.  HA adds push tag after eDSA tag based on outgoing port push tag configuration.
    4.  EPCL lookup on Target Falcon based on Analyzer TrgDev and ePort triggers TrgDevMirroring IPv4 Thread in PHA.
    5.  PHA FW threads adds the necessary ERSPAN tunnel to the outgoing mirror packet.
    6.  Mirror packet exits the device via analyzer port (physical port#1) with ERSPAN tunnel
        and TO_ANALYZER 8B DSA
*/

UTF_TEST_CASE_MAC(prvTgfLcMirrorTargetFalconEgressErspan)
{
    /*
     * LC Ingress ERSPAN mirroring on Target Falcon use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfLcMirrorTargetFalconEgressErspanConfigSet();

    /* Generate traffic */
    prvTgfIngressMirrorTargetFalconErspanTest(egressDirection,withVoQ);

    /* Packets output verification */
    prvTgfIngressMirrorErspanVerification(lcMirror,egressDirection,protocolIpv4,withVoQ);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();
}

/*----------------------------------------------------------------------------------*/

/* AUTODOC: Test LC Egress mirroring without VoQ on Target Falcon using ERSPAN Type II encapsulation

    1.  TO_ANALYZER eDSA mirror packet from Source Falcon enters the device(Target Falcon) via ingress port.
    2.  Analyzer Target ePort(global ePort) in TO_ANALYZER eDSA triggers L2ECMP.
    3.  HA adds push tag after eDSA tag based on outgoing port push tag configuration.
    4.  EPCL lookup on Target Falcon based on Analyzer TrgDev and ePort triggers TrgDevMirroring IPv4 Thread in PHA.
    5.  PHA FW threads adds the necessary ERSPAN tunnel to the outgoing mirror packet.
    6.  Mirror packet exits the device via analyzer port (physical port#1) with ERSPAN tunnel
        and TO_ANALYZER 8B DSA
*/

UTF_TEST_CASE_MAC(prvTgfLcMirrorTargetFalconWithoutVoQEgressErspan)
{
    /*
     * LC Ingress ERSPAN mirroring on Target Falcon use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfLcMirrorTargetFalconWithoutVoQEgressErspanConfigSet();

    /* Generate traffic */
    prvTgfIngressMirrorTargetFalconErspanTest(egressDirection,withoutVoQ);

    /* Packets output verification */
    prvTgfIngressMirrorErspanVerification(lcMirror,egressDirection,protocolIpv4, withoutVoQ);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();
}


/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test Falcon Egress mirroring on Source Falcon using ERSPAN Type II encapsulation

    1.  FORWARD eDSA packet enters the device via ingress port.
    2.  CC configuration converts LC source device/port to Falcon device/port for regular processing.
    3.  ePort mirroring is triggered based on ingress port and global ePort is assigned target port for analyzer.
    4.  EPCL lookup on Source Falcon based on Word2 Reserved Bit(21) triggers SrcDevMirroring Thread in PHA.
    5.  PHA FW threads adds the Source Falcon Device ID to the outgoing TO_ANALYZER eDSA mirror packet.
    5.  Mirror packet exits the device via physical port#1 which is a cascde link in device map table to Target Falcon.
    6.  The final output packet from source Falcon(#2) is then treated as input to target Falcon (#0).
*/

UTF_TEST_CASE_MAC(prvTgfFalconMirrorSourceFalconEgressErspan)
{
    /*
     * Falcon Egress mirroring on Source Falcon use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFalconMirrorSourceFalconEgressErspanConfigSet();

    /* Generate traffic */
    prvTgfFalconIngressMirrorErspanTest();

    /* Packets output verification */
    prvTgfLcMirrorSourceFalconIngressErspanVerification(falconSourceDev,egressDirection);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test Falcon Egress mirroring on Target Falcon using ERSPAN Type II encapsulation

    1.  TO_ANALYZER eDSA mirror packet enters the device via ingress port.
    2.  Analyzer target ePort(global ePort) in TO_ANALYZER eDSA triggers L2ECMP.
    3.  HA adds push tag after eDSA tag based on outgoing port push tag configuration.
    4.  EPCL lookup on Target Falcon based on Analyzer TrgDev and ePort triggers TrgDevMirroring IPv4 Thread in PHA.
    5.  PHA FW threads adds the necessary ERSPAN tunnel to the outgoing mirror packet.
    6.  Mirror packet exits the device via analyzer port (physical port#1) with ERSPAN tunnel
        and TO_ANALYZER 8B DSA
*/

UTF_TEST_CASE_MAC(prvTgfFalconMirrorTargetFalconEgressErspan)
{
    /*
     * Falcon Egress mirroring on Target Falcon use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFalconMirrorTargetFalconEgressErspanConfigSet();

    /* Generate traffic */
    prvTgfFalconIngressMirrorTargetFalconErspanTest(egressDirection);

    /* Packets output verification */
    prvTgfIngressMirrorErspanVerification(falconMirror,egressDirection,protocolIpv4,withoutVoQ);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test Falcon Ingress mirroring on Target Falcon using ERSPAN Type II encapsulation

    1.  TO_ANALYZER eDSA mirror packet enters the device via ingress port.
    2.  Analyzer target ePort(global ePort) in TO_ANALYZER eDSA triggers L2ECMP.
    3.  HA adds push tag after eDSA tag based on outgoing port push tag configuration.
    4.  EPCL lookup on Target Falcon based on Analyzer TrgDev and ePort triggers TrgDevMirroring IPv6 Thread in PHA.
    5.  PHA FW threads adds the necessary ERSPAN tunnel to the outgoing mirror packet.
    6.  Mirror packet exits the device via analyzer port (physical port#1) with ERSPAN tunnel
        and TO_ANALYZER 8B DSA
*/

UTF_TEST_CASE_MAC(prvTgfFalconMirrorTargetFalconEgressErspanIpv6)
{
    /*
     * Falcon Ingress mirroring on Target Falcon use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
        UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
        UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_AC5P_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfFalconMirrorTargetFalconIngressErspanIpv6ConfigSet();

    /* Generate traffic */
    prvTgfFalconIngressMirrorTargetFalconErspanTest(egressDirection);

    /* Packets output verification */
    prvTgfIngressMirrorErspanVerification(falconMirror,egressDirection,protocolIpv6,withoutVoQ);

    /* Restore configuration */
    prvTgfIngressMirrorErspanConfigRestore();
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test Enhanced Mirroring Priority for Falcon device.
   For each weight tested:
   1) Enables only the relevant mirroring/trapping.
   2) Generate traffic.
   3) Set weight to zero.
   4) Generate traffic.
   5) check statistics.
   */

UTF_TEST_CASE_MAC(prvTgfBrgEnhancedMirroringPriority)
{
/* Test only in GM */
#ifndef ASIC_SIMULATION
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    prvTgfBrgEnhancedMirroringPriorityTest();
#endif
}


/*----------------------------------------------------------------------------------*/
/* AUTODOC: Hawk EREP convert dropped packets to trap or mirror to analyzer
    Create VLAN[portIdx]: 3[2, 3];
    Set Port not Member in VLAN Drop counter mode;
    Enable ingress filtering on portIdx 0;
    Enable EREP drop packet mode to TRAP mode:
    Send tagged traffic with vlanId 3 on portIdx 0
        - vefify to get no traffic and verify drop counters
        - verify packets all packets are trapped to CPU
        - verify global TRAP outgoing counter are equal to dropped packets
    Enable EREP drop packet mode to mirror-to-analyzer mode and analyzer index:
        - vefify to get no traffic and verify drop counters
        - verify all packets are mirrored to analyzer port
        - verify global MIRROR outgoing counter are equal to dropped packets
    */
UTF_TEST_CASE_MAC(prvTgfHawkErepDropPacketToTrapOrMirror)
{
    /*
        1. Set configuration
        2. Generate traffic
        3. Restore configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_CPSS_PP_ALL_SIP6_10_CNS);

    prvTgfHawkErepDropToTrapOrMirrorTest();
}

/* AUTODOC: Hawk EREP convert dropped packets to trap or mirror to analyzer (with mirror trafic)
    Create VLAN 5 with untagged ports [1,2,3,4]
    Set analyzer interface index=0; analyzer devNum=0, port=0
    Enable Tx mirroring on port=2, analyzer index=0
    Disable analyzer port = 0
    Enable Egress Replicated Packet Drop mode
    Enable EREP drop packet mode to TRAP mode
    Learn packet on port=2
    Send tagged traffic with VLAN 5 to port=3
        - vefify to get no traffic on analyzer port and verify drop counters
        - verify all mirrored and dropped packets are trapped to CPU
        - verify global TRAP outgoing counter are equal to dropped packets
    Enable EREP drop packet mode to mirror-to-analyzer mode and analyzer index:
        - vefify to get no traffic on analyzer port and verify drop counters
        - verify all packets are mirrored to analyzer port
        - verify global MIRROR outgoing counter are equal to dropped packets
    */
UTF_TEST_CASE_MAC(prvTgfHawkErepDropPacketToTrapOrMirrorWithMirroring)
{
    /*
        1. Set configuration
        2. Generate traffic
        3. Restore configuration
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_CPSS_PP_ALL_SIP6_10_CNS);
    prvTgfHawkErepDropToTrapOrMirrorTestWithEgressMirroring();
}


/* AUTODOC: Test ingress Mirroring with SFLOW for Falcon device.
   1) Enable the mirroring with PHA.
   2) Generate traffic.
   3) check packet's SFLOW header fields for IPV4 tunnel encapsulation.
 */
UTF_TEST_CASE_MAC (prvTgfFalconMirrorIngressSflowIpv4)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_FALCON_E);

    /*config set*/
    prvTgfFalconMirrorIngressSflowIpv4ConfigurationSet();
    /*Test with traffic*/
    prvTgfFalconMirrorIngressSflowIpv4TrafficSend();
    /*config reset*/
    prvTgfFalconMirrorIngressSflowIpv4ConfigurationReset();

}

/* AUTODOC: Test ingress Mirroring with SFLOW for Falcon device.
   1) Enable the mirroring with PHA.
   2) Generate traffic.
   3) check packet's SFLOW header fields for IPV6 tunnel encapsulation.
 */
UTF_TEST_CASE_MAC (prvTgfFalconMirrorIngressSflowIpv6)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_FALCON_E);
    /*config set*/
    prvTgfFalconMirrorIngressSflowIpv6ConfigurationSet();
    /*Test with traffic*/
    prvTgfFalconMirrorIngressSflowIpv6TrafficSend();
    /*config reset*/
    prvTgfFalconMirrorIngressSflowIpv6ConfigurationReset();

}
/* AUTODOC: Test ingress Mirroring with multi SFLOW collectors for Falcon device.
   1) Enable the mirroring with PHA.
   2) Generate traffic.
   3) check mirrored packet's SFLOW header fields for IPV4 and IPv6 tunnel encapsulation.
 */
UTF_TEST_CASE_MAC (prvTgfFalconMirrorIngressSflowMultiTarget)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_FALCON_E);
    /*config set*/
    prvTgfFalconMirrorIngressSflowMultiTargetConfig();
    /*Test with traffic*/
    prvTgfFalconMirrorIngressSflowMultiTargetTraffic();
    /*config reset*/
    prvTgfFalconMirrorIngressSflowMultiTargetConfigReset();
}
/** AUTODOC: Test ingress Mirroring with IPv4+ERPSAN type II 
*            header encapsulation for Falcon device.
*   1) Enable the RX mirroring and trigger PHA.
*   2) Generate traffic.
*   3) check mirrored packet's GRE+ERSPAN header fields for IPV4 encapsulation.
**/
UTF_TEST_CASE_MAC (prvTgfFalconErspanSameDevMirrorIpv4Ingress)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_FALCON_E);
    /*config set*/
    prvTgfFalconErspanSameDevMirrorIpv4ConfigurationSet(CPSS_DIRECTION_INGRESS_E);
    /*Test with traffic*/
    prvTgfFalconErspanSameDevMirrorTraffic(CPSS_IP_PROTOCOL_IPV4_E);
    /*config reset*/
    prvTgfFalconErspanSameDevMirrorConfigurationReset(CPSS_DIRECTION_INGRESS_E);
}
/** AUTODOC: Test ingress Mirroring with IPv6+ERPSAN type II 
*            header encapsulation for Falcon device.
*   1) Enable the RX mirroring and trigger PHA.
*   2) Generate traffic.
*   3) check mirrored packet's GRE+ERSPAN header fields for IPV6 encapsulation.
**/
UTF_TEST_CASE_MAC (prvTgfFalconErspanSameDevMirrorIpv6Ingress)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_FALCON_E);
    /*config set*/
    prvTgfFalconErspanSameDevMirrorIpv6ConfigurationSet(CPSS_DIRECTION_INGRESS_E);
    /*Test with traffic*/
    prvTgfFalconErspanSameDevMirrorTraffic(CPSS_IP_PROTOCOL_IPV6_E);
    /*config reset*/
    prvTgfFalconErspanSameDevMirrorIpv6ConfigurationReset(CPSS_DIRECTION_INGRESS_E);
}
/** AUTODOC: Test egress Mirroring with IPv4+ERPSAN type II 
*            header encapsulation for Falcon device.
*   1) Enable the TX mirroring and trigger PHA.
*   2) Generate traffic.
*   3) check mirrored packet's GRE+ERSPAN header fields for IPV4 encapsulation.
**/
UTF_TEST_CASE_MAC (prvTgfFalconErspanSameDevMirrorIpv4Egress)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_FALCON_E);
    /*config set*/
    prvTgfFalconErspanSameDevMirrorIpv4ConfigurationSet(CPSS_DIRECTION_EGRESS_E);
    /*Test with traffic*/
    prvTgfFalconErspanSameDevMirrorTraffic(CPSS_IP_PROTOCOL_IPV4_E);
    /*config reset*/
    prvTgfFalconErspanSameDevMirrorConfigurationReset(CPSS_DIRECTION_EGRESS_E);
}
/** AUTODOC: Test egress Mirroring with IPv4+ERPSAN type II 
*            header encapsulation for Falcon device.
*   1) Enable the TX mirroring and trigger PHA.
*   2) Generate traffic.
*   3) check mirrored packet's GRE+ERSPAN header fields for IPV6 encapsulation.
**/
UTF_TEST_CASE_MAC (prvTgfFalconErspanSameDevMirrorIpv6Egress)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_FALCON_E);
    /*config set*/
    prvTgfFalconErspanSameDevMirrorIpv6ConfigurationSet(CPSS_DIRECTION_EGRESS_E);
    /*Test with traffic*/
    prvTgfFalconErspanSameDevMirrorTraffic(CPSS_IP_PROTOCOL_IPV6_E);
    /*config reset*/
    prvTgfFalconErspanSameDevMirrorIpv6ConfigurationReset(CPSS_DIRECTION_EGRESS_E);
}

/*
 * Configuration of tgfMirror suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfMirror)
    /*for device with multiple Rx/Tx analyzer */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorHopByHop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorSourceBased)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorEndToEnd)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorHighestIndexSelection)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorHopByHop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorHopByHop_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorSourceBased)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorSourceBased_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorEndToEnd)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorEndToEnd_fromCascadePortDsaTagToAnalyzer)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorForwardingModeChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorForwardingModeChange)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorVlanTagRemoval)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorVlanTagRemoval)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorEportVsPhysicalPort)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorEportVsPhysicalPort)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressVlanMirroring)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressVlanMirroring)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorAnalyzerIsEportIsVidx)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfIngressMirrorAnalyzerIsEportIsVidx_fromCascadePortDsaTagToAnalyzer)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorAnalyzerIsEportIsVidx_fromCascadePortDsaTagToAnalyzer)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfEgressMirrorToAnalyzerCounter)

    /* Ingress & Egress ERSPAN mirroring Type II use cases */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfLcMirrorSourceFalconIngressErspan)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfLcMirrorTargetFalconIngressErspan)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfLcMirrorTargetFalconIngressErspanIpv6)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconMirrorSourceFalconIngressErspan)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconMirrorTargetFalconIngressErspan)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconMirrorTargetFalconIngressErspanIpv6)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfLcMirrorSourceFalconEgressErspan)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfLcMirrorTargetFalconEgressErspan)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfLcMirrorTargetFalconWithoutVoQEgressErspan)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconMirrorSourceFalconEgressErspan)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconMirrorTargetFalconEgressErspan)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconMirrorTargetFalconEgressErspanIpv6)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgEnhancedMirroringPriority)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfHawkErepDropPacketToTrapOrMirror)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfHawkErepDropPacketToTrapOrMirrorWithMirroring)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconMirrorIngressSflowIpv4)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconMirrorIngressSflowIpv6)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconMirrorIngressSflowMultiTarget)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconErspanSameDevMirrorIpv4Ingress)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconErspanSameDevMirrorIpv4Egress)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconErspanSameDevMirrorIpv6Ingress)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfFalconErspanSameDevMirrorIpv6Egress)

UTF_SUIT_END_TESTS_MAC(tgfMirror)

