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
* @file tgfCommonHighAvailabilityUT.c
*
* @brief Enhanced UTs for CPSS High Availability
* @version   1
********************************************************************************
*/


/* HA UT not supported in CAP - commenting the entire file for CAP - since there are many references to appDemo
   Note: Please use wrappers for appDemo functions - Refer wrappers in cpssEnabler/utfWrappers/src/utfExtHelpers.c
*/
#ifndef CPSS_APP_PLATFORM_REFERENCE

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <common/tgfBridgeGen.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>


extern GT_VOID tgfBasicTrafficSanity_extern
(
    GT_VOID
);

UTF_TEST_CASE_MAC(tgTrafficSanityTest)
{


    GT_BOOL                         portMng;
    GT_STATUS                       st = 0 ;

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC(UTF_FALCON_E|UTF_ALDRIN2_E)
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC( prvTgfDevNum, (~(UTF_FALCON_E|UTF_ALDRIN2_E)) )

    st = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "ERROR : cpssPxPortManagerEnableGet FAILED:st-%d",st);

    PRV_UTF_LOG1_MAC("port manager active %d \n",portMng);


    /* verify simple traffic */
    tgfBasicTrafficSanity_extern();

    PRV_UTF_LOG0_MAC("\n flush FDB ");
    /* flush FDB include static entries */
    prvTgfBrgFdbFlush(GT_TRUE);
}

/*
 * Configuration of tgfSanity suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfSanity)

    UTF_SUIT_DECLARE_TEST_MAC(tgTrafficSanityTest)

UTF_SUIT_END_TESTS_MAC(tgfSanity)

#endif
