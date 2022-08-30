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
* @file tgfCommonCncUT.c
*
* @brief Enhanced UTs for CPSS CNC
*
* @version   17
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <common/tgfCncGen.h>
#include <cnc/prvTgfCncVlanL2L3.h>
#include <cnc/prvTgfAfCncIngressVlanClientIndexMode.h>
#include <cnc/prvTgfCncEgrQue.h>
#include <cnc/prvTgfCncGen.h>
#include <cnc/prvTgfCncTcam.h>
#include <cnc/prvTgfCncEgrPacketType.h>
#include <cnc/prvTgfCncHashClient.h>
#include <cnc/prvTgfCncEgressQuePassDropRecudedMode.h>

/* string for GM fails */
#define GM_TXQ_CNC_FAIL_STR     "JIRA : TXQ-1088 : CNC events are not generated in GM \n"

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test CNC INGRESS VLAN client Index mode using AutoFlow library:
    configure VLAN, FDB entries;
    configure CNC clients - set range and byte count mode;
    configure VLAN client index mode;
    send traffic;
    verify CNC counters increment respectively to index mode.
*/
UTF_TEST_CASE_MAC(tgfAfCncIngressVlanClientIndexMode)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    prvTgfAfCncIngressVlanClientIndexModeConfigure();

    prvTgfAfCncIngressVlanClientIndexModeTest();

    prvTgfAfCncIngressVlanClientIndexModeReset();
}



/*----------------------------------------------------------------------------*/
/* AUTODOC: Test CNC L2/L3 Ingress VLAN clients:
    configure VLAN, FDB entries;
    configure CNC clients - set range and byte count mode;
    send IPv4 traffic;
    verify CNC counters are equal to packet burst.
*/
UTF_TEST_CASE_MAC(tgfCncVlanL2L3)
{
    prvTgfCncTestVlanL2L3Configure();

    prvTgfCncTestVlanL2L3TrafficGenerateAndCheck();

    if(prvTgfResetModeGet() == GT_TRUE)
    {
        prvTgfResetAndInitSystem();
    }
    else
    {
        prvTgfCncTestCommonReset();
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test CNC counter format:
    configure VLAN, FDB entries;
    configure CNC clients - set range and byte count mode;
    send Ethernet traffic for each CNC mode;
    verify CNC counters are correct for each mode.
*/
UTF_TEST_CASE_MAC(tgfCncCounterFormatsTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Test */
    prvTgfCncTestVlanL2L3Configure();

    prvTgfCncTestVlanL2L3TrafficTestFormats();

    prvTgfCncTestCommonReset();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test CNC wrap around:
    configure VLAN, FDB entries;
    configure CNC clients - set range and byte count mode;
    set counter values (1 << numOfBits) - 1) - rize all bits;
    send Ethernet traffic for each CNC mode;
    verify CNC counters and wrap around indexes are correct.
*/
UTF_TEST_CASE_MAC(tgfCncWraparoundTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    prvTgfCncTestVlanL2L3Configure();

    prvTgfCncTestVlanL2L3TrafficTestWraparound();

    prvTgfCncTestCommonReset();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test CNC egress queue:
    configure VLAN, FDB entries;
    configure Egress Queue Pass/Drop CNC clients - set range and byte count mode;
        configure QoS entry and Trust mode;
    send Ethernet traffic;
    verify CNC counters.
*/
UTF_TEST_CASE_MAC(tgfCncEgrQueTest)
{
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC(GM_TXQ_CNC_FAIL_STR);

    prvTgfCncTestEgrQueConfigure();

    prvTgfCncTestEgrQueTrafficGenerateAndCheck();

    prvTgfCncTestCommonReset();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfCncGenEgrVlanPass
*/

UTF_TEST_CASE_MAC(prvTgfCncGenEgrVlanPass)
{
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC(GM_TXQ_CNC_FAIL_STR);

    /* Test */
    prvTgfCncGenEgrVlanTrafficGenerateAndCheck(GT_FALSE);

    prvTgfCncGenEgrVlanCfgRestore();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfCncGenEgrVlanDrop
*/

UTF_TEST_CASE_MAC(prvTgfCncGenEgrVlanDrop)
{
    CPSS_TBD_BOOKMARK_FALCON    /* Skip test until EGF eVlan CNC client is implemented at Falcon */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_CPSS_PP_ALL_SIP6_CNS);

    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC(GM_TXQ_CNC_FAIL_STR);

    /* Test */
    prvTgfCncGenEgrVlanTrafficGenerateAndCheck(GT_TRUE);

    prvTgfCncGenEgrVlanCfgRestore();
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfCncGenTxqPassTailDrop
*/

UTF_TEST_CASE_MAC(prvTgfCncGenTxqPassTailDrop)
{
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC(GM_TXQ_CNC_FAIL_STR);

    /* Test */
    prvTgfCncGenTxqTrafficGenerateAndCheck(GT_FALSE, GT_FALSE);
    prvTgfCncGenTxqCfgRestore(GT_FALSE, GT_FALSE);
}



/*----------------------------------------------------------------------------*/
/*
    prvTgfCncGenTxqPassCnMode
*/

UTF_TEST_CASE_MAC(prvTgfCncGenTxqPassCnMode)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E);

    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC(GM_TXQ_CNC_FAIL_STR);

    /* Test */
    prvTgfCncGenTxqTrafficGenerateAndCheck(GT_FALSE, GT_TRUE);
    prvTgfCncGenTxqCfgRestore(GT_FALSE, GT_TRUE);
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfCncGenTxqDropTailDrop
*/

UTF_TEST_CASE_MAC(prvTgfCncGenTxqDropTailDrop)
{
    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Test */
    prvTgfCncGenTxqTrafficGenerateAndCheck(GT_TRUE, GT_FALSE);

    prvTgfCncGenTxqCfgRestore(GT_TRUE, GT_FALSE);
#endif /*ASIC_SIMULATION*/
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfCncGenTxqDropCnMode
*/

UTF_TEST_CASE_MAC(prvTgfCncGenTxqDropCnMode)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E);

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Test */
    prvTgfCncGenTxqTrafficGenerateAndCheck(GT_TRUE, GT_TRUE);
    prvTgfCncGenTxqCfgRestore(GT_TRUE, GT_TRUE);
#endif /*ASIC_SIMULATION*/
}


/*----------------------------------------------------------------------------*/
/*
    prvTgfCncGenIPclLookup0
*/

UTF_TEST_CASE_MAC(prvTgfCncGenIPclLookup0)
{
    /* Test */
    prvTgfCncGenIPclTrafficGenerateAndCheck(0);

    prvTgfCncGenIPclCfgRestore(0);
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfCncGenIPclLookup1
*/

UTF_TEST_CASE_MAC(prvTgfCncGenIPclLookup1)
{
    /* Test */
    prvTgfCncGenIPclTrafficGenerateAndCheck(1);

    prvTgfCncGenIPclCfgRestore(1);
}

/*----------------------------------------------------------------------------*/
/*
    prvTgfCncGenIPclLookup2
*/

UTF_TEST_CASE_MAC(prvTgfCncGenIPclLookup2)
{
    if (PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.pcl.iPcl0Bypass ||
        PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(prvTgfDevNum))
    {
        /* not have third lookup*/
        SKIP_TEST_MAC;
    }
    /* Test */
    prvTgfCncGenIPclTrafficGenerateAndCheck(2);

    prvTgfCncGenIPclCfgRestore(2);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test CNC counter with egress PCL rule:
    configure VLAN, FDB entries;
    configure PCL rule to forward packets with specific counter;
    configure typical CNC configuration;
    send Ethernet traffic to match rule;
    verify CNC counters are correct.
*/
UTF_TEST_CASE_MAC(prvTgfCncGenEPcl)
{
    /* Test */
    prvTgfCncGenEPclTrafficGenerateAndCheck();

    prvTgfCncGenEPclCfgRestore();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test CNC counter with egress PCL rule:
    configure VLAN, FDB entries;
    configure TTI rule to forward packets with specific counter;
    configure typical CNC configuration;
    send Ethernet traffic to match rule;
    verify CNC counters are correct.
*/
UTF_TEST_CASE_MAC(prvTgfCncGenTti)
{
    /* Test */
    prvTgfCncGenTtiTrafficGenerateAndCheck();

    prvTgfCncGenTtiCfgRestore();
}

/*
    AUTODOC: prvTgfCnsTcamClientsTest1
    Test on TTI, IPCL0, IPCL1, IPCL2, EPCL all parallel sublookups
    in SIP5 and above devices TCAM.
    This tests assumes L2 byte counting mode for all counters
    and checks bytes couters according to L2 mode.
*/
UTF_TEST_CASE_MAC(prvTgfCnsTcamClientsTest1)
{
    GT_U32 notAppFamilyBmp;

    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    prvTgfCnsTcamClientsTest1();
}

/*
    AUTODOC: tgfCncEgrPacketTypePassDrop
    Test on EREP unit for CNC client Egress packet Typw Pass/drop client.
    Apllicable device : Falcon
    This test perfoms
        1. Configures Cnc Client egress packet type pass/drop client.
        2. Test different cnc index by setting
           egress PCL rule to Forward, mirror, trap, soft drop and hard drop packet command
           and configure cpu code for drop and mirror/trap case.
        3. Verify the counter entries.

*/
UTF_TEST_CASE_MAC(tgfCncEgrPacketTypePassDrop)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~UTF_CPSS_PP_ALL_SIP6_CNS));

    prvTgfCncTestEgrPacketTypeTrafficGenerateAndCheck();
}

/*
    AUTODOC: prvTgfCncGenQueueStatistics
    Test on PRE-Q unit for CNC client Queue statistics.
    Apllicable device : Falcon, this test is not applicable for simulation.
    This test perfoms
        1. Configures Cnc Client Queue Statistics.
        2. Configure QOS profile.
        3. Configure queue threshold.
        4. Close txq of the egress port
        5. Send packet to egress port.
        6. Enable txq of the egress port.
        7. Verify the counter entries.

*/
UTF_TEST_CASE_MAC(prvTgfCncGenQueueStatistics)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~UTF_CPSS_PP_ALL_SIP6_CNS));

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Test */
    prvTgfCncGenQueueStaticsTrafficGenerateAndCheck(GT_FALSE); /*test Queue stat with index calulated from txq port*/
    prvTgfCncGenQueueStaticsRestore(GT_FALSE);
#endif /*ASIC_SIMULATION*/

}


/*
    AUTODOC: prvTgfCncGenPortStatistics
    Test on PRE-Q unit for CNC client Queue statistics.
    Apllicable device : Falcon, this test is not applicable for simulation.
    This test perfoms
        1. Configures Cnc Client Queue Statistics.
        2. Configure QOS profile.
        3. Configure port threshold.
        4. Close txq of the egress port
        5. Send packet to egress port.
        6. Enable txq of the egress port.
        7. Verify the counter entries.

*/
UTF_TEST_CASE_MAC(prvTgfCncGenPortStatistics)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~UTF_CPSS_PP_ALL_SIP6_CNS));

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Test */
    prvTgfCncGenPortStaticsTrafficGenerateAndCheck();

    prvTgfCncGenPortStaticsRestore();
#endif /*ASIC_SIMULATION*/

}


/*UTF_TEST_CASE_MAC(prvTgfCncHashClient)
{
    prvTgfCncHashClientTest();
}*/
UTF_TEST_CASE_MAC(prvTgfCncHashClient)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~UTF_CPSS_PP_ALL_SIP6_CNS));
    prvTgfCncTestHashClient();

}


UTF_TEST_CASE_MAC(prvTgfCncEgressQuePassDropRecudedMode)
{
    /* test for SIP 6 and above */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~UTF_CPSS_PP_ALL_SIP6_CNS));

    /* asic simulation not support this --> skip */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    /* Test */
    prvTgfCncEgressPassDropRecudedModeQueIdTest();

#endif /*ASIC_SIMULATION*/
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test CNC PHA client:
    configure PCL rule and set action as flowId = 6;
    configure PHA CNC clients - set range and byte count mode;
    configure PHA client index mode;
    send traffic;
    verify CNC counters increment respectively to byteCount mode.
*/
UTF_TEST_CASE_MAC(prvTgfCncPhaClient)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~UTF_CPSS_PP_ALL_SIP6_10_CNS));
/*
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
*/
    /* Skip the test for GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    if(GT_FALSE == tgfCncIsSupportedClient(prvTgfDevNum,PRV_TGF_CNC_CLIENT_PHA_E))
    {
        SKIP_TEST_MAC;
    }

    prvTgfCncPhaClientConfigure(PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E);
    prvTgfCncPhaClientConfigure(PRV_TGF_CNC_BYTE_COUNT_MODE_L3_E);
}

/*
 * Configuration of tgfCnc suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfCnc)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCncVlanL2L3)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCncCounterFormatsTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCncWraparoundTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCncEgrQueTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenEgrVlanPass)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenEgrVlanDrop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenTxqPassTailDrop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenTxqPassCnMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenTxqDropTailDrop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenTxqDropCnMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenIPclLookup0)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenIPclLookup1)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenIPclLookup2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenEPcl)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenTti)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCnsTcamClientsTest1)
    UTF_SUIT_DECLARE_TEST_MAC(tgfAfCncIngressVlanClientIndexMode)
    UTF_SUIT_DECLARE_TEST_MAC(tgfCncEgrPacketTypePassDrop)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenQueueStatistics)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncGenPortStatistics)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncHashClient)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncEgressQuePassDropRecudedMode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCncPhaClient)
UTF_SUIT_END_TESTS_MAC(tgfCnc)


