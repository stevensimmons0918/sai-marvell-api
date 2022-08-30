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
* @file cpssDxCh3pBrgVlanUT.c
*
* @brief Unit tests for cpssDxCh3pBrgVlan, that provides
* CPSS DxCh3p VLAN facility API
*
* @version   1.2
********************************************************************************
*/

/* includes */
#include <cpss/dxCh/dxCh3p/bridge/cpssDxCh3pBrgVlan.h>
#include <cpss/generic/bridge/private/prvCpssBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* defines */
/* Invalid enum */
#define BRG_VLAN_INVALID_ENUM_CNS     0x5AAAAAA5


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pBrgVlanVidToMllMappingSet
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    IN GT_U32                       mllPointer
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pBrgVlanVidToMllMappingSet)
{
/*
    ITERATE_DEVICES (DXCH3P)
    1.1. Call with vlanId[0 / 4095],
                   mllPointer[0 / 4095].
    Expected: GT_OK.
    1.2. Call cpssDxCh3pBrgVlanVidToMllTableMappingGet with not-NULL mllPointerPtr
                                                            and other param from 1.1.
    Expected: GT_OK and mllPointer as was set.
    1.3. Call with out of range vidx[4096]
                   and other param from 1.1.
    Expected: NOT GT_OK
    1.4. Call with out of range mllPointer [4096]
                   and other param from 1.1.
    Expected: NOT GT_OK
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U16      vlanId        = 0;
    GT_U32      mllPointer    = 0;
    GT_U32      mllPointerGet = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId[0 / 4095],
                           mllPointer[0 / 4095].
            Expected: GT_OK.
        */
        /* iterate with vlanId = 0 */
        vlanId     = 0;
        mllPointer = 0;

        st = cpssDxCh3pBrgVlanVidToMllMappingSet(dev, vlanId, mllPointer);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, mllPointer);

        /*
            1.2. Call cpssDxCh3pBrgVlanVidToMllTableMappingGet with not-NULL mllPointerPtr
                                                                    and other param from 1.1.
            Expected: GT_OK and mllPointer as was set.
        */
        st = cpssDxCh3pBrgVlanVidToMllTableMappingGet(dev, vlanId, &mllPointerGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgVlanVidToMllTableMappingGet: %d, %d",
                                     dev, vlanId);

        UTF_VERIFY_EQUAL2_STRING_MAC(mllPointer, mllPointerGet, "got anothe rmllPointer then was set: %d, %d",
                                     dev, vlanId);

        /* iterate with vlanId = 4095 */
        vlanId     = 4095;
        mllPointer = 4095;

        st = cpssDxCh3pBrgVlanVidToMllMappingSet(dev, vlanId, mllPointer);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, vlanId, mllPointer);

        /*
            1.2. Call cpssDxCh3pBrgVlanVidToMllTableMappingGet with not-NULL mllPointerPtr
                                                                    and other param from 1.1.
            Expected: GT_OK and mllPointer as was set.
        */
        st = cpssDxCh3pBrgVlanVidToMllTableMappingGet(dev, vlanId, &mllPointerGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgVlanVidToMllTableMappingGet: %d, %d",
                                     dev, vlanId);

        UTF_VERIFY_EQUAL2_STRING_MAC(mllPointer, mllPointerGet, "got anothe rmllPointer then was set: %d, %d",
                                     dev, vlanId);

        /*
            1.3. Call with out of range vlanId[4096]
                           and other param from 1.1.
            Expected: NOT GT_OK
        */
        vlanId = 4096;

        st = cpssDxCh3pBrgVlanVidToMllMappingSet(dev, vlanId, mllPointer);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

        vlanId = 0;

        /*
            1.4. Call with out of range mllPointer [4096]
                           and other param from 1.1.
            Expected: NOT GT_OK
        */
        mllPointer = 4096;

        st = cpssDxCh3pBrgVlanVidToMllMappingSet(dev, vlanId, mllPointer);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "%d, mllPointer = %d", dev, mllPointer);
    }

    vlanId     = 0;
    mllPointer = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pBrgVlanVidToMllMappingSet(dev, vlanId, mllPointer);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pBrgVlanVidToMllMappingSet(dev, vlanId, mllPointer);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxCh3pBrgVlanVidToMllTableMappingGet
(
    IN GT_U8                        devNum,
    IN GT_U16                       vlanId,
    OUT GT_U32                      *mllPointerPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxCh3pBrgVlanVidToMllTableMappingGet)
{
/*
    ITERATE_DEVICES (DXCH3P and above)
    1.1. Call with vlanId [0 / 4095]
                   and not NULL mllPointerPtr.
    Expected: GT_OK.
    1.2. Call with out of range vlanId [4096]
                   and other param from 1.1.
    Expected: NOT GT_OK
    1.3. Call with mllPointer [NULL]
                   and other param from 1.1.
    Expected: GT_BAD_PTR
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev;

    GT_U16      vlanId     = 0;
    GT_U32      mllPointer = 0;


    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with vlanId [0 / 4095]
                           and not NULL mllPointerPtr.
            Expected: GT_OK.
        */
        /* iterate with vlanId = 0 */
        vlanId = 0;

        st = cpssDxCh3pBrgVlanVidToMllTableMappingGet(dev, vlanId, &mllPointer);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

        /* iterate with vlanId = 4095 */
        vlanId = 4095;

        st = cpssDxCh3pBrgVlanVidToMllTableMappingGet(dev, vlanId, &mllPointer);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

        /*
            1.2. Call with out of range vlanId [4096]
                           and other param from 1.1.
            Expected: NOT GT_OK
        */
        vlanId = 4096;

        st = cpssDxCh3pBrgVlanVidToMllTableMappingGet(dev, vlanId, &mllPointer);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, vlanId);

        vlanId = 0;

        /*
            1.3. Call with mllPointer [NULL]
                           and other param from 1.1.
            Expected: GT_BAD_PTR
        */
        st = cpssDxCh3pBrgVlanVidToMllTableMappingGet(dev, vlanId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, mllPointerPtr = NULL", dev);
    }

    vlanId = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxCh3pBrgVlanVidToMllTableMappingGet(dev, vlanId, &mllPointer);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxCh3pBrgVlanVidToMllTableMappingGet(dev, vlanId, &mllPointer);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
    Test function to Fill VID-To-MLL table.
*/
UTF_TEST_CASE_MAC(cpssDxCh3pBrgVlanFillVidToMLLTable)
{
/*
    ITERATE_DEVICE (DxCh3p)
    1.1. Get table Size.
         Call cpssDxChCfgTableNumEntriesGet with table [CPSS_DXCH_CFG_TABLE_VLAN_E]
                                                 and non-NULL numEntriesPtr.
    Expected: GT_OK.
    1.2. Fill all entries in VID-To-MLL address table.
         Call cpssDxCh3pBrgVlanVidToMllMappingSet with vlanId[0..numEntries-1],
                                                       mllPointer[0].
    Expected: GT_OK.
    1.3. Try to write entry with index out of range.
         Call cpssDxCh3pBrgVlanVidToMllMappingSet with vlanId[numEntries].
    Expected: NOT GT_OK.
    1.4. Read all entries in VID-To-MLL address table and compare with original.
         Call cpssDxCh3pBrgVlanVidToMllTableMappingGet with not-NULL mllPointerPtr
                                                            and other param from 1.2.
    Expected: GT_OK and mllPointer as was set.
    1.5. Try to read entry with index out of range.
         Call cpssDxCh3pBrgVlanVidToMllTableMappingGet with vlanId[numEntries].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_U32      numEntries = 0;
    GT_U16      iTemp      = 0;

    GT_U32      mllPointer    = 0;
    GT_U32      mllPointerGet = 0;


    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* Fill the entry for VID-To-MLL address table */
    mllPointer = 0;

    /* prepare device iterator */
    st = prvUtfNextDeviceReset(&dev, UTF_DXCH3_FAMILY_SET_CNS);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Get table Size */
        st = cpssDxChCfgTableNumEntriesGet(dev, CPSS_DXCH_CFG_TABLE_VLAN_E, &numEntries);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChCfgTableNumEntriesGet: %d", dev);

        /* 1.2. Fill all entries in VID-To-MLL address table */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            st = cpssDxCh3pBrgVlanVidToMllMappingSet(dev, iTemp, mllPointer);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgVlanVidToMllMappingSet: %d, %d", dev, iTemp);
        }

        /* 1.3. Try to write entry with index out of range. */
        st = cpssDxCh3pBrgVlanVidToMllMappingSet(dev, (GT_U16)(numEntries), mllPointer);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgVlanVidToMllMappingSet: %d, %d", dev, numEntries);

        /* 1.4. Read all entries in VID-To-MLL address table and compare with original */
        for(iTemp = 0; iTemp < numEntries; ++iTemp)
        {
            st = cpssDxCh3pBrgVlanVidToMllTableMappingGet(dev, iTemp, &mllPointerGet);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgVlanVidToMllTableMappingGet: %d, %d", dev, iTemp);
            UTF_VERIFY_EQUAL1_STRING_MAC(mllPointer, mllPointerGet, "get another mllPointer than was set: %d, %d", dev);
        }

        /* 1.5. Try to read entry with index out of range. */
        st = cpssDxCh3pBrgVlanVidToMllTableMappingGet(dev, (GT_U16)(numEntries), &mllPointerGet);
        UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxCh3pBrgVlanVidToMllTableMappingGet: %d, %d", dev, numEntries);
    }
}
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of cpssDxCh3pBrgVlan suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxCh3pBrgVlan)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pBrgVlanVidToMllMappingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pBrgVlanVidToMllTableMappingGet)
    /* Test for Table */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxCh3pBrgVlanFillVidToMLLTable)
UTF_SUIT_END_TESTS_MAC(cpssDxCh3pBrgVlan)

