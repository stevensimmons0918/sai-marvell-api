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
* @file tgfCommonMplsUT.c
*
* @brief Enhanced UTs for CPSS MPLS
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#include <common/tgfCommon.h>
#include <common/tgfIpGen.h>
#include <mpls/prvTgfMpls.h>
#include <mpls/prvTgfMplsSrEntropyLabelUseCase.h>
#include <mpls/prvTgfMplsUnifiedSRUseCase.h>


/*----------------------------------------------------------------------------*/
/* AUTODOC: Segment Routing with MPLS entropy label:
            Send MPLS packet with 8 Label to diffent ports,
            Bind PHA threads to different eport,
            Verify the egress packet with the ELI and EL, accoridng to the bound PHA thread configuration.
*/
UTF_TEST_CASE_MAC(tgfMplsSrEntropyLabel)
{
   PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
            UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
            UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
            UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* Set Segment Routing with Entropy Label configuration */
    prvTgfMplsSrEntropyLabelConfigurationSet();

    /* Generate traffic */
    prvTgfMplsSrEntropyLabelTrafficGenerate();

    /* Restore configuration */
    prvTgfMplsSrEntropyLabelConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Unified SR IPv6:
            Send packet to TS enabled ingress port,
            Bind Unified SR ipv6 PHA to egress port,
            Verify the egress packet with the labels copied from SIP of the tunnel,
            and verify the updates SIP as template value.
*/
UTF_TEST_CASE_MAC(tgfMplsUnifiedSrIpv6)
{
   /* Set the PHA firmware thread type that is triggered in this test */
   CPSS_DXCH_PHA_THREAD_TYPE_ENT   threadTypeArr[1] = { CPSS_DXCH_PHA_THREAD_TYPE_UNIFIED_SR_E };

   PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CH1_E |
            UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
            UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
            UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

   /* Skip test if PHA firmware threads of Unified SR IPv6 use case are not supported in current fw image */
   if(GT_FALSE == prvTgfPhaFwThreadValidityCheck(threadTypeArr,1))
   {
       prvUtfSkipTestsSet();
       return;
   }

    /* Set Unified SR IPv6 configuration */
    prvTgfUnifiedSrIpv6ConfigurationSet();

    /* Generate traffic */
    prvTgfUnifiedSrIpv6TrafficGenerate();

    /* Restore configuration */
    prvTgfUnifiedSrIpv6ConfigurationRestore();
}

/*
 * Configuration of tgfMpls suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfMpls)
    UTF_SUIT_DECLARE_TEST_MAC(tgfMplsSrEntropyLabel)
    UTF_SUIT_DECLARE_TEST_MAC(tgfMplsUnifiedSrIpv6)

UTF_SUIT_END_TESTS_MAC(tgfMpls)
