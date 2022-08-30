/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file tgfIpfixManagerUT.c
*
* @brief  Functional tests for IPFIX Manager Feature
*
* @version   1
********************************************************************************
*/

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssCommon/cpssPresteraDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfPacketGen.h>
#include <common/tgfConfigGen.h>
#include <gtOs/gtOsMem.h>

#include <ipfixManager/prvAppIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/prvCpssDxChIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/cpssDxChIpfixManager.h>
#include <ipfixManager/prvTgfIpfixManager.h>
#include <ipfixManager/prvAppIpfixManagerDbg.h>

#define PRV_TGF_IPFIX_MANAGER_HW_TESTS_DISABLE     1
#define IPFIX_MANAGER_SKIP_RUN_ON_SIMULATION       1
#define IPFIX_MANAGER_WAIT_TIME_BEFORE_CLEAN       2000
#define IPFIX_MANAGER_WAIT_TIME_AFTER_TRAFFIC_SEND 2000
#define IPFIX_MANAGER_TRAFFIC_SEND_INTERVAL ((PRV_APP_IPFIX_MGR_ENTRY_IDLE_TIMEOUT_CNS/2)*1000)

#if PRV_TGF_IPFIX_MANAGER_HW_TESTS_DISABLE
    #define PRV_TGF_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC SKIP_TEST_MAC
#else
    #define PRV_TGF_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC
#endif

/**
 * @brief Test Learning of new flows
 *          1.1: Ipfix Manager config and initialization
 *          1.2: Send 4 flows and check they are learned
 *          1.3: Send same 4 flows again and check they are not learned again
 *          1.4: Delete Ipfix manager and restore configurations
 */
UTF_TEST_CASE_MAC(tgfIpfixManagerFirstPacketsCheck)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      burstCount = 4;
    PRV_APP_IPFIX_MGR_DBG_COUNTERS_STC dbgCounters;
    GT_U32      portIdx = 1;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E|UTF_HARRIER_E);

#if IPFIX_MANAGER_SKIP_RUN_ON_SIMULATION /* Set it to 0 to run in simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = appDemoIpfixManagerMain(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerMain failed");


    /* 1.2: Send 4 flows and check they are learned */
    prvTgfIpfixManagerTrafficGenerate(portIdx, burstCount,GT_FALSE /* captureEnable*/);

    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixManagerDebugCountersDump();

    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.pktsFetchedByHost,
                                 "Debug counters check failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeEntries,
                                 "Debug counters check failed");


    /* 1.3: Send same 4 flows again and check they are not learned again */
    prvTgfIpfixManagerTrafficGenerate(portIdx, burstCount,GT_FALSE /* captureEnable*/);

    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_BEFORE_CLEAN);
    appDemoIpfixManagerDebugCountersDump();
    appDemoIpfixManagerStatisticsDump();

    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.pktsFetchedByHost,
                                 "Debug counters check failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeEntries,
                                 "Debug counters check failed");


    /* 1.4: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixManagerClear(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpfixManagerCreate failed");

}

/**
 * @brief Test whether data packets are being received
 *          1.1: Ipfix Manager config and initialization
 *          1.2: Send 4 flows from each of the 4 test ports and
 *               check they are learned
 *          1.3: Send same 4 flows from each of the 4 test ports again
 *               for 5 times and check they are not learned again and
 *               flow statistics are as expected
 *          1.4: Delete Ipfix manager and restore configurations
 */
UTF_TEST_CASE_MAC(tgfIpfixManagerDataPacketsCheck)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i, j;
    GT_U32      validFlows = 0;
    PRV_APP_IPFIX_MGR_DBG_COUNTERS_STC dbgCounters;
    PRV_APP_IPFIX_MGR_FLOWS_STC flow;
    GT_U32      portIdx;
    GT_U32      testPorts = 2;
    GT_U32      burstCount = 4;
    GT_U32      flowCount = 3;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E|UTF_HARRIER_E);

#if IPFIX_MANAGER_SKIP_RUN_ON_SIMULATION /* Set it to 0 to run in simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = appDemoIpfixManagerMain(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerMain failed");


    /* 1.2: Send 4 flows from each of the 4 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfIpfixManagerTrafficGenerate(portIdx, burstCount, GT_FALSE);
    }

    /* Wait until the flow is learned*/
    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixManagerDebugCountersDump();
    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), dbgCounters.pktsFetchedByHost,
                                 "Debug counters check failed");
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), dbgCounters.activeEntries,
                                 "Debug counters check failed");


    /* 1.3: Send same 4 flows from each of the 4 test ports again
            for 5 times and check they are not learned again and
            flow statistics are as expected
    */
    for (i = 0; i < flowCount; i++)
    {
        for (portIdx = 0; portIdx < testPorts; portIdx++)
        {
            prvTgfIpfixManagerTrafficGenerate(portIdx, burstCount, GT_FALSE);
        }
    }

    /* check debug counters */
    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixManagerDebugCountersDump();
    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), dbgCounters.pktsFetchedByHost,
                                 "Debug counters check failed");
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), dbgCounters.activeEntries,
                                 "Debug counters check failed");

    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_BEFORE_CLEAN);
    /* check flow statistics */
    validFlows = 0;
    for (i=0; i<CPSS_DXCH_MAX_PORT_GROUPS_CNS; i++)
    {
        for (j = 0; j < PRV_APP_IPFIX_MGR_FLOWS_PER_PORT_GROUP_MAX_CNS; j++)
        {
            rc = appDemoIpfixManagerFlowGet(i, j, &flow);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerFlowGet failed");
            if (flow.isactive == GT_FALSE)
            {
                continue;
            }
            validFlows++;

            UTF_VERIFY_EQUAL2_STRING_MAC(flowCount, flow.data.packetCount[0],
                  "ipfix data packet count didn't match for flow {%d,%d}", i,j);
            UTF_VERIFY_EQUAL2_STRING_MAC(i, (flow.data.flowId >> 12),
                  "port group didn't matchfor the flow {%d,%d}", i, j);
            UTF_VERIFY_EQUAL2_STRING_MAC(j, (flow.data.flowId & 0xFFF),
                  "flowId didn't match for flow {%d,%d}", i, j);
        }
    }
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), validFlows,
                                 "Flows count didn't match");

    /* Wait until the flow flow is learned */
    appDemoIpfixManagerDebugCountersDump();
    appDemoIpfixManagerStatisticsDump();

    /*1.4: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixManagerClear(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerClear failed");
}

/**
 * @brief Test whether data packets are being received
 *          1.1: Ipfix Manager config and initialization
 *          1.2: Send 4 flows from each of the 4 test ports and
 *               check they are learned
 *          1.3: Send same 4 flows from each of the 4 test ports again
 *               for 5 times and check they are not learned again and
 *               flow statistics are as expected
 *          1.4: Delete Ipfix manager and restore configurations
 */
UTF_TEST_CASE_MAC(tgfIpfixManagerEntriesAddDelete)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i, j;
    PRV_APP_IPFIX_MGR_DBG_COUNTERS_STC dbgCounters;
    PRV_APP_IPFIX_MGR_FLOWS_STC flow;
    GT_U32      portIdx;
    GT_U32      deletedFlows = 0;
    GT_U32      testPorts = 2;
    GT_U32      burstCount = 4;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E|UTF_HARRIER_E);

#if IPFIX_MANAGER_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = appDemoIpfixManagerMain(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerMain failed");


    /* 1.2: Send 4 flows from each of the 4 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfIpfixManagerTrafficGenerate(portIdx, burstCount, GT_FALSE);
    }

    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixManagerDebugCountersDump();
    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), dbgCounters.pktsFetchedByHost,
                                 "Debug counters check failed");
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), dbgCounters.activeEntries,
                                 "Debug counters check failed");


    /* 1.3: Delete all flows from first two tiles */
    deletedFlows = 0;
    for (i=0; i<CPSS_DXCH_MAX_PORT_GROUPS_CNS/2; i++)
    {
        for (j = 0; j < PRV_APP_IPFIX_MGR_FLOWS_PER_PORT_GROUP_MAX_CNS; j++)
        {
            rc = appDemoIpfixManagerFlowGet(i, j, &flow);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerFlowGet failed");
            if (flow.isactive == GT_FALSE)
            {
                continue;
            }

            rc = appDemoIpfixManagerFlowDelete(prvTgfDevNum, i, j);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerFlowDelete failed");

            deletedFlows++;
        }
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(testPorts/2 * burstCount, deletedFlows,
                                 "Number of deleted Flows are not as expected");
    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixManagerDebugCountersDump();
    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerDebugCountersGet failed");
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), dbgCounters.pktsFetchedByHost,
                                 "Debug counters check failed");
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount - deletedFlows),
                                 dbgCounters.activeEntries,
                                 "Debug counters check failed");


    /* 1.4: Send 4 flows from each of the 4 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfIpfixManagerTrafficGenerate(portIdx, burstCount, GT_FALSE);
    }

    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixManagerDebugCountersDump();
    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerDebugCountersGet failed");
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount + deletedFlows),
                                 dbgCounters.pktsFetchedByHost,
                                 "Debug counters check failed");
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount),
                                 dbgCounters.activeEntries,
                                 "Debug counters check failed");


    /* 1.5: Delete all flows and add again */
    rc = appDemoIpfixManagerFlowDeleteAll(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerFlowDeleteAll failed");

    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerDebugCountersGet failed");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeEntries,
                                 "Debug counters check failed for active entries");

    /* 1.6: Resend Send 4 flows from each of the 4 test ports and check they are learned */
    /* Reset debug counters */
    rc = appDemoIpfixManagerDebugCountersReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerDebugCountersReset failed");

    /* Send flows and check debug counters */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfIpfixManagerTrafficGenerate(portIdx, burstCount, GT_FALSE);
    }

    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixManagerDebugCountersDump();
    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), dbgCounters.pktsFetchedByHost,
                                 "Debug counters check failed for pkts fetched by host");
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), dbgCounters.activeEntries,
                                 "Debug counters check failed for active entries");

    /* Wait until the flow flow is learned */
    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_BEFORE_CLEAN);
    appDemoIpfixManagerDebugCountersDump();
    appDemoIpfixManagerStatisticsDump();


    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixManagerClear(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerClear failed");
}

UTF_TEST_CASE_MAC(tgfIpfixManagerAging)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx;
    GT_U32      testPorts = 2;
    GT_U32      burstCount = 4;
    PRV_APP_IPFIX_MGR_DBG_COUNTERS_STC dbgCounters;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E|UTF_HARRIER_E);

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    /* 1.1: Ipfix Manager config and initialization */
    rc = appDemoIpfixManagerMain(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerMain failed");

    /* Enable Aging */
    rc = appDemoIpfixManagerAgingEnable(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerAgingEnable failed");

    /* 1.2: Send 4 flows from each of the 2 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfIpfixManagerTrafficGenerate(portIdx, burstCount, GT_FALSE);
        cpssOsTimerWkAfter(IPFIX_MANAGER_TRAFFIC_SEND_INTERVAL);
    }

    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_AFTER_TRAFFIC_SEND);
    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixManagerDebugCountersDump();
    appDemoIpfixManagerStatisticsDump();
    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts * burstCount), dbgCounters.pktsFetchedByHost,
                                 "Debug counters check failed for pkts fetched by host");
    UTF_VERIFY_EQUAL0_STRING_MAC((testPorts/2 * burstCount), dbgCounters.activeEntries,
                                 "Debug counters check failed for active entries");

    cpssOsTimerWkAfter(IPFIX_MANAGER_TRAFFIC_SEND_INTERVAL);
    cpssOsTimerWkAfter(IPFIX_MANAGER_WAIT_TIME_BEFORE_CLEAN);
    appDemoIpfixManagerDebugCountersDump();
    appDemoIpfixManagerStatisticsDump();
    rc = appDemoIpfixManagerDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeEntries,
                                 "Debug counters check failed for active entries");


    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixManagerClear(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixManagerClear failed");

}

UTF_SUIT_BEGIN_TESTS_MAC(tgfIpfixManager)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixManagerFirstPacketsCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixManagerDataPacketsCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixManagerEntriesAddDelete)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixManagerAging)
UTF_SUIT_END_TESTS_MAC(tgfIpfixManager)
