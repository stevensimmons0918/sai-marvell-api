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
* @file cpssDxChHwInitUT.c
*
* @brief Unit tests for cpssDxChHwInitUT, that provides
* CPSS DXCH level basic Hw initialization functions.
*
* @version   51
********************************************************************************
*/
/* includes */

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChModuleConfig.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/driver/interrupts/cpssDrvComIntEvReqQueues.h>
#include <cpss/driver/interrupts/cpssDrvComIntSvcRtn.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterruptsInit.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
/* get DxCh Trunk private types */
#include <cpss/dxCh/dxChxGen/trunk/private/prvCpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/pcl/private/prvCpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/**/
#include <cpss/extServices/cpssExtServices.h>
#include <gtOs/gtOsTimer.h>

#define HWINIT_GLOVAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir._var)

/* Valid port num value used for testing */
#define HWINIT_VALID_PORTNUM_CNS         0

/* Valid period low bound range*/
#define HWINIT_PERIOD_LOW_BOUND_CNS         320

/* Valid period high bound range*/
#define HWINIT_PERIOD_HIGH_BOUND_CNS       5242560

/* maximal number of units in device */
#define MAX_UNIT_NUM_CNS                    35

/* maximal number of register addresses used for trace hw test */
#define MAX_NUM_OF_REG_ADDRS_CNS   15

/* maximal MPP selector numbers */
#define MAX_MPP_SELECTORS_NUM_CNS                   64
/* Hawk maximal MPP selector numbers */
#define AC5P_MAX_MPP_SELECTORS_NUM_CNS              48
/* Harrier maximal MPP selector numbers */
#define HARRIER_MAX_MPP_SELECTORS_NUM_CNS           32
/* maximal MPP selector value */
#define MAX_MPP_SELECTOR_VALUE_CNS                  16

/**
* @struct HWINIT_ERRATA_BAD_ADDR_RANGE_STC
 *
 * @brief Describes restricted address range
*/
typedef struct{

    /** Range start offset */
    GT_U32 addrStart;

    /** Range end offset */
    GT_U32 addrEnd;

} HWINIT_ERRATA_BAD_ADDR_RANGE_STC;


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwAuDescSizeGet
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwAuDescSizeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with descSizePtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32     descSize;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChHwAuDescSizeGet(dev, &descSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with descSizePtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChHwAuDescSizeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwAuDescSizeGet(dev, &descSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwAuDescSizeGet(dev, &descSize);
    /* the devType is not used yet */
    /*UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);*/
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwCoreClockGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *coreClkDbPtr,
    OUT GT_U32  *coreClkHwPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwCoreClockGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with coreClkDbPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with coreClkHwPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32     coreClkDb;
    GT_U32     coreClkHw;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChHwCoreClockGet(dev, &coreClkDb, &coreClkHw);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with coreClkDbPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChHwCoreClockGet(dev, NULL, &coreClkHw);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with coreClkHbPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChHwCoreClockGet(dev, &coreClkDb, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwCoreClockGet(dev, &coreClkDb, &coreClkHw);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwCoreClockGet(dev, &coreClkDb, &coreClkHw);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwInterruptCoalescingSet
(
    IN GT_U8    devNum,
    IN GT_BOOL  enable,
    IN GT_U32   period,
    IN GT_BOOL  linkChangeOverride
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwInterruptCoalescingSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with enable [GT_TRUE / GT_FALSE],
                    period [HWINIT_PERIOD_LOW_BOUND_CNS /
                            (HWINIT_PERIOD_HIGH_BOUND_CNS +
                             HWINIT_PERIOD_LOW_BOUND_CNS) / 2   /
                             HWINIT_PERIOD_HIGH_BOUND_CNS],
                    linkChangeOverride [GT_TRUE / GT_FALSE].
    Expected: GT_OK.
    1.2. Call cpssDxChHwInterruptCoalescingGet with the same params.
    Expected: GT_OK and the same values.
    1.3. Call with enable [GT_FALSE],
                   period [HWINIT_PERIOD_HIGH_BOUND_CNS + 1]
                   (ignored when enable [GT_FALSE]),
                   linkChangeOverride [GT_FALSE].
    Expected: GT_OK.
    1.4. Call with enable [GT_TRUE],
                   period [HWINIT_PERIOD_HIGH_BOUND_CNS + 1]
                   linkChangeOverride [GT_FALSE].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL  enable = GT_TRUE;
    GT_U32   period = 0;
    GT_BOOL  linkChangeOverride = GT_TRUE;

    GT_BOOL  enableGet = GT_FALSE;
    GT_U32   periodGet = 1;
    GT_BOOL  linkChangeOverrideGet = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable [GT_TRUE / GT_FALSE],
                            period [HWINIT_PERIOD_LOW_BOUND_CNS /
                                    (HWINIT_PERIOD_HIGH_BOUND_CNS +
                                     HWINIT_PERIOD_LOW_BOUND_CNS) / 2   /
                                     HWINIT_PERIOD_HIGH_BOUND_CNS],
                            linkChangeOverride [GT_TRUE / GT_FALSE].
            Expected: GT_OK.
        */
        /* call with
                    enable = GT_TRUE;
                    period = HWINIT_PERIOD_LOW_BOUND_CNS;
                    linkChangeOverride = GT_TRUE;
        */

        enable = GT_TRUE;
        period = HWINIT_PERIOD_LOW_BOUND_CNS;
        linkChangeOverride = GT_TRUE;

        st = cpssDxChHwInterruptCoalescingSet(dev, enable, period, linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChHwInterruptCoalescingGet with the same params.
            Expected: GT_OK and the same values.
        */

        st = cpssDxChHwInterruptCoalescingGet(dev, &enableGet, &periodGet,
                                                &linkChangeOverrideGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK ==st)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                 "get another enable than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(period, periodGet,
                 "get another period than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(linkChangeOverride, linkChangeOverrideGet,
                 "get another linkChangeOverride than was set: dev = %d", dev);
        }

        /* call with
                    enable = GT_TRUE;
                    period = HWINIT_PERIOD_LOW_BOUND_CNS;
                    linkChangeOverride = GT_FALSE;
        */

        enable = GT_TRUE;
        period = HWINIT_PERIOD_LOW_BOUND_CNS;
        linkChangeOverride = GT_FALSE;

        st = cpssDxChHwInterruptCoalescingSet(dev, enable, period, linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChHwInterruptCoalescingGet with the same params.
            Expected: GT_OK and the same values.
        */

        st = cpssDxChHwInterruptCoalescingGet(dev, &enableGet, &periodGet,
                                                &linkChangeOverrideGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK ==st)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                 "get another enable than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(period, periodGet,
                 "get another period than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(linkChangeOverride, linkChangeOverrideGet,
                 "get another linkChangeOverride than was set: dev = %d", dev);
        }

        /* call with
                    enable = GT_TRUE;
                    period = (HWINIT_PERIOD_HIGH_BOUND_CNS + HWINIT_PERIOD_LOW_BOUND_CNS) / 2;
                    linkChangeOverride = GT_TRUE;
        */

        enable = GT_TRUE;
        period = (HWINIT_PERIOD_HIGH_BOUND_CNS + HWINIT_PERIOD_LOW_BOUND_CNS) / 2;
        linkChangeOverride = GT_TRUE;

        st = cpssDxChHwInterruptCoalescingSet(dev, enable, period, linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChHwInterruptCoalescingGet with the same params.
            Expected: GT_OK and the same values.
        */

        st = cpssDxChHwInterruptCoalescingGet(dev, &enableGet, &periodGet,
                                                &linkChangeOverrideGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK ==st)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                 "get another enable than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(period, periodGet,
                 "get another period than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(linkChangeOverride, linkChangeOverrideGet,
                 "get another linkChangeOverride than was set: dev = %d", dev);
        }

        /* call with
                    enable = GT_TRUE;
                    period = HWINIT_PERIOD_HIGH_BOUND_CNS;
                    linkChangeOverride = GT_TRUE;
        */

        enable = GT_TRUE;
        period = HWINIT_PERIOD_HIGH_BOUND_CNS;
        linkChangeOverride = GT_TRUE;

        st = cpssDxChHwInterruptCoalescingSet(dev, enable, period, linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChHwInterruptCoalescingGet with the same params.
            Expected: GT_OK and the same values.
        */

        st = cpssDxChHwInterruptCoalescingGet(dev, &enableGet, &periodGet,
                                                &linkChangeOverrideGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK ==st)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                 "get another enable than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(period, periodGet,
                 "get another period than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(linkChangeOverride, linkChangeOverrideGet,
                 "get another linkChangeOverride than was set: dev = %d", dev);
        }

        /* call with
                    enable = GT_FALSE;
                    period = 0;
                    linkChangeOverride = GT_TRUE;
        */

        enable = GT_FALSE;
        period = 0;
        linkChangeOverride = GT_TRUE;

        st = cpssDxChHwInterruptCoalescingSet(dev, enable, period, linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call cpssDxChHwInterruptCoalescingGet with the same params.
            Expected: GT_OK and the same values.
        */

        st = cpssDxChHwInterruptCoalescingGet(dev, &enableGet, &periodGet,
                                                &linkChangeOverrideGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK ==st)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                 "get another enable than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(period, periodGet,
                 "get another period than was set: dev = %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(linkChangeOverride, linkChangeOverrideGet,
                 "get another linkChangeOverride than was set: dev = %d", dev);
        }
        /*
            1.3. Call with enable [GT_FALSE],
                           period [HWINIT_PERIOD_HIGH_BOUND_CNS + 1]
                           (ignored when enable [GT_FALSE]),
                           linkChangeOverride [GT_FALSE].
            Expected: GT_OK.
        */

        enable = GT_FALSE;
        period = HWINIT_PERIOD_HIGH_BOUND_CNS+1;
        linkChangeOverride = GT_FALSE;

        st = cpssDxChHwInterruptCoalescingSet(dev, enable, period, linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.4. Call with enable [GT_TRUE],
                           period [HWINIT_PERIOD_HIGH_BOUND_CNS + 1]
                           linkChangeOverride [GT_FALSE].
            Expected: GT_OUT_OF_RANGE.
        */

        enable = GT_TRUE;
        period = HWINIT_PERIOD_HIGH_BOUND_CNS+1;
        linkChangeOverride = GT_FALSE;

        st = cpssDxChHwInterruptCoalescingSet(dev, enable, period, linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwInterruptCoalescingSet(dev, enable, period, linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwInterruptCoalescingSet(dev, enable, period, linkChangeOverride);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwInterruptCoalescingGet
(
    IN GT_U8        devNum,
    OUT GT_BOOL     *enablePtr,
    OUT GT_U32      *periodPtr,
    OUT GT_BOOL     *linkChangeOverridePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwInterruptCoalescingGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-null pointers.
    Expected: GT_OK.
    1.2. Call with wrong enablePtr [NULL].
    Expected: GT_BAD_PTR.
    1.3. Call with wrong periodPtr [NULL].
    Expected: GT_BAD_PTR.
    1.4. Call with wrong linkChangeOverridePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL  enable = GT_TRUE;
    GT_U32   period = 0;
    GT_BOOL  linkChangeOverride = GT_TRUE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null pointers.
            Expected: GT_OK.
        */

        st = cpssDxChHwInterruptCoalescingGet(dev, &enable, &period,
                                                &linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with wrong enablePtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChHwInterruptCoalescingGet(dev, NULL, &period,
                                                &linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with wrong periodPtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChHwInterruptCoalescingGet(dev, &enable, NULL,
                                                &linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with wrong linkChangeOverridePtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChHwInterruptCoalescingGet(dev, &enable, &period, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwInterruptCoalescingGet(dev, &enable, &period,
                                                &linkChangeOverride);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwInterruptCoalescingGet(dev, &enable, &period,
                                            &linkChangeOverride);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpPhase1Init
(
    IN      CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   *ppPhase1ParamsPtr,
    OUT     CPSS_PP_DEVICE_TYPE                 *deviceTypePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpPhase1Init)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with wrong ppPhase1ParamsPtr [NULL].
    Expected: GT_BAD_PTR.
    1.2. Call with wrong deviceTypePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   ppPhase1Params;
    CPSS_PP_DEVICE_TYPE                 deviceType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&ppPhase1Params, sizeof(ppPhase1Params));

    /* avoid exception in LOG */
    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with wrong ppPhase1ParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChHwPpPhase1Init(NULL, &deviceType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.2. Call with wrong deviceTypePtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChHwPpPhase1Init(&ppPhase1Params, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpPhase1Init(&ppPhase1Params, &deviceType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpPhase2Init
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_PP_PHASE2_INIT_INFO_STC       *ppPhase2ParamsPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpPhase2Init)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with wrong ppPhase2ParamsPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC   ppPhase2Params;

    /* initialize mac entry */
    cpssOsMemSet(&ppPhase2Params,0,sizeof(ppPhase2Params));

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with wrong ppPhase2ParamsPtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChHwPpPhase2Init(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpPhase2Init(dev, &ppPhase2Params);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwPpPhase2Init(dev, &ppPhase2Params);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpSoftResetTrigger
(
    IN  GT_U8        devNum,
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpSoftResetTrigger)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct dev.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null pointers.
            Expected: GT_OK.
        */

#if 0
/* don't execute this test on HW. The API will reset PEX registers
   and device will be not manageable!!!!! */
        st = cpssDxChHwPpSoftResetTrigger(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
#endif
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpSoftResetTrigger(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwPpSoftResetTrigger(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpStartInit
(
    IN  GT_U8                       devNum,
    IN  CPSS_REG_VALUE_INFO_STC     *initDataListPtr,
    IN  GT_U32                      initDataListLen
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpStartInit)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct dev.
    Expected: GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_REG_VALUE_INFO_STC     initDataList;
    GT_U32                      initDataListLen = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    cpssOsBzero((GT_VOID*)&initDataList, sizeof(initDataList));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null pointers.
            Expected: GT_OK.
        */
        st = cpssDxChHwPpStartInit(dev, &initDataList, initDataListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);


        st = cpssDxChHwPpStartInit(dev, &initDataList, initDataListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with wrong initDataListPtr [NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChHwPpStartInit(dev, NULL, initDataListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpStartInit(dev, &initDataList, initDataListLen);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwPpStartInit(dev, &initDataList, initDataListLen);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwRxBufAlignmentGet
(
    IN  CPSS_PP_DEVICE_TYPE devType,
    OUT GT_U32              *byteAlignmentPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwRxBufAlignmentGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with byteAlignmentPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32     byteAlignment;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChHwRxBufAlignmentGet(dev, &byteAlignment);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with byteAlignmentPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChHwRxBufAlignmentGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwRxBufAlignmentGet(dev, &byteAlignment);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwRxBufAlignmentGet(dev, &byteAlignment);
    /* the devType is not used yet */
    /*UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);*/
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwRxDescSizeGet
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwRxDescSizeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with descSizePtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32     descSize;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChHwRxDescSizeGet(dev, &descSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with descSizePtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChHwRxDescSizeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwRxDescSizeGet(dev, &descSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwRxDescSizeGet(dev, &descSize);
    /* the devType is not used yet */
    /*UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);*/
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwTxDescSizeGet
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSizePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwTxDescSizeGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with descSizePtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32     descSize;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChHwTxDescSizeGet(dev, &descSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with descSizePtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChHwTxDescSizeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwTxDescSizeGet(dev, &descSize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwTxDescSizeGet(dev, &descSize);
    /* the devType is not used yet */
    /*UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);*/
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgHwDevNumSet
(
    IN GT_U8    devNum,
    IN GT_U8    hwDevNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgHwDevNumSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct hwDevNum [0 / 15 / 31].
    Expected: GT_OK.
    1.2. Call with wrong hwDevNum [BIT_5].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_HW_DEV_NUM   hwDevNum = 0;
    GT_HW_DEV_NUM   hwDevNumGet;
    GT_HW_DEV_NUM   hwDevNumForRestore;
    CPSS_DXCH_CFG_DEV_INFO_STC devInfo;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.0. Get the HW device number mode and
            Save device HW number for restore at test end
            Expected: GT_OK.
        */

        st = cpssDxChCfgDevInfoGet(dev,&devInfo);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChCfgHwDevNumGet(dev, &hwDevNumForRestore);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.1. Call with correct hwDevNum [0 / 15 / 31].
            Expected: single HW device number: GT_OK.
                      dual HW device number: 0 - GT_OK; 15/31 - GT_BAD_PARAM
        */

        hwDevNum = 0;
        st = cpssDxChCfgHwDevNumSet(dev, hwDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            Call cpssDxChCfgHwDevNumGet with the same params.
            Expected: GT_OK and the same values.
        */

        st = cpssDxChCfgHwDevNumGet(dev, &hwDevNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        if(GT_OK == st)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(hwDevNum, hwDevNumGet,
                 "get another hwDevNum than was set: dev = %d", dev);
        }


        hwDevNum = 15;
        st = cpssDxChCfgHwDevNumSet(dev, hwDevNum);

        if (HWINIT_GLOVAR(sysGenGlobalInfo.supportDualDeviceId) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /*
                Call cpssDxChCfgHwDevNumGet with the same params.
                Expected: GT_OK and the same values.
            */

            st = cpssDxChCfgHwDevNumGet(dev, &hwDevNumGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if(GT_OK == st)
            {
                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(hwDevNum, hwDevNumGet,
                     "get another hwDevNum than was set: dev = %d", dev);
            }

        }

        hwDevNum = 31;
        st = cpssDxChCfgHwDevNumSet(dev, hwDevNum);
        if (HWINIT_GLOVAR(sysGenGlobalInfo.supportDualDeviceId) == GT_TRUE)
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            /*
                Call cpssDxChCfgHwDevNumGet with the same params.
                Expected: GT_OK and the same values.
            */

            st = cpssDxChCfgHwDevNumGet(dev, &hwDevNumGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            if(GT_OK == st)
            {
                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(hwDevNum, hwDevNumGet,
                     "get another hwDevNum than was set: dev = %d", dev);
            }

        }

        /*
            1.2. Call with wrong hwDevNum [BIT_5].
            Expected: GT_OUT_OF_RANGE.
        */

        hwDevNum = UTF_CPSS_PP_MAX_HW_DEV_NUM_CNS(dev);

        st = cpssDxChCfgHwDevNumSet(dev, hwDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);

        /* 1.3 restore HW device number */
        st = cpssDxChCfgHwDevNumSet(dev, hwDevNumForRestore);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgHwDevNumSet(dev, hwDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgHwDevNumSet(dev, hwDevNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpImplementWaInit
(
    IN GT_U8    devNum,
    IN GT_U32                       numOfWa,
    IN CPSS_DXCH_IMPLEMENT_WA_ENT   waArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpImplementWaInit)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with correct params.
    Expected: GT_OK.
    1.2. Call with wrong waArrPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32                       numOfWa = 0;
    CPSS_DXCH_IMPLEMENT_WA_ENT   waArr[4] = {0};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with correct params.
            Expected: GT_OK or GT_BAD_STATE
        */

        numOfWa = 0;

        waArr[numOfWa++] = CPSS_DXCH_IMPLEMENT_WA_FDB_AU_FIFO_E;
        waArr[numOfWa++] = CPSS_DXCH_IMPLEMENT_WA_SDMA_PKTS_FROM_CPU_STACK_PADDING_E;
        waArr[numOfWa++] = CPSS_DXCH_IMPLEMENT_WA_RESTRICTED_ADDRESS_FILTERING_E;

        /* for some WA this API need to be called before relate library already initialized, */
        /* thus GT_BAD_STATE is ok. If this WA already initialized the function will return GT_OK */
        /* No need to check device type, all WA can be initialized for all devices */
        st = cpssDxChHwPpImplementWaInit(dev, numOfWa, waArr, 0);
        if (st != GT_OK && st != GT_BAD_STATE)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,"expected rc is GT_OK or GT_BAD_STATE in %d", dev);
        }


        /*
            1.2. Call with wrong waArrPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChHwPpImplementWaInit(dev, numOfWa, NULL, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpImplementWaInit(dev, numOfWa, waArr, 0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwPpImplementWaInit(dev, numOfWa, waArr, 0);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChCfgHwDevNumGet
(
    IN GT_U8    devNum,
    OUT GT_U8   *hwDevNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChCfgHwDevNumGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with hwDevNumPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS       st = GT_OK;
    GT_U8           dev;
    GT_HW_DEV_NUM   hwDevNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with hwDevNumPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChCfgHwDevNumGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChCfgHwDevNumGet(dev, &hwDevNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*        Check the following restricted ranges :
*
*           XCAT A1 ranges:
*
*           1.  0x02900000 - 0x0290FFFF   (Unit 5)
*               0x02C00000 - 0x02CFFFFF
*           2.  0x07E40000 - 0x07FC0000     (Unit 15)
*           3.  0x08800000 - 0x08801700     (Unit 17)
*               0x08801C00 - 0x08803F00
*           4.  0x09000000 - 0x092FFFF0     (Unit 18)
*               0x09380000 - 0x097FFFFF
*           5.  0x098007FF - 0x09800C00     (Unit 19)
*               0x098017FF - 0x09801C00
*               0x098027FF - 0x09802C00
*               0x098037FF - 0x09803C00
*               0x098047FF - 0x09804C00
*               0x098057FF - 0x09805C00
*               0x098067FF - 0x09806C00
*               0x098077FF - 0x09807C00
*               0x098087FF - 0x09808C00
*               0x098097FF - 0x09809C00
*               0x0980F7FF - 0x0980FC00
*           6.  0x0A807000 - 0x0A80F800 (Unit 21)
*
*
*           Lion B0 ranges (only for port groups 1,2,3):
*
*           1.  0x03800000 - 0x0387FFFC
*           2.  0x07000000 - 0x0707FFFC
*           3.  0x0A000000 - 0x0A7FFFFC
*           4.  0x10800000 - 0x10FFFFFC
*           only for port group 2 and 3
*           5.  0x11800000 - 0x11FFFFFC
*/
UTF_TEST_CASE_MAC(cpssDxChCfgHwCheckBadAddrWa)
{
/*
    1.1. Call with restricted addresses.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      singleRanges;
    GT_U32      i;
    GT_U32      numOfWords;
    GT_U32      data;
    GT_U32      maxNumOfWordsToWrite = 0x10000;
    GT_U32      *dataPtr;
    CPSS_DXCH_IMPLEMENT_WA_ENT          waFromCpss[1];
    HWINIT_ERRATA_BAD_ADDR_RANGE_STC    badAddrsArray[25];
    GT_U32      portGroupId;
    GT_U32      portGroupsBmp;
    GT_U32      allRanges;
    GT_STATUS   expectedStatus;
    GT_U32      lastValidAddress = 0xFFFFFFFF;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    dataPtr = (GT_U32 *)cpssOsMalloc(maxNumOfWordsToWrite * sizeof(GT_U32));
    if (dataPtr == NULL)
    {
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)dataPtr,
                                        "cpssOsMalloc: Memory allocation error.");
        return;
    }

    cpssOsMemSet(dataPtr,0,(maxNumOfWordsToWrite * sizeof(GT_U32)));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        {
            /* XCAT3 */

            /* All units single ranges */

            badAddrsArray[0].addrStart = 0x00092000;
            badAddrsArray[0].addrEnd = 0x00040100;

            badAddrsArray[1].addrStart = 0x02001000;
            badAddrsArray[1].addrEnd = 0x02002000;

            badAddrsArray[2].addrStart = 0x04310000;
            badAddrsArray[2].addrEnd = 0x04320000;

            badAddrsArray[3].addrStart = 0x06080280;
            badAddrsArray[3].addrEnd = 0x06080300;

            badAddrsArray[4].addrStart = 0x0E60010C;
            badAddrsArray[4].addrEnd = 0x0E601000;

            badAddrsArray[5].addrStart = 0x0F001000;
            badAddrsArray[5].addrEnd = 0x0F002000;

            badAddrsArray[6].addrStart = 0x11007D00;
            badAddrsArray[6].addrEnd = 0x11008000;

            badAddrsArray[7].addrStart = 0x121B2200;
            badAddrsArray[7].addrEnd = 0x121B3000;

            badAddrsArray[8].addrStart = 0x1300B020;
            badAddrsArray[8].addrEnd = 0x1300C000;

            badAddrsArray[9].addrStart = 0x15028400;
            badAddrsArray[9].addrEnd = 0x15028600;

            badAddrsArray[10].addrStart = 0x16034100;
            badAddrsArray[10].addrEnd = 0x16040000;

            badAddrsArray[11].addrStart = 0x17080400;
            badAddrsArray[11].addrEnd = 0x17081000;

            badAddrsArray[12].addrStart = 0x18080400;
            badAddrsArray[12].addrEnd = 0x18081000;

            badAddrsArray[13].addrStart = 0x1A0D3000;
            badAddrsArray[13].addrEnd = 0x1A0D5000;

            /* includes two ranges */
            badAddrsArray[14].addrStart = 0x1B000D80;
            badAddrsArray[14].addrEnd = 0x1B0DA100;

            /* includes two ranges */
            badAddrsArray[15].addrStart = 0x1C014100;
            badAddrsArray[15].addrEnd = 0x1C048100;

            singleRanges = 14;
            allRanges = 16;
            portGroupsBmp = 0x1;

       }

        /* enable Check Address WA */
        waFromCpss[0] = CPSS_DXCH_IMPLEMENT_WA_RESTRICTED_ADDRESS_FILTERING_E;
        st = cpssDxChHwPpImplementWaInit(dev,1,&waFromCpss[0],0);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if(PRV_CPSS_DXCH_PP_MAC(dev)->errata.
            info_PRV_CPSS_DXCH_XCAT_RESTRICTED_ADDRESS_FILTERING_WA_E.unitRangesPtr[0] == NULL)
        {
            /* The WA is not initialized */
            cpssOsFree(dataPtr);
            return;
        }

        /* 1.1. Go over all active port groups from the bitmap */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(dev, portGroupsBmp, portGroupId)
        {

            /* Read/Write one restricted address, write single ranges */
            for (i = 0; i < singleRanges; i++)
            {
                if ((portGroupId == 1) && (badAddrsArray[i].addrStart == 0x11800000))
                {
                    /* In Lion B0 in portGroup 1 unit 35 don't have
                       restricted addresses */
                    expectedStatus = GT_OK;
                }
                else
                {
                    expectedStatus = GT_BAD_PARAM;
                }

                st = prvCpssDrvHwPpPortGroupReadRegister(dev, portGroupId,
                                                         badAddrsArray[i].addrStart,
                                                         &data);
                UTF_VERIFY_EQUAL3_PARAM_MAC(expectedStatus, st, dev, portGroupId,
                                            badAddrsArray[i].addrStart);

                st = prvCpssDrvHwPpPortGroupWriteRegister(dev, portGroupId,
                                                          badAddrsArray[i].addrEnd,
                                                          0);
                if (lastValidAddress < badAddrsArray[i].addrEnd)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId,
                                            badAddrsArray[i].addrEnd);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(expectedStatus, st, dev, portGroupId,
                                            badAddrsArray[i].addrEnd);
                }

                /* calculate the number of words to write */
                numOfWords = ((badAddrsArray[i].addrEnd - badAddrsArray[i].addrStart) / 4) + 1;

                if (numOfWords > maxNumOfWordsToWrite)
                {
                    numOfWords = maxNumOfWordsToWrite;
                }

                st = prvCpssDrvHwPpPortGroupWriteRam(dev, portGroupId,
                                                     badAddrsArray[i].addrStart,
                                                     numOfWords, dataPtr);

                UTF_VERIFY_EQUAL3_PARAM_MAC(expectedStatus, st, dev, portGroupId,
                                            badAddrsArray[i].addrStart);
            }

            /* Write ranges that include two or more ranges or units */
            for (i = singleRanges; i < allRanges; i++)
            {
                /* calculate the number of words to write */
                numOfWords = ((badAddrsArray[i].addrEnd - badAddrsArray[i].addrStart) / 4) + 1;

                if (numOfWords > maxNumOfWordsToWrite)
                {
                    numOfWords = maxNumOfWordsToWrite;
                }

                st = prvCpssDrvHwPpPortGroupWriteRam(dev, portGroupId,
                                                     badAddrsArray[i].addrStart,
                                                     numOfWords, dataPtr);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
            }
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

        /* Disable Address Checking WA */
        st = prvCpssDrvAddrCheckWaBind(dev,NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    cpssOsFree(dataPtr);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpInitStageGet
(
    IN  GT_U8                      dev,
    OUT CPSS_HW_PP_INIT_STAGE_ENT  *initStagePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpInitStageGet)
{
/*
    ITERATE_DEVICES(All DxCh Devices)
    1.1. Call with not null initStagePtr.
    Expected: GT_OK.
    1.2. Call api with wrong initStagePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                      dev;
    CPSS_HW_PP_INIT_STAGE_ENT  initStage;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with not null initStagePtr.
            Expected: GT_OK.
        */
        st = cpssDxChHwPpInitStageGet(dev, &initStage);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong initStagePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChHwPpInitStageGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, initStagePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpInitStageGet(dev, &initStage);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwPpInitStageGet(dev, &initStage);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpSoftResetSkipParamGet
(
    IN  GT_U8                           dev,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    OUT GT_BOOL                         *skipEnablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpSoftResetSkipParamGet)
{
/*
    ITERATE_DEVICES(All DxCh Devices)
    1.1. Call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E],
    Expected: GT_OK.
    1.2. Call api with wrong skipType [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.3. Call api with wrong skipEnablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                           dev;
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;
    GT_BOOL                         skipEnable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E],
            Expected: GT_OK.
        */

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;

        st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E;

        st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E;

        st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E;

        st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnable);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E;

        st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call api with wrong skipType [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChHwPpSoftResetSkipParamGet
                            (dev, skipType, &skipEnable),
                            skipType);

        /*
            1.3. Call api with wrong skipEnablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, skipEnablePtr = NULL", dev);
    }

    /* restore correct values */
    skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpSoftResetSkipParamSet
(
    IN  GT_U8                          dev,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT skipType,
    IN  GT_BOOL                        skipEnable
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpSoftResetSkipParamSet)
{
/*
    ITERATE_DEVICES(All DxCh Devices)
    1.1. Call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E],
                   skipEnable[GT_TRUE / GT_FALSE / GT_TRUE / GT_TRUE / GT_TRUE],
    Expected: GT_OK.
    1.2. Call cpssDxChHwPpSoftResetSkipParamGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong skipType [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                          dev;
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;
    GT_BOOL                        skipEnable = GT_FALSE;
    GT_BOOL                        skipEnableGet = GT_FALSE;

    /* there is no DFX in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E],
                           skipEnable[GT_TRUE / GT_FALSE / GT_TRUE / GT_TRUE / GT_TRUE],
            Expected: GT_OK.
        */

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E],
                     skipEnable[GT_TRUE] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;
        skipEnable = GT_TRUE;

        st = cpssDxChHwPpSoftResetSkipParamSet(dev, skipType, skipEnable);
        if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_SUPPORTED, st, dev);
        }
        else
        {
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /*
            1.2. Call cpssDxChHwPpSoftResetSkipParamGet
                   with the same parameters.
            Expected: GT_OK and the same values than was set.
        */
        st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChHwPpSoftResetSkipParamGet: %d ", dev);

        if (PRV_CPSS_SIP_6_CHECK_MAC(dev) == GT_FALSE)
        {
            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(skipEnable, skipEnableGet,
                           "got another skipEnable then was set: %d", dev);
        }

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E],
                     skipEnable[GT_FALSE] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E;
        skipEnable = GT_FALSE;

        st = cpssDxChHwPpSoftResetSkipParamSet(dev, skipType, skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.2. */
        st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChHwPpSoftResetSkipParamGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(skipEnable, skipEnableGet,
                       "got another skipEnable then was set: %d", dev);

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E],
                     skipEnable[GT_TRUE] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E;
        skipEnable = GT_TRUE;
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            skipEnable = GT_FALSE; /* GT_TRUE not supported */
        }

        st = cpssDxChHwPpSoftResetSkipParamSet(dev, skipType, skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call get func again, 1.2. */
        st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                  "cpssDxChHwPpSoftResetSkipParamGet: %d ", dev);

        /* Verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(skipEnable, skipEnableGet,
                       "got another skipEnable then was set: %d", dev);

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E],
                     skipEnable[GT_TRUE] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E;
        skipEnable = GT_TRUE;

        st = cpssDxChHwPpSoftResetSkipParamSet(dev, skipType, skipEnable);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.2. */
            st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChHwPpSoftResetSkipParamGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(skipEnable, skipEnableGet,
                           "got another skipEnable then was set: %d", dev);


        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E],
                     skipEnable[GT_TRUE] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E;
        skipEnable = GT_TRUE;
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            skipEnable = GT_FALSE; /* GT_TRUE not supported */
        }

        st = cpssDxChHwPpSoftResetSkipParamSet(dev, skipType, skipEnable);

            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /* call get func again, 1.2. */
            st = cpssDxChHwPpSoftResetSkipParamGet(dev, skipType, &skipEnableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChHwPpSoftResetSkipParamGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(skipEnable, skipEnableGet,
                           "got another skipEnable then was set: %d", dev);



        /*
            1.3. Call api with wrong skipType [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChHwPpSoftResetSkipParamSet
                            (dev, skipType, skipEnable),
                            skipType);
    }

    /* restore correct values */
    skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;
    skipEnable = GT_TRUE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpSoftResetSkipParamSet(dev, skipType, skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwPpSoftResetSkipParamSet(dev, skipType, skipEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChTraceHwAccessInfoTest
(
    IN GT_U8    devNum
)
*/
static void prv_cpssDxChHwAccessInfoStoreTest(void)
{
    GT_STATUS                           st = GT_OK;
    GT_U8                               dev;
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT   accessType;
    CPSS_ENABLER_TRACE_OUTPUT_MODE_ENT      outPutMode;
    GT_U32                              portGroupId;
    GT_U32                              portGroupsBmp;
    GT_U32                              regAddrArr[MAX_NUM_OF_REG_ADDRS_CNS];
    GT_U32                              dataArr[MAX_NUM_OF_REG_ADDRS_CNS];
    GT_U32                              maskArr[MAX_NUM_OF_REG_ADDRS_CNS];
    GT_BOOL                             isrContextArr[MAX_NUM_OF_REG_ADDRS_CNS];
    CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT addrSpaceArr[MAX_NUM_OF_REG_ADDRS_CNS];
    GT_U32                              numOfHwAccesses;
    GT_U32                              i,jj;
    GT_U32                              entryLength;
    GT_U32                              mask;
    GT_U32                              fieldOffset, fieldLength, fieldData;
    GT_U32                              readVal;
    GT_U32                              indexInDb;  /* real index in trace db */
    GT_BOOL                             dbIsCorrupted = GT_TRUE;
    GT_U32                              checkPci_e; /* do access PCI_e registers */

    /* get to CPU to take care of events from previous tests */
    cpssOsTimerWkAfter(500);

    CPSS_TBD_BOOKMARK_XCAT3
    /* test on XCAT3 board is skipped, because of segmentation fault in LSP,
       during MSYS access on board with internal CPU - should be removed after
       LSP will be fixed */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_XCAT3_E | UTF_AC5_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        checkPci_e = PRV_CPSS_DRV_HW_IF_PCI_COMPATIBLE_MAC(dev);

        if (checkPci_e && PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT2_E)
        {
            /* it's BC2, Caelum and Cetus cases */
            CPSS_HW_INFO_STC *hwInfoPtr;
            hwInfoPtr = cpssDrvHwPpHwInfoStcPtrGet(dev, 0);
            if (hwInfoPtr && (hwInfoPtr->busType == CPSS_HW_INFO_BUS_TYPE_MBUS_E))
            {
                /* PCI_e interface may be powered down and access
                   to it's registers may stuck CPU */
                checkPci_e = 0;
            }
        }
        /*
            1.1. Configure output mode to be "store to db"
            Expected: GT_OK.
        */
        outPutMode = CPSS_ENABLER_TRACE_OUTPUT_MODE_DB_E;
        st = prvWrAppTraceHwAccessOutputModeSet(outPutMode);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);


        /* fill in register addresses */
        i=0;

        /* check bus type, PEX shouldn't be tested for SMI and TWSI  */
        if (checkPci_e)
        {
            if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
#ifdef ASIC_SIMULATION
                regAddrArr[i] = 0;
#else
                regAddrArr[i] = 0x41910;
#endif /*ASIC_SIMULATION*/
                    i++;
            }
            else
            {
                /* set PEX address */
                switch (PRV_CPSS_PP_MAC(dev)->devFamily)
                {
                    case PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_CASES_MAC:
                        regAddrArr[i] = 0x41910;
                        i++;
                        break;
                    case CPSS_PP_FAMILY_DXCH_LION2_E:
                        regAddrArr[i] = 0x71910;
                        i++;
                        break;
                    default:
                        regAddrArr[i] = 0x0;
                        i++;
                        break;
                }
           }
        }

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            regAddrArr[i] = PRV_DXCH_REG1_UNIT_L2I_MAC(dev).
                    MACBasedQoSTable.MACQoSTableEntry[0];
        }
        else
        {
            regAddrArr[i] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->bridgeRegs.qosProfileRegs.macQosTable;
        }
        i++;

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            regAddrArr[i] = PRV_DXCH_REG1_UNIT_TTI_MAC(dev).protocolMatch.protocolsConfig[0];
        }
        else
        {
            regAddrArr[i] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->bridgeRegs.vlanRegs.protoTypeBased;
        }
        i++;

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            regAddrArr[i] = PRV_DXCH_REG1_UNIT_TTI_MAC(dev).IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg0;
        }
        else
        {
            regAddrArr[i] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->bridgeRegs.ipv6McastSolicitNodeMaskBase;
        }
        i++;

        /* Reset and Init Controller (DFX) access */
        if( (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE) &&
            (GT_FALSE == prvUtfIsGmCompilation()) )
        {
            regAddrArr[i] =
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(dev)->
                    DFXServerUnits.DFXServerRegs.serverInterruptSummaryMask;
            i++;
            regAddrArr[i] =
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(dev)->
                    DFXServerUnits.DFXServerRegs.serverInterruptMask;
            i++;
        }

        /* would be written/read like an entry       */
        /* used registers with consecutive addresses */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            regAddrArr[i] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->
                L2I.MACBasedQoSTable.MACQoSTableEntry[0];
            i++;
            regAddrArr[i] = regAddrArr[i-1] + 4;
            i++;
        }
        else
        {
            regAddrArr[i] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->bridgeRegs.qosProfileRegs.dscp2DscpMapTable;
            i++;
            regAddrArr[i] = regAddrArr[i-1] + 4;
            i++;
        }
        /* would be written/read like an entry */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /* next registers tested on 2 consecutive addresses */
            regAddrArr[i] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->
                EQ.applicationSpecificCPUCodes.IPProtocolCPUCodeEntry[0];
            i++;
            regAddrArr[i] = regAddrArr[i-1] + 4;
            i++;
        }
        else
        {
            regAddrArr[i] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->sFlowRegs.sFlowControl;
            i++;
            regAddrArr[i] = regAddrArr[i-1] + 4;
            i++;
        }

        if(PRV_CPSS_DXCH_PP_MAC(dev)->hwInfo.dedicatedCpuMac.isNotSupported == GT_TRUE)
        {
            /* no dedicated MAC for CPU ...so use some other register */
            regAddrArr[i] = PRV_DXCH_REG1_UNIT_EGF_QAG_MAC(dev).
                distributor.distributorGeneralConfigs;
            i++;
        }
        else
        {
            regAddrArr[i] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->macRegs.cpuPortRegs.macStatus;
            i++;
            regAddrArr[i] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->macRegs.cpuPortRegs.autoNegCtrl;
            i++;
        }

        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            regAddrArr[i] = PRV_DXCH_REG1_UNIT_L2I_MAC(dev).
                bridgeEngineConfig.bridgeGlobalConfig0;
        }
        else
        {
            regAddrArr[i] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->bridgeRegs.portControl[0];
        }
        i++;

        if(PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            portGroupsBmp = BIT_0;
            /* there maybe more bits in portGroupsBmp but only one portGroupId is supported
               by driver */
        }
        else
        {
            portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
        }

        /* Go over all active port groups from the bitmap */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(dev, portGroupsBmp, portGroupId)
        {
            /*
                1.2. Enable hw info tracing for write access.
                Expected: GT_OK.
            */
            accessType = CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_WRITE_E;
            st = prvWrAppTraceHwAccessEnable(dev, accessType, GT_TRUE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, accessType);

            /*
                1.3. Call write driver APIs,
                retrieve info from access DB and compare
            */

            jj=0;

            /* check bus type, this test shouldn't be done for SMI and TWSI */
            if (checkPci_e)
            {
                /* WriteInternalPciReg - interrupt mask.
                  do not write reserved and HitDfltWinErr, TxRamParErr bits
                  to avoid redundant interrupts for the test */
                dataArr[jj] = 0x4B;
                isrContextArr[jj] = GT_FALSE;
                addrSpaceArr[jj] = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PCI_PEX_E;
                st = prvCpssDrvHwPpPortGroupWriteInternalPciReg(dev, portGroupId,
                                                                regAddrArr[jj],
                                                                dataArr[jj]);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
                jj++;
            }

            /* WriteRegister */
            dataArr[jj] = 0x55555555;
            isrContextArr[jj] = GT_FALSE;
            addrSpaceArr[jj] = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            st = prvCpssDrvHwPpPortGroupWriteRegister(dev, portGroupId,
                                                      regAddrArr[jj],
                                                      dataArr[jj]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            jj++;

            /* WriteRegBitMask */
            dataArr[jj] = 0x55AA55AA;
            mask = 0xF00FF00F;
            isrContextArr[jj] = GT_FALSE;
            addrSpaceArr[jj] = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            /* read before write */
            st = prvCpssDrvHwPpPortGroupReadRegister(dev, portGroupId,
                                                     regAddrArr[jj],
                                                     &readVal);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            st = prvCpssDrvHwPpPortGroupWriteRegBitMask(dev, portGroupId,
                                                        regAddrArr[jj],
                                                        mask, dataArr[jj]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            /* update the data with the mask, this will be the expected data */
            readVal &= (~mask);                  /* Turn the field off.                        */
            readVal |= (dataArr[jj] & mask);     /* Insert the new value of field in its place.*/
            dataArr[jj] = readVal;
            jj++;

            /* SetRegField */
            st = prvCpssDrvHwPpPortGroupReadRegister(dev, portGroupId,
                                                     regAddrArr[jj],
                                                     &dataArr[jj]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            fieldOffset = 0xF;
            fieldLength = 4;
            fieldData = 0xC;
            isrContextArr[jj] = GT_FALSE;
            addrSpaceArr[jj] = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            st = prvCpssDrvHwPpPortGroupSetRegField(dev, portGroupId,
                                                    regAddrArr[jj],
                                                    fieldOffset,
                                                    fieldLength, fieldData);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);

            /* update the data with the mask, this will be the expected data */
            CALC_MASK_MAC(fieldLength, fieldOffset, mask);
            dataArr[jj] &= ~mask;   /* turn the field off */
            /* insert the new value of field in its place */
            dataArr[jj] |= ((fieldData << fieldOffset) & mask);
            jj++;

/* Reset and Init Controller (DFX) access */
             if( (PRV_CPSS_SIP_5_CHECK_MAC(dev) == GT_TRUE) &&
                 (GT_FALSE == prvUtfIsGmCompilation()) )
            {
                /* WriteRegister */
                dataArr[jj] = 0x5A5A5A5A;
                isrContextArr[jj] = GT_FALSE;
                addrSpaceArr[jj] =
                        CPSS_TRACE_HW_ACCESS_ADDR_SPACE_RESET_AND_INIT_CTRL_E;

                st = prvCpssDrvHwPpResetAndInitControllerWriteReg(dev,
                                                             regAddrArr[jj],
                                                             dataArr[jj]);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);

                jj++;

                /* SetRegField */
                st = prvCpssDrvHwPpResetAndInitControllerReadReg(dev,
                                                             regAddrArr[jj],
                                                             &dataArr[jj]);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);

                fieldOffset = 0x7;
                fieldLength = 16;
                fieldData = 0xA5A5;
                isrContextArr[jj] = GT_FALSE;
                addrSpaceArr[jj] =
                        CPSS_TRACE_HW_ACCESS_ADDR_SPACE_RESET_AND_INIT_CTRL_E;
                st = prvCpssDrvHwPpResetAndInitControllerSetRegField(dev,
                                                              regAddrArr[jj],
                                                              fieldOffset,
                                                              fieldLength,
                                                              fieldData);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);

                /* update the data with the mask, this will be the expected data */
                CALC_MASK_MAC(fieldLength, fieldOffset, mask);
                dataArr[jj] &= ~mask;   /* turn the field off */
                /* insert the new value of field in its place */
                dataArr[jj] |= ((fieldData << fieldOffset) & mask);

                jj++;
            }

            /* WriteRam */
            dataArr[jj] = 0xA5A5A5A5;
            dataArr[jj+1] = 0xB0B0B0B0;
            isrContextArr[jj] = isrContextArr[jj+1] = GT_FALSE;
            addrSpaceArr[jj] = addrSpaceArr[jj+1] =
                                           CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            entryLength = 2;
            st = prvCpssDrvHwPpPortGroupWriteRam(dev, portGroupId,
                                                 regAddrArr[jj],
                                                 entryLength,
                                                 &dataArr[jj]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            jj = jj+entryLength;

            /* WriteRamInReverse */
            dataArr[jj] = 0xFFFF0000;
            dataArr[jj+1] = 0x5A5A5A5A;
            isrContextArr[jj] = isrContextArr[jj+1] = GT_FALSE;
            addrSpaceArr[jj] = addrSpaceArr[jj+1] =
                                           CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            entryLength = 2;
            st = cpssDrvPpHwRamInReverseWrite(dev, portGroupId,
                                                          regAddrArr[jj],
                                                          entryLength,
                                                          &dataArr[jj]);
            if (st != GT_NOT_SUPPORTED)
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
                jj = jj+entryLength;
            }

            /* IsrWrite */
            dataArr[jj] = 0xF0F0F0F0;
            isrContextArr[jj] = GT_TRUE;
            addrSpaceArr[jj] = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            st = prvCpssDrvHwPpPortGroupIsrWrite(dev, portGroupId,
                                                 regAddrArr[jj],
                                                 dataArr[jj]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            jj++;

            /* number of write hw accesses */
            numOfHwAccesses = jj;

            /* real index in hw trace db */
            indexInDb = 0;
            /* compare between the written data and appDemo hw access write db */
            for (jj = 0; jj < numOfHwAccesses; indexInDb++)
            {
                st = prvWrAppTraceHwAccessInfoCompare(dev, accessType, indexInDb,
                                                     portGroupId,
                                                     isrContextArr[jj],
                                                     addrSpaceArr[jj],
                                                     regAddrArr[jj],
                                                     0xFFFFFFFF, /* mask */
                                                     dataArr[jj]);
                if (st == GT_BAD_STATE)
                {
                    /* there was an unexpected ISR access, skip it */
                    continue;
                }
                UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, dev, accessType, portGroupId,
                                            jj, regAddrArr[jj], dataArr[jj]);
                jj++;
            }

            /*
                1.4. Disable hw info tracing for write access.
                Expected: GT_OK.
            */
            st = prvWrAppTraceHwAccessEnable(dev, accessType, GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, accessType);

             /*
                1.5 . Check if data base is corrupted
                Expected: GT_FALSE.
            */

            st = prvWrAppTraceHwAccessDbIsCorrupted(&dbIsCorrupted);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, dbIsCorrupted);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

             /*
                1.6. Clear data base .
                Expected: GT_OK.
            */

            st = prvWrAppTraceHwAccessClearDb();
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /*
                1.7. Enable hw info tracing for read access.
                Expected: GT_OK.
            */
            accessType = CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_READ_E;
            st = prvWrAppTraceHwAccessEnable(dev, accessType, GT_TRUE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, accessType);

            /*
                1.8. Call read driver APIs,
                retrieve info from access DB and compare
            */

            jj=0;

            /* check bus type, this test shouldn't be done for SMI and TWSI */
            if (checkPci_e)
            {
                /* ReadInternalPciReg */
                isrContextArr[jj] = GT_FALSE;
                addrSpaceArr[jj] = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PCI_PEX_E;
                maskArr[jj] = 0xFFFFFFFF;
                st = prvCpssDrvHwPpPortGroupReadInternalPciReg(dev, portGroupId,
                                                               regAddrArr[jj],
                                                               &dataArr[jj]);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
                jj++;
            }

            /* ReadRegister */
            isrContextArr[jj] = GT_FALSE;
            addrSpaceArr[jj] = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            maskArr[jj] = 0xFFFFFFFF;
            st = prvCpssDrvHwPpPortGroupReadRegister(dev, portGroupId,
                                                     regAddrArr[jj],
                                                     &dataArr[jj]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            jj++;

            /* ReadRegBitMask */
            dataArr[jj] = 0xFFFFFFFF;
            maskArr[jj] = 0xF0F0F0F0;
            isrContextArr[jj] = GT_FALSE;
            addrSpaceArr[jj] = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            st = prvCpssDrvHwPpPortGroupReadRegBitMask(dev, portGroupId,
                                                       regAddrArr[jj],
                                                       maskArr[jj],
                                                       &dataArr[jj]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            jj++;

            /* GetRegField */
            fieldOffset = 0xF;
            fieldLength = 4;
            fieldData = 0xC;
            isrContextArr[jj] = GT_FALSE;
            addrSpaceArr[jj] = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            CALC_MASK_MAC(fieldLength, fieldOffset, mask);
            maskArr[jj] = mask;
            st = prvCpssDrvHwPpPortGroupGetRegField(dev, portGroupId,
                                                    regAddrArr[jj],
                                                    fieldOffset,
                                                    fieldLength, &fieldData);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            /* move the data to its real offset */
            /* insert the new value of field in its place */
            dataArr[jj] = (fieldData << fieldOffset);
            jj++;

            /* Reset and Init Controller (DFX) access */
            if( (PRV_CPSS_SIP_5_CHECK_MAC(dev) == GT_TRUE) &&
                (GT_FALSE == prvUtfIsGmCompilation()) )
            {
                /* ReadRegister */
                isrContextArr[jj] = GT_FALSE;
                addrSpaceArr[jj] =
                        CPSS_TRACE_HW_ACCESS_ADDR_SPACE_RESET_AND_INIT_CTRL_E;
                maskArr[jj] = 0xFFFFFFFF;
                st = prvCpssDrvHwPpResetAndInitControllerReadReg(dev,
                                                             regAddrArr[jj],
                                                             &dataArr[jj]);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);

                jj++;

                /* GetRegField */
                fieldOffset = 0x7;
                fieldLength = 16;
                isrContextArr[jj] = GT_FALSE;
                addrSpaceArr[jj] =
                        CPSS_TRACE_HW_ACCESS_ADDR_SPACE_RESET_AND_INIT_CTRL_E;
                CALC_MASK_MAC(fieldLength, fieldOffset, mask);
                maskArr[jj] = mask;
                st = prvCpssDrvHwPpResetAndInitControllerGetRegField(dev,
                                                              regAddrArr[jj],
                                                              fieldOffset,
                                                              fieldLength,
                                                              &fieldData);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);

                /* move the data to its real offset */
                /* insert the new value of field in its place */
                dataArr[jj] = (fieldData << fieldOffset);

                jj++;
            }

            /* ReadRam */
            dataArr[jj] = 0xA5A5A5A5;
            dataArr[jj+1] = 0xB0B0B0B0;
            isrContextArr[jj] = isrContextArr[jj+1] = GT_FALSE;
            addrSpaceArr[jj] = addrSpaceArr[jj+1] =
                                           CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            maskArr[jj] = 0xFFFFFFFF;
            maskArr[jj+1] = 0xFFFFFFFF;
            entryLength = 2;
            st = prvCpssDrvHwPpPortGroupReadRam(dev, portGroupId,
                                                regAddrArr[jj],
                                                entryLength,
                                                &dataArr[jj]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            jj = jj+2;

            /* IsrRead */
            isrContextArr[jj] = GT_TRUE;
            addrSpaceArr[jj] = CPSS_TRACE_HW_ACCESS_ADDR_SPACE_PP_E;
            maskArr[jj] = 0xFFFFFFFF;
            st = prvCpssDrvHwPpPortGroupIsrRead(dev, portGroupId,
                                                regAddrArr[jj],
                                                &dataArr[jj]);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, regAddrArr[jj]);
            jj++;

            /* number of read hw accesses */
            numOfHwAccesses = jj;

            /* real index in hw trace db */
            indexInDb = 0;

            /* compare between the written data and appDemo hw access write db */
            for (jj = 0; jj < numOfHwAccesses; indexInDb++)
            {
                st = prvWrAppTraceHwAccessInfoCompare(dev, accessType, indexInDb,
                                                     portGroupId,
                                                     isrContextArr[jj],
                                                     addrSpaceArr[jj],
                                                     regAddrArr[jj],
                                                     maskArr[jj],
                                                     dataArr[jj]);
                if (st == GT_BAD_STATE)
                {
                    /* there was an unexpected ISR access, skip it */
                    continue;
                }
                UTF_VERIFY_EQUAL6_PARAM_MAC(GT_OK, st, dev, accessType, portGroupId,
                                            jj, regAddrArr[jj], dataArr[jj]);
                jj++;
            }
            /*
                1.9. Disable hw info tracing for read access.
                Expected: GT_OK.
            */
            st = prvWrAppTraceHwAccessEnable(dev, accessType, GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, accessType);

              /*
                1.10  . Check if data base is corrupted
                Expected: GT_FALSE.
            */

            st = prvWrAppTraceHwAccessDbIsCorrupted(&dbIsCorrupted);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_FALSE, dbIsCorrupted);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

             /*
                1.11. Clear data base .
                Expected: GT_OK.
            */

            st = prvWrAppTraceHwAccessClearDb();
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(dev, portGroupsBmp, portGroupId)

        /*
            1.12 . Configure output mode to be "direct print"
            Expected: GT_OK.
        */
        outPutMode = CPSS_ENABLER_TRACE_OUTPUT_MODE_DIRECT_E;
        st = prvWrAppTraceHwAccessOutputModeSet(outPutMode);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }
}


UTF_TEST_CASE_MAC(cpssDxChHwAccessInfoStoreTest)
{
    GT_STATUS st;
    GT_U8                               dev;

    /* execute test */
    prv_cpssDxChHwAccessInfoStoreTest();

    /* clean after the test */
     /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvWrAppTraceHwAccessEnable(dev, CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_BOTH_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
        st = prvWrAppTraceHwAccessOutputModeSet(CPSS_ENABLER_TRACE_OUTPUT_MODE_DIRECT_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }
}

UTF_TEST_CASE_MAC(cpssDxChCfgHwCheckBadEndAddrWa)
{
/*
    1.1. Call with restricted addresses.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      data;
    GT_U32      portGroupId;
    GT_U32      portGroupsBmp;
    GT_U32      badAddrs;
    CPSS_DXCH_IMPLEMENT_WA_ENT          waFromCpss[1];


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if ((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(dev)->devFamily)||
            (PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
             badAddrs = 0x118F0E04;

        }
        else if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev))
        {
            badAddrs = 0x1D00C100;
        }
        else /* XCAT, XCAT2 */
        {
            badAddrs = 0xF80C100;
        }

        /* enable Check Address WA */
        waFromCpss[0] = CPSS_DXCH_IMPLEMENT_WA_RESTRICTED_ADDRESS_FILTERING_E;
        st = cpssDxChHwPpImplementWaInit(dev,1,&waFromCpss[0],0);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if(PRV_CPSS_DXCH_PP_MAC(dev)->errata.
            info_PRV_CPSS_DXCH_XCAT_RESTRICTED_ADDRESS_FILTERING_WA_E.unitRangesPtr[0] == NULL)
        {
            /* The WA is not initialized */
            return;
        }

        portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

        /* 1.1. Go over all active port groups from the bitmap */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(dev, portGroupsBmp, portGroupId)
        {

            /* Read/Write one restricted address, write single ranges */
            st = prvCpssDrvHwPpPortGroupReadRegister(dev, portGroupId,
                                                         badAddrs,
                                                    &data);
           UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId,
                                       badAddrs);

           st = prvCpssDrvHwPpPortGroupWriteRegister(dev, portGroupId,
                                                     badAddrs,
                                                     0);
           UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupId,
                                       badAddrs);
        }


        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)


        /* Disable Address Checking WA */
        st = prvCpssDrvAddrCheckWaBind(dev,NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChGpioPhyConfigSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          directionBitmap
)
*/
UTF_TEST_CASE_MAC(cpssDxChGpioPhyConfigSet)
{
/*
    ITERATE_DEVICES (AC3X)
    ITERATE_DEVICES_PORTS (AC3X)

     1.1. Call function with good parameters
    Expected: GT_OK on extended cascade ports, GT_BAD_PARAM on other ports.
    1.2. Call cpssDxChGpioPhyConfigGet with the same parameters
    Expected: GT_OK and the same values.
    1.3. Call with wrong parameters.
    Expected: GT_BAD_PARAM
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    PRV_CPSS_DXCH_PORT_INFO_STC        *portPtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC    *portsMapInfoShadowPtr;
    GT_PHYSICAL_PORT_NUM                portNum    = 0;
    GT_BOOL                             isExtendedCascadePort;
    GT_BOOL                             iter;                   /* Just an iterator */
    GT_U32                              directionBitmapSave;
    GT_U32                              modeBitmapSave;
    GT_U32                              directionBitmap;
    GT_U32                              directionBitmapGet;
    GT_U32                              modeBitmapGet;

    /* there is no GPIO in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->port;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            portsMapInfoShadowPtr = &portPtr->portsMapInfoShadowArr[portNum];
            isExtendedCascadePort = portsMapInfoShadowPtr->valid &&
                                    portsMapInfoShadowPtr->portMap.isExtendedCascadePort;
            st = cpssDxChGpioPhyConfigGet(dev, portNum, &modeBitmapSave, &directionBitmapSave);
            UTF_VERIFY_EQUAL0_PARAM_MAC(isExtendedCascadePort ? GT_OK : GT_BAD_PARAM, st);

            for (iter = 0; iter < 2; iter++)
            {
                directionBitmap = (0x0f << ((2*portNum+iter) % 16)) & modeBitmapSave;

                st = cpssDxChGpioPhyConfigSet(dev, portNum, directionBitmap);
                if (isExtendedCascadePort)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, portNum,
                                            directionBitmap);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum,
                                            directionBitmap);
                }
                if (st == GT_OK)
                {

                    /*
                        1.2. Call cpssDxChGpioPhyConfigGet
                        Expected: GT_OK and the same state.
                    */
                    st = cpssDxChGpioPhyConfigGet(dev, portNum, &modeBitmapGet, &directionBitmapGet);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, st,
                        "cpssDxChLedPhyControlGlobalGet: %d %d %d %d %d %d %d",
                                    dev, portNum,
                                        directionBitmap);
                    UTF_VERIFY_EQUAL2_STRING_MAC(directionBitmap, directionBitmapGet,
                        "get different data than was set: %d/%d", dev, portNum);
                }
            }

            /*
                1.3. Call with wrong directionBitmap [BIT_18].
                Expected: GT_BAD_PARAM
            */
            directionBitmap = BIT_18;
            st = cpssDxChGpioPhyConfigSet(dev, portNum, directionBitmap);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum,
                                directionBitmap);

            directionBitmap = 0;

            st = cpssDxChGpioPhyConfigSet(dev, portNum, directionBitmapSave);
            UTF_VERIFY_EQUAL0_PARAM_MAC(isExtendedCascadePort ? GT_OK : GT_BAD_PARAM, st);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        directionBitmap = 0;

        /* 2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 2.1. Call function for each non-active port */
            st = cpssDxChGpioPhyConfigSet(dev, portNum, directionBitmap);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 2.2. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;

        st = cpssDxChGpioPhyConfigSet(dev, portNum, directionBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 2.3. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChGpioPhyConfigSet(dev, portNum, directionBitmap);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 3. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    portNum = 0;
    directionBitmap = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChGpioPhyConfigSet(dev, portNum, directionBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.1 Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChGpioPhyConfigSet(dev, portNum, directionBitmap);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChGpioPhyConfigGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                         *modeBitmapPtr,
    OUT GT_U32                         *directionBitmapPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChGpioPhyConfigGet)
{
/*
    ITERATE_DEVICES (AC3X)
    ITERATE_DEVICES_PORTS (AC3X)

     1.1. Call function
    Expected: GT_OK on extended cascade ports, GT_BAD_PARAM on other ports.
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    PRV_CPSS_DXCH_PORT_INFO_STC        *portPtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC    *portsMapInfoShadowPtr;
    GT_PHYSICAL_PORT_NUM                portNum    = 0;
    GT_BOOL                             isExtendedCascadePort;
    GT_U32                              directionBitmapGet;
    GT_U32                              modeBitmapGet;

    /* there is no GPIO in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->port;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            portsMapInfoShadowPtr = &portPtr->portsMapInfoShadowArr[portNum];
            isExtendedCascadePort = portsMapInfoShadowPtr->valid &&
                                    portsMapInfoShadowPtr->portMap.isExtendedCascadePort;

            st = cpssDxChGpioPhyConfigGet(dev, portNum, &modeBitmapGet, &directionBitmapGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(isExtendedCascadePort ? GT_OK : GT_BAD_PARAM, st, dev, portNum);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChGpioPhyConfigGet(dev, portNum, &modeBitmapGet, &directionBitmapGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChGpioPhyConfigGet(dev, portNum, &modeBitmapGet, &directionBitmapGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChGpioPhyConfigGet(dev, portNum, &modeBitmapGet, &directionBitmapGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChGpioPhyConfigGet(dev, portNum, &modeBitmapGet, &directionBitmapGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, portNum);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChGpioPhyConfigGet(dev, portNum, &modeBitmapGet, &directionBitmapGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChGpioPhyDataRead
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                         *dataBitmapPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChGpioPhyDataRead)
{
/*
    ITERATE_DEVICES (AC3X)
    ITERATE_DEVICES_PORTS (AC3X)

     1.1. Call function
    Expected: GT_OK on extended cascade ports, GT_BAD_PARAM on other ports.
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    PRV_CPSS_DXCH_PORT_INFO_STC        *portPtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC    *portsMapInfoShadowPtr;
    GT_PHYSICAL_PORT_NUM                portNum    = 0;
    GT_BOOL                             isExtendedCascadePort;
    GT_U32                              dataBitmapGet;

    /* there is no GPIO in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->port;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            portsMapInfoShadowPtr = &portPtr->portsMapInfoShadowArr[portNum];
            isExtendedCascadePort = portsMapInfoShadowPtr->valid &&
                                    portsMapInfoShadowPtr->portMap.isExtendedCascadePort;

            st = cpssDxChGpioPhyDataRead(dev, portNum, &dataBitmapGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(isExtendedCascadePort ? GT_OK : GT_BAD_PARAM, st, dev, portNum);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChGpioPhyDataRead(dev, portNum, &dataBitmapGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChGpioPhyDataRead(dev, portNum, &dataBitmapGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChGpioPhyDataRead(dev, portNum, &dataBitmapGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChGpioPhyDataRead(dev, portNum, &dataBitmapGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, portNum);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChGpioPhyDataRead(dev, portNum, &dataBitmapGet);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChGpioPhyDataWrite
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          dataBitmap,
    IN  GT_U32                          dataBitmapMask
)
*/
UTF_TEST_CASE_MAC(cpssDxChGpioPhyDataWrite)
{
/*
    ITERATE_DEVICES (AC3X)
    ITERATE_DEVICES_PORTS (AC3X)

     1.1. Call function
    Expected: GT_OK on extended cascade ports, GT_BAD_PARAM on other ports.
*/

    GT_STATUS   st = GT_OK;

    GT_U8       dev;
    PRV_CPSS_DXCH_PORT_INFO_STC        *portPtr;
    CPSS_DXCH_DETAILED_PORT_MAP_STC    *portsMapInfoShadowPtr;
    GT_PHYSICAL_PORT_NUM                portNum    = 0;
    GT_BOOL                             isExtendedCascadePort;
    GT_U32                              dataBitmap;
    GT_U32                              dataBitmapMask;

    /* there is no GPIO in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_AC3X_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        portPtr = &PRV_CPSS_DXCH_PP_MAC(dev)->port;

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_TRUE))
        {
            portsMapInfoShadowPtr = &portPtr->portsMapInfoShadowArr[portNum];
            isExtendedCascadePort = portsMapInfoShadowPtr->valid &&
                                    portsMapInfoShadowPtr->portMap.isExtendedCascadePort;

            dataBitmap = 0xffff;
            dataBitmapMask = (0xff << ((portNum) % 8));
            st = cpssDxChGpioPhyDataWrite(dev, portNum, dataBitmap, dataBitmapMask);
            UTF_VERIFY_EQUAL2_PARAM_MAC(isExtendedCascadePort ? GT_OK : GT_BAD_PARAM, st, dev, portNum);
        }

        st = prvUtfNextMacPortReset(&portNum, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        dataBitmap = 0xffff;
        dataBitmapMask = 0xffff;
        /* 1.2. For all active devices go over all non available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&portNum, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChGpioPhyDataWrite(dev, portNum, dataBitmap, dataBitmapMask);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for portNum number.                      */
        portNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;
        st = cpssDxChGpioPhyDataWrite(dev, portNum, dataBitmap, dataBitmapMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChGpioPhyDataWrite(dev, portNum, dataBitmap, dataBitmapMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
    }

    /* 2. For not active devices check that function returns non GT_OK. */
    portNum = 0;
    dataBitmap = 0xffff;
    dataBitmapMask = 0xffff;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChGpioPhyDataWrite(dev, portNum, dataBitmap, dataBitmapMask);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, portNum);
    }

    /* 3. Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChGpioPhyDataWrite(dev, portNum, dataBitmap, dataBitmapMask);
    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portNum);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpSoftResetChipletsTrigger
(
    IN  GT_U8                   devNum,
    IN  GT_CHIPLETS_BMP         chipletsBmp
);
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpSoftResetChipletsTrigger)
{
/*
    ITERATE_DEVICES (SIP6)
    1.1. Call with correct dev.
    Expected: GT_OK.
*/
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_CHIPLETS_BMP         chipletsBmp;

    /* There are no wrong values for chipletsBmp - 0-value also valid.  */
    /* The API checks all existing chiplets to be configured or skipped */
    /* according to this bitmap and ignores other bits in the bitmap    */
    chipletsBmp = CPSS_CHIPLETS_UNAWARE_MODE_CNS;

    /* prepare iterator for go over all active SIP6 devices */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-null pointers.
            Expected: GT_OK.
        */

        /* The API will reset all specified chiplets and will make them not manageable */
        /* So called with empty chiplets bitmap                                        */
        st = cpssDxChHwPpSoftResetChipletsTrigger(dev, (GT_CHIPLETS_BMP)0);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpSoftResetChipletsTrigger(dev, chipletsBmp);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwPpSoftResetChipletsTrigger(dev, chipletsBmp);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpSoftResetChipletsSkipParamGet
(
    IN  GT_U8                           devNum,
    IN  GT_CHIPLETS_BMP                 chipletsBmp,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    OUT GT_BOOL                         *skipEnablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpSoftResetChipletsSkipParamGet)
{
/*
    ITERATE_DEVICES(All SIP6 Devices)
    1.1. Call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E],
    Expected: GT_OK.
    1.2. Call api with wrong skipType [wrong enum values].
    Expected: GT_BAD_PARAM.
    1.3. Call api with wrong skipEnablePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8                           dev;
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;
    GT_CHIPLETS_BMP                 chipletsBmp;
    GT_BOOL                         skipEnable;

    chipletsBmp = CPSS_CHIPLETS_UNAWARE_MODE_CNS;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E],
            Expected: GT_OK.
        */

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;

        st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
            dev, chipletsBmp, skipType, &skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E;

        st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
            dev, chipletsBmp, skipType, &skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E;

        st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
            dev, chipletsBmp, skipType, &skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E] */
        skipType = CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E;

        st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
            dev, chipletsBmp, skipType, &skipEnable);

        /*
            1.2. Call api with wrong chipletsBmp.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
            dev, 0/*chipletsBmp*/, skipType, &skipEnable);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_BAD_PARAM, st, "%d, skipEnablePtr = NULL", dev);

        /*
            1.3. Call api with wrong skipType [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChHwPpSoftResetChipletsSkipParamGet
                            (dev, chipletsBmp, skipType, &skipEnable),
                            skipType);

        /*
            1.4. Call api with wrong skipEnablePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
            dev, chipletsBmp, skipType, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_BAD_PTR, st, "%d, skipEnablePtr = NULL", dev);
    }

    /* restore correct values */
    skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
            dev, chipletsBmp, skipType, &skipEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
        dev, chipletsBmp, skipType, &skipEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwPpSoftResetChipletsSkipParamSet
(
    IN  GT_U8                           devNum,
    IN  GT_CHIPLETS_BMP                 chipletsBmp,
    IN  CPSS_HW_PP_RESET_SKIP_TYPE_ENT  skipType,
    IN  GT_BOOL                         skipEnable
);
*/
UTF_TEST_CASE_MAC(cpssDxChHwPpSoftResetChipletsSkipParamSet)
{
/*
    ITERATE_DEVICES(All SIP6 Devices)
    1.1. Call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E /
                            CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E],
                   skipEnable[GT_TRUE / GT_FALSE / GT_TRUE / GT_TRUE / GT_TRUE],
    Expected: GT_OK.
    1.2. Call cpssDxChHwPpSoftResetSkipParamGet
           with the same parameters.
    Expected: GT_OK and the same values than was set.
    1.3. Call api with wrong skipType [wrong enum values].
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS st = GT_OK;

    GT_U8                          dev;
    GT_CHIPLETS_BMP                chipletsBmp;
    CPSS_HW_PP_RESET_SKIP_TYPE_ENT skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;
    GT_BOOL                        skipEnable = GT_FALSE;
    GT_BOOL                        skipEnableGet = GT_FALSE;
    GT_U32                         skipEnableCase;
    GT_U32                         tileIndex;
    GT_U32                         chipletIndex;
    GT_BOOL                        skipUnit;

    /* there is no DFX in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* There are no wrong values for chipletsBmp - 0-value also valid.  */
    /* The API checks all existing chiplets to be configured or skipped */
    /* according to this bitmap and ignores other bits in the bitmap    */
    chipletsBmp = CPSS_CHIPLETS_UNAWARE_MODE_CNS;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E /
                                    CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E],
                           skipEnable[GT_TRUE / GT_FALSE / GT_TRUE / GT_TRUE / GT_TRUE],
            Expected: GT_OK.
        */
        for (skipEnableCase = 0; (skipEnableCase < 2); skipEnableCase++)
        {
            skipEnable = BIT2BOOL_MAC((skipEnableCase & 1));
            chipletsBmp = CPSS_CHIPLETS_UNAWARE_MODE_CNS;

            /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E],
                         skipEnable[GT_TRUE] */
            skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;

            st = cpssDxChHwPpSoftResetChipletsSkipParamSet(
                dev, chipletsBmp, skipType, skipEnable);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call cpssDxChHwPpSoftResetSkipParamGet
                       with the same parameters.
                Expected: GT_OK and the same values than was set.
            */
            st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
                dev, chipletsBmp, skipType, &skipEnableGet);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                      "cpssDxChHwPpSoftResetChipletsSkipParamGet: %d ", dev);

            /* Verifying values */
            UTF_VERIFY_EQUAL1_STRING_MAC(skipEnable, skipEnableGet,
                           "got another skipEnable then was set: %d", dev);

            /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E],
                         skipEnable[GT_FALSE] */
            skipType = CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E;

            tileIndex = 0;
            chipletIndex = 1;
            chipletsBmp = (1 << chipletIndex);
            prvCpssFalconRavenMemoryAccessCheck(dev, tileIndex, chipletIndex + PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E, &skipUnit);
            if (skipUnit == GT_FALSE)
            {
                st = cpssDxChHwPpSoftResetChipletsSkipParamSet(
                    dev, chipletsBmp, skipType, skipEnable);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* call get func again, 1.2. */
                st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
                    dev, chipletsBmp, skipType, &skipEnableGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChHwPpSoftResetChipletsSkipParamGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(skipEnable, skipEnableGet,
                               "got another skipEnable then was set: %d", dev);
            }

            /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E],
                         skipEnable[GT_TRUE] */
            skipType = CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E;
            chipletIndex = 2;
            chipletsBmp = (1 << chipletIndex);
            prvCpssFalconRavenMemoryAccessCheck(dev, tileIndex, chipletIndex + PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E, &skipUnit);
            if (skipUnit == GT_FALSE)
            {
                st = cpssDxChHwPpSoftResetChipletsSkipParamSet(
                    dev, chipletsBmp, skipType, skipEnable);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* call get func again, 1.2. */
                st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
                    dev, chipletsBmp, skipType, &skipEnableGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChHwPpSoftResetChipletsSkipParamGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(skipEnable, skipEnableGet,
                               "got another skipEnable then was set: %d", dev);
            }

            /* call with skipType[CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E],
                         skipEnable[GT_TRUE] */
            skipType = CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E;
            chipletIndex = 3;
            chipletsBmp = (1 << chipletIndex);
            prvCpssFalconRavenMemoryAccessCheck(dev, tileIndex, chipletIndex + PRV_CPSS_DXCH_UNIT_BASE_RAVEN_0_E, &skipUnit);
            if (skipUnit == GT_FALSE)
            {
                st = cpssDxChHwPpSoftResetChipletsSkipParamSet(
                    dev, chipletsBmp, skipType, skipEnable);

                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* call get func again, 1.2. */
                st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
                    dev, chipletsBmp, skipType, &skipEnableGet);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                          "cpssDxChHwPpSoftResetChipletsSkipParamGet: %d ", dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(skipEnable, skipEnableGet,
                               "got another skipEnable then was set: %d", dev);
            }
        }

        /*
            1.3. Call api with wrong skipType [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChHwPpSoftResetChipletsSkipParamSet
                            (dev, chipletsBmp, skipType, skipEnable),
                            skipType);
    }

    /* restore correct values */
    skipType = CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E;
    skipEnable = GT_TRUE;
    chipletsBmp = CPSS_CHIPLETS_UNAWARE_MODE_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
            dev, chipletsBmp, skipType, &skipEnableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwPpSoftResetChipletsSkipParamGet(
        dev, chipletsBmp, skipType, &skipEnableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwMppSelectSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          mppNum,
    OUT GT_U32                          mppSelect
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwMppSelectSet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with valid parameters.
    Expected: GT_OK.
    1.2. Call with wrong mppNum, other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.3. Call with out of range mppSelect, other params same as in 1.1.
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      mppNum;
    GT_U32      mppSelect;
    GT_U32      mppSelectGet;
    GT_U32      maxMppSelectors;
    GT_U32      startFromNotUsedMpp = 0;   /* Start cycle from MPP that is not in use for current device */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        switch(PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                maxMppSelectors = HARRIER_MAX_MPP_SELECTORS_NUM_CNS;
                startFromNotUsedMpp = 29;
                break;
            case CPSS_PP_FAMILY_DXCH_AC5P_E:
                maxMppSelectors = AC5P_MAX_MPP_SELECTORS_NUM_CNS;
                startFromNotUsedMpp = 31;
                break;
            default:
                maxMppSelectors = MAX_MPP_SELECTORS_NUM_CNS;
                startFromNotUsedMpp = 46;
        }

        for(mppNum = startFromNotUsedMpp; mppNum < maxMppSelectors; mppNum++)
        {
            for(mppSelect = 0; mppSelect < MAX_MPP_SELECTOR_VALUE_CNS; mppSelect++)
            {
                /*
                    1.1. Call with non-NULL pointers.
                    Expected: GT_OK.
                */
                st = cpssDxChHwMppSelectSet(dev, mppNum, mppSelect);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /*
                    1.2. Call with descSizePtr [NULL], other params same as in 1.1.
                    Expected: GT_BAD_PTR.
                */
                st = cpssDxChHwMppSelectGet(dev, mppNum, &mppSelectGet);
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* Verifying values */
                UTF_VERIFY_EQUAL1_STRING_MAC(mppSelect, mppSelectGet,
                               "got another mppSelect then was set: %d", dev);

            }
        }

        /*
            1.3. Call with wrong MPP number.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChHwMppSelectSet(dev, mppNum, mppSelect-1);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

            /*
            1.3. Call with wrong MPP number.
            Expected: GT_OUT_OF_RANGE.
        */
        st = cpssDxChHwMppSelectSet(dev, mppNum-1, mppSelect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    mppNum = 0;
    mppSelect = 0;
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwMppSelectSet(dev, mppNum, mppSelect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwMppSelectSet(dev, mppNum, mppSelect);
    /* the devType is not used yet */
    /*UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);*/
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChHwMppSelectGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          mppNum,
    OUT GT_U32                         *mppSelectPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChHwMppSelectGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with mppSelectPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with wrong mppNum, other params same as in 1.1.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      mppNum;
    GT_U32      mppSelect;
    GT_U32      maxMppSelectors;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        switch(PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_HARRIER_E:
                maxMppSelectors = HARRIER_MAX_MPP_SELECTORS_NUM_CNS;
                break;
            case CPSS_PP_FAMILY_DXCH_AC5P_E:
                maxMppSelectors = AC5P_MAX_MPP_SELECTORS_NUM_CNS;
                break;
            default:
                maxMppSelectors = MAX_MPP_SELECTORS_NUM_CNS;
        }

        for(mppNum = 0; mppNum < maxMppSelectors; mppNum++)
        {
            /*
                1.1. Call with non-NULL pointers.
                Expected: GT_OK.
            */
            st = cpssDxChHwMppSelectGet(dev, mppNum, &mppSelect);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.2. Call with descSizePtr [NULL], other params same as in 1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChHwMppSelectGet(dev, mppNum, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
        }
        /*
            1.3. Call with wrong MPP number.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChHwMppSelectGet(dev, mppNum, &mppSelect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    mppNum = 0;
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChHwMppSelectGet(dev, mppNum, &mppSelect);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChHwMppSelectGet(dev, mppNum, &mppSelect);
    /* the devType is not used yet */
    /*UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);*/
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChHwInit suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChHwInit)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgHwCheckBadEndAddrWa)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwAuDescSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwCoreClockGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwInterruptCoalescingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwInterruptCoalescingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpPhase1Init)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpPhase2Init)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpSoftResetTrigger)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpStartInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwRxBufAlignmentGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwRxDescSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwTxDescSizeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgHwDevNumSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpImplementWaInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgHwDevNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChCfgHwCheckBadAddrWa)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpInitStageGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpSoftResetSkipParamGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpSoftResetSkipParamSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwAccessInfoStoreTest)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChGpioPhyConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChGpioPhyConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChGpioPhyDataRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChGpioPhyDataWrite)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpSoftResetChipletsTrigger)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpSoftResetChipletsSkipParamGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwPpSoftResetChipletsSkipParamSet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwMppSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChHwMppSelectGet)

UTF_SUIT_END_TESTS_MAC(cpssDxChHwInit)



GT_VOID  checkBadAddrWa
(
    void
)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      data;
    CPSS_DXCH_IMPLEMENT_WA_ENT          waFromCpss[1];
    GT_U32      portGroupId;
    GT_U32      portGroupsBmp;
    GT_U32      unitArray[MAX_UNIT_NUM_CNS];    /* Array of units that are used */
    GT_U32      numOfUnits;         /* total number of units in the device */
    GT_U32      unitId;             /* unit index in the array of units */
    GT_U32      regAddr;            /* register address  */
    GT_U32      unitAddrRange;      /* the last address in the unit */


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        {
            /* XCAT units db initialization  */
            /* Unit Id is defined by bits 23-28 */
            numOfUnits = 25;

            unitArray[0] =  0x0;    /* 0x00000000   */
            unitArray[1] =  0x3;    /* 0x01800000   */
            unitArray[2] =  0x4;    /* 0x02000000   */
            unitArray[3] =  0x5;    /* 0x02800000   */
            unitArray[4] =  0x6;    /* 0x03000000   */
            unitArray[5] =  0x7;    /* 0x03800000   */
            unitArray[6] =  0x8;    /* 0x04000000   */
            unitArray[7] =  0xC;    /* 0x06000000   */
            unitArray[8] =  0xF;    /* 0x07800000   */
            unitArray[9] =  0x10;   /* 0x08000000   */
            unitArray[10] = 0x11;   /* 0x08800000   */
            unitArray[11] = 0x12;   /* 0x09000000   */
            unitArray[12] = 0x13;   /* 0x09800000   */
            unitArray[13] = 0x14;   /* 0x0A000000   */
            unitArray[14] = 0x15;   /* 0x0A800000   */
            unitArray[15] = 0x16;   /* 0x0B000000   */
            unitArray[16] = 0x17;   /* 0x0B800000   */
            unitArray[17] = 0x18;   /* 0x0C000000   */
            unitArray[18] = 0x19;   /* 0x0C800000   */
            unitArray[19] = 0x1A;   /* 0x0D000000   */
            unitArray[20] = 0x1B;   /* 0x0D800000   */
            unitArray[21] = 0x1C;   /* 0x0E000000   */
            unitArray[22] = 0x1D;   /* 0x0E800000   */
            unitArray[23] = 0x1E;   /* 0x0F000000   */
            unitArray[24] = 0x1F;   /* 0x0F800000   */

            portGroupsBmp = 0x1;
        }

        /* enable Check Address WA */
        waFromCpss[0] = CPSS_DXCH_IMPLEMENT_WA_RESTRICTED_ADDRESS_FILTERING_E;
        st = cpssDxChHwPpImplementWaInit(dev,1,&waFromCpss[0],0);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if(PRV_CPSS_DXCH_PP_MAC(dev)->errata.
            info_PRV_CPSS_DXCH_XCAT_RESTRICTED_ADDRESS_FILTERING_WA_E.unitRangesPtr == NULL)
        {
            /* The WA is not initialized */
            return;
        }

        /* 1.1. Go over all active port groups from the bitmap */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(dev, portGroupsBmp, portGroupId)
        {
            for (unitId = 0; unitId < numOfUnits; unitId++)
            {
              PRV_UTF_LOG0_MAC("------------------------------\n");
              PRV_UTF_LOG1_MAC("unit = %d\n", unitArray[unitId]);
              PRV_UTF_LOG0_MAC("------------------------------\n");

                /* The first register address in the unit */
                regAddr = (unitArray[unitId] << 23);

                /* calculate the memory range to check */
                unitAddrRange = regAddr + 0x7FFFFF;

                for (regAddr = regAddr; regAddr < unitAddrRange; regAddr +=4 /*64*/)
                {
                    /*cpssOsPrintSync("regAddr = 0x%x\n", regAddr);*/

                    st = prvCpssDrvHwPpPortGroupReadRegister(dev, portGroupId,
                                                             regAddr,
                                                             &data);

                    /* GT_BAD_PARAM status - indicates a restricted address,
                       GT_OK status - indicates valid address */
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_FAIL, st, dev, portGroupId,
                                                regAddr);

                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_NOT_INITIALIZED, st, dev, portGroupId,
                                                regAddr);

                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_HW_ERROR, st, dev, portGroupId,
                                                regAddr);
                }
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)


        /* Disable Address Checking WA */
        st = prvCpssDrvAddrCheckWaBind(dev,NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }
}

/* Restricted address check for xCat3*/
GT_VOID cpssDxChHwCheckxCat3BadAddrWa
(
    void
)
{
/*
    1.1. Call with all address ranges starting from
         0x00000000-0x1D0C0FF(last valid address of xCat3) with WA enabled.
    Expected: GT_OK        - for valid address
              GT_BAD_PARAM - for restricted address.

*/
    GT_STATUS   st = GT_OK,rc = GT_OK;
    GT_U8       dev;
    GT_U32      singleRanges=0;
    GT_U32      i,j;
    GT_U32      data;
    CPSS_DXCH_IMPLEMENT_WA_ENT          waFromCpss[1];
    HWINIT_ERRATA_BAD_ADDR_RANGE_STC    AddrsArray[35];
    GT_U32      portGroupId;
    GT_U32      portGroupsBmp = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_XCAT_E| UTF_XCAT2_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev))
        {

            AddrsArray[0].addrStart = 0x00000000;
            AddrsArray[0].addrEnd = 0x00FFFFFFC;

            AddrsArray[1].addrStart = 0x01000000;
            AddrsArray[1].addrEnd = 0x01FFFFFC;

            AddrsArray[2].addrStart = 0x02000000;
            AddrsArray[2].addrEnd = 0x02FFFFFC;

            AddrsArray[3].addrStart = 0x03000000;
            AddrsArray[3].addrEnd = 0x03FFFFFC;

            AddrsArray[4].addrStart = 0x04000000;
            AddrsArray[4].addrEnd = 0x04FFFFFC;

            AddrsArray[5].addrStart = 0x05000000;
            AddrsArray[5].addrEnd = 0x05FFFFFC;

            AddrsArray[6].addrStart = 0x06000000;
            AddrsArray[6].addrEnd = 0x06FFFFFC;

            AddrsArray[7].addrStart = 0x07000000;
            AddrsArray[7].addrEnd = 0x07FFFFFC;

            AddrsArray[8].addrStart = 0x08000000;
            AddrsArray[8].addrEnd = 0x08FFFFFC;

            AddrsArray[9].addrStart = 0x09000000;
            AddrsArray[9].addrEnd = 0x09FFFFFC;

            AddrsArray[10].addrStart = 0x0A000000;
            AddrsArray[10].addrEnd = 0x0AFFFFFC;

            AddrsArray[11].addrStart = 0x0B000000;
            AddrsArray[11].addrEnd = 0x0BFFFFFC;

            AddrsArray[12].addrStart = 0x0C000000;
            AddrsArray[12].addrEnd = 0x0CFFFFFC;

            AddrsArray[13].addrStart = 0x0D000000;
            AddrsArray[13].addrEnd = 0x0DFFFFFC;

            AddrsArray[14].addrStart = 0x0E000000;
            AddrsArray[14].addrEnd = 0x0EFFFFFC;

            AddrsArray[15].addrStart = 0x0F000000;
            AddrsArray[15].addrEnd = 0x0FFFFFFC;

            AddrsArray[16].addrStart = 0x10000000;
            AddrsArray[16].addrEnd = 0x10FFFFFC;

            AddrsArray[17].addrStart = 0x11000000;
            AddrsArray[17].addrEnd = 0x11FFFFFC;

            AddrsArray[18].addrStart = 0x12000000;
            AddrsArray[18].addrEnd = 0x12FFFFFC;

            AddrsArray[19].addrStart = 0x13000000;
            AddrsArray[19].addrEnd = 0x13FFFFFC;

            AddrsArray[20].addrStart = 0x14000000;
            AddrsArray[20].addrEnd = 0x14FFFFFC;

            AddrsArray[21].addrStart = 0x15000000;
            AddrsArray[21].addrEnd = 0x15FFFFFC;

            AddrsArray[22].addrStart = 0x16000000;
            AddrsArray[22].addrEnd = 0x16FFFFFC;

            AddrsArray[23].addrStart = 0x17000000;
            AddrsArray[23].addrEnd = 0x17FFFFFC;

            AddrsArray[24].addrStart = 0x18000000;
            AddrsArray[24].addrEnd = 0x18FFFFFC;

            AddrsArray[25].addrStart = 0x19000000;
            AddrsArray[25].addrEnd = 0x19FFFFFC;

            AddrsArray[26].addrStart = 0x1A000000;
            AddrsArray[26].addrEnd = 0x1AFFFFFC;

            AddrsArray[27].addrStart = 0x1B000000;
            AddrsArray[27].addrEnd = 0x1BFFFFFC;

            AddrsArray[28].addrStart = 0x1C000000;
            AddrsArray[28].addrEnd = 0x1CFFFFFC;

            AddrsArray[29].addrStart = 0x1D000000;
            AddrsArray[29].addrEnd = 0x1DFFFFFC;

            singleRanges = 30;
        }

        /* enable Check Address WA */
        waFromCpss[0] = CPSS_DXCH_IMPLEMENT_WA_RESTRICTED_ADDRESS_FILTERING_E;
        st = cpssDxChHwPpImplementWaInit(dev,1,&waFromCpss[0],0);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if(PRV_CPSS_DXCH_PP_MAC(dev)->errata.
            info_PRV_CPSS_DXCH_XCAT_RESTRICTED_ADDRESS_FILTERING_WA_E.unitRangesPtr[0] == NULL)
        {
            /* The WA is not initialized */
            return;
        }

        /* 1.1. Go over all active port groups from the bitmap */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(dev, portGroupsBmp, portGroupId)
        {
            for (i = 0; i < singleRanges; i++)
            {
                for (j = AddrsArray[i].addrStart; j <= AddrsArray[i].addrEnd; j=j+0x4)
                {

                    rc = prvCpssDrvHwPpPortGroupReadRegister(dev, portGroupId,
                                                              j,
                                                             &data);

                    st = prvCpssDrvHwPpPortGroupReadRegister(dev, portGroupId,
                                                  0x00000050, &data);
                    if(data == 0x11AB)
                    {
                        cpssOsPrintf("\n%d: 0x%8.8x",rc,j);
                    }
                    else
                    {
                        return;
                    }
                }
            }
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

        /* Disable Address Checking WA */
        st = prvCpssDrvAddrCheckWaBind(dev,NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

}


/* Restricted address check */
GT_VOID cpssDxChHwCheckRegAccess_test
(
    GT_U32 startAddr,
    GT_U32 endAddr,
    GT_U32 printRation
)
{
/*
    1.1. Call with all address ranges starting from
         0x00000000-0x1D0C0FF(last valid address of xCat3) with WA enabled.
    Expected: GT_OK        - for valid address
              GT_BAD_PARAM - for restricted address.

*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      i,j;
    GT_U32      data;
    CPSS_DXCH_IMPLEMENT_WA_ENT          waFromCpss[1];
    GT_U32      portGroupId;
    GT_U32      portGroupsBmp = 1;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_XCAT_E| UTF_XCAT2_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* enable Check Address WA */
        waFromCpss[0] = CPSS_DXCH_IMPLEMENT_WA_RESTRICTED_ADDRESS_FILTERING_E;
        st = cpssDxChHwPpImplementWaInit(dev,1,&waFromCpss[0],0);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if(PRV_CPSS_DXCH_PP_MAC(dev)->errata.
            info_PRV_CPSS_DXCH_XCAT_RESTRICTED_ADDRESS_FILTERING_WA_E.unitRangesPtr[0] == NULL)
        {
            /* The WA is not initialized */
            return;
        }

        /* 1.1. Go over all active port groups from the bitmap */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_IN_BMP_MAC(dev, portGroupsBmp, portGroupId)
        {
            for (j = startAddr, i = 0; j <= endAddr; j=j+0x4, i++)
            {
                if((i%printRation) == 0)
                {
                    cpssOsPrintf("\n0x%8.8x",j);
                }

                prvCpssDrvHwPpPortGroupReadRegister(dev, portGroupId,
                                                          j,
                                                         &data);
            }
        }

        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_IN_BMP_MAC(devNum,portGroupsBmp,portGroupId)

        /* Disable Address Checking WA */
        st = prvCpssDrvAddrCheckWaBind(dev,NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

}


GT_STATUS   calcReadWriteTime
(
    IN GT_U8    dev,
    IN GT_U32   startRegAddr,
    IN GT_U32   numOfCycles
)
{
    GT_U32      startSeconds;
    GT_U32      startNanoSeconds;
    GT_U32      endSeconds;
    GT_U32      endNanoSeconds;
    GT_U32      seconds;
    GT_U32      nanoseconds;
    GT_U32      i;
    GT_U32      regAddr;
    GT_U32      data;
    GT_STATUS   rc;

    /* Reading cycles */

    regAddr = startRegAddr;

    rc = cpssOsTimeRT(&startSeconds, &startNanoSeconds);
    if (rc != GT_OK)
    {
        return rc;
    }


    for (i = 0; i < numOfCycles; i++)
    {
       rc = prvCpssDrvHwPpReadRegister(dev, regAddr, &data);
       if (rc != GT_OK)
       {
           return rc;
       }

       regAddr += 4;
    }

    rc = cpssOsTimeRT(&endSeconds, &endNanoSeconds);
    if (rc != GT_OK)
    {
        return rc;
    }

    PRV_UTF_LOG0_SYNC_MAC("------- Reading ---------\n");

    PRV_UTF_LOG2_SYNC_MAC("Start Time:seconds = %d, nanoseconds = %d\n",
                          startSeconds, startNanoSeconds);
    PRV_UTF_LOG2_SYNC_MAC("End Time:seconds = %d, nanoseconds = %d\n",
                          endSeconds, endNanoSeconds);

    seconds = endSeconds - startSeconds;
    if (seconds == 0)
    {
        nanoseconds = endNanoSeconds - startNanoSeconds;
    }
    else
        nanoseconds = 0;

    PRV_UTF_LOG3_SYNC_MAC("Reading %d cycles:seconds = %d, nanoseconds = %d\n",
                          numOfCycles, seconds, nanoseconds);

    return GT_OK;
}

GT_STATUS   test_copyBitsInMemory(IN GT_U32 trgBit , IN GT_U32 srcBit , IN GT_U32 pattern)
{
    GT_U32  ii;
    GT_U32  array[8];    /* 256 bits array */
    /*GT_U32  pattern = BIT_0 | BIT_3 | BIT_9 | BIT_10 | BIT_15 | BIT_23 | BIT_27 | BIT_31;*/

    for(ii = 0 ; ii < 8 ;ii++)
    {
        array[ii] = pattern;
    }

    /* print bit value : 0/1 */
#define PRINT_BIT(arr,bit)  \
    cpssOsPrintf("%d",((arr[bit>>5] & (1<<(bit&0x1f)))?1:0))

    for(ii = 0 ; ii < 256 ;ii+=10)
    {
        cpssOsPrintf("%d",(ii/10)%10);

        cpssOsPrintf(" ");  /*1*/
        cpssOsPrintf(" ");  /*2*/
        cpssOsPrintf(" ");  /*3*/
        cpssOsPrintf(" ");  /*4*/
        cpssOsPrintf(" ");  /*5*/
        cpssOsPrintf(" ");  /*6*/
        cpssOsPrintf(" ");  /*7*/
        cpssOsPrintf(" ");  /*8*/
        cpssOsPrintf(" ");  /*9*/
    }
    cpssOsPrintf(" \n\n");

    /* the original values */
    cpssOsPrintf("the original values \n");
    for(ii = 0 ; ii < 256 ;ii++)
    {
        PRINT_BIT(array,ii);
    }
    cpssOsPrintf(" \n\n");

    cpssOsPrintf("move bits from %d to %d : \n",srcBit,trgBit);
    /* move bits from srcBit to trgBit */
    copyBitsInMemory(array,
        trgBit,/*targetStartBit*/
        srcBit,/*sourceStartBit*/
        256-trgBit/*numBits*/);

    for(ii = 0 ; ii < 256 ;ii++)
    {
        PRINT_BIT(array,ii);
    }
    cpssOsPrintf(" \n\n");

    return GT_OK;
}
