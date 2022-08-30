/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssDxChFlowManagerUT.c
*
* @brief The mainUt of CPSS DXCH High Level IPFIX Flow Manager
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/flowManager/prvCpssDxChFlowManagerDb.h>
#include <cpss/dxCh/dxChxGen/flowManager/cpssDxChFlowManager.h>
#include <gtUtil/gtStringUtil.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC
static GT_U8   devListArr[PRV_CPSS_MAX_PP_DEVICES_CNS];

/* get the bmp of families that support eArch and non-eArch devices */
#define UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(_notAppFamilyBmpPtr) \
     *(_notAppFamilyBmpPtr) = (UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |              \
                               UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E | UTF_LION_E | UTF_XCAT2_E | UTF_LION2_E) | \
                               UTF_CPSS_PP_E_ARCH_CNS

/* Create Flow Manager */
/* make sure no device in the flow manager */
/* add the first device */
/* --- cleaning config --- */
/* remove the device from the flow manager */
/* delete the flow manager */
/* make sure flow manager does not exist */
/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChPortCnCNMEnableSet
(
    IN GT_U32  flowMngId,
    IN GT_BOOL enable
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerEnableSet)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Call with enable [GT_TRUE].
    Expected: GT_OK;
    1.3. Call with enable [GT_FALSE].
    Expected: GT_OK;
    1.4. Call function with non existing flow manager id.
    Expected: GT_NOT_FOUND;
    1.5. Call function with out of bound value for flow manager id.
    Expected: GT_BAD_PARAM;
    1.6. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    GT_BOOL                                     enable = GT_FALSE;
    GT_BOOL                                     enableGet = GT_FALSE;
    GT_U32                                      notAppFamilyBmp = 0;
    GT_BOOL                                     isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /*
           1.2. Call with enable [GT_TRUE].
           Expected: GT_OK.
        */
        enable = GT_TRUE;
        st = cpssDxChFlowManagerEnableSet(flowMngId, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChFlowManagerEnableGet(flowMngId, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerEnableSet: %d\n", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "got another enable then was set for flow manager id: %d\n", flowMngId);

        /*
           1.3. Call with enable [GT_FALSE].
           Expected: GT_OK.
         */
        enable = GT_FALSE;
        st = cpssDxChFlowManagerEnableSet(flowMngId, enable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        st = cpssDxChFlowManagerEnableGet(flowMngId, &enableGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerEnableGet: %d\n", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enableGet,
            "got another enable then was set for flow manager id: %d\n", flowMngId);

        /*
           1.4 Call function with non existing flow manager id.
           Expected: GT_NOT_FOUND
        */
        flowMngId = 2;
        enable = GT_TRUE;
        st = cpssDxChFlowManagerEnableSet(flowMngId, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st, "cpssDxChFlowManagerEnableSet returned not found"
            "for flow manager id: %d\n", flowMngId);

        /*
           1.5. Call function with out of bound value for flow manager id.
           Expected: GT_BAD_PARAM
        */
        flowMngId = PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1;
        st = cpssDxChFlowManagerEnableSet(flowMngId, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerEnableSet returned bad param"
            "for flow manager id: %d\n", flowMngId);

        /*
           1.6. Delete the Flow Manager.
           Expected: GT_NOT_INITIALIZED
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete succeeded flowMngId = %d\n",
            flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerEnableGet
(
    IN  GT_U32  flowMngId,
    OUT GT_BOOL *enablePtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerEnableGet)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Call with enable [GT_TRUE].
    Expected: GT_OK;
    1.3. Call with non-null enablePtr.
    Expected: GT_OK;
    1.4. Call with NULL enablePtr.
    Expected: GT_BAD_PTR;
    1.5. Call function with non existing flow manager id.
    Expected: GT_NOT_FOUND;
    1.6. Call function with out of bound value for flow manager id.
    Expected: GT_BAD_PARAM;
    1.7. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    GT_BOOL                                     enable = GT_FALSE;
    GT_BOOL                                     enablePtr;
    GT_U32                                      notAppFamilyBmp = 0;
    GT_BOOL                                     isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /*
           1.2. Call with enable [GT_TRUE].
           Expected: GT_OK.
        */
        enable = GT_TRUE;
        st = cpssDxChFlowManagerEnableSet(flowMngId, enable);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerEnableSet: %d\n", dev);
        /*
           1.3. Call with non-null enablePtr.
           Expected: GT_OK.
        */
        st = cpssDxChFlowManagerEnableGet(flowMngId, &enablePtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(enable, enablePtr,
            "got another enable then was set for flow manager id: %d\n", flowMngId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);

        /*
           1.4. Call with NULL enablePtr
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChFlowManagerEnableGet(flowMngId, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, enablePtr = NULL", flowMngId);

        /*
           1.5. Call function with non existing flow manager id
           Expected: GT_NOT_FOUND.
        */
        flowMngId = 2;
        enable = GT_TRUE;
        st = cpssDxChFlowManagerEnableGet(flowMngId, &enablePtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_FOUND, st, "cpssDxChFlowManagerEnableGet returned not found"
                                     "for flow manager id: %d\n", flowMngId);

        /*
           1.6. Call function with out of bound value for flow manager id
           Expected: GT_BAD_PARAM
        */
        flowMngId = PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1;
        st = cpssDxChFlowManagerEnableGet(flowMngId, &enablePtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerEnableGet returned bad param"
                                     "for flow manager id: %d\n", flowMngId);

        /*
           1.7. Delete the Flow Manager
           Expected: GT_OK
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);
        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerConfigurationParamsValidate
(
    IN  GT_U32                                      flowMngId,
    IN  CPSS_DXCH_FLOW_MANAGER_STC                  *fwFlowMngInfoPtr,
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerConfigurationParamsValidate)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Call device list add with one incorrect parameters in subsequent call.
    Expected: GT_BAD_PARAM;
    1.3. Call function with out of bound value for flow db size.
    Expected: GT_BAD_PARAM;
    1.4. Enable flow manager service.
    Expected: GT_OK;
    1.5. Add device to flow manager.
    Expected: GT_OK;
    1.6. Remove device from flow manager.
    Expected: GT_OK;
    1.7. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      notAppFamilyBmp = 0;
    GT_BOOL                                     isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 129;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 3;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 2;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.2. add the first device with wrong RX SDMA channel
           Expected: GT_BAD_PARAM
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerDevListAdd\n");

        /*
           1.3. Delete the Flow Manager
           Expected: GT_OK
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        /* 1.4. add the first device with wrong TX SDMA channel
           Expected: GT_BAD_PARAM
        */
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 129;

        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerDevListAdd\n");

        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);
        /* 1.5. add the first device with wrong messagePacketCpuCode
           Expected: GT_BAD_PARAM
        */
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 64;

        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerDevListAdd\n");

        /* 1.6. remove the device
           Expected: GT_OK
        */
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);
        /* 1.7. add the first device with wrong cpuCodeStart
           Expected: GT_BAD_PARAM
        */
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 3;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 64;

        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerDevListAdd\n");

        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);
        /* 1.8. add the first device with wrong tcamStartIndex
           Expected: GT_BAD_PARAM
        */
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 2;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 18*_1K;

        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerDevListAdd\n");

        /* 1.9. delete the device
           Expected: GT_OK
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);
        /* 1.10. add the first device with wrong ipfixStartIndex
           Expected: GT_BAD_PARAM
        */
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 5*_1K;

        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerDevListAdd\n");

        /* 1.11. delete the device
           Expected: GT_OK
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);
        /* 1.12. add the first device with wrong flowDBSize
           Expected: GT_BAD_PARAM
        */
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = _8K + 1;

        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerDevListAdd\n");

        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);
        /* 1.13. add the first device with wrong flowShortDBSize
           Expected: GT_BAD_PARAM
        */
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 501;

        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerDevListAdd\n");

        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);
        /* 1.14. add the first device with all correct params
           Expected: GT_BAD_PARAM
        */
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;

        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        /* 1.15. validate the flow manager id acceptable limit
           Expected: GT_BAD_PARAM
        */
        flowMngId = PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1;
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerCreate\n");

        /* 1.16. validate the cpss flow db acceptable limit
           Expected: GT_BAD_PARAM
        */
        flowMngId = 1;
        fwFlowMngInfo.flowDbSize = PRV_CPSS_DXCH_FLOW_DB_MAX_CNS + 1;
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerCreate\n");

        /* 1.17. remove the device
           Expected: GT_OK
        */
        fwFlowMngInfo.flowDbSize = 2000;
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.18. Delete the Flow Manager
           Expected: GT_OK
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerCreate
(
    IN  GT_U32                                      flowMngId,
    IN  CPSS_DXCH_FLOW_MANAGER_STC                  *fwFlowMngInfoPtr,
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerCreate)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Call function with out of bound value for flow manager id.
    Expected: GT_BAD_PARAM;
    1.3. Call function with out of bound value for flow db size.
    Expected: GT_BAD_PARAM;
    1.4. Enable flow manager service.
    Expected: GT_OK;
    1.5. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.6. Remove device from flow manager.
    Expected: GT_OK;
    1.7. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      notAppFamilyBmp = 0;
    GT_BOOL                                     isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 3;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 2;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_20B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /*
           1.7. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.2. validate the flow manager id acceptable limit
           Expected: GT_BAD_PARAM
        */
        flowMngId = PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1;
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerCreate\n");

        /* 1.3. validate the cpss flow db acceptable limit
           Expected: GT_BAD_PARAM
        */
        fwFlowMngInfo.flowDbSize = PRV_CPSS_DXCH_FLOW_DB_MAX_CNS + 1;
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChFlowManagerCreate\n");

        /* 1.4. enable flow manager service
           Expected: GT_OK
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerEnableSet(flowMngId, GT_TRUE);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        /* 1.5. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        devListArr[1] = 1;
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_ALREADY_EXIST, st, "cpssDxChFlowManagerDevListAdd\n");

        /* 1.6. remove the device
           Expected: GT_OK
        */
        devListArr[1] = dev;
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.7. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerKeyEntryUserDefinedByteSet
(
    IN GT_U32                                     flowMngId,
    IN GT_U32                                     keyTableIndex,
    IN GT_U32                                     keySizeInBytes,
    IN GT_U32                                     udbIndex,
    IN CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT          offsetType,
    IN GT_U8                                      offsetOrPattern,
    IN GT_U8                                      mask
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerKeyEntryUserDefinedByteSet)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call with all the correct params.
    Expected: GT_OK.
    1.5. Call with wrong enumvalues for offsetType.
    Expected: GT_BAD_PARAM.
    1.6. Call with out of range values of offsetOrPattern for L2/L3/L4 anchor type.
    Expected: GT_OUT_OF_RANGE.
    1.7. Call with out of range value for flowMngId.
    Expected: GT_BAD_PARAM.
    1.8. Call with wrong values of flowMngId.
    Expected: GT_NOT_FOUND.
    1.9. Call with not usable udb index for tcam rule size 30B.
    Expected: GT_BAD_PARAM.
    1.10. Call with not allowed offset values for SRC PORT anchor type.
    Expected: GT_BAD_PARAM.
    1.11. Call with not allowed offset values for VLAN ID anchor type.
    Expected: GT_BAD_PARAM.
    1.12. Remove device from flow manager.
    Expected: GT_OK;
    1.13. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/

    CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT           offsetType;
    GT_U32                                      udbIndex;
    GT_U32                                      keyTableIndex;
    GT_U8                                       offsetOrPattern;
    GT_U8                                       mask;
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      notAppFamilyBmp = 0;
    GT_BOOL                                     isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;
    GT_U32                                      keySizeInBytes;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        udbIndex = 3;
        keyTableIndex = 0;
        keySizeInBytes = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        offsetType = CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E;
        offsetOrPattern = 14;
        mask = 0xFF;

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        /*
           1.5. Call with wrong enum values offsetType.
           Expected: GT_BAD_PARAM.
         */
        UTF_ENUMS_CHECK_MAC(cpssDxChFlowManagerKeyEntryUserDefinedByteSet
                           (flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                            offsetOrPattern, mask),offsetType);

        /*
           1.6. Call with wrong value for offsetOrPattern for L2 anchor type.
           Expected: GT_OUT_OF_RANGE.
         */
        offsetOrPattern = 128;
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, flowMngId);

        /*
           1.7. Call with not allowed values of flowMngId.
           Expected: GT_BAD_PARAM.
         */
        offsetOrPattern = 14;
        flowMngId = PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1;
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, flowMngId);

        /*
           1.8. Call with not used values of flowMngId.
           Expected: GT_NOT_FOUND.
         */
        flowMngId = 2;
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, flowMngId);

        /*
           1.9. Call with not usable udb index for tcam rule size 30B.
           Expected: GT_BAD_PARAM.
         */
        flowMngId = 1;
        udbIndex = 30;
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, flowMngId);

        /*
           1.10. Call with not allowed value for SRC PORT anchor type.
           Expected: GT_BAD_PARAM.
         */
        flowMngId = 1;
        udbIndex = 3;
        offsetType = CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E;
        offsetOrPattern = 2;
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, flowMngId);

        /*
           1.11. Call with not allowed value for SRC PORT anchor type.
           Expected: GT_BAD_PARAM.
         */
        flowMngId = 1;
        udbIndex = 3;
        offsetType = CPSS_DXCH_FLOW_MANAGER_ANCHOR_VID_E;
        offsetOrPattern = 2;
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, st, flowMngId);

        /* 1.12. remove the device
           Expected: GT_OK
        */
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.13. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerKeyEntryUserDefinedByteGet
(
    IN  GT_U32                                    flowMngId,
    IN  GT_U32                                    keyTableIndex,
    IN  GT_U32                                    udbIndex,
    OUT GT_U8                                     *keySizeInBytesPtr,
    OUT CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT         *offsetTypePtr,
    OUT GT_U8                                     *offsetPtr,
    OUT GT_U8                                     *maskPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerKeyEntryUserDefinedByteGet)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call  cpssDxChFlowManagerKeyEntryUserDefinedByteSet with all the correct params.
    Expected: GT_OK.
    1.5. Call  cpssDxChFlowManagerKeyEntryUserDefinedByteGet with all the correct params.
    Expected: GT_OK.
    1.6. Call with wrong value for offsetTypePtr [NULL].
    Expected: GT_BAD_PTR.
    1.7. Call with wrong value for offsetPtr [NULL].
    Expected: GT_BAD_PTR.
    1.8. Call with wrong value for maskPtr [NULL].
    Expected: GT_BAD_PTR.
    1.9. Call with out of range value for flowMngId.
    Expected: GT_BAD_PARAM.
    1.10. Call with wrong values of flowMngId.
    Expected: GT_NOT_FOUND.
    1.11. Remove device from flow manager.
    Expected: GT_OK;
    1.12. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT           offsetTypePtr;
    GT_U8                                       offsetPtr;
    GT_U8                                       maskPtr;
    GT_U32                                      keySizeInBytesPtr;
    GT_U32                                      keyTableIndex;
    GT_U32                                      udbIndex;
    CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT           offsetType;
    GT_U8                                       offsetOrPattern;
    GT_U8                                       mask;
    GT_U32                                      flowMngId;
    GT_U32                                      keySizeInBytes;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      notAppFamilyBmp = 0;
    GT_BOOL                                     isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    static GT_U32                               numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        flowMngId = 1;
        udbIndex = 3;
        keyTableIndex = 0;
        offsetType = CPSS_DXCH_FLOW_MANAGER_ANCHOR_L2_E;
        keySizeInBytes = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        offsetOrPattern = 14;
        mask = 0xFF;

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        /*
           1.5. Call with all correct params.
           Expected: GT_BAD_PARAM.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteGet(flowMngId, keyTableIndex, udbIndex, &keySizeInBytesPtr, &offsetTypePtr,
                                                           &offsetPtr, &maskPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        /*
           1.6. Call with wrong value for keySizeInBytesPtr [NULL].
           Expected: GT_BAD_PTR.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteGet(flowMngId, keyTableIndex, udbIndex, NULL,
                                                           &offsetTypePtr, &offsetPtr, &maskPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, flowMngId);

        /*
           1.6. Call with wrong value for offsetTypePtr [NULL].
           Expected: GT_BAD_PTR.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteGet(flowMngId, keyTableIndex, udbIndex, &keySizeInBytesPtr,
                                                           NULL, &offsetPtr, &maskPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, flowMngId);

        /*
           1.7. Call with wrong value for offsetPtr [NULL].
           Expected: GT_BAD_PTR.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteGet(flowMngId, keyTableIndex, udbIndex,&keySizeInBytesPtr,
                                                           &offsetTypePtr, NULL, &maskPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, flowMngId);

        /*
           1.8. Call with wrong value for maskPtr [NULL].
           Expected: GT_BAD_PTR.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteGet(flowMngId, keyTableIndex, udbIndex, &keySizeInBytesPtr, &offsetTypePtr,
                                                           &offsetPtr, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, flowMngId);

        /*
           1.9. Call with not allowed values of flowMngId.
           Expected: GT_BAD_PARAM.
         */
        flowMngId = PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1;
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteGet(flowMngId, keyTableIndex, udbIndex, &keySizeInBytesPtr, &offsetTypePtr,
                                                           &offsetPtr, &maskPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, flowMngId);

        /*
           1.10. Call with not used values of flowMngId.
           Expected: GT_NOT_FOUND.
         */
        flowMngId = 2;
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteGet(flowMngId, keyTableIndex, udbIndex, &keySizeInBytesPtr, &offsetTypePtr,
                                                           &offsetPtr, &maskPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, flowMngId);

        /* 1.11. remove the device
           Expected: GT_OK
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.12. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }
    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet
(
    IN GT_U32                                     devNum,
    IN GT_PORT_NUM                                sourcePort,
    IN GT_U32                                     keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E],
    IN GT_U32                                     enableBitmap
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet with all the correct params.
    Expected: GT_OK.
    1.5. Call cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet with not allowed value for source port.
    Expected: GT_BAD_PARAM.
    1.6. Call cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet with not allowed value for enableBitmap.
    Expected: GT_BAD_PARAM.
    1.7. Call cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet with not allowed value for keyIndexArr.
    Expected: GT_BAD_PARAM.
    1.8. Remove device from flow manager.
    Expected: GT_OK;
    1.9. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    GT_U32                                      sourcePort;
    GT_U32                                      enableBitmap;
    GT_U32                                      keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E];
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      notAppFamilyBmp = 0;
    GT_BOOL                                     isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        dev = devListArr[0];;
        sourcePort = 0;
        enableBitmap = 1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = 0;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] = 1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E] = 2;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E] = 3;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E] = 4;

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(dev, sourcePort,
                                                           keyIndexArr, enableBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        /*
           1.5. Call with not allowed value for source port.
           Expected: GT_OK.
         */
        sourcePort = 128;
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(dev, sourcePort,
                                                           keyIndexArr, enableBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, flowMngId);

        /*
           1.6. Call with not allowed value for enableBitmap.
           Expected: GT_OK.
         */
        sourcePort = 0;
        enableBitmap = BIT_5;
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(dev, sourcePort,
                                                           keyIndexArr, enableBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, flowMngId);

        /*
           1.7. Call with not allowed value for keyIndexArr.
           Expected: GT_OK.
         */
        sourcePort = 0;
        enableBitmap = 1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = 17;
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(dev, sourcePort,
                                                           keyIndexArr, enableBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, flowMngId);

        /* 1.8. remove the device
           Expected: GT_OK
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.9. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }
    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChIpfixFlowManagerSrcPortPktTypeToKeyIndexMappingGet
(
    IN  GT_U32                                     flowMngId,
    IN  GT_PORT_NUM                                sourcePort,
    OUT GT_U32                                     keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E],
    OUT GT_U32                                     *enableBitmapPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet with all the correct params.
    Expected: GT_OK.
    1.5. Call cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet with all the correct params.
    Expected: GT_OK.
    1.6. Remove device from flow manager.
    Expected: GT_OK;
    1.7. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    GT_U32                                      sourcePort;
    GT_U32                                      enableBitmap;
    GT_U32                                      enableBitmapPtr;
    GT_U32                                      keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E] = {0};
    GT_U32                                      keyIndexArrPtr;
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev;
    GT_U32                                      notAppFamilyBmp = 0;
    static GT_BOOL                              isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        sourcePort = 0;
        enableBitmap = 1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = 0;
        dev = devListArr[0];

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(dev, sourcePort,
                                                           keyIndexArr, enableBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        /*
           1.5. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet(dev, sourcePort,
                                                           &keyIndexArrPtr, &enableBitmapPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        /*
           1.6. Call with enableBitmapPtr as NULL.
           Expected: GT_BAD_PTR.
         */
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet(dev, sourcePort,
                                                           &keyIndexArrPtr, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, flowMngId);

        /* 1.7. remove the device
           Expected: GT_OK
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.8. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }
    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/* ISF for new flow */
static GT_U8 messagePacketNewFlowSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* ISF for removed flow*/
static GT_U8 messagePacketRemovedFlowSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x12, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};
#if 0
/* ISF for existent flow */
static GT_U8 messagePacketFlowDataSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* ISF for new and removed flow */
static GT_U8 messagePacketNewAndRemovedFlowSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x01, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xa0, 0xb0, 0x00, 0x00, 0x00, 0xff, 0x55, 0x55,
    0x55, 0x55
};

/* DATA of packet */
static GT_U8 messagePacketSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* DATA of packet */
static GT_U8 messagePacketSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* DATA of packet */
static GT_U8 messagePacketSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};
#endif

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerMessageParseAndCpssDbUpdate
(
    IN GT_U32                               devNum,
    INOUT GT_U32                            numOfBuff,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[],
)
*/

UTF_TEST_CASE_MAC(cpssDxChFlowManagerMessageParseAndCpssDbUpdate)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call with all correct params.
    Expected: GT_OK.
    1.5. Call with packetBuffsArrPtr [NULL] and other params from 1.4.
    Expected: GT_BAD_PTR
    1.6. Call with buffLenArr [NULL] and other params from 1.4.
    Expected: GT_BAD_PTR
    1.7. Remove device from flow manager.
    Expected: GT_OK;
    1.8. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    GT_U32                                      numOfBuff = 1;
    GT_U8                                       *packetBuffsArrPtr[1] = {messagePacketNewFlowSdmaBuffer};
    GT_U32                                      buffLenArr[1] = {sizeof(messagePacketNewFlowSdmaBuffer)};
    GT_U32                                      multiBuffLenArr[2] = {0,0};
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev = 0;
    GT_U32                                      notAppFamilyBmp = 0;
    static GT_BOOL                              isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
        */
        dev = devListArr[0];
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_FAIL;
        }
        else
        {
          st = GT_OK;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->messageNewFlowCount, 0x1, "New flow count %d"
                                     "different than in DB %d",0x1, flowMngDbPtr->messageNewFlowCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowDataDb[0].flowId, 0x0, "New flow id %d"
                                     "different than in DB %d",0x0, flowMngDbPtr->flowDataDb[0].flowId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowDataDb[0].keyId, 0x0, "New flow key id %d"
                                     "different than in DB %d",0x0, flowMngDbPtr->flowDataDb[0].keyId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowDataDb[0].keySize, 0x1e, "New flow key size %d"
                                     "different than in DB %d",0x1e, flowMngDbPtr->flowDataDb[0].keySize);

        packetBuffsArrPtr[0] = messagePacketRemovedFlowSdmaBuffer;
        buffLenArr[0] = sizeof(messagePacketRemovedFlowSdmaBuffer);
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);
        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_FAIL;
        }
        else
        {
          st = GT_OK;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->messageRemovedFlowCount, 0x1, "Removed flow count %d"
                                     "different than in DB %d",0x1, flowMngDbPtr->messageNewFlowCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[0].flowId, 0x0, "Removed flow id %d"
                                     "different than in DB %d",0x0, flowMngDbPtr->flowCounterDb[0].flowId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->removedFlowCounterDb[0].byteCount, 0x40, "Removed flow byte count %d"
                                     "different than in DB %d",0x40, flowMngDbPtr->removedFlowCounterDb[0].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->removedFlowCounterDb[0].packetCount, 0x01, "Removed flow packet count %d"
                                     "different than in DB %d",0x01, flowMngDbPtr->removedFlowCounterDb[0].packetCount);

        /*
           1.5. Call with packetBuffsArrPtr [NULL] and other params from 1.1.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, NULL, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, packetBuffsArrPtr = NULL", dev);

        /*
           1.5. Call with packetBuffsArrPtr[0] as NULL and other params from 1.1.
           Expected: GT_BAD_PTR.
        */
        packetBuffsArrPtr[0] = NULL;
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, packetBuffsArrPtr = NULL", dev);

        /*
           1.5. Call with packetBuffsArrPtr[0] as NULL and other params from 1.1.
           Expected: GT_BAD_PTR.
        */
        numOfBuff = 2;
        multiBuffLenArr[0] = 64;
        multiBuffLenArr[1] = 64;
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, multiBuffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, packetBuffsArrPtr = NULL", dev);

        /*
           1.6. Call with buffLenArr [NULL] and other params from 1.1.
           Expected: GT_BAD_PTR.
        */
        numOfBuff = 1;
        packetBuffsArrPtr[0] = messagePacketRemovedFlowSdmaBuffer;
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, buffLenArr = NULL", dev);

        /* 1.7. remove the device
           Expected: GT_OK
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.8. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
            flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_OK;
        }
        else
        {
          st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }
    if (isTestSkipped)
    {
      SKIP_TEST_MAC;
    }
}

/* ISF for new flow to track flows learnt per port ipv4 tcp */
static GT_U8 messagePacketIpv4TcpNewFlowLearntPerPortSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* FW */
/* ISF for new flow to track flows learnt per port ipv4 udp */
static GT_U8 messagePacketIpv4UdpNewFlowLearntPerPortSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x01,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x06, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerSrcPortFlowLearntGet
(
    IN  GT_U8                                                devNum,
    IN  GT_PORT_NUM                                          sourcePort,
    OUT CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC         *flowsLearntPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerSrcPortFlowLearntGet)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call with all the correct params.
    Expected: GT_OK.
    1.5. Call with wrong enumvalues for offsetType.
    Expected: GT_BAD_PARAM.
    1.6. Call with out of range values of offsetOrPattern for L2/L3/L4 anchor type.
    Expected: GT_OUT_OF_RANGE.
    1.7. Call with out of range value for flowMngId.
    Expected: GT_BAD_PARAM.
    1.8. Call with wrong values of flowMngId.
    Expected: GT_NOT_FOUND.
    1.9. Call with not usable udb index for tcam rule size 30B.
    Expected: GT_BAD_PARAM.
    1.10. Remove device from flow manager.
    Expected: GT_OK;
    1.11. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/

    CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT             offsetType;
    GT_U32                                        udbIndex;
    GT_U32                                        keyTableIndex;
    GT_U8                                         offsetOrPattern;
    GT_U8                                         mask;
    GT_U32                                        flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC       appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                    fwFlowMngInfo;
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         dev;
    GT_U32                                        notAppFamilyBmp = 0;
    GT_BOOL                                       isTestSkipped = GT_TRUE;
    GT_U32                                        utfFamilyBit = 0;
    GT_U32                                        numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC                *flowMngDbPtr;
    GT_U32                                        keySizeInBytes;
    GT_U32                                        numOfBuff = 1;
    GT_U8                                         *packetBuffsArrPtr[1] = {messagePacketIpv4TcpNewFlowLearntPerPortSdmaBuffer};
    GT_U32                                        buffLenArr[1] = {sizeof(messagePacketIpv4TcpNewFlowLearntPerPortSdmaBuffer)};
    CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC  flowsLearntPtr;
    GT_U32                                        sourcePort;
    GT_U32                                        enableBitmap;
    GT_U32                                        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E];

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));
    cpssOsMemSet(&flowsLearntPtr, 0, sizeof(CPSS_DXCH_FLOW_MANAGER_PORT_FLOWS_LEARNT_STC));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        udbIndex = 2;
        keyTableIndex = 0;
        keySizeInBytes = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        offsetType = CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E;
        offsetOrPattern = 0;
        mask = 0xFF;

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        dev = devListArr[0];
        sourcePort = 0x18;
        enableBitmap = 0x1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = 0;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] = 1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E] = 2;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E] = 3;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E] = 4;

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(dev, sourcePort,
                                                           keyIndexArr, enableBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
        */
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        st = cpssDxChFlowManagerSrcPortFlowLearntGet(dev, sourcePort, &flowsLearntPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, st, "dev %d, source port %d", dev, sourcePort);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearntPtr.ipv4TcpFlowsLearnt, 0x1, "IPV4 TCP flows learnt expected %d and got %d",
                                     0x1, flowsLearntPtr.ipv4TcpFlowsLearnt);

        /* IPCs treatment allowed only if FW is idle or after regular intervals
         * of 5ms and hence adding some delay for next set of configurations.
         */
        cpssOsTimerWkAfter(500);
        keyTableIndex = 1;
        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = 0;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] = 1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E] = 2;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E] = 3;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E] = 4;
        enableBitmap = 0x2;
        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(dev, sourcePort,
                                                           keyIndexArr, enableBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);
        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
        */
        packetBuffsArrPtr[0] = messagePacketIpv4UdpNewFlowLearntPerPortSdmaBuffer;
        buffLenArr[0] = sizeof(messagePacketIpv4UdpNewFlowLearntPerPortSdmaBuffer);
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        st = cpssDxChFlowManagerSrcPortFlowLearntGet(dev, sourcePort, &flowsLearntPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_NO_MORE, st, "dev %d, source port %d", dev, sourcePort);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowsLearntPtr.ipv4UdpFlowsLearnt, 0x1, "IPV4 UDP flows learnt expected %d and got %d",
                                     0x1, flowsLearntPtr.ipv4UdpFlowsLearnt);

        /* 1.10. remove the device
           Expected: GT_OK
        */
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.11. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/* ISF for new flow to track flows learnt per port */
static GT_U8 messagePacketiFirstNewFlowLearntPerPortSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x07, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* ISF for new flow to track flows learnt per port */
static GT_U8 messagePacketSecondNewFlowLearntPerPortSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerSrcPortFlowIdGetNext
(
    IN    GT_U8                                   devNum,
    IN    GT_PORT_NUM                             sourcePort,
    IN    GT_BOOL                                 getFirst,
    IN    GT_U32                                  *flowIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerSrcPortFlowIdGetNext)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call with all the correct params.
    Expected: GT_OK.
    1.5. Call with wrong enumvalues for offsetType.
    Expected: GT_BAD_PARAM.
    1.6. Call with out of range values of offsetOrPattern for L2/L3/L4 anchor type.
    Expected: GT_OUT_OF_RANGE.
    1.7. Call with out of range value for flowMngId.
    Expected: GT_BAD_PARAM.
    1.8. Call with wrong values of flowMngId.
    Expected: GT_NOT_FOUND.
    1.9. Call with not usable udb index for tcam rule size 30B.
    Expected: GT_BAD_PARAM.
    1.10. Remove device from flow manager.
    Expected: GT_OK;
    1.11. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/

    CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT             offsetType;
    GT_U32                                        udbIndex;
    GT_U32                                        keyTableIndex;
    GT_U8                                         offsetOrPattern;
    GT_U8                                         mask;
    GT_U32                                        flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC       appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                    fwFlowMngInfo;
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         dev;
    GT_U32                                        notAppFamilyBmp = 0;
    GT_BOOL                                       isTestSkipped = GT_TRUE;
    GT_U32                                        utfFamilyBit = 0;
    GT_U32                                        numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC                *flowMngDbPtr;
    GT_U32                                        keySizeInBytes;
    GT_U32                                        numOfBuff = 1;
    GT_U8                                         *packetBuffsArrPtr[1] = {messagePacketiFirstNewFlowLearntPerPortSdmaBuffer};
    GT_U32                                        buffLenArr[1] = {sizeof(messagePacketiFirstNewFlowLearntPerPortSdmaBuffer)};
    GT_U32                                        flowIdPtr;
    GT_U32                                        sourcePort;
    GT_U32                                        enableBitmap;
    GT_U32                                        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E];
    GT_BOOL                                       getFirst;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));
    cpssOsMemSet(&flowIdPtr, 0, sizeof(GT_U32));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        udbIndex = 2;
        keyTableIndex = 0;
        keySizeInBytes = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        offsetType = CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E;
        offsetOrPattern = 0;
        mask = 0xFF;

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        dev = devListArr[0];;
        sourcePort = 0x18;
        enableBitmap = 0x1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = 0;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] = 1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E] = 2;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E] = 3;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E] = 4;

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(dev, sourcePort,
                                                           keyIndexArr, enableBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
        */
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        packetBuffsArrPtr[0] = messagePacketSecondNewFlowLearntPerPortSdmaBuffer;
        buffLenArr[0] = sizeof(messagePacketSecondNewFlowLearntPerPortSdmaBuffer);

        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        getFirst = GT_TRUE;
        st = cpssDxChFlowManagerSrcPortFlowIdGetNext(dev, sourcePort, getFirst, &flowIdPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "dev %d, source port %d", dev, sourcePort);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowIdPtr, 0x1, "Flow Id learnt per port expected %d and got %d",
                                     0x1, flowIdPtr);
        getFirst = GT_FALSE;
        st = cpssDxChFlowManagerSrcPortFlowIdGetNext(dev, sourcePort, getFirst, &flowIdPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "dev %d, source port %d", dev, sourcePort);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowIdPtr, 0x0, "Flow Id learnt per port expected %d and got %d",
                                     0x0, flowIdPtr);

        /* 1.5. remove the device
           Expected: GT_OK
        */
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.11. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerReSync
(
    IN    GT_U32                   flowMngId
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerReSync)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.3. Call with all the correct params.
    Expected: GT_OK.
    1.4. Call with not allowed values of flowMngId.
    Expected: GT_BAD_PARAM.
    1.5. Call with not used values of flowMngId.
    Expected: GT_NOT_FOUND.
    1.6. Remove device from flow manager.
    Expected: GT_OK;
    1.7. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/

    GT_U32                                        flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC       appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                    fwFlowMngInfo;
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         dev;
    GT_U32                                        notAppFamilyBmp = 0;
    GT_BOOL                                       isTestSkipped = GT_TRUE;
    GT_U32                                        utfFamilyBit = 0;
    GT_U32                                        numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC                *flowMngDbPtr;
    GT_U32                                        flowIdPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));
    cpssOsMemSet(&flowIdPtr, 0, sizeof(GT_U32));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.2. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        /*
           1.3. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerReSync(flowMngId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        /*
           1.4. Call with not allowed values of flowMngId.
           Expected: GT_BAD_PARAM.
         */
        flowMngId = PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1;
        st = cpssDxChFlowManagerReSync(flowMngId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, flowMngId);

        /*
           1.5. Call with not used values of flowMngId.
           Expected: GT_NOT_FOUND.
         */
        flowMngId = 2;
        st = cpssDxChFlowManagerReSync(flowMngId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, flowMngId);


        /* 1.6. remove the device
           Expected: GT_OK
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.7. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/* ISF for new flow */
static GT_U8 messagePacketStatsNewFlowSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x09, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* ISF for removed flow*/
static GT_U8 messagePacketStatsRemovedFlowSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x12, 0x34, 0x22, 0x56, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* ISF for new flow with incorrect sequence number */
static GT_U8 messagePacketStatsNewFlow2SdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x03, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x12, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};


/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerMessagePacketStatisticsGet
(
    IN  GT_U32                                                flowMngId,
    OUT CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC  *messagePacketStatsPtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChFlowManagerMessagePacketStatisticsGet)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call with all correct params.
    Expected: GT_OK.
    1.5. Call with packetBuffsArrPtr [NULL] and other params from 1.4.
    Expected: GT_BAD_PTR
    1.6. Call with buffLenArr [NULL] and other params from 1.4.
    Expected: GT_BAD_PTR
    1.7. Remove device from flow manager.
    Expected: GT_OK;
    1.8. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    GT_U32                                      numOfBuff = 1;
    GT_U8                                       *packetBuffsArrPtr[1] = {messagePacketStatsNewFlowSdmaBuffer};
    GT_U32                                      buffLenArr[1] = {sizeof(messagePacketStatsNewFlowSdmaBuffer)};
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev = 0;
    GT_U32                                      notAppFamilyBmp = 0;
    static GT_BOOL                              isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    GT_U32                                      firstTimeStamp;
    GT_U32                                      lastTimeStamp;
    GT_U32                                      deltaSec;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;
    CPSS_DXCH_FLOW_MANAGER_MESSAGE_PACKET_STATISTICS_STC messagePacketStatsPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));
    cpssOsMemSet(&messagePacketStatsPtr, 0, sizeof(messagePacketStatsPtr));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
        */
        dev = devListArr[0];
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_FAIL;
        }
        else
        {
          st = GT_OK;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        st = cpssDxChFlowManagerMessagePacketStatisticsGet(flowMngId, &messagePacketStatsPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        UTF_VERIFY_EQUAL2_STRING_MAC(messagePacketStatsPtr.messageNewFlowCount, 0x1, "New flow count %d"
                                     "different than in DB %d",0x1, messagePacketStatsPtr.messageNewFlowCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(messagePacketStatsPtr.sequenceNumber, 0x1, "Sequence number %d"
                                     "different than in DB %d",0x1, messagePacketStatsPtr.sequenceNumber);

        packetBuffsArrPtr[0] = messagePacketStatsRemovedFlowSdmaBuffer;
        buffLenArr[0] = sizeof(messagePacketStatsRemovedFlowSdmaBuffer);
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);
        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_FAIL;
        }
        else
        {
          st = GT_OK;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        st = cpssDxChFlowManagerMessagePacketStatisticsGet(flowMngId, &messagePacketStatsPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        UTF_VERIFY_EQUAL2_STRING_MAC(messagePacketStatsPtr.messageRemovedFlowCount, 0x1, "Removed flow count %d"
                                     "different than in DB %d",0x1, messagePacketStatsPtr.messageRemovedFlowCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(messagePacketStatsPtr.sequenceNumber, 0x2, "Sequence number %d"
                                     "different than in DB %d",0x2, messagePacketStatsPtr.sequenceNumber);

        firstTimeStamp = flowMngDbPtr->flowCounterDb[0].firstTimeStamp >> 32;
        lastTimeStamp = flowMngDbPtr->flowCounterDb[0].lastTimeStamp >> 32;
        deltaSec = messagePacketStatsRemovedFlowSdmaBuffer[35] - messagePacketStatsRemovedFlowSdmaBuffer[33];
        UTF_VERIFY_EQUAL3_STRING_MAC(deltaSec, (lastTimeStamp-firstTimeStamp), "deltaSec %d, lastTs %d firstTs %d", deltaSec, lastTimeStamp, firstTimeStamp);

        packetBuffsArrPtr[0] = messagePacketStatsNewFlow2SdmaBuffer;
        buffLenArr[0] = sizeof(messagePacketStatsNewFlow2SdmaBuffer);
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_FAIL;
        }
        else
        {
          st = GT_OK;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        st = cpssDxChFlowManagerMessagePacketStatisticsGet(flowMngId, &messagePacketStatsPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        UTF_VERIFY_EQUAL2_STRING_MAC(messagePacketStatsPtr.reSyncCount, 0x1, "Re Sync count %d"
                                     "different than in DB %d",0x1, messagePacketStatsPtr.reSyncCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(messagePacketStatsPtr.sequenceNumber, 0x0, "Sequence number %d"
                                     "different than in DB %d",0x0, messagePacketStatsPtr.sequenceNumber);

        /*
           1.4. Call with not allowed values of flowMngId.
           Expected: GT_BAD_PARAM.
         */
        flowMngId = PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1;
        st = cpssDxChFlowManagerReSync(flowMngId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, flowMngId);

        /*
           1.5. Call with not used values of flowMngId.
           Expected: GT_NOT_FOUND.
         */
        flowMngId = 2;
        st = cpssDxChFlowManagerReSync(flowMngId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, flowMngId);

        /* 1.7. remove the device
           Expected: GT_OK
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.8. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
            flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_OK;
        }
        else
        {
          st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }
    if (isTestSkipped)
    {
      SKIP_TEST_MAC;
    }
}

/* ISF for new flow to track flows learnt globally */
static GT_U8 messagePacketFirstNewFlowLearntGloballySdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x0a, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* ISF for new flow to track flows learnt globally */
static GT_U8 messagePacketSecondNewFlowLearntGloballySdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x0b, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerFlowIdGetNext
(
    IN    GT_U8                                   devNum,
    IN    GT_BOOL                                 getFirst,
    IN    GT_U32                                  *flowIdPtr
)
*/
UTF_TEST_CASE_MAC(cpssDxChFlowManagerFlowIdGetNext)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call with all the correct params.
    Expected: GT_OK.
    1.5. Call with wrong enumvalues for offsetType.
    Expected: GT_BAD_PARAM.
    1.6. Call with out of range values of offsetOrPattern for L2/L3/L4 anchor type.
    Expected: GT_OUT_OF_RANGE.
    1.7. Call with out of range value for flowMngId.
    Expected: GT_BAD_PARAM.
    1.8. Call with wrong values of flowMngId.
    Expected: GT_NOT_FOUND.
    1.9. Call with not usable udb index for tcam rule size 30B.
    Expected: GT_BAD_PARAM.
    1.10. Remove device from flow manager.
    Expected: GT_OK;
    1.11. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/

    CPSS_DXCH_FLOW_MANAGER_ANCHOR_ENT             offsetType;
    GT_U32                                        udbIndex;
    GT_U32                                        keyTableIndex;
    GT_U8                                         offsetOrPattern;
    GT_U8                                         mask;
    GT_U32                                        flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC       appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                    fwFlowMngInfo;
    GT_STATUS                                     st = GT_OK;
    GT_U8                                         dev;
    GT_U32                                        notAppFamilyBmp = 0;
    GT_BOOL                                       isTestSkipped = GT_TRUE;
    GT_U32                                        utfFamilyBit = 0;
    GT_U32                                        numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC                *flowMngDbPtr;
    GT_U32                                        keySizeInBytes;
    GT_U32                                        numOfBuff = 1;
    GT_U8                                         *packetBuffsArrPtr[1] = {messagePacketFirstNewFlowLearntGloballySdmaBuffer};
    GT_U32                                        buffLenArr[1] = {sizeof(messagePacketFirstNewFlowLearntGloballySdmaBuffer)};
    GT_U32                                        flowIdPtr;
    GT_U32                                        sourcePort;
    GT_U32                                        enableBitmap;
    GT_U32                                        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_NUM_E];
    GT_BOOL                                       getFirst;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));
    cpssOsMemSet(&flowIdPtr, 0, sizeof(GT_U32));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        udbIndex = 2;
        keyTableIndex = 0;
        keySizeInBytes = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        offsetType = CPSS_DXCH_FLOW_MANAGER_ANCHOR_SRC_PORT_E;
        offsetOrPattern = 0;
        mask = 0xFF;

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        keyTableIndex = 1;
        st = cpssDxChFlowManagerKeyEntryUserDefinedByteSet(flowMngId, keyTableIndex, keySizeInBytes, udbIndex, offsetType,
                                                           offsetOrPattern, mask);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        dev = devListArr[0];;
        sourcePort = 0x18;
        enableBitmap = 0x3;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = 0;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] = 1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E] = 2;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E] = 3;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E] = 4;

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
         */
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(dev, sourcePort,
                                                           keyIndexArr, enableBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);

        sourcePort = 0x19;
        enableBitmap = 0x3;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_TCP_E] = 0;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV4_UDP_E] = 1;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_TCP_E] = 2;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_IPV6_UDP_E] = 3;
        keyIndexArr[CPSS_DXCH_FLOW_MANAGER_PACKET_TYPE_OTHER_E] = 4;
        st = cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet(dev, sourcePort,
                                                           keyIndexArr, enableBitmap);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, flowMngId);
        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
        */
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        packetBuffsArrPtr[0] = messagePacketSecondNewFlowLearntGloballySdmaBuffer;
        buffLenArr[0] = sizeof(messagePacketSecondNewFlowLearntGloballySdmaBuffer);

        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        getFirst = GT_TRUE;
        st = cpssDxChFlowManagerFlowIdGetNext(flowMngId, getFirst, &flowIdPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "dev %d, source port %d", dev, sourcePort);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowIdPtr, 0x1, "Flow Id learnt globally expected %d and got %d",
                                     0x1, flowIdPtr);
        getFirst = GT_FALSE;
        st = cpssDxChFlowManagerFlowIdGetNext(flowMngId, getFirst, &flowIdPtr);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, st, "dev %d, source port %d", dev, sourcePort);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowIdPtr, 0x0, "Flow Id learnt globally expected %d and got %d",
                                     0x0, flowIdPtr);

        /* 1.5. remove the device
           Expected: GT_OK
        */
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.11. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
                                     flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
            st = GT_OK;
        }
        else
        {
            st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }

    if (isTestSkipped)
    {
        SKIP_TEST_MAC;
    }
}

/* ISF for new flow */
static GT_U8 messagePacketFlowDataGetNewFlowSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* ISF for removed flow to store and retrieve flow data */
static GT_U8 messagePacketFlowDataGetRemovedFlowSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x12, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerFlowDataGet
(
    IN  GT_U32                                  flowMngId,
    IN  GT_U32                                  flowId,
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC   *flowEntryPtr,
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_STC         *flowDataPtr
)
*/

UTF_TEST_CASE_MAC(cpssDxChFlowManagerFlowDataGet)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call with all correct params.
    Expected: GT_OK.
    1.5. Call with packetBuffsArrPtr [NULL] and other params from 1.4.
    Expected: GT_BAD_PTR
    1.6. Call with buffLenArr [NULL] and other params from 1.4.
    Expected: GT_BAD_PTR
    1.7. Remove device from flow manager.
    Expected: GT_OK;
    1.8. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    GT_U32                                      numOfBuff = 1;
    GT_U8                                       *packetBuffsArrPtr[1] = {messagePacketFlowDataGetNewFlowSdmaBuffer};
    GT_U32                                      buffLenArr[1] = {sizeof(messagePacketFlowDataGetNewFlowSdmaBuffer)};
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev = 0;
    GT_U32                                      notAppFamilyBmp = 0;
    static GT_BOOL                              isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_ENTRY_STC   flowEntryPtr;
    OUT CPSS_DXCH_FLOW_MANAGER_FLOW_STC         flowDataPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));
    cpssOsMemSet(&flowEntryPtr, 0, sizeof(flowEntryPtr));
    cpssOsMemSet(&flowDataPtr, 0, sizeof(flowDataPtr));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
        */
        dev = devListArr[0];
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_FAIL;
        }
        else
        {
          st = GT_OK;
        }
        st = cpssDxChFlowManagerFlowDataGet(flowMngId, 0x0, &flowEntryPtr, &flowDataPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowDataPtr.keySize, 0x1e, "New flow key size %d"
                                     "different than in message packet %d",0x1e, flowDataPtr.keySize);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowEntryPtr.flowId, 0x0, "New flow id %d"
                                     "different than in message packet %d", 0x0, flowEntryPtr.flowId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowDataPtr.keyId, 0x0, "New flow key id %d"
                                     "different than in message packet %d",0x0, flowDataPtr.keyId);

        packetBuffsArrPtr[0] = messagePacketFlowDataGetRemovedFlowSdmaBuffer;
        buffLenArr[0] = sizeof(messagePacketFlowDataGetRemovedFlowSdmaBuffer);
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);
        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_FAIL;
        }
        else
        {
          st = GT_OK;
        }
        st = cpssDxChFlowManagerFlowDataGet(flowMngId, 0x0, &flowEntryPtr, &flowDataPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowDataPtr.keySize, 0x1e, "Removed flow key size %d"
                                     "different than in message packet %d",0x1e, flowDataPtr.keySize);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowEntryPtr.flowId, 0x0, "Removed flow id %d"
                                     "different than in message packet %d", 0x0, flowEntryPtr.flowId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowDataPtr.keyId, 0x0, "Removed flow key id %d"
                                     "different than in message packet %d",0x0, flowDataPtr.keyId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowEntryPtr.packetCount, 0x1, "Removed flow packet count %d"
                                     "different than in message packet %d",0x1, flowEntryPtr.byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowEntryPtr.byteCount, 0x40, "Removed flow byte count %d"
                                     "different than in message packet %d",0x40, flowEntryPtr.byteCount);

        st = cpssDxChFlowManagerFlowDataGet(flowMngId, 0x2, &flowEntryPtr, &flowDataPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NO_SUCH, st, "flowMngId = %d\n", flowMngId);

        /*
           1.5. Call with flowEntryPtr [NULL] and other correct params.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChFlowManagerFlowDataGet(flowMngId, 0x1, NULL, &flowDataPtr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, flowEntryPtr = NULL", dev);

        /*
           1.5. Call with flowDataPtr [NULL] and other params from 1.1.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChFlowManagerFlowDataGet(flowMngId, 0x1, &flowEntryPtr, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, flowDataPtr = NULL", dev);

        /*
           1.4. Call with not allowed values of flowMngId.
           Expected: GT_BAD_PARAM.
         */
        flowMngId = PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1;
        st = cpssDxChFlowManagerFlowDataGet(flowMngId, 0x0, &flowEntryPtr, &flowDataPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, flowMngId);

        /*
           1.5. Call with not used values of flowMngId.
           Expected: GT_NOT_FOUND.
         */
        flowMngId = 2;
        st = cpssDxChFlowManagerFlowDataGet(flowMngId, 0x0, &flowEntryPtr, &flowDataPtr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, flowMngId);

        /* 1.7. remove the device
           Expected: GT_OK
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.8. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
            flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_OK;
        }
        else
        {
          st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }
    if (isTestSkipped)
    {
      SKIP_TEST_MAC;
    }
}

/* ISF for new flow */
static GT_U8 messagePacketFlowCountersToDbGetNewFlowSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/* ISF for removed flow to store and retrieve flow data */
static GT_U8 messagePacketFlowCountersToDbGetFlowDataSdmaBuffer[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x12, 0x00, 0x00, 0x00, 0xff,
    0x55, 0x55, 0x55, 0x55
};

/*----------------------------------------------------------------------------*/
/*
GT_STATUS cpssDxChFlowManagerFlowCountersToDbGet
(
    IN  GT_U32                                  flowMngId,
    IN  GT_U32                                  flowIdCount,
    IN  GT_U32                                  flowIdArr[]
)
*/

UTF_TEST_CASE_MAC(cpssDxChFlowManagerFlowCountersToDbGet)
{
/*
    1.1. Create Flow manager with correct parameters.
    Expected: GT_OK;
    1.2. Enable flow manager service.
    Expected: GT_OK;
    1.3. Add device to flow manager.
    Expected: GT_NOT_INITIALIZED;
    1.4. Call with all correct params.
    Expected: GT_OK.
    1.5. Call with flowIdArr [NULL] and other params from 1.4.
    Expected: GT_BAD_PTR
    1.6. Call with not allowed values of flowIdCount and other params from 1.4.
    Expected: GT_BAD_PARAM
    1.7. Call with not used values of flowMngId and other params from 1.4.
    Expected: GT_BAD_PARAM
    1.8. Call with not allowed values of flowMngId and other params from 1.4.
    Expected: GT_BAD_PARAM
    1.9. Remove device from flow manager.
    Expected: GT_OK;
    1.10. Delete the Flow Manager.
    Expected: GT_NOT_INITIALIZED;
*/
    GT_U32                                      numOfBuff = 1;
    GT_U8                                       *packetBuffsArrPtr[1] = {messagePacketFlowCountersToDbGetNewFlowSdmaBuffer};
    GT_U32                                      buffLenArr[1] = {sizeof(messagePacketFlowCountersToDbGetNewFlowSdmaBuffer)};
    GT_U32                                      flowMngId;
    CPSS_DXCH_FLOW_MANAGER_NEW_FLOW_CB_FUNC     appCbNewFlowNotify = NULL;
    CPSS_DXCH_FLOW_MANAGER_STC                  fwFlowMngInfo;
    GT_STATUS                                   st = GT_OK;
    GT_U8                                       dev = 0;
    GT_U32                                      notAppFamilyBmp = 0;
    static GT_BOOL                              isTestSkipped = GT_TRUE;
    GT_U32                                      utfFamilyBit = 0;
    GT_U32                                      numOfDevs = 0;
    GT_U32                                      flowIdArr[1] = {0};
    GT_U32                                      flowIdCount;
    PRV_CPSS_DXCH_FLOW_MANAGER_STC              *flowMngDbPtr;

    /* skip this test for all families under Golden Model */
    PRV_TGF_SKIP_LONG_TEST_GM_MAC(UTF_ALL_FAMILY_E);

    cpssOsMemSet(&fwFlowMngInfo, 0, sizeof(fwFlowMngInfo));

    /* this feature is currently on Aldrin2 device */
    UTF_E_ARCH_FLOW_MANAGER_SUPPORTED_FAMILY_GET_MAC(&notAppFamilyBmp);
    notAppFamilyBmp &= (~UTF_ALDRIN2_E);

    /* Look for devices - one device family at a time */
    for (utfFamilyBit = (UTF_PP_FAMILY_BIT__END_OF_DEVICES___E - 1); utfFamilyBit != UTF_NONE_FAMILY_E; utfFamilyBit >>= 1)
    {

        if ((utfFamilyBit & notAppFamilyBmp) != 0)
        {
            /* Not applicable device family - skip */
            continue;
        }

        /* prepare iterator for go over all active devices */
        PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(&dev, ~utfFamilyBit);

        /* Go over all active devices. */
        numOfDevs = 0;
        while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
        {
            devListArr[numOfDevs++] = dev;
        }

        if (numOfDevs == 0)
        {
            /* No device found - skip */
            continue;
        }

        /* Found applicable device - test is not skipped */
        isTestSkipped = GT_FALSE;
        flowMngId = 1;
        fwFlowMngInfo.cpssFwCfg.rxSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.txSdmaQ = 7;
        fwFlowMngInfo.cpssFwCfg.messagePacketCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
        fwFlowMngInfo.cpssFwCfg.cpuCodeStart = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        fwFlowMngInfo.cpssFwCfg.tcamStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.ipfixStartIndex = 0;
        fwFlowMngInfo.cpssFwCfg.tcamNumOfShortRules = 100;
        fwFlowMngInfo.cpssFwCfg.tcamShortRuleSize = CPSS_DXCH_FLOW_MANAGER_KEY_SIZE_30B_E;
        fwFlowMngInfo.cpssFwCfg.reducedRuleSize = 20;
        fwFlowMngInfo.cpssFwCfg.flowDBSize = 500;
        fwFlowMngInfo.cpssFwCfg.flowShortDBSize = 300;
        fwFlowMngInfo.flowDbSize = 2000;
        fwFlowMngInfo.appCbFlowNotificaion = appCbNewFlowNotify;

        /*
           1.1. Create Flow manager with correct parameters.
           Expected: GT_OK;
        */
        st = cpssDxChFlowManagerCreate(flowMngId, &fwFlowMngInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerCreate passed\n");

        /* 1.3. add the first device
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDevListAdd(flowMngId, devListArr, 1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDevListAdd\n");

        /*
           1.4. Call with all correct params.
           Expected: GT_OK.
        */
        dev = devListArr[0];
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_FAIL;
        }
        else
        {
          st = GT_OK;
        }

        packetBuffsArrPtr[0] = messagePacketFlowCountersToDbGetFlowDataSdmaBuffer;
        buffLenArr[0] = sizeof(messagePacketFlowCountersToDbGetFlowDataSdmaBuffer);
        st = cpssDxChFlowManagerMessageParseAndCpssDbUpdate(dev, numOfBuff, (GT_U8 **)packetBuffsArrPtr, buffLenArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "%d, flowMngId %d", flowMngId);
        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_FAIL;
        }
        else
        {
          st = GT_OK;
        }
        flowIdArr[0] = 0;
        flowIdCount = 1;
        st = cpssDxChFlowManagerFlowCountersToDbGet(flowMngId, flowIdCount, flowIdArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "flowMngId = %d\n", flowMngId);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->seqNoCpss, 0x2, "Flow data sequence number %d"
                                     "different than in message packet %d",0x2, flowMngDbPtr->seqNoCpss);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[0].byteCount, 0x40, "Flow data packet count %d"
                                     "different than in message packet %d",0x40, flowMngDbPtr->flowCounterDb[0].byteCount);
        UTF_VERIFY_EQUAL2_STRING_MAC(flowMngDbPtr->flowCounterDb[0].packetCount, 0x01, "Flow data byte count %d"
                                     "different than in message packet %d",0x01, flowMngDbPtr->flowCounterDb[0].packetCount);

        /*
           1.5. Call with flowIdArr [NULL] and other correct params.
           Expected: GT_BAD_PTR.
        */
        st = cpssDxChFlowManagerFlowCountersToDbGet(flowMngId, flowIdCount, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "%d, flowIdArr = NULL", dev);

        /*
           1.6. Call with not allowed values of flowIdCount and other correct params.
           Expected: GT_BAD_PTR.
        */
        flowIdCount = _8K + 1;
        st = cpssDxChFlowManagerFlowCountersToDbGet(flowMngId, flowIdCount, flowIdArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d, dev", dev);

        /*
           1.6. Call with not allowed values of flowIdCount and other correct params.
           Expected: GT_BAD_PTR.
        */
        flowIdCount = 0;
        st = cpssDxChFlowManagerFlowCountersToDbGet(flowMngId, flowIdCount, flowIdArr);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "%d, dev", dev);

        /*
           1.7. Call with not allowed values of flowMngId.
           Expected: GT_BAD_PARAM.
         */
        flowIdCount = 1;
        flowMngId = PRV_CPSS_DXCH_FLOW_MANAGER_MAX_ID_CNS + 1;
        st = cpssDxChFlowManagerFlowCountersToDbGet(flowMngId, flowIdCount, flowIdArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, flowMngId);

        /*
           1.8. Call with not used values of flowMngId.
           Expected: GT_NOT_FOUND.
         */
        flowMngId = 2;
        st = cpssDxChFlowManagerFlowCountersToDbGet(flowMngId, flowIdCount, flowIdArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_FOUND, st, flowMngId);

        /* 1.9. remove the device
           Expected: GT_OK
        */
        flowMngId = 1;
        st = cpssDxChFlowManagerDevListRemove(flowMngId, devListArr, numOfDevs);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, st, "cpssDxChFlowManagerDevListRemove failed for flowMngId = %d\n",
            flowMngId);

        /*
           1.10. Delete the Flow Manager
           Expected: GT_NOT_INITIALIZED
        */
        st = cpssDxChFlowManagerDelete(flowMngId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChFlowManagerDelete invoked in earlier call flowMngId = %d\n",
            flowMngId);

        flowMngDbPtr = prvCpssDxChFlowManagerDbFlowMngGet(flowMngId);
        if (flowMngDbPtr == NULL)
        {
          st = GT_OK;
        }
        else
        {
          st = GT_FAIL;
        }
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "Expected: Flow Manager deleted so no DB exists flowMngId = %d\n", flowMngId);
    }
    if (isTestSkipped)
    {
      SKIP_TEST_MAC;
    }
}

/*----------------------------------------------------------------------------*/
/*
 * Test of cpssDxChFlowManager suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChFlowManager)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerEnableGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerConfigurationParamsValidate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerCreate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerKeyEntryUserDefinedByteSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerKeyEntryUserDefinedByteGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerSrcPortPktTypeToKeyIndexMappingGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerMessageParseAndCpssDbUpdate)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerSrcPortFlowLearntGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerSrcPortFlowIdGetNext)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerReSync)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerMessagePacketStatisticsGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerFlowIdGetNext)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerFlowDataGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChFlowManagerFlowCountersToDbGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChFlowManager)
