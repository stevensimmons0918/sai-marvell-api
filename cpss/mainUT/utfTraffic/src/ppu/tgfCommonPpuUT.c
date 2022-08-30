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
* @file tgfCommonPpuUT.c
*
* @brief Enhanced UTs for CPSS PPU
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <ppu/prvTgfPpuGeneve.h>
#include <ppu/prvTgfPpuIpv6DipUpdate.h>

/*----------------------------------------------------------------------------*/
/* Test: tgfPpuGeneveTest
 *
 * Configure VLAN, TTI, PPU
 * Send Geneve packet with 1 TLV
 * Check modified eVlan and user priority
 */
UTF_TEST_CASE_MAC(tgfPpuGeneveTest)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E
        | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* set basic port and vlan configuration */
    prvTgfPpuGeneveBasicConfigSet();

    /* set PPU configuratio */
    prvTgfPpuGenevePpuConfigSet();

    /* Generate Traffic and compare results */
    prvTgfPpuGeneveTrafficGenerate(GT_FALSE /*isTunnelTerminated*/);

    /* Restore configuration*/
    prvTgfPpuGeneveRestore();
}

/*----------------------------------------------------------------------------*/
/* Test: tgfPpuGeneveTunnelTerminate
 *
 * Configure VLAN, TTI, PPU
 * Send Geneve packet with 1 TLV
 * Check modified eVlan and user priority
 */
UTF_TEST_CASE_MAC(tgfPpuGeneveTunnelTerminate)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E
        | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* set basic port and vlan configuration */
    prvTgfPpuGeneveBasicConfigSet();

    /* set PPU configuratio */
    prvTgfPpuGenevePpuConfigSet();

    /* set variable tunnel length configuration*/
    prvTgfPpuGeneveVariableTunnelLengthConfigSet();

    /* set tti configuration */
    prvTgfPpuGeneveTtiConfigSet();

    /* Generate Traffic and compare results */
    prvTgfPpuGeneveTrafficGenerate(GT_TRUE /*isTunnelTerminated*/);

    /* Restore configuration*/
    prvTgfPpuGeneveRestore();
}

/*----------------------------------------------------------------------------*/
/* Test: tgfPpuGeneveLoopbackTest
 *
 * Configure VLAN, TTI, PPU
 * Send Geneve packet with 6 TLVs
 * Check modified eVlan and user priority
 */
UTF_TEST_CASE_MAC(tgfPpuGeneveLoopbackTest)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E
        | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    prvTgfPpuGeneveLoopbackConfigure();

    prvTgfPpuGeneveLoopbackTrafficGenerate();

    prvTgfPpuGeneveLoopbackRestore();
}

/*----------------------------------------------------------------------------*/
/* Test: tgfPpuGeneveLoopbackProtectedWindow
 *
 * Configure VLAN, TTI, PPU
 * Send Geneve packet with 6 TLVs
 * Check modified eVlan and user priority
 */
UTF_TEST_CASE_MAC(tgfPpuGeneveLoopbackProtectedWindow)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E
        | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    prvTgfPpuGeneveLoopbackConfigure();

    prvTgfPpuGeneveProtectedWindowTest();

    prvTgfPpuGeneveLoopbackRestore();
}

/*----------------------------------------------------------------------------*/
/* Test: tgfPpuGeneveLoopbackErrorTest
 *
 * Configure VLAN, TTI, PPU
 * Send Geneve packet with 6 TLVs
 * Check modified eVlan and user priority
 */
UTF_TEST_CASE_MAC(tgfPpuGeneveLoopbackErrorTest)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E
        | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    prvTgfPpuGeneveLoopbackConfigure();

    prvTgfPpuGeneveLoopbackErrorTest();

    prvTgfPpuGeneveLoopbackRestore();
}

/*----------------------------------------------------------------------------*/
/* Test: tgfPpuGeneveLoopbackOorErrorTest
 *
 * Configure VLAN, TTI, PPU
 * Send Geneve packet with 6 TLVs
 * Check modified eVlan and user priority
 */
UTF_TEST_CASE_MAC(tgfPpuGeneveLoopbackOorErrorTest)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E
        | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    prvTgfPpuGeneveLoopbackConfigure();

    prvTgfPpuGeneveOorErrorTest();

    prvTgfPpuGeneveLoopbackRestore();
}

/*----------------------------------------------------------------------------*/
/* Test: tgfPpuGeneveLoopbackDebugCounters
 *
 * Configure VLAN, TTI, PPU
 * Send Geneve packet with 6 TLVs
 * Check modified eVlan and user priority
 */
UTF_TEST_CASE_MAC(tgfPpuGeneveLoopbackDebugCounters)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E
        | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    prvTgfPpuGeneveLoopbackConfigure();

    prvTgfPpuGeneveDebugCountersTest();

    prvTgfPpuGeneveLoopbackRestore();
}

UTF_TEST_CASE_MAC(tgfPpuIpv6DipUpdate)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_BOBCAT3_E
        | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* set basic port and vlan configuration */
    prvTgfPpuIpv6DipUpdateBaseConfigSet();

    /* set PPU configuratio */
    prvTgfPpuIpv6DipUpdatePpuConfigSet();

    prvTgfPpuIpv6DipUpdateRoutingConfigSet();

    /* Generate Traffic and compare results */
    prvTgfPpuIpv6DipUpdateTrafficGenerate();

    /* Restore configuration*/
    prvTgfPpuIpv6DipUpdateRestore();
}

/*
 * Configuration of tgfPpu suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfPpu)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPpuGeneveTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPpuGeneveTunnelTerminate)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPpuGeneveLoopbackTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPpuGeneveLoopbackProtectedWindow)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPpuGeneveLoopbackErrorTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPpuGeneveLoopbackOorErrorTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPpuGeneveLoopbackDebugCounters)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPpuIpv6DipUpdate)
UTF_SUIT_END_TESTS_MAC(tgfPpu)
