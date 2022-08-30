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
* @file tgfCommonPtpUT.c
*
* @brief Enhanced UTs for CPSS PTP
*
* @version   4
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <common/tgfPtpGen.h>
#include <ptp/prvTgfPtpGen.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>


/* index of port in processing pipe 0 for BC3 device */
#define EGRESS_PORT_INDEX_PIPE_0 0
/* index of port in processing pipe 1 for BC3 device.
   Same pipe as above is used for single pipe devices */
#define EGRESS_PORT_INDEX_PIPE_1 3

UTF_TEST_CASE_MAC(tgfPtpAddTimeAfterPtpHeaderTest)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_BOBCAT2_E)

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /* Test use TSU functionality that is absent in the GM.
       Skip test for GM. */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    prvTgfPtpGenVidAndFdbDefCfgSet();

    prvTgfPtpGenAddTimeAfterPtpHeaderCfgSet();

    prvTgfPtpGenAddTimeAfterPtpHeaderTrafficGenerateAndCheck();

    prvTgfPtpGenCfgRestore();

    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

UTF_TEST_CASE_MAC(tgfPtpIngressCaptureTest)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_BOBCAT2_E)

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    prvTgfPtpGenVidAndFdbDefCfgSet();

    /* egress port index for PIPE1 */
    prvTgfPtpGenIngressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);

    prvTgfPtpGenTrafficGenerate();

    prvTgfPtpGenIngressCaptureQueue1Check(GT_TRUE);

    prvTgfPtpGenCfgRestore();

    prvTgfPtpGenVidAndFdbDefCfgRestore();

    prvTgfPtpGenIngressCaptureRestore(EGRESS_PORT_INDEX_PIPE_1);
}

static GT_U32 tgfPtpAddCorrectionTimeTestSkipOnSimulation = 1;

void tgfPtpAddCorrectionTimeTestSkipOnSimulationSet(GT_U32 skip)
{
    tgfPtpAddCorrectionTimeTestSkipOnSimulation = skip;
}

UTF_TEST_CASE_MAC(tgfPtpAddCorrectionTimeTest)
{
    if (tgfPtpAddCorrectionTimeTestSkipOnSimulation)
    {
        /* test checks nanoseconds resolution times - not supported by simulation */
        ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    }

    /* Test use timestamp unit(TSU/PTP). There is no TSU/PTP in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_BOBCAT2_E)

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    prvTgfPtpGenVidAndFdbDefCfgSet();

    prvTgfPtpGenAddCorrectedTimeCfgSet();

    prvTgfPtpGenAddCorrectionTimeTrafficCheck();

    prvTgfPtpGenAddCorrectedTimeCfgRestore();
}



UTF_TEST_CASE_MAC(tgfPtpIngressTimestampQueueReading)
{

  /* AUTODOC: test checks that timstemps entrys Ingress queues are reading in in both Pipe0 (Egress port index 0) and Pipe1 (Egress port index 3) without starvation
     by sendind packets to both pipes and expected to read each time from another pipe (RR)*/
  PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

  prvTgfPtpGenVidAndFdbDefCfgSet();

  /* egress port index for PIPE1 */
  prvTgfPtpGenIngressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE0 */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenIngressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE1 */
  prvTgfPtpGenFdbDefCfgSet(3);
  prvTgfPtpGenIngressCaptureCfgSet(3);
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE0 */
  prvTgfPtpGenFdbDefCfgSet(0);
  prvTgfPtpGenIngressCaptureCfgSet(0);
  prvTgfPtpGenTrafficGenerate();


  /* AUTODOC: read the packets from ingress , queue 1, one pipe at a time */
  prvTgfPtpGenIngressCaptureQueue1Check(GT_TRUE);
  prvTgfPtpGenIngressCaptureQueue1Check(GT_TRUE);
  prvTgfPtpGenIngressCaptureQueue1Check(GT_TRUE);
  prvTgfPtpGenIngressCaptureQueue1Check(GT_TRUE);
  /* NOTE : the Golden model not supporting dequeueing */
  if (GT_FALSE == prvUtfIsGmCompilation())
  {
      prvTgfPtpGenIngressCaptureQueue1Check(GT_FALSE);
  }

  prvTgfPtpGenCfgRestore();

  prvTgfPtpGenVidAndFdbDefCfgRestore();

}


UTF_TEST_CASE_MAC(tgfPtpEgressTimestampQueueReading)
{

  /* AUTODOC: test checks that timstemps entrys Egress queues are reading in both Pipe0 (Egress port index 0) and Pipe1 (Egress port index 3) without starvation
     by sendind packets to both pipes and expected to read each time from another pipe (RR)*/
  PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

  /* GM does not support the test */
  GM_NOT_SUPPORT_THIS_TEST_MAC

  prvTgfPtpGenEgressTimestempPortsConfig(EGRESS_PORT_INDEX_PIPE_0,EGRESS_PORT_INDEX_PIPE_1);

  prvTgfPtpGenVidAndFdbDefCfgSet();
  /* egress port index for PIPE1 */
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE0 */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE1 */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE0 */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();

  /* AUTODOC: read the packets from egress , queue 1, one pipe at a time */
  prvTgfPtpGenEgressCaptureQueue1Check(GT_TRUE);
  prvTgfPtpGenEgressCaptureQueue1Check(GT_TRUE);
  prvTgfPtpGenEgressCaptureQueue1Check(GT_TRUE);
  prvTgfPtpGenEgressCaptureQueue1Check(GT_TRUE);
  /* NOTE : the Golden model not supporting dequeueing */
  if (GT_FALSE == prvUtfIsGmCompilation())
  {
      prvTgfPtpGenEgressCaptureQueue1Check(GT_FALSE);
  }
  prvTgfPtpGenCfgRestore();

  prvTgfPtpGenVidAndFdbDefCfgRestoreEgress();

}


UTF_TEST_CASE_MAC(tgfPtpEgressTimestampQueueinterrupt)
{
    /* AUTODOC: test checks support on cpss unify event for egress ptp timestemps interrupts:
       new timestamp and full queue in per-port and ermrk queues.
       NOTE : full ermrk queue interrupt disables per port queue. */

  PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    CPSS_TBD_BOOKMARK_FALCON    /* Skip test until PTP Event support is implemented at Falcon */
    CPSS_TBD_BOOKMARK_AC5P      /* Skip test until PTP Event support is implemented at Hawk */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

  /* GM does not support the test */
  GM_NOT_SUPPORT_THIS_TEST_MAC

  prvTgfPtpGenEgressTimestempQueueInteruptsConfig(EGRESS_PORT_INDEX_PIPE_0,EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenVidAndFdbDefCfgSet();

  /* egress port index for PIPE1 */
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();

  /* check for interrupt*/
  prvTgfPtpGenGetEvent(CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,2,GT_TRUE);

  /* for BC3: egress port index for PIPE0 ,PIPE1 ,PIPE0 ,PIPE1 */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();

  prvTgfPtpGenGetEvent(CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,8,GT_TRUE);

  /* for BC3: egress port index for PIPE0 ,PIPE1 ,PIPE0 ,PIPE1 */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();

  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();
 /* ptp queue port index 3 is full  (ptp queue size is 4)
     ermrk ptp queue get the new timestemp   */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();
 /* ptp queue port index 0 is full (ptp queue size is 4)
      ermrk ptp queue get the new timestemp  */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();
  /* ermrk ptp port queue and egress ptp queue are full*/
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();
  /* ermrk ptp port queue and egress ptp queue are full*/
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();

  prvTgfPtpGenGetEvent(CPSS_PP_GTS_GLOBAL_FIFO_FULL_E, 4, GT_TRUE);
  prvTgfPtpGenGetEvent(CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,8,GT_TRUE);

  prvTgfPtpGenEgressTimestempQueueInteruptsRestore();

}

/* AUTODOC:
   Test for Timestamping of Mirrored To CPU packets.
       - Configure VLAN and FDB Entries.
       - Configure OAM engine based timestamping:
          - Configure IPCL entry to trigger OAM
          - Configure OAM entry with timestamp enable and OAM/PTP Offset
          - Configure Timestamp Configuration entry with AddTime action on offset 0
          - Configure Timestamp tag to be Extended on CPU port
          - Configure Timestamp global parameters
       - Send packet that matches IPCL entry.
       - Verify if packet Send to CPU and verify TS EtherType.
         Verify that new TS extended is after L2 header.
*/
UTF_TEST_CASE_MAC(tgfPtpTimestampTagOnMirroredToCPU)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        UTF_SIP6_GM_NOT_READY_SKIP_MAC("\nTest is skipped since OAM is not supported yet in GM\n");
    }

    prvTgfPtpGenVidAndFdbDefCfgSet();

    prvTgfPtpGenAddTimeStampTagAndMirrorSet();

    prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);

    prvTgfPtpGenTrafficAndCheckCpuGetTimeStampTag();

    prvTgfPtpGenAddTimeStampTagAndMirrorrRestore();
}

/* AUTODOC:
   Test for Ingress and Egress Timestamp action. The action is used for OAM based timestamping.
       - Configure VLAN and FDB Entries.
       - Configure OAM engine based timestamping:
          - Configure IPCL entry to trigger OAM
          - Configure OAM entry with timestamp enable and OAM/PTP Offset
          - Configure Timestamp Configuration entry with AddIngressEgressTime action on offset=0 and offset2=16
          - Configure Timestamp tag to be Extended on egress port
          - Configure Timestamp global parameters
       - Send packet that matches IPCL entry.
       - Verify if packet captured on egress port and verify TS EtherType.
         Verify that new TS extended is after L2 header.
*/
UTF_TEST_CASE_MAC(tgfPtpAddIngressEgressTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        UTF_SIP6_GM_NOT_READY_SKIP_MAC("\nTest is skipped since OAM is not supported yet in GM\n");
    }

    prvTgfPtpGenVidAndFdbDefCfgSet();

    prvTgfPtpGenIngressEgressCaptureCfgSet();

    prvTgfPtpGenAddIngressEgressTrafficGenerateAndCheck();

    prvTgfPtpGenIngressEgressCaptureRestore();
}

/* AUTODOC:
   Test for PTP Egress Exception Configuration
       - Configure VLAN FDB Entries.
       - Configure basic PTP Cofiguration to receive PTP packet.
       - COnfigure PTP Egress Execption
             In Case of "Invalid Incoming Piggyback Interrupt"
             Set Pkt Cmd = TRAP_TO_CPU,
                CPU CODE = CPSS_NET_PTP_HEADER_ERROR_E.
       - Send PTP packet with invalid timestamp format.
       - Verify if packet Send to CPU and verify CPU Code.
*/
UTF_TEST_CASE_MAC(tgfPtpEgressExceptionTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    prvTgfPtpGenVidAndFdbDefCfgSet();

    prvTgfPtpEgresssExceptionCfgSet();

    prvTgfPtpEgressExceptionTrafficGenerateAndCheck();

    prvTgfPtpEgressExceptionTestRestore();
}

/* AUTODOC:
   Test for Invalid Ptp packet Command and CPU code config
       - Configure VLAN FDB Entries.
       - Configure basic PTP Cofiguration to receive PTP packet Over UDP IPv4.
             --Enable PTP over UDP IPv4.
             --Set PTP UDP Destination port.
       - COnfigure PTP Egress Execption config with
             In Case of "Invalid PTP Interrupt"
             Set Pkt Cmd = TRAP_TO_CPU,
                CPU CODE = CPSS_NET_PTP_HEADER_ERROR_E.
       - Send PTP packet Over UDP IPv4 with invalid PTP header (only first 4 bytes).
            - "Invalid PTP interrupt" is generated and packet command and cpu code is set as per config.
       - Verify if packet Send to CPU and verify CPU Code.
*/
UTF_TEST_CASE_MAC(tgfPtpInvalidPtpTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    prvTgfPtpGenVidAndFdbDefCfgSet();

    prvTgfPtpInvalidPtpCfgSet();

    prvTgfPtpInvalidPtpTrafficGenerateAndCheck();

    prvTgfPtpInvalidPtpCfgRestore();
}

/* AUTODOC:
   Test for Capture Previous Mode check.
       -  Set the TOD to N sec by "cpssDxChPtpTaiTodSet"
       -  Enable Previous Mode Capture by "cpssDxChPtpTsuControlSet"
       -  Set the TOD to N+1000 sec by "cpssDxChPtpTaiTodSet"
       -  Read & verify the capture value  should be N+1 by cpssDxChPtpTsIngressTimestampQueueEntryRead
*/
UTF_TEST_CASE_MAC(tgfPtpCapturePreviousModeTest)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));
    /* GM does not support the test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    prvTgfPtpGenVidAndFdbDefCfgSet();

    prvTgfPtpCapturePreviousModeCfgAndVerifyCheck();

    prvTgfPtpCapturePreviousModeCfgRestore();
}

#if 0
UTF_TEST_CASE_MAC(tgfPtpTimeStampUDBTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    prvTgfPtpGenVidAndFdbDefCfgSet();

    prvTgfPtpGenTimeStampUDBCfgSet();

    prvTgfPtpGenTimeStampTrafficGenerateAndCheck();

    prvTgfPtpGenTimeStampUDBRestore();
}
#endif

/* AUTODOC:
   Test for PTP PiggyBack Configuration:                                                                         .
   In this mode the timestamp is piggybacked onto the 1588v2 packet, using the 4B <reserved> field in the PTP header.                                            .
   In this mode no tag is actually added to the packet.                                                                                                                                                              .                                          																	 .
       - Configure VLAN FDB Entries.
       - Configure basic PTP Cofiguration to receive PTP packet.
       - Configure on egress port piggyback enable.
       - Configure Timestamp Configuration entry on egress port with CaptureIngress action.
       - Send PTP packet.
       - Expect capture packet to have: timestamp tag in <resreved> field == timestamp tag in ingress queue entry.
*/
UTF_TEST_CASE_MAC(tgfPtpPiggyBackTest)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_BOBCAT2_E)

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /* Test use timestamp unit(TSU/PSU). There is no TSU/PUS in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    prvTgfPtpGenVidAndFdbDefCfgSet();

    prvTgfPtpGenIngressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);

    prvTgfPtpGenTsTagPortCfgPiggyBackSet(EGRESS_PORT_INDEX_PIPE_1);

    prvTgfPtpGenPiggyBackTimestampTrafficGenerateAndCheck();

    prvTgfPtpGenCfgRestore();

    prvTgfPtpGenTsTagPortCfgPiggyBackRestore(EGRESS_PORT_INDEX_PIPE_1);

    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

/*
 * Configuration of tgfPtp suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfPtp)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpAddTimeAfterPtpHeaderTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpIngressCaptureTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpAddCorrectionTimeTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpIngressTimestampQueueReading)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpEgressTimestampQueueReading)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpEgressTimestampQueueinterrupt)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpTimestampTagOnMirroredToCPU)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpAddIngressEgressTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpEgressExceptionTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpInvalidPtpTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpCapturePreviousModeTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpPiggyBackTest)

UTF_SUIT_END_TESTS_MAC(tgfPtp)


