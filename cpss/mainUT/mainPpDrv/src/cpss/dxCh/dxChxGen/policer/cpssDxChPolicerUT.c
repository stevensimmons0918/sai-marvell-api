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
* @file cpssDxChPolicerUT.c
*
* @brief Unit tests for cpssDxChPolicer, that provides
* Ingress Policing Engine function implementations.
*
* @version   17
********************************************************************************
*/
/* includes */
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerInit
(
    IN GT_U8                                    devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerInit)
{
/*
    ITERATE_DEVICES (DxCh1, DxCh2)
    1.1. Call function with not null dev.
    Expected: GT_OK.
*/
    GT_STATUS                         st   = GT_OK;
    GT_U8                             dev;

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call function with not null dev.
            Expected: GT_OK.
        */

        st = cpssDxChPolicerInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerInit(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet
(
        IN GT_U8            devNum,
        IN GT_BOOL          enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet)
{
/*
    ITERATE_DEVICE (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    Expected: GT_OK.
    1.2. Call cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet with [GT_TRUE/GT_FALSE].
    Expected: GT_OK and the same enable as was set.
    1.3. Call with wrong device
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    GT_BOOL     state    = GT_TRUE;
    GT_BOOL     stateGet = GT_TRUE;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_TRUE and GT_FALSE].
            Expected: GT_OK.
        */

        /* Call function with enable [GT_FALSE] */
        state = GT_FALSE;

        st = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChEthernetOverMplsTunnelStartTaggingGet: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d", dev);

        /* Call function with enable [GT_TRUE] */
        state = GT_TRUE;

        st = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(dev, state);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, state);

        /*
            1.2. Call cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet.
            Expected: GT_OK and the same enable.
        */
        st = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet(dev, &stateGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                     "cpssDxChEthernetOverMplsTunnelStartTaggingGet: %d", dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(state, stateGet,
                                     "get another enable than was set: %d", dev);
    }

    state = GT_TRUE;
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(dev, state);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(dev, state);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet
(
    IN   GT_U8          devNum,
    OUT  GT_BOOL       *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet)
{
/*
    ITERATE_DEVICE (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
    1.1. Call with not NULL enablePtr.
    Expected: GT_OK.
    1.2. Call enablePtr [NULL].
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      notAppFamilyBmp = 0;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not NULL enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call enablePtr [NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, StatePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_BOBCAT2_E |
         UTF_CAELUM_E| UTF_BOBCAT3_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_ALDRIN2_E);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet(dev, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPolicer suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPolicer)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet)
UTF_SUIT_END_TESTS_MAC(cpssDxChPolicer)

