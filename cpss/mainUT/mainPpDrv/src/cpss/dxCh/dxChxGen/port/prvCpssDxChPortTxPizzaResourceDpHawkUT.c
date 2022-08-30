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
* @file prvCpssDxChPortTxPizzaResourceDpHawkUT.c
 *
 * @brief Unit tests for CPSS internal functions,
 * that provides Hawk DP resources managment.
 *
* @version   1
********************************************************************************
*/
/* includes */
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceFalcon.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxPizzaResourceHawk.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define MAX_PORT_ARRAY_SIZE 128

typedef struct
{
    GT_PHYSICAL_PORT_NUM             port;
    CPSS_PORT_INTERFACE_MODE_ENT     ifMode;
    CPSS_PORT_SPEED_ENT              speed;
} PRV_CHANNEL_SPEED_STC;

static PRV_CHANNEL_SPEED_STC saveCfg[MAX_PORT_ARRAY_SIZE];

static void utilChannelCfgGet
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    OUT PRV_CHANNEL_SPEED_STC                     *readCfgPtr
)
{
    GT_STATUS                     rc;
    GT_PHYSICAL_PORT_NUM          port;
    CPSS_PORT_SPEED_ENT           speed;
    GT_U32                        channel;
    GT_U32                        channelIndex;
    GT_U32                        portUnitIndex;
    GT_U32                        macNum;
    CPSS_PORT_INTERFACE_MODE_ENT  portIfMode;

    channelIndex = 0;

    for (port = 0; (port < PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum)); port++)
    {
        rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
            devNum, port, &portUnitIndex, &channel);
        if (rc != GT_OK) continue;

        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(
            devNum, port, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &macNum);
        if (rc != GT_OK) continue;
        portIfMode = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portIfMode;

        rc = prvCpssDxChTxPortSpeedPizzaResourcesChannelSpeedByTxDmaProfileGet(
            devNum, project, portUnitIndex, channel, &speed);
        if (rc != GT_OK) continue;
        if (speed == CPSS_PORT_SPEED_NA_E) continue;

        readCfgPtr[channelIndex].port     = port;
        readCfgPtr[channelIndex].ifMode   = portIfMode;
        readCfgPtr[channelIndex].speed    = speed;

        channelIndex ++;
        if(channelIndex == (MAX_PORT_ARRAY_SIZE - 1))
        {
            /* avoid access violation ! */
            break;
        }
    }

    /* end mark */
    readCfgPtr[channelIndex].port     = 0xFFFFFFFF;
    readCfgPtr[channelIndex].ifMode   = CPSS_PORT_INTERFACE_MODE_NA_E;
    readCfgPtr[channelIndex].speed    = CPSS_PORT_SPEED_NA_E;
}

static void utilChannelCfgReset
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN  const PRV_CHANNEL_SPEED_STC               *resetCfgPtr
)
{
    GT_STATUS            rc;
    GT_U32               channelIndex;

    for (channelIndex = 0;
          (resetCfgPtr[channelIndex].speed != CPSS_PORT_SPEED_NA_E);
          channelIndex++)
    {
        rc = prvCpssDxChTxPortSpeedPizzaResourcesPortDpDown(
            devNum, resetCfgPtr[channelIndex].port, project);
        UTF_VERIFY_EQUAL2_PARAM_MAC(
            GT_OK, rc, devNum, resetCfgPtr[channelIndex].port);
    }
}

static void utilChannelCfgArrayPrint
(
    IN  const PRV_CHANNEL_SPEED_STC               *cfgPtr
)
{
    GT_U32               channelIndex;
    GT_U32               speedInMbSec;

    cpssOsPrintf("=================================================================\n");
    cpssOsPrintf("Port speeds: ");
    for (channelIndex = 0;
          (cfgPtr[channelIndex].speed != CPSS_PORT_SPEED_NA_E);
          channelIndex++)
    {
        speedInMbSec = prvCpssCommonPortSpeedEnumToMbPerSecConvert(
            cfgPtr[channelIndex].speed);
        cpssOsPrintf("%d - %dMB/S  ", cfgPtr[channelIndex].port, speedInMbSec);
    }
    cpssOsPrintf("\n");
}

static void utilChannelCfgSet
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project,
    IN  const PRV_CHANNEL_SPEED_STC               *setCfgPtr
)
{
    GT_STATUS            rc;
    GT_U32               channelIndex;

    for (channelIndex = 0;
          (setCfgPtr[channelIndex].speed != CPSS_PORT_SPEED_NA_E);
          channelIndex++)
    {
        rc = prvCpssDxChTxPortSpeedPizzaResourcesPortDpConfigure(
            devNum, setCfgPtr[channelIndex].port, project,
            setCfgPtr[channelIndex].ifMode,
            setCfgPtr[channelIndex].speed);
        UTF_VERIFY_EQUAL2_PARAM_MAC(
            GT_OK, rc, devNum, setCfgPtr[channelIndex].port);
    }
}

UTF_TEST_CASE_MAC(prvCpssDxChPorttTxPizzaResHawkDpSpeedCfg)
{
    GT_STATUS rc;
    GT_U8  dev;
    PRV_CPSS_DXCH_PIZZA_ARBITER_PROJECT_ENT   project;
    GT_U32 testIdx;
    GT_U32 unitTypeIdx;
    const PRV_CHANNEL_SPEED_STC** testsPtrArr;
    GT_U32 testsPtrArrSize;
    GT_U32 dupmUnitsBmp;
    GT_U32 channel;
    GT_U32 channelIndex;
    GT_U32 unitIndex;

    static const PRV_CHANNEL_SPEED_STC test0Cfg[] =
    {
        {0, CPSS_PORT_INTERFACE_MODE_KR8_E, CPSS_PORT_SPEED_400G_E},
        {0xFFFFFFFF, PRV_CPSS_PORT_NOT_EXISTS_E, CPSS_PORT_SPEED_NA_E}
    };
    static const PRV_CHANNEL_SPEED_STC test1Cfg[] =
    {
        { 0, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_50000_E},
        { 1, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_50000_E},
        { 2, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E},
        { 6, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_5000_E},
        {10, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E},
        {14, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_5000_E},
        {18, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E},
        {22, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_5000_E},
        {0xFFFFFFFF, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E}
    };
    static const PRV_CHANNEL_SPEED_STC test2Cfg[] =
    {
        {0,  CPSS_PORT_INTERFACE_MODE_KR4_E, CPSS_PORT_SPEED_200G_E},
        {10, CPSS_PORT_INTERFACE_MODE_KR4_E, CPSS_PORT_SPEED_200G_E},
        {0xFFFFFFFF, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E}
    };
    static const PRV_CHANNEL_SPEED_STC test3Cfg[] =
    {
        {0,  CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_100G_E},
        {2,  CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_100G_E},
        {10, CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_100G_E},
        {18, CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_100G_E},
        {0xFFFFFFFF, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E}
    };
    static const PRV_CHANNEL_SPEED_STC test4Cfg[] =
    {
        {0,  CPSS_PORT_INTERFACE_MODE_KR4_E, CPSS_PORT_SPEED_200G_E},
        {8,  CPSS_PORT_INTERFACE_MODE_KR4_E, CPSS_PORT_SPEED_200G_E},
        {0xFFFFFFFF, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E}
    };
    static const PRV_CHANNEL_SPEED_STC test5Cfg[] =
    {
        {0,  CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_100G_E},
        {8,  CPSS_PORT_INTERFACE_MODE_KR2_E, CPSS_PORT_SPEED_100G_E},
        {0xFFFFFFFF, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E}
    };
    static const PRV_CHANNEL_SPEED_STC* hawkTestsPtrArr[] =
    {
        test0Cfg, test1Cfg, test2Cfg, test3Cfg
    };
    static const GT_U32 hawkTestsPtrArrSize =
        (sizeof(hawkTestsPtrArr) / sizeof(hawkTestsPtrArr[0]));

    static const PRV_CHANNEL_SPEED_STC phoenixTest0Cfg[] =
    {
        {50, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_25000_E},
        {51, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_25000_E},
        {52, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_25000_E},
        {53, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_25000_E},
        {0xFFFFFFFF, CPSS_PORT_INTERFACE_MODE_NA_E, CPSS_PORT_SPEED_NA_E}
    };
    static const PRV_CHANNEL_SPEED_STC* phoenixTestsPtrArr[] =
    {
        phoenixTest0Cfg
    };
    static const GT_U32 phoenixTestsPtrArrSize =
        (sizeof(phoenixTestsPtrArr) / sizeof(phoenixTestsPtrArr[0]));

    static const PRV_CHANNEL_SPEED_STC* harrierTestsPtrArr[] =
    {
        test0Cfg, test4Cfg, test5Cfg
    };
    static const GT_U32 harrierTestsPtrArrSize =
        (sizeof(harrierTestsPtrArr) / sizeof(harrierTestsPtrArr[0]));


    static const PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_ENT      dumpUnitsArr[] =
    {
        PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_TX_DMA_E,
        PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PB_GPC_PKT_READ_E,
        PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_SFF_E,
        PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_PCA_BRG_TX_E
    };
    static const GT_U32 dumpUnitsArrSize =
        (sizeof(dumpUnitsArr) / sizeof(dumpUnitsArr[0]));

    /* prepare iterator for go over all active devices */
    PRV_TGF_NOT_APPLIC_DEV_RESET_MAC(
        &dev, UTF_CH1_E | UTF_CH1_DIAMOND_E | UTF_CH2_E | UTF_CH3_E |
        UTF_XCAT_E | UTF_XCAT3_E | UTF_AC5_E |UTF_XCAT2_E | UTF_LION_E | UTF_LION2_E |
        UTF_BOBCAT2_E | UTF_BOBCAT3_E | UTF_CAELUM_E | UTF_ALDRIN_E
        | UTF_AC3X_E | UTF_ALDRIN2_E | UTF_FALCON_E);

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        if (PRV_CPSS_SIP_6_20_CHECK_MAC(dev))
        {
            /*Harrier*/
            testsPtrArr     = harrierTestsPtrArr;
            testsPtrArrSize = harrierTestsPtrArrSize;
        }
        else if (PRV_CPSS_SIP_6_15_CHECK_MAC(dev))
        {
            /*Phoenix*/
            testsPtrArr     = phoenixTestsPtrArr;
            testsPtrArrSize = phoenixTestsPtrArrSize;
        }
        else
        {
            /*Hawk*/
            testsPtrArr     = hawkTestsPtrArr;
            testsPtrArrSize = hawkTestsPtrArrSize;
        }

        /* get list of affected DPs for dump */
        dupmUnitsBmp = 0;
        for (channelIndex = 0; (channelIndex < testsPtrArrSize); channelIndex++)
        {
            rc = prvCpssDxChGetTxIndexAndChannelIndexfromPortNum(
                dev, testsPtrArr[channelIndex]->port, &unitIndex, &channel);
            if (rc != GT_OK) continue;
            dupmUnitsBmp |= (1 << unitIndex);
        }

        rc = prvCpssDxChTxPortSpeedPizzaProjectGet(dev, &project);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* save configuration and reset */
        utilChannelCfgGet(dev, project, saveCfg);
        utilChannelCfgReset(dev, project, saveCfg);

        for (testIdx = 0; (testIdx < testsPtrArrSize); testIdx++)
        {
            /* set test configuration*/
            utilChannelCfgArrayPrint(testsPtrArr[testIdx]);
            utilChannelCfgSet(dev, project, testsPtrArr[testIdx]);

            for (unitTypeIdx = 0; (unitTypeIdx < dumpUnitsArrSize); unitTypeIdx++)
            {
                for (unitIndex = 0; (unitIndex < 32); unitIndex++)
                {
                    if (((1 << unitIndex) & dupmUnitsBmp) == 0) continue;
                    /* usefull dumps */
                    prvCpssDxChTxPortSpeedPizzaResourcesDump(
                        dev, dumpUnitsArr[unitTypeIdx], unitIndex);
                }
            }

            /* reset test configuration*/
            utilChannelCfgReset(dev, project, testsPtrArr[testIdx]);
        }
        /* restore saved configuration*/
        utilChannelCfgArrayPrint(saveCfg);
        utilChannelCfgSet(dev, project, saveCfg);
        for (unitTypeIdx = 0; (unitTypeIdx < dumpUnitsArrSize); unitTypeIdx++)
        {
            for (unitIndex = 0; (unitIndex < 32); unitIndex++)
            {
                if (((1 << unitIndex) & dupmUnitsBmp) == 0) continue;
                /* usefull dumps */
                prvCpssDxChTxPortSpeedPizzaResourcesDump(
                    dev, dumpUnitsArr[unitTypeIdx], unitIndex);
            }
        }
    }
}

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

/*
 * Configuration of prvCpssDxChPortTxPizzaResDpHawk suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(prvCpssDxChPortTxPizzaResDpHawk)
    UTF_SUIT_DECLARE_TEST_MAC(prvCpssDxChPorttTxPizzaResHawkDpSpeedCfg)
UTF_SUIT_END_TESTS_MAC(prvCpssDxChPortTxPizzaResDpHawk)


