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
* @file cpssDxChPpuUT.c
*
* @brief Unit tests for cpssDxChPpu.h
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/ppu/private/prvCpssDxChPpu.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpu.h>
#include <cpss/dxCh/dxChxGen/ppu/cpssDxChPpuTypes.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
/*
GT_STATUS cpssDxChPpuEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPpuEnableSet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_BOOL     enable;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Check for enable = GT_TRUE */
        enable = GT_TRUE;
        rc = cpssDxChPpuEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPpuEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);

        /* Check for enable = GT_FALSE */
        enable = GT_FALSE;
        rc = cpssDxChPpuEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPpuEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(enable, enableGet, dev);

    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    enable = GT_TRUE;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuEnableSet(dev,enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}


/*
GT_STATUS cpssDxChPpuEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_BOOL     *enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChPpuEnableGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_BOOL     enableGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        rc = cpssDxChPpuEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check NULL pointer */
        rc = cpssDxChPpuEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuEnableGet(dev, &enableGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/* GT_STATUS cpssDxChPpuMaxLoopbackSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      maxLoopback
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuMaxLoopbackSet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U32      maxLoopback;
    GT_U32      maxLoopbackGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Check for proper register setting */
        maxLoopback = 2;
        rc = cpssDxChPpuMaxLoopbackSet(dev, maxLoopback);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPpuMaxLoopbackGet(dev, &maxLoopbackGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(maxLoopback, maxLoopbackGet, dev);

        /* check out of range */
        maxLoopback = CPSS_DXCH_PPU_MAX_LOOPBACK_NUM_CNS +1;
        rc = cpssDxChPpuMaxLoopbackSet(dev, maxLoopback);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    maxLoopback = 2;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuMaxLoopbackSet(dev, maxLoopback);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuMaxLoopbackSet(dev, maxLoopback);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuMaxLoopbackGet
(
    IN  GT_U8       devNum,
    OUT GT_U32      *maxLoopbackPtr
); */
UTF_TEST_CASE_MAC(cpssDxChPpuMaxLoopbackGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U32      maxLoopbackGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        rc = cpssDxChPpuMaxLoopbackGet(dev, &maxLoopbackGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuMaxLoopbackGet(dev, &maxLoopbackGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuMaxLoopbackGet(dev, &maxLoopbackGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/* GT_STATUS cpssDxChPpuGlobalConfigSet
(
    IN  GT_U8       devNum,
    IN  CPSS_DXCH_PPU_GLOBAL_CONFIG_STC     *ppuGlobalConfigPtr
); */
UTF_TEST_CASE_MAC(cpssDxChPpuGlobalConfigSet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    CPSS_DXCH_PPU_GLOBAL_CONFIG_STC      globalCfg;
    CPSS_DXCH_PPU_GLOBAL_CONFIG_STC      globalCfgGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsMemSet(&globalCfg, 0, sizeof(CPSS_DXCH_PPU_GLOBAL_CONFIG_STC));
        cpssOsMemSet(&globalCfgGet, 0, sizeof(CPSS_DXCH_PPU_GLOBAL_CONFIG_STC));

        globalCfg.maxLoopBack = 3;
        rc = cpssDxChPpuGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPpuGlobalConfigGet(dev, &globalCfgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(globalCfg.maxLoopBack, globalCfgGet.maxLoopBack, dev);

        rc = cpssDxChPpuGlobalConfigSet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuGlobalConfigSet(dev, &globalCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/*GT_STATUS cpssDxChPpuGlobalConfigGet
(
    IN  GT_U8       devNum,
    OUT CPSS_DXCH_PPU_GLOBAL_CONFIG_STC     *ppuGlobalConfigPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPpuGlobalConfigGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    CPSS_DXCH_PPU_GLOBAL_CONFIG_STC      globalCfgGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        rc = cpssDxChPpuGlobalConfigGet(dev, &globalCfgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check NULL pointer */
        rc = cpssDxChPpuGlobalConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuGlobalConfigGet(dev, &globalCfgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuGlobalConfigGet(dev, &globalCfgGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuTtiActionProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ttiRuleIndex,
    IN  GT_U32                              profileNum
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuTtiActionProfileIndexSet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U32      ttiRuleIndex = 964;
    GT_U32      profileNum;
    GT_U32      profileNumGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        profileNum = 8;
        rc = cpssDxChPpuTtiActionProfileIndexSet(dev, ttiRuleIndex, profileNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPpuTtiActionProfileIndexGet(dev, ttiRuleIndex, &profileNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(profileNum, profileNumGet, dev);

        profileNum = CPSS_DXCH_PPU_PROFILE_ENTRIES_MAX_CNS;
        rc = cpssDxChPpuTtiActionProfileIndexSet(dev, ttiRuleIndex, profileNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        profileNum = 8;
        rc = cpssDxChPpuTtiActionProfileIndexSet(dev, ttiRuleIndex, profileNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuTtiActionProfileIndexSet(dev, ttiRuleIndex, profileNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuTtiActionProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ttiRuleIndex,
    IN  GT_U32                              *profileNumPtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuTtiActionProfileIndexGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U32      ttiRuleIndex = 964;
    GT_U32      profileNumGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        rc = cpssDxChPpuTtiActionProfileIndexGet(dev, ttiRuleIndex, &profileNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPpuTtiActionProfileIndexGet(dev, ttiRuleIndex, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuTtiActionProfileIndexGet(dev, ttiRuleIndex, &profileNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuTtiActionProfileIndexGet(dev, ttiRuleIndex, &profileNumGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuSrcPortProfileIndexSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    IN  GT_U32                              profileNum
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuSrcPortProfileIndexSet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U32      srcPortNum = 4;
    GT_U32      profileNum;
    GT_U32      profileNumGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        profileNum = 8;
        rc = cpssDxChPpuSrcPortProfileIndexSet(dev, srcPortNum, profileNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPpuSrcPortProfileIndexGet (dev, srcPortNum, &profileNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        UTF_VERIFY_EQUAL1_PARAM_MAC(profileNum, profileNumGet, dev);

        profileNum = CPSS_DXCH_PPU_PROFILE_ENTRIES_MAX_CNS;
        rc = cpssDxChPpuSrcPortProfileIndexSet(dev, srcPortNum, profileNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        profileNum = 8;
        rc = cpssDxChPpuSrcPortProfileIndexSet(dev, srcPortNum, profileNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuSrcPortProfileIndexSet(dev, srcPortNum, profileNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuSrcPortProfileIndexGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                srcPortNum,
    OUT GT_U32                              *profileNumPtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuSrcPortProfileIndexGet )
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U32      srcPortNum = 4;
    GT_U32      profileNumGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        rc = cpssDxChPpuSrcPortProfileIndexGet (dev, srcPortNum, &profileNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChPpuSrcPortProfileIndexGet (dev, srcPortNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuSrcPortProfileIndexGet (dev, srcPortNum, &profileNumGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuSrcPortProfileIndexGet (dev, srcPortNum, &profileNumGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuProfileSet
(
    IN  GT_U8                   devNum,
    IN  GT_U8                   profileNum,
    IN  CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuProfileSet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U8       profileNum;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC      ppuProfile;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC      ppuProfileGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(profileNum = 0; profileNum < CPSS_DXCH_PPU_SP_BUS_PROFILE_ENTRIES_MAX_CNS; profileNum++)
        {
            cpssOsMemSet(&ppuProfile, 0, sizeof(CPSS_DXCH_PPU_PROFILE_ENTRY_STC));
            cpssOsMemSet(&ppuProfileGet, 0, sizeof(CPSS_DXCH_PPU_PROFILE_ENTRY_STC));

            /* check for proper profile setting */
            ppuProfile.ppuEnable = GT_TRUE;
            ppuProfile.spBusDefaultProfile = 1;
            ppuProfile.offset = 16;

            rc = cpssDxChPpuProfileSet(dev, profileNum, &ppuProfile);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChPpuProfileGet(dev, profileNum, &ppuProfileGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(ppuProfile.ppuEnable, ppuProfileGet.ppuEnable, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(ppuProfile.spBusDefaultProfile, ppuProfileGet.spBusDefaultProfile, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(ppuProfile.offset, ppuProfileGet.offset, dev);
        }
        /* check for null pointer */
        rc = cpssDxChPpuProfileSet(dev, profileNum, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range pofile index */
        profileNum = CPSS_DXCH_PPU_PROFILE_ENTRIES_MAX_CNS;
        rc = cpssDxChPpuProfileSet(dev, profileNum, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    profileNum = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuProfileSet(dev, profileNum, &ppuProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuProfileSet(dev, profileNum, &ppuProfile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/* GT_STATUS cpssDxChPpuProfileGet
(
    IN  GT_U8                   devNum,
    IN  GT_U8                   profileNum,
    OUT CPSS_DXCH_PPU_PROFILE_ENTRY_STC *ppuProfilePtr
); */
UTF_TEST_CASE_MAC(cpssDxChPpuProfileGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U8       profileNum;
    CPSS_DXCH_PPU_PROFILE_ENTRY_STC      ppuProfileGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(profileNum = 0; profileNum < CPSS_DXCH_PPU_SP_BUS_PROFILE_ENTRIES_MAX_CNS; profileNum++)
        {
            rc = cpssDxChPpuProfileGet(dev, profileNum, &ppuProfileGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }

        /* check NULL pointer */
        rc = cpssDxChPpuProfileGet(dev, profileNum, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range pofile index */
        profileNum = CPSS_DXCH_PPU_PROFILE_ENTRIES_MAX_CNS;
        rc = cpssDxChPpuProfileGet(dev, profileNum, &ppuProfileGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */
    profileNum = 1;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuProfileGet(dev, profileNum, &ppuProfileGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuProfileGet(dev, profileNum, &ppuProfileGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/* GT_STATUS cpssDxChPpuSpBusDefaultProfileSet
(
    IN  GT_U32                  devNum,
    IN  GT_U32                  profileNum,
    IN  CPSS_DXCH_PPU_SP_BUS_STC    *spBusProfilePtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPpuSpBusDefaultProfileSet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U8       profileNum;
    CPSS_DXCH_PPU_SP_BUS_STC      spBusProfile;
    CPSS_DXCH_PPU_SP_BUS_STC      spBusProfileGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(profileNum = 0; profileNum < CPSS_DXCH_PPU_SP_BUS_PROFILE_ENTRIES_MAX_CNS; profileNum++)
        {
            cpssOsMemSet(&spBusProfile, 0, sizeof(CPSS_DXCH_PPU_SP_BUS_STC));
            cpssOsMemSet(&spBusProfileGet, 0, sizeof(CPSS_DXCH_PPU_SP_BUS_STC));

            /* check for proper profile setting */
            spBusProfile.data[0] = 0x12;
            spBusProfile.data[2] = 0x34;
            spBusProfile.data[4] = 0x56;
            spBusProfile.data[6] = 0x78;

            rc = cpssDxChPpuSpBusDefaultProfileSet(dev, profileNum, &spBusProfile);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            rc = cpssDxChPpuSpBusDefaultProfileGet(dev, profileNum, &spBusProfileGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

            UTF_VERIFY_EQUAL1_PARAM_MAC(spBusProfile.data[0], spBusProfileGet.data[0], dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(spBusProfile.data[2], spBusProfileGet.data[2], dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(spBusProfile.data[4], spBusProfileGet.data[4], dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(spBusProfile.data[6], spBusProfileGet.data[6], dev);
        }

        profileNum = 0;

        /* check for null pointer */
        rc = cpssDxChPpuSpBusDefaultProfileSet(dev, profileNum, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range pofile index */
        profileNum = CPSS_DXCH_PPU_SP_BUS_PROFILE_ENTRIES_MAX_CNS;
        rc = cpssDxChPpuSpBusDefaultProfileSet(dev, profileNum, &spBusProfile);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    profileNum = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuSpBusDefaultProfileSet(dev, profileNum, &spBusProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuSpBusDefaultProfileSet(dev, profileNum, &spBusProfile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/*GT_STATUS cpssDxChPpuSpBusDefaultProfileGet
(
    IN  GT_U32                  devNum,
    IN  GT_U32                  profileNum,
    OUT CPSS_DXCH_PPU_SP_BUS_STC    *spBusProfilePtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPpuSpBusDefaultProfileGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U8       profileNum;
    CPSS_DXCH_PPU_SP_BUS_STC      spBusProfileGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(profileNum = 0; profileNum < CPSS_DXCH_PPU_SP_BUS_PROFILE_ENTRIES_MAX_CNS; profileNum++)
        {
            rc = cpssDxChPpuSpBusDefaultProfileGet(dev, profileNum, &spBusProfileGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }

        profileNum = 0;

        /* check NULL pointer */
        rc = cpssDxChPpuSpBusDefaultProfileGet(dev, profileNum, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range pofile index */
        profileNum = CPSS_DXCH_PPU_SP_BUS_PROFILE_ENTRIES_MAX_CNS;
        rc = cpssDxChPpuSpBusDefaultProfileGet(dev, profileNum, &spBusProfileGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */
    profileNum = 1;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuSpBusDefaultProfileGet(dev, profileNum, &spBusProfileGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuSpBusDefaultProfileGet(dev, profileNum, &spBusProfileGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuKstgKeyGenProfileSet
(
    IN  GT_U8           devNum,
    IN  GT_U8           kstgNum,
    IN  GT_U8           profileNum,
    IN  CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPpuKstgKeyGenProfileSet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U8       profileNum = 0;
    GT_U8       kstgNum;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  keygenProfile;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  keygenProfileGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(kstgNum = 0; kstgNum < CPSS_DXCH_PPU_KSTGS_MAX_CNS; kstgNum++)
        {
            for(profileNum = 0; profileNum < CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILES_MAX_CNS; profileNum++)
            {
                cpssOsMemSet(&keygenProfile, 0, sizeof(CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC));
                cpssOsMemSet(&keygenProfileGet, 0, sizeof(CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC));

                /* check for proper profile setting */
                keygenProfile.byteSelOffset[0] = 4;
                keygenProfile.byteSelOffset[1] = 6;
                keygenProfile.byteSelOffset[2] = 8;
                keygenProfile.byteSelOffset[3] = 10;
                keygenProfile.byteSelOffset[4] = 12;
                keygenProfile.byteSelOffset[5] = 14;
                keygenProfile.byteSelOffset[6] = 16;

                rc = cpssDxChPpuKstgKeyGenProfileSet(dev, kstgNum, profileNum, &keygenProfile);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, kstgNum);

                rc = cpssDxChPpuKstgKeyGenProfileGet(dev, kstgNum, profileNum, &keygenProfileGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, kstgNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(keygenProfile.byteSelOffset[0], keygenProfileGet.byteSelOffset[0], dev, kstgNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(keygenProfile.byteSelOffset[1], keygenProfileGet.byteSelOffset[1], dev, kstgNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(keygenProfile.byteSelOffset[2], keygenProfileGet.byteSelOffset[2], dev, kstgNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(keygenProfile.byteSelOffset[3], keygenProfileGet.byteSelOffset[3], dev, kstgNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(keygenProfile.byteSelOffset[4], keygenProfileGet.byteSelOffset[4], dev, kstgNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(keygenProfile.byteSelOffset[5], keygenProfileGet.byteSelOffset[5], dev, kstgNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(keygenProfile.byteSelOffset[6], keygenProfileGet.byteSelOffset[6], dev, kstgNum);
            }

            profileNum = 0;

            /* check for null pointer */
            rc = cpssDxChPpuKstgKeyGenProfileSet(dev, kstgNum, profileNum, NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

            /* check for out of range pofile index */
            profileNum = CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILES_MAX_CNS;
            rc = cpssDxChPpuKstgKeyGenProfileSet(dev, kstgNum, profileNum, &keygenProfile);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
        }

        /* check for out of range kstg number */
        kstgNum = CPSS_DXCH_PPU_KSTGS_MAX_CNS;
        profileNum = 0;
        rc = cpssDxChPpuKstgKeyGenProfileSet(dev, kstgNum, profileNum, &keygenProfile);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    profileNum = 1;
    kstgNum = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuKstgKeyGenProfileSet(dev, kstgNum, profileNum, &keygenProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuKstgKeyGenProfileSet(dev, kstgNum, profileNum, &keygenProfile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/*GT_STATUS cpssDxChPpuKstgKeyGenProfileGet
(
    IN  GT_U8           devNum,
    IN  GT_U8           kstgNum,
    IN  GT_U8           profileNum,
    OUT CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  *keygenProfilePtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuKstgKeyGenProfileGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U8       profileNum = 0;
    GT_U8       kstgNum;
    CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILE_STC  keygenProfileGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(kstgNum = 0; kstgNum < CPSS_DXCH_PPU_KSTGS_MAX_CNS; kstgNum++)
        {
            for(profileNum = 0; profileNum < CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILES_MAX_CNS; profileNum++)
            {
                /* check normal functionality */
                rc = cpssDxChPpuKstgKeyGenProfileGet(dev, kstgNum, profileNum, &keygenProfileGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, kstgNum);
            }

            profileNum = 0;

            /* check for null pointer */
            rc = cpssDxChPpuKstgKeyGenProfileGet(dev, kstgNum, profileNum, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, rc, dev, kstgNum );

            /* check for out of range pofile index */
            profileNum = CPSS_DXCH_PPU_KSTG_KEY_GEN_PROFILES_MAX_CNS;
            rc = cpssDxChPpuKstgKeyGenProfileGet(dev, kstgNum, profileNum, &keygenProfileGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev, kstgNum);
        }

        /* check for out of range kstg number */
        kstgNum = CPSS_DXCH_PPU_KSTGS_MAX_CNS;
        profileNum = 0;
        rc = cpssDxChPpuKstgKeyGenProfileGet(dev, kstgNum, profileNum, &keygenProfileGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    profileNum = 1;
    kstgNum = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuKstgKeyGenProfileGet(dev, kstgNum, profileNum, &keygenProfileGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuKstgKeyGenProfileGet(dev, kstgNum, profileNum, &keygenProfileGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/*GT_STATUS cpssDxChPpuKstgTcamEntrySet
(
    IN  GT_U8           devNum,
    IN  GT_U8           kstgNum,
    IN  GT_U8           profileNum,
    IN  CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPpuKstgTcamEntrySet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U8       profileNum = 0;
    GT_U8       kstgNum;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC  kstgTcamEntry;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC  kstgTcamEntryGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(kstgNum = 0; kstgNum < CPSS_DXCH_PPU_KSTGS_MAX_CNS; kstgNum++)
        {
            for(profileNum = 0; profileNum < CPSS_DXCH_PPU_KSTG_TCAM_ENTRIES_MAX_CNS; profileNum++)
            {
                cpssOsMemSet(&kstgTcamEntry, 0, sizeof(CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC));
                cpssOsMemSet(&kstgTcamEntryGet, 0, sizeof(CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC));

                kstgTcamEntry.isValid = 1;
                kstgTcamEntry.keyLsb = 0x44332211;
                kstgTcamEntry.keyMsb = 0x00776655;
                kstgTcamEntry.maskLsb = 0xFFFFFFFF;
                kstgTcamEntry.maskMsb = 0x00FFFFFF;

                /* check for proper profile setting */
                rc = cpssDxChPpuKstgTcamEntrySet(dev, kstgNum, profileNum, &kstgTcamEntry);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, kstgNum);

                rc = cpssDxChPpuKstgTcamEntryGet(dev, kstgNum, profileNum, &kstgTcamEntryGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, kstgNum);

                UTF_VERIFY_EQUAL2_PARAM_MAC(kstgTcamEntry.isValid, kstgTcamEntryGet.isValid, dev, kstgNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(kstgTcamEntry.keyLsb, kstgTcamEntryGet.keyLsb, dev, kstgNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(kstgTcamEntry.keyMsb, kstgTcamEntryGet.keyMsb, dev, kstgNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(kstgTcamEntry.maskLsb, kstgTcamEntryGet.maskLsb, dev, kstgNum);
                UTF_VERIFY_EQUAL2_PARAM_MAC(kstgTcamEntry.maskMsb, kstgTcamEntryGet.maskMsb, dev, kstgNum);
            }

            profileNum = 0;

            /* check for null pointer */
            rc = cpssDxChPpuKstgTcamEntrySet(dev, kstgNum, profileNum, NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

            /* check for out of range pofile index */
            profileNum = CPSS_DXCH_PPU_KSTG_TCAM_ENTRIES_MAX_CNS;
            rc = cpssDxChPpuKstgTcamEntrySet(dev, kstgNum, profileNum, &kstgTcamEntry);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
        }

        /* check for out of range kstg number */
        kstgNum = CPSS_DXCH_PPU_KSTGS_MAX_CNS;
        profileNum = 0;
        rc = cpssDxChPpuKstgTcamEntrySet(dev, kstgNum, profileNum, &kstgTcamEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    profileNum = 1;
    kstgNum = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuKstgTcamEntrySet(dev, kstgNum, profileNum, &kstgTcamEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuKstgTcamEntrySet(dev, kstgNum, profileNum, &kstgTcamEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuKstgTcamEntryGet
(
    IN  GT_U8           devNum,
    IN  GT_U8           kstgNum,
    IN  GT_U8           profileNum,
    OUT CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC   *tcamEntryPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPpuKstgTcamEntryGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U8       profileNum = 0;
    GT_U8       kstgNum;
    CPSS_DXCH_PPU_KSTG_TCAM_ENTRY_STC  kstgTcamEntryGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(kstgNum = 0; kstgNum < CPSS_DXCH_PPU_KSTGS_MAX_CNS; kstgNum++)
        {
            for(profileNum = 0; profileNum < CPSS_DXCH_PPU_KSTG_TCAM_ENTRIES_MAX_CNS; profileNum++)
            {
                /* check for normal functionality */
                rc = cpssDxChPpuKstgTcamEntryGet(dev, kstgNum, profileNum, &kstgTcamEntryGet);
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, kstgNum);
            }

            profileNum = 0;

            /* check for null pointer */
            rc = cpssDxChPpuKstgTcamEntryGet(dev, kstgNum, profileNum, NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

            /* check for out of range pofile index */
            profileNum = CPSS_DXCH_PPU_KSTG_TCAM_ENTRIES_MAX_CNS;
            rc = cpssDxChPpuKstgTcamEntryGet(dev, kstgNum, profileNum, &kstgTcamEntryGet);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
        }

        /* check for out of range kstg number */
        kstgNum = CPSS_DXCH_PPU_KSTGS_MAX_CNS;
        profileNum = 0;
        rc = cpssDxChPpuKstgTcamEntryGet(dev, kstgNum, profileNum, &kstgTcamEntryGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    profileNum = 1;
    kstgNum = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuKstgTcamEntryGet(dev, kstgNum, profileNum, &kstgTcamEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuKstgTcamEntryGet(dev, kstgNum, profileNum, &kstgTcamEntryGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/* GT_STATUS cpssDxChPpuActionTableEntrySet
(
    IN  GT_U8           devNum,
    IN  GT_U8           kstgNum,
    IN  GT_U8           index,
    IN  CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
)*/
UTF_TEST_CASE_MAC(cpssDxChPpuActionTableEntrySet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC actionEntry;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC actionEntryGet;
    GT_U8       kstgNum;
    GT_U8       index;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(kstgNum = 0; kstgNum < CPSS_DXCH_PPU_KSTGS_MAX_CNS; kstgNum++)
        {
            for(index = 0; index < CPSS_DXCH_PPU_KSTG_ACTION_TABLE_ENTRIES_MAX_CNS; index++)
            {
                cpssOsMemSet(&actionEntry, 0, sizeof(CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC));
                cpssOsMemSet(&actionEntryGet, 0, sizeof(CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC));

                actionEntry.setConstNextShift = 0x4;
                actionEntry.rotActionEntry[2].srcRegSel = 1;
                actionEntry.rotActionEntry[2].funcSecondOperand = 2;

                /* check normal functionality */
                rc = cpssDxChPpuActionTableEntrySet(dev, kstgNum, index, &actionEntry);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, dev, kstgNum, index);

                rc = cpssDxChPpuActionTableEntryGet(dev, kstgNum, index, &actionEntryGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, dev, kstgNum, index);

                UTF_VERIFY_EQUAL3_PARAM_MAC(actionEntry.setConstNextShift, actionEntryGet.setConstNextShift, dev,
                                            kstgNum, index);

                UTF_VERIFY_EQUAL3_PARAM_MAC(actionEntry.rotActionEntry[2].srcRegSel,
                                        actionEntryGet.rotActionEntry[2].srcRegSel, dev, kstgNum, index);

                UTF_VERIFY_EQUAL3_PARAM_MAC(actionEntry.rotActionEntry[2].funcSecondOperand,
                        actionEntryGet.rotActionEntry[2].funcSecondOperand, dev, kstgNum, index);
            }
        }

        kstgNum = 1;
        index = 1;
        /* check for null pointer */
        rc = cpssDxChPpuActionTableEntrySet(dev, kstgNum, index, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check out of range for kstgNum */
        kstgNum = CPSS_DXCH_PPU_KSTGS_MAX_CNS;
        index = 1;
        rc = cpssDxChPpuActionTableEntrySet(dev, kstgNum, index, &actionEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, kstgNum);

        /* check out of range for index */
        kstgNum = 1;
        index = CPSS_DXCH_PPU_KSTG_ACTION_TABLE_ENTRIES_MAX_CNS;
        rc = cpssDxChPpuActionTableEntrySet(dev, kstgNum, index, &actionEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, index);

        /* check for sp bus overlap */
        kstgNum = 0;
        index = 4;
        cpssOsMemSet(&actionEntry, 0, sizeof(CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC));
        actionEntry.rotActionEntry[3].target          = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
        actionEntry.rotActionEntry[3].ldSpBusOffset   = 10;
        actionEntry.rotActionEntry[3].ldSpBusNumBytes = 3;

        actionEntry.rotActionEntry[4].target          = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
        actionEntry.rotActionEntry[4].ldSpBusOffset   = 12;
        actionEntry.rotActionEntry[4].ldSpBusNumBytes = 2;
        rc = cpssDxChPpuActionTableEntrySet(dev, kstgNum, index, &actionEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, rc);

        /* check for sp bus offset exceeds param */
        cpssOsMemSet(&actionEntry, 0, sizeof(CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC));
        actionEntry.rotActionEntry[0].target          = CPSS_DXCH_PPU_KSTG_ROT_TARGET_SEL_SP_BUS_E;
        actionEntry.rotActionEntry[0].ldSpBusOffset   = 19;
        actionEntry.rotActionEntry[0].ldSpBusNumBytes = 3;
        rc = cpssDxChPpuActionTableEntrySet(dev, kstgNum, index, &actionEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    kstgNum = 1;
    index = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuActionTableEntrySet(dev, kstgNum, index, &actionEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuActionTableEntrySet(dev, 0, 0, &actionEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/*GT_STATUS cpssDxChPpuActionTableEntryGet
(
    IN  GT_U8          devNum,
    IN  GT_U8          kstgNum,
    IN  GT_U8          index,
    OUT CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC *ppuActionEntryPtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuActionTableEntryGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    CPSS_DXCH_PPU_KSTG_ACTION_ENTRY_STC actionEntryGet;
    GT_U8       kstgNum;
    GT_U8       index;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(kstgNum = 0; kstgNum < CPSS_DXCH_PPU_KSTGS_MAX_CNS; kstgNum++)
        {
            for(index = 0; index < CPSS_DXCH_PPU_KSTG_ACTION_TABLE_ENTRIES_MAX_CNS; index++)
            {
                rc = cpssDxChPpuActionTableEntryGet(dev, kstgNum, index, &actionEntryGet);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, dev, kstgNum, index);
            }
        }

        kstgNum = 1;
        index = 1;
        /* check for null pointer */
        rc = cpssDxChPpuActionTableEntryGet(dev, kstgNum, index, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check out of range for kstgNum */
        kstgNum = CPSS_DXCH_PPU_KSTGS_MAX_CNS;
        index = 1;
        rc = cpssDxChPpuActionTableEntryGet(dev, kstgNum, index, &actionEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, kstgNum);

        /* check out of range for index */
        kstgNum = 1;
        index = CPSS_DXCH_PPU_KSTG_ACTION_TABLE_ENTRIES_MAX_CNS;
        rc = cpssDxChPpuActionTableEntryGet(dev, kstgNum, index, &actionEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, index);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    kstgNum = 1;
    index = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuActionTableEntryGet(dev, kstgNum, index, &actionEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    rc = cpssDxChPpuActionTableEntryGet(dev, kstgNum, index, &actionEntryGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuDauProfileEntrySet
(
    IN  GT_U32          devNum,
    IN  GT_U8           profileIndex,
    IN  CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuDauProfileEntrySet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U8       profileNum;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC  dauEntry;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC  dauEntryGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(profileNum = 0; profileNum < CPSS_DXCH_PPU_DAU_PROFILE_ENTRIES_MAX_CNS; profileNum++)
        {
            cpssOsMemSet(&dauEntry, 0, sizeof(CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC));
            cpssOsMemSet(&dauEntryGet, 0, sizeof(CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC));

            dauEntry.setDescBits[0].spByteNumBits      = 4;
            dauEntry.setDescBits[0].spByteSrcOffset    = 0;
            dauEntry.setDescBits[0].spByteWriteEnable  = GT_TRUE;
            dauEntry.setDescBits[0].spByteTargetOffset = 20;

            /* check for proper profile setting */
            rc = cpssDxChPpuDauProfileEntrySet(dev, profileNum, &dauEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, profileNum);

            rc = cpssDxChPpuDauProfileEntryGet(dev, profileNum, &dauEntryGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, profileNum);

            UTF_VERIFY_EQUAL2_PARAM_MAC(dauEntry.setDescBits[0].spByteNumBits      ,
                    dauEntryGet.setDescBits[0].spByteNumBits      , dev, profileNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(dauEntry.setDescBits[0].spByteSrcOffset    ,
                    dauEntryGet.setDescBits[0].spByteSrcOffset    , dev, profileNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(dauEntry.setDescBits[0].spByteWriteEnable  ,
                    dauEntryGet.setDescBits[0].spByteWriteEnable  , dev, profileNum);
            UTF_VERIFY_EQUAL2_PARAM_MAC(dauEntry.setDescBits[0].spByteTargetOffset ,
                    dauEntryGet.setDescBits[0].spByteTargetOffset , dev, profileNum);

            dauEntry.setDescBits[0].spByteTargetOffset = 3000;
            /* check for proper spByteTargetOffset setting */
            rc = cpssDxChPpuDauProfileEntrySet(dev, profileNum, &dauEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev, profileNum);

            dauEntry.setDescBits[0].spByteTargetOffset = 30;
            dauEntry.setDescBits[0].spByteSrcOffset = 21*8;
            /* check for proper spByteSrcOffset setting */
            rc = cpssDxChPpuDauProfileEntrySet(dev, profileNum, &dauEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev, profileNum);

            dauEntry.setDescBits[0].spByteSrcOffset = 64;
            dauEntry.setDescBits[0].spByteNumBits = 8;
            /* check for proper spByteNumBits setting */
            rc = cpssDxChPpuDauProfileEntrySet(dev, profileNum, &dauEntry);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev, profileNum);
        }

        profileNum = 0;

        /* check for null pointer */
        rc = cpssDxChPpuDauProfileEntrySet(dev, profileNum, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range pofile index */
        profileNum = CPSS_DXCH_PPU_DAU_PROFILE_ENTRIES_MAX_CNS;
        rc = cpssDxChPpuDauProfileEntrySet(dev, profileNum, &dauEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);

    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    profileNum = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuDauProfileEntrySet(dev, profileNum, &dauEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuDauProfileEntrySet(dev, profileNum, &dauEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/*GT_STATUS cpssDxChPpuDauProfileEntryGet
(
    IN  GT_U8           devNum,
    IN  GT_U8           profileIndex,
    OUT CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC *dauProfilePtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuDauProfileEntryGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U8       profileNum;
    CPSS_DXCH_PPU_DAU_PROFILE_ENTRY_STC  dauEntryGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(profileNum = 0; profileNum < CPSS_DXCH_PPU_DAU_PROFILE_ENTRIES_MAX_CNS; profileNum++)
        {
            /* check for proper profile setting */
            rc = cpssDxChPpuDauProfileEntryGet(dev, profileNum, &dauEntryGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, profileNum);
        }

        profileNum = 0;

        /* check for null pointer */
        rc = cpssDxChPpuDauProfileEntryGet(dev, profileNum, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range pofile index */
        profileNum = CPSS_DXCH_PPU_DAU_PROFILE_ENTRIES_MAX_CNS;
        rc = cpssDxChPpuDauProfileEntryGet(dev, profileNum, &dauEntryGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    profileNum = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuDauProfileEntryGet(dev, profileNum, &dauEntryGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuDauProfileEntryGet(dev, profileNum, &dauEntryGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/*GT_STATUS cpssDxChPpuDauProtectedWindowSet
(
    IN  GT_U32          devNum,
    IN  GT_U8           index,
    IN  CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *protWinPtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuDauProtectedWindowSet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U32      index;
    CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC  protWin;
    CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC  protWinGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0; index < CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_MAX_CNS; index++)
        {
            cpssOsMemSet(&protWin, 0, sizeof(CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC));
            cpssOsMemSet(&protWinGet, 0, sizeof(CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC));

            protWin.protWinEnable      = 1;
            protWin.protWinStartOffset = 0;
            protWin.protWinEndOffset   = 15;

            /* check for proper profile setting */
            rc = cpssDxChPpuDauProtectedWindowSet(dev, index, &protWin);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, index);

            rc = cpssDxChPpuDauProtectedWindowGet(dev, index, &protWinGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, index);

            UTF_VERIFY_EQUAL1_PARAM_MAC(protWin.protWinEnable     , protWinGet.protWinEnable     , dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(protWin.protWinStartOffset, protWinGet.protWinStartOffset, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(protWin.protWinEndOffset  , protWinGet.protWinEndOffset  , dev);
        }

        index = 0;
        /* check for null pointer */
        rc = cpssDxChPpuDauProtectedWindowSet(dev, index, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range pofile index */
        index = CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_MAX_CNS;
        rc = cpssDxChPpuDauProtectedWindowSet(dev, index, &protWin);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    index = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuDauProtectedWindowSet(dev, index, &protWin);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuDauProtectedWindowSet(dev, index, &protWin);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuDauProtectedWindowGet
(
    IN  GT_U32          devNum,
    IN  GT_U8           index,
    OUT CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC *protWinPtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuDauProtectedWindowGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U32      index;
    CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_STC  protWinGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0; index < CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_MAX_CNS; index++)
        {
            /* check for proper profile setting */
            rc = cpssDxChPpuDauProtectedWindowGet(dev, index, &protWinGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, index);
        }

        index = 0;
        /* check for null pointer */
        rc = cpssDxChPpuDauProtectedWindowSet(dev, index, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range pofile index */
        index = CPSS_DXCH_PPU_DAU_DATA_PROTECTED_WINDOW_MAX_CNS;
        rc = cpssDxChPpuDauProtectedWindowGet(dev, index, &protWinGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    index = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuDauProtectedWindowGet(dev, index, &protWinGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuDauProtectedWindowGet(dev, index, &protWinGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

}

/*GT_STATUS cpssDxChPpuErrorProfileSet
(
    IN  GT_U32                  devNum,
    IN  GT_U32                  profileNum,
    IN  CPSS_DXCH_PPU_ERROR_PROFILE_STC  *errorProfilePtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuErrorProfileSet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U32      index;
    CPSS_DXCH_PPU_ERROR_PROFILE_STC  errorProfile;
    CPSS_DXCH_PPU_ERROR_PROFILE_STC  errorProfileGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0; index < CPSS_DXCH_PPU_ERROR_PROFILES_MAX_CNS; index++)
        {
            cpssOsMemSet(&errorProfile, 0, sizeof(CPSS_DXCH_PPU_ERROR_PROFILE_STC));
            cpssOsMemSet(&errorProfileGet, 0, sizeof(CPSS_DXCH_PPU_ERROR_PROFILE_STC));

            errorProfile.errProfileField[1].writeEnable = GT_TRUE;
            errorProfile.errProfileField[1].targetOffset = 23;
            errorProfile.errProfileField[3].numBits = 5;
            errorProfile.errProfileField[3].errorDataField = 0x3f;

            /* check for proper profile setting */
            rc = cpssDxChPpuErrorProfileSet(dev, index, &errorProfile);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, index);

            rc = cpssDxChPpuErrorProfileGet(dev, index, &errorProfileGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, index);

            UTF_VERIFY_EQUAL1_PARAM_MAC(errorProfile.errProfileField[1].writeEnable,
                                        errorProfileGet.errProfileField[1].writeEnable, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(errorProfile.errProfileField[1].targetOffset,
                                        errorProfileGet.errProfileField[1].targetOffset, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(errorProfile.errProfileField[3].numBits,
                                        errorProfileGet.errProfileField[3].numBits, dev);
            UTF_VERIFY_EQUAL1_PARAM_MAC(errorProfile.errProfileField[3].errorDataField,
                                        errorProfileGet.errProfileField[3].errorDataField, dev);

        }

        index = 0;
        /* check for null pointer */
        rc = cpssDxChPpuErrorProfileSet(dev, index, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range index */
        index = CPSS_DXCH_PPU_ERROR_PROFILES_MAX_CNS;
        rc = cpssDxChPpuErrorProfileSet(dev, index, &errorProfile);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    index = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuErrorProfileSet(dev, index, &errorProfile);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuErrorProfileSet(dev, index, &errorProfile);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuErrorProfileGet
(
    IN  GT_U32                  devNum,
    IN  GT_U32                  profileNum,
    OUT CPSS_DXCH_PPU_ERROR_PROFILE_STC *errorProfilePtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuErrorProfileGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    GT_U32      index;
    CPSS_DXCH_PPU_ERROR_PROFILE_STC  errorProfileGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(index = 0; index < CPSS_DXCH_PPU_ERROR_PROFILES_MAX_CNS; index++)
        {
            cpssOsMemSet(&errorProfileGet, 0, sizeof(CPSS_DXCH_PPU_ERROR_PROFILE_STC));

            /* check for proper profile setting */
            rc = cpssDxChPpuErrorProfileGet(dev, index, &errorProfileGet);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, rc, dev, index);
        }

        index = 0;
        /* check for null pointer */
        rc = cpssDxChPpuErrorProfileGet(dev, index, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);

        /* check for out of range index */
        index = CPSS_DXCH_PPU_ERROR_PROFILES_MAX_CNS;
        rc = cpssDxChPpuErrorProfileGet(dev, index, &errorProfileGet);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OUT_OF_RANGE, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    index = 1;
    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuErrorProfileGet(dev, index, &errorProfileGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuErrorProfileGet(dev, index, &errorProfileGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuDebugCountersGet
(
    IN  GT_U8                                devNum,
    OUT CPSS_DXCH_PPU_DEBUG_COUNTERS_STC *dbgCountersPtr
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuDebugCountersGet)
{
    GT_STATUS   rc;
    GT_U8       dev;
    CPSS_DXCH_PPU_DEBUG_COUNTERS_STC  dbgCountersGet;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for proper register setting */
        rc = cpssDxChPpuDebugCountersGet(dev, &dbgCountersGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for null pointer */
        rc = cpssDxChPpuDebugCountersGet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, rc);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuDebugCountersGet(dev, &dbgCountersGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuDebugCountersGet(dev, &dbgCountersGet);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*GT_STATUS cpssDxChPpuDebugCountersClear
(
    IN  GT_U8                               devNum
);*/
UTF_TEST_CASE_MAC(cpssDxChPpuDebugCountersClear)
{
    GT_STATUS   rc;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* 1. Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for proper register setting */
        rc = cpssDxChPpuDebugCountersClear(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*
        2. For not-active devices and devices from non-applicable family
           check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_CPSS_PP_ALL_SIP6_10_CNS);

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChPpuDebugCountersClear(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChPpuDebugCountersClear(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChPpu suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChPpu)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuMaxLoopbackSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuMaxLoopbackGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuTtiActionProfileIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuTtiActionProfileIndexGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuSrcPortProfileIndexSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuSrcPortProfileIndexGet )
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuGlobalConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuGlobalConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuSpBusDefaultProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuSpBusDefaultProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuKstgKeyGenProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuKstgKeyGenProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuKstgTcamEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuKstgTcamEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuActionTableEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuActionTableEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuDauProfileEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuDauProfileEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuDauProtectedWindowSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuDauProtectedWindowGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuErrorProfileSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuErrorProfileGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuDebugCountersGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChPpuDebugCountersClear)
UTF_SUIT_END_TESTS_MAC(cpssDxChPpu)


