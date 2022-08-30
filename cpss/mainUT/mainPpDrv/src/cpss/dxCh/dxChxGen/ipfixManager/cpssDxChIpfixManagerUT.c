/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file cpssDxChIpfixManager.c
*
* @brief Unit tests for cpssDxChIpfixManager, that provides
* The CPSS DXCH Ip HW structures APIs
*
* @version   1
********************************************************************************
*/

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <ipfixManager/prvAppIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/prvCpssDxChIpfixManager.h>
#include <cpss/dxCh/dxChxGen/ipfixManager/cpssDxChIpfixManager.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#define PRV_IPFIX_MANAGER_HW_TESTS_DISABLE 1

#if PRV_IPFIX_MANAGER_HW_TESTS_DISABLE
    #define PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC SKIP_TEST_MAC
#else
    #define PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC
#endif

/* Check cpssDxChIpfixManagerCreate and cpssDxChIpfixManagerDelete APIs */
/* GT_STATUS cpssDxChIpfixManagerCreate
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC  *attributesPtr
);
GT_STATUS cpssDxChIpfixManagerEntryDelete
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  flowId
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerCreateDelete)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC  attributes;
    GT_U8       cpuNum;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* check for all valid service cpu numbers */
        for (cpuNum = 16; cpuNum < 20; cpuNum++)
        {
            /* create ipfix manager */
            attributes.serviceCpuNum = cpuNum;
            st = cpssDxChIpfixManagerCreate(dev, &attributes);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixManagerCreate failed, dev: %d", dev);

            /* create ipfix manager for second time and expect an error */
            attributes.serviceCpuNum = cpuNum;
            st = cpssDxChIpfixManagerCreate(dev, &attributes);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_CREATE_ERROR, st, "cpssDxChIpfixManagerCreate failed, dev: %d", dev);

            /* create ipfix manager with null pointer */
            st = cpssDxChIpfixManagerCreate(dev, NULL);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st, "cpssDxChIpfixManagerCreate failed, dev: %d", dev);

            /* delete ipfix manager */
            st = cpssDxChIpfixManagerDelete(dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixManagerDelete failed, dev: %d", dev);

            /* try deleting second time and expect an error */
            st = cpssDxChIpfixManagerDelete(dev);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st, "cpssDxChIpfixManagerDelete failed, dev: %d", dev);

        }

        cpuNum = 20;
        attributes.serviceCpuNum = cpuNum;
        /* create ipfix manager with wrong service cpu number */
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st, "cpssDxChIpfixManagerCreate failed, dev : %d", dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    cpuNum = 16;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* create ipfix manager for non applicable device */
        attributes.serviceCpuNum = cpuNum;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                    "cpssDxChIpfixManagerCreate failed, dev: %d", dev);

        /* delete ipfix manager for non applicable device */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                    "cpssDxChIpfixManagerDelete failed, dev: %d", dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* create ipfix manager with invalid device number */
    attributes.serviceCpuNum = cpuNum;
    st = cpssDxChIpfixManagerCreate(dev, &attributes);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st, "cpssDxChIpfixManagerCreate failed, dev: %d", dev);

    /* delete ipfix manager with invalid device number */
    st = cpssDxChIpfixManagerDelete(dev);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                "cpssDxChIpfixManagerDelete failed, dev: %d", dev);
}

/* GT_STATUS cpssDxChIpfixManagerEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_BOOL      ipfixEnable
);*/
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerEnableSet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC  attributes;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Enable IPFIX Manager for second time */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Disable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Delete IPFIX Manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* Enable IPFIX Manager */
    st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);
}

/* GT_STATUS cpssDxChIpfixManagerGlobalConfigSet
(
    IN  GT_U8                                      devNum,
    IN  CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC  *globalCfgPtr
); */
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerGlobalConfigSet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_IPFIX_MANAGER_GLOBAL_CONFIG_STC  globalCfg;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC     attributes;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Global config Set */
        cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
        st = cpssDxChIpfixManagerGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChIpfixManagerGlobalConfigSet failed, dev: %d", dev);

        /* NULL pointer check */
        st = cpssDxChIpfixManagerGlobalConfigSet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                "cpssDxChIpfixManagerGlobalConfigSet failed, dev: %d", dev);

        /* queue number check with wrong value */
        cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
        globalCfg.ipfixDataQueueNum = PRV_CPSS_DXCH_SDMA_QUEUE_MAX_CNS;
        st = cpssDxChIpfixManagerGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
                                "cpssDxChIpfixManagerGlobalConfigSet failed, dev : %d", dev);

        /* idle timeout check with wrong value */
        cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
        globalCfg.idleTimeout = 5000; /* in seconds */
        st = cpssDxChIpfixManagerGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
                                "cpssDxChIpfixManagerGlobalConfigSet failed, dev : %d", dev);

        /* idle timeout check with wrong value */
        cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
        globalCfg.activeTimeout = 5000; /* in seconds */
        st = cpssDxChIpfixManagerGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
                                "cpssDxChIpfixManagerGlobalConfigSet failed, dev : %d", dev);

        /* active timeout check with wrong value */
        cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
        globalCfg.activeTimeout = 5000; /* in seconds */
        st = cpssDxChIpfixManagerGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
                                "cpssDxChIpfixManagerGlobalConfigSet failed, dev : %d", dev);

        /* idle timeout check with wrong value */
        cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
        globalCfg.dataPktMtu = 2000; /* in seconds */
        st = cpssDxChIpfixManagerGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
                                "cpssDxChIpfixManagerGlobalConfigSet failed, dev : %d", dev);

        /* delete ipfix manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixManagerDelete failed, dev: %d", dev);

        /* Global config Set */
        cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
        st = cpssDxChIpfixManagerGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                "cpssDxChIpfixManagerGlobalConfigSet failed, dev: %d", dev);

    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Global config Set */
        cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
        st = cpssDxChIpfixManagerGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                "cpssDxChIpfixManagerGlobalConfigSet failed, dev: %d", dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* Global config Set */
    cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
    st = cpssDxChIpfixManagerGlobalConfigSet(dev, &globalCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                            "cpssDxChIpfixManagerGlobalConfigSet failed, dev: %d", dev);
}

/* GT_STATUS cpssDxChIpfixManagerPortGroupConfigSet
(
    IN  GT_U8                                          devNum,
    IN  GT_PORT_GROUPS_BMP                             portGroupsBmp,
    IN  CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC  *portGroupCfgPtr
); */
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerPortGroupConfigSet)
{
    GT_STATUS               st = GT_OK;
    GT_U8                   dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId;
    CPSS_DXCH_IPFIX_MANAGER_PORT_GROUP_CONFIG_STC  portGroupCfg;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC     attributes;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Initialize port group. */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            /* configure correct maxIpfixIndex */
            cpssOsMemSet(&portGroupCfg, 0, sizeof(portGroupCfg));
            portGroupCfg.maxIpfixIndex = 2000;
            st = cpssDxChIpfixManagerPortGroupConfigSet(dev, portGroupsBmp, &portGroupCfg);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerPortGroupConfigSet failed, dev: %d", dev);

            /* configure wrong ipfix index and expect error */
            portGroupCfg.maxIpfixIndex = 5000;
            st = cpssDxChIpfixManagerPortGroupConfigSet(dev, portGroupsBmp, &portGroupCfg);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
                                    "cpssDxChIpfixManagerPortGroupConfigSet failed, dev: %d", dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)

        /* Null pointer check */
        st = cpssDxChIpfixManagerPortGroupConfigSet(dev, portGroupsBmp, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                "cpssDxChIpfixManagerPortGroupConfigSet failed, dev: %d", dev);

        /* delete ipfix manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixManagerDelete failed, dev: %d", dev);

        /* configure correct maxIpfixIndex */
        cpssOsMemSet(&portGroupCfg, 0, sizeof(portGroupCfg));
        portGroupCfg.maxIpfixIndex = 2000;
        st = cpssDxChIpfixManagerPortGroupConfigSet(dev, portGroupsBmp, &portGroupCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                "cpssDxChIpfixManagerPortGroupConfigSet failed, dev: %d", dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Initialize port group. */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            cpssOsMemSet(&portGroupCfg, 0, sizeof(portGroupCfg));
            st = cpssDxChIpfixManagerPortGroupConfigSet(dev, portGroupsBmp, &portGroupCfg);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                    "cpssDxChIpfixManagerPortGroupConfigSet failed, dev: %d", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
    }

    /* Initialize port group. */
    portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* port group config Set */
    cpssOsMemSet(&portGroupCfg, 0, sizeof(portGroupCfg));
    st = cpssDxChIpfixManagerPortGroupConfigSet(dev, portGroupsBmp, &portGroupCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                            "cpssDxChIpfixManagerPortGroupConfigSet failed, dev: %d", dev);
}

/* GT_STATUS cpssDxChIpfixManagerConfigGet
(
    IN  GT_U8                               devNum
); */
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerConfigGet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC     attributes;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Call IPC Message config Get */
        st = cpssDxChIpfixManagerConfigGet(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerConfigGet, dev: %d", dev);

        /* delete ipfix manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixManagerDelete failed, dev: %d", dev);

        /* Call IPC Message config Get */
        st = cpssDxChIpfixManagerConfigGet(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                    "cpssDxChIpfixManagerConfigGet, dev: %d", dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Call IPC Message config Get */
        st = cpssDxChIpfixManagerConfigGet(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                    "cpssDxChIpfixManagerConfigGet, dev: %d", dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* Call IPC Message config Get */
    st = cpssDxChIpfixManagerConfigGet(dev);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                "cpssDxChIpfixManagerConfigGet, dev: %d", dev);
}

/* GT_STATUS cpssDxChIpfixManagerEntryAdd
(
    IN  GT_U8                                    devNum,
    IN  GT_PORT_GROUPS_BMP                       portGroupsBmp,
    IN  CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC *entryParamsPtr,
    OUT GT_U32                                   *flowIdPtr
); */
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerEntryAdd)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC     attributes;
    CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC   entryParams;
    GT_U32                                     flowId;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Initialize port group. */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            /* entry add with correct params */
            cpssOsMemSet(&entryParams, 0, sizeof(entryParams));
            st = cpssDxChIpfixManagerEntryAdd(dev, portGroupsBmp, &entryParams, &flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpfixManagerEntryAdd failed, dev: %d", dev);

            /* Delete added flowId */
            st = cpssDxChIpfixManagerEntryDelete(dev,portGroupsBmp,flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpfixManagerEntryDelete failed, dev: %d", dev);

            /* entry add with correct params */
            cpssOsMemSet(&entryParams, 0, sizeof(entryParams));
            entryParams.firstTs = 2000;
            entryParams.firstTsValid = GT_TRUE;
            st = cpssDxChIpfixManagerEntryAdd(dev, portGroupsBmp, &entryParams, &flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpfixManagerEntryAdd failed, dev: %d", dev);

            /* Delete added flowId */
            st = cpssDxChIpfixManagerEntryDelete(dev,portGroupsBmp,flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpfixManagerEntryDelete failed, dev: %d", dev);

            /* entry add with Time stamp out of range */
            cpssOsMemSet(&entryParams, 0, sizeof(entryParams));
            entryParams.firstTs = CPSS_DXCH_IPFIX_MANAGER_FIRST_TS_MAX_CNS;
            entryParams.firstTsValid = GT_TRUE;
            st = cpssDxChIpfixManagerEntryAdd(dev, portGroupsBmp, &entryParams, &flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
                                         "cpssDxChIpfixManagerEntryAdd failed, dev: %d", dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)

        /* check null pointer */
        st = cpssDxChIpfixManagerEntryAdd(dev, portGroupsBmp, NULL, &flowId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "cpssDxChIpfixManagerEntryAdd failed, dev: %d", dev);
        st = cpssDxChIpfixManagerEntryAdd(dev, portGroupsBmp, &entryParams, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                     "cpssDxChIpfixManagerEntryAdd failed, dev: %d", dev);

        /* Disable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        cpssOsMemSet(&entryParams, 0, sizeof(entryParams));
        st = cpssDxChIpfixManagerEntryAdd(dev, portGroupsBmp, &entryParams, &flowId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_READY, st,
                                     "cpssDxChIpfixManagerEntryAdd failed, dev: %d", dev);

        /* delete ipfix manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixManagerDelete failed, dev: %d", dev);

        cpssOsMemSet(&entryParams, 0, sizeof(entryParams));
        st = cpssDxChIpfixManagerEntryAdd(dev, portGroupsBmp, &entryParams, &flowId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                     "cpssDxChIpfixManagerEntryAdd failed, dev: %d", dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Initialize port group. */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            cpssOsMemSet(&entryParams, 0, sizeof(entryParams));
            st = cpssDxChIpfixManagerEntryAdd(dev, portGroupsBmp, &entryParams, &flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                         "cpssDxChIpfixManagerEntryAdd failed, dev: %d", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixManagerEntryAdd(dev, portGroupsBmp, &entryParams, &flowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                 "cpssDxChIpfixManagerEntryAdd failed, dev: %d", dev);

}

/* GT_STATUS cpssDxChIpfixManagerEntryDelete
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  flowId
); */
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerEntryDelete)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC     attributes;
    GT_U32                                     flowId;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Initialize port group. */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            /* Delete non existing entry */
            flowId = 1024;
            st = cpssDxChIpfixManagerEntryDelete(dev, portGroupsBmp, flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_VALUE, st,
                                         "cpssDxChIpfixManagerEntryDelete failed, dev: %d", dev);

            /* Delete non existing entry */
            flowId = CPSS_DXCH_IPFIX_MANAGER_FLOWS_PER_PORT_GROUP_MAX_CNS;
            st = cpssDxChIpfixManagerEntryDelete(dev, portGroupsBmp, flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
                                         "cpssDxChIpfixManagerEntryDelete failed, dev: %d", dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)

        /* Disable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        st = cpssDxChIpfixManagerEntryDelete(dev, portGroupsBmp, flowId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_READY, st,
                                         "cpssDxChIpfixManagerEntryDelete failed, dev: %d", dev);

        /* delete ipfix manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixManagerDelete failed, dev: %d", dev);

        st = cpssDxChIpfixManagerEntryDelete(dev, portGroupsBmp, flowId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                         "cpssDxChIpfixManagerEntryDelete failed, dev: %d", dev);

    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Initialize port group. */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpfixManagerEntryDelete(dev,portGroupsBmp,flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                         "cpssDxChIpfixManagerEntryDelete failed, dev: %d", dev);
        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* Delete added flowId */
    st = cpssDxChIpfixManagerEntryDelete(dev,portGroupsBmp,flowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                 "cpssDxChIpfixManagerEntryDelete failed, dev: %d", dev);

}


/* GT_STATUS cpssDxChIpfixManagerEntryDeleteAll
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numDeletedPtr
); */
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerEntryDeleteAll)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC  attributes;
    GT_U32      numDeleted;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Call IPC Message EntryDeleteAll */
        st = cpssDxChIpfixManagerEntryDeleteAll(dev, &numDeleted);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEntryDeleteAll, dev: %d", dev);

        /* Call IPC Message EntryDeleteAll */
        st = cpssDxChIpfixManagerEntryDeleteAll(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                    "cpssDxChIpfixManagerEntryDeleteAll, dev: %d", dev);

        /* Disable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Delete IPFIX Manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Call IPC Message EntryDeleteAll */
        st = cpssDxChIpfixManagerEntryDeleteAll(dev, &numDeleted);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                    "cpssDxChIpfixManagerEntryDeleteAll, dev: %d", dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Call IPC Message EntryDeleteAll */
        st = cpssDxChIpfixManagerEntryDeleteAll(dev, &numDeleted);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                    "cpssDxChIpfixManagerEntryDeleteAll, dev: %d", dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* Call IPC Message EntryDeleteAll */
    st = cpssDxChIpfixManagerEntryDeleteAll(dev, &numDeleted);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                "cpssDxChIpfixManagerEntryDeleteAll, dev: %d", dev);
}

/* GT_STATUS cpssDxChIpfixManagerIpfixDataGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PORT_GROUPS_BMP                     portGroupsBmp,
    IN  GT_U32                                 flowId
); */
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerIpfixDataGet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_PORT_GROUPS_BMP      portGroupsBmp = 1;
    GT_U32                  portGroupId;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC     attributes;
    CPSS_DXCH_IPFIX_MANAGER_ENTRY_PARAMS_STC   entryParams;
    GT_U32                  flowId = 0;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Initialize port group. */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            /* entry add with correct params */
            cpssOsMemSet(&entryParams, 0, sizeof(entryParams));
            st = cpssDxChIpfixManagerEntryAdd(dev, portGroupsBmp, &entryParams, &flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpfixManagerEntryAdd failed, dev: %d", dev);

            st = cpssDxChIpfixManagerIpfixDataGet(dev, portGroupsBmp, flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                        "cpssDxChIpfixManagerIpfixDataGet failed, dev : %d", dev);

            /* Delete added flowId */
            st = cpssDxChIpfixManagerEntryDelete(dev,portGroupsBmp,flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                         "cpssDxChIpfixManagerEntryDelete failed, dev: %d", dev);

            flowId = CPSS_DXCH_IPFIX_MANAGER_FLOWS_PER_PORT_GROUP_MAX_CNS;
            st = cpssDxChIpfixManagerIpfixDataGet(dev, portGroupsBmp, flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OUT_OF_RANGE, st,
                                        "cpssDxChIpfixManagerIpfixDataGet failed, dev : %d", dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)

        /* Disable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* IPFIX manager needs to be enabled for this API call */
        st = cpssDxChIpfixManagerIpfixDataGet(dev, portGroupsBmp, flowId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_READY, st,
                                        "cpssDxChIpfixManagerIpfixDataGet failed, dev : %d", dev);

        /* delete ipfix manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st, "cpssDxChIpfixManagerDelete failed, dev: %d", dev);

        /* API call is not allowed after IPFIX Manager is deleted. Expect no initialized error*/
        st = cpssDxChIpfixManagerIpfixDataGet(dev, portGroupsBmp, flowId);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                        "cpssDxChIpfixManagerIpfixDataGet failed, dev : %d", dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Initialize port group. */
        portGroupsBmp = 1;

        /* 1.1. Go over all active port groups. */
        PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(dev, portGroupId)
        {
            /* Set next active port group. */
            portGroupsBmp = (1 << portGroupId);

            st = cpssDxChIpfixManagerIpfixDataGet(dev, portGroupsBmp, flowId);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                        "cpssDxChIpfixManagerIpfixDataGet failed, dev : %d", dev);

        }
        PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(dev, portGroupId)

    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIpfixManagerIpfixDataGet(dev, portGroupsBmp, flowId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                "cpssDxChIpfixManagerIpfixDataGet failed, dev : %d", dev);

}

/* GT_STATUS cpssDxChIpfixManagerIpfixDataGetAll
(
    IN  GT_U8                                  devNum
); */
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerIpfixDataGetAll)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC  attributes;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Call IPC Message dataGetAll */
        st = cpssDxChIpfixManagerIpfixDataGetAll(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerIpfixDataGetAll, dev: %d", dev);

        /* Disable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Call IPC Message dataGetAll */
        st = cpssDxChIpfixManagerIpfixDataGetAll(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_READY, st,
                                    "cpssDxChIpfixManagerIpfixDataGetAll, dev: %d", dev);
        /* Delete IPFIX Manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerDelete failed, dev : %d", dev);

        /* Call IPC Message dataGetAll */
        st = cpssDxChIpfixManagerIpfixDataGetAll(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                    "cpssDxChIpfixManagerIpfixDataGetAll, dev: %d", dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Call IPC Message dataGetAll */
        st = cpssDxChIpfixManagerIpfixDataGetAll(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                    "cpssDxChIpfixManagerIpfixDataGetAll, dev: %d", dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* Call IPC Message dataGetAll */
    st = cpssDxChIpfixManagerIpfixDataGetAll(dev);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                "cpssDxChIpfixManagerIpfixDataGetAll, dev: %d", dev);
}

/* GT_STATUS cpssDxChIpfixManagerIpcMsgFetch
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numOfMsgFetchedPtr
); */
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerIpcMsgFetch)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      numFetched = 0;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC  attributes;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Call IPC Message fetch */
        st = cpssDxChIpfixManagerIpcMsgFetch(dev, &numFetched);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerIpcMsgFetchAll, dev: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(0, numFetched,
                                    "Num Fetched is not as expected. dev: %d", dev);

        /* Call IPC Message config Get */
        st = cpssDxChIpfixManagerConfigGet(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerConfigGet, dev: %d", dev);

        cpssOsTimerWkAfter(100);

        /* Call IPC Message fetch */
        st = cpssDxChIpfixManagerIpcMsgFetch(dev, &numFetched);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerIpcMsgFetchAll, dev: %d", dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(1, numFetched,
                                    "Num Fetched is not as expected. dev: %d", dev);


        /* Call IPC Message fetch */
        st = cpssDxChIpfixManagerIpcMsgFetch(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                    "cpssDxChIpfixManagerIpcMsgFetchAll, dev: %d", dev);

        /* Disable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Delete IPFIX Manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Call IPC Message fetch */
        st = cpssDxChIpfixManagerIpcMsgFetch(dev, &numFetched);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                    "cpssDxChIpfixManagerIpcMsgFetchAll, dev: %d", dev);

    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Call IPC Message fetch */
        st = cpssDxChIpfixManagerIpcMsgFetch(dev, &numFetched);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                    "cpssDxChIpfixManagerIpcMsgFetch, dev: %d", dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* Call IPC Message fetch */
    st = cpssDxChIpfixManagerIpcMsgFetch(dev, &numFetched);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                "cpssDxChIpfixManagerIpcMsgFetch, dev: %d", dev);
}

/* GT_STATUS cpssDxChIpfixManagerIpcMsgFetchAll
(
    IN  GT_U8     devNum,
    OUT GT_U32    *numOfMsgFetchedPtr
); */
UTF_TEST_CASE_MAC(cpssDxChIpfixManagerIpcMsgFetchAll)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;
    GT_U32      numFetched = 0;
    CPSS_DXCH_IPFIX_MANAGER_ATTRIBUTES_STC  attributes;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_MANAGER_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Create IPFIX Manager */
        attributes.serviceCpuNum = 16;
        st = cpssDxChIpfixManagerCreate(dev, &attributes);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Call IPC Message fetch all */
        st = cpssDxChIpfixManagerIpcMsgFetchAll(dev, &numFetched);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerIpcMsgFetchAll, dev: %d", dev);

        /* Call IPC Message fetch all */
        st = cpssDxChIpfixManagerIpcMsgFetchAll(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                    "cpssDxChIpfixManagerIpcMsgFetchAll, dev: %d", dev);

        /* Enable IPFIX Manager */
        st = cpssDxChIpfixManagerEnableSet(dev, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerEnableSet failed, dev : %d", dev);

        /* Delete IPFIX Manager */
        st = cpssDxChIpfixManagerDelete(dev);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                    "cpssDxChIpfixManagerCreate failed, dev : %d", dev);

        /* Call IPC Message fetch all */
        st = cpssDxChIpfixManagerIpcMsgFetchAll(dev, &numFetched);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_INITIALIZED, st,
                                    "cpssDxChIpfixManagerIpcMsgFetchAll, dev: %d", dev);

    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, UTF_FALCON_E) ;

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Call IPC Message fetch all */
        st = cpssDxChIpfixManagerIpcMsgFetchAll(dev, &numFetched);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                    "cpssDxChIpfixManagerIpcMsgFetchAll, dev: %d", dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* Call IPC Message fetch all */
    st = cpssDxChIpfixManagerIpcMsgFetchAll(dev, &numFetched);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                "cpssDxChIpfixManagerIpcMsgFetchAll, dev: %d", dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChIpfixManager suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChIpfixManager)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerCreateDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerGlobalConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerPortGroupConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerEntryAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerEntryDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerEntryDeleteAll)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerIpfixDataGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerIpfixDataGetAll)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerIpcMsgFetch)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixManagerIpcMsgFetchAll)
UTF_SUIT_END_TESTS_MAC(cpssDxChIpfixManager)
