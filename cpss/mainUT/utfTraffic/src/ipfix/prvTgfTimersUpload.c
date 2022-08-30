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
* @file prvTgfTimersUpload.c
*
* @brief Timers upload tests for IPFIX
*
* @version   5
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/generic/pcl/cpssPcl.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPclGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPolicerGen.h>
#include <common/tgfIpfixGen.h>

#include <ipfix/prvTgfTimersUpload.h>


/**
* @internal prvTgfIpfixTimersRunningCheck function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersRunningCheck
(
    PRV_TGF_POLICER_STAGE_TYPE_ENT stage,
    GT_U32                         timeout
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_IPFIX_TIMER_STC                 timerGet;
    GT_U32                                  saveNanoSec;
    GT_U32                                  loopIndex;

    rc = prvTgfIpfixTimerGet(prvTgfDevNum, stage, &timerGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
        prvTgfDevNum, stage);

    saveNanoSec = timerGet.nanoSecondTimer;

    for (loopIndex = 0; (loopIndex < timeout); loopIndex++)
    {
        rc = prvTgfIpfixTimerGet(prvTgfDevNum, stage, &timerGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(
            GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
            prvTgfDevNum, stage);
        if (saveNanoSec != timerGet.nanoSecondTimer)
        {
            /* time changed - OK */
            break;
        }
        cpssOsTimerWkAfter(1);
    }
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        saveNanoSec, timerGet.nanoSecondTimer, "policer timer nanoseconds not changed");

}

/**
* @internal prvTgfIpfixTimersRunningTest function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersRunningTest
(
    GT_VOID
)
{
    prvTgfIpfixTimersRunningCheck(PRV_TGF_POLICER_STAGE_INGRESS_0_E, 400);
    if (GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        prvTgfIpfixTimersRunningCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E, 400);
    }
    if (GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        prvTgfIpfixTimersRunningCheck(PRV_TGF_POLICER_STAGE_EGRESS_E, 400);
    }
}

/**
* @internal prvTgfIpfixTimersUploadAbsoluteTest function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersUploadAbsoluteTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     uploadDone;
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC uploadSet;
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC uploadGet;
    PRV_TGF_IPFIX_TIMER_STC                 timerGet;
    GT_U32  ingr0_second[2];
    GT_U32  ingr1_second[2];
    GT_U32  egr_second[2];
    GT_U32  timeout_sec;
    GT_U32  sec0_overflow;
    GT_U32  lowTimeBound;

    /* check running of TOD */
    prvTgfIpfixTimersRunningTest();

    timeout_sec     = 5;
    if (cpssDeviceRunCheck_onEmulator())
    {
        /* the emulator clock is slower than CPU clock more than 1000 times */
        /* we have no reason to wait more than 1000 seconds                 */
        timeout_sec     = 0;
    }
    ingr0_second[0] = 0;
    ingr0_second[1] = 0;
    ingr1_second[0] = 0xFFFFFFFF - (timeout_sec / 2);
    ingr1_second[1] = 0xFFFFFFFF;
    egr_second[0]   = 100;
    egr_second[1]   = 0;

    uploadSet.uploadMode = PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E;
    uploadSet.timer.nanoSecondTimer = 0;
    uploadSet.timer.secondTimer.l[0] = ingr0_second[0];
    uploadSet.timer.secondTimer.l[1] = ingr0_second[1];

    rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                       PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                       &uploadSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        uploadSet.timer.secondTimer.l[0] = ingr1_second[0];
        uploadSet.timer.secondTimer.l[1] = ingr1_second[1];

        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_INGRESS_1_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);
    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        uploadSet.timer.secondTimer.l[0] = egr_second[0];
        uploadSet.timer.secondTimer.l[1] = egr_second[1];

        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_EGRESS_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);
    }

    rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                       PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                       &uploadGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                 "Upload mode different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                 "Upload nanoSecond different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(ingr0_second[0], uploadGet.timer.secondTimer.l[0],
                                 "Upload second.l[0] different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(ingr0_second[1], uploadGet.timer.secondTimer.l[1],
                                 "Upload second.l[1] different then expected");

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_INGRESS_1_E,
                                           &uploadGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                     "Upload mode different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                     "Upload nanoSecond different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(ingr1_second[0], uploadGet.timer.secondTimer.l[0],
                                     "Upload second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(ingr1_second[1], uploadGet.timer.secondTimer.l[1],
                                     "Upload second.l[1] different then expected");
    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_EGRESS_E,
                                           &uploadGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                     "Upload mode different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                     "Upload nanoSecond different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(egr_second[0], uploadGet.timer.secondTimer.l[0],
                                     "Upload second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(egr_second[1], uploadGet.timer.secondTimer.l[1],
                                     "Upload second.l[1] different then expected");
    }

    rc = prvTgfIpfixTimestampUploadTrigger(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadTrigger: %d",
                                 prvTgfDevNum);

    cpssOsTimerWkAfter(10);

    rc = prvTgfIpfixTimestampUploadStatusGet(prvTgfDevNum, &uploadDone);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadStatusGet: %d",
                                 prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, uploadDone,
                                 "Upload was not finished as expected");

    cpssOsTimerWkAfter(timeout_sec * 1000);

    /* get IPFIX timers */

    rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E, &timerGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    sec0_overflow = ((ingr0_second[0] + timeout_sec) < ingr0_second[0]) ? 1 : 0;

    lowTimeBound = (timerGet.secondTimer.l[0] == 0) ? 0 : (timerGet.secondTimer.l[0] - 1);
    if (timerGet.secondTimer.l[0] < (lowTimeBound + timeout_sec) ||
        timerGet.secondTimer.l[0] > (ingr0_second[0] + timeout_sec + 1))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC((ingr0_second[0] + timeout_sec), timerGet.secondTimer.l[0],
                                     "ipfix second.l[0] different then expected");
    }

    lowTimeBound = (timerGet.secondTimer.l[1] == 0) ? 0 : (timerGet.secondTimer.l[1] - 1);
    if( timerGet.secondTimer.l[1] < (lowTimeBound + sec0_overflow) ||
        timerGet.secondTimer.l[1] > (ingr0_second[1] + sec0_overflow + 1))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC((ingr0_second[1] + sec0_overflow), timerGet.secondTimer.l[1],
                                     "ipfix second.l[1] different then expected");
    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E, &timerGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);

        sec0_overflow = ((ingr1_second[0] + timeout_sec) < ingr1_second[0]) ? 1 : 0;

        lowTimeBound = (timerGet.secondTimer.l[0] == 0) ? 0 : (timerGet.secondTimer.l[0] - 1);
        if( timerGet.secondTimer.l[0] < (lowTimeBound + timeout_sec) ||
            timerGet.secondTimer.l[0] > (ingr1_second[0] + timeout_sec + 1))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC((ingr1_second[0] + timeout_sec), timerGet.secondTimer.l[0],
                                         "ipfix second.l[0] different then expected");
        }

        lowTimeBound = (timerGet.secondTimer.l[1] == 0) ? 0 : (timerGet.secondTimer.l[1] - 1);
        if( timerGet.secondTimer.l[1] < (lowTimeBound + sec0_overflow) ||
            timerGet.secondTimer.l[1] > (ingr1_second[1] + sec0_overflow + 1))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC((ingr1_second[1] + sec0_overflow), timerGet.secondTimer.l[1],
                                         "ipfix second.l[1] different then expected");
        }
    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &timerGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);

        sec0_overflow = ((egr_second[0] + timeout_sec) < egr_second[0]) ? 1 : 0;

        lowTimeBound = (timerGet.secondTimer.l[0] == 0) ? 0 : (timerGet.secondTimer.l[0] - 1);
        if( timerGet.secondTimer.l[0] < (lowTimeBound + timeout_sec) ||
            timerGet.secondTimer.l[0] > (egr_second[0] + timeout_sec + 1))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC((egr_second[0] + timeout_sec), timerGet.secondTimer.l[0],
                                         "ipfix second.l[0] different then expected");
        }

        lowTimeBound = (timerGet.secondTimer.l[1] == 0) ? 0 : (timerGet.secondTimer.l[1] - 1);
        if( timerGet.secondTimer.l[1] < (lowTimeBound + sec0_overflow) ||
            timerGet.secondTimer.l[1] > (egr_second[1] + sec0_overflow + 1))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC((egr_second[1] + sec0_overflow), timerGet.secondTimer.l[1],
                                         "ipfix second.l[1] different then expected");
        }
    }
}

/**
* @internal prvTgfIpfixTimersUploadIncrementalTest function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersUploadIncrementalTest
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     uploadDone;
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC uploadSet;
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC uploadGet;
    PRV_TGF_IPFIX_TIMER_STC                 timerGet,timerGet1;

    uploadSet.uploadMode = PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E;
    uploadSet.timer.nanoSecondTimer = 0;
    uploadSet.timer.secondTimer.l[0] = 0;
    uploadSet.timer.secondTimer.l[1] = 0;

    rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                       PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                       &uploadSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    uploadSet.timer.secondTimer.l[0] = 0xFFFFFFFF;
    uploadSet.timer.secondTimer.l[1] = 0xFFFFFFFF;

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_INGRESS_1_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);
    }

    uploadSet.timer.secondTimer.l[0] = 100;
    uploadSet.timer.secondTimer.l[1] = 0;

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_EGRESS_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);
    }

    rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                       PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                       &uploadGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                 "Upload mode different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                 "Upload nanoSecond different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.secondTimer.l[0],
                                 "Upload second.l[0] different then expected");

    UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.secondTimer.l[1],
                                 "Upload second.l[1] different then expected");

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_INGRESS_1_E,
                                           &uploadGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                     "Upload mode different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                     "Upload nanoSecond different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, uploadGet.timer.secondTimer.l[0],
                                     "Upload second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0xFFFFFFFF, uploadGet.timer.secondTimer.l[1],
                                     "Upload second.l[1] different then expected");
    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimestampUploadGet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_EGRESS_E,
                                           &uploadGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(uploadSet.uploadMode, uploadGet.uploadMode,
                                     "Upload mode different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.nanoSecondTimer,
                                     "Upload nanoSecond different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(100, uploadGet.timer.secondTimer.l[0],
                                     "Upload second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(0, uploadGet.timer.secondTimer.l[1],
                                     "Upload second.l[1] different then expected");
    }

    rc = prvTgfIpfixTimestampUploadTrigger(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadTrigger: %d",
                                 prvTgfDevNum);

    cpssOsTimerWkAfter(10);

    rc = prvTgfIpfixTimestampUploadStatusGet(prvTgfDevNum, &uploadDone);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadStatusGet: %d",
                                 prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, uploadDone,
                                 "Upload was not finished as expected");
    if (cpssDeviceRunCheck_onEmulator())
    {
        /* the time on emulator is slower more than 1000 times slower */
        /* then on real HW and testing CPU - this test not relevant   */
        /* check running of TOD using nona-seconds counter            */
        prvTgfIpfixTimersRunningTest();
        return;
    }

    cpssOsTimerWkAfter(4000);

    /* get IPFIX timers */

    rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E, &timerGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E, &timerGet1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(timerGet.secondTimer.l[0], timerGet1.secondTimer.l[0]+1,
                                     "ipfix second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(timerGet.secondTimer.l[1], timerGet1.secondTimer.l[1],
                                     "ipfix second.l[1] different then expected");

    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimerGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &timerGet1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixEntryGet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);

        UTF_VERIFY_EQUAL0_STRING_MAC(timerGet.secondTimer.l[0] + 100, timerGet1.secondTimer.l[0],
                                     "ipfix second.l[0] different then expected");

        UTF_VERIFY_EQUAL0_STRING_MAC(timerGet.secondTimer.l[1], timerGet1.secondTimer.l[1],
                                     "ipfix second.l[1] different then expected");
    }
}

/**
* @internal prvTgfIpfixTimersUploadRestoreByClear function
* @endinternal
*
*/
GT_VOID prvTgfIpfixTimersUploadRestoreByClear
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_BOOL     uploadDone;
    PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_CONF_STC uploadSet;

    uploadSet.uploadMode = PRV_TGF_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E;
    uploadSet.timer.nanoSecondTimer = 0;
    uploadSet.timer.secondTimer.l[0] = 0;
    uploadSet.timer.secondTimer.l[1] = 0;

    rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                       PRV_TGF_POLICER_STAGE_INGRESS_0_E,
                                       &uploadSet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                 prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_0_E);

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_INGRESS_1_E))
    {
        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_INGRESS_1_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_INGRESS_1_E);
    }

    if(GT_TRUE ==  prvTgfPolicerStageCheck(PRV_TGF_POLICER_STAGE_EGRESS_E))
    {
        rc = prvTgfIpfixTimestampUploadSet(prvTgfDevNum,
                                           PRV_TGF_POLICER_STAGE_EGRESS_E,
                                           &uploadSet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadSet: %d, %d",
                                     prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E);
    }

    rc = prvTgfIpfixTimestampUploadTrigger(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadTrigger: %d",
                                 prvTgfDevNum);

    cpssOsTimerWkAfter(10);

    rc = prvTgfIpfixTimestampUploadStatusGet(prvTgfDevNum, &uploadDone);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpfixTimestampUploadStatusGet: %d",
                                 prvTgfDevNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, uploadDone,
                                 "Upload was not finished as expected");
}


