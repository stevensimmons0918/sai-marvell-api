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
* @file tgfCommonEventUT.c
*
* @brief Enhanced UTs for CPSS events
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <event/prvTgfEventDeviceGenerate.h>


/*----------------------------------------------------------------------------*/
/* AUTODOC: prvTgfGenEventDeviceGenerate:
    Description:
    The purpose of this test is to check the functionality
    of event generation from CPSS API.

*/
UTF_TEST_CASE_MAC(prvTgfGenEventDeviceGenerate)
{
    GT_U32      notAppFamilyBmp;

    prvTgfReRunVariable = TGF_RE_RUN_STATUS_DISABLED_E; /*disables re-run*/

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    GM_NOT_SUPPORT_THIS_TEST_MAC;

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT3_E, "JIRA-6591");

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Enable HW errors ignoring */
        prvWrAppEventFatalErrorEnable(CPSS_ENABLER_FATAL_ERROR_SILENT_TYPE_E);
    }

    /* Unmask skipped events */
    prvWrAppSkipEventMaskSet(prvTgfDevNum, CPSS_EVENT_UNMASK_E);

    /* Test */
    prvTgfGenEvent();

    /* Mask skipped events */
    prvWrAppSkipEventMaskSet(prvTgfDevNum, CPSS_EVENT_MASK_E);

    if(PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        /* AUTODOC: Disable HW errors ignoring */
        prvWrAppEventFatalErrorEnable(CPSS_ENABLER_FATAL_ERROR_EXECUTE_TYPE_E);
    }
}

UTF_TEST_CASE_MAC(prvTgfGenEventPhysicalPortExtExDataMapping)
{
    GT_U32      notAppFamilyBmp;
    prvTgfReRunVariable = TGF_RE_RUN_STATUS_DISABLED_E; /*disables re-run*/

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* Test */
    prvTgfPhyToMacPortMapping();
}

UTF_TEST_CASE_MAC(prvTgfGenInterruptCoalescing)
{
  GT_U32      notAppFamilyBmp;

  /* Supported by eArch devices only */
  UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
  PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

  /* asic simulation not support this --> skip */
  ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
  /* default coalescing behavior of disabled */
  /* Generate FWS traffic */
  prvTgfInterruptCoalescingWireSpeedTrafficGenerate(GT_FALSE);

  /* Restore FWS configuration */
  prvTgfInterruptCoalescingWireSpeedConfigurationRestore();

  /* coalescing behavior enabled */
  prvTgfInterruptCoalescingConfig(GT_TRUE, GT_TRUE);

  /* Generate FWS traffic */
  prvTgfInterruptCoalescingWireSpeedTrafficGenerate(GT_TRUE);

  /* Restore FWS configuration */
  prvTgfInterruptCoalescingWireSpeedConfigurationRestore();

  /* restore coalescing behavior */
  prvTgfInterruptCoalescingConfig(GT_FALSE, GT_FALSE);

#endif /*ASIC_SIMULATION*/
}

UTF_TEST_CASE_MAC(prvTgfGenInterruptCoalescingForLinkChangeEvent)
{
  GT_U32      notAppFamilyBmp;

  /* Supported by eArch devices only */
  UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
  PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

  /* asic simulation not support this --> skip */
  ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION

    PM_TBD
    /* test link change method conflicts with PM logic */
    if(GT_TRUE == prvUtfIsPortManagerUsed())
    {
        SKIP_TEST_MAC;
    }
    /* coalescing behavior enabled  with link state override disabled */
    prvTgfInterruptCoalescingConfig(GT_TRUE, GT_TRUE);

    /* Generate Link State Change Event  */
    prvTgfInterruptCoalescingForLinkChangeEvent(GT_TRUE);

    /* restore coalescing behavior */
    prvTgfInterruptCoalescingConfig(GT_FALSE, GT_TRUE);

    /* coalescing behavior enabled  with link state override enabled */
    prvTgfInterruptCoalescingConfig(GT_TRUE, GT_FALSE);

    /* Generate Link State Change Event  */
    prvTgfInterruptCoalescingForLinkChangeEvent(GT_FALSE);

    /* restore coalescing behavior */
    prvTgfInterruptCoalescingConfig(GT_FALSE, GT_TRUE);

#endif /*ASIC_SIMULATION*/
}

UTF_TEST_CASE_MAC(prvTgfGenEventUnbindBind)
{
    GT_U32      notAppFamilyBmp;
    prvTgfReRunVariable = TGF_RE_RUN_STATUS_DISABLED_E; /*disables re-run*/

    /* Supported by eArch devices only */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, notAppFamilyBmp);

    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* Test */
    prvTgfEventPortLinkStatusUnbindBind();
    prvTgfPhyToMacPortMapping();
}

/*
 * Configuration of tgfEvent suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfEvent)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfGenEventDeviceGenerate)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfGenEventPhysicalPortExtExDataMapping)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfGenEventUnbindBind)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfGenInterruptCoalescing)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfGenInterruptCoalescingForLinkChangeEvent)

UTF_SUIT_END_TESTS_MAC(tgfEvent)


