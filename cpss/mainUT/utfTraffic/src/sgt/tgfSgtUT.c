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
* @file tgfSgtUT.c
*
* @brief Enhanced UTs for SGT (Security Group tag) feature
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <common/tgfBridgeGen.h>
#include <trunk/prvTgfTrunk.h>
#include <sgt/prvTgfSgt.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

static GT_BOOL isTestSupportPha(void)
{
    if(0 == PRV_CPSS_DXCH_PP_MAC(prvTgfDevNum)->hwInfo.phaInfo.numOfPpg)
    {
        /* the device not supports PHA unit */
        return GT_FALSE;
    }

    if(GT_TRUE == prvUtfIsGmCompilation())
    {
        /* the GM not supports PHA unit */
        return GT_FALSE;
    }

    return GT_TRUE;
}

static GT_BOOL isTestSupportPhaSgtFw(void)
{
    if(GT_FALSE == isTestSupportPha())
    {
        return GT_FALSE;
    }

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* the PHA FW of the device not supports SGT */
        return GT_FALSE;
    }

    return GT_TRUE;
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify SGT Tag - User Port:
*/
UTF_TEST_CASE_MAC(tgfSgtUserPort)
{
    /* SGT Supported in HA, in case of SIP6_30 devices */
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
    }
    else if(GT_FALSE  == isTestSupportPhaSgtFw())
    {
        SKIP_TEST_MAC;
    }

    /* Test case for user port related test */
    prvTgfSgtUserPort_test();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify SGT Tag - Network Port:
*/
UTF_TEST_CASE_MAC(tgfSgtNetworkPort)
{
    /* SGT Supported in HA, in case of SIP6_30 devices */
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
    }
    else if(GT_FALSE  == isTestSupportPhaSgtFw())
    {
        SKIP_TEST_MAC;
    }

    /* Test case for user port related test */
    prvTgfSgtNetworkPort_test();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify SGT Tag - Tunnel Port:
*/
UTF_TEST_CASE_MAC(tgfSgtTunnelPort)
{
    /* SGT Supported in HA, in case of SIP6_30 devices */
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
    }
    else if(GT_FALSE  == isTestSupportPhaSgtFw())
    {
        SKIP_TEST_MAC;
    }

    /* Test case for user port related test */
    prvTgfSgtTunnelPort_test();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify SGT Tag - Cascade Port:
*/
UTF_TEST_CASE_MAC(tgfSgtCascadePort)
{
    if(GT_FALSE  == isTestSupportPhaSgtFw())
    {
        SKIP_TEST_MAC;
    }

    /* Test case for user port related test */
    prvTgfSgtCascadePort_test();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify Use Src Trg Mode = SRC_DST_EPG:
 *          TTI configures -> src EPG
 *          FDB configures -> dst EPG
 *          EPCL derives final index. (6 Bits from Src EPG & 6 bits from Dst EPG)
*/
UTF_TEST_CASE_MAC(tgfSgtUseSrcTrgMode)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))

    /* Test case for Use Src Trg mode */
    tgfSgtUseSrcTrgMode_test1();
}

/*
 * test cases for tgfSGT suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfSgt)

    UTF_SUIT_DECLARE_TEST_MAC(tgfSgtUserPort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSgtNetworkPort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSgtTunnelPort)
    UTF_SUIT_DECLARE_TEST_MAC(tgfSgtCascadePort)

    /* SGT functionality in HA */
    UTF_SUIT_DECLARE_TEST_MAC(tgfSgtUseSrcTrgMode)

UTF_SUIT_END_TESTS_MAC(tgfSgt)
