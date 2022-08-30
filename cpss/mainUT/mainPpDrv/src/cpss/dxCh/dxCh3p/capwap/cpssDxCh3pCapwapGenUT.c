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
* @file cpssDxCh3pCapwapGenUT.c
*
* @brief Unit tests for cpssDxCh3pCapwapGen, that provides
* CPSS DxCh3p CAPWAP API.
*
*
* @version   2.
********************************************************************************
*/

/* includes */
#include <cpss/dxCh/dxCh3p/capwap/cpssDxCh3pCapwapGen.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */
/* Invalid enum */
#define CAPWAP_GEN_INVALID_ENUM_CNS     0x5AAAAAA5


    /* TBD - not supported yet */
#if 0
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenReassemblyEngineContextsNumberSet
(
    IN GT_U8    devNum,
    IN GT_U32   reassemblyContextNumber
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenReassemblyEngineContextsNumberSet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with reassemblyContextNumber  [0 / 2047].
    Expected: GT_OK.
    1.2. Call cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet with not-NULL reassemblyContextNumberPtr.
    Expected: GT_OK and same reassemblyContextNumber as was set.
    1.3. Call with out of range reassemblyContextNumber [2048].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32   reassemblyContextNumber    = 0;
    GT_U32   reassemblyContextNumberGet = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with reassemblyContextNumber  [0 / 2047].
            Expected: GT_OK.
        */
        /* iterate with reassemblyContextNumber = 0 */
        reassemblyContextNumber = 0;

        st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberSet(dev, reassemblyContextNumber);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, reassemblyContextNumber);

        /*
            1.2. Call cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet
            with not-NULL reassemblyContextNumberPtr.
            Expected: GT_OK and same reassemblyContextNumber as was set.
        */
        st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet(dev, &reassemblyContextNumberGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(reassemblyContextNumber, reassemblyContextNumberGet,
                                     "got another reassemblyContextNumber than was set: %d", dev);

        /* iterate with reassemblyContextNumber = 2047 */
        reassemblyContextNumber = 2047;

        st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberSet(dev, reassemblyContextNumber);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, reassemblyContextNumber);

        /*
            1.2. Call cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet
            with not-NULL reassemblyContextNumberPtr.
            Expected: GT_OK and same reassemblyContextNumber as was set.
        */
        st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet(dev, &reassemblyContextNumberGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(reassemblyContextNumber, reassemblyContextNumberGet,
                                     "got another reassemblyContextNumber than was set: %d", dev);

        /*
            1.3. Call with out of range reassemblyContextNumber [2048].
            Expected: NOT GT_OK.
        */
        reassemblyContextNumber = 2048;

        st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberSet(dev, reassemblyContextNumber);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, reassemblyContextNumber);
    }

    reassemblyContextNumber = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberSet(dev, reassemblyContextNumber);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberSet(dev, reassemblyContextNumber);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *reassemblyContextNumberPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with not NULL reassemblyContextNumberPtr.
    Expected: GT_OK.
    1.2. Call with reassemblyContextNumberPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      reassemblyContextNumber = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL reassemblyContextNumberPtr.
            Expected: GT_OK.
        */
        st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet(dev, &reassemblyContextNumber);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with reassemblyContextNumberPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, reassemblyContextNumberPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet(dev, &reassemblyContextNumber);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet(dev, &reassemblyContextNumber);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutSet
(
    IN GT_U8    devNum,
    IN GT_U32   timeout
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutSet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with timeout [1 / 500].
    Expected: GT_OK.
    1.2. Call cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet with not NULL timeoutPtr.
    Expected: GT_OK and same timeout as was set.
    1.3. Call with timeout [501].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      timeout    = 0;
    GT_U32      timeoutGet = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with timeout [1 / 500].
            Expected: GT_OK.
        */
        /* iterate with timeout = 1 */
        timeout = 1;

        st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutSet(dev, timeout);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, timeout);

        /*
            1.2. Call cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet with not NULL timeoutPtr.
            Expected: GT_OK and same timeout as was set.
        */
        st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet(dev, &timeoutGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(timeout, timeoutGet, "got another timeout then was set: %d", dev);

        /* iterate with timeout = 500 */
        timeout = 500;

        st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutSet(dev, timeout);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, timeout);

        /*
            1.2. Call cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet with not NULL timeoutPtr.
            Expected: GT_OK and same timeout as was set.
        */
        st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet(dev, &timeoutGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(timeout, timeoutGet, "got another timeout then was set: %d", dev);

        /*
            1.3. Call with timeout [501].
            Expected: NOT GT_OK.
        */
        timeout = 501;

        st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutSet(dev, timeout);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, timeout);
    }

    timeout = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutSet(dev, timeout);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutSet(dev, timeout);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet
(
    IN  GT_U8       devNum,
    OUT GT_U32     *timeoutPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with not NULL timeoutPtr.
    Expected: GT_OK
    1.2. Call with timeoutPtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      timeout = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL timeoutPtr.
            Expected: GT_OK
        */
        st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet(dev, &timeout);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with timeoutPtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, timeoutPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet(dev, &timeout);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet(dev, &timeout);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterSet
(
    IN GT_U8    devNum,
    IN GT_U32   counterValue
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterSet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with counterValue [0 / 0xFFFFFFFF]
    Expected: GT_OK
    1.2. Call cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet with not-NULL counterValuePtr.
    Expected: GT_OK and same counterValue as was set.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32   counterValue    = 0;
    GT_U32   counterValueGet = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with counterValue [0 / 0xFFFFFFFF]
            Expected: GT_OK
        */
        /* iterate with counterValue = 0 */
        counterValue = 0;

        st = cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterSet(dev, counterValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, counterValue);

        /*
            1.2. Call cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet with not-NULL counterValuePtr.
            Expected: GT_OK and same counterValue as was set.
        */
        st = cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet(dev, &counterValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(counterValue, counterValueGet, "got another counterValue then was set: %d", dev);

        /* iterate with counterValue = 0xFFFFFFFF */
        counterValue = 0xFFFFFFFF;

        st = cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterSet(dev, counterValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, counterValue);

        /*
            1.2. Call cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet with not-NULL counterValuePtr.
            Expected: GT_OK and same counterValue as was set.
        */
        st = cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet(dev, &counterValueGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(counterValue, counterValueGet, "got another counterValue then was set: %d", dev);
    }

    counterValue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterSet(dev, counterValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterSet(dev, counterValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet
(
    IN  GT_U8       devNum,
    OUT GT_U32     *counterValuePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with not NULL counterValuePtr.
    Expected: GT_OK
    1.2. Call with counterValuePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32     counterValue = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL counterValuePtr.
            Expected: GT_OK
        */
        st = cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet(dev, &counterValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with counterValuePtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "counterValuePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet(dev, &counterValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet(dev, &counterValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
#endif
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenRssiWeightProfileEntrySet
(
    IN GT_U8                                               devNum,
    IN GT_U32                                              profileIndex,
    IN CPSS_DXCH3P_CAPWAP_GEN_RSSI_WEIGHT_PROFILE_ENTRY_STC *rssiProfileEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenRssiWeightProfileEntrySet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with profileIndex [0 / 3],
                   rssiProfileEntryPtr{ weight[0 / 100],
                                        minRssiPacketTreshold[0 / 255],
                                        minRssiAverageTreshold[0 / 255] }
    Expected: GT_OK.
    1.2. Call cpssDxCh3pCapwapGenRssiWeightProfileEntryGet with not NULL rssiProfileEntryPtr
                                                                and other params from 1.1.
    Expected: GT_OK and the same rssiProfileEntry as was set.
    1.3. Call with profileIndex[4]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with rssiProfileEntryPtr->weight [101]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with rssiProfileEntryPtr-> minRssiPacketTreshold [256]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with rssiProfileEntryPtr-> minRssiAverageTreshold [256]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.7. Call with rssiProfileEntryPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      profileIndex = 0;
    GT_BOOL     isEqual      = GT_FALSE;

    CPSS_DXCH3P_CAPWAP_GEN_RSSI_WEIGHT_PROFILE_ENTRY_STC    rssiProfileEntry;
    CPSS_DXCH3P_CAPWAP_GEN_RSSI_WEIGHT_PROFILE_ENTRY_STC    rssiProfileEntryGet;


    cpssOsBzero((GT_VOID*) &rssiProfileEntry, sizeof(rssiProfileEntry));
    cpssOsBzero((GT_VOID*) &rssiProfileEntryGet, sizeof(rssiProfileEntryGet));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileIndex [0 / 3],
                           rssiProfileEntryPtr{ weight[0 / 100],
                                                minRssiPacketTreshold[0 / 255],
                                                minRssiAverageTreshold[0 / 255] }
            Expected: GT_OK.
        */
        /* iterate with profileIndex = 0 */
        profileIndex = 0;

        rssiProfileEntry.weight                 = 0;
        rssiProfileEntry.minRssiPacketTreshold  = 0;
        rssiProfileEntry.minRssiAverageTreshold = 0;

        st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

        /*
            1.2. Call cpssDxCh3pCapwapGenRssiWeightProfileEntryGet with not NULL rssiProfileEntryPtr
                                                                        and other params from 1.1.
            Expected: GT_OK and the same rssiProfileEntry as was set.
        */
        st = cpssDxCh3pCapwapGenRssiWeightProfileEntryGet(dev, profileIndex, &rssiProfileEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenRssiWeightProfileEntryGet: %d, %d", dev, profileIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &rssiProfileEntry,
                                     (GT_VOID*) &rssiProfileEntryGet,
                                     sizeof(rssiProfileEntry) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another rssiProfileEntry then was set: %d", dev);

        /* iterate with profileIndex = 3 */
        profileIndex = 3;

        rssiProfileEntry.weight                 = 100;
        rssiProfileEntry.minRssiPacketTreshold  = 255;
        rssiProfileEntry.minRssiAverageTreshold = 255;

        st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

        /*
            1.2. Call cpssDxCh3pCapwapGenRssiWeightProfileEntryGet with not NULL rssiProfileEntryPtr
                                                                        and other params from 1.1.
            Expected: GT_OK and the same rssiProfileEntry as was set.
        */
        st = cpssDxCh3pCapwapGenRssiWeightProfileEntryGet(dev, profileIndex, &rssiProfileEntryGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenRssiWeightProfileEntryGet: %d, %d", dev, profileIndex);

        /* verifying values */
        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &rssiProfileEntry,
                                     (GT_VOID*) &rssiProfileEntryGet,
                                     sizeof(rssiProfileEntry) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another rssiProfileEntry then was set: %d", dev);

        /*
            1.3. Call with profileIndex[4]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        profileIndex = 4;

        st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

        profileIndex = 0;

        /*
            1.4. Call with rssiProfileEntryPtr->weight [101]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        rssiProfileEntry.weight = 101;

        st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, rssiProfileEntryPtr->weight = %d",
                                         dev, rssiProfileEntry.weight);

        rssiProfileEntry.weight = 0;

        /*
            1.5. Call with rssiProfileEntryPtr-> minRssiPacketTreshold [256]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        rssiProfileEntry.minRssiPacketTreshold = 256;

        st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, rssiProfileEntryPtr->minRssiPacketTreshold = %d",
                                         dev, rssiProfileEntry.minRssiPacketTreshold);

        rssiProfileEntry.minRssiPacketTreshold = 0;

        /*
            1.6. Call with rssiProfileEntryPtr-> minRssiAverageTreshold [256]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        rssiProfileEntry.minRssiAverageTreshold = 256;

        st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, rssiProfileEntryPtr->minRssiAverageTreshold = %d",
                                         dev, rssiProfileEntry.minRssiAverageTreshold);

        rssiProfileEntry.minRssiAverageTreshold = 0;

        /*
            1.7. Call with rssiProfileEntryPtr [NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, profileIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rssiProfileEntryPtr = NULL", dev);
    }

    profileIndex = 0;

    rssiProfileEntry.weight                 = 0;
    rssiProfileEntry.minRssiPacketTreshold  = 0;
    rssiProfileEntry.minRssiAverageTreshold = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, profileIndex, &rssiProfileEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenRssiWeightProfileEntryGet
(
    IN  GT_U8                                               devNum,
    IN  GT_U32                                              profileIndex,
    OUT CPSS_DXCH3P_CAPWAP_GEN_RSSI_WEIGHT_PROFILE_ENTRY_STC *rssiProfileEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenRssiWeightProfileEntryGet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with profileIndex[0 / 3]
                   and not NULL rssiProfileEntryPtr.
    Expected: GT_OK.
    1.2. Call with profileIndex[4]
                   and not NULL rssiProfileEntryPtr.
    Expected: NOT GT_OK.
    1.3. Call with rssiProfileEntryPtr [NULL]
                   and other param from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32                                               profileIndex = 0;
    CPSS_DXCH3P_CAPWAP_GEN_RSSI_WEIGHT_PROFILE_ENTRY_STC rssiProfileEntry;


    cpssOsBzero((GT_VOID*) &rssiProfileEntry, sizeof(rssiProfileEntry));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with profileIndex[0 / 3]
                           and not NULL rssiProfileEntryPtr.
            Expected: GT_OK.
        */
        /* iterate with profileIndex = 0 */
        profileIndex = 0;

        st = cpssDxCh3pCapwapGenRssiWeightProfileEntryGet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

        /* iterate with profileIndex = 3 */
        profileIndex = 3;

        st = cpssDxCh3pCapwapGenRssiWeightProfileEntryGet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

        /*
            1.2. Call with profileIndex[4]
                           and not NULL rssiProfileEntryPtr.
            Expected: NOT GT_OK.
        */
        profileIndex = 4;

        st = cpssDxCh3pCapwapGenRssiWeightProfileEntryGet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, profileIndex);

        profileIndex = 0;

        /*
            1.3. Call with rssiProfileEntryPtr [NULL]
                           and other param from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenRssiWeightProfileEntryGet(dev, profileIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rssiProfileEntryPtr = NULL", dev);
    }

    profileIndex = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenRssiWeightProfileEntryGet(dev, profileIndex, &rssiProfileEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenRssiWeightProfileEntryGet(dev, profileIndex, &rssiProfileEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenRssiExceptionStatusGet
(
    IN  GT_U8                devNum,
    OUT GT_U32               *rssiValuePtr,
    OUT GT_ETHERADDR         *macSaPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenRssiExceptionStatusGet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with not NULL rssiValuePtr and not NULL macSaPtr.
    Expected: GT_OK.
    1.2. Call with rssiValuePtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with macSaPtr [NULL] and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32          rssiValue = 0;
    GT_ETHERADDR    macSa;


    cpssOsBzero((GT_VOID*) &macSa, sizeof(macSa));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL rssiValuePtr and not NULL macSaPtr.
            Expected: GT_OK.
        */
        st = cpssDxCh3pCapwapGenRssiExceptionStatusGet(dev, &rssiValue, &macSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with rssiValuePtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenRssiExceptionStatusGet(dev, NULL, &macSa);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, rssiValuePtr = NULL", dev);

        /*
            1.3. Call with macSaPtr [NULL] and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenRssiExceptionStatusGet(dev, &rssiValue, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, macSaPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenRssiExceptionStatusGet(dev, &rssiValue, &macSa);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenRssiExceptionStatusGet(dev, &rssiValue, &macSa);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenVirtualDevicesSet
(
    IN GT_U8    devNum,
    IN GT_U32   virtDevicesBitmap
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenVirtualDevicesSet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with virtDevicesBitmap[0 / 31].
    Expected: GT_OK.
    1.2. Call cpssDxCh3pCapwapGenVirtualDevicesGet with not NULL virtDevicesBitmapPtr.
    Expected: GT_OK and same virtDevicesBitmap as was set.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32   virtDevicesBitmap    = 0;
    GT_U32   virtDevicesBitmapGet = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with virtDevicesBitmap[0 / 31].
            Expected: GT_OK.
        */
        /* iterate with virtDevicesBitmap = 0 */
        virtDevicesBitmap = 0;

        st = cpssDxCh3pCapwapGenVirtualDevicesSet(dev, virtDevicesBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, virtDevicesBitmap);

        /*
            1.2. Call cpssDxCh3pCapwapGenVirtualDevicesGet with not NULL virtDevicesBitmapPtr.
            Expected: GT_OK and same virtDevicesBitmap as was set.
        */
        st = cpssDxCh3pCapwapGenVirtualDevicesGet(dev, &virtDevicesBitmapGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenVirtualDevicesGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtDevicesBitmap, virtDevicesBitmapGet,
                                     "got another virtDevicesBitmap then was set: %d", dev);

        /* iterate with virtDevicesBitmap = 31 */
        virtDevicesBitmap = 0x1F;

        st = cpssDxCh3pCapwapGenVirtualDevicesSet(dev, virtDevicesBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, virtDevicesBitmap);

        /*
            1.2. Call cpssDxCh3pCapwapGenVirtualDevicesGet with not NULL virtDevicesBitmapPtr.
            Expected: GT_OK and same virtDevicesBitmap as was set.
        */
        st = cpssDxCh3pCapwapGenVirtualDevicesGet(dev, &virtDevicesBitmapGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenVirtualDevicesGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtDevicesBitmap, virtDevicesBitmapGet,
                                     "got another virtDevicesBitmap then was set: %d", dev);
    }

    virtDevicesBitmap = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenVirtualDevicesSet(dev, virtDevicesBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenVirtualDevicesSet(dev, virtDevicesBitmap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenVirtualDevicesGet
(
    IN GT_U8    devNum,
    OUT GT_U32  *virtDevicesBitmapPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenVirtualDevicesGet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with not NULL virtDevicesBitmap.
    Expected: GT_OK.
    1.2. Call with virtDevicesBitmapPtr [NULL]
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      virtDevicesBitmap = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL virtDevicesBitmap.
            Expected: GT_OK.
        */
        st = cpssDxCh3pCapwapGenVirtualDevicesGet(dev, &virtDevicesBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with virtDevicesBitmapPtr [NULL]
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenVirtualDevicesGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, virtDevicesBitmapPtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenVirtualDevicesGet(dev, &virtDevicesBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenVirtualDevicesGet(dev, &virtDevicesBitmap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenVirtualPortMappingEntrySet
(
    IN GT_U8   devNum,
    IN GT_U8   virtDevNum,
    IN GT_U8   virtPortNum,
    IN CPSS_DXCH3P_CAPWAP_GEN_VIRTUAL_PORT_MAPPING_ENTRY_STC *virtPortMappingTablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenVirtualPortMappingEntrySet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with virtDevNum[0],
                   virtPortNum [0],
                   virtPortMappingTablePtr{ isTunnelStart[GT_TRUE],
                                            tunnelStartPointer[0],
                                            physicalInterface{  type[CPSS_INTERFACE_PORT_E],
                                                                devPort { devNum[dev], portNum[0]} }
                                            srcIdFilterBitmap[0xFFFFFFFF] }
    Expected: GT_OK.
    1.2. Call cpssDxCh3pCapwapGenVirtualPortMappingEntryGet with not NULL virtPortMappingTablePtr
                                                                 and other params from 1.1.
                                                                 (the same for 1.9, 1.10, 1.13)
    Expected: GT_OK.
    1.3. Call with virtPortMappingTablePtr->physicalInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with virtPortMappingTablePtr->physicalInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with virtPortMappingTablePtr->physicalInterface.vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                   and other params from 1.1. (not relevant)
    Expected: GT_OK.
    1.6. Call with out of range virtDevNum[32]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.7. Call with out of range virtPortNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.8. Call with virtPortMappingTablePtr->physicalInterface.type [0x5AAAAAA5]
                   and other params from 1.1.
    Expected: GT_BAD_PARAM.
    1.9. Call with virtDevNum[1],
                   virtPortNum [1],
                   virtPortMappingTablePtr{ isTunnelStart[GT_TRUE],
                                            tunnelStartPointer[0],
                                            physicalInterface{  type[CPSS_INTERFACE_TRUNK_E],
                                                                trunkId [100] }
                                            srcIdFilterBitmap[0xFFFFFFFF] }
    Expected: GT_OK.
    1.10. Call with virtPortMappingTablePtr->physicalInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS]
                    and other params from 1.9. (Not relevant)
    Expected: GT_OK.
    1.11. Call with virtPortMappingTablePtr-> physicalInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                    and other params from 1.9. (Not relevant)
    Expected: GT_OK.
    1.12. Call with virtPortMappingTablePtr-> physicalInterface.vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS] (not used this field)
                    and other params from 1.13.
    Expected: GT_OK.
    1.13. Call with virtPortMappingTablePtr[NULL]
                    and other params from 1.13.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8   virtDevNum  = 0;
    GT_U8   virtPortNum = 0;

    CPSS_DXCH3P_CAPWAP_GEN_VIRTUAL_PORT_MAPPING_ENTRY_STC virtPortMappingTable;
    CPSS_DXCH3P_CAPWAP_GEN_VIRTUAL_PORT_MAPPING_ENTRY_STC virtPortMappingTableGet;


    cpssOsBzero((GT_VOID*) &virtPortMappingTable, sizeof(virtPortMappingTable));
    cpssOsBzero((GT_VOID*) &virtPortMappingTableGet, sizeof(virtPortMappingTableGet));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with virtDevNum[0],
                           virtPortNum [0],
                           virtPortMappingTablePtr{ isTunnelStart[GT_TRUE],
                                                    tunnelStartPointer[0],
                                                    physicalInterface{  type[CPSS_INTERFACE_PORT_E],
                                                                        devPort { devNum[dev], portNum[0]} } }
                                                    srcIdFilterBitmap[0xFFFFFFFF] }
            Expected: GT_OK.
        */
        virtDevNum  = 0;
        virtPortNum = 0;
        virtPortMappingTable.isTunnelStart          = GT_TRUE;
        virtPortMappingTable.tunnelStartPointer     = 0;
        virtPortMappingTable.physicalInterface.type = CPSS_INTERFACE_PORT_E;
        virtPortMappingTable.srcIdFilterBitmap      = 0xFFFFFFFF;

        virtPortMappingTable.physicalInterface.devPort.hwDevNum  = dev;
        virtPortMappingTable.physicalInterface.devPort.portNum = 0;

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, virtDevNum, virtPortNum);

        /*
            1.2. Call cpssDxCh3pCapwapGenVirtualPortMappingEntryGet with not NULL virtPortMappingTablePtr
                                                                         and other params from 1.1.
                                                                         (the same for 1.9, 1.10, 1.13)
            Expected: GT_OK.
        */
        st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, virtPortNum, &virtPortMappingTableGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenVirtualPortMappingEntryGet: %d, %d, %d",
                                     dev, virtDevNum, virtPortNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.isTunnelStart, virtPortMappingTableGet.isTunnelStart,
                                     "got another virtPortMappingTablePtr->isTunnelStart then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.tunnelStartPointer, virtPortMappingTableGet.tunnelStartPointer,
                                     "got another virtPortMappingTablePtr->tunnelStartPointer then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.physicalInterface.type, virtPortMappingTableGet.physicalInterface.type,
                                     "got another virtPortMappingTablePtr->physicalInterface.type then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.physicalInterface.devPort.hwDevNum, virtPortMappingTableGet.physicalInterface.devPort.hwDevNum,
                                     "got another virtPortMappingTablePtr->physicalInterface.devPort.hwDevNum then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.physicalInterface.devPort.portNum, virtPortMappingTableGet.physicalInterface.devPort.portNum,
                                     "got another virtPortMappingTablePtr->physicalInterface.devPort.portNum then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.srcIdFilterBitmap, virtPortMappingTableGet.srcIdFilterBitmap,
                                     "got another virtPortMappingTablePtr->srcIdFilterBitmap then was set: %d", dev);

        /*
            1.3. Call with virtPortMappingTablePtr->physicalInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        virtPortMappingTable.physicalInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, virtPortMappingTablePtr->physicalInterface.devPort.hwDevNum = %d",
                                        dev, virtPortMappingTable.physicalInterface.devPort.hwDevNum);

        virtPortMappingTable.physicalInterface.devPort.hwDevNum = 0;

        /*
            1.4. Call with virtPortMappingTablePtr->physicalInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        virtPortMappingTable.physicalInterface.devPort.portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, virtPortMappingTablePtr->physicalInterface.devPort.portNum = %d",
                                        dev, virtPortMappingTable.physicalInterface.devPort.portNum);

        virtPortMappingTable.physicalInterface.devPort.portNum = 0;

        /*
            1.5. Call with virtPortMappingTablePtr->physicalInterface.vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS]
                           and other params from 1.1. (not relevant)
            Expected: GT_OK.
        */
        virtPortMappingTable.physicalInterface.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, virtPortMappingTablePtr->physicalInterface.vlanId = %d",
                                     dev, virtPortMappingTable.physicalInterface.vlanId);

        virtPortMappingTable.physicalInterface.vlanId = 0;

        /*
            1.6. Call with out of range virtDevNum[32]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        virtDevNum = 32;

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, virtDevNum = %d", dev, virtDevNum);

        virtDevNum = 0;

        /*
            1.7. Call with out of range virtPortNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        virtPortNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, virtPortNum = %d", dev, virtPortNum);

        virtPortNum = 0;

        /*
            1.8. Call with virtPortMappingTablePtr->physicalInterface.type [0x5AAAAAA5]
                           and other params from 1.1.
            Expected: GT_BAD_PARAM.
        */
        virtPortMappingTable.physicalInterface.type = 0x5AAAAAA5;

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "%d, virtPortMappingTablePtr->physicalInterface.type = %d",
                                     dev, virtPortMappingTable.physicalInterface.type);

        virtPortMappingTable.physicalInterface.type = CPSS_INTERFACE_PORT_E;

        /*
            1.9. Call with virtDevNum[1],
                           virtPortNum [1],
                           virtPortMappingTablePtr{ isTunnelStart[GT_TRUE],
                                                    tunnelStartPointer[0],
                                                    physicalInterface{  type[CPSS_INTERFACE_TRUNK_E],
                                                                        trunkId [100] }
                                                    srcIdFilterBitmap[0xFFFFFFFF] }
            Expected: GT_OK.
        */
        virtDevNum  = 1;
        virtPortNum = 1;
        virtPortMappingTable.isTunnelStart          = GT_FALSE;
        virtPortMappingTable.tunnelStartPointer     = 0;
        virtPortMappingTable.physicalInterface.type = CPSS_INTERFACE_TRUNK_E;
        virtPortMappingTable.srcIdFilterBitmap      = 0xFFFFFFFF;

        virtPortMappingTable.physicalInterface.trunkId = 100;

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, virtDevNum, virtPortNum);

        /*
            1.2. Call cpssDxCh3pCapwapGenVirtualPortMappingEntryGet with not NULL virtPortMappingTablePtr
                                                                         and other params from 1.1.
                                                                         (the same for 1.9, 1.10, 1.13)
            Expected: GT_OK.
        */
        st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, virtPortNum, &virtPortMappingTableGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenVirtualPortMappingEntryGet: %d, %d, %d",
                                     dev, virtDevNum, virtPortNum);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.isTunnelStart, virtPortMappingTableGet.isTunnelStart,
                                     "got another virtPortMappingTablePtr->isTunnelStart then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.tunnelStartPointer, virtPortMappingTableGet.tunnelStartPointer,
                                     "got another virtPortMappingTablePtr->tunnelStartPointer then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.physicalInterface.type, virtPortMappingTableGet.physicalInterface.type,
                                     "got another virtPortMappingTablePtr->physicalInterface.type then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.physicalInterface.trunkId, virtPortMappingTableGet.physicalInterface.trunkId,
                                     "got another virtPortMappingTablePtr->physicalInterface.trunkId then was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.srcIdFilterBitmap, virtPortMappingTableGet.srcIdFilterBitmap,
                                     "got another virtPortMappingTablePtr->srcIdFilterBitmap then was set: %d", dev);

        /*
            1.10. Call with virtPortMappingTablePtr->physicalInterface.devPort.hwDevNum [PRV_CPSS_MAX_PP_DEVICES_CNS]
                            and other params from 1.9. (Not relevant)
            Expected: GT_OK.
        */
        virtPortMappingTable.physicalInterface.devPort.hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, virtPortMappingTablePtr->physicalInterface.devPort.hwDevNum = %d",
                                    dev, virtPortMappingTable.physicalInterface.devPort.hwDevNum);

        virtPortMappingTable.physicalInterface.devPort.hwDevNum = 0;

        /*
            1.11. Call with virtPortMappingTablePtr-> physicalInterface.devPort.portNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                            and other params from 1.9. (Not relevant)
            Expected: GT_OK.
        */
        virtPortMappingTable.physicalInterface.devPort.portNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, virtPortMappingTablePtr->physicalInterface.devPort.portNum = %d",
                                    dev, virtPortMappingTable.physicalInterface.devPort.portNum);

        virtPortMappingTable.physicalInterface.devPort.portNum = 0;

        /*
            1.12. Call with virtPortMappingTablePtr-> physicalInterface.vlanId [PRV_CPSS_MAX_NUM_VLANS_CNS] (not used this field)
                            and other params from 1.13.
            Expected: GT_OK.
        */
        virtPortMappingTable.physicalInterface.vlanId = PRV_CPSS_MAX_NUM_VLANS_CNS;

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "%d, virtPortMappingTablePtr->physicalInterface.vlanId = %d",
                                     dev, virtPortMappingTable.physicalInterface.vlanId);

        virtPortMappingTable.physicalInterface.vlanId = 0;

        /*
            1.13. Call with virtPortMappingTablePtr[NULL]
                            and other params from 1.13.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, virtPortMappingTablePtr = NULL", dev);
    }

    virtDevNum  = 0;
    virtPortNum = 0;
    virtPortMappingTable.isTunnelStart          = GT_TRUE;
    virtPortMappingTable.tunnelStartPointer     = 0;
    virtPortMappingTable.physicalInterface.type = CPSS_INTERFACE_PORT_E;
    virtPortMappingTable.srcIdFilterBitmap      = 0xFFFFFFFF;

    virtPortMappingTable.physicalInterface.devPort.hwDevNum  = 0;
    virtPortMappingTable.physicalInterface.devPort.portNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenVirtualPortMappingEntryGet
(
    IN  GT_U8   devNum,
    IN  GT_U8   virtDevNum,
    IN  GT_U8   virtPortNum,
    OUT CPSS_DXCH3P_CAPWAP_GEN_VIRTUAL_PORT_MAPPING_ENTRY_STC *virtPortMappingTablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenVirtualPortMappingEntryGet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with virtDevNum[0 / 31],
                   virtPortNum[0 / 63]
                   and not NULL virtPortMappingTablePtr.
    Expected: GT_OK.
    1.2. Call with out of range virtDevNum[PRV_CPSS_MAX_PP_DEVICES_CNS]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with out of range virtPortNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with virtPortMappingTablePtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U8   virtDevNum  = 0;
    GT_U8   virtPortNum = 0;

    CPSS_DXCH3P_CAPWAP_GEN_VIRTUAL_PORT_MAPPING_ENTRY_STC virtPortMappingTable;

    cpssOsBzero((GT_VOID*) &virtPortMappingTable, sizeof(virtPortMappingTable));


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with virtDevNum[0 / 31],
                           virtPortNum[0 / 63]
                           and not NULL virtPortMappingTablePtr.
            Expected: GT_OK.
        */
        /* iterate with virtDevNum  = 0 */
        virtDevNum  = 0;
        virtPortNum = 0;

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, virtDevNum, virtPortNum);

        /* iterate with virtDevNum  = 31 */
        virtDevNum  = 31;
        virtPortNum = 63;

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, virtDevNum, virtPortNum);

        /*
            1.2. Call with out of range virtDevNum[PRV_CPSS_MAX_PP_DEVICES_CNS]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        virtDevNum = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, virtDevNum);

        virtDevNum = 0;

        /*
            1.3. Call with out of range virtPortNum [UTF_CPSS_PP_MAX_PORTS_NUM_CNS]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        virtPortNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, virtPortNum = %d", dev, virtPortNum);

        virtPortNum = 0;

        /*
            1.4. Call with virtPortMappingTablePtr [NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, virtPortNum, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, virtPortMappingTablePtr = NULL", dev);
    }

    virtDevNum  = 0;
    virtPortNum = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet
(
    IN GT_U8    devNum,
    IN GT_U32   tableIndex,
    IN GT_U32   tidIndex,
    IN CPSS_DXCH3P_CAPWAP_GEN_TID_QOS_MAPPING_ENTRY_STC *qosEntryPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with tableIndex[0 / 7],
                   tidIndex[0 / 15],
                   qosEntryPtr{ qosProfile [0 / 127],
                                up [0 / 7] }.
    Expected: GT_OK
    1.2. Call cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet with not NULL qosEntryPtrandPtr
                                                                     other params from 1.1.
    Expected: GT_OK and same qosEntryPtrand as was set.
    1.3. Call with out of range tableIndex[8]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range tidIndex [16]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with out of range qosEntryPtr->qosProfile [128]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.6. Call with out of range qosEntryPtr->up [8]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.7. Call with qosEntryPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32   tableIndex = 0;
    GT_U32   tidIndex   = 0;
    GT_BOOL  isEqual    = GT_FALSE;

    CPSS_DXCH3P_CAPWAP_GEN_TID_QOS_MAPPING_ENTRY_STC qosEntry;
    CPSS_DXCH3P_CAPWAP_GEN_TID_QOS_MAPPING_ENTRY_STC qosEntryGet;


    cpssOsBzero((GT_VOID*) &qosEntry, sizeof(qosEntry));
    cpssOsBzero((GT_VOID*) &qosEntryGet, sizeof(qosEntryGet));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tableIndex[0 / 7],
                           tidIndex[0 / 15],
                           qosEntryPtr{ qosProfile [0 / 127],
                                        up [0 / 7] }.
            Expected: GT_OK
        */
        /* iterate with tableIndex = 0 */
        tableIndex = 0;
        tidIndex   = 0;

        qosEntry.qosProfile = 0;
        qosEntry.up         = 0;

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tableIndex, tidIndex);

        /*
            1.2. Call cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet with not NULL qosEntryPtrandPtr
                                                                             other params from 1.1.
            Expected: GT_OK and same qosEntryPtrand as was set.
        */
        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, tableIndex, tidIndex, &qosEntryGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet: %d, %d, %d",
                                    dev, tableIndex, tidIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &qosEntry,
                                     (GT_VOID*) &qosEntryGet,
                                     sizeof(qosEntry) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another qosEntry than was set: %d", dev);

        /* iterate with tableIndex = 7 */
        tableIndex = 7;
        tidIndex   = 15;

        qosEntry.qosProfile = 127;
        qosEntry.up         = 7;

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tableIndex, tidIndex);

        /*
            1.2. Call cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet with not NULL qosEntryPtrandPtr
                                                                             other params from 1.1.
            Expected: GT_OK and same qosEntryPtrand as was set.
        */
        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, tableIndex, tidIndex, &qosEntryGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet: %d, %d, %d",
                                    dev, tableIndex, tidIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &qosEntry,
                                     (GT_VOID*) &qosEntryGet,
                                     sizeof(qosEntry) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another qosEntry than was set: %d", dev);

        /*
            1.3. Call with out of range tableIndex[8]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        tableIndex = 8;

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tableIndex);

        tableIndex = 0;

        /*
            1.4. Call with out of range tidIndex [16]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        tidIndex = 16;

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tidIndex = %d", dev, tidIndex);

        tidIndex = 0;

        /*
            1.5. Call with out of range qosEntryPtr-> qosProfile [128]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        qosEntry.qosProfile = 128;

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, qosEntryPtr->qosProfile = %d", dev, qosEntry.qosProfile);

        qosEntry.qosProfile = 0;

        /*
            1.6. Call with out of range qosEntryPtr->up [8]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        qosEntry.up = 8;

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, qosEntryPtr->up = %d", dev, qosEntry.up);

        qosEntry.up = 0;

        /*
            1.7. Call with qosEntryPtr [NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, tidIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, qosEntryPtr = NULL", dev);
    }

    tableIndex = 0;
    tidIndex   = 0;

    qosEntry.qosProfile = 0;
    qosEntry.up         = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, tidIndex, &qosEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   tableIndex,
    IN  GT_U32   tidIndex,
    OUT CPSS_DXCH3P_CAPWAP_GEN_TID_QOS_MAPPING_ENTRY_STC *qosEntryPtr

)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with tableIndex[7],
                   tidIndex[15]
                   and not NULL qosEntryPtr.
    Expected: GT_OK
    1.2. Call with out of range tableIndex[8]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with out of range tidIndex [16]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with qosEntryPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32   tableIndex = 0;
    GT_U32   tidIndex   = 0;

    CPSS_DXCH3P_CAPWAP_GEN_TID_QOS_MAPPING_ENTRY_STC qosEntry;


    cpssOsBzero((GT_VOID*) &qosEntry, sizeof(qosEntry));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with tableIndex[7],
                           tidIndex[15]
                           and not NULL qosEntryPtr.
            Expected: GT_OK
        */
        tableIndex = 7;
        tidIndex   = 15;

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, tableIndex, tidIndex);

        /*
            1.2. Call with out of range tableIndex[8]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        tableIndex = 8;

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, tableIndex);

        tableIndex = 7;

        /*
            1.3. Call with out of range tidIndex [16]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        tidIndex = 16;

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, tidIndex = %d", dev, tidIndex);

        tidIndex = 7;

        /*
            1.4. Call with qosEntryPtr [NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, tableIndex, tidIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, qosEntryPtr = NULL", dev);
    }

    tableIndex = 7;
    tidIndex   = 15;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, tableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, tableIndex, tidIndex, &qosEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       qosProfileMappingTableIndex,
    IN GT_U32                                       qosProfileIndex,
    IN CPSS_DXCH3P_CAPWAP_GEN_QOS_CONTROL_ENTRY_STC *qosControlPtr

)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with qosProfileMappingTableIndex [0 / 3],
                   qosProfileIndex [0 / 127],
                   qosControl { tid[0 / 0xFFFFFFFF],
                                ackPolicy[0 / 0xFFFFFFFF] }
    Expected: GT_OK.
    1.2. Call cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet with not NULL qosControlPtr
                                                                     and other params from 1.1.
    Expected: GT_OK.
    1.3. Call with out of range qosProfileMappingTableIndex [4]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range qosProfileIndex [128]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.5. Call with out of range qosControlPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      qosProfileMappingTableIndex = 0;
    GT_U32      qosProfileIndex             = 0;
    GT_BOOL     isEqual                     = GT_FALSE;

    CPSS_DXCH3P_CAPWAP_GEN_QOS_CONTROL_ENTRY_STC qosControl;
    CPSS_DXCH3P_CAPWAP_GEN_QOS_CONTROL_ENTRY_STC qosControlGet;


    cpssOsBzero((GT_VOID*) &qosControl, sizeof(qosControl));
    cpssOsBzero((GT_VOID*) &qosControlGet, sizeof(qosControlGet));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with qosProfileMappingTableIndex [0 / 3],
                           qosProfileIndex [0 / 127],
                           qosControl { tid[0 / 0xF],
                                        ackPolicy[0 / 0x3] }
            Expected: GT_OK.
        */
        /* iterate with qosProfileIndex = 0 */
        qosProfileMappingTableIndex = 0;
        qosProfileIndex             = 0;

        qosControl.tid       = 0;
        qosControl.ackPolicy = 0;

        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfileMappingTableIndex, qosProfileIndex);

        /*
            1.2. Call cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet with not NULL qosControlPtr
                                                                             and other params from 1.1.
            Expected: GT_OK.
        */
        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControlGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet: %d, %d, %d",
                                    dev, qosProfileMappingTableIndex, qosProfileIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &qosControl, (GT_VOID*) &qosControlGet, sizeof(qosControl) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another qosControl than was set: %d", dev);

        /* iterate with qosProfileIndex = 0 */
        qosProfileMappingTableIndex = 3;
        qosProfileIndex             = 127;

        qosControl.tid       = 15;
        qosControl.ackPolicy = 3;

        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfileMappingTableIndex, qosProfileIndex);

        /*
            1.2. Call cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet with not NULL qosControlPtr
                                                                             and other params from 1.1.
            Expected: GT_OK.
        */
        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControlGet);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet: %d, %d, %d",
                                    dev, qosProfileMappingTableIndex, qosProfileIndex);

        isEqual = (0 == cpssOsMemCmp((GT_VOID*) &qosControl, (GT_VOID*) &qosControlGet, sizeof(qosControl) )) ? GT_TRUE : GT_FALSE;
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another qosControl than was set: %d", dev);

        /*
            1.3. Call with out of range qosProfileMappingTableIndex [4]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        qosProfileMappingTableIndex = 4;

        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfileMappingTableIndex);

        qosProfileMappingTableIndex = 0;

        /*
            1.4. Call with out of range qosProfileIndex [128]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        qosProfileIndex = 128;

        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, qosProfileIndex = %d", dev, qosProfileIndex);

        qosProfileIndex = 0;

        /*
            1.5. Call with out of range qosControlPtr [NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet(dev, qosProfileMappingTableIndex, qosProfileIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, qosControlPtr = NULL", dev);
    }

    qosProfileMappingTableIndex = 0;
    qosProfileIndex             = 0;

    qosControl.tid       = 0;
    qosControl.ackPolicy = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       qosProfileMappingTableIndex,
    IN  GT_U32                                       qosProfileIndex,
    OUT CPSS_DXCH3P_CAPWAP_GEN_QOS_CONTROL_ENTRY_STC *qosControlPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet)
{
/*
    ITERATE_DEVICES (DxCh3p and above)
    1.1. Call with qosProfileMappingTableIndex[3],
                   qosProfileIndex[127]
                   and not NULL qosControlPtr
    Expected: GT_OK.
    1.2. Call with out of range qosProfileMappingTableIndex [4]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.3. Call with out of range qosProfileIndex [128]
                   and other params from 1.1.
    Expected: NOT GT_OK.
    1.4. Call with out of range qosControlPtr [NULL]
                   and other params from 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;


    GT_U32      qosProfileMappingTableIndex = 0;
    GT_U32      qosProfileIndex             = 0;

    CPSS_DXCH3P_CAPWAP_GEN_QOS_CONTROL_ENTRY_STC qosControl;


    cpssOsBzero((GT_VOID*) &qosControl, sizeof(qosControl));

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with qosProfileMappingTableIndex[3],
                           qosProfileIndex[127]
                           and not NULL qosControlPtr
            Expected: GT_OK.
        */
        qosProfileMappingTableIndex = 3;
        qosProfileIndex             = 127;

        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, qosProfileMappingTableIndex, qosProfileIndex);

        /*
            1.2. Call with out of range qosProfileMappingTableIndex [4]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        qosProfileMappingTableIndex = 4;

        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, qosProfileMappingTableIndex);

        qosProfileMappingTableIndex = 3;

        /*
            1.3. Call with out of range qosProfileIndex [128]
                           and other params from 1.1.
            Expected: NOT GT_OK.
        */
        qosProfileIndex = 128;

        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, qosProfileIndex = %d", dev, qosProfileIndex);

        qosProfileIndex = 127;

        /*
            1.4. Call with out of range qosControlPtr [NULL]
                           and other params from 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, qosProfileMappingTableIndex, qosProfileIndex, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, qosControlPtr = NULL", dev);
    }

    qosProfileMappingTableIndex = 3;
    qosProfileIndex             = 127;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill RSSI Weight Profile table.
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenFillRssiWeightProfileTable)
{
/*
    ITERATE_DEVICE (DxCh3p)
    1.1. Get table Size. numEntries [3].
    1.2. Fill all entries in RSSI Weight Profile table.
         Call cpssDxCh3pCapwapGenRssiWeightProfileEntrySet profileIndex [0...numEntries],
                           rssiProfileEntryPtr{ weight[0],
                                                minRssiPacketTreshold[0],
                                                minRssiAverageTreshold[0] }
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxCh3pCapwapGenRssiWeightProfileEntrySet profileIndex [numEntries] and other params from 1.2.
    Expected: NOT GT_OK.
    1.4. Read all entries in RSSI Weight Profile table and compare with original.
         Call cpssDxCh3pCapwapGenRssiWeightProfileEntryGet with not NULL rssiProfileEntryPtr
                                                                and other params from 1.2.
    Expected: GT_OK and the same rssiProfileEntry as was set.
    1.5. Try to read entry with index out of range.
         Call cpssDxCh3pCapwapGenRssiWeightProfileEntryGet with profileIndex [numEntries].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_BOOL     isEqual    = GT_FALSE;
    GT_U32      numEntries = 0;
    GT_U32      iTemp      = 0;

    CPSS_DXCH3P_CAPWAP_GEN_RSSI_WEIGHT_PROFILE_ENTRY_STC    rssiProfileEntry;
    CPSS_DXCH3P_CAPWAP_GEN_RSSI_WEIGHT_PROFILE_ENTRY_STC    rssiProfileEntryGet;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &rssiProfileEntry, sizeof(rssiProfileEntry));
    cpssOsBzero((GT_VOID*) &rssiProfileEntryGet, sizeof(rssiProfileEntryGet));

    /* Fill the entry for RSSI Weight Profile table */
    rssiProfileEntry.weight                 = 0;
    rssiProfileEntry.minRssiPacketTreshold  = 0;
    rssiProfileEntry.minRssiAverageTreshold = 0;

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        numEntries = 4;

        /* 1.2. Fill all entries in RSSI Weight Profile table */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, iTemp, &rssiProfileEntry);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenRssiWeightProfileEntrySet: %d, %d", dev, iTemp);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxCh3pCapwapGenRssiWeightProfileEntrySet(dev, numEntries, &rssiProfileEntry);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenRssiWeightProfileEntrySet: %d, %d", dev, numEntries);

        /* 1.4. Read all entries in RSSI Weight Profile table and compare with original */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            st = cpssDxCh3pCapwapGenRssiWeightProfileEntryGet(dev, iTemp, &rssiProfileEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenRssiWeightProfileEntryGet: %d, %d", dev, iTemp);

            /* verifying values */
            isEqual = (0 == cpssOsMemCmp((GT_VOID*) &rssiProfileEntry,
                                         (GT_VOID*) &rssiProfileEntryGet,
                                         sizeof(rssiProfileEntry) )) ? GT_TRUE : GT_FALSE;
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another rssiProfileEntry then was set: %d", dev);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxCh3pCapwapGenRssiWeightProfileEntryGet(dev, numEntries, &rssiProfileEntryGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenRssiWeightProfileEntryGet: %d, %d", dev, numEntries);
    }
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill virtual port mapping table.
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenFillVirtualPortMappingTable)
{
/*
    ITERATE_DEVICE (DXCH3)
    1.1. Get table Size. maxVirtDev [32], maxVirtPort [BIT_6].
    1.2. Fill all entries in virtual port mapping table.
            Call cpssDxCh3pCapwapGenVirtualPortMappingEntrySet with virtDevNum[0],
                                                   virtPortNum [0],
                                                   virtPortMappingTablePtr{ isTunnelStart[GT_TRUE],
                                                                            tunnelStartPointer[0],
                                                                            physicalInterface{  type[CPSS_INTERFACE_PORT_E],
                                                                                                devPort { devNum[dev], portNum[0]} }
                                                                            srcIdFilterBitmap[0xFFFFFFFF] }
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
            Call cpssDxCh3pCapwapGenVirtualPortMappingEntrySet with virtDevNum[maxVirtDev],
                                                                    virtPortNum [maxVirtPort].
    Expected: NOT GT_OK.
    1.4. Read all entries in virtual port mapping table and compare with original.
            Call cpssDxCh3pCapwapGenVirtualPortMappingEntryGet with not NULL virtPortMappingTablePtr and other params from 1.2.
    Expected: GT_OK.
    1.5. Try to read entry with index out of range.
            Call cpssDxCh3pCapwapGenVirtualPortMappingEntryGet with virtDevNum[maxVirtDev],
                                                                    virtPortNum [maxVirtPort].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U32      maxVirtDev  = 0;
    GT_U32      maxVirtPort = 0;
    GT_U8       virtDevNum  = 0;
    GT_U8       virtPortNum = 0;

    CPSS_DXCH3P_CAPWAP_GEN_VIRTUAL_PORT_MAPPING_ENTRY_STC virtPortMappingTable;
    CPSS_DXCH3P_CAPWAP_GEN_VIRTUAL_PORT_MAPPING_ENTRY_STC virtPortMappingTableGet;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &virtPortMappingTable, sizeof(virtPortMappingTable));
    cpssOsBzero((GT_VOID*) &virtPortMappingTableGet, sizeof(virtPortMappingTableGet));

    /* Fill the entry for virtual port mapping table */
    virtDevNum  = 0;
    virtPortNum = 0;
    virtPortMappingTable.isTunnelStart          = GT_TRUE;
    virtPortMappingTable.tunnelStartPointer     = 0;
    virtPortMappingTable.physicalInterface.type = CPSS_INTERFACE_PORT_E;
    virtPortMappingTable.srcIdFilterBitmap      = 0xFFFFFFFF;

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Fill the physicalInterface for entry */
        virtPortMappingTable.physicalInterface.devPort.hwDevNum  = dev;
        virtPortMappingTable.physicalInterface.devPort.portNum = 0;

        /* 1.1. Get table Size */
        maxVirtDev  = 32;
        maxVirtPort = BIT_6;

        /* 1.2. Fill all entries in virtual port mapping table */
        for(virtDevNum = 0; virtDevNum < maxVirtDev; ++virtDevNum)
            for(virtPortNum = 0; virtPortNum < maxVirtPort; ++virtPortNum)
            {
                st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, virtPortNum, &virtPortMappingTable);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenVirtualPortMappingEntrySet: %d, %d, %d", dev, virtDevNum, virtPortNum);
            }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, (GT_U8)(maxVirtDev), virtPortNum, &virtPortMappingTable);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenVirtualPortMappingEntrySet: %d, %d, %d",
                                         dev, maxVirtDev, virtPortNum);

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntrySet(dev, virtDevNum, (GT_U8)(maxVirtPort), &virtPortMappingTable);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenVirtualPortMappingEntrySet: %d, %d, %d",
                                         dev, virtDevNum, maxVirtPort);

        /* 1.4. Read all entries in virtual port mapping table and compare with original */
        for(virtDevNum = 0; virtDevNum < maxVirtDev; ++virtDevNum)
            for(virtPortNum = 0; virtPortNum < maxVirtPort; ++virtPortNum)
            {
                st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, virtPortNum, &virtPortMappingTableGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenVirtualPortMappingEntryGet: %d, %d, %d",
                                             dev, virtDevNum, virtPortNum);

                /* verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.isTunnelStart, virtPortMappingTableGet.isTunnelStart,
                                             "got another virtPortMappingTablePtr->isTunnelStart then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.tunnelStartPointer, virtPortMappingTableGet.tunnelStartPointer,
                                             "got another virtPortMappingTablePtr->tunnelStartPointer then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.physicalInterface.type, virtPortMappingTableGet.physicalInterface.type,
                                             "got another virtPortMappingTablePtr->physicalInterface.type then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.physicalInterface.devPort.hwDevNum, virtPortMappingTableGet.physicalInterface.devPort.hwDevNum,
                                             "got another virtPortMappingTablePtr->physicalInterface.devPort.hwDevNum then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.physicalInterface.devPort.portNum, virtPortMappingTableGet.physicalInterface.devPort.portNum,
                                             "got another virtPortMappingTablePtr->physicalInterface.devPort.portNum then was set: %d", dev);
                UTF_VERIFY_EQUAL1_STRING_MAC(virtPortMappingTable.srcIdFilterBitmap, virtPortMappingTableGet.srcIdFilterBitmap,
                                             "got another virtPortMappingTablePtr->srcIdFilterBitmap then was set: %d", dev);
            }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, (GT_U8)(maxVirtDev), virtPortNum, &virtPortMappingTableGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenVirtualPortMappingEntryGet: %d, %d, %d",
                                         dev, maxVirtDev, virtPortNum);

        st = cpssDxCh3pCapwapGenVirtualPortMappingEntryGet(dev, virtDevNum, (GT_U8)(maxVirtPort), &virtPortMappingTableGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenVirtualPortMappingEntryGet: %d, %d, %d",
                                         dev, virtDevNum, maxVirtPort);
    }
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill TID-to-QoS Mapping table.
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenFillTidtoQoSMappingTable)
{
/*
    ITERATE_DEVICE (DxCh3p)
    1.1. Get table Size. maxTableIndex [8], maxTidIndex [16].
    1.2. Fill all entries in TID-to-QoS Mapping table.
         Call cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet with tableIndex[0 ... maxTableIndex-1],
                           tidIndex[0 ... maxTidIndex-1],
                           qosEntryPtr{ qosProfile [0],
                                        up [0] }.
    Expected: GT_OK
    1.3. Try to write entry with index out of range.
         Call cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet with tableIndex [maxTableIndex], tidIndex [maxTidIndex].
    Expected: NOT GT_OK.
    1.4. Read all entries in TID-to-QoS Mapping table and compare with original.
         Call cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet with not NULL qosEntryPtrandPtr other params from 1.2.
    Expected: GT_OK and same qosEntryPtrand as was set.
    1.5. Try to read entry with index out of range.
         Call cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet with tableIndex [maxTableIndex], tidIndex [maxTidIndex].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     isEqual = GT_FALSE;

    GT_U32      maxTableIndex = 0;
    GT_U32      maxTidIndex   = 0;
    GT_U32      tableIndex    = 0;
    GT_U32      tidIndex      = 0;

    CPSS_DXCH3P_CAPWAP_GEN_TID_QOS_MAPPING_ENTRY_STC qosEntry;
    CPSS_DXCH3P_CAPWAP_GEN_TID_QOS_MAPPING_ENTRY_STC qosEntryGet;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &qosEntry, sizeof(qosEntry));
    cpssOsBzero((GT_VOID*) &qosEntryGet, sizeof(qosEntryGet));

    /* Fill the entry for TID-to-QoS Mapping table */
    qosEntry.qosProfile = 0;
    qosEntry.up         = 0;

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        maxTableIndex   = 8;
        maxTidIndex     = 16;

        /* 1.2. Fill all entries in TID-to-QoS Mapping table */
        for(tableIndex = 0; tableIndex < maxTableIndex; ++tableIndex)
            for(tidIndex = 0; tidIndex < maxTidIndex; ++tidIndex)
            {
                st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, tidIndex, &qosEntry);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet: %d, %d, %d", dev, tableIndex, tidIndex);
            }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, maxTableIndex, tidIndex, &qosEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet: %d, %d, %d",
                                         dev, maxTableIndex, tidIndex);

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet(dev, tableIndex, maxTidIndex, &qosEntry);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet: %d, %d, %d",
                                         dev, tableIndex, maxTidIndex);

        /* 1.4. Read all entries in TID-to-QoS Mapping table and compare with original */
        for(tableIndex = 0; tableIndex < maxTableIndex; ++tableIndex)
            for(tidIndex = 0; tidIndex < maxTidIndex; ++tidIndex)
            {
                st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, tableIndex, tidIndex, &qosEntryGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet: %d, %d, %d",
                                            dev, tableIndex, tidIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &qosEntry,
                                             (GT_VOID*) &qosEntryGet,
                                             sizeof(qosEntry) )) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another qosEntry than was set: %d", dev);
            }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, maxTableIndex, tidIndex, &qosEntryGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet: %d, %d, %d",
                                         dev, maxTableIndex, tidIndex);

        st = cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet(dev, tableIndex, maxTidIndex, &qosEntryGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet: %d, %d, %d",
                                         dev, tableIndex, maxTidIndex);
    }
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill QoS-Profile-to-802.11e Mapping table.
*/
UTF_TEST_CASE_MAC(cpssDxCh3pCapwapGenFillQosProfileToTidMappingTable)
{
/*
    ITERATE_DEVICE (DxCh3p)
    1.1. Get table Size.
            maxQosProfileMappingTableIndex [BIT_2],
            maxQosProfileIndex [BIT_7].
    1.2. Fill all entries in QoS-Profile-to-802.11e Mapping table.
         Call cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet with qosProfileMappingTableIndex [0 ... maxQosProfileMappingTableIndex-1],
                                                                       qosProfileIndex [0 ... maxQosProfileIndex-1],
                                                                       qosControl { tid[0], ackPolicy[0] }
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet with
                                    qosProfileMappingTableIndex [maxQosProfileMappingTableIndex],
                                    qosProfileIndex [maxQosProfileIndex].
    Expected: NOT GT_OK.
    1.4. Read all entries in QoS-Profile-to-802.11e Mapping and compare with original.
         Call cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet with not NULL qosControlPtr and other params from 1.2.
    Expected: GT_OK.
    1.5. Try to read entry with index out of range.
         Call cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet
                                    qosProfileMappingTableIndex [maxQosProfileMappingTableIndex],
                                    qosProfileIndex [maxQosProfileIndex].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     isEqual = GT_FALSE;

    GT_U32      maxQosProfileMappingTableIndex = 0;
    GT_U32      maxQosProfileIndex             = 0;
    GT_U32      qosProfileMappingTableIndex    = 0;
    GT_U32      qosProfileIndex                = 0;

    CPSS_DXCH3P_CAPWAP_GEN_QOS_CONTROL_ENTRY_STC qosControl;
    CPSS_DXCH3P_CAPWAP_GEN_QOS_CONTROL_ENTRY_STC qosControlGet;


    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    cpssOsBzero((GT_VOID*) &qosControl, sizeof(qosControl));
    cpssOsBzero((GT_VOID*) &qosControlGet, sizeof(qosControlGet));

    /* Fill the entry for QoS-Profile-to-802.11e Mapping table */
    qosControl.tid       = 0;
    qosControl.ackPolicy = 0;

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        maxQosProfileMappingTableIndex  = BIT_2;
        maxQosProfileIndex              = BIT_7;

        /* 1.2. Fill all entries in QoS-Profile-to-802.11e Mapping table */
        for(qosProfileMappingTableIndex = 0; qosProfileMappingTableIndex < maxQosProfileMappingTableIndex; ++qosProfileMappingTableIndex)
            for(qosProfileIndex = 0; qosProfileIndex < maxQosProfileIndex; ++qosProfileIndex)
            {
                st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControl);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet: %d, %d, %d",
                                             dev, qosProfileMappingTableIndex, qosProfileIndex);
            }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet(dev, maxQosProfileMappingTableIndex, qosProfileIndex, &qosControl);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet: %d, %d, %d",
                                         dev, maxQosProfileMappingTableIndex, qosProfileIndex);

        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet(dev, qosProfileMappingTableIndex, maxQosProfileIndex, &qosControl);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet: %d, %d, %d",
                                         dev, qosProfileMappingTableIndex, maxQosProfileIndex);

        /* 1.4. Read all entries in QoS-Profile-to-802.11e Mapping table and compare with original */
        for(qosProfileMappingTableIndex = 0; qosProfileMappingTableIndex < maxQosProfileMappingTableIndex; ++qosProfileMappingTableIndex)
            for(qosProfileIndex = 0; qosProfileIndex < maxQosProfileIndex; ++qosProfileIndex)
            {
                st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, qosProfileMappingTableIndex, qosProfileIndex, &qosControlGet);
                UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet: %d, %d, %d",
                                            dev, qosProfileMappingTableIndex, qosProfileIndex);

                isEqual = (0 == cpssOsMemCmp((GT_VOID*) &qosControl, (GT_VOID*) &qosControlGet, sizeof(qosControl) )) ? GT_TRUE : GT_FALSE;
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isEqual, "got another qosControl than was set: %d", dev);
            }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, maxQosProfileMappingTableIndex, qosProfileIndex, &qosControlGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet: %d, %d, %d",
                                         dev, maxQosProfileMappingTableIndex, qosProfileIndex);

        st = cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet(dev, qosProfileMappingTableIndex, maxQosProfileIndex, &qosControlGet);
        UTF_VERIFY_NOT_EQUAL3_STRING_MAC(GT_OK, st, "cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet: %d, %d, %d",
                                         dev, qosProfileMappingTableIndex, maxQosProfileIndex);
    }
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxCh3pCapwapGen suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxCh3pCapwapGen)
    /* TBD - not supported yet */
    /*UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenReassemblyEngineContextsNumberSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenReassemblyEngineContextsNumberGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenReassemblyEngineAgingTimeoutGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenFragmentDiscardGlobalCounterGet)*/
    /* TBD - not supported yet */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenRssiWeightProfileEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenRssiWeightProfileEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenRssiExceptionStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenVirtualDevicesSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenVirtualDevicesGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenVirtualPortMappingEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenVirtualPortMappingEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenTidToQoSProfileMappingEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenTidToQoSProfileMappingEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenQosProfileToTidMappingEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenQosProfileToTidMappingEntryGet)
    /* Tests for tables */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenFillRssiWeightProfileTable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenFillVirtualPortMappingTable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenFillTidtoQoSMappingTable)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pCapwapGenFillQosProfileToTidMappingTable)
UTF_SUIT_END_TESTS_MAC(cpssDxCh3pCapwapGen)

