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
* @file cpssDxChDiagUT.c
*
* @brief Unit tests for cpssDxChDiagUT, that provides
* CPSS DXCH Diagnostic API
*
* @version   66
********************************************************************************
*/
/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/generic/diag/private/prvCpssGenDiag.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/tcam/private/prvCpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChTables.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Valid port num value used for testing */
#define DIAG_VALID_PORTNUM_CNS       0

/* Invalid lane num used for testing */
#define DIAG_INVALID_LANENUM_CNS     4

#ifdef CPSS_LOG_ENABLE
extern char *  prvCpssLogEnum_CPSS_DXCH_UNIT_ENT[];
extern GT_U32  prvCpssLogEnum_size_CPSS_DXCH_UNIT_ENT;
#endif

#ifdef CPSS_LOG_ENABLE
extern PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[];
extern GT_U32  prvCpssLogEnum_size_CPSS_DXCH_TABLE_ENT;
#endif


static GT_STATUS prvCpssDxChDiagTableSkip
(
    IN   GT_U8                devNum,
    IN   CPSS_DXCH_TABLE_ENT  tableType
)
{

    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    devFamily = PRV_CPSS_PP_MAC(devNum)->devFamily;

    switch(devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:

            if(tableType == CPSS_DXCH_SIP5_TABLE_LPM_MEM_E || tableType == CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E)
            {
                return GT_FAIL;
            }
        break;

        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
            if(tableType == CPSS_DXCH_SIP5_20_TABLE_LPM_MEM_PIPE_1_E || tableType == CPSS_DXCH_SIP5_TABLE_LPM_MEM_E)
            {
                 return GT_FAIL;
            }

        break;

        case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
            if(tableType == CPSS_DXCH_TABLE_FDB_E || tableType == CPSS_DXCH_SIP5_TABLE_LPM_MEM_E || tableType == CPSS_DXCH_SIP5_TABLE_INGRESS_POLICER_1_COUNTING_E)
            {
                 return GT_FAIL;
            }
        break;

        case PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_CASES_MAC:
            if(tableType == CPSS_DXCH2_ROUTE_NEXT_HOP_AGE_E)
            {
                return GT_FAIL;
            }
        break;

        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
            if(tableType == CPSS_DXCH_SIP5_TABLE_LPM_MEM_E)
            {
                return GT_FAIL;
            }
        break;

        default:
        break;
    }
    return GT_OK;
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagAllMemTest
(
    IN GT_U8                     devNum,
    OUT GT_BOOL                  *testStatusPtr,
    OUT GT_U32                   *addrPtr,
    OUT GT_U32                   *readValPtr,
    OUT GT_U32                   *writeValPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagAllMemTest)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with testStatusPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with addrPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with readValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with writeValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     testStatus;
    GT_U32      addr;
    GT_U32      readVal;
    GT_U32      writeVal;

    PRV_TGF_SKIP_SIMULATION_FAILED_TEST_MAC(UTF_BOBCAT3_E, "JIRA-6750");

    /* Skip this test for GM - performance reason */
    GM_SKIP_TEST_DUE_TO_POOR_PERFORMANCE_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagAllMemTest(dev, &testStatus, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
            1.2. Call with testStatusPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllMemTest(dev, NULL, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with addrPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllMemTest(dev, &testStatus, NULL, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with readValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllMemTest(dev, &testStatus, &addr, NULL, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call with writeValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllMemTest(dev, &testStatus, &addr, &readVal, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        if(!prvUtfIsGmCompilation())
        {
            /* after messing with the memories ... synch shadow to HW state */
            prvCpssDxChDiagDataIntegrityShadowAndHwSynch(dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagAllMemTest(dev, &testStatus, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagAllMemTest(dev, &testStatus, &addr, &readVal, &writeVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagAllRegTest
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *testStatusPtr,
    OUT GT_U32  *badRegPtr,
    OUT GT_U32  *readValPtr,
    OUT GT_U32  *writeValPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagAllRegTest)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with testStatusPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.3. Call with badRegPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.4. Call with readValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with writeValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    GT_BOOL     testStatus;
    GT_U32      badReg   = 0;
    GT_U32      readVal  = 0;
    GT_U32      writeVal = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with non-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagAllRegTest(dev, &testStatus, &badReg, &readVal, &writeVal);
        if((st != GT_OK) || (testStatus != GT_TRUE))
        {
            PRV_UTF_LOG3_MAC("cpssDxChDiagAllRegTest: FAILED on register address[0x%8.8x] with writeVal[0x%8.8x] and readVal[0x%8.8x] \n",
                badReg,writeVal,readVal);
        }
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_TRUE, testStatus, dev);

        /*
            1.2. Call with testStatusPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllRegTest(dev, NULL, &badReg, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.3. Call with badRegPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllRegTest(dev, &testStatus, NULL, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.4. Call with readValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllRegTest(dev, &testStatus, &badReg, NULL, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call with writeValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagAllRegTest(dev, &testStatus, &badReg, &readVal, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagAllRegTest(dev, &testStatus, &badReg, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagAllRegTest(dev, &testStatus, &badReg, &readVal, &writeVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagMemRead
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    OUT GT_U32                        *dataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagMemRead)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with memType = [ CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                               CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                  offset [0], and non-NULL dataPtr.
    Expected: GT_OK.
    1.2. Call with wrong enum values memType  and
         other params same as in 1.1.
    Expected: not GT_OK.
    1.3. Call with dataPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_TYPE_ENT  memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
    GT_U32                     offset = 0;
    GT_U32                     data;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            1.1. Call with memType = [  CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                                        CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                          offset [0], and non-NULL dataPtr.
            Expected: GT_OK.
        */
        memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;

        st = cpssDxChDiagMemRead(dev, memType, offset, &data);
        if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily ||
            UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Lion2 does not have CPU access to buffer memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);


        memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;

        st = cpssDxChDiagMemRead(dev, memType, offset, &data);
        if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
            (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
        {
            /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

        /*
            1.2. Call with wrong enum values memType and other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemRead
                            (dev, memType, offset, &data),
                            memType);

        /*
            1.3. Call with dataPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagMemRead(dev, memType, offset, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagMemRead(dev, memType, offset, &data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagMemRead(dev, memType, offset, &data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagMemTest
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         startOffset,
    IN GT_U32                         size,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *addrPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagMemTest)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with  memType [CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                             CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                        startOffset [0],
                        size [_8K],
                        profile [CPSS_DIAG_TEST_RANDOM_E]
                    and non-NULL pointers.
    Expected: GT_OK.
    1.2. Call with wrong enum values memType and other params same as in 1.1.
    Expected: not GT_OK.
    1.3. Call with wrong enum values profile and other params same as in 1.1.
    Expected: not GT_OK.
    1.4. Call with testStatusPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.5. Call with addrPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.6. Call with readValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
    1.7. Call with writeValPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_TYPE_ENT  memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
    GT_U32                     startOffset = 0;
    GT_U32                     size = 0;
    CPSS_DIAG_TEST_PROFILE_ENT profile = CPSS_DIAG_TEST_RANDOM_E;
    GT_BOOL                    testStatus;
    GT_U32                     addr;
    GT_U32                     readVal;
    GT_U32                     writeVal;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);
        /*
            1.1. Call with memType = [ CPSS_DIAG_PP_MEM_BUFFER_DRAM_E,
                                       CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E /
                                       CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                          offset [0], and non-NULL dataPtr.
            Expected: GT_OK.
        */

        /*call with memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E*/
        memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
        size = _8K;

        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, &readVal, &writeVal);
        if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Lion2 does not have CPU access to buffer memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);


        /*call with memType = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E*/
        memType = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E;

        /* cheetah has entry size 12 bytes. Need to align tested size with 12 bytes */
        size = _4K;

        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

        /*call with memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E*/
        memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
        size = PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.fdb;

        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, &readVal, &writeVal);
        if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
            (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
        {
            /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

        /*
            1.2. Call with wrong enum values memType and other params same as in 1.1.
            Expected: not GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemTest
                            (dev, memType, startOffset, size, profile,
                             &testStatus, &addr, &readVal, &writeVal),
                            memType);

        /*
            1.3. Call with wrong enum values profile and other params same as in 1.1.
            Expected: not GT_OK.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemTest
                            (dev, memType, startOffset, size, profile,
                             &testStatus, &addr, &readVal, &writeVal),
                            profile);

        /*
            1.4. Call with testStatusPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 NULL, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.5. Call with addrPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, NULL, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.6. Call with readValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, NULL, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.7. Call with writeValPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, &readVal, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        if(!prvUtfIsGmCompilation())
        {
            /* after messing with the memories ... synch shadow to HW state */
            prvCpssDxChDiagDataIntegrityShadowAndHwSynch(dev);
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                                 &testStatus, &addr, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagMemTest(dev, memType, startOffset, size, profile,
                             &testStatus, &addr, &readVal, &writeVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagMemWrite
(
    IN GT_U8                          devNum,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagMemWrite)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Call with memType = [ CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                               CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                  offset [0] and data [0].
    Expected: GT_OK.
    1.2. Call with wrong enum values memType  and
         other params same as in 1.1.
    Expected: not GT_OK.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_TYPE_ENT  memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
    GT_U32                     offset = 0;
    GT_U32                     data = 0;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);
        /*
            1.1. Call with memType = [  CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                                        CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E ],
                  offset [0] and data[0].
            Expected: GT_OK.
        */
        memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;

        st = cpssDxChDiagMemWrite(dev, memType, offset, data);
        if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
        {
            /* Lion2 does not have CPU access to buffer memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

        memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;

        st = cpssDxChDiagMemWrite(dev, memType, offset, data);
        if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
            (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
        {
            /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
        }
        else
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

        /*
            1.2. Call with wrong enum values memType  and other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemWrite
                            (dev, memType, offset, data),
                            memType);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagMemWrite(dev, memType, offset, data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagMemWrite(dev, memType, offset, data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortCheckEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortCheckEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChDiagPrbsPortCheckEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E | UTF_AC5_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }

            }

            /* iterate with enable - GT_FALSE */
            enable = GT_FALSE;
            st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    /*
                        1.1.2. Call cpssDxChDiagPrbsPortCheckEnableGet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enableGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChDiagPrbsPortCheckEnableGet: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "got another enable then was set: %d, %d", dev, port);

                    /* iterate with enable - GT_TRUE */
                    enable = GT_TRUE;

                    st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                    /*
                        1.1.2. Call cpssDxChDiagPrbsPortCheckEnableGet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enableGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChDiagPrbsPortCheckEnableGet: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "got another enable then was set: %d, %d", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_FALSE;
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortCheckEnableSet(dev, port, lane, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortCheckEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortCheckEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E | UTF_AC5_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with not-NULL enable.
                Expected: GT_OK.
            */
            st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
            if(prvCpssDxChPortRemotePortCheck(dev,port))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
            }
            else
            {

                /* the feature supported for all Gig ports
                   and XG ports of CH3 and above devices */
                if( IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType)) )
                {
                    if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                    {
                        localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                        if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                            UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                        else
                            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                    else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
                    {
                        if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                        {
                            if(IS_PORT_XG_E(portType))
                            {
                                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                            }
                            else
                            {
                                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                            }
                        }
                        else
                        {
                            if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                            {
                                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                            }
                            else
                            {
                                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                            }
                        }
                    }
                    else
                            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                        1.1.2. Call enablePtr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortCheckEnableGet(dev, port, lane, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortCheckReadyGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    OUT GT_BOOL    *isReadyPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortCheckReadyGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL isReady.
    Expected: GT_OK.
    1.1.2. Call isReadyPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;
    GT_BOOL     isReady    = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            if( IS_PORT_GE_E(portType) )
            {
                /*
                    1.1.1. Call with not-NULL isReady.
                    Expected: GT_OK.
                */
                st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                        1.1.2. Call isReadyPtr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                                 "%d, %d, isReadyPtr = NULL", dev, port);
                }
            }
            else
            {
                /* not supported not GE_E ports*/
                st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, isReady);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortCheckReadyGet(dev, port, &isReady);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortGenerateEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortGenerateEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call cpssDxChDiagPrbsPortGenerateEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     enableGet = GT_FALSE;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E | UTF_AC5_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                         notSupported = GT_TRUE;
                    }
                }
            }

            /* iterate with enable - GT_FALSE */
            enable = GT_FALSE;
            st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, enable);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    /*
                        1.1.2. Call cpssDxChDiagPrbsPortGenerateEnableGet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enableGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChDiagPrbsPortGenerateEnableGet: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "got another enable then was set: %d, %d", dev, port);

                    /* iterate with enable - GT_TRUE */
                    enable = GT_TRUE;

                    st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);

                    /*
                        1.1.2. Call cpssDxChDiagPrbsPortGenerateEnableGet with not-NULL enablePtr.
                        Expected: GT_OK and the same enable as was set.
                    */
                    st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enableGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChDiagPrbsPortGenerateEnableGet: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                   "got another enable then was set: %d, %d", dev, port);

                }

            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
        }

        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    enable = GT_FALSE;
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortGenerateEnableSet(dev, port, lane, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortGenerateEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortGenerateEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E | UTF_AC5_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with not-NULL enable.
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                         notSupported = GT_TRUE;
                    }
                }
            }

            st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                        1.1.2. Call enablePtr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortGenerateEnableGet(dev, port, lane, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortStatusGet
(
    IN   GT_U8          devNum,
    IN   GT_U8          portNum,
    IN   GT_U32         laneNum,
    OUT  GT_BOOL        *checkerLockedPtr,
    OUT  GT_U32         *errorCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortStatusGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call checkerLockedPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call errorCntrPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     checkerLocked;
    GT_U32      errorCntr;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E | UTF_AC5_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with not-NULL pointers.
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                     notSupported = GT_TRUE;
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                         notSupported = GT_TRUE;
                    }
                }
            }
            st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                        1.1.2. Call checkerLockedPtr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, NULL, &errorCntr);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                                 "%d, %d, checkerLockedPtr = NULL", dev, port);

                    /*
                        1.1.2. Call errorCntr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                                 "%d, %d, errorCntr = NULL", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, checkerLocked);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortStatusGet(dev, port, lane, &checkerLocked, &errorCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortTransmitModeSet
(
    IN   GT_U8                    devNum,
    IN   GT_U8                    portNum,
    IN   GT_U32                   laneNum,
    IN   CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortTransmitModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with mode [CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChDiagPrbsPortTransmitModeGet with not-NULL modePtr.
    Expected: GT_OK and the same mode as was set.
    1.1.3. Call with wrong enum values mode.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT modeGet = 1;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E | UTF_AC5_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with mode [CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E].
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                         notSupported = GT_TRUE;
                    }
                }
            }

            mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E;

            st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

                    /*
                        1.1.2. Call cpssDxChDiagPrbsPortTransmitModeGet with not-NULL modePtr.
                        Expected: GT_OK and the same mode as was set.
                    */
                    st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &modeGet);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                "cpssDxChDiagPrbsPortTransmitModeGet: %d, %d", dev, port);
                    UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                                   "got another mode then was set: %d, %d", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
            }

            /*
                1.1.3. Call with wrong enum values mode.
                Expected: GT_BAD_PARAM.
            */

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( IS_PORT_GE_E(portType) ||
                (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType)) )
            {
                if (GT_FALSE == notSupported)
                {
                    UTF_ENUMS_CHECK_MAC(cpssDxChDiagPrbsPortTransmitModeSet
                                        (dev, port, lane, mode),
                                        mode);
                }
            }
        }

        mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortTransmitModeSet(dev, port, lane, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsPortTransmitModeGet
(
    IN   GT_U8                    devNum,
    IN   GT_U8                    portNum,
    IN   GT_U32                   laneNum,
    OUT  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsPortTransmitModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL mode.
    Expected: GT_OK.
    1.1.2. Call modePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode;
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E | UTF_AC5_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with not-NULL mode.
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if(PRV_CPSS_SIP_5_20_CHECK_MAC(dev))
                {
                    if(IS_PORT_XG_E(portType))
                    {
                        notSupported = GT_TRUE;
                    }
                }
                else
                {
                    if (prvUtfPortMacModuloCalc(dev, port, 2) && IS_PORT_XG_E(portType))
                    {
                         notSupported = GT_TRUE;
                    }
                }
            }
            st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);

            /* the feature supported for all Gig ports
               and XG ports of CH3 and above devices */
            if( (IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType))) &&
                (GT_FALSE == notSupported))
            {                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                    /*
                        1.1.2. Call modePtr[NULL].
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                                 "%d, %d, modePtr = NULL", dev, port);
                }
            }
            else
            {
                UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsPortTransmitModeGet(dev, port, lane, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagRegTest
(
    IN GT_U8                          devNum,
    IN GT_U32                         regAddr,
    IN GT_U32                         regMask,
    IN CPSS_DIAG_TEST_PROFILE_ENT     profile,
    OUT GT_BOOL                       *testStatusPtr,
    OUT GT_U32                        *readValPtr,
    OUT GT_U32                        *writeValPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagRegTest)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1. Call with not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call wrong enum values profile.
    Expected: GT_BAD_PARAM.
    1.1.3. Call testStatusPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call readValPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.5. Call writeValPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    GT_U32                         regAddr = 0;
    GT_U32                         regMask = 0;
    CPSS_DIAG_TEST_PROFILE_ENT     profile = CPSS_DIAG_TEST_RANDOM_E;
    GT_BOOL                        testStatus;
    GT_U32                         readVal = 0;
    GT_U32                         writeVal = 0;


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* register '0' not exists (make it 0 from MG unit) */
            regAddr = 0 + prvCpssDxChHwUnitBaseAddrGet(dev,PRV_CPSS_DXCH_UNIT_MG_E,NULL);
        }
        else
        {
            regAddr = 0;
        }

        /*
            1.1.1. Call with not-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, &readVal, &writeVal);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2. Call wrong enum values profile.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagRegTest
                            (dev, regAddr, regMask, profile, &testStatus, &readVal, &writeVal),
                            profile);

        /*
            1.1.3. Call testStatusPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                                 NULL, &readVal, &writeVal);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4. Call readValPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, NULL, &writeVal);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.5. Call writeValPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, &readVal, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                                 &testStatus, &readVal, &writeVal);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagRegTest(dev, regAddr, regMask, profile,
                             &testStatus, &readVal, &writeVal);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagRegsDump)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1. Call with not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call regAddrPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call regDataPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    GT_U32    regsNum = 0;
    GT_U32    offset = 0;
    GT_U32    regAddr= 0;
    GT_U32    regData[1] = { 0 };


    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with not-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagRegsDump(dev, &regsNum, offset, &regAddr, regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2. Call regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegsDump(dev, NULL, offset, &regAddr, regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.3. Call regAddr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegsDump(dev, &regsNum, offset, NULL, regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4. Call regData[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegsDump(dev, &regsNum, offset, &regAddr, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagRegsDump(dev, &regsNum, offset, &regAddr, regData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagRegsDump(dev, &regsNum, offset, &regAddr, regData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagRegsNumGet
(
    IN  GT_U8     devNum,
    OUT GT_U32    *regsNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagRegsNumGet)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1. Call with not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_U32      regsNum;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with not-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2. Call regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagRegsNumGet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagRegsNumGet(dev, &regsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsCyclicDataSet
(
    IN   GT_U8          devNum,
    IN   GT_U8          portNum,
    IN   GT_U32         laneNum,
    IN   GT_U32         cyclicDataArr[4]
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsCyclicDataSet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh3 and above)
    1.1.1. Call with correct params.
    Expected: GT_OK.
    1.1.2. Call cpssDxChDiagPrbsCyclicDataGet with the same params.
    Expected: GT_OK and the same cyclicDataArr as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_U8       i = 0;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane = 0;
    GT_U32      cyclicDataArr[4] = {0,1,2,3};
    GT_U32      cyclicDataArrGet[4] = {1,2,3,4};
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E | UTF_AC5_E, CPSS-6066);
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with correct params.
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if (prvUtfPortMacModuloCalc(dev, port, 2))
                {
                     notSupported = GT_TRUE;
                }
            }


            st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);

            if( IS_PORT_XG_E(portType) && (GT_FALSE == notSupported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                /*
                    1.1.2. Call cpssDxChDiagPrbsCyclicDataGet.
                    Expected: GT_OK and the same cyclicDataArr as was set.
                */
                st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArrGet);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                            "cpssDxChDiagPrbsCyclicDataGet: %d, %d", dev, port);

                for(i = 0; i < 4; i++)
                {
                    UTF_VERIFY_EQUAL2_STRING_MAC(cyclicDataArr[i], cyclicDataArrGet[i],
                          "got another cyclicDataArr then was set: %d, %d", dev, port);
                }
            }
            else
            {
                /* not supported not XG_E ports*/
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsCyclicDataSet(dev, port, lane, cyclicDataArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsCyclicDataGet
(
    IN   GT_U8          devNum,
    IN   GT_U8          portNum,
    IN   GT_U32         laneNum,
    OUT  GT_U32         cyclicDataArr[4]
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsCyclicDataGet)
{
/*
    ITERATE_DEVICES_PHY_CPU_PORT (DxCh3 and above)
    1.1.1. Call with correct params.
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = DIAG_VALID_PORTNUM_CNS;
    GT_U32      lane = 0;
    GT_U32      cyclicDataArr[4] = {0,1,2,3};
    GT_BOOL     notSupported;
    GT_PHYSICAL_PORT_NUM localPort;
    PRV_CPSS_PORT_TYPE_ENT  portType;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_XCAT3_E | UTF_AC5_E, CPSS-6066);
        /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            /*
                1.1.1. Call with correct params.
                Expected: GT_OK.
            */
            notSupported = GT_FALSE;
            if(PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(dev,port);
                if((localPort%2 != 0) && (localPort != 9) && (localPort != 11))
                    notSupported = GT_TRUE;
            }
            else if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
            {
                if (prvUtfPortMacModuloCalc(dev, port, 2))
                {
                     notSupported = GT_TRUE;
                }
            }

            st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);

            if( IS_PORT_XG_E(portType) && (GT_FALSE == notSupported))
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else
            {
                /* not supported other ports*/
                UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsCyclicDataGet(dev, port, lane, cyclicDataArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPhyRegRead
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPhyRegRead)
{
/*
    1.1.1. Call with data=NULL
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U32                          baseAddr = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel = 0;
    GT_U32                          smiRegOffset = 0;
    GT_U32                          phyAddr = 0;
    GT_U32                          offset = 0;
    GT_BOOL                         doByteSwap = GT_TRUE;

    st = cpssDxChDiagPhyRegRead(baseAddr, ifChannel, smiRegOffset,
                                  phyAddr, offset, NULL, doByteSwap);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPhyRegWrite
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN GT_U32                          smiRegOffset,
    IN GT_U32                          phyAddr,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPhyRegWrite)
{
/*
    1.1.1. Call with phyAddr = 33
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_BAD_PARAM;

    GT_U32                          baseAddr = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel = 0;
    GT_U32                          smiRegOffset = 0;
    GT_U32                          phyAddr = 33;
    GT_U32                          offset = 0;
    GT_U32                          data = 0;
    GT_BOOL                         doByteSwap = GT_TRUE;


    st = cpssDxChDiagPhyRegWrite(baseAddr, ifChannel, smiRegOffset,
                                  phyAddr, offset, data, doByteSwap);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagRegRead
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    OUT GT_U32                         *dataPtr,
    IN GT_BOOL                         doByteSwap
)
*/
static void ut_cpssDxChDiagRegRead(IN GT_U8 dev)
{
/*
    1.1.1. Call with dataPtr == NULL
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_UINTPTR                      baseAddr = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannelArr[] = {
            CPSS_CHANNEL_PCI_E,
            CPSS_CHANNEL_PEX_E,
            CPSS_CHANNEL_PEX_MBUS_E,

            CPSS_CHANNEL_LAST_E
            };
    CPSS_DIAG_PP_REG_TYPE_ENT       regType = 0;
    GT_U32                          offset = 0;
    GT_U32                          regsArray[] = {0,0,0,0xFFFFFFFF};
    GT_BOOL                         doByteSwap = GT_TRUE;
    GT_U32                          portGroupId = 0;
    GT_U32                          orig_regValue,regValueRead,newRegValueWrite;
    GT_U32                          ii,jj;
    GT_BOOL                         devExists = GT_FALSE;

    st = cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset,
                             NULL, doByteSwap);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

    if (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[dev]->mngInterfaceType == CPSS_CHANNEL_PEX_KERNEL_E)
    {
        PRV_UTF_LOG1_MAC("PEX_KERNEL channel connects device %d to CPU\n", dev);
        PRV_UTF_LOG0_MAC("Test skipped \n");
        SKIP_TEST_MAC;
    }

    devExists = (PRV_CPSS_IS_DEV_EXISTS_MAC(dev)) &&
                (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[dev] != NULL);
    if(!devExists)
    {
        /* not found our device ?! */
        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_FALSE, devExists);
        return;
    }

    if(CPSS_CHANNEL_PCI_E == PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[dev]->mngInterfaceType ||
       CPSS_CHANNEL_PEX_E == PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[dev]->mngInterfaceType ||
       CPSS_CHANNEL_PEX_MBUS_E == PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[dev]->mngInterfaceType)
    {
        CPSS_HW_INFO_STC *hwInfoPtr;

        PRV_UTF_LOG1_MAC("baseAddr [0x%8.8x] of device \n", baseAddr);

        UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(CPSS_PARAM_NOT_USED_CNS, baseAddr);

        hwInfoPtr = cpssDrvHwPpHwInfoStcPtrGet(dev, portGroupId);
        if (!hwInfoPtr)
        {
            return;
        }

        baseAddr = hwInfoPtr->resource.switching.start;
        if(baseAddr == CPSS_PARAM_NOT_USED_CNS || baseAddr == 0)
        {
            return;
        }

        regType = CPSS_DIAG_PP_REG_INTERNAL_E;

        /* test PCI mode */
        ifChannel = CPSS_CHANNEL_PCI_E;
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            /* appDemo provides 4M window for SIP5 devices. Therefore only new address completion
               should be used for SIP5 and above devices - PEX_MBUS. */
            ifChannel = CPSS_CHANNEL_PEX_MBUS_E;
        }

        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* register '0x50' not exists (make it 0 from MG unit) */
            offset = 0x50 + prvCpssDxChHwUnitBaseAddrGet(dev,PRV_CPSS_DXCH_UNIT_MG_E,NULL);
        }
        else
        {
            offset = 0x50;
        }

        PRV_UTF_LOG1_MAC("ifChannel = [0x%8.8x] \n", ifChannel);

        /* read register that hold the 0x11AB to understand the 'swap' value */
        doByteSwap = GT_FALSE;
        st = cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset,
                                 &regValueRead, doByteSwap);

        PRV_UTF_LOG1_MAC("regAddr = [0x%8.8x] \n", offset);
        PRV_UTF_LOG1_MAC("doByteSwap = [0x%8.8x] \n", doByteSwap);
        PRV_UTF_LOG1_MAC("regValueRead = [0x%8.8x] \n", regValueRead);
        PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
        if(regValueRead == 0x11AB)
        {
        }
        else
        if(regValueRead == 0xAB110000)
        {
            doByteSwap = GT_TRUE;
            st = cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset,
                                     &regValueRead, doByteSwap);
            PRV_UTF_LOG1_MAC("doByteSwap = [0x%8.8x] \n", doByteSwap);
            PRV_UTF_LOG1_MAC("regValueRead = [0x%8.8x] \n", regValueRead);
            PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
        }

        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);
        UTF_VERIFY_EQUAL0_PARAM_MAC(0x11AB , regValueRead);

        /* registers taken from cpssDxChDiagAllRegTest */
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /* the device not supports those SIP5 TXQ features */
            regsArray[0] = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(dev).deviceMapConfigs.localTrgPortMapOwnDevEn[0];
            regsArray[1] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->TXQ.sdq[0].global_config;
            regsArray[2] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->IPvX.IPv4GlobalCtrl.IPv4CtrlReg0;
        }
        else
        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE)
        {
            regsArray[0] = PRV_DXCH_REG1_UNIT_EGF_EFT_MAC(dev).deviceMapConfigs.localTrgPortMapOwnDevEn[0];
            regsArray[1] = PRV_DXCH_REG1_UNIT_TXQ_DQ_MAC(dev).scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg0[0];
            regsArray[2] = PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(dev)->IPvX.IPv4GlobalCtrl.IPv4CtrlReg0;
        }
        else
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
        {
            regsArray[0] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->txqVer1.egr.global.egressInterruptMask;
            regsArray[1] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->txqVer1.dq.shaper.tokenBucketUpdateRate;
            regsArray[2] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->ipRegs.routerGlobalReg;
        }
        else
        {
            regsArray[0] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->interrupts.txqIntMask;
            regsArray[1] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->bridgeRegs.egressFilter;
            regsArray[2] = PRV_CPSS_DXCH_DEV_REGS_MAC(dev)->egrTxQConf.txPortRegs[0].wrrWeights0;
        }

        for(jj = 0 ; regsArray[jj] != 0xFFFFFFFF ; jj++)
        {
            offset = regsArray[jj];
            PRV_UTF_LOG1_MAC("offset = [0x%8.8x] \n", offset);

            /* read register by 'regular driver' API */
            cpssDrvPpHwRegisterRead(dev,portGroupId,offset, &orig_regValue);
            PRV_UTF_LOG1_MAC("orig_regValue = [0x%8.8x] \n", orig_regValue);

            newRegValueWrite = 0xAABBCCDD + offset;
            PRV_UTF_LOG1_MAC("(by driver) newRegValueWrite = [0x%8.8x] \n", newRegValueWrite);
            /* write register by 'regular driver' API */
            cpssDrvPpHwRegisterWrite(dev,portGroupId,offset, newRegValueWrite);

            PRV_UTF_LOG1_MAC("doByteSwap = [0x%8.8x] \n", doByteSwap);

            for(ii = 0 ; ifChannelArr[ii] != CPSS_CHANNEL_LAST_E ; ii++)
            {
                ifChannel = ifChannelArr[ii];

                if(ifChannel == CPSS_CHANNEL_PEX_MBUS_E &&
                    !PRV_CPSS_SIP_5_CHECK_MAC(dev))
                {
                    continue;
                }

                if ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) == GT_TRUE) ||
                    (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev)))
                {
                    /* appDemo provides 4M window for SIP5 devices. Therefore only new address completion
                       should be used for SIP5 and above devices - PEX_MBUS. */
                    if(ifChannel != CPSS_CHANNEL_PEX_MBUS_E)
                    {
                        continue;
                    }
                }

                PRV_UTF_LOG1_MAC("ifChannel = [0x%8.8x] \n", ifChannel);

                regValueRead = 0;
                st = cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset,
                                         &regValueRead, doByteSwap);
                PRV_UTF_LOG1_MAC("(first)regValueRead = [0x%8.8x] \n", regValueRead);
                PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(newRegValueWrite , regValueRead);

                newRegValueWrite = 0xF55F11FF + offset + (ifChannel << 10);
                PRV_UTF_LOG1_MAC("(by diag)newRegValueWrite = [0x%8.8x] \n", newRegValueWrite);
                st = cpssDxChDiagRegWrite(baseAddr, ifChannel, regType, offset,
                                         newRegValueWrite, doByteSwap);
                PRV_UTF_LOG1_MAC("newRegValueWrite = [0x%8.8x] \n", newRegValueWrite);
                PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);

                regValueRead = 0;
                st = cpssDxChDiagRegRead(baseAddr, ifChannel, regType, offset,
                                         &regValueRead, doByteSwap);
                PRV_UTF_LOG1_MAC("(second)regValueRead = [0x%8.8x] \n", regValueRead);
                PRV_UTF_LOG1_MAC("st = [0x%8.8x] \n", st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK , st);
                UTF_VERIFY_EQUAL0_PARAM_MAC(newRegValueWrite , regValueRead);
            }

            /* restore register by 'regular driver' API */
            cpssDrvPpHwRegisterWrite(dev,portGroupId,offset, orig_regValue);
        }

    }


}

UTF_TEST_CASE_MAC(cpssDxChDiagRegRead)
{
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        ut_cpssDxChDiagRegRead(dev);
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagRegWrite
(
    IN GT_U32                          baseAddr,
    IN CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel,
    IN CPSS_DIAG_PP_REG_TYPE_ENT       regType,
    IN GT_U32                          offset,
    IN GT_U32                          data,
    IN GT_BOOL                         doByteSwap
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagRegWrite)
{
/*
    1.1.1. Call with ifChannel not valid
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_BAD_PARAM;

    GT_U32                          baseAddr = 0;
    CPSS_PP_INTERFACE_CHANNEL_ENT   ifChannel = 0x55555;
    CPSS_DIAG_PP_REG_TYPE_ENT       regType = 0;
    GT_U32                          offset = 0;
    GT_U32                          data = 0;
    GT_BOOL                         doByteSwap = GT_TRUE;


    st = cpssDxChDiagRegWrite(baseAddr, ifChannel, regType, offset, data, doByteSwap);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
}

static void prvUtfDxChPortSerdesNumGet(GT_U8 dev, GT_PHYSICAL_PORT_NUM port, GT_U32 *serdesNum)
{
    CPSS_PORT_INTERFACE_MODE_ENT   ifMode = CPSS_PORT_INTERFACE_MODE_NA_E;
    GT_STATUS st;
    GT_U32      startSerdes;
    GT_U32      maxLaneNum;

    st = cpssDxChPortInterfaceModeGet(dev, port, &ifMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChPortInterfaceModeGet: %d, %d",
                                 dev, port);

    if(PRV_CPSS_SIP_5_CHECK_MAC(dev) ||
       PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev) )
    {
        st = prvCpssDxChPortIfModeSerdesNumGet(dev, port, ifMode, &startSerdes, &maxLaneNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChPortIfModeSerdesNumGet: %d, %d", dev, port);

        *serdesNum = maxLaneNum;

        return;
    }

    switch (ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_QX_E:
        case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
            if((PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev)) &&
               (PRV_CPSS_DXCH_IS_FLEX_LINK_MAC(dev,port) == GT_TRUE))
            {   /* xCat Flex ports have 4 SERDESes even part of them are not used */
                *serdesNum = 4;
            }
            else
                *serdesNum = 1;
            break;
        case CPSS_PORT_INTERFACE_MODE_HX_E:
        case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
            *serdesNum = 2;
            break;
        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
            *serdesNum = 4;
            break;
        case CPSS_PORT_INTERFACE_MODE_XLG_E:
            *serdesNum = 8;
            break;
        default:
            *serdesNum = 0;
            break;
    }
}

extern GT_STATUS prvCpssLockLowLevelApiSet(GT_U8   devNum, GT_BOOL lockStatus);

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesTestEnableSet
    +
GT_STATUS cpssDxChDiagPrbsSerdesTransmitModeSet
    +
GT_STATUS cpssDxChDiagPrbsSerdesStatusGet
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesFunctionalityTest)
{
/*
    ITERATE_DEVICES_PHY_PORT (BobCat2)
    1.1.1. Configure SERDES loopback
    1.1.2. Configure SERDES PRBS transmit mode to MODE_PRBS7_E
    1.1.3. Enable SERDES PRBS feature
    1.1.4. Check status of RPBS
    Expected:   lock status = GT_TRUE
                pattern counter = not equal to 0
*/

    GT_U8       dev;

#ifndef ASIC_SIMULATION
    GT_STATUS   st   = GT_OK;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_U32      serdesNum;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;
    GT_BOOL     checkerLocked = GT_FALSE;
    GT_U32      errorCntr = 0;
    GT_U64      patternCntr;
    GT_BOOL     freePmLock = GT_FALSE;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;

    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
    cpssOsMemSet(&patternCntr, 0, sizeof(patternCntr));
#endif

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E |
                                           UTF_AC5_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E |
                                           UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E );

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    if (cpssDeviceRunCheck_onEmulator())
    {
        SKIP_TEST_MAC
    }

#ifndef ASIC_SIMULATION
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        devFamily = PRV_CPSS_PP_MAC(dev)->devFamily;

        switch( devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_FALCON_E:
            case CPSS_PP_FAMILY_DXCH_AC5P_E:
            case CPSS_PP_FAMILY_DXCH_AC5X_E:
            case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
                prvCpssLockLowLevelApiSet(dev,  GT_FALSE);
                freePmLock = GT_TRUE;
            break;

            default:
            break;
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while (GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                             dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d", dev, port);

            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            for (lane = 0; lane < serdesNum; lane++)
            {
                st = cpssDxChPortSerdesLoopbackModeSet(dev, port, (1<<lane), CPSS_DXCH_PORT_SERDES_LOOPBACK_ANALOG_TX2RX_E);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);

                /* Transmit mode should be set before enabling test mode */
                st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);

                if (portType != PRV_CPSS_PORT_FE_E)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }
                else /* FE port */
                {
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                                     dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                    }
                }

                /* Enable PRBS feature */
                enable = GT_TRUE;
                st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);

                if (portType != PRV_CPSS_PORT_FE_E)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }
                else /* FE port */
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                }

                /* Check that status is "LOCKED" and pattern counter was incremented */
                st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                        &checkerLocked, &errorCntr, &patternCntr);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                if(st == GT_OK)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_TRUE, checkerLocked, dev, port);
                    if(PRV_CPSS_SIP_5_15_CHECK_MAC(dev) && !PRV_CPSS_SIP_6_10_CHECK_MAC(dev))
                    {
                        /* BobK does not support counter. It always 0. */
                        UTF_VERIFY_EQUAL2_PARAM_MAC(0, patternCntr.l[0], dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(0, patternCntr.l[0], dev, port);
                    }
                }

                /* Disable PRBS feature */
                enable = GT_FALSE;
                st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);

                if (portType != PRV_CPSS_PORT_FE_E)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                }
                else /* FE port */
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                }

                /* Disable SERDES Loopback */
                st = cpssDxChPortSerdesLoopbackModeSet(dev, port, (1<<lane), CPSS_DXCH_PORT_SERDES_LOOPBACK_DISABLE_E);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);

            }
        }
        if (freePmLock)
        {
            prvCpssLockLowLevelApiSet(dev,  GT_TRUE);
        }
    }

#endif
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesStatusGet
(
    IN   GT_U8     devNum,
    IN   GT_U8     portNum,
    IN   GT_U32    laneNum,
    OUT  GT_BOOL  *lockedPtr,
    OUT  GT_U32   *errorCntrPtr,
    OUT  GT_U64   *patternCntrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesStatusGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (xCat and above)
    1.1.1. Call with laneNum [0 - 3] and not-NULL pointers.
    Expected: GT_OK.
    1.1.2. Call with wrong laneNum [DIAG_INVALID_LANENUM_CNS].
    Expected: NOT GT_OK for flexLink ports and GT_OK for other.
    1.1.3. Call  witn wrong checkerLockedPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4. Call witn wrong errorCntrPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.5. Call witn wrong patternCntrPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;

    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;
    GT_U32      lane      = 0;
    GT_BOOL     checkerLocked = GT_FALSE;
    GT_U32      errorCntr = 0;
    GT_U64      patternCntr;
    GT_U32      serdesNum;
    CPSS_PP_FAMILY_TYPE_ENT         devFamily;
    PRV_CPSS_PORT_TYPE_ENT          portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;
    GT_BOOL                         isFlex;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
    GT_U32                                      boardIdx;
    GT_U32                                      boardRevId;
    GT_U32                                      reloadEeprom;

    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_BOBCAT2_E | UTF_XCAT3_E | UTF_AC5_E, "JIRA-6749");

    CPSS_TBD_BOOKMARK_LION2
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E );

    prvWrAppInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

    /* skip mixed multidevice boards */
    if((boardIdx == 31) /* Lion2 + BC2 */)
    {
        SKIP_TEST_MAC;
    }

    cpssOsMemSet(&patternCntr, 0, sizeof(patternCntr));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC;
        if (cpssDeviceRunCheck_onEmulator())
        {
            SKIP_TEST_MAC
        }
        if(prvUtfIsGmCompilation())
        {
            break;
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            st = prvUtfPortIsFlexLinkGet(dev, port, &isFlex);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortIsFlexLinkGet: %d, %d",
                                         dev, port);

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);

            /*
                1.1.1. Call with laneNum [0 - 3] and not-NULL pointers.
                Expected: GT_OK.
            */
            for(lane = 0; lane < serdesNum; lane++)
            {
                st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                        &checkerLocked, &errorCntr, &patternCntr);
                if(portType != PRV_CPSS_PORT_FE_E)
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_INITIALIZED, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                }

            }

            /*
                1.1.2. Call with wrong laneNum [DIAG_INVALID_LANENUM_CNS].
                Expected: NOT GT_OK for flexLink ports and GT_OK for other.
            */
            lane = serdesNum;
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                    &checkerLocked, &errorCntr, &patternCntr);

            if(isFlex == GT_TRUE)
            {
                if(CPSS_PP_FAMILY_DXCH_LION2_E != devFamily)
                    UTF_VERIFY_NOT_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                else
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            }
            else if(portType == PRV_CPSS_PORT_GE_E)
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                }
            }
            else /*FE port */
            {
                UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, lane);
            }

            lane = 0;

            /*
                1.1.3. Call  witn wrong checkerLockedPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                                 NULL, &errorCntr, &patternCntr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, checkerLockedPtr = NULL", dev, port);

            /*
                1.1.4. Call witn wrong errorCntrPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                                 &checkerLocked, NULL, &patternCntr);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, errorCntr = NULL", dev, port);

            /*
                1.1.5. Call witn wrong patternCntr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                                 &checkerLocked, &errorCntr, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, patternCntr = NULL", dev, port);
        }

        lane = 0;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
            st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                        &checkerLocked, &errorCntr, &patternCntr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);
        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                    &checkerLocked, &errorCntr, &patternCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                    &checkerLocked, &errorCntr, &patternCntr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E );

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
        st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                    &checkerLocked, &errorCntr, &patternCntr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
    st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, GT_TRUE);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
    st = cpssDxChDiagPrbsSerdesStatusGet(dev, port, lane,
                                   &checkerLocked, &errorCntr, &patternCntr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesTestEnableSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    IN  GT_BOOL    enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesTestEnableSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (xCat and above)
    1.1.1. Call with laneNum [0 - 3] enable[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.1.2. Call with wrong laneNum [DIAG_INVALID_LANENUM_CNS].
    Expected: NOT GT_OK for flexLink ports and GT_OK for other.
    1.1.3. Call cpssDxChDiagPrbsSerdesTestEnableGet with not-NULL enablePtr.
    Expected: GT_OK and the same enable as was set.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;
    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_U32      serdesNum;
    GT_BOOL     enableGet = GT_FALSE;
    PRV_CPSS_PORT_TYPE_ENT          portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;

    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
CPSS_TBD_BOOKMARK_LION2
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC;
        if (cpssDeviceRunCheck_onEmulator())
        {
            SKIP_TEST_MAC
        }

        if(prvUtfIsGmCompilation())
        {
            break;
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortTypeGet: %d, %d",
                                         dev, port);

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);
            /*
                1.1.1. Call with enable[GT_FALSE / GT_TRUE].
                Expected: GT_OK.
            */
            for(lane = 0; lane < serdesNum; lane++)
            {
                /* Transmit mode should be set before enabling test mode */
                st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
                if(portType != PRV_CPSS_PORT_FE_E)
                {
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                    }
                }
                else /* FE port */
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                }

                for(enable = GT_FALSE; enable <= GT_TRUE; enable++)
                {
                    st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                    }
                    else
                    {
                        if(portType != PRV_CPSS_PORT_FE_E)
                        {
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, enable);
                        }
                        else /* FE port */
                        {
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                        }

                        /*
                            1.1.2. Call cpssDxChDiagPrbsSerdesTestEnableGet with not-NULL enablePtr.
                            Expected: GT_OK and the same enable as was set.
                        */
                        st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enableGet);

                        if(portType != PRV_CPSS_PORT_FE_E)
                        {
                            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                        "cpssDxChDiagPrbsSerdesTestEnableGet: %d, %d", dev, port);
#ifndef ASIC_SIMULATION
                            UTF_VERIFY_EQUAL2_STRING_MAC(enable, enableGet,
                                           "got another enable then was set: %d, %d", dev, port);
#endif
                        }
                        else /* FE port */
                        {
                            UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                        }
                    }
                }
            }
        }

        lane = 0;
        enable = GT_FALSE;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    lane = 0;
    enable = GT_FALSE;
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesTestEnableSet(dev, port, lane, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesTestEnableGet
(
    IN  GT_U8      devNum,
    IN  GT_U8      portNum,
    IN  GT_U32     laneNum,
    OUT GT_BOOL    *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesTestEnableGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (xCat and above)
    1.1.1. Call with lane [0 - 3] and not-NULL enable.
    Expected: GT_OK.
    1.1.2. Call enablePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane      = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_U32      serdesNum;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;

CPSS_TBD_BOOKMARK_LION2
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC;
        if (cpssDeviceRunCheck_onEmulator())
        {
            SKIP_TEST_MAC
        }

        if(prvUtfIsGmCompilation())
        {
            break;
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);
            /*
                1.1.1. Call with lane [0 - 3] and not-NULL enable.
                Expected: GT_OK.
            */
            for(lane = 0; lane < serdesNum; lane++)
            {
                st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);

                /* the feature supported for all Gig ports
                   and XG ports of CH3 and above devices */
                if( IS_PORT_GE_E(portType) ||
                    (PRV_CPSS_DXCH3_FAMILY_CHECK_MAC(dev) && IS_PORT_XG_E(portType)) )
                {
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

                        /*
                            1.1.2. Call enablePtr[NULL].
                            Expected: GT_BAD_PTR.
                        */
                        st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, NULL);
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "%d, %d, enablePtr = NULL", dev, port);
                    }
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port, enable);
                }
            }
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        lane = 0;
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesTestEnableGet(dev, port, lane, &enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesTransmitModeSet
(
    IN   GT_U8                    devNum,
    IN   GT_U8                    portNum,
    IN   GT_U32                   laneNum,
    IN   CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   mode
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesTransmitModeSet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with laneNum [0 - 3] and mode [CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E].
    Expected: GT_OK.
    1.1.2. Call cpssDxChDiagPrbsSerdesTransmitModeGet with not-NULL modePtr.
    Expected: GT_OK and the same mode as was set.
    1.1.3. Call with wrong laneNum [DIAG_INVALID_LANENUM_CNS].
    Expected: NOT GT_OK.
    1.1.4. Call with wrong mode.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with wrong mode [CPSS_DXCH_DIAG_TRANSMIT_MODE_REGULAR_E].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT modeGet = 1;
    GT_U32      serdesNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;
    GT_BOOL                         isFlex;

CPSS_TBD_BOOKMARK_LION2
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC;
        if (cpssDeviceRunCheck_onEmulator())
        {
            SKIP_TEST_MAC
        }

        if(prvUtfIsGmCompilation())
        {
            break;
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            st = prvUtfPortIsFlexLinkGet(dev, port, &isFlex);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortIsFlexLinkGet: %d, %d",
                                         dev, port);

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);
            /*
                1.1.1. Call with mode [CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E].
                Expected: GT_OK.
            */
            mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;

            for(lane = 0; lane < serdesNum; lane++)
            {
                st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);

                if(portType != PRV_CPSS_PORT_FE_E)
                {
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, mode);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, mode);

                        /*
                            1.1.2. Call cpssDxChDiagPrbsSerdesTransmitModeGet with not-NULL modePtr.
                            Expected: GT_OK and the same mode as was set.
                        */
                        st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &modeGet);

                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                    "cpssDxChDiagPrbsSerdesTransmitModeGet: %d, %d", dev, port);
                        UTF_VERIFY_EQUAL2_STRING_MAC(mode, modeGet,
                                       "got another mode then was set: %d, %d", dev, port);
                    }
                }
                else
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

            }

            /*
                1.1.3. Call with wrong laneNum[DIAG_INVALID_LANENUM_CNS].
                Expected: NOT GT_OK for flexLink ports and GT_OK for others.
            */

            lane = serdesNum;

            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);

            if(isFlex == GT_TRUE)
            {
                if(CPSS_PP_FAMILY_DXCH_LION2_E != devFamily)
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                else
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else if(portType == PRV_CPSS_PORT_GE_E)
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                }
            }
            else /* FE Port */
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }

            lane = 0;


            /*
                1.1.5. Call with wrong mode.
                Expected: GT_NOT_SUPPORTED.
            */

            for(mode = 0; mode < CPSS_DXCH_DIAG_TRANSMIT_MODE_MAX_E; mode++)
            {
                if((PRV_CPSS_SIP_5_CHECK_MAC(dev)) ||
                   (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev)) ||
                   (devFamily == CPSS_PP_FAMILY_DXCH_LION2_E))
                {
                    if(mode < CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E)
                    {
                        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
                        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, lane, mode);
                    }
                }
                else
                {
                    if((mode < CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E)
                        || (mode > CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS31_E))
                    {
                        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
                        UTF_VERIFY_NOT_EQUAL4_PARAM_MAC(GT_OK, st, dev, port, lane, mode);
                    }
                }

            }

            mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
        }

        lane = 0;
        mode = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPrbsSerdesTransmitModeGet
(
    IN   GT_U8                    devNum,
    IN   GT_U8                    portNum,
    IN   GT_U32                   laneNum,
    OUT  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT   *modePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesTransmitModeGet)
{
/*
    ITERATE_DEVICES_PHY_PORT (DxChx)
    1.1.1. Call with not-NULL mode.
    Expected: GT_OK.
    1.1.2. Call with wrong laneNum[DIAG_INVALID_LANENUM_CNS].
    Expected: NOT GT_OK for flexLink ports and GT_OK for others.
    1.1.3. Call modePtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;
    GT_PHYSICAL_PORT_NUM  port = DIAG_VALID_PORTNUM_CNS;

    GT_U32      lane = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT mode;
    GT_U32      serdesNum;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    PRV_CPSS_PORT_TYPE_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;
    GT_BOOL                         isFlex;

CPSS_TBD_BOOKMARK_LION2
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_LION2_E );

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        GM_NOT_SUPPORT_THIS_TEST_MAC;
        if (cpssDeviceRunCheck_onEmulator())
        {
            SKIP_TEST_MAC
        }

        if(prvUtfIsGmCompilation())
        {
            break;
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            st = prvUtfPortInterfaceModeGet(dev, port, &portIfMode);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortInterfaceModeGet: %d, %d",
                                         dev, port);

            if(portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                continue;
            }

            st = prvUtfPortIsFlexLinkGet(dev, port, &isFlex);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvUtfPortIsFlexLinkGet: %d, %d",
                                         dev, port);

            st = prvUtfPortTypeGet(dev, port, &portType);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);

            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);
            /*
                1.1.1. Call with not-NULL mode.
                Expected: GT_OK
            */
            for(lane = 0; lane < serdesNum; lane++)
            {
                st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E);
                if(portType != PRV_CPSS_PORT_FE_E)
                {
                    if(prvCpssDxChPortRemotePortCheck(dev,port))
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                    }
                }
                else /* FE Port */
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

                st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
                if(portType != PRV_CPSS_PORT_FE_E)
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }
                else /* FE Port */
                {
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                }

            }

            /*
                1.1.2. Call with wrong laneNum[DIAG_INVALID_LANENUM_CNS].
                Expected: NOT GT_OK for flexLink ports and GT_OK for others.
            */

            lane = serdesNum;
            st = cpssDxChDiagPrbsSerdesTransmitModeSet(dev, port, lane, CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E);
            if(isFlex == GT_TRUE)
            {
                if(CPSS_PP_FAMILY_DXCH_LION2_E != devFamily)
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                else
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else if(portType == PRV_CPSS_PORT_GE_E)
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }

            st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);

            if(isFlex == GT_TRUE)
            {
                if(CPSS_PP_FAMILY_DXCH_LION2_E != devFamily)
                    UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
                else
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, port);
            }
            else if(portType == PRV_CPSS_PORT_GE_E)
            {
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                }
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_SUPPORTED, st, dev, port);
            }

            lane = 0;

            /*
                1.1.2. Call with wrong modePtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "%d, %d, modePtr = NULL", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }

    port = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesTransmitModeGet(dev, port, lane, &mode);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagMemoryBistBlockStatusGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT  memBistType,
    IN  GT_U32                          blockIndex,
    OUT GT_BOOL                         *blockFixedPtr,
    OUT GT_U32                          *replacedIndexPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagMemoryBistBlockStatusGet)
{
/*
    ITERATE_DEVICES (DxCh3)
    1.1. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E /
                                CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                and blockIndex [0].
    Expected: GT_OK.
    1.2. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E],
                                and blockIndex [13].
    Expected: GT_OK.
    1.3. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                and blockIndex [19].
    Expected: GT_OK.
    1.4. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E],
                                and  out of range blockIndex [14].
    Expected: NOT GT_OK.
    1.5. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                and  out of range blockIndex [20].
    Expected: NOT GT_OK.
    1.6. Call with wrong enum values memBistType.
    Expected: GT_BAD_PARAM.
    1.7. Call with wrong blockFixedPtr [NULL].
    Expected: GT_BAD_PTR.
    1.8. Call with wrong replacedIndex [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_BIST_TYPE_ENT  memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;
    GT_U32                          blockIndex = 0;
    GT_BOOL                         blockFixed;
    GT_U32                          replacedIndex;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E /
                                        CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                        and blockIndex [0].
            Expected: GT_OK
        */
        blockIndex  = 0;

        /*call with memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E */
        memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        /*call with memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E */
        memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        /*
            1.2. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E],
                                        and blockIndex [13].
            Expected: GT_OK.
        */
        blockIndex  = 13;
        memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        blockIndex  = 0;

        /*
            1.3. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                        and blockIndex [19].
            Expected: GT_OK.
        */
        blockIndex  = 19;
        memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        blockIndex  = 0;

        /*
            1.4. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E],
                                        and  out of range blockIndex [14].
            Expected: NOT GT_OK.
        */
        blockIndex  = 14;
        memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        blockIndex  = 0;

        /*
            1.5. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                                        and  out of range blockIndex [20].
            Expected: NOT GT_OK.
        */
        blockIndex  = 20;
        memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E;

        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

        blockIndex  = 0;

        /*
            1.6. Call with wrong enum values memBistType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemoryBistBlockStatusGet
                            (dev, memBistType, blockIndex, &blockFixed, &replacedIndex),
                            memBistType);

        /*
            1.7. Call with wrong blockFixedPtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, NULL, &replacedIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        /*
            1.8. Call with wrong replacedIndex [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E | UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                    blockIndex, &blockFixed, &replacedIndex);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagMemoryBistBlockStatusGet(dev, memBistType,
                                blockIndex, &blockFixed, &replacedIndex);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagMemoryBistsRun
(
    IN  GT_U8                           devNum,
    IN  CPSS_DIAG_PP_MEM_BIST_TYPE_ENT  memBistType,
    IN  GT_U32                          timeOut,
    IN  GT_BOOL                         clearMemoryAfterTest
)
*/

/**
* @internal prvUtfDxChDiagTcamBistCheck function
* @endinternal
*
* @brief   Check that TCAM's content was changed by BIST:
*          After BIST X/Y bytes changed to be 0x55 or 0xAA
*          After Restore to default X/Y bytes changed to be 0xFF
*          Function checks first and last entries of each TCAM Bank
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X Aldrin2; Bobcat3.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat2; Lion; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] clearMemoryAfterTest     - GT_TRUE  - Clear memory after BIST.
*                                      GT_FALSE - Don't clear memory after BIST.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong parameter
* @retval GT_FAIL                  - on wrong TCAM content
*
*
*/
GT_STATUS prvUtfDxChDiagTcamBistCheck
(
    IN GT_U8 devNum,
    IN GT_BOOL clearMemoryAfterTest
)
{
    GT_U32                       tcamSize;
    GT_U32                       numOfTcamBanks;
    GT_U32                       ruleIndex, tcamTableEntryX, tcamTableEntryY;
    GT_U32                       ii, kk;
    GT_STATUS                    rc;
    GT_STATUS                    outRc = GT_OK;
    GT_U32                       portGroupId = 0;
    GT_U32                       entryDataX[3] = {0};
    GT_U32                       entryDataY[3] = {0};

    rc = cpssDxChCfgTableNumEntriesGet(devNum, CPSS_DXCH_CFG_TABLE_PCL_TCAM_E, &tcamSize);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* calculate number of TCAM banks  */
    numOfTcamBanks = tcamSize / CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS;

    PRV_UTF_LOG2_MAC("TCAM BIST Result check: banks %d, is default %d\n", numOfTcamBanks, clearMemoryAfterTest);

    for (ii = 0; ii < numOfTcamBanks; ii++)
    {
        /* check first and last entries of each bank */
        for (kk = 0; kk < 2; kk++)
        {
            ruleIndex = (ii % CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS)  + (ii / CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS) * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS +
                        (kk*(CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_FLOOR_CNS-CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS));

            rc = prvCpssDxChTcamRuleIndexToEntryNumber(devNum, ruleIndex, &tcamTableEntryX, &tcamTableEntryY);
            if (rc != GT_OK)
            {
                return rc;
            }

            rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                     tcamTableEntryX,
                                                     entryDataX);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc =  prvCpssDxChPortGroupReadTableEntry(devNum,
                                                     portGroupId,
                                                     CPSS_DXCH_SIP5_TABLE_TCAM_E,
                                                     tcamTableEntryY,
                                                     entryDataY);
            if(rc != GT_OK)
            {
                return rc;
            }

            if (clearMemoryAfterTest == GT_FALSE)
            {
                if (!((entryDataX[0] == 0xaaaaaaaa) && (entryDataX[1] == 0xaaaaaaaa) && (entryDataX[2] == 0x000aaaaa) &&
                      (entryDataY[0] == 0x55555555) && (entryDataY[1] == 0x55555555) && (entryDataY[2] == 0x00055555)))
                {
                    cpssOsPrintf(" Failed entry %d X %d Y %d\n", ruleIndex,tcamTableEntryX,tcamTableEntryY);
                    cpssOsPrintf("   X %08x %08x %08x\n", entryDataX[0],entryDataX[1],entryDataX[2]);
                    cpssOsPrintf("   Y %08x %08x %08x\n", entryDataY[0],entryDataY[1],entryDataY[2]);
                    outRc = GT_FAIL;
                }
            }
            else
            {
                if (!((entryDataX[0] == 0xffffffff) && (entryDataX[1] == 0xffffffff) && (entryDataX[2] == 0x000fffff) &&
                      (entryDataY[0] == 0xffffffff) && (entryDataY[1] == 0xffffffff) && (entryDataY[2] == 0x000fffff)))
                {
                    cpssOsPrintf(" Failed entry %d X %d Y %d\n", ruleIndex,tcamTableEntryX,tcamTableEntryY);
                    cpssOsPrintf("   X %08x %08x %08x\n", entryDataX[0],entryDataX[1],entryDataX[2]);
                    cpssOsPrintf("   Y %08x %08x %08x\n", entryDataY[0],entryDataY[1],entryDataY[2]);
                    outRc = GT_FAIL;
                }
            }
        }
    }

    return outRc;
}

UTF_TEST_CASE_MAC(cpssDxChDiagMemoryBistsRun)
{
/*
    ITERATE_DEVICES (DxCh3)
    1.1. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E /
                                CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                       timeOut [0 / 10]
                   and clearMemoryAfterTest[GT_FALSE / GT_TRUE].
    Expected: GT_OK.
    1.2. Call with wrong enum values memBistType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_BIST_TYPE_ENT  memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;
    CPSS_DIAG_PP_MEM_BIST_TEST_TYPE_ENT bistTestType = CPSS_DIAG_PP_MEM_BIST_PURE_MEMORY_TEST_E;
    GT_U32                          timeOut = 0;
    GT_BOOL                         clearMemoryAfterTest = GT_TRUE;
    GT_U32                               testsToRunBmpPtr = 0;
    GT_U32                               testsResultBmpPtr;

    /* TCAM BIST is not implemented in GM*/
    GM_NOT_SUPPORT_THIS_TEST_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E
        | UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with memBistType [CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E /
                                        CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E],
                               timeOut [0 / 10]
                               and clearMemoryAfterTest[GT_FALSE / GT_TRUE].
            Expected: GT_OK
        */

        /* call with memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E */
        memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;
        timeOut = 1000;
        clearMemoryAfterTest = GT_FALSE;

        st = cpssDxChDiagMemoryBistsRun(dev, memBistType, bistTestType,
                            timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr);
#ifdef ASIC_SIMULATION
        st = (GT_TIMEOUT == st) ? GT_OK : st;
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);
#else
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);
        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            /* check TCAM content to see that BIST executed */
            st = prvUtfDxChDiagTcamBistCheck(dev, clearMemoryAfterTest);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

            clearMemoryAfterTest = GT_TRUE;

            st = cpssDxChDiagMemoryBistsRun(dev, memBistType, bistTestType,
                                timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);

            /* check TCAM content to see that BIST executed but TCAM restored to it's default */
            st = prvUtfDxChDiagTcamBistCheck(dev, clearMemoryAfterTest);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);
        }
#endif

        /* call with memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E */
        /* test without timeout leaves TCAM BIST in bad state that influences on following SIP_5 Data Integrity tests.
           Soft Reset solves the problem. Skip test case for SIP_5 when soft reset is not supported. */
        if((prvTgfResetModeGet() == GT_TRUE) || (!PRV_CPSS_SIP_5_CHECK_MAC(dev)))
        {
            memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;
            timeOut = 0;
            clearMemoryAfterTest = GT_FALSE;

            st = cpssDxChDiagMemoryBistsRun(dev, memBistType, bistTestType,
                                timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr);
            st = (GT_TIMEOUT == st) ? GT_OK : st;
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);
        }

        /* call with memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E */
        memBistType = CPSS_DIAG_PP_MEM_BIST_ROUTER_TCAM_E;
        timeOut = 10;
        clearMemoryAfterTest = GT_TRUE;

        st = cpssDxChDiagMemoryBistsRun(dev, memBistType, bistTestType,
                            timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr);
        if (PRV_CPSS_SIP_5_CHECK_MAC(dev))
        {
            /* there is no Router TCAM in SIP_5 */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev, memBistType);
        }
        else
        {
            st = (GT_TIMEOUT == st) ? GT_OK : st;
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memBistType);
        }

        /*
            1.2. Call with wrong enum values memBistType.
            Expected: GT_BAD_PARAM.
        */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagMemoryBistsRun
                            (dev, memBistType, bistTestType,
                            timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr),
                            memBistType);
    }

    memBistType = CPSS_DIAG_PP_MEM_BIST_PCL_TCAM_E;
    timeOut     = 0;
    clearMemoryAfterTest = GT_FALSE;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_XCAT_E | UTF_XCAT2_E);

    timeOut = 10;

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagMemoryBistsRun(dev, memBistType, bistTestType,
                            timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagMemoryBistsRun(dev, memBistType, bistTestType,
                            timeOut, clearMemoryAfterTest, &testsToRunBmpPtr, &testsResultBmpPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPortGroupMemWrite
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    IN GT_U32                         data
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPortGroupMemWrite)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1. Write / Read memory word for each port group,
         Call with memType = [CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                              CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E /
                              CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E ],
                  offset [0], and non-NULL dataPtr.
    Expected: GT_OK.
    1.2. Write memory word with CPSS_PORT_GROUP_UNAWARE_MODE_CNS
         and read for each port group.
    Expected: GT_OK and the same data.
    1.3. Write different memory words for each port groups. Read memory word with
        CPSS_PORT_GROUP_UNAWARE_MODE_CNS, it should be equal to one from
        port group 0.
    Expected: GT_OK and the same data.
    1.4. Write to specific port group and check that other port groups are not
    influenced.
    Expected: GT_OK and the same data.
    1.5. Call with wrong enum values memType and other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.6. Call with dataPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_TYPE_ENT  memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
    GT_U32                     offset = 0;
    GT_U32                     data = 0;
    GT_U32                     readData = 0;
    GT_PORT_GROUPS_BMP         portGroupsBmp = 1;
    CPSS_DIAG_PP_MEM_TYPE_ENT  memTypesArray[3];
    GT_U32                     portGroupId;
    GT_U32                     memTypeId;
    GT_U32                     memTypeLen = 3;
    GT_U32                     i;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily = CPSS_PP_FAMILY_CHEETAH_E;

    memTypesArray[0] = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
    memTypesArray[1] = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E;
    memTypesArray[2] = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);


    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /*
            1.1. Write / Read memory word for each port group,
                 Call with memType = [CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                                      CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E /
                                      CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E ],
                          offset [0], and non-NULL dataPtr.
            Expected: GT_OK.
        */

#ifdef ASIC_SIMULATION
        if (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev))
        {
            /* on those devices the 'buffer memory' memories not simulated although the device had it . */
            /* do not test CPSS_DIAG_PP_MEM_BUFFER_DRAM_E */
            memTypeLen = 2;
        }
#endif /* ASIC_SIMULATION*/

        for (memTypeId = 0; memTypeId < memTypeLen; memTypeId++)
        {
            memType = memTypesArray[memTypeId];

           PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
           {
               portGroupsBmp = (1 << portGroupId);

               if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
               {
                    if(devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                    {
                       /* In Lion Egress VLAN exist only in two port groups */
                        if ((0x3 & portGroupsBmp) == 0x0)
                        {
                            break;
                        }
                    }
                    else
                    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                    {
                        /* the EGF_SHT that hold (part of) the egress vlan table
                           is on 'even' port groups */
                        if ((0x55555555 & portGroupsBmp) == 0x0)
                        {
                            /* jump to next port group */
                            continue;
                        }
                    }
               }

                data = 0xAAAA5555;
                offset = 0x0;

                st = cpssDxChDiagPortGroupMemWrite(dev, portGroupsBmp,
                                                   memType, offset, data);
                if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                    (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
                {
                    /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                if ((CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) &&
                    (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E))
                {
                    /* Lion2 does not have CPU access to buffer memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                    st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp,
                                                       memType, offset, &readData);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                    UTF_VERIFY_EQUAL2_STRING_MAC(data, readData,
                       "The read data differs from written data: offset = %d, portGroupId = %d",
                                                 offset, portGroupId);
                }
                data = 0xFFFFFFFF;
                offset = 0x40;

                st = cpssDxChDiagPortGroupMemWrite(dev, portGroupsBmp,
                                                   memType, offset, data);
                if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                    (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
                {
                    /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                if ((CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) &&
                    (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E))
                {
                    /* Lion2 does not have CPU access to buffer memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                    st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp,
                                                       memType, offset, &readData);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                    UTF_VERIFY_EQUAL2_STRING_MAC(data, readData,
                      "The read data differs from written data: offset = %d, portGroupId = %d",
                                                 offset, portGroupId);
                }
           }
           PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        }

        /*
            1.2. Write memory word with CPSS_PORT_GROUP_UNAWARE_MODE_CNS
                 and read for each port group.
            Expected: GT_OK and the same data.
        */

        for (memTypeId = 0; memTypeId < memTypeLen; memTypeId++)
        {
            memType = memTypesArray[memTypeId];
            data = 0xAAAA5555;
            offset = 0x80;

            st = cpssDxChDiagPortGroupMemWrite(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                               memType, offset, data);
            if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
            {
                /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
            if ((CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) &&
                (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E))
            {
                /* Lion2 does not have CPU access to buffer memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                /* read for each port group */
               PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
               {
                    /* set next port */
                    portGroupsBmp = (1 << portGroupId);

                   if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
                   {
                        if(devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                        {
                           /* In Lion Egress VLAN exist only in two port groups */
                            if ((0x3 & portGroupsBmp) == 0x0)
                            {
                                break;
                            }
                        }
                        else
                        if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                        {
                            /* the EGF_SHT that hold (part of) the egress vlan table
                               is on 'even' port groups */
                            if ((0x55555555 & portGroupsBmp) == 0x0)
                            {
                                /* jump to next port group */
                                continue;
                            }
                        }
                   }

                    st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp,
                                                      memType, offset, &readData);
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                    UTF_VERIFY_EQUAL2_STRING_MAC(data, readData,
                     "The read data differs from written data: offset = %d, portGroupId = %d",
                                                 offset, portGroupId);
               }
               PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
            }
        }


        /*
            1.3. Write different memory words for each port groups. Read memory word with
                CPSS_PORT_GROUP_UNAWARE_MODE_CNS, it should be equal to one from
                port group 0.
                Note: In Lion B0, the Egress VLAN table is duplicated in core 0 and 1.
                Expected: GT_OK and the same data.
        */

        for (memTypeId = 0; memTypeId < memTypeLen; memTypeId++)
        {
            memType = memTypesArray[memTypeId];
            data = 0xAAAAAAAA;
            offset = 0x40;

            /* write to each port group */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
            {
                /* set next port */
                portGroupsBmp = (1 << portGroupId);

               if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
               {
                    if(devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                    {
                       /* In Lion Egress VLAN exist only in two port groups */
                        if ((0x3 & portGroupsBmp) == 0x0)
                        {
                            break;
                        }
                    }
                    else
                    if (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
                    {
                        /* the EGF_SHT that hold (part of) the egress vlan table
                           is on 'even' port groups */
                        if ((0x55555555 & portGroupsBmp) == 0x0)
                        {
                            /* jump to next port group */
                            continue;
                        }
                    }
               }

                st = cpssDxChDiagPortGroupMemWrite(dev, portGroupsBmp, memType, offset, data);
                if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                    (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
                {
                    /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                if ((CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) &&
                    (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E))
                {
                    /* Lion2 does not have CPU access to buffer memory */
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                }
                else
                {
                    UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);
                }

                data |= (0x55 << (portGroupId * 4));
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

            /* Verify that the read data equals to the read data from the port group 0 */
            st = cpssDxChDiagPortGroupMemRead(dev,
                    CPSS_PORT_GROUP_UNAWARE_MODE_CNS, memType, offset, &readData);
            if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
            {
                /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
            if ((CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)) &&
                (memType == CPSS_DIAG_PP_MEM_BUFFER_DRAM_E))
            {
                /* Lion2 does not have CPU access to buffer memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
            {
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                data = 0xAAAAAAAA;
                if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
                {
                    if(devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                    {
                        /* This will be the data written to portGroup 0 and 1*/
                        data = 0xAAAAAAFF;
                    }
                }

                UTF_VERIFY_EQUAL2_STRING_MAC(data, readData,
                 "The read data differs from written data: offset = %d, portGroupId = %d",
                                             offset, portGroupId);
            }
        }

        /*
            1.4. Write to specific port group and check that other port groups are not
            influenced.
            Expected: GT_OK and the same data.
        */
        memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
        offset  = 0x0;
        data    = 0x0;

        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
        {
            data = 0x0;

            /* write 0 to all port groups */
            st = cpssDxChDiagPortGroupMemWrite(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                               memType, offset, data);
            if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
            {
                /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* write memory word to specific port group id */

            portGroupsBmp = (1 << portGroupId);

            data = 0x5555AAAA;

            st = cpssDxChDiagPortGroupMemWrite(dev, portGroupsBmp,
                                               memType, offset, data);
            if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
            {
                /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /* read data from other port groups and check that it was not changed */
            PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,i)
            {
                if (portGroupId != i)
                {
                    portGroupsBmp = (1 << i);

                    st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp,
                                                      memType, offset, &data);
                    if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                        (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
                    {
                        /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
                    }
                    else
                    {
                        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

                        UTF_VERIFY_EQUAL2_STRING_MAC(0x0, data,
                         "The read data differs from written data: offset = %d, portGroupId = %d",
                                                     offset, i);
                    }
                }
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,i)
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /*
            1.5. Call with wrong enum values memType and other params same as in 1.1.
            Expected: GT_BAD_PARAM.
        */
        data = 0xAAAAAAAA;
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagPortGroupMemWrite
                            (dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, memType, offset, data),
                            memType);

        /*
            1.6. Call with dataPtr [NULL], other params same as in 1.1.
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagPortGroupMemRead(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          memType, offset, NULL);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPortGroupMemWrite(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          memType, offset, data);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    st = cpssDxChDiagPortGroupMemWrite(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                      memType, offset, data);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPortGroupMemRead
(
    IN GT_U8                          devNum,
    IN GT_PORT_GROUPS_BMP             portGroupsBmp,
    IN CPSS_DIAG_PP_MEM_TYPE_ENT      memType,
    IN GT_U32                         offset,
    OUT GT_U32                        *dataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPortGroupMemRead)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxChx)
    1.1.1.  Call with memType = [ CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                                CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E /
                                CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E ],
                  offset [0 / 100 / 1000], and non-NULL dataPtr.
    Expected: GT_OK.
    1.1.2. Call with wrong enum values memType and other params same as in 1.1.
    Expected: GT_BAD_PARAM.
    1.1.3. Call with dataPtr [NULL], other params same as in 1.1.
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DIAG_PP_MEM_TYPE_ENT  memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
    GT_U32                     offset = 0;
    GT_U32                     readData;
    GT_PORT_GROUPS_BMP         portGroupsBmp = 1;
    GT_U32                     portGroupId;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily = CPSS_PP_FAMILY_CHEETAH_E;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Getting device family */
        st = prvUtfDeviceFamilyGet(dev, &devFamily);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "prvUtfDeviceFamilyGet: %d", dev);

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1.  Call with memType = [CPSS_DIAG_PP_MEM_BUFFER_DRAM_E /
                                             CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E /
                                             CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E ],
                              offset [0 / 100 / 1000], and non-NULL dataPtr.
                Expected: GT_OK.
            */
            /* call with memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E */
            memType = CPSS_DIAG_PP_MEM_BUFFER_DRAM_E;
            offset = 0;

            st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp, memType, offset, &readData);
            if (CPSS_PP_FAMILY_DXCH_LION2_E == devFamily || UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                /* Lion2 does not have CPU access to buffer memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /*
                1.1.2. Call with wrong enum values memType and other params same as in 1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagPortGroupMemRead
                                (dev, portGroupsBmp, memType, offset, &readData),
                                memType);

            /* call with memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E */
            memType = CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E;
            offset = 100;

            st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp, memType, offset, &readData);
            if (memType == CPSS_DIAG_PP_MEM_MAC_TBL_MEM_E &&
                (UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
            {
                /* Bobcat2; Caelum; Bobcat3 does not have CPU access to FDB memory */
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, memType);
            }
            else
                UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /*
                1.1.2. Call with wrong enum values memType and other params same as in 1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagPortGroupMemRead
                                (dev, portGroupsBmp, memType, offset, &readData),
                                memType);

            /* call with memType = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E */
            memType = CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E;
            /* The offset inside the entry should be within 297 bits - 9 words
               For example offset 980 bytes is entry 15, with offset 20 bytes
               inside the entry */
            offset = 980;

            if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev))
            {
                /* NOTE: bobk not hold 9 words but only 5 !!! */
                offset = 64*15 + 16;/* 976 = entry 15 word 4 */
            }


            /* In Lion Egress VLAN exist only in two port groups */
           if (memType == CPSS_DIAG_PP_MEM_VLAN_TBL_MEM_E)
           {
                if(devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
                {
                   /* In Lion Egress VLAN exist only in two port groups */
                    if ((0x3 & portGroupsBmp) == 0x0)
                    {
                        continue;
                    }
                }
                else
                if ((UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev)))
                {
                    /* the EGF_SHT that hold (part of) the egress vlan table
                       is on 'even' port groups */
                    if ((0x55555555 & portGroupsBmp) == 0x0)
                    {
                        /* jump to next port group */
                        continue;
                    }
                }
           }

            st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp, memType, offset, &readData);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, memType);

            /*
                1.1.2. Call with wrong enum values memType and other params same as in 1.1.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagPortGroupMemRead
                                (dev, portGroupsBmp, memType, offset, &readData),
                                memType);

            /*
                1.1.3. Call with dataPtr [NULL], other params same as in 1.1.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp,
                                              memType, offset, NULL);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, memType);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next non-active port */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChDiagPortGroupMemRead(dev, portGroupsBmp, memType, offset, &readData);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
    }

    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPortGroupMemRead(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                          memType, offset, &readData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPortGroupMemRead(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                      memType, offset, &readData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPortGroupRegsNumGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *regsNumPtr
)
*/
/* UTF_TEST_CASE_MAC(cpssDxChDiagPortGroupRegsNumGet) */
GT_VOID cpssDxChDiagPortGroupRegsNumGetUT(GT_VOID)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1  Call for each port group cpssDxChDiagPortGroupRegsNumGet and then
           cpssDxChPortGroupDiagRegsDump with not-NULL pointers.
    Expected: GT_OK.
    1.1.2 Call for CPSS_PORT_GROUP_UNAWARE_MODE_CNS cpssDxChDiagPortGroupRegsNumGet
          and then cpssDxChPortGroupDiagRegsDump with not-NULL pointers.
    Expected: GT_OK.
    1.1.3. Call cpssDxChDiagPortGroupRegsNumGet for CPSS_PORT_GROUP_UNAWARE_MODE_CNS
           with regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st   = GT_OK;
    GT_U8                       dev;
    GT_U32                      regsNum = 0;
    GT_PORT_GROUPS_BMP          portGroupsBmp = 1;
    GT_U32                      *regAddrPtr;
    GT_U32                      *regDataPtr;
    GT_U32                      portGroupId;

    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            SKIP_TEST_MAC;
        }

        /*
            1.1.1  Call for each port group cpssDxChDiagPortGroupRegsNumGet and then
            cpssDxChPortGroupDiagRegsDump with not-NULL pointers.
            Expected: GT_OK.
        */
        /* read for each port group */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set next port */
            portGroupsBmp = (1 << portGroupId);

            /* read the registers number */
            st = cpssDxChDiagPortGroupRegsNumGet(dev, portGroupsBmp, &regsNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /*cpssOsPrintf("cpssDxChDiagPortGroupRegsNumGet : regsNum = %d \n",regsNum);*/

            /* allocate space for regAddress and regData */
            regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
            regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                        "cpssOsMalloc: Memory allocation error.");
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                        "cpssOsMalloc: Memory allocation error.");

            /* dump registers */
            st = cpssDxChDiagPortGroupRegsDump(dev,portGroupsBmp, &regsNum, 0,
                                               regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (regAddrPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regAddrPtr);
                regAddrPtr = (GT_U32*)NULL;
            }

            if (regDataPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regDataPtr);
                regDataPtr = (GT_U32*)NULL;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /*
            1.1.2 Call for CPSS_PORT_GROUP_UNAWARE_MODE_CNS cpssDxChDiagPortGroupRegsNumGet
            and then cpssDxChPortGroupDiagRegsDump with not-NULL pointers.
            Expected: GT_OK.
        */
        st = cpssDxChDiagPortGroupRegsNumGet(dev,
                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* allocate space for regAddress and regData */
        regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
        regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                        "cpssOsMalloc: Memory allocation error.");
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                        "cpssOsMalloc: Memory allocation error.");

        st = cpssDxChDiagPortGroupRegsDump(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                           &regsNum, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if (regAddrPtr != NULL)
        {
            /* free memory if allocated */
            cpssOsFree(regAddrPtr);
            regAddrPtr = (GT_U32*)NULL;
        }

        if (regDataPtr != NULL)
        {
            /* free memory if allocated */
            cpssOsFree(regDataPtr);
            regDataPtr = (GT_U32*)NULL;
        }

        /*
            1.1.3. Call cpssDxChDiagPortGroupRegsNumGet for CPSS_PORT_GROUP_UNAWARE_MODE_CNS
            with regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagPortGroupRegsNumGet(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* allocate space for one register */
    regAddrPtr = cpssOsMalloc(sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(sizeof(GT_U32));

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                    "cpssOsMalloc: Memory allocation error.");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                    "cpssOsMalloc: Memory allocation error.");

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPortGroupRegsNumGet(dev,
                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS, &regsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagRegsNumGet(dev, &regsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if (regAddrPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regAddrPtr);
        regAddrPtr = (GT_U32*)NULL;
    }

    if (regDataPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regDataPtr);
        regDataPtr = (GT_U32*)NULL;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagResetAndInitControllerRegsNumGet
(
    IN  GT_U8                   devNum,
    OUT GT_U32                  *regsNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagResetAndInitControllerRegsNumGet)
{
/*
    ITERATE_DEVICES(Bobcat2, Caelum, Bobcat3)
    1.1 Call for cpssDxChDiagResetAndInitControllerRegsNumGet and then
        cpssDxChDiagResetAndInitControllerRegsDump with not-NULL pointers.
    Expected: GT_OK.
    1.2 Call cpssDxChDiagResetAndInitControllerRegsNumGet with regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS                   st   = GT_OK;
    GT_U8                       dev;
    GT_U32                      regsNum = 0;
    GT_U32                      *regAddrPtr;
    GT_U32                      *regDataPtr;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E |
                                     UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1 Call for cpssDxChDiagResetAndInitControllerRegsNumGet and then
                cpssDxChDiagResetAndInitControllerRegsDump with not-NULL pointers.
            Expected: GT_OK.
        */

        /* read the registers number */
        st = cpssDxChDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if( (GT_FALSE == prvUtfIsGmCompilation()) || (regsNum > 0) )
        {
            /* allocate space for regAddress and regData */
            regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
            regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                             "cpssOsMalloc: Memory allocation error.");
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                             "cpssOsMalloc: Memory allocation error.");

            /* dump registers */
            st = cpssDxChDiagResetAndInitControllerRegsDump(dev, &regsNum, 0,
                                                            regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (regAddrPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regAddrPtr);
                regAddrPtr = (GT_U32*)NULL;
            }

            if (regDataPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regDataPtr);
                regDataPtr = (GT_U32*)NULL;
            }
        }

        /*
            1.2 Call cpssDxChDiagResetAndInitControllerRegsNumGet with regsNumPtr[NULL].
                Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagResetAndInitControllerRegsNumGet(dev, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* allocate space for one register */
    regAddrPtr = cpssOsMalloc(sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(sizeof(GT_U32));

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                    "cpssOsMalloc: Memory allocation error.");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                    "cpssOsMalloc: Memory allocation error.");

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if (regAddrPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regAddrPtr);
    }

    if (regDataPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regDataPtr);
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagPortGroupRegsDump
(
    IN    GT_U8                 devNum,
    IN    GT_PORT_GROUPS_BMP    portGroupsBmp,
    INOUT GT_U32                *regsNumPtr,
    IN    GT_U32                offset,
    OUT   GT_U32                *regAddrPtr,
    OUT   GT_U32                *regDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPortGroupRegsDump)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (DxChx)
    1.1.1. Call with regAddrPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.2. Call with regDataPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call with regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4 Call for each port group cpssDxChDiagPortGroupRegsDump
          with regsNumPtr bigger then the real registers number.
    Expected: GT_OK.

*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_U32              regsNum = 0;
    GT_PORT_GROUPS_BMP  portGroupsBmp = 1;
    GT_U32              *regAddrPtr = 0;
    GT_U32              *regDataPtr = 0;
    GT_U32              portGroupId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next active port */
            portGroupsBmp = (1 << portGroupId);

            /*
                1.1.1. Call with regAddrPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                               &regsNum, 0, NULL, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            /*
                1.1.2. Call with regDataPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                               &regsNum, 0, regAddrPtr, NULL);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            /*
                1.1.3. Call with regsNumPtr[NULL].
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                               NULL, 0, regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

            /*
                1.1.4 Call for each port group cpssDxChDiagPortGroupRegsDump
                with regsNumPtr bigger then the real registers number.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagPortGroupRegsNumGet(dev, portGroupsBmp,
                                                 &regsNum);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            /* allocate space for regAddress and regData */
            regAddrPtr = cpssOsMalloc((regsNum+1) * sizeof(GT_U32));
            regDataPtr = cpssOsMalloc((regsNum+1) * sizeof(GT_U32));

            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                    "cpssOsMalloc: Memory allocation error.");
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                    "cpssOsMalloc: Memory allocation error.");

            /* clean buffers to avoid trash in log */
            cpssOsMemSet(regAddrPtr, 0, (regsNum+1) * sizeof(GT_U32));
            cpssOsMemSet(regDataPtr, 0, (regsNum+1) * sizeof(GT_U32));
            regsNum += 1;

            st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                               &regsNum, 0, regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

            if (regAddrPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regAddrPtr);
                regAddrPtr = (GT_U32*)NULL;
            }

            if (regDataPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regDataPtr);
                regDataPtr = (GT_U32*)NULL;
            }
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)

        /* restore port group */
        portGroupsBmp = 1;

        /* 1.2. For not-active port groups check that function returns GT_BAD_PARAM. */
        PRV_CPSS_GEN_PP_START_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
        {
            /* set non-active port group bitmap */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                           &regsNum, 0, NULL, regDataPtr);

            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PTR, st, dev, portGroupsBmp);
        }
        PRV_CPSS_GEN_PP_END_LOOP_NON_ACTIVE_PORT_GROUPS_MAC(dev,portGroupId)
    }

    portGroupsBmp = 1;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* allocate space for one register */
    regAddrPtr = cpssOsMalloc(sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(sizeof(GT_U32));

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                            "cpssOsMalloc: Memory allocation error.");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                            "cpssOsMalloc: Memory allocation error.");

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        regsNum = 1;
        st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                           &regsNum, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    regsNum = 1;
    st = cpssDxChDiagPortGroupRegsDump(dev, portGroupsBmp,
                                       &regsNum, 0, regAddrPtr, regDataPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if (regAddrPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regAddrPtr);
        regAddrPtr = (GT_U32*)NULL;
    }

    if (regDataPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regDataPtr);
        regDataPtr = (GT_U32*)NULL;
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagResetAndInitControllerRegsDump
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *regsNumPtr,
    IN    GT_U32    offset,
    OUT   GT_U32    *regAddrPtr,
    OUT   GT_U32    *regDataPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagResetAndInitControllerRegsDump)
{
/*
    ITERATE_DEVICES_PORT_GROUPS (Bobcat2, Caelum, Bobcat3)
    1.1.1. Call with regAddrPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.2. Call with regDataPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3. Call with regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4 Call with regsNumPtr bigger then the real registers number.
    Expected: GT_BAD_PTR.

*/
    GT_STATUS           st = GT_OK;
    GT_U8               dev;
    GT_U32              regsNum = 0;
    GT_U32              *regAddrPtr = 0;
    GT_U32              *regDataPtr = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                     UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |
                                     UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1.1. Call with regAddrPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagResetAndInitControllerRegsDump(
                                            dev, &regsNum, 0, NULL, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.2. Call with regDataPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagResetAndInitControllerRegsDump(
                                            dev, &regsNum, 0, regAddrPtr, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.3. Call with regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagResetAndInitControllerRegsDump(
                                          dev, NULL, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4 Call with regsNumPtr bigger then the real registers number.
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChDiagResetAndInitControllerRegsNumGet(dev, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        if( (GT_FALSE == prvUtfIsGmCompilation()) || (regsNum > 0) )
        {
            /* allocate space for regAddress and regData */
            regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
            regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                             "cpssOsMalloc: Memory allocation error.");
            UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                             "cpssOsMalloc: Memory allocation error.");

            regsNum += 1;

            st = cpssDxChDiagResetAndInitControllerRegsDump(
                                    dev, &regsNum, 0, regAddrPtr, regDataPtr);
            UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

            if (regAddrPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regAddrPtr);
                regAddrPtr = (GT_U32*)NULL;
            }

            if (regDataPtr != NULL)
            {
                /* free memory if allocated */
                cpssOsFree(regDataPtr);
                regDataPtr = (GT_U32*)NULL;
            }
        }
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* allocate space for one register */
    regAddrPtr = cpssOsMalloc(sizeof(GT_U32));
    regDataPtr = cpssOsMalloc(sizeof(GT_U32));

    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                            "cpssOsMalloc: Memory allocation error.");
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                            "cpssOsMalloc: Memory allocation error.");

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        regsNum = 1;
        st = cpssDxChDiagResetAndInitControllerRegsDump(
                                    dev, &regsNum, 0, regAddrPtr, regDataPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    regsNum = 1;
    st = cpssDxChDiagResetAndInitControllerRegsDump(
                                    dev, &regsNum, 0, regAddrPtr, regDataPtr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if (regAddrPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regAddrPtr);
        regAddrPtr = (GT_U32*)NULL;
    }

    if (regDataPtr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(regDataPtr);
        regDataPtr = (GT_U32*)NULL;
    }
}

UTF_TEST_CASE_MAC(cpssDxChDiagDeviceVoltageGet)
{
    GT_STATUS st = GT_OK;
    GT_U8  dev = 0;
    GT_U32 voltage_milivolts;
    GT_U32 sensorNum;
    GT_U32      notAppFamilyBmp = 0;

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_FALCON_E);
    GM_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* sometimes test result in TEMP_INT interrupt and shutdown of the board */
    PRV_TGF_SKIP_FAILED_TEST_MAC(UTF_ALDRIN_E, "JIRA:CPSS-7658");
#endif

    /*PRV_UTF_LOG0_MAC("starting Voltage sensor test\n");*/
    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*PRV_UTF_LOG1_MAC("dev=%d\n", dev);*/
        for (sensorNum = 0 ; sensorNum < 4 ; sensorNum++)
        {

            st = cpssDxChDiagDeviceVoltageGet(dev, sensorNum, &voltage_milivolts );
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            PRV_UTF_LOG3_MAC("dev=%d sensorNum=%d mV=%d\n", dev, sensorNum, voltage_milivolts);
#ifdef ASIC_SIMULATION
            /* in WM for hwValue = 0 and divider_en =0 divider_cfg =1 calculated value should be 1208mV */
            if (GT_OK == st && voltage_milivolts != 1208)
#else
            if (GT_OK == st && (voltage_milivolts <= 900 || voltage_milivolts >= 1150 ))
#endif
            {
                st = GT_OUT_OF_RANGE;
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }
        }

        /* 4. sensorNumber out-of-range */
        st = cpssDxChDiagDeviceVoltageGet(dev, 4, &voltage_milivolts );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 5. NULL OUT ptr */
        st = cpssDxChDiagDeviceVoltageGet(dev, 0, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    UTF_E_ARCH_NOT_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, notAppFamilyBmp | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E | UTF_FALCON_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDeviceVoltageGet(dev, 0, &voltage_milivolts );
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceVoltageGet(dev, 0, &voltage_milivolts );
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagDeviceTemperatureGet
(
    IN  GT_U8  dev,
    OUT GT_32  *temperaturePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagDeviceTemperatureGet)
{
/*
    ITERATE_DEVICES(Lion and above)
    1.1. Call with not null temperaturePtr.
    Expected: GT_OK.
    1.2. Call api with wrong temperaturePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS st = GT_OK;

    GT_U8  dev;
    GT_32  temperature;
    GT_BOOL didAnySensore;
    GT_U32  ii;

    if (cpssDeviceRunCheck_onEmulator())
    {
        SKIP_TEST_MAC
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        didAnySensore = GT_FALSE;
        for(ii = 0 ; ii < (CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_MAX_E+1); ii ++)
        {
            st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev,ii);
            if(ii == 0 && st == GT_NOT_APPLICABLE_DEVICE)
            {
                /* the device not supports this API */
                /* so just get it's temperature */
                ii = 100;/*cause loop to end after this iteration */
            }
            else
            if(st == GT_BAD_PARAM)
            {
                /* this sensor is not supported by the device */
                continue;
            }
            else
            {
                UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
            }

            didAnySensore = GT_TRUE;

            /*
                1.1. Call with not null temperaturePtr.
                Expected: GT_OK.
            */
            st = cpssDxChDiagDeviceTemperatureGet(dev, &temperature);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

#ifndef ASIC_SIMULATION
            /* make sure we get acceptable value that is not under 0 Celsius  and not above 200 */
            UTF_VERIFY_EQUAL2_STRING_MAC(0, (temperature <= 0 || temperature >= 200) ? 1 : 0,
                                       "(sensor[%d] --> temperature[%d] lower then 0 or higher then 200 degrees Celsius)",
                                       ii, temperature);
#endif
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, didAnySensore,
                                   "didAnySensore = %d", didAnySensore);

        /*
            1.2. Call api with wrong temperaturePtr [NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagDeviceTemperatureGet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                   "%d, temperaturePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E | UTF_XCAT_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDeviceTemperatureGet(dev, &temperature);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceTemperatureGet(dev, &temperature);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagSerdesTuningRxTune
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC    *portLaneArrPtr,
    IN  GT_U32                                      portLaneArrLength,
    IN  CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT            prbsType,
    IN  GT_U32                                      prbsTime,
    IN  CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT     optMode,
    OUT CPSS_PORT_SERDES_TUNE_STC              *optResultArrPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagSerdesTuningRxTune)
{
/*
    ITERATE_DEVICES_PHY_PORT (xCat, Lion)
    1.1.1. Call cpssDxChDiagSerdesTuningRxTune with
                    prbsType[CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E,
                             CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E,
                             CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E],
                    prbsTime[0, 50, 100],
                    optMode[CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E],
                    portLaneArrLength[1],
                    valid portLaneArr.laneNum;
    Expected: GT_OK.
    1.1.2. Call with out of range laneNum.
    Expected: NOT GT_OK.
    1.1.3. Call with out of range prbsType and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.4. Call with out of range optMode and other valid params.
    Expected: GT_BAD_PARAM.
    1.1.5. Call with out of range optResultArrPtr[NULL] and other valid params.
    Expected: GT_BAD_PTR.
*/

#ifndef ASIC_SIMULATION
    GT_STATUS                                   st = GT_OK;
    GT_PHYSICAL_PORT_NUM                        port = DIAG_VALID_PORTNUM_CNS;
    GT_U8                                       dev = 0;
    CPSS_DXCH_DIAG_SERDES_TUNE_PORT_LANE_STC    portLaneArr         = {0, 0};
    GT_U32                                      portLaneArrLength   = 0;
    CPSS_DXCH_DIAG_TRANSMIT_MODE_ENT            prbsType            = 0;
    GT_U32                                      prbsTime            = 0;
    CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ENT     optMode             = 0;
    CPSS_PORT_SERDES_TUNE_STC                   optResultArr;

    GT_U32                                      serdesNum           = 0;
    GT_U32                                      lane                = 0;
    GT_BOOL                                     defaultTimeTested   = GT_FALSE;

    cpssOsMemSet(&optResultArr, 0, sizeof(optResultArr));
#endif

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    /* skip to reduce size of CPSS API log */
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Initialize Serdes optimization system */
        st = cpssDxChDiagSerdesTuningSystemInit(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            /* get max lane num */
            prvUtfDxChPortSerdesNumGet(dev, port, &serdesNum);
            if(0 == serdesNum)
            {/* could be port which serdeses occupied by neighbor */
                continue;
            }

            /*
                1.1.1. Call cpssDxChDiagSerdesTuningRxTune with
                        prbsType[CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E,
                                 CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E,
                                 CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E],
                        prbsTime[0, 50, 100],
                        optMode[CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E];
                Expected: GT_OK.
            */
            for (lane = 0; lane < serdesNum; lane++)
            {
                if(!defaultTimeTested)
                {/* test 200 ms just once, otherwise this test will take too long */

                    /* call with prbsType[CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E] */
                    prbsType = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS7_E;
                    prbsTime = 0;
                    optMode = CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E;

                    portLaneArr.laneNum = lane;
                    portLaneArr.portNum = port;
                    portLaneArrLength = 1;

                    st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                        prbsType, prbsTime, optMode, &optResultArr);

                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                                 "dev %d, port %d", dev, port);

                    defaultTimeTested = GT_TRUE;
                }

                /* call with prbsType[CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E] */
                prbsType = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS15_E;
                prbsTime = 2;
                optMode = CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E;

                portLaneArr.laneNum = lane;
                portLaneArr.portNum = port;
                portLaneArrLength = 1;

                st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                    prbsType, prbsTime, optMode, &optResultArr);
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "dev %d, port %d", dev, port);


                /* call with prbsType[CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E] */
                prbsType = CPSS_DXCH_DIAG_TRANSMIT_MODE_PRBS23_E;
                prbsTime = 4;
                optMode = CPSS_DXCH_DIAG_SERDES_OPTIMIZE_MODE_ACCUR_AWARE_E;

                portLaneArr.laneNum = lane;
                portLaneArr.portNum = port;
                portLaneArrLength = 1;

                st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                    prbsType, prbsTime, optMode, &optResultArr);

                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st,
                                             "dev %d, port %d", dev, port);
            }

            /*
                1.1.2. Call with out of range laneNum.
                Expected: NOT GT_OK.
            */
            portLaneArr.laneNum = serdesNum;

            st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                prbsType, prbsTime, optMode, &optResultArr);
            UTF_VERIFY_NOT_EQUAL2_STRING_MAC(GT_OK, st,
                                             "dev %d, port %d", dev, port);

            /* restore value */
            portLaneArr.laneNum = 0;

            /*
                1.1.2. Call with out of range prbsType and other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagSerdesTuningRxTune
                                (dev, &portLaneArr, portLaneArrLength,
                                prbsType, prbsTime, optMode, &optResultArr),
                                prbsType);

            /*
                1.1.4. Call with out of range optMode and other valid params.
                Expected: GT_BAD_PARAM.
            */
            UTF_ENUMS_CHECK_MAC(cpssDxChDiagSerdesTuningRxTune
                                (dev, &portLaneArr, portLaneArrLength,
                                prbsType, prbsTime, optMode, &optResultArr),
                                optMode);

            /*
                1.1.5. Call with out of range optResultArrPtr and other valid params.
                Expected: GT_BAD_PTR.
            */
            st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                prbsType, prbsTime, optMode, NULL);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st,
                                         "dev %d, port %d", dev, port);
        }

        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* 1.2. For all active devices go over all non available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            portLaneArr.portNum = port;
            st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                                prbsType, prbsTime, optMode, &optResultArr);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        portLaneArr.portNum = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                            prbsType, prbsTime, optMode, &optResultArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* 1.4. For active device check that function returns GT_BAD_PARAM */
        /* for CPU port number.                                            */
        portLaneArr.portNum = CPSS_CPU_PORT_NUM_CNS;
        st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                            prbsType, prbsTime, optMode, &optResultArr);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);

        /* free all resources allocated by tuning algorithm */
        st = cpssDxChDiagSerdesTuningSystemClose(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    portLaneArr.portNum = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                            prbsType, prbsTime, optMode, &optResultArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagSerdesTuningRxTune(dev, &portLaneArr, portLaneArrLength,
                                        prbsType, prbsTime, optMode, &optResultArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
#endif
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagSerdesTuningSystemClose
(
    IN    GT_U8  devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagSerdesTuningSystemClose)
{
/*
    ITERATE_DEVICES (xCat, Lion)
    Old - 1.1. Call cpssDxChDiagSerdesTuningSystemClose (before init).
    Expected: NON_GT_OK. - not applicable - changed and won't fail any more.
    1.1. Call cpssDxChDiagSerdesTuningSystemInit.
    Expected: GT_OK.
    1.2. Call cpssDxChDiagSerdesTuningSystemClose (after init).
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChDiagSerdesTuningSystemInit.
            Expected: GT_OK.
        */
        st = cpssDxChDiagSerdesTuningSystemInit(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.2. Call cpssDxChDiagSerdesTuningSystemClose (after init).
            Expected: GT_OK.
        */
        st = cpssDxChDiagSerdesTuningSystemClose(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagSerdesTuningSystemClose(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagSerdesTuningSystemClose(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagSerdesTuningSystemInit
(
    IN    GT_U8  devNum
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagSerdesTuningSystemInit)
{
/*
    ITERATE_DEVICES (xCat, Lion)
    1.1. Call cpssDxChDiagSerdesTuningSystemInit.
    Expected: GT_OK.
*/
    GT_STATUS   st   = GT_OK;
    GT_U8       dev;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call cpssDxChDiagSerdesTuningSystemInit.
            Expected: GT_OK.
        */
        st = cpssDxChDiagSerdesTuningSystemInit(dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagSerdesTuningSystemInit(dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagSerdesTuningSystemInit(dev);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChDiagSerdesTuningTracePrintEnable
(
    IN    GT_U8  devNum,
    IN    GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagSerdesTuningTracePrintEnable)
{
/*
    ITERATE_DEVICES (xCat, Lion)
    1.1. Call with enable[GT_FALSE, GT_TRUE].
    Expected: GT_OK.
*/
    GT_STATUS                               st          = GT_OK;
    GT_U8                                   dev         = 0;
    GT_BOOL                                 enable      = GT_FALSE;

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /*
            1.1. Call with enable[GT_FALSE, GT_TRUE].
            Expected: GT_OK.
        */

        /* call with enable[GT_FALSE] */
        enable = GT_FALSE;

        st = cpssDxChDiagSerdesTuningTracePrintEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* call with enable[GT_TRUE] */
        enable = GT_TRUE;

        st = cpssDxChDiagSerdesTuningTracePrintEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.            */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E |
                                           UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT2_E | UTF_LION2_E |
                                            UTF_BOBCAT2_E | UTF_CAELUM_E | UTF_ALDRIN_E | UTF_AC3X_E | UTF_BOBCAT3_E | UTF_ALDRIN2_E | UTF_CPSS_PP_ALL_SIP6_CNS);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagSerdesTuningTracePrintEnable(dev, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagSerdesTuningTracePrintEnable(dev, enable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChDiagDeviceTemperatureSensorsSelectSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with sensorType[CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E \
                              CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_1_E \
                              CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_2_E \
                              CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E \
                              CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_4_E \
                              CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_AVERAGE_E].
    Expected: GT_OK.
    1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.3. Call with out of range sensorType.
    Expected: GT_BAD_PARAM.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;
    GT_U32      tsenNum = 5; /* number of temperature sensors */
    GT_BOOL     supportAverAndMax = GT_TRUE;

    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT sensorType    = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT sensorTypeGet = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT sensorTypeSave;

    /* there is no DFX in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;
    if (cpssDeviceRunCheck_onEmulator())
    {
        SKIP_TEST_MAC
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        supportAverAndMax = GT_TRUE;
        switch (PRV_CPSS_PP_MAC(dev)->devFamily)
        {
            case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
                if (PRV_CPSS_PP_MAC(dev)->devSubFamily == CPSS_PP_SUB_FAMILY_NONE_E)
                {
                    /* BC2 */
                    tsenNum = 2;
                }
                else
                {
                    /* Cetus and Caelum */
                    tsenNum = 1;
                }
                break;
            case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
            case CPSS_PP_FAMILY_DXCH_AC3X_E:
            case CPSS_PP_FAMILY_DXCH_XCAT3_E:
                tsenNum = 1;
                break;
            case CPSS_PP_FAMILY_DXCH_AC5_E:
            case CPSS_PP_FAMILY_DXCH_AC5P_E:
                tsenNum = 2;
                supportAverAndMax = GT_FALSE;
                break;
            case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
                tsenNum = 4;
                break;
            case CPSS_PP_FAMILY_DXCH_FALCON_E:
                tsenNum = (PRV_CPSS_PP_MAC(dev)->multiPipe.numOfTiles < 4) ? 1 : 2;
                supportAverAndMax = GT_FALSE;
                break;
            default:
                tsenNum = 5;
                break;
        }

        /* save sensor selection */
        st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeSave);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

        /* 1.2. */
        st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                        "get another sensorType than was set: %d", dev);

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_1_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if(tsenNum < 2)
        {
            /* this sensor is not connected */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_2_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if(tsenNum < 3)
        {
            /* this sensor is not connected */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if(tsenNum < 4)
        {
            /* this sensor is not connected */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_4_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if(tsenNum < 5)
        {
            /* this sensor is not connected */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_AVERAGE_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if(supportAverAndMax != GT_TRUE)
        {
            /* this sensor is not supported */
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.1. */
        sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_MAX_E;

        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        if((!UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) &&
            !PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev)) ||
           (supportAverAndMax != GT_TRUE))
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, sensorType);
        }
        else
        {
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorType);

            /* 1.2. */
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorTypeGet);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

            UTF_VERIFY_EQUAL1_STRING_MAC(sensorType, sensorTypeGet,
                            "get another sensorType than was set: %d", dev);
        }

        /* 1.3. */
        UTF_ENUMS_CHECK_MAC(cpssDxChDiagDeviceTemperatureSensorsSelectSet
                                    (dev, sensorType),
                                    sensorType);

        /* restore sensor type */
        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorTypeSave);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, sensorTypeSave);
    }

    sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceTemperatureSensorsSelectSet(dev, sensorType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChDiagDeviceTemperatureSensorsSelectGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with non-NULL sensorTypePtr.
    Expected: GT_OK.
    1.2. Call with sensorTypePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT sensorType = CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_0_E;

    if (cpssDeviceRunCheck_onEmulator())
    {
        SKIP_TEST_MAC
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,  UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. */
        st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* 1.2. */
                st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, NULL);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, sensorTypePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
            st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorType);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceTemperatureSensorsSelectGet(dev, &sensorType);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChDiagDeviceTemperatureThresholdSet)
{
/*
    ITERATE_DEVICES
    1.1. Call with thresholdValue for Lion2 [-142 \ 0 \ 228],
                                  for Bobcat2, Caelum, Aldrin, AC3X, Bobcat3 [-277 \ 0 \ 198],
    Expected: GT_OK.
    1.2. Call GET function  with same params.
    Expected: GT_OK and same value as written.
    1.3. Call with out of range thresholdValue[-143 \ 229].
    Expected: NOT GT_OK.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_32       thresholdValue    = 0;
    GT_32       thresholdValueGet = 0;
    GT_32       thresholdSave;

    /* there is no DFX in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC;
    if (cpssDeviceRunCheck_onEmulator())
    {
        SKIP_TEST_MAC
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* save threshould */
        st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdSave);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* test low threshoulds on simulation only because it may shutdown board */
#ifdef ASIC_SIMULATION
            /* 1.1. */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
           PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev))
        {
            thresholdValue = -40;
        }
        else
        {
            thresholdValue = -142;
        }

        st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        /* 1.2. */
        st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* the value is rounded down by Set function */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
           PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev))
        {

            thresholdValue = -40;
        }
        else
        {
            thresholdValue = -141;
        }

        UTF_VERIFY_EQUAL1_STRING_MAC(thresholdValue, thresholdValueGet,
                        "get another thresholdValue than was set: %d", dev);

        /* 1.1. */
        thresholdValue = 0;

        st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        /* 1.2. */
        st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(thresholdValue, thresholdValueGet,
                        "get another thresholdValue than was set: %d", dev);
#endif
        /* 1.1. */
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(dev) ||
            (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
        {
            thresholdValue = 125;
        }
        else if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            thresholdValue = 110;
        }
        else if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
                PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            thresholdValue = 150;
        }
        else
        {
            thresholdValue = 228;
        }

        st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        /* 1.2. */
        if (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
        {
            thresholdValue = 125;
        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(dev) ||
            (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
        {
            thresholdValue = 124;
        }
        else if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            thresholdValue = 110;
        }
        else if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
                PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            /* the value is rounded down by Set function */
            thresholdValue = 149;
        }

        st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValueGet);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        UTF_VERIFY_EQUAL1_STRING_MAC(thresholdValue, thresholdValueGet,
                        "get another thresholdValue than was set: %d", dev);

        /* 1.3. */
        if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
           PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(dev))
        {
            thresholdValue = -41;
        }
        else
        {
            thresholdValue = -143;
        }

        st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        /* 1.3. */
        if (PRV_CPSS_SIP_6_10_CHECK_MAC(dev) ||
            (PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
        {
            thresholdValue = 126;
        }
        else if (PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            thresholdValue = 111;
        }
        else if(UTF_CPSS_PP_HW_INFO_E_ARCH_SUPPORTED_MAC(dev) ||
                PRV_CPSS_PP_MAC(dev)->devFamily == CPSS_PP_FAMILY_DXCH_XCAT3_E)
        {
            thresholdValue = 151;
        }
        else
        {
            thresholdValue = 229;
        }

        st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_NOT_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdValue);

        thresholdValue = 0;

        /* restore thresholdValue */
        st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdSave);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_OK, st, dev, thresholdSave);
    }

    thresholdValue = 0;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
            st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceTemperatureThresholdSet(dev, thresholdValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
UTF_TEST_CASE_MAC(cpssDxChDiagDeviceTemperatureThresholdGet)
{
/*
    ITERATE_DEVICES
    1.1. Call with non-NULL thresholdValuePtr.
    Expected: GT_OK.
    1.2. Call with thresholdValuePtr [NULL].
    Expected: GT_BAD_PTR.
*/
    GT_STATUS   st  = GT_OK;
    GT_U8       dev = 0;

    GT_32       thresholdValue = 0;

    if (cpssDeviceRunCheck_onEmulator())
    {
        SKIP_TEST_MAC
    }

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
            /* 1.1. */
        st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

                /* 1.2. */
                st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, NULL);
                UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, thresholdValuePtr = NULL", dev);
    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
                                           UTF_XCAT_E | UTF_LION_E | UTF_XCAT2_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
            st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValue);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagDeviceTemperatureThresholdGet(dev, &thresholdValue);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}
/*----------------------------------------------------------------------------*/
/*static GT_STATUS cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    IN  GT_BOOL               enable
)*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet)
{
    GT_STATUS               st  = GT_OK;                    /* function return value */
    GT_U8                   dev  = 0;                       /* device number */
    GT_PHYSICAL_PORT_NUM    port = DIAG_VALID_PORTNUM_CNS;  /* port number */
    GT_U32                  lane;                           /* lane number per port */
    GT_BOOL                 clearOnReadEnable;              /* clear on read enable flag */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
         UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

     /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            for(lane = 0; lane < PRV_CPSS_MAX_PORT_LANES_CNS; lane++)
            {
                /* 1.1.1 Set GT_FALSE */
                st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, lane, GT_FALSE);
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, lane, &clearOnReadEnable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_FALSE, clearOnReadEnable," Set GT_FALSE but Get GT_TRUE : %d %d %d", dev, port, lane);
                    /* 1.1.2 Set GT_TRUE */
                    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, lane, GT_TRUE);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, lane, &clearOnReadEnable);
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                    UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, clearOnReadEnable," Set GT_TRUE but Get GT_FALSE : %d %d %d", dev, port, lane);
                }
            }
            /*
                1.1.3. Call with lane number out of range.
                Expected: GT_BAD_PARAM.
            */
            st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, lane, GT_FALSE);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "lane number out of range: %d %d", dev, port);
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. For all active devices go over all non available physical ports.
           Expected: GT_BAD_PARAM
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, 0, GT_FALSE);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, 0, GT_FALSE);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, 0, GT_FALSE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
          check that function returns GT_BAD_PARAM.                        */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet(dev, port, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*static GT_STATUS cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  GT_U32                laneNum,
    OUT GT_BOOL              *enablePtr
)*/
UTF_TEST_CASE_MAC(cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet)
{
    GT_STATUS               st  = GT_OK;                    /* function return value */
    GT_U8                   dev  = 0;                       /* device number */
    GT_PHYSICAL_PORT_NUM    port = DIAG_VALID_PORTNUM_CNS;  /* port number */
    GT_U32                  lane;                           /* lane number per port */
    GT_BOOL                 clearOnReadEnable;              /* clear on read enable flag */

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev,
         UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /* 1.1. For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_TRUE))
        {
            for(lane = 0; lane < PRV_CPSS_MAX_PORT_LANES_CNS; lane++)
            {
                /*
                    1.1.1. Call with not-NULL clearOnReadEnable.
                    Expected: GT_OK.
                */
                st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, lane, &clearOnReadEnable);
                if(prvCpssDxChPortRemotePortCheck(dev,port))
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_BAD_PARAM, st, dev, port, lane);
                }
                else
                {
                    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, st, dev, port, lane);
                    /*
                        1.1.2. Call with NULL clearOnReadEnable.
                        Expected: GT_BAD_PTR.
                    */
                    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, lane, NULL);
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PTR, st, "clearOnReadEnable pointer = NULL: %d %d", dev, port);
                }
            }
            /*
                1.1.3. Call with lane number out of range.
                Expected: GT_BAD_PARAM.
            */
            st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, PRV_CPSS_MAX_PORT_LANES_CNS, &clearOnReadEnable);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_BAD_PARAM, st, "lane number out of range: %d %d", dev, port);
        }
        st = prvUtfNextMacPortReset(&port, dev);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.2. For all active devices go over all non available physical ports.
           Expected: GT_BAD_PARAM
        */
        while(GT_OK == prvUtfNextMacPortGet(&port, GT_FALSE))
        {
            /* 1.2.1. Call function for each non-active port */
            st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, 0, &clearOnReadEnable);
            UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
        }

        /* 1.3. For active device check that function returns GT_BAD_PARAM */
        /* for out of bound value for port number.                         */
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, 0, &clearOnReadEnable);
        UTF_VERIFY_EQUAL2_PARAM_MAC(GT_BAD_PARAM, st, dev, port);
    }
    port   = DIAG_VALID_PORTNUM_CNS;

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_NOT_APPLICABLE_DEVICE.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, 0, &clearOnReadEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.
          check that function returns GT_BAD_PARAM.                        */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet(dev, port, 0, &clearOnReadEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*----------------------------------------------------------------------------------------------------------*/

/*
GT_STATUS cpssDxChDiagPortGroupUnitRegsPrint
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  CPSS_DXCH_UNIT_ENT      unitId,
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagPortGroupUnitRegsPrint)
{

    /*
    ITERATE_DEVICES (DxChx)
    1.1.1  For each port group and unitId, call cpssDxChDiagPortGroupRegsNumGet,
           cpssDxChPortGroupDiagRegsDump and cpssDxChPortGroupDiagRegsPrint with not-NULL pointers.
    Expected: GT_OK.
    1.1.2  For CPSS_PORT_GROUP_UNAWARE_MODE_CNS and unitId, call cpssDxChDiagPortGroupRegsNumGet
           cpssDxChPortGroupDiagRegsDump and cpssDxChPortGroupDiagRegsPrint with not-NULL pointers.
    Expected: GT_OK.
    */

    GT_STATUS   st   =  GT_OK;
    GT_U8               dev;
    GT_PORT_GROUPS_BMP  pgBmp = 1;
    GT_U32              regsNum = 0;
    GT_U32             *regAddrPtr;
    GT_U32             *regDataPtr;
    GT_U32              portGroupId;
    CPSS_DXCH_UNIT_ENT  unitId;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Read for each port group */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next port */
            pgBmp = (1 << portGroupId);
            /*
                1.1.1  For each port group and unitId, call cpssDxChDiagRegsNumGet,
                cpssDxChPortGroupDiagRegsDump and cpssDxChPortGroupDiagRegsPrint with not-NULL pointers.
                Expected: GT_OK.
            */

            for(unitId = 0; unitId < CPSS_DXCH_UNIT_LAST_E; ++unitId)
            {

                /* read the registers number */
                st = cpssDxChDiagUnitRegsNumGet(dev, pgBmp, unitId, &regsNum);
                if(st != GT_OK)
                {
                    continue;
                }
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagUnitRegsNumGet: %d, %d", pgBmp, unitId);

                if(regsNum != 0)
                {
                    /* allocate space for regAddress and regData */
                    regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
                    regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

                    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                            "cpssOsMalloc: Memory allocation error.");
                    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                            "cpssOsMalloc: Memory allocation error.");
                    /* dump registers */
                    st = cpssDxChDiagUnitRegsDump(dev,pgBmp, unitId, &regsNum, 0,
                                                  regAddrPtr, regDataPtr);
                    if(st != GT_OK)
                    {
                        st = GT_OK;
                    }
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagUnitRegsDump: %d, %d", pgBmp, unitId);

                    if(regsNum < 20)
                    {
                        st = cpssDxChDiagUnitRegsPrint(dev, pgBmp, unitId);
                        if(st != GT_OK)
                        {
                            st = GT_OK;
                        }
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagUnitRegsPrint: %d, %d", pgBmp, unitId);
                    }

                    if(regAddrPtr != NULL)
                    {
                        /* free memory if allocated */
                        cpssOsFree(regAddrPtr);
                    }
                    if(regDataPtr != NULL)
                    {
                        /* free memory if allocated */
                        cpssOsFree(regDataPtr);
                    }
                }
                /* clear regsNum */
                regsNum = 0;
            }
            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev,portGroupId)
        }
        /*
            1.1.2 For CPSS_PORT_GROUP_UNAWARE_MODE_CNS and each unitId, call cpssDxChDiagPortGroupRegsNumGet
            cpssDxChPortGroupDiagRegsDump and cpssDxChPortGroupDiagRegsPrint with not-NULL pointers.
            Expected: GT_OK.
        */

        for(unitId = 0; unitId < CPSS_DXCH_UNIT_LAST_E; ++unitId)
        {

            st = cpssDxChDiagUnitRegsNumGet(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, unitId, &regsNum);
            if(st != GT_OK)
            {
                continue;
            }
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagUnitRegsNumGet: %d, %d", pgBmp, unitId);

            if(regsNum != 0)
            {

                /* allocate space for regAddress and regData */
                regAddrPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));
                regDataPtr = cpssOsMalloc(regsNum * sizeof(GT_U32));

                UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regAddrPtr,
                                                 "cpssOsMalloc: Memory allocation error.");
                UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)regDataPtr,
                                                 "cpssOsMalloc: Memory allocation error.");

                st = cpssDxChDiagUnitRegsDump(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, unitId,
                                               &regsNum, 0, regAddrPtr, regDataPtr);
                if(st != GT_OK)
                {
                    st = GT_OK;
                }
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagUnitRegsDump: %d, %d", pgBmp, unitId);

                if(regsNum < 20)
                {
                    st = cpssDxChDiagUnitRegsPrint(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, unitId);
                    if(st != GT_OK)
                    {
                        st = GT_OK;
                    }
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagUnitRegsPrint: %d, %d", pgBmp, unitId);
                }

                if(regAddrPtr != NULL)
                {
                    /* free memory if allocated */
                    cpssOsFree(regAddrPtr);
                }
                if(regDataPtr != NULL)
                {
                    /* free memory if allocated */
                    cpssOsFree(regDataPtr);
                }
                /* clear regsNum */
                regsNum = 0;
            }
        }
    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagUnitRegsPrint(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_DXCH_UNIT_LPM_E);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagUnitRegsPrint(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_DXCH_UNIT_LPM_E);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*
GT_STATUS cpssDxChDiagUnitRegsNumGet
(
    IN  GT_U8                devNum,
    IN  GT_PORT_GROUPS_BMP   pgBmp,
    IN  CPSS_DXCH_UNIT_ENT   unitId,
    OUT GT_U32              *regsNumPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagUnitRegsNumGet)
{
    /*
        ITERATE_DEVICES (DxChx)
        1.1.1 call with valid inputs
        Expected: GT_OK
        1.1.2 Call with NULL pointers.
        Expected: GT_BAD_PTR
        1.1.3 Call for CPSS_PORT_GROUP_UNAWARE_MODE_CNS with regsNumPtr[NULL]
        Expected: GT_BAD_PTR
        1.1.4 Call with invalid unitId
        Expected: GT_BAD_PARAM.
        1.1.5 Call with unsupported unitId
        Expected: GT_NOT_SUPPORTED.
    */

    GT_STATUS   st   =  GT_OK;
    GT_U8               dev;
    GT_PORT_GROUPS_BMP  pgBmp = 1;
    GT_U32              regsNum = 0;
    CPSS_DXCH_UNIT_ENT  unitId = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1.1 Call with valid inputs
            Expected: GT_OK
        */
        st = cpssDxChDiagUnitRegsNumGet(dev, pgBmp, unitId, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2 Call with NULL pointer
            Expected: GT_BAD_PTR
        */
        st = cpssDxChDiagUnitRegsNumGet(dev, pgBmp, unitId, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.3 Call cpssDxChDiagPortGroupRegsNumGet for CPSS_PORT_GROUP_UNAWARE_MODE_CNS
            with regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */
        st = cpssDxChDiagUnitRegsNumGet(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, unitId, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4 call with invalid UnitId
            Expected: GT_BAD_PARAM.
        */

        st = cpssDxChDiagUnitRegsNumGet(dev, pgBmp, CPSS_DXCH_UNIT_LAST_E, &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*
            1.1.5 Call with unsupported unitId
            Expected: GT_NOT_SUPPORTED
        */

        st = cpssDxChDiagUnitRegsNumGet(dev, pgBmp,
                                        ((PRV_CPSS_SIP_6_CHECK_MAC(dev)) ? CPSS_DXCH_UNIT_TM_E  :  CPSS_DXCH_UNIT_EREP_E),
                                        &regsNum);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);

    }
    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM. */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagUnitRegsNumGet(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, unitId, &regsNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagUnitRegsNumGet(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, unitId, &regsNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}
/*--------------------------------------------------------------------------*/

/*
GT_STATUS cpssDxChDiagUnitRegsDump
(
    IN    GT_U8                devNum,
    IN    GT_PORT_GROUPS_BMP   pgBmp,
    IN    CPSS_DXCH_UNIT_ENT   unitId,
    INOUT GT_U32              *regsNumPtr,
    IN    GT_U32               offset,
    OUT   GT_U32               regAddrArr[],
    OUT   GT_U32               regDataArr[]
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagUnitRegsDump)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1 Call with valid inputs
    Expected: GT_OK
    1.1.2 Call regsNumPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3 Call regAddrArr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4 Call regDataArr[NULL].
    Expected: GT_BAD_PTR.
    1.1.5 Call with invalid unitId
    Expected: GT_BAD_PARAM
    1.1.6 Call with unsupported unitId
    Expected: GT_NOT_SUPPORTED
*/
    GT_STATUS   st   =  GT_OK;
    GT_U8               dev;
    GT_PORT_GROUPS_BMP  pgBmp = 1;
    GT_U32              regsNum = 1;
    GT_U32              offset = 0;
    GT_U32              regAddr;
    GT_U32              regData;
    CPSS_DXCH_UNIT_ENT  unitId = 0;
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1.1 Call with valid inputs
            Expected: GT_OK
        */
        st = cpssDxChDiagUnitRegsDump(dev, pgBmp, unitId, &regsNum, offset, &regAddr, &regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2. Call regsNumPtr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagUnitRegsDump(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, unitId, NULL, offset, &regAddr, &regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.3 Call regAddr[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagUnitRegsDump(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, unitId, &regsNum, offset, NULL, &regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4 Call regData[NULL].
            Expected: GT_BAD_PTR.
        */

        st = cpssDxChDiagUnitRegsDump(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, unitId, &regsNum, offset, &regAddr, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.5 Call with invalid unitId
            Expected: GT_BAD_PARAM
        */

        st = cpssDxChDiagUnitRegsDump(dev, pgBmp, CPSS_DXCH_UNIT_LAST_E,
                                      &regsNum, offset, &regAddr, &regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*
            1.1.6 Call with unsupported UnitId
            Expected: GT_NOT_SUPPORTED
        */

        st = cpssDxChDiagUnitRegsDump(dev, pgBmp,
                                      ((PRV_CPSS_SIP_6_CHECK_MAC(dev)) ? CPSS_DXCH_UNIT_TM_E  :  CPSS_DXCH_UNIT_EREP_E),
                                      &regsNum, offset, &regAddr, &regData);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        regsNum = 1;
        st = cpssDxChDiagUnitRegsDump(dev, pgBmp, CPSS_DXCH_UNIT_TTI_E, &regsNum, offset, &regAddr, &regData);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    regsNum = 1;

    st = cpssDxChDiagUnitRegsDump(dev, pgBmp, CPSS_DXCH_UNIT_TTI_E, &regsNum, offset, &regAddr, &regData);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

}

/*--------------------------------------------------------------------------*/

/*
GT_STATUS cpssDxChDiagUnitRegsPrint
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  pgBmp,
    IN  CPSS_DXCH_UNIT_ENT  unitId,
)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagUnitRegsPrint)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1 Call with valid input
    Expected: GT_OK.
    1.1.2 Call with Invalid unitId
    Expected: GT_BAD_PARAM.
    1.1.3 Call with unsupported unitId
    Expected: GT_NOT_SUPPORTED
    2. For not-active devices and non applicable devices
    Expected: GT_BAD_PARAM
    3. For out of bound value for device id
    Expected: GT_BAD_PARAM
*/
    GT_STATUS   st    = GT_OK;
    GT_U8               dev;
    GT_PORT_GROUPS_BMP  pgBmp = 1;
    CPSS_DXCH_UNIT_ENT  unitId = 0;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1.2 Call with valid unitId
            Expected: GT_OK
        */
        st = cpssDxChDiagUnitRegsPrint(dev, pgBmp, unitId);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2 Call with Invalid unitId
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChDiagUnitRegsPrint(dev, pgBmp, CPSS_DXCH_UNIT_LAST_E);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);
        /*
            1.1.3 Call with unsupported UnitId
            Expected: GT_NOT_SUPPORTED
        */
        st = cpssDxChDiagUnitRegsPrint(dev, pgBmp,
                                       ((PRV_CPSS_SIP_6_CHECK_MAC(dev)) ? CPSS_DXCH_UNIT_TM_E  :  CPSS_DXCH_UNIT_EREP_E));
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagUnitRegsPrint(dev, pgBmp, unitId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagUnitRegsPrint(dev, pgBmp, unitId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------------------------------------*/

/*
GT_STATUS cpssDxChDiagPortGroupTablePrint
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  CPSS_DXCH_TABLE_ENT     tableType,
    IN  GT_BOOL                 tableValid,
    IN  GT_U32                  firstEntry,
    IN  GT_U32                  lastEntry

)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagPortGroupTablePrint)
{

    /*
    ITERATE_DEVICES (DxChx)
    1.1.1  For each port group and tableType, call cpssDxChDiagTableDump and cpssDxChDiagTablePrint
           with not-NULL pointers.
    Expected: GT_OK.
    1.1.2  For CPSS_PORT_GROUP_UNAWARE_MODE_CNS and tableType, call cpssDxChDiagTableDump and cpssDxChDiagTablePrint
           with not-NULL pointers.
    Expected: GT_OK.
    */

    GT_STATUS   st   =  GT_OK;
    GT_U8               dev;
    GT_PORT_GROUPS_BMP  pgBmp = 1;
    GT_U32              firstEntry = 0;
    GT_U32              numEntries = 0;
    GT_BOOL             tableValid = GT_FALSE;
    GT_U32              *tableBufferPtr = NULL;
    GT_U32              portGroupId;
    GT_U32              i = 0;
    GT_U32              entrySize = 0 ;
    CPSS_DXCH_TABLE_ENT tableType = 0 ;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);

    /* skip to reduce size of CPSS API log */
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    /* Test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_SKIP_MAC("Dump table isn't implemented in GM\n")

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Read for each port group */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* set next port */
            pgBmp = (1 << portGroupId);
            /*
                1.1.1  For each port group and tableType, call cpssDxChDiagTableDump and cpssDxChDiagTablePrint
                with not-NULL pointers.
                Expected: GT_OK.
            */

            for(tableType = 0; tableType < CPSS_DXCH_TABLE_LAST_E; tableType++)
            {

                /* read the registers number */
                st = prvCpssDxChTableNumEntriesGet(dev, tableType, &numEntries);
                if(st != GT_OK || numEntries == 0)
                {
                   continue;
                }

                st = prvCpssDxChDiagTableSkip(dev, tableType);
                if(st != GT_OK)
                {
                    numEntries = 0;
                }


                if(numEntries > 0)
                {
                    /* get entry size in words*/
                    st = prvCpssDxChTableEntrySizeGet(dev, tableType, &entrySize);
                    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                    /* Allocate space for tableBufferPtr */
                    tableBufferPtr = cpssOsMalloc(entrySize * sizeof(GT_U32));
                    UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)tableBufferPtr,
                                            "cpssOsMalloc: Memory allocation error.");

                    cpssOsMemSet(tableBufferPtr, 0, entrySize * sizeof(GT_U32));


                    for(i = firstEntry; i < numEntries; i++)
                    {

                        /* Dump Table entry*/
                        st = cpssDxChDiagTableDump(dev, pgBmp, tableType, tableValid, &entrySize, i, i, tableBufferPtr, &entrySize);
                        if(st != GT_OK)
                        {
                            st = GT_OK;
                        }
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagTableDump: %d, %d", pgBmp, tableType);

                    }

                    if(tableBufferPtr != NULL)
                    {
                        /* free memory if allocated */
                        cpssOsFree(tableBufferPtr);
                    }

                    if(numEntries < 10)
                    {
                        st = cpssDxChDiagTablePrint(dev, pgBmp, tableType, tableValid, firstEntry, numEntries);
                        if(st != GT_OK)
                        {
                            st = GT_OK;
                        }
                        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagTablePrint: %d, %d", pgBmp, tableType);
                    }

                }
                /* clear numEntries */
                numEntries = 0;
                entrySize = 0;
            }

            PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        }
        /*
            1.1.2  For CPSS_PORT_GROUP_UNAWARE_MODE_CNS and unitId, call cpssDxChDiagTableDump and cpssDxChDiagTablePrint
                   with not-NULL pointers.
                   Expected: GT_OK.
        */

        for(tableType = 0; tableType < CPSS_DXCH_TABLE_LAST_E; tableType++)
        {

            /* read the registers number */
            st = prvCpssDxChTableNumEntriesGet(dev, tableType, &numEntries);
            if(st != GT_OK || numEntries == 0)
            {
                continue;
            }

            st = prvCpssDxChDiagTableSkip(dev, tableType);
            if(st != GT_OK)
            {
                 numEntries = 0;
            }

            if(numEntries > 0)
            {
                /* get entry size in words*/
                st = prvCpssDxChTableEntrySizeGet(dev, tableType, &entrySize);
                UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

                /* Allocate space for tableBufferPtr */
                tableBufferPtr = cpssOsMalloc(entrySize * sizeof(GT_U32));
                UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)tableBufferPtr,
                                                  "cpssOsMalloc: Memory allocation error.");

                cpssOsMemSet(tableBufferPtr, 0, entrySize * sizeof(GT_U32));

                for(i = firstEntry; i < numEntries; i++)
                {

                    /* Dump Table entry*/
                    st = cpssDxChDiagTableDump(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, tableType, tableValid,
                                               &entrySize, i, i, tableBufferPtr, &entrySize);
                    if(st != GT_OK)
                    {
                       st = GT_OK;
                    }
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagTableDump: %d, %d", pgBmp, tableType);
                }

                if(tableBufferPtr != NULL)
                {
                    /* free memory if allocated */
                    cpssOsFree(tableBufferPtr);
                }

                if(numEntries < 10)
                {
                    st = cpssDxChDiagTablePrint(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, tableType, tableValid, firstEntry, numEntries);
                    if(st != GT_OK)
                    {
                        st = GT_OK;
                    }
                    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagTablePrint: %d, %d", pgBmp, tableType);
                }
            }
            /* clear numEntries */
            numEntries = 0;
            entrySize = 0;
        }

    }
}

/*----------------------------------------------------------------------------------------------------------*/

/*
GT_STATUS cpssDxChDiagTableDump
(
    IN      GT_U8                  devNum,
    IN      GT_PORT_GROUPS_BMP     portGroupsBmp,
    IN      CPSS_DXCH_TABLE_ENT    tableType,
    IN      GT_BOOL                tableValid,
    INOUT   GT_U32                *bufferLengthPtr,
    IN      GT_U32                 firstEntry,
    IN      GT_U32                 lastEntry,
    OUT     GT_U32                 tableBufferArr[],
    OUT     GT_U32                *entrySizePtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChDiagTableDump)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1 Call with valid inputs
    Expected: GT_OK
    1.1.2 Call bufferLengthPtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.3 Call tableBufferArr[NULL].
    Expected: GT_BAD_PTR.
    1.1.4 Call entrySizePtr[NULL].
    Expected: GT_BAD_PTR.
    1.1.5 Call with invalid tableType
    Expected: GT_BAD_PARAM
    1.1.6 Call with tableValid set for tables other than VLAN and FDB
    Expected: GT_BAD_PARAM.
    1.1.7 Call with unsupported tableType
    Expected: GT_NOT_SUPPORTED
*/

    GT_STATUS   st    =   GT_OK;
    GT_U8                 dev;
    GT_PORT_GROUPS_BMP    pgBmp = 1;
    CPSS_DXCH_TABLE_ENT   tableType = 0;
    GT_U32                firstEntry = 0;
    GT_U32                lastEntry = 0;
    GT_U32                *tableBufferArr = NULL;
    GT_U32                entrySize;
    GT_U32                tableValid = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_SKIP_MAC("Dump table isn't implemented in GM\n")

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* get entry size in words*/
        st = prvCpssDxChTableEntrySizeGet(dev, tableType, &entrySize);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* Allocate space for tableBufferPtr */
        tableBufferArr = cpssOsMalloc(entrySize * sizeof(GT_U32));
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC((GT_UINTPTR)NULL, (GT_UINTPTR)tableBufferArr,
                                              "cpssOsMalloc: Memory allocation error.");

        /*
            1.1.1 Call with valid inputs(tableValid = GT_FALSE)
            Expected: GT_OK
        */
        st = cpssDxChDiagTableDump(dev, pgBmp, tableType, tableValid, &entrySize, firstEntry, lastEntry, tableBufferArr, &entrySize);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2 Call with bufferLength[NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChDiagTableDump(dev, pgBmp, tableType, tableValid, NULL, firstEntry, lastEntry, tableBufferArr, &entrySize);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.3 Call with tableBufferArr[NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChDiagTableDump(dev, pgBmp, tableType, tableValid, &entrySize, firstEntry, lastEntry, NULL, &entrySize);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);

        /*
            1.1.4 Call with entrySize[NULL].
            Expected: GT_BAD_PTR
        */
        st = cpssDxChDiagTableDump(dev, pgBmp, tableType, tableValid, &entrySize, firstEntry, lastEntry, tableBufferArr, NULL);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PTR, st);
        /*
            1.1.5 Call with Invalid tableType
            Expected: GT_BAD_PARAM.
        */

        st = cpssDxChDiagTableDump(dev, pgBmp, CPSS_DXCH_TABLE_LAST_E, tableValid, &entrySize, firstEntry, lastEntry, tableBufferArr, &entrySize);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*
            1.1.6 Call with tableValid set for tables other than VLAN and FDB
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChDiagTableDump(dev, pgBmp, tableType, GT_TRUE, &entrySize, firstEntry, lastEntry, tableBufferArr, &entrySize);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*
            1.1.7 Call with unsupported tableType
            Expected: GT_NOT_SUPPORTED
        */
        st = cpssDxChDiagTableDump(dev, pgBmp, CPSS_DXCH_SIP6_TABLE_LAST_E, tableValid, &entrySize, firstEntry, lastEntry, tableBufferArr, &entrySize);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagTableDump(dev, pgBmp, tableType, tableValid, &entrySize, firstEntry, lastEntry, tableBufferArr, &entrySize);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagTableDump(dev, pgBmp, tableType, tableValid, &entrySize, firstEntry, lastEntry, tableBufferArr, &entrySize);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

    if(tableBufferArr != NULL)
    {
        /* free memory if allocated */
        cpssOsFree(tableBufferArr);
    }

}

/*----------------------------------------------------------------------------------------------------------*/

/*
GT_STATUS cpssDxChDiagTablePrint
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  CPSS_DXCH_TABLE_ENT     tableType,
    IN  GT_BOOL                 tableValid,
    IN  GT_U32                  firstEntry,
    IN  GT_U32                  lastEntry

)
*/
UTF_TEST_CASE_MAC(cpssDxChDiagTablePrint)
{
/*
    ITERATE_DEVICES (DxChx)
    1.1.1 Call with valid input
    Expected: GT_OK.
    1.1.2 Call with Invalid tableType
    Expected: GT_BAD_PARAM.
    1.1.3 Call with tableValid set for tables other than VLAN and FDB
    Expected: GT_BAD_PARAM.
    1.1.4 Call with unsupported tableType
    Expected: GT_NOT_SUPPORTED
    2. For not-active devices and non applicable devices
    Expected: GT_BAD_PARAM
    3. For out of bound value for device id
    Expected: GT_BAD_PARAM
*/

    GT_STATUS   st    =   GT_OK;
    GT_U8                 dev;
    GT_PORT_GROUPS_BMP    pgBmp = 1;
    CPSS_DXCH_TABLE_ENT   tableType = 0;
    GT_U32                firstEntry = 0;
    GT_U32                lastEntry = 3;
    GT_U32                tableValid = GT_FALSE;

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* Test is not ready for GM */
    UTF_SIP5_GM_NOT_READY_SKIP_MAC("Dump table isn't implemented in GM\n")

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {

        /*
            1.1.1 Call with valid inputs
            Expected: GT_OK
        */
        st = cpssDxChDiagTablePrint(dev, pgBmp, tableType, tableValid, firstEntry, lastEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /*
            1.1.2 Call with Invalid tableType
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChDiagTablePrint(dev, pgBmp, CPSS_DXCH_TABLE_LAST_E, tableValid, firstEntry, lastEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*
            1.1.3 Call with tableValid set for tables other than VLAN and FDB
            Expected: GT_BAD_PARAM.
        */
        st = cpssDxChDiagTablePrint(dev, pgBmp, tableType, GT_TRUE, firstEntry, lastEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_BAD_PARAM, st);

        /*
            1.1.4 Call with unsupported tableType
            Expected: GT_NOT_SUPPORTED
        */
        st = cpssDxChDiagTablePrint(dev, pgBmp, CPSS_DXCH_SIP6_TABLE_LAST_E, tableValid, firstEntry, lastEntry);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_NOT_SUPPORTED, st);

    }

    /* 2. For not-active devices and devices from non-applicable family */
    /* check that function returns GT_BAD_PARAM.                        */
    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);
    /* go over all non active devices */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChDiagTablePrint(dev, pgBmp, tableType, tableValid, firstEntry, lastEntry);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChDiagTablePrint(dev, pgBmp, tableType, tableValid, firstEntry, lastEntry);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------*/

UTF_TEST_CASE_MAC(cpssDxChDiag_cleanup)
{
    PRV_TGF_SKIP_TO_REDUCE_LOG_SIZE_MAC;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

    /* TCAM BIST and other tests leave device in bad state. Use Soft reset to fix it. */
    if(prvTgfResetModeGet() == GT_TRUE)
    {
        /* give priority to reset with HW */
        prvTgfResetAndInitSystem();
    }
    else
    {
        /* not applicable device */
        prvUtfSkipTestsSet();
    }
}

#ifdef CPSS_LOG_ENABLE
GT_VOID utf_cpssDxChDiagDeviceUnitRegsPrint()
{

    GT_STATUS   st   =  GT_OK;
    GT_U8               dev;
    GT_U32              regsNum = 0, i;
    CPSS_DXCH_UNIT_ENT  unitId = 0;
    GT_U32              len = 0;
    GT_CHAR unitName[120];
    GT_CHAR strNameBuffer[120];

    /* prepare device iterator */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(unitId = 0; unitId < CPSS_DXCH_UNIT_LAST_E; unitId++)
        {
            st = cpssDxChDiagUnitRegsNumGet(dev, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, unitId, &regsNum);
            if(st != GT_OK)
            {
                continue;
            }
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "cpssDxChDiagUnitRegsNumGet: %d, %d", regsNum, unitId);

            if(regsNum > 0)
            {
                cpssOsMemSet(unitName, 0, sizeof(unitName));
                cpssOsMemSet(strNameBuffer, 0, sizeof(strNameBuffer));
                cpssOsSprintf(strNameBuffer, "\"%s\"", prvCpssLogEnum_CPSS_DXCH_UNIT_ENT[unitId]);
                cpssOsSprintf(unitName, "%s", strNameBuffer+16);
                len = cpssOsStrlen(unitName);
                unitName[len-3] = '"';     /* Remove the last "_E" */
                unitName[len-2] = '\0';    /* Remove the last "_E" */

                /* Small case convert */
                for(i = 0; i < len; i++)
                {
                    if((unitName[i] >= 65) && (unitName[i] <= 90))
                        unitName[i] += 32;
                }

                PRV_UTF_LOG3_MAC("[\"%-30s] = { value= %-40s, help = %s},\n", unitName, strNameBuffer, strNameBuffer);
            }
        }
    }
}
#endif

#ifdef CPSS_LOG_ENABLE
GT_VOID utf_cpssDxChDiagDeviceTablePrint()
{

    GT_STATUS   st   =   GT_OK;
    GT_U8                dev;
    GT_U32               numEntries = 0;
    CPSS_DXCH_TABLE_ENT  tableType = 0;
    GT_U32               len = 0, len1 = 0;
    GT_CHAR tableName[500];
    GT_CHAR strNameBuffer[500];
    GT_CHAR *key = "TABLE";
    GT_CHAR *result = NULL, *result1 = NULL;
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, UTF_NONE_FAMILY_E);

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        for(tableType = 0; tableType < CPSS_DXCH_SIP6_TABLE_LAST_E; tableType++)
        {

            st = prvCpssDxChTableNumEntriesGet(dev, tableType, &numEntries);
            if(st != GT_OK)
            {
                continue;
            }
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "prvCpssDxChTableNumEntriesGet: %d, %d", numEntries, tableType);

            if(numEntries > 0)
            {

                cpssOsMemSet(tableName, 0, sizeof(tableName));
                cpssOsMemSet(strNameBuffer, 0, sizeof(strNameBuffer));
                cpssOsSprintf(strNameBuffer, "\"%s\"", prvCpssLogEnum_map_CPSS_DXCH_TABLE_ENT[tableType].namePtr);
                cpssOsSprintf(tableName, "%s", strNameBuffer);
                len = cpssOsStrlen(tableName);
                len1 = cpssOsStrlen(key);
                tableName[len-3] = '"';     /* Remove the last "_E" */
                tableName[len-2] = '\0';    /* Remove the last "_E" */

                if(cpssOsStrStr(tableName, "LAST")!= NULL)
                {
                   continue;
                }

                result = cpssOsStrStr(tableName, key);
                if(result != NULL)
                {
                    result += len1 + 1;
                }
                else if(cpssOsStrStr(tableName, "CPSS_DXCH_SIP6_TXQ")!= NULL)
                {
                    result = tableName + 16;
                }
                else
                {
                    result = tableName + 12;
                }

                result1 = result;
                cpssOsPrintf("[%c", '"');
                while(*result != '\0')
                {
                    if((*result >= 65) && (*result <= 90))
                    {
                        cpssOsPrintf("%c", *result + 32);
                    }
                    else if(*result == '_')
                    {
                        *result = '-';
                        cpssOsPrintf("%c", *result);
                    }
                    else
                    {
                        cpssOsPrintf("%c", *result);
                    }
                    result++;
                }

                PRV_UTF_LOG2_MAC("""]= { value= %-40s, help = %c ",strNameBuffer,'"');

                while(*result1 != '\0')
                {
                    if((*result1 >= 65) && (*result1 <= 90))
                    {
                        cpssOsPrintf("%c", *result1 + 32);
                    }
                    else if(*result1 == '-' || *result1 == '"')
                    {
                        *result1 = ' ';
                        cpssOsPrintf("%c", *result1);
                    }
                    else
                    {
                        cpssOsPrintf("%c", *result1);
                    }
                    result1++;
                }
                cpssOsPrintf("table %c},\n",'"');
            }
        }
    }
}
#endif


/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChDiag suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChDiag)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagAllMemTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagAllRegTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagMemRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagMemTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagMemWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortCheckEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortCheckEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortCheckReadyGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortGenerateEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortGenerateEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortTransmitModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsPortTransmitModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagRegTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagRegsDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagRegsNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagResetAndInitControllerRegsDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagResetAndInitControllerRegsNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsCyclicDataSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsCyclicDataGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPhyRegRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPhyRegWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagRegRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagRegWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesTestEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesTestEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesTransmitModeSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesTransmitModeGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagMemoryBistBlockStatusGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagMemoryBistsRun)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPortGroupMemWrite)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPortGroupMemRead)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPortGroupRegsNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPortGroupRegsDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceTemperatureGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceTemperatureSensorsSelectSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceTemperatureSensorsSelectGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceTemperatureThresholdSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceTemperatureThresholdGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagDeviceVoltageGet)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagSerdesTuningSystemClose)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagSerdesTuningRxTune)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagSerdesTuningSystemInit)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagSerdesTuningTracePrintEnable)

    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesFunctionalityTest)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesCounterClearOnReadEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPrbsSerdesCounterClearOnReadEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagUnitRegsNumGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagUnitRegsDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagUnitRegsPrint)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPortGroupUnitRegsPrint)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagTableDump)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagTablePrint)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiagPortGroupTablePrint)
    /* should be last */
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChDiag_cleanup)
UTF_SUIT_END_TESTS_MAC(cpssDxChDiag)

