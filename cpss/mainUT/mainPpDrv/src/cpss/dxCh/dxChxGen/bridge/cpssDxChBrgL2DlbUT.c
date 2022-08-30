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
* @file cpssDxChBrgL2L2DlbUT.c
*
* @brief Unit tests for cpssDxChBrgL2Dlb, that provides
* L2 DLB facility CPSS DxCh implementation.
*
* @version   1
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Dlb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Ecmp.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>


/* Defines */

/* number entries in the ECMP table*/
#define    ECMP_TABLE_SIZE_CNS(_dev) (PRV_CPSS_DXCH_PP_MAC(_dev)->bridge.l2EcmpNumMembers - \
    PRV_CPSS_DXCH_PP_MAC(_dev)->bridge.l2EcmpNumMembers_usedByTrunk) /*actual is _8K but high part used for the trunk entries*/
#define    MAX_NUM_OF_PATH_CNS      16/* maximum number of paths */
#define    MAX_TIMESTAMP_VALUE      0x7FFFF
#define    TEST_BASE_EPORT_NUM      100
#define    MAX_TARGET_4_PATH        2048
#define    MAX_TARGET_8_PATH        1024
#define    MAX_TARGET_16_PATH       512

static GT_U32    path4WeightArr[4] = {25, 25, 25, 25};
static GT_U32    path4HwWeightArr[4] = {25, 50, 75, 100};
static GT_U32    path8WeightArr[8] = {10, 12, 20, 10, 18, 10, 10, 10 };
static GT_U32    path8HwWeightArr[8] = {10, 22, 42, 52, 70, 80, 90, 100 };
static GT_U32    path16WeightArr[16] = {5, 6, 7, 8, 9, 10, 11, 12 ,
                                        2, 3, 4, 5, 4,  4,  5, 5};
static GT_U32    path16HwWeightArr[16] = {5, 11, 18, 26, 35, 45, 56, 68 ,
                                        70, 73, 77, 82, 86,  90,  95, 100};
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbEnableSet
(
    IN  GT_U8       devNum,
    IN GT_BOOL      enable
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbEnableSet)
{
/*
    1. Call with enable GT_TRUE/GT_FALSE
    Expected: GT_OK.
    2. For not active devices check that function returns non GT_OK.
    Expected: GT_OK .
    3.Call function with out of bound value for device id
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_BOOL     enable, enableGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

     /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable GT_TRUE/GT_FALSE
            Expected: GT_OK.
        */
        enable = GT_TRUE;

        st = cpssDxChBrgL2DlbEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        st = cpssDxChBrgL2DlbEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enableGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another value than was set: %d", dev);

        enable = GT_FALSE;
        st = cpssDxChBrgL2DlbEnableSet(dev, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enable);

        st = cpssDxChBrgL2DlbEnableGet(dev, &enableGet);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, enableGet);

        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
                       "get another value than was set: %d", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    enable = GT_FALSE;
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbEnableSet(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbEnableSet(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbEnableGet
(
    IN  GT_U8       devNum,
    OUT GT_BOOL     *enablePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbEnableGet)
{
/*
    1.1. Call with non null enable.
    Expected: GT_OK.
    1.2. Call with enable [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_BOOL     enable;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

     /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non null enablePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgL2DlbEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with enable[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgL2DlbEnableGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enable= NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbEnableGet(dev, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbEnableGet(dev, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbLttIndexBaseEportSet
(
    IN GT_U8            devNum,
    IN GT_PORT_NUM      dlbIndexBaseEport
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbLttIndexBaseEportSet)
{
/*
    ITERATE_DEVICES (SIP5)
    1.1. Call with dlbIndexBaseEport [0 / 32 / maxEPortNum]
    Expected: GT_OK.
    1.2. Call cpssDxChBrgL2DlbLttIndexBaseEportGet with not NULL dlbIndexBaseEportPtr.
    Expected: GT_OK and the same dlbIndexBaseEport.
    1.3. Call with out of range dlbIndexBaseEport [maxEPortNum + 1].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_PORT_NUM dlbIndexBaseEport    = 0,dlbIndexBaseEportGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with dlbIndexBaseEport [0 / 32 / maxEPortNum]
            Expected: GT_OK.
        */
        /* call with dlbIndexBaseEport = 0 */

        dlbIndexBaseEport = 0;

        st = cpssDxChBrgL2DlbLttIndexBaseEportSet(dev, dlbIndexBaseEport);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dlbIndexBaseEport);
        /*
            1.2. Call cpssDxChBrgL2DlbLttIndexBaseEportGet with not NULL dlbIndexBaseEportPtr.
            Expected: GT_OK and the same dlbIndexBaseEport.
        */
        st = cpssDxChBrgL2DlbLttIndexBaseEportGet(dev, &dlbIndexBaseEportGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2DlbLttIndexBaseEportGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(dlbIndexBaseEport, dlbIndexBaseEportGet,
                       "get another dlbIndexBaseEport than was set: %d", dev);

         /* call with dlbIndexBaseEport = 32 */
        dlbIndexBaseEport = 32;

        st = cpssDxChBrgL2DlbLttIndexBaseEportSet(dev, dlbIndexBaseEport);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dlbIndexBaseEport);

        /*
            1.2. Call cpssDxChBrgL2DlbLttIndexBaseEportGet with not NULL dlbIndexBaseEportPtr.
            Expected: GT_OK and the same dlbIndexBaseEport.
        */
        st = cpssDxChBrgL2DlbLttIndexBaseEportGet(dev, &dlbIndexBaseEportGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2DlbLttIndexBaseEportGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(dlbIndexBaseEport, dlbIndexBaseEportGet,
                       "get another dlbIndexBaseEport than was set: %d", dev);

        /* call with dlbIndexBaseEport = maxEPortNum */
        dlbIndexBaseEport = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(dev);

        st = cpssDxChBrgL2DlbLttIndexBaseEportSet(dev, dlbIndexBaseEport);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, dlbIndexBaseEport);

        /*
            1.2. Call cpssDxChBrgL2DlbLttIndexBaseEportGet with not NULL dlbIndexBaseEportPtr.
            Expected: GT_OK and the same dlbIndexBaseEport.
        */
        st = cpssDxChBrgL2DlbLttIndexBaseEportGet(dev, &dlbIndexBaseEportGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2DlbLttIndexBaseEportGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(dlbIndexBaseEport, dlbIndexBaseEportGet,
                       "get another dlbIndexBaseEport than was set: %d", dev);

        /*
            1.3. Call with out of range dlbIndexBaseEport [maxEPortNum + 1].
            Expected: NON GT_OK.
        */
        dlbIndexBaseEport = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(dev) + 1;

        st = cpssDxChBrgL2DlbLttIndexBaseEportSet(dev, dlbIndexBaseEport);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, dlbIndexBaseEport);

    }

    dlbIndexBaseEport = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbLttIndexBaseEportSet(dev, dlbIndexBaseEport);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbLttIndexBaseEportSet(dev, dlbIndexBaseEport);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbLttIndexBaseEportGet
(
    IN GT_U8            devNum,
    OUT GT_PORT_NUM     *dlbIndexBaseEportPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbLttIndexBaseEportGet)
{

/*
    1.1. Call with non null dlbIndexBaseEportPtr.
    Expected: GT_OK.
    1.2. Call with dlbIndexBaseEportPtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_U32      dlbIndexBaseEport = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non null dlbIndexBaseEportPtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgL2DlbLttIndexBaseEportGet(dev, &dlbIndexBaseEport);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with dlbIndexBaseEportPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgL2DlbLttIndexBaseEportGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dlbIndexBaseEportPtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbLttIndexBaseEportGet(dev, &dlbIndexBaseEport);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbLttIndexBaseEportGet(dev, &dlbIndexBaseEport);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbTableEntrySet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    IN  CPSS_DXCH_BRG_L2_DLB_ENTRY_STC     *dlbEntryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbTableEntrySet)
{
/*
    1.1. Call with index [0 / maxIndex],
                   pathId [0 / max value],
                   lastSeenTimeStamp [0 / max value].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgL2DlbTableEntryGet with same index
                                             not NULL dlbEntryPtr,
    Expected: GT_OK and the same pathId, lastSeenTimeStamp.
    1.3. Call with out of range index [maxIndex + 1],
                   and other valid params.
    Expected: NON GT_OK.
    1.4. Call with out of range pathId [maxPathId + 1],
                   and other valid params.
    Expected: NON GT_OK.
    1.5. Call with out of range lastSeenTimeStamp [max value + 1],
                   and other valid params.
    Expected: NON GT_OK.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_U32          index           = 0;
    CPSS_DXCH_BRG_L2_DLB_ENTRY_STC dlbEntry = {0,0};
    CPSS_DXCH_BRG_L2_DLB_ENTRY_STC dlbEntryGet = {0,0};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0 / maxIndex],
                           pathId [0 / max value],
                           lastSeenTimeStamp [0 / max value].
            Expected: GT_OK.
        */
        /* call with index = 0 */
        index        = 0;
        dlbEntry.pathId = 0;
        dlbEntry.lastSeenTimeStamp = 0;

        st = cpssDxChBrgL2DlbTableEntrySet(dev, index, &dlbEntry);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, index, dlbEntry.pathId, dlbEntry.lastSeenTimeStamp);

        /*
            1.2. Call cpssDxChBrgL2DlbTableEntryGet with same index
                                                     not NULL dlbEntryPtr,
            Expected: GT_OK and the same pathId, lastSeenTimeStamp.
        */
        st = cpssDxChBrgL2DlbTableEntryGet(dev, index, &dlbEntryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2DlbTableEntryGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(dlbEntry.pathId, dlbEntryGet.pathId,
                       "get another pathId than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(dlbEntry.lastSeenTimeStamp, dlbEntryGet.lastSeenTimeStamp,
                       "get another lastSeenTimeStamp than was set: %d", dev);

        /* call with index = maxIndex */
        index        = ECMP_TABLE_SIZE_CNS(dev) - 1;
        dlbEntry.pathId  = MAX_NUM_OF_PATH_CNS - 1;;
        dlbEntry.lastSeenTimeStamp = MAX_TIMESTAMP_VALUE;

        st = cpssDxChBrgL2DlbTableEntrySet(dev, index, &dlbEntry);
        UTF_VERIFY_EQUAL4_PARAM_MAC(GT_OK, st, dev, index, dlbEntry.pathId, dlbEntry.lastSeenTimeStamp);

        /*
            1.2. Call cpssDxChBrgL2DlbTableEntryGet with same index
                                                     not NULL dlbEntryPtr,
            Expected: GT_OK and the same pathId, lastSeenTimeStamp.
        */
        st = cpssDxChBrgL2DlbTableEntryGet(dev, index, &dlbEntryGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2DlbTableEntryGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(dlbEntry.pathId, dlbEntryGet.pathId,
                       "get another pathId than was set: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(dlbEntry.lastSeenTimeStamp, dlbEntryGet.lastSeenTimeStamp,
                       "get another lastSeenTimeSTamp than was set: %d", dev);

        /*
            1.3. Call with out of range index [maxIndex + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */
        index = ECMP_TABLE_SIZE_CNS(dev);

        st = cpssDxChBrgL2DlbTableEntrySet(dev, index, &dlbEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        index = 0;

        /*
            1.4. Call with out of range pathId [maxValue + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */
        dlbEntry.pathId = MAX_NUM_OF_PATH_CNS + 1;

        st = cpssDxChBrgL2DlbTableEntrySet(dev, index, &dlbEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        dlbEntry.pathId = 0;

        /*
            1.5. Call with out of range lastSeenTimeStamp [maxValue + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */
        dlbEntry.lastSeenTimeStamp = MAX_TIMESTAMP_VALUE + 1;

        st = cpssDxChBrgL2DlbTableEntrySet(dev, index, &dlbEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        dlbEntry.lastSeenTimeStamp = 0;
    }

    index        = 0;
    dlbEntry.pathId  = 0;
    dlbEntry.lastSeenTimeStamp = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbTableEntrySet(dev, index, &dlbEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbTableEntrySet(dev, index, &dlbEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbTableEntryGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               index,
    OUT CPSS_DXCH_BRG_L2_DLB_ENTRY_STC     *dlbEntryPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbTableEntryGet)
{
/*
    1.1. Call with index [0 / maxIndex],
                   non NULL dlbEntryPtr,
    Expected: GT_OK.
    1.2. Call with out of range index [maxIndex + 1],
                   and other valid params.
    Expected: NON GT_OK.
    1.3. Call with dlbEntryPtr [NULL],
                   and other valid params.
    Expected: GT_BAD_PTR.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_U32                          index        = 0;
    CPSS_DXCH_BRG_L2_DLB_ENTRY_STC dlbEntry = {0,0};

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with index [0 / maxIndex],
                           non NULL dlbEntryPtr,
            Expected: GT_OK.
        */
        /* call with index = 0 */
        index = 0;

        st = cpssDxChBrgL2DlbTableEntryGet(dev, index, &dlbEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* call with index = maxIndex */
        index = ECMP_TABLE_SIZE_CNS(dev) - 1;

        st = cpssDxChBrgL2DlbTableEntryGet(dev, index, &dlbEntry);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /*
            1.2. Call with out of range index [maxIndex + 1],
                           and other valid params.
            Expected: NON GT_OK.
        */
        index = ECMP_TABLE_SIZE_CNS(dev);

        st = cpssDxChBrgL2DlbTableEntryGet(dev, index, &dlbEntry);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, index);

        /* restore values */
        index = 0;

        /*
            1.3. Call with dlbEntry [NULL],
                           and other valid params.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgL2DlbTableEntryGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, dlbEntry = NULL", dev);
    }

    index = 0;
    dlbEntry.pathId = 0;
    dlbEntry.lastSeenTimeStamp = 0;

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */

    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbTableEntryGet(dev, index, &dlbEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbTableEntryGet(dev, index, &dlbEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbPathUtilizationScaleModeSet
(
    IN GT_U8                                devNum,
    IN CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT  scaleMode
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbPathUtilizationScaleModeSet)
{
/*
    1.1. Call with selectionMode [CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E /
                                  CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E /
                                  CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E].
    Expected: GT_OK.
    1.2. Call cpssDxChBrgL2DlbPathUtilizationScaleModeGet with not NULL scaleModePtr.
    Expected: GT_OK and the same scaleMode.
    1.3. Call with scaleMode [wrong enum values].
    Expected: GT_BAD_PARAM.
*/

    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT scaleMode, scaleModeGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with scaleMode [CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E /
                                      CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E /
                                      CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E].
            Expected: GT_OK.
        */
        /* call with scaleMode = CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E */
        scaleMode = CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, scaleMode);

        /*
            1.2. Call cpssDxChBrgL2DlbPathUtilizationScaleModeGet with not NULL scaleModePtr.
            Expected: GT_OK and the same scaleMode.
        */
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeGet(dev, &scaleModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2DlbPathUtilizationScaleModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scaleMode, scaleModeGet,
                       "get another scaleMode than was set: %d", dev);

        /* call with scaleMode = CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E */
        scaleMode = CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, scaleMode);

        /*
            1.2. Call cpssDxChBrgL2DlbPathUtilizationScaleModeGet with not NULL scaleModePtr.
            Expected: GT_OK and the same scaleMode.
        */
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeGet(dev, &scaleModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2DlbPathUtilizationScaleModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scaleMode, scaleModeGet,
                       "get another scaleMode than was set: %d", dev);

        /* call with scaleMode = CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E */
        scaleMode = CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, scaleMode);

        /*
            1.2. Call cpssDxChBrgL2DlbPathUtilizationScaleModeGet with not NULL scaleModePtr.
            Expected: GT_OK and the same scaleMode.
        */
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeGet(dev, &scaleModeGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                       "cpssDxChBrgL2DlbPathUtilizationScaleModeGet: %d", dev);

        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(scaleMode, scaleModeGet,
                       "get another scaleMode than was set: %d", dev);
        /*
            1.3. Call with scaleMode [wrong enum values].
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChBrgL2DlbPathUtilizationScaleModeSet
                            (dev, scaleMode),
                            scaleMode);
    }
    scaleMode = CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E;
    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbPathUtilizationScaleModeGet
(
    IN GT_U8                                 devNum,
    OUT CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT  *scaleModePtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbPathUtilizationScaleModeGet)
{
/*
    1.1. Call with non null scaleModePtr.
    Expected: GT_OK.
    1.2. Call with scaleModePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT scaleMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non null scaleModePtr.
            Expected: GT_OK.
        */
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeGet(dev, &scaleMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with scaleModePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, scaleModePtr = NULL", dev);
    }

    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeGet(dev, &scaleMode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbPathUtilizationScaleModeGet(dev, &scaleMode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbMinimumFlowletIpgSet
(
    IN GT_U8    devNum,
    IN GT_U32   minimumIpg
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbMinimumFlowletIpgSet)
{
/*
    1.1. Call with minimumIpg[0 / 32 / maxValue].
    Expected: GT_OK.
    1.2 Call cpssDxChBrgL2DlbMinimumFlowletIpgGet with non Null minimumIpgPtr
    Expected: GT_OK , same minimumIpg value as set.
    1.3. Call with minimumIpg [maxValue + 1].
    Expected: GT_OUT_OF_RANGE.
*/
    GT_STATUS   st              = GT_OK;
    GT_U8       dev             = 0;
    GT_U32      minimumIpg, minimumIpgGet;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with minimumIpg[0 / 32 / maxValue].
            Expected: GT_OK.

        */
        /* Call with minimumIpg = 0*/
        minimumIpg = 0;
        st = cpssDxChBrgL2DlbMinimumFlowletIpgSet(dev, minimumIpg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.2  Call cpssDxChBrgL2DlbMinimumFlowletIpgGet
                      with non Null minimumIpgPtr
            Expected: GT_OK.
        */
        st = cpssDxChBrgL2DlbMinimumFlowletIpgGet(dev, &minimumIpgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(minimumIpg, minimumIpgGet,
                       "get another minimumIpg than was set: %d", dev);

        /* Call with minimumIpg = 32*/
        minimumIpg = 32;
        st = cpssDxChBrgL2DlbMinimumFlowletIpgSet(dev, minimumIpg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.2  Call cpssDxChBrgL2DlbMinimumFlowletIpgGet
                      with non Null minimumIpgPtr
            Expected: GT_OK.
        */
        st = cpssDxChBrgL2DlbMinimumFlowletIpgGet(dev, &minimumIpgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(minimumIpg, minimumIpgGet,
                       "get another minimumIpg than was set: %d", dev);

        /* Call with minimumIpg = maxValue*/
        minimumIpg = MAX_TIMESTAMP_VALUE;
        st = cpssDxChBrgL2DlbMinimumFlowletIpgSet(dev, minimumIpg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /*
            1.2  Call cpssDxChBrgL2DlbMinimumFlowletIpgGet
                      with non Null minimumIpgPtr
            Expected: GT_OK.
        */
        st = cpssDxChBrgL2DlbMinimumFlowletIpgGet(dev, &minimumIpgGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* verifying values */
        UTF_VERIFY_EQUAL1_STRING_MAC(minimumIpg, minimumIpgGet,
                       "get another minimumIpg than was set: %d", dev);

        /* 1.3. Call with minimumIpg [maxValue + 1].
           Expected: GT_BAD_PTR.
        */
        minimumIpg = MAX_TIMESTAMP_VALUE + 1;
        st = cpssDxChBrgL2DlbMinimumFlowletIpgSet(dev, minimumIpg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, dev);
    }
    minimumIpg = 0;
    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbMinimumFlowletIpgSet(dev, minimumIpg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbMinimumFlowletIpgSet(dev, minimumIpg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbMinimumFlowletIpgGet
(
    IN GT_U8                          devNum,
    OUT GT_U32                        *minimumIpgPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbMinimumFlowletIpgGet)
{
    /*
        1.1. Call with non NULL minimumIpgPtr .
        Expected: GT_OK.
        1.2. Call with NULL minimumIpgPtr.
        Expected: NON GT_OK.
    */

        GT_STATUS   st                  = GT_OK;
        GT_U8       dev                 = 0;
        GT_U32      minimumIpg;

        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

        /* 1. Go over all active devices. */
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            /*
                1.1. Call with non NULL minimumIpgPtr .
                Expected: GT_OK.
            */

            st = cpssDxChBrgL2DlbMinimumFlowletIpgGet(dev, &minimumIpg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            /*
                1.1. Call with NULL minimumIpgPtr .
                Expected: NON GT_OK.
            */
            st = cpssDxChBrgL2DlbMinimumFlowletIpgGet(dev, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, " %d", dev);
        }

        /* 2. For not active devices check that function returns non GT_OK.*/
        /* prepare device iterator */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

        /* Go over all non active devices. */
        while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
        {
            st = cpssDxChBrgL2DlbMinimumFlowletIpgGet(dev, &minimumIpg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
        }

        /* 3.Call function with out of bound value for device id */
        dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

        st = cpssDxChBrgL2DlbMinimumFlowletIpgGet(dev, &minimumIpg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbPathUtilizationTableEntrySet
(
    IN GT_U8                               devNum,
    IN GT_U32                              index,
    IN CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbEntryArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH]
)
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbPathUtilizationTableEntrySet)
{
    /*
        1.1. Call with index [0],
                       add dlbEntryArr for scaleMode[CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E\
                                                     CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E\
                                                     CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E]
             Expected: GT_OK.
        1.2. Call cpssDxChBrgL2DlbTableEntryGet with same index
                                             not NULL dlbEntryPtr,
             Expected: GT_OK and the same , targetEport, targetHwDevice, weight.
        1.3. Call with out of range index [maxIndex + 1],
                   and other valid params.
             Expected: NON GT_OK.
        1.4. Call with invalid weight [sum of path weight !=100],
                   and other valid params.
             Expected: NON GT_OK.
        1.5. Call with out of range targetEport [maxEPortNum + 1],
                   and other valid params.
             Expected: NON GT_OK.
        1.6. Call with out of range targetHwDevice [targetHwDevice + 1],
                   and other valid params.
             Expected: NON GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      noOfPaths = 0,i,index = 0;
    CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbPathEntryArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH];
    CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbPathEntryGetArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH];
    CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT scaleMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*1.1. Call with index [0],
                       add dlbEntryArr for scaleMode[CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E\
                                                     CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E\
                                                     CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E]
        */
        /* Call for scaleMode CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E */
        cpssOsMemSet(dlbPathEntryArr,0,sizeof(dlbPathEntryArr));
        cpssOsMemSet(dlbPathEntryGetArr,0,sizeof(dlbPathEntryArr));

        noOfPaths = 4;
        scaleMode = CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        for(i = 0; i < noOfPaths; i++)
        {
            dlbPathEntryArr[i].targetEport = TEST_BASE_EPORT_NUM + i;
            dlbPathEntryArr[i].targetHwDevice = 0;
            dlbPathEntryArr[i].weight = path4WeightArr[i];
        }
        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* 1.2. Call cpssDxChBrgL2DlbTableEntryGet with same index
                                             not NULL dlbEntryPtr,
             Expected: GT_OK and the same , targetEport, targetHwDevice, weight.*/
        st = cpssDxChBrgL2DlbPathUtilizationTableEntryGet(dev, index, dlbPathEntryGetArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        for(i = 0; i < noOfPaths;i++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(dlbPathEntryArr[i].targetEport, dlbPathEntryGetArr[i].targetEport,
                       "get another value than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(dlbPathEntryArr[i].targetHwDevice, dlbPathEntryGetArr[i].targetHwDevice,
                       "get another value than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(path4HwWeightArr[i], dlbPathEntryGetArr[i].weight,
                       "get another value than was set: %d", dev);
        }

        /* Call for scaleMode CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E */
        cpssOsMemSet(dlbPathEntryArr,0,sizeof(dlbPathEntryArr));
        cpssOsMemSet(dlbPathEntryGetArr,0,sizeof(dlbPathEntryArr));

        noOfPaths = 8;
        scaleMode = CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        for(i = 0; i < noOfPaths; i++)
        {
            dlbPathEntryArr[i].targetEport = TEST_BASE_EPORT_NUM + i;
            dlbPathEntryArr[i].targetHwDevice = 0;
            dlbPathEntryArr[i].weight = path8WeightArr[i];
        }

        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* 1.2. Call cpssDxChBrgL2DlbTableEntryGet with same index
                                             not NULL dlbEntryPtr,
             Expected: GT_OK and the same , targetEport, targetHwDevice, weight.*/
        st = cpssDxChBrgL2DlbPathUtilizationTableEntryGet(dev, index, dlbPathEntryGetArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        for(i = 0; i < noOfPaths;i++)
        {

            UTF_VERIFY_EQUAL1_STRING_MAC(dlbPathEntryArr[i].targetEport, dlbPathEntryGetArr[i].targetEport,
                       "get another value than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(dlbPathEntryArr[i].targetHwDevice, dlbPathEntryGetArr[i].targetHwDevice,
                       "get another value than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(path8HwWeightArr[i], dlbPathEntryGetArr[i].weight,
                       "get another value than was set: %d", dev);
        }

        cpssOsMemSet(dlbPathEntryArr,0,sizeof(dlbPathEntryArr));
        cpssOsMemSet(dlbPathEntryGetArr,0,sizeof(dlbPathEntryArr));

        /* Call for scaleMode CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E */
        noOfPaths = 16;
        scaleMode = CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        for(i = 0; i < noOfPaths; i++)
        {
            dlbPathEntryArr[i].targetEport = TEST_BASE_EPORT_NUM + i;
            dlbPathEntryArr[i].targetHwDevice = 0;
            dlbPathEntryArr[i].weight = path16WeightArr[i];
        }

        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        /* 1.2. Call cpssDxChBrgL2DlbTableEntryGet with same index
                                             not NULL dlbEntryPtr,
             Expected: GT_OK and the same , targetEport, targetHwDevice, weight.*/
        st = cpssDxChBrgL2DlbPathUtilizationTableEntryGet(dev, index, dlbPathEntryGetArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        for(i = 0; i < noOfPaths;i++)
        {

            UTF_VERIFY_EQUAL1_STRING_MAC(dlbPathEntryArr[i].targetEport, dlbPathEntryGetArr[i].targetEport,
                       "get another value than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(dlbPathEntryArr[i].targetHwDevice, dlbPathEntryGetArr[i].targetHwDevice,
                       "get another value than was set: %d", dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(path16HwWeightArr[i], dlbPathEntryGetArr[i].weight,
                       "get another value than was set: %d", dev);
        }

        /* 1.3 Call with invalid index */
        /* Call with index = MAX_TARGET_4_PATH;*/
        cpssOsMemSet(dlbPathEntryArr,0,sizeof(dlbPathEntryArr));

        noOfPaths = 4;
        scaleMode = CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E;
        index = MAX_TARGET_4_PATH;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        for(i = 0; i < noOfPaths; i++)
        {
            dlbPathEntryArr[i].targetEport = TEST_BASE_EPORT_NUM + i;
            dlbPathEntryArr[i].targetHwDevice = 0;
            dlbPathEntryArr[i].weight = path4WeightArr[i];
        }
        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Call with index = MAX_TARGET_8_PATH;*/
        cpssOsMemSet(dlbPathEntryArr,0,sizeof(dlbPathEntryArr));
        noOfPaths = 8;
        scaleMode = CPSS_DXCH_BRG_L2_DLB_8_PATHS_MODE_E;
        index = MAX_TARGET_8_PATH;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        for(i = 0; i < noOfPaths; i++)
        {
            dlbPathEntryArr[i].targetEport = TEST_BASE_EPORT_NUM + i;
            dlbPathEntryArr[i].targetHwDevice = 0;
            dlbPathEntryArr[i].weight = path8WeightArr[i];
        }
        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Call with index = MAX_TARGET_16_PATH;*/
        cpssOsMemSet(dlbPathEntryArr,0,sizeof(dlbPathEntryArr));
        noOfPaths = 16;
        scaleMode = CPSS_DXCH_BRG_L2_DLB_16_PATHS_MODE_E;
        index = MAX_TARGET_16_PATH;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        for(i = 0; i < noOfPaths; i++)
        {
            dlbPathEntryArr[i].targetEport = TEST_BASE_EPORT_NUM + i;
            dlbPathEntryArr[i].targetHwDevice = 0;
            dlbPathEntryArr[i].weight = path16WeightArr[i];
        }
        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        index = 0;
        /*1.4. Call with invalid weight [sum of path weight !=100],
                   and other valid params.
             Expected: NON GT_OK.*/
        /* Call with sum of weight < 100 */
        dlbPathEntryArr[0].weight = 0;
        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* Call with sum of weight > 100 */
        dlbPathEntryArr[0].weight = 200;
        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.5. Call with out of range targetEport [maxEPortNum + 1],
                   and other valid params.
             Expected: NON GT_OK.*/
        cpssOsMemSet(dlbPathEntryArr,0,sizeof(dlbPathEntryArr));
        noOfPaths = 4;
        scaleMode = CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        for(i = 0; i < noOfPaths; i++)
        {
            dlbPathEntryArr[i].targetEport = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(dev) + 1;
            dlbPathEntryArr[i].targetHwDevice = 0;
            dlbPathEntryArr[i].weight = path4WeightArr[i];
        }
        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        /* 1.6. Call with out of range targetHwDevice [targetHwDevice + 1],
                   and other valid params.
             Expected: NON GT_OK.*/
        cpssOsMemSet(dlbPathEntryArr,0,sizeof(dlbPathEntryArr));
        noOfPaths = 4;
        scaleMode = CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
        for(i = 0; i < noOfPaths; i++)
        {
            dlbPathEntryArr[i].targetEport = TEST_BASE_EPORT_NUM + 1;
            dlbPathEntryArr[i].targetHwDevice = PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(dev) + 1;
            dlbPathEntryArr[i].weight = path4WeightArr[i];
        }
        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }
    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    cpssOsMemSet(dlbPathEntryArr,0,sizeof(dlbPathEntryArr));
    noOfPaths = 4;
    scaleMode = CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E;
    st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);
    for(i = 0; i < noOfPaths; i++)
    {
        dlbPathEntryArr[i].targetEport = TEST_BASE_EPORT_NUM + 1;
        dlbPathEntryArr[i].targetHwDevice = 0;
        dlbPathEntryArr[i].weight = path4WeightArr[i];
    }
    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbPathUtilizationTableEntrySet(dev, index, dlbPathEntryArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbPathUtilizationTableEntryGet
(
    IN GT_U8                           devNum,
    IN GT_U32                          index,
    OUT CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbEntryArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH]
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbPathUtilizationTableEntryGet)
{

    /*
        1.1. Call with non NULL dlbEntryArr .
        Expected: GT_OK.
        1.2. Call with NULL dlbEntryArr.
        Expected: NON GT_OK.
    */
    CPSS_DXCH_BRG_L2_DLB_PATH_ENTRY_STC dlbPathEntryArr[CPSS_DXCH_BRG_L2_DLB_MAX_PATH];
    GT_STATUS   st                  = GT_OK;
    GT_U8       dev                 = 0;
    GT_U32      index;
    CPSS_DXCH_BRG_L2_DLB_SCALE_MODE_ENT scaleMode;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        index = 0;
        /*
            1.1. Call with non NULL dlbEntryArr .
            Expected: GT_OK.
        */

        scaleMode = CPSS_DXCH_BRG_L2_DLB_4_PATHS_MODE_E;
        st = cpssDxChBrgL2DlbPathUtilizationScaleModeSet(dev, scaleMode);

        st = cpssDxChBrgL2DlbPathUtilizationTableEntryGet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with NULL dlbPathEntryArr .
            Expected: NON GT_OK.
        */
        st = cpssDxChBrgL2DlbPathUtilizationTableEntryGet(dev, index, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, " %d", dev);
        /*
            1.3 Call with index [maxIndex+1]*/
        index = MAX_TARGET_4_PATH;
        st = cpssDxChBrgL2DlbPathUtilizationTableEntryGet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, " %d", dev);
    }
    index = 0;
    /* 2. For not active devices check that function returns non GT_OK.*/
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_CPSS_PP_ALL_SIP6_CNS);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbPathUtilizationTableEntryGet(dev, index, dlbPathEntryArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3.Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChBrgL2DlbPathUtilizationTableEntryGet(dev, index, dlbPathEntryArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbWaLttInfoSet
(
    IN GT_U8                                devNum,
    IN GT_U32                               lttIndex,
    IN GT_U32                               secondaryDlbBaseIndex
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbWaLttInfoSet)
{

    GT_STATUS   st                      = GT_OK;
    GT_U8       dev                     = 0;
    GT_U32      lttIndex                = 0;
    GT_U32      secondaryDlbBaseIndex   = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* Skip this case for GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Add WA entry (First node to the list) */
        lttIndex                = 20;
        secondaryDlbBaseIndex   = 1020;
        st = cpssDxChBrgL2DlbWaLttInfoSet(dev, lttIndex, secondaryDlbBaseIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChBrgL2DlbWaLttInfoSet: Failed");

        /* Add WA entry (Second node to the existing list) */
        lttIndex                = 30;
        secondaryDlbBaseIndex   = 1050;
        st = cpssDxChBrgL2DlbWaLttInfoSet(dev, lttIndex, secondaryDlbBaseIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChBrgL2DlbWaLttInfoSet: Failed");

        /* Update WA entry */
        lttIndex                = 20;
        secondaryDlbBaseIndex   = 1070;
        st = cpssDxChBrgL2DlbWaLttInfoSet(dev, lttIndex, secondaryDlbBaseIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChBrgL2DlbWaLttInfoSet: Failed");

        /* Update WA entry */
        lttIndex                = 30;
        secondaryDlbBaseIndex   = 1100;
        st = cpssDxChBrgL2DlbWaLttInfoSet(dev, lttIndex, secondaryDlbBaseIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChBrgL2DlbWaLttInfoSet: Failed");
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    secondaryDlbBaseIndex = 0;
    lttIndex = 0;

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbWaLttInfoSet(dev, lttIndex, secondaryDlbBaseIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChBrgL2DlbWaLttInfoSet(dev, lttIndex, secondaryDlbBaseIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChBrgL2DlbWaExecute
(
    IN GT_U8                                devNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbWaExecute)
{

    GT_STATUS   st                      = GT_OK;
    GT_U8       dev                     = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* Skip this case for GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = cpssDxChBrgL2DlbWaExecute(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* Go over all non active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChBrgL2DlbWaExecute(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* Call function with out of bound value for device id */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChBrgL2DlbWaExecute(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

#define L2_DLB_LTT_BLOCK_SIZE_CNS       256
#define L2_DLB_LTT_ENTRY_CNS            100
#define L2_DLB_MAX_ECMP_CNS             4
#define L2_DLB_PRIMARY_BLOCK_SIZE       (L2_DLB_LTT_ENTRY_CNS * L2_DLB_MAX_ECMP_CNS)
static CPSS_DXCH_BRG_L2_DLB_ENTRY_STC   l2DlbEntryArr[L2_DLB_LTT_ENTRY_CNS][L2_DLB_MAX_ECMP_CNS];

static GT_STATUS prvcpssDxChBrgL2DlbLttEntryVerify
(
    IN GT_U8                                 devNum,
    IN GT_U32                                lttIndex,
    IN CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC  *ecmpLttInfoExpPtr
)
{
    GT_STATUS                               st;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC     ecmpLttInfoGet;

    st = cpssDxChBrgL2EcmpLttTableGet(devNum, lttIndex, &ecmpLttInfoGet);
    if(st != GT_OK)
    {
        PRV_UTF_LOG2_MAC("cpssDxChBrgL2EcmpLttTableGet: Failed, Device[%d], LttIndex[%d]", devNum, lttIndex);
        return st;
    }

    if(ecmpLttInfoExpPtr->ecmpStartIndex != ecmpLttInfoGet.ecmpStartIndex)
    {
        PRV_UTF_LOG4_MAC("Failed, ecmpStartIndex Mismatch[Exp-%d,Got-%d], Device[%d], LttIndex[%d]",
                ecmpLttInfoExpPtr->ecmpStartIndex, ecmpLttInfoGet.ecmpStartIndex, devNum, lttIndex);
        return GT_FAIL;
    }
    if(ecmpLttInfoExpPtr->ecmpNumOfPaths != ecmpLttInfoGet.ecmpNumOfPaths)
    {
        PRV_UTF_LOG4_MAC("Failed, ecmpNumOfPaths Mismatch[Exp-%d,Got-%d], Device[%d], LttIndex[%d]",
                ecmpLttInfoExpPtr->ecmpNumOfPaths, ecmpLttInfoGet.ecmpNumOfPaths, devNum, lttIndex);
        return GT_FAIL;
    }
    if(ecmpLttInfoExpPtr->ecmpEnable != ecmpLttInfoGet.ecmpEnable)
    {
        PRV_UTF_LOG2_MAC("Failed, ecmpEnable Mismatch, Device[%d], LttIndex[%d]", devNum, lttIndex);
        return GT_FAIL;
    }
    if(ecmpLttInfoExpPtr->ecmpRandomPathEnable != ecmpLttInfoGet.ecmpRandomPathEnable)
    {
        PRV_UTF_LOG2_MAC("Failed, ecmpRandomPathEnable Mismatch, Device[%d], LttIndex[%d]", devNum, lttIndex);
        return GT_FAIL;
    }
    return GT_OK;
}

/*----------------------------------------------------------------------------*/
/* Create primary and secondary region for LTT/DLB table.
 * Populate entries, Call WA Multiple times and verify the content of the region accordingly
*/
UTF_TEST_CASE_MAC(cpssDxChBrgL2DlbWaExecute_regionEntryVerification)
{
    GT_STATUS                           st;
    GT_U32                              lttIndex[L2_DLB_LTT_ENTRY_CNS];
    GT_U32                              secondaryDlbBaseIndex[L2_DLB_LTT_ENTRY_CNS];
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC ecmpLttInfo[L2_DLB_LTT_ENTRY_CNS];
    GT_U8                               dev                     = 0;
    GT_U32                              primaryLttBaseIndex;
    GT_U32                              ii, pathNumber;
    CPSS_DXCH_BRG_L2_ECMP_LTT_ENTRY_STC ecmpLttInfoExp;
    CPSS_DXCH_BRG_L2_DLB_ENTRY_STC      dlbEntryGet;
    CPSS_DXCH_IMPLEMENT_WA_ENT          waArr[1]                = {CPSS_DXCH_FALCON_DLB_TS_WRAP_WA_E};
    GT_U32                              additionalInfoBmpArr[1];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~UTF_FALCON_E);

    /* Skip this case for GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* first active device. */
    st = prvUtfNextDeviceGet(&dev, GT_TRUE);
    if(st != GT_OK)
    {
        SKIP_TEST_MAC
    }
    dev = 0;

    st = cpssDxChBrgL2DlbLttIndexBaseEportGet(dev, &primaryLttBaseIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChBrgL2DlbLttIndexBaseEportGet: Failed");

    /* WA Init */
    additionalInfoBmpArr[0] = primaryLttBaseIndex + L2_DLB_LTT_BLOCK_SIZE_CNS;
    st = cpssDxChHwPpImplementWaInit(dev, 1, &waArr[0], &additionalInfoBmpArr[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChHwPpImplementWaInit: Failed");

    /* Create 10 LTT entry in primary region and call WaLttInfoSet for these LTT index */
    for(ii = 0; ii <L2_DLB_LTT_ENTRY_CNS; ii++)
    {
        lttIndex[ii]                         = ((cpssOsRand() & 3) << 7) | ii;  /* 2Bits Random, 7Bits from iterator */
        ecmpLttInfo[ii].ecmpStartIndex       = (ii * L2_DLB_MAX_ECMP_CNS);
        ecmpLttInfo[ii].ecmpNumOfPaths       = (cpssOsRand() % L2_DLB_MAX_ECMP_CNS)+1;  /* 0 invalid*/
        ecmpLttInfo[ii].ecmpEnable           = GT_TRUE;
        ecmpLttInfo[ii].ecmpRandomPathEnable = GT_FALSE;
        secondaryDlbBaseIndex[ii]            = (ii * L2_DLB_MAX_ECMP_CNS) + L2_DLB_PRIMARY_BLOCK_SIZE;

        /* Add LTT entry to primary region */
        st = cpssDxChBrgL2EcmpLttTableSet(dev, lttIndex[ii], &ecmpLttInfo[ii]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgL2EcmpLttTableSet: Failed, index[%d]", ii);

        /* Add DLB entries to primary region */
        for(pathNumber = 0; pathNumber<ecmpLttInfo[ii].ecmpNumOfPaths; pathNumber++)
        {
            l2DlbEntryArr[ii][pathNumber].pathId             = ((cpssOsRand() & 0x3) << 2) | pathNumber; /* 2Bits Random, 2Bits from iterator, Range 0-15 */
            l2DlbEntryArr[ii][pathNumber].lastSeenTimeStamp  = cpssOsRand() & 0x7FFFF;
            st = cpssDxChBrgL2DlbTableEntrySet(dev,
                    ecmpLttInfo[ii].ecmpStartIndex+pathNumber,
                    &l2DlbEntryArr[ii][pathNumber]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgL2DlbTableEntrySet: Failed, index[%d], pathNumber[%d]",
                    ii, pathNumber);
        }

        /* Add WA entry for the above LTT entry */
        st = cpssDxChBrgL2DlbWaLttInfoSet(dev, lttIndex[ii], secondaryDlbBaseIndex[ii]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChBrgL2DlbWaLttInfoSet: Failed, Index[%d]", ii);

        /* Verify entry in LTT table secondary region got added by WA API (Considering primary base index as 0) */
        cpssOsMemCpy(&ecmpLttInfoExp, &ecmpLttInfo[ii], sizeof(ecmpLttInfoExp));
        ecmpLttInfoExp.ecmpStartIndex = secondaryDlbBaseIndex[ii];
        st = prvcpssDxChBrgL2DlbLttEntryVerify(dev, lttIndex[ii]+L2_DLB_LTT_BLOCK_SIZE_CNS, &ecmpLttInfoExp);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvcpssDxChBrgL2DlbLttEntryVerify: Failed, Index[%d]", ii);
    }

    /* Call Wa execute(First Time) & verify
     * LTT Entry            - Remain same
     * DLB Initial Primary  - Becomes current secondary
     * DLB Initial Secondary- Becomes current primary
     * */
    st = cpssDxChBrgL2DlbWaExecute(dev);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChBrgL2DlbWaExecute: Failed");
    for(ii = 0; ii <L2_DLB_LTT_ENTRY_CNS; ii++)
    {
        /* Verify entry in LTT table Initial primary region  */
        st = prvcpssDxChBrgL2DlbLttEntryVerify(dev, lttIndex[ii], &ecmpLttInfo[ii]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvcpssDxChBrgL2DlbLttEntryVerify: Failed, index[%d]", ii);

        /* Verify DLB entries in Initial primary region (last seen time will remain as it is) */
        for(pathNumber = 0; pathNumber<ecmpLttInfo[ii].ecmpNumOfPaths; pathNumber++)
        {
            st = cpssDxChBrgL2DlbTableEntryGet(dev,
                    ecmpLttInfo[ii].ecmpStartIndex+pathNumber,
                    &dlbEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgL2DlbTableEntrySet: Failed, index[%d], pathNumber[%d]",
                    ii, pathNumber);
            UTF_VERIFY_EQUAL2_STRING_MAC(dlbEntryGet.pathId, l2DlbEntryArr[ii][pathNumber].pathId,
                    "Path ID did not match, index[%d], pathNumber[%d]",
                    ii, pathNumber);
            UTF_VERIFY_EQUAL2_STRING_MAC(dlbEntryGet.lastSeenTimeStamp, l2DlbEntryArr[ii][pathNumber].lastSeenTimeStamp,
                    "Last Seen time did not match, index[%d], pathNumber[%d]",
                    ii, pathNumber);
        }

        /* Verify entry in LTT table Initial Secondary region  */
        cpssOsMemCpy(&ecmpLttInfoExp, &ecmpLttInfo[ii], sizeof(ecmpLttInfoExp));
        ecmpLttInfoExp.ecmpStartIndex = secondaryDlbBaseIndex[ii];
        st = prvcpssDxChBrgL2DlbLttEntryVerify(dev, lttIndex[ii]+L2_DLB_LTT_BLOCK_SIZE_CNS, &ecmpLttInfoExp);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvcpssDxChBrgL2DlbLttEntryVerify: Failed, index[%d]", ii);

        /* Verify DLB entries in Initial Secondary region (last seen time will be updated as 0 - As it is the current primary) */
        for(pathNumber = 0; pathNumber<ecmpLttInfoExp.ecmpNumOfPaths; pathNumber++)
        {
            st = cpssDxChBrgL2DlbTableEntryGet(dev,
                    ecmpLttInfoExp.ecmpStartIndex+pathNumber,
                    &dlbEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgL2DlbTableEntrySet: Failed, index[%d], pathNumber[%d]",
                    ii, pathNumber);
            UTF_VERIFY_EQUAL2_STRING_MAC(dlbEntryGet.pathId, l2DlbEntryArr[ii][pathNumber].pathId,
                    "Path ID did not match, index[%d], pathNumber[%d]",
                    ii, pathNumber);
            UTF_VERIFY_EQUAL2_STRING_MAC(dlbEntryGet.lastSeenTimeStamp, 0,
                    "Last Seen time did not match, index[%d], pathNumber[%d]",
                    ii, pathNumber);
        }
    }

    /* Call Wa execute(Second Time) & verify
     * DLB Initial Primary  - Becomes current primary
     * DLB Initial Secondary- Becomes current Secondary
     * */
    st = cpssDxChBrgL2DlbWaExecute(dev);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChBrgL2DlbWaExecute: Failed");
    for(ii = 0; ii <L2_DLB_LTT_ENTRY_CNS; ii++)
    {
        /* Verify entry in LTT table Initial primary region  */
        st = prvcpssDxChBrgL2DlbLttEntryVerify(dev, lttIndex[ii], &ecmpLttInfo[ii]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvcpssDxChBrgL2DlbLttEntryVerify: Failed, index[%d]", ii);

        /* Verify DLB entries in Initial primary region ((last seen time will be updated as 0 - As it is the current primary) */
        for(pathNumber = 0; pathNumber<ecmpLttInfo[ii].ecmpNumOfPaths; pathNumber++)
        {
            st = cpssDxChBrgL2DlbTableEntryGet(dev,
                    ecmpLttInfo[ii].ecmpStartIndex+pathNumber,
                    &dlbEntryGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                    "cpssDxChBrgL2DlbTableEntrySet: Failed, index[%d], pathNumber[%d]",
                    ii, pathNumber);
            UTF_VERIFY_EQUAL2_STRING_MAC(dlbEntryGet.pathId, l2DlbEntryArr[ii][pathNumber].pathId,
                    "Path ID did not match, index[%d], pathNumber[%d]",
                    ii, pathNumber);
            UTF_VERIFY_EQUAL2_STRING_MAC(dlbEntryGet.lastSeenTimeStamp, 0,
                    "Last Seen time did not match, index[%d], pathNumber[%d]",
                    ii, pathNumber);
        }
    }
}
/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChBrgL2Dlb suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChBrgL2Dlb)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbLttIndexBaseEportSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbLttIndexBaseEportGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbTableEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbTableEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbPathUtilizationScaleModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbPathUtilizationScaleModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbMinimumFlowletIpgSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbMinimumFlowletIpgGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbPathUtilizationTableEntrySet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbPathUtilizationTableEntryGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbWaExecute_regionEntryVerification)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbWaLttInfoSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChBrgL2DlbWaExecute)
UTF_SUIT_END_TESTS_MAC(cpssDxChBrgL2Dlb)
