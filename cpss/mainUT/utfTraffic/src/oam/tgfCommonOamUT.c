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
* @file tgfCommonOamUT.c
*
* @brief Enhanced UTs for CPSS OAM
*
* @version   10
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <oam/prvTgfOamGeneral.h>
#include <oam/prvTgfOamDelayMeasurement.h>
#include <oam/prvTgfOamLossMeasurement.h>
#include <oam/prvTgfOamGenericKeepalive.h>
#include <oam/prvTgfOamCurrentHopTelemetryUseCase.h>
#include <oam/prvTgfOamEgressNodeUseCase.h>
#include <oam/prvTgfOamSrv6UseCase.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>

/*
    Set mode for the test: using TTI/IPCL for the OAM profile assignment.
    in any case the IPCL is used for other parameters setting.


    NOTE: ALL tests can run in this mode !!!
*/
static GT_BOOL useTtiForOam = GT_FALSE;


/*----------------------------------------------------------------------------*/
/*
    AUTODOC: MEG level check

    2.1.1 tgfOamMegLevelCheck

    Create Vlan 5 on ports with indexs 0, 1, 2, 3;
    Create trunk 6 with port members 0, 2;
    Create 4 TCAM rules:
       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
            trigger Vlan Id = 5, source port 1, ethertype 0x8902; MEL 3
       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
            trigger Vlan Id = 5, source port 1, ethertype 0x8902; MEL - Don't care
       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
            trigger Vlan Id = 5, Trunk Id = 6, MEL 5
       - PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E -
            trigger Vlan Id = 5, source port 2, ethertype 0x8902; MEL 3
    Create 4 IOAM, 4 IPLR billing entries binded to IPCL rules

    Create 4 TCAM rules:
      - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
           trigger DSA Command = FROM_CPU0; Opcode = 46
      - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
           trigger DSA Command = TO_CPU1; Opcode = 46
      - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
           trigger DSA Command = FROM_CPU1; Opcode = 47
      - PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E -
           trigger DSA Command = TO_CPU0; Opcode = 47
    Create 4 EOAM, 4 EPLR billing entries binded to EPCL rules

    Send OAM traffic on port 1 with different MEL values.
    Check that packets forwarded or trapped to CPU accordingly.
*/
UTF_TEST_CASE_MAC(tgfOamMegLevelCheck)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_AC3X_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfDefOamGeneralConfigurationSet();

    prvTgfDefOamGeneralMelTrafficTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Source interface verification

    2.1.2 tgfOamSourceInterfaceVerification

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Send OAM traffic on port 1 and trun 6 with different OAM source interface settings.
    Check that packets forwarded or trapped to CPU accordingly.

*/
UTF_TEST_CASE_MAC(tgfOamSourceInterfaceVerification)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    prvTgfDefOamGeneralConfigurationSet();

    prvTgfDefOamGeneralSourceInterfaceTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Dual-Ended Loss Measurement Packets Test

    2.1.3  tgfOamDualEndedLossMeasurementPacketCommand

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Send OAM traffic on port 1 with dual-ended LM opcode.
    Check that packets trapped to CPU.

*/
UTF_TEST_CASE_MAC(tgfOamDualEndedLossMeasurementPacketCommand)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /*
        Set mode for the test: using TTI/IPCL for the OAM profile assignment.
        in any case the IPCL is used for other parameters setting.
    */
    prvTgfOamGenericUseTtiForOam(useTtiForOam);

    prvTgfDefOamGeneralConfigurationSet();

    prvTgfDefOamGeneralDualEndedLossMeasurementTest();

    prvTgfDefOamGeneralConfigurationRestore();

    /* restore default values */
    prvTgfOamGenericUseTtiForOam(GT_FALSE);

}
/* AUTODOC : test tgfOamDualEndedLossMeasurementPacketCommand with TTI action that sets oamProcessingEn and oamIndex */
UTF_TEST_CASE_MAC(tgfOamDualEndedLossMeasurementPacketCommand_useTtiOamProfile)
{
    useTtiForOam = GT_TRUE;
    UTF_TEST_CALL_MAC(tgfOamDualEndedLossMeasurementPacketCommand);
    useTtiForOam = GT_FALSE;
}


/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Delay Measurement Verification

    2.2.1  tgfOamDelayMeasurementVerification

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Send OAM DMM packet on port 1;
    Wait 2 seconds;
    Send OAM DMR packet on port 2;
    Check recieved PDU header's field: MEL, DMM/DMR opcodes and timestamp results.
*/
UTF_TEST_CASE_MAC(tgfOamDelayMeasurementVerification)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /* Test use timestamp unit(TSU/PTP). There is no TSU/PTP in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamDelayMeasurementTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Loss Measurement Verification

    2.3.1  tgfOamLossMeasurementVerification

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Send 2 OAM LMM packet on port 1;
    Send 2 OAM LMR packet on port 2;
    Check recieved PDU header's field: MEL, LMM/LMR opcodes and billing counters.

*/
UTF_TEST_CASE_MAC(tgfOamLossMeasurementVerification)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamLossMeasurementTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Loss Measurement Verification (Green Counter Snapshot)

    2.3.2  tgfOamLossMeasurementGreenCntrSnapshotVerification

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Send 2 OAM LMM packet on port 1;
    Send 2 OAM LMR packet on port 2;
    Check recieved PDU header's field: MEL, LMM/LMR opcodes and billing counters.

*/
UTF_TEST_CASE_MAC(tgfOamLossMeasurementGreenCntrSnapshotVerification)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamLossMeasurementGreenCounterSnapshotTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Keepalive Age Verification

    2.4.1  tgfOamKeepAliveAgeState

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Set aging daemon basic configuration

    Phase 1:
    Set IOAM entry age state = 3
    Send 1 OAM CCM packet to port 1
    Check IOAM entry age state = 0

    Phase 2:
    Set IOAM entry age state = 0
    Send 1 OAM CCM packet to port 1
    Delay 5 seconds
    Check IOAM entry age state = 0; exception type - keepalive aging;
    exception counters > 0

    Reset aging daemon basic configuration
*/
UTF_TEST_CASE_MAC(tgfOamKeepAliveAgeState)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamKeepAliveAgeTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Keepalive Excess Verification

    2.4.2  tgfOamKeepAliveExcessDetection

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Set aging daemon basic configuration

    Phase 1:
    Send 8 OAM packets to port 1 with delay 1/2 second
    Check keepalive excess message counter = 7 (excess message threshold)
    Delay 5 seconds
    Check keepalive excess exception = 1
    Check keepalive excess message counter = 0 (cleared by daemon)

    Phase 2:
    Send 4 OAM packets to port 1 with delay 1 second
    Check keepalive excess message counter = 4
    Delay 5 seconds
    Check no keepalive excess exception
    Check keepalive excess message counter = 0 (cleared by daemon)

    Reset aging daemon basic configuration
*/
UTF_TEST_CASE_MAC(tgfOamKeepAliveExcessDetection)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamKeepAliveExcessDetectionTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Keepalive RDI bit checking

    2.4.3  tgfOamKeepAliveRdiBitChecking

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Set aging daemon basic configuration
    Enable RDI bit checking
    Phase 1:
    Set OAM packet RDI status == 1
    Send 1 packet
    Check 1 packet trapped to CPU, RDI exception == 1
    Phase 2:
    Set OAM packet RDI status == 0
    Send 1 packet
    Check 1 packet forwarded, no RDI exception
*/
UTF_TEST_CASE_MAC(tgfOamKeepAliveRdiBitChecking)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamKeepAliveRdiBitCheckingTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Keepalive period field checking

    2.4.4  tgfOamKeepAlivePeriodFieldChecking

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Set aging daemon basic configuration
    Enable period field checking
    Phase 1:
    Set keepalive period field
    Send 1 packet
    Check 1 packet trapped to CPU, period field exception == 1
    Phase 2:
    Reset keepalive period field
    Check 1 packet forwarded, no period field exception
*/
UTF_TEST_CASE_MAC(tgfOamKeepAlivePeriodFieldChecking)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamKeepAlivePeriodFieldCheckingTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

/*----------------------------------------------------------------------------*/
/*
    AUTODOC: Keepalive flow hash checking

    2.4.5  tgfOamKeepAliveFlowHashChecking

    Set common bridge configuration (see paragraph 2.1.1);
    Set common PCL configuration (see paragraph 2.1.1);
    Set egress PCL configuration (see paragraph 2.1.1);

    Set aging daemon basic configuration
    Enable period field checking
    Phase 1:
    Set keepalive flow hash field
    Enable lock hash value
    Send 1 packet
    Check 1 packet trapped to CPU, invalid hash exception == 1,
    OAM new flow hash value not locked
    Phase 2:
    Set keepalive flow hash field
    Disable lock hash value
    Check 1 packet trapped to CPU, invalid hash exception == 1,
    OAM new flow hash value locked, OAM flow lock hash enabled
*/
UTF_TEST_CASE_MAC(tgfOamKeepAliveFlowHashChecking)
{
 /*
    1. Set VLAN and IPCL configuration
    2. Generate traffic
    3. Restore VLAN and IPCL configuration
 */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfOamKeepAliveFlowHashCheckingTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

UTF_TEST_CASE_MAC(tgfIOamKeepAliveFlowHashVerificationBitSelection)
{
    /*
        1. Configure Keepalive engine
        2. Do the test
        2.1 Change src MAC to change Flow ID
        2.2 Set new combination of selected bits
        2.3 Send Packets
        2.4 Check if new Flow ID changed and it as expected

        Changes of src MAC designed to check the width of selected bits in hash
        start start bit
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    prvTgfDefOamGeneralConfigurationSet();

    tgfIOamKeepAliveFlowHashVerificationBitSelectionTest();

    prvTgfDefOamGeneralConfigurationRestore();
}

UTF_TEST_CASE_MAC(tgfEOamKeepAliveFlowHashVerificationBitSelection)
{
    /*
        1. Configure egress keepalive engine
        2. Do the test
        2.1 Change src MAC to change Flow ID
        2.2 Set expected Flow ID to MAC entry
        2.3 Send Packets
        2.4 Check if new Flow ID is as expected

        Changes of src MAC designed to check the width of selected bits in hash
        start start bit
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E);

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfEOamKeepAliveFlowHashVerificationBitSelectionConfigSet();

    tgfEOamKeepAliveFlowHashVerificationBitSelectionTest();

    tgfEOamKeepAliveFlowHashVerificationBitSelectionRestore();
}

UTF_TEST_CASE_MAC(tgfEOamCurrentHopTelemetryUseCaseVerification)
{
    /*
     * CurrentHop Telemetry use case
    */

    /* Set the PHA firmware threads type that are triggered in this test */
    CPSS_DXCH_PHA_THREAD_TYPE_ENT   threadTypeArr[2] = { CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_MIRRORING_E
                                                        ,CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E };

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Skip test if PHA firmware threads of CurrentHop Telemetry use case are not supported in current fw image */
    if(GT_FALSE == prvTgfPhaFwThreadValidityCheck(threadTypeArr,2))
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfOamCurrentHopTelemetryConfigSet();

    tgfOamCurrentHopTelemetryTest();

    tgfOamCurrentHopTelemetryConfigRestore();
}

UTF_TEST_CASE_MAC(tgfEOamEgressNodeUseCaseVerification)
{
   /*
    * OAM Egress Node use case
    */

    /* Set the PHA firmware threads type that are triggered in this test */
    CPSS_DXCH_PHA_THREAD_TYPE_ENT   threadTypeArr[2] = { CPSS_DXCH_PHA_THREAD_TYPE_INT_IOAM_EGRESS_SWITCH_E
                                                        ,CPSS_DXCH_PHA_THREAD_TYPE_IOAM_EGRESS_SWITCH_IPV6_E };

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* Skip test if PHA firmware threads of OAM Egress Node use case are not supported in current fw image */
    if(GT_FALSE == prvTgfPhaFwThreadValidityCheck(threadTypeArr,2))
    {
        prvUtfSkipTestsSet();
        return;
    }

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfOamEgressNodeConfigSet();

    tgfOamEgressNodeTest();

    tgfOamEgressNodeConfigRestore();
}

/*
    AUTODOC: Test case to verify "SRV6 One Segment PHA" which is getting applied on egress.
    Required data is attached by Tunnel start and PHA config.
    Config:
     1. Bridge config - To Route the packet from ingress to Egress
     2. Add TS IPv6 at egress Port
     3. PHA - Attach PHA Thread "1 segment" at Egress port
    Verify:
    - Verify SRH Header.
    - SRH Header last 4 Bytes should be 0
*/
UTF_TEST_CASE_MAC(tgfSrv6OneSegmentUseCaseVerification)
{
    /*
     * SRv6 one segment use case
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfSrv6OneSegmentConfigSet();

    tgfSrv6OneSegmentTest();

    tgfSrv6OneSegmentConfigRestore();
}

/*
    AUTODOC: Test case to verify "SRV6 Two Segment PHA" which is getting applied on egress.
    Required data is attached by "eDSA tag" & "Tunnel start profile" by a loop back port.

    Config:
     1. Bridge config - To Route the packet from ingress to Egress
        Enable loopback profile.
     2. Add TS Generic IPv6 and generic and Egress port.
     3. PHA - Assign PHA thread "SRv6 SECOND_PASS_2_SEGMENTS" to egress ePort.
              Assign PHA thread "SRv6 FIRST_PASS_2_3_SEGMENTS" to egress Port.
    Verify:
    - Verify SRH header
    - Verify 2 Segments.
*/
UTF_TEST_CASE_MAC(tgfSrv6TwoSegmentUseCaseVerification)
{
    /*
     * SRv6 two segment use case
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfSrv6TwoSegmentConfigSet();

    tgfSrv6TwoThreeSegmentTest(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_SEGMENTS_E);

    tgfSrv6TwoThreeSegmentConfigRestore();
}

/*
    AUTODOC: Test case to verify "SRV6 Three Segment PHA" which is getting applied on egress.
    Required data is attached by "eDSA tag" & "Tunnel start profile" by a loop back port.

    Config:
     1. Bridge config - To Route the packet from ingress to Egress
        Enable loopback profile.
     2. Add TS Generic IPv6 and generic and Egress port.
     3. PHA - Assign PHA thread "SRv6 SECOND_PASS_3_SEGMENTS" to egress ePort.
        Assign PHA thread "SRv6 FIRST_PASS_2_3_SEGMENTS" to egress Port.
    Verify:
    - Verify SRH header
    - Verify 3 Segments.
*/
UTF_TEST_CASE_MAC(tgfSrv6ThreeSegmentUseCaseVerification)
{
    /*
     * SRv6 Three segment use case
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfSrv6ThreeSegmentConfigSet();

    tgfSrv6TwoThreeSegmentTest(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_SEGMENTS_E);

    tgfSrv6TwoThreeSegmentConfigRestore();
}

/*
    AUTODOC: Test case to verify Egress mirroring meta data added by PHA thread.

    Config:
     1. Bridge Configuration - To Route the packet from ingress to transit port
        Enable loop back profile.
     2. Add TS Generic IPv6 at Egress port.
     3. Mirroring configuration: Enable TX mirroring on transit port.
     4. Enable cos profile and UDB config.
     5. Enable ingress mirroring as egress on analyzer port.
     3. PHA - Assign PHA thread "SRv6 SECOND_PASS_3_SEGMENTS" to egress ePort.
        Assign PHA thread "SRv6 FIRST_PASS_2_3_SEGMENTS" to egress Port.
    Verify:
    - Verify PHA processed packet at Egress for below params:
        Queue Length, Queue Limit, Group Quue index,
        Queue offset, Queue FB
        (Verify with configured UDB and tunnel start profile.)
*/
UTF_TEST_CASE_MAC(tgfEgressMirroringMetadataVerification)
{
    /*
     * Egress Mirroring Metadata verification
    */

    /* Set the PHA firmware thread type that is triggered in this test */
    CPSS_DXCH_PHA_THREAD_TYPE_ENT   threadTypeArr[1] = { CPSS_DXCH_PHA_THREAD_TYPE_EGRESS_MIRRORING_METADATA_E };

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* Skip test if PHA firmware threads of Egress Mirroring Metadata use case are not supported in current fw image */
    if(GT_FALSE == prvTgfPhaFwThreadValidityCheck(threadTypeArr,1))
    {
        prvUtfSkipTestsSet();
        return;
    }

    tgfEgressMirroringMetadataVerificationConfigSet();

    tgfEgressMirroringMetadataVerificationTrafficTest();

    tgfEgressMirroringMetadataVerificationConfigRestore();
}

/*
    AUTODOC: Test case to verify "SRV6 Best Effort Tunnel Parse on Border Node".
    Config:
     1. TTI config -  To assign descriptor CopyReserved and Source-ID based on Key fields DIP.SGT-ID and DIP.APP-ID
     2. IPCL config - To redirect packet to the outgoing egress interface
     3. EPCL config - To classify packets based on assigned incoming copy reserved value and trap packet to CPU with a specific CPU code
     4. Set CPU code table entry with the above CPU code for a specific traffic class
    Verify:
    - Verify CopyReserved assignment through EPCL classification and Rx SDMA counters of specific tc for EPCL actions TRAP to CPU
*/
UTF_TEST_CASE_MAC(tgfSrv6BestEffortTunnelParseOnBorderNodeVerification)
{
    /*
     * SRv6 Best Effort Tunnel Parse on Border Node use case
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfSrv6BestEffortTunnelParseOnBorderNodeConfigSet();

    tgfSrv6BestEffortTunnelParseOnBorderNodeTest();

    tgfSrv6BestEffortTunnelParseOnBorderNodeConfigRestore();
}

/*
    AUTODOC: Test case to verify "SRV6 Best Effort Tunnels Parse on Leaf Node".
    Config:
     1. TTI config - To assign forward command and redirect to egress interface
     2. PPU config - To assign descriptor source id and copy reserved based on DIP-SGT_ID and DIP-APP_ID match
     3. EPCL config - To classify packets based on assigned incoming copy reserved value and trap packet to CPU with a specific CPU code
     4. Set CPU code table entry with the above CPU code for a specific traffic class
    Verify:
    - Verify CopyReserved assignment through EPCL classification and Rx SDMA counters of specific tc for EPCL actions TRAP to CPU
*/
UTF_TEST_CASE_MAC(tgfSrv6BestEffortTunnelParseOnLeafNodeVerification)
{
    /*
     * SRv6 Best Effort Tunnel use case
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_IRONMAN_L_E | UTF_FALCON_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfSrv6BestEffortTunnelParseOnLeafNodeConfigSet();

    tgfSrv6BestEffortTunnelParseOnLeafNodeTest();

    tgfSrv6BestEffortTunnelParseOnLeafNodeConfigRestore();
}

/*
    AUTODOC: Test case to verify "SRV6 Best Effort Tunnels" which is getting applied on egress.
    Required data is attached by Tunnel start and PHA config.
    During init default PHA fw image is loaded and SRv6 Best Effort Tunnel thread is part of FW image #1
    and hence need to load that using cpssDxChPhaInit inorder to perform the test.
    Config:
     1. TTI config - Assign descriptor Source-ID and CopyReserved
     2. PCL config - Redirect packet to egress interface
     2. HA  - Add TS IPv6 at egress Port
     3. PHA - Attach PHA Thread "SRv6 BE Tunnel" at Egress port
    Verify:
    - Verify Tunnel Header Content.
*/
UTF_TEST_CASE_MAC(tgfSrv6BestEffortTunnelVerification)
{
    /*
     * SRv6 Best Effort Tunnel use case
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfSrv6BestEffortTunnelConfigSet();

    tgfSrv6BestEffortTunnelTest();

    tgfSrv6BestEffortTunnelConfigRestore();
}

/*
    AUTODOC: Test case to verify "SRV6 G-SID Tunnels One SRH Container" which is applied for egress packet.
    Required data is attached by Tunnel start and PHA config.
    Config:
     1. TTI config - Assign descriptor source-id and copy reserved
     2. PCL config - Redirect packet to egress interface
     3. Add TS IPv6 at egress interface
     4. PHA - Attach PHA thread "Srv6 G-SID One SRH Container" at egress interface
    Verify:
    - Verify SRH Header
    - Verify 1 SRH Container
*/
UTF_TEST_CASE_MAC(tgfSrv6GSidTunnelOneSrhContainerVerification)
{
    /*
     * SRv6 Best Effort Tunnel use case
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfSrv6GSidTunnelOneSrhContainerConfigSet();

    tgfSrv6GSidTunnelOneSrhContainerTest();

    tgfSrv6GSidTunnelOneSrhContainerConfigRestore();
}

/*
    AUTODOC: Test case to verify "SRV6 Two SRH Container PHA" which is getting applied on egress.
    Required data is attached by "eDSA tag" & "Tunnel start profile" by a loop back port.

    Config:
     1. Bridge config - To Route the packet from ingress to Egress
        Enable loopback profile.
     2. Add TS Generic IPv6 and generic and Egress port.
     3. PHA - Assign PHA thread "SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E" to first pass egress ePort.
              Assign PHA thread "SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E" to second pass egress Port.
    Verify:
    - Verify SRH header
    - Verify 2 SRH Containers.
*/
UTF_TEST_CASE_MAC(tgfSrv6GSidTunnelTwoSrhContainerVerification)
{
    /*
     * SRv6 two segment use case
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfSrv6GSidTunnelTwoSrhContainerConfigSet();

    tgfSrv6GSidTunnelTwoSrhContainerTest(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_2_CONTAINERS_E);

    tgfSrv6GSidTunnelTwoSrhContainerConfigRestore();
}

/*
    AUTODOC: Test case to verify "SRV6 Three SRH Container PHA" which is getting applied on egress.
    Required data is attached by "eDSA tag" & "Tunnel start profile" by a loop back port.

    Config:
     1. Bridge config - To Route the packet from ingress to Egress
        Enable loopback profile.
     2. Add TS Generic IPv6 and generic and Egress port.
     3. PHA - Assign PHA thread "SRV6_SRC_NODE_FIRST_PASS_2_3_CONTAINERS_E" to first pass egress ePort.
              Assign PHA thread "SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E" to second pass egress Port.
    Verify:
    - Verify SRH header
    - Verify 3 SRH Containers.
*/
UTF_TEST_CASE_MAC(tgfSrv6GSidTunnelThreeSrhContainerVerification)
{
    /*
     * SRv6 two segment use case
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfSrv6GSidTunnelThreeSrhContainerConfigSet();

    tgfSrv6GSidTunnelThreeSrhContainerTest(CPSS_DXCH_PHA_THREAD_TYPE_SRV6_SRC_NODE_SECOND_PASS_3_CONTAINERS_E);

    tgfSrv6GSidTunnelThreeSrhContainerConfigRestore();
}

/*
    AUTODOC: Test case to verify "SRV6 G-SID Tunnels CoC32 End Node Processing".
    Config:
     1. TTI config - Assign IPCL UDB Configuration table index and IPCL Configuration table index
     2. PCL config - Redirect packet to router next hop entry for PBR
     3. Router config - Assign target ePort associated with PHA thread
     4. PHA - Attach PHA thread "Srv6 G-SID Tunnels CoC32 end node processing" at egress interface
    Verify:
    - Verify SRH Header for SL
    - Verify Tunnel DIP for CoC32 G-SID and SI update
    - Verify L2 MAC
*/
UTF_TEST_CASE_MAC(tgfSrv6GSidTunnelCoc32EndNodeVerification)
{
    /*
     * SRv6 Best Effort Tunnel use case
    */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_IRONMAN_L_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    tgfSrv6GSidTunnelCoc32EndNodeConfigSet();

    tgfSrv6GSidTunnelCoc32EndNodeTest();

    tgfSrv6GSidTunnelCoc32EndNodeConfigRestore();
}

/* AUTODOC: Test case to verify "SRV6 DIP update". */
UTF_TEST_CASE_MAC(tgfSrv6TunnelEndNodeDipUpdateVerification)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E |
        UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_IRONMAN_L_E);

    tgfSrv6TunnelEndNodeDipUpdateVerificationTest();
}

/*
 * Configuration of tgfOam suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfOam)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamMegLevelCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamSourceInterfaceVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamDualEndedLossMeasurementPacketCommand)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamDualEndedLossMeasurementPacketCommand_useTtiOamProfile)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamDelayMeasurementVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamLossMeasurementVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamLossMeasurementGreenCntrSnapshotVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamKeepAliveAgeState)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamKeepAliveExcessDetection)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamKeepAliveRdiBitChecking)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamKeepAlivePeriodFieldChecking)
    UTF_SUIT_DECLARE_TEST_MAC(tgfOamKeepAliveFlowHashChecking)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIOamKeepAliveFlowHashVerificationBitSelection)
    UTF_SUIT_DECLARE_TEST_MAC(tgfEOamKeepAliveFlowHashVerificationBitSelection)
    UTF_SUIT_DECLARE_TEST_MAC(tgfEOamCurrentHopTelemetryUseCaseVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfEOamEgressNodeUseCaseVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6OneSegmentUseCaseVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6TwoSegmentUseCaseVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6ThreeSegmentUseCaseVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfEgressMirroringMetadataVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6BestEffortTunnelVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6BestEffortTunnelParseOnBorderNodeVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6BestEffortTunnelParseOnLeafNodeVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6GSidTunnelOneSrhContainerVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6GSidTunnelTwoSrhContainerVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6GSidTunnelThreeSrhContainerVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6GSidTunnelCoc32EndNodeVerification)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSrv6TunnelEndNodeDipUpdateVerification)
UTF_SUIT_END_TESTS_MAC(tgfOam)


