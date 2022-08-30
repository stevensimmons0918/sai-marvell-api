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
* @file tgfCommonIpfixUT.c
*
* @brief Enhanced UTs for CPSS IPFIX
*
* @version   15
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <ipfix/prvTgfBasicIngressPclKeys.h>
#include <ipfix/prvTgfBasicTests.h>
#include <ipfix/prvTgfAging.h>
#include <ipfix/prvTgfWraparound.h>
#include <ipfix/prvTgfDropMode.h>
#include <ipfix/prvTgfSampling.h>
#include <ipfix/prvTgfTimersUpload.h>
#include <ipfix/prvTgfPortGroupEntry.h>
#include <ipfix/prvTgfPortGroupAging.h>
#include <ipfix/prvTgfPortGroupWraparound.h>
#include <ipfix/prvTgfPortGroupAlarms.h>

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixHelloTest
*/
UTF_TEST_CASE_MAC(tgfIpfixHelloTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    /* following tests do similar logic. Skip this test to save running time on GM. */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    /* SIP_5 GMs do not support IPFIX properly  */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("SIP_5 GM does not support IPFIX\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixHelloTestInit,              /* Set configuration */
                                 prvTgfIpfixHelloTestTrafficGenerate,   /* Generate traffic  */
                                 prvTgfIpfixHelloTestRestore);          /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixFirstIngressTest
*/
UTF_TEST_CASE_MAC(tgfIpfixFirstIngressTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* SIP_5 GMs do not support IPFIX properly  */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("SIP_5 GM does not support IPFIX\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixFirstIngressTestInit,         /* Set configuration */
                                 prvTgfIpfixIngressTestTrafficGenerate,   /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);          /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixFirstUseAllIngressTest
*/
UTF_TEST_CASE_MAC(tgfIpfixFirstUseAllIngressTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* SIP_5 GMs do not support IPFIX properly  */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("SIP_5 GM does not support IPFIX\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixFirstUseAllIngressTestInit, /* Set configuration */
                                 prvTgfIpfixIngressTestTrafficGenerate, /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);        /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixSecondUseAllIngressTest
*/
UTF_TEST_CASE_MAC(tgfIpfixSecondUseAllIngressTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* SIP_5 GMs do not support IPFIX properly  */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("SIP_5 GM does not support IPFIX\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixSecondUseAllIngressTestInit,    /* Set configuration */
                                 prvTgfIpfixIngressTestTrafficGenerate,     /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);            /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixTimestampVerificationTest
*/
UTF_TEST_CASE_MAC(tgfIpfixTimestampVerificationTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_DXCH_E);

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* SIP_5 GMs do not support IPFIX properly  */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("SIP_5 GM does not support IPFIX\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixTimestampVerificationTestInit,              /* Set configuration */
                                 prvTgfIpfixTimestampVerificationTestTrafficGenerate,   /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);                        /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixAgingTest
*/
UTF_TEST_CASE_MAC(tgfIpfixAgingTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_SKIP_MAC("IPFIX Aging daemon isn't implemented in GM\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixAgingTestInit,              /* Set configuration */
                                 prvTgfIpfixAgingTestTrafficGenerate,   /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);        /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixWraparoundFreezeTest
*/
UTF_TEST_CASE_MAC(tgfIpfixWraparoundFreezeTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("IPFIX is not alligned with GM yet\n")

    /* Set configuration */
    prvTgfIpfixWraparoundTestInit();

    /* Generate traffic */
    prvTgfIpfixWraparoundFreezeTestTrafficGenerate();

    /* Restore configuration */
    prvTgfIpfixWraparoundTestRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixWraparoundClearTest
*/
UTF_TEST_CASE_MAC(tgfIpfixWraparoundClearTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("IPFIX is not alligned with GM yet\n")

    /* Set configuration */
    prvTgfIpfixWraparoundTestInit();

    /* Generate traffic */
    prvTgfIpfixWraparoundClearTestTrafficGenerate();

    /* Restore configuration */
    prvTgfIpfixWraparoundTestRestore();
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixWraparoundMaxBytesTest
*/
UTF_TEST_CASE_MAC(tgfIpfixWraparoundMaxBytesTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("IPFIX is not alligned with GM yet\n")

    /* Set configuration */
    prvTgfIpfixWraparoundTestInit();

    /* Generate traffic */
    prvTgfIpfixWraparoundMaxBytesTestTrafficGenerate();

#ifdef ASIC_SIMULATION
    /* Restore configuration */
    prvTgfIpfixWraparoundTestRestore();
#endif
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixAgingTest
*/
UTF_TEST_CASE_MAC(tgfIpfixDropModeSimpleTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* SIP_5 GMs do not support IPFIX properly  */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("SIP_5 GM does not support IPFIX\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixDropModeSimpleTestInit,             /* Set configuration */
                                 prvTgfIpfixDropModeSimpleTestTrafficGenerate,  /* Generate traffic  */
                                 prvTgfIpfixIngressTestRestore);                /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixSamplingSimpleTest
*/
UTF_TEST_CASE_MAC(tgfIpfixSamplingSimpleTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* SIP_5 GMs do not support IPFIX properly  */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("SIP_5 GM does not support IPFIX\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixSamplingSimpleTestInit,             /* Set configuration */
                                 prvTgfIpfixSamplingSimpleTestTrafficGenerate,  /* Generate traffic  */
                                 prvTgfIpfixSamplingTestRestore);               /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixSamplingToCpuTest
*/
UTF_TEST_CASE_MAC(tgfIpfixSamplingToCpuTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* SIP_5 GMs do not support IPFIX properly  */
    UTF_SIP5_NOT_SIP6_GM_NOT_READY_SKIP_MAC("SIP_5 GM does not support IPFIX\n")

    PRV_TGF_SKIP_PASS_OK_LONG_TEST_GM_MAC(UTF_FALCON_E);

    prvTgfIpfixTrafficGenManager(prvTgfIpfixSamplingToCpuTestInit,              /* Set configuration */
                                 prvTgfIpfixSamplingToCpuTestTrafficGenerate,   /* Generate traffic  */
                                 prvTgfIpfixSamplingTestRestore);               /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixAlarmEventsTest
*/
UTF_TEST_CASE_MAC(tgfIpfixAlarmEventsTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_SKIP_MAC("IPFIX Alarm logic is absent in GM\n")

    prvTgfIpfixTrafficGenManager(prvTgfIpfixAlarmEventsTestInit,            /* Set configuration */
                                 prvTgfIpfixAlarmEventsTestTrafficGenerate, /* Generate traffic  */
                                 prvTgfIpfixSamplingTestRestore);           /* Restore configuration */

}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixTimersUploadAbsoluteTest
*/
UTF_TEST_CASE_MAC(tgfIpfixTimersUploadAbsoluteTest)
{
/********************************************************************
    1. Upload timers
    2. Restore
********************************************************************/

    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_SKIP_MAC("IPFIX timers aren't implemented in GM\n")

    /* Upload timers */
    prvTgfIpfixTimersUploadAbsoluteTest();

    /* Restore */
    prvTgfIpfixTimersUploadRestoreByClear();
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixTimersUploadIncrementalTest
*/
UTF_TEST_CASE_MAC(tgfIpfixTimersUploadIncrementalTest)
{
/********************************************************************
    1. Upload timers
    2. Restore
********************************************************************/

    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_SKIP_MAC("IPFIX timers aren't implemented in GM\n")

    /* Upload timers */
    prvTgfIpfixTimersUploadIncrementalTest();

    /* Restore */
    prvTgfIpfixTimersUploadRestoreByClear();
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixPortGroupEntryTest
*/
UTF_TEST_CASE_MAC(tgfIpfixPortGroupEntryTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_CPSS_PP_E_ARCH_CNS);
    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")


    prvTgfIpfixPortGroupEntryTestInit();            /* Set configuration */
    prvTgfIpfixPortGroupEntryTestTrafficGenerate(); /* Generate traffic  */
    prvTgfIpfixPortGroupEntryTestRestore();         /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixPortGroupAgingTest
*/
UTF_TEST_CASE_MAC(tgfIpfixPortGroupAgingTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixPortGroupEntryTestInit();            /* Set configuration */
    prvTgfIpfixPortGroupAgingTestTrafficGenerate(); /* Generate traffic  */
    prvTgfIpfixPortGroupEntryTestRestore();         /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixPortGroupWraparoundTest
*/
UTF_TEST_CASE_MAC(tgfIpfixPortGroupWraparoundTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    prvTgfIpfixPortGroupEntryTestInit();                    /* Set configuration */
    prvTgfIpfixPortGroupWraparoundTestTrafficGenerate();    /* Generate traffic  */
    prvTgfIpfixPortGroupEntryTestRestore();                 /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixPortGroupAlarmsTest
*/
UTF_TEST_CASE_MAC(tgfIpfixPortGroupAlarmsTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_CPSS_PP_E_ARCH_CNS);

    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_FAIL_MAC("IPFIX is not alligned with GM yet\n")

    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    prvTgfIpfixPortGroupAlarmsTestInit();               /* Set configuration */
    prvTgfIpfixPortGroupAlarmsTestTrafficGenerate();    /* Generate traffic  */
    prvTgfIpfixPortGroupAlarmsTestRestore();            /* Restore configuration */
}

/*----------------------------------------------------------------------------*/
/*
    tgfIpfixTimersUploadIncrementalTest
*/
UTF_TEST_CASE_MAC(tgfIpfixTimestampToCpuTest)
{
/********************************************************************
    1. Upload timers
    2. Restore
********************************************************************/

    /* test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_SKIP_MAC("IPFIX timestamp isn't implemented in GM\n")

    prvTgfIpfixTimestampToCpuTestInit();            /* Set configuration */
    prvTgfIpfixTimestampToCpuTestTrafficGenerate(); /* Generate traffic  */
    prvTgfIpfixTimestampToCpuTestRestore();         /* Restore configuration */
}

UTF_TEST_CASE_MAC(tgfIpfixEgressSamplingToCpuTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    prvTgfIpfixEgressSamplingToCpuTestInit();          /* Set configuration */
    prvTgfIpfixSamplingSimpleTestTrafficGenerate();   /* Generate traffic  */
    prvTgfIpfixEgressSamplingTestRestore();            /* Restore configuration */
}

UTF_TEST_CASE_MAC(tgfIpfixFirstNPacketsTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    prvTgfIpfixFirstNPacketsTestInit();               /* Set configuration */
    prvTgfIpfixFirstNPacketsTestTrafficGenerate();    /* Generate traffic  */
    prvTgfIpfixFirstNPacketsTestRestore();            /* Restore configuration */
}

UTF_TEST_CASE_MAC(tgfIpfixEnableIpclEpclTest)
{
/********************************************************************
    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    if(GT_FALSE == prvTgfPclPolicerCheck())
    {
        /* the device may support policer but not from the PCL/TTI */
        return;
    }

    prvTgfIpfixEnableIpclEpclTestInit();               /* Set configuration */
    prvTgfIpfixEnableIpclEpclTestTrafficGenerate();    /* Generate traffic  */
    prvTgfIpfixEnableIpclEpclTestRestore();            /* Restore configuration */
}

/*
 * Configuration of tgfIpfix suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfIpfix)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixHelloTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFirstIngressTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFirstUseAllIngressTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixSecondUseAllIngressTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixTimestampVerificationTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixAgingTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixWraparoundFreezeTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixWraparoundClearTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixWraparoundMaxBytesTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixDropModeSimpleTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixSamplingSimpleTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixSamplingToCpuTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixAlarmEventsTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixTimersUploadAbsoluteTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixTimersUploadIncrementalTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixPortGroupEntryTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixPortGroupAgingTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixPortGroupWraparoundTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixPortGroupAlarmsTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixTimestampToCpuTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixEgressSamplingToCpuTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFirstNPacketsTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixEnableIpclEpclTest)
UTF_SUIT_END_TESTS_MAC(tgfIpfix)

















