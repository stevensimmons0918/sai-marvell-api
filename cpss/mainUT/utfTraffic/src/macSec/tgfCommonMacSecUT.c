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
* @file tgfCommonMacSecUT.c
*
* @brief Enhanced UTs for CPSS MACSec
*
* @version   18
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <common/tgfCscdGen.h>
#include <macSec/prvTgfMacSecUseCase.h>


/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test MACSec Egress path

    1. Configure Egress MACSec devices: EIP-163 for classifer and EIP-164 for Transform
    2. Send first packet for Authentication and Encryption
    3. Send second packet for Authentication only (Disable Encryption)
    3. Verify that output packet include addition of SecTag (16B) + ICV(16B)
    4. Verify SecTag fields are set as expected
*/
UTF_TEST_CASE_MAC(prvTgfMacSecEgress)
{
    /*
     * MACSec Egress use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
                prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    /* Do not run this test for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfMacSecEgressConfigSet(GT_TRUE);

    /* Generate traffic */
    prvTgfMacSecEgressTest();

    /* Packets output verification */
    prvTgfMacSecEgressVerification();

    /* Restore configuration */
    prvTgfMacSecEgressConfigRestore();
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test MACSec Egress path without encryption

    1. Configure Egress MACSec devices: EIP-163 for classifer and EIP-164 for Transform
    2. Send first packet for Authentication and Encryption
    3. Send second packet for Authentication only (Disable Encryption)
    3. Verify that output packet include addition of SecTag (16B) + ICV(16B)
    4. Verify SecTag fields are set as expected
*/
UTF_TEST_CASE_MAC(prvTgfMacSecEgressWithoutEncryption)
{
    /*
     * MACSec Egress use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
                prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    /* Do not run this test for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfMacSecEgressConfigSet(GT_FALSE);

    /* Generate traffic */
    prvTgfMacSecEgressTest();

    /* Packets output verification */
    prvTgfMacSecEgressVerification();

    /* Restore configuration */
    prvTgfMacSecEgressConfigRestore();
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test MACSec Egress & Ingress path

    1. Configure Egress and Ingress MACSec devices: EIP-163 for classifer and EIP-164 for Transform
    2. Send first packet for Authentication and Encryption
    3. Send second packet for Authentication only (Disable Encryption)
    4. Verify that output packet is exactly the same as the one we send since:
       - Upon Egress SecTag and ICV are added
       - Upon Ingress SecTag and ICV are removed

*/
UTF_TEST_CASE_MAC(prvTgfMacSecEgressIngress)
{
    /*
     * MACSec Egress & Ingress use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
                prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    /* Do not run this test for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfMacSecEgressIngressConfigSet();

    /* Generate traffic */
    prvTgfMacSecEgressIngressTest();

    /* Packets output verification */
    prvTgfMacSecEgressIngressVerification();

    /* Restore configuration */
    prvTgfMacSecEgressIngressConfigRestore();
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test MACSec Events

    1. Configure Egress MACSec devices: EIP-163 for classifer and EIP-164 for Transform
    2. Send first packet for Authentication and Encryption
    3. Send second packet for Authentication only (Disable Encryption)
    3. Verify that output packet include addition of SecTag (16B) + ICV(16B)
    4. Verify SecTag fields are set as expected
*/
UTF_TEST_CASE_MAC(prvTgfMacSecEvents)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
                prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    /* Do not run this test for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

/* CaseI: Sequence number threshold less than the packet sequence number. Send
 *        traffic. Observed below events:
 *        CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E
 *        CPSS_PP_MACSEC_SA_PN_FULL_E */
    /* Save configuration parameters */
    prvTgfMacSecEventConfigStore();

    /* Set configuration */
    prvTgfMacSecEventConfigSet(3 /*sequence number*/, 2 /* threshold number*/, 2 /* test packet count */);
    prvTgfMacSecEgressConfigSet(GT_TRUE);

    /* Generate traffic */
    prvTgfMacSecEgressTest();

    /* Packets output verification */
    prvTgfMacSecEgressVerification();

    /* MACsec events verification */
    prvTgfMacSecEventsVerification(1);

    /* Restore configuration */
    prvTgfMacSecEventConfigRestore();
    prvTgfMacSecEgressConfigRestore();

/* CaseII: Sequence number threshold set to 0 and packet sequence number 1 less
 *        than the max number before rollover i.e. 0xFFFFFFFE
 *        Send traffic. Observed below events:
 *        CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E
 *        CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E */
    /* Save configuration parameters */
    prvTgfMacSecEventConfigStore();

    /* Set configuration */
    prvTgfMacSecEventConfigSet(0xFFFFFFFE /*sequence number*/, 0 /* threshold number*/, 2 /* test packet count */);
    prvTgfMacSecEgressConfigSet(GT_TRUE);

    /* Generate traffic */
    prvTgfMacSecEgressTest();

    /* Packets output verification */
    prvTgfMacSecEgressVerification();

    /* MACsec events verification */
    prvTgfMacSecEventsVerification(2);

    /* Restore configuration */
    prvTgfMacSecEventConfigRestore();
    prvTgfMacSecEgressConfigRestore();
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Test MACSec Egress & Ingress path

    1. Configure Egress and Ingress MACSec devices: EIP-163 for classifer and EIP-164 for Transform
    2. Send first packet for Authentication and Encryption
    3. Send second packet for Authentication only (Disable Encryption)
    4. Verify that output packet is exactly the same as the one we send since:
       - Upon Egress SecTag and ICV are added
       - Upon Ingress SecTag and ICV are removed

*/
UTF_TEST_CASE_MAC(prvTgfMacSecDiEvents)
{
    /*
     * MACSec Egress & Ingress use case
     */

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
                prvTgfDevNum, (~ (UTF_AC5P_E | UTF_AC5X_E)))

    /* Do not run this test for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration and Generate traffic */
    prvTgfMacSecDiEventsTest();

    /* Restore configuration */
    prvTgfMacSecDiEventsRestore();
}


/*
 * Configuration of tgfMacSec suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfMacSec)

    /* Test MACSec Egress */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfMacSecEgress)

    /* Test MACSec Egress Without Encryption */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfMacSecEgressWithoutEncryption)

    /* Test MACSec Egress & Ingress */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfMacSecEgressIngress)

    /* Test MACSec Events */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfMacSecEvents)

    /* Test MACSec DI Events */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfMacSecDiEvents)

UTF_SUIT_END_TESTS_MAC(tgfMacSec)

