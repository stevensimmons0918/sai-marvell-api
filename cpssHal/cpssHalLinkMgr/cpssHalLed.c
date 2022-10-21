/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

//
// file cpssHalLed.c
//

#include "prvCpssBindFunc.h"
#include "cpssHalUtil.h"
#include "gtGenTypes.h"
#include "cpssHalDevice.h"
#include "cpssHalProfile.h"
#include "xpsCommon.h"
#include "cpssHalInit.h"
#include "cpss/common/cpssTypes.h"
#include "cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h"
#include "cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInitLedCtrl.h"
#include "cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h"

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


#define ALDRIN2_LED_IF_NUM_CNS   4

typedef struct
{
    GT_U32     ledIntf;
    GT_U32     ledStart;
    GT_U32     ledEnd;
} CPSSHAL_ALDRIN2XL_LED_STREAM_INDICATIONS_STC;

/* Led Stream Data for TG4810M */
/*
 * Class 1:10G Link - Controlled by SW (forced mode) / Default setting is Link status (for all speeds)
 * Class 2: Link and Activity - Link and TX or RX activity
 * LED0 - Unused
 * LED1 - mac_ports - 21,22,23 (3 ports)
 * LED2 - mac_ports - 48 - 69 , except 57 (21 ports)
 * LED3 - mac_ports - 24 - 47 (24 ports)
*/
static CPSSHAL_ALDRIN2XL_LED_STREAM_INDICATIONS_STC
aldrin2xl_TG4810M_led_stream[ALDRIN2_LED_IF_NUM_CNS] =
{
    {0, 0, 0},
    {1, 32, 64+12},
    {2, 32, 64+22},
    {3, 32, 64+24}
};

typedef struct
{
    GT_U32     ledStart;
    GT_U32     ledEnd;
    GT_BOOL    cpuPort;
} CPSSHAL_FALCON_LED_STREAM_INDICATIONS_STC;


static CPSSHAL_FALCON_LED_STREAM_INDICATIONS_STC
falcon_6_4_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 66, GT_TRUE },   /* Raven  0 - 2 LED ports + CPU Port */
    {64, 65, GT_FALSE},   /* Raven  1 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  2 - 2 LED ports */
    {64, 66, GT_TRUE },   /* Raven  3 - 2 LED ports + CPU Port */
    {64, 66, GT_TRUE },   /* Raven  4 - 2 LED ports + CPU Port */
    {64, 65, GT_FALSE},   /* Raven  5 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  6 - 2 LED ports */
    {64, 66, GT_TRUE },   /* Raven  7 - 2 LED ports + CPU Port */
};

static CPSSHAL_FALCON_LED_STREAM_INDICATIONS_STC
falcon_12_8_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 65, GT_FALSE},   /* Raven  0 - 2 LED ports */
    {64, 66, GT_TRUE },   /* Raven  1 - 2 LED ports + CPU Port */
    {64, 66, GT_TRUE },   /* Raven  2 - 2 LED ports + CPU Port */
    {64, 65, GT_FALSE},   /* Raven  3 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  4 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  5 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  6 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  7 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  8 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven  9 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 10 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 11 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 12 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 13 - 2 LED ports */
    {64, 65, GT_FALSE},   /* Raven 14 - 2 LED ports */
    {64, 65, GT_FALSE}    /* Raven 15 - 2 LED ports */
};

static CPSSHAL_FALCON_LED_STREAM_INDICATIONS_STC
falcon_2T_4T_led_indications[CPSS_CHIPLETS_MAX_NUM_CNS] =
{
    {64, 79, GT_FALSE},   /* Raven  0 - 16 LED ports */
    { 0,  0, GT_FALSE},   /* Raven  1 - not used */
    {64, 79, GT_FALSE},   /* Raven  2 - 16 LED ports */
    { 0,  0, GT_FALSE},   /* Raven  3 - not used */
    {64, 79, GT_FALSE},   /* Raven  4 - 16 LED ports */
    {64, 80, GT_TRUE },   /* Raven  5 - 16 LED ports + CPU Port */
    { 0,  0, GT_FALSE},   /* Raven  6 - not used */
    {64, 80, GT_TRUE },   /* Raven  7 - 16 LED ports + CPU Port */
};

CPSSHAL_FALCON_LED_STREAM_INDICATIONS_STC * ledStreamIndication;
static GT_U32 numOfTiles;
static GT_U32 cpuPorts[16];


#include "cpssHalLed.h"

GT_STATUS cpssHalLedInit(GT_U8 devNum, XP_DEV_TYPE_T devType,
                         LED_PROFILE_TYPE_E ledProfile)
{
    GT_STATUS rc         = GT_OK;
    GT_U32    ledIntf    = 0;
    GT_U32    numLedIntf = 1;

    if (devType == ALDRIN2XL || IS_DEVICE_FUJITSU_LARGE(devType))
    {
        numLedIntf = ALDRIN2_LED_IF_NUM_CNS;
    }

    for (ledIntf = 0; ledIntf < numLedIntf; ledIntf++)
    {
        rc = cpssHalLedConfig(devNum, ledIntf, devType, ledProfile);
        if (rc != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssHalLedConfig failed ledIntf %d, rc %d", ledIntf, rc);
        }
    }

    return rc;
}

GT_STATUS cpssHalLedConfig(GT_U8 devNum, GT_U32 ledInterfaceNum,
                           XP_DEV_TYPE_T devType, LED_PROFILE_TYPE_E ledProfile)
{
    CPSS_LED_CONF_STC ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_STATUS cpssRet;
    CPSS_DXCH_LED_PORT_TYPE_ENT portType;
    GT_U32  classNum;
    GT_U32 cpuPortMacFirst;
    int ledUnit = 0;

    cpssOsMemSet(&ledConfig, 0, sizeof(CPSS_LED_CONF_STC));
    cpssOsMemSet(cpuPorts, 0xFF, sizeof(cpuPorts));

    ledConfig.ledOrganize             = CPSS_LED_ORDER_MODE_BY_CLASS_E;
    ledConfig.disableOnLinkDown       = GT_TRUE;
    ledConfig.blink0DutyCycle         =
        CPSS_LED_BLINK_DUTY_CYCLE_0_E; /*  25% on, 75% off */
    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_3_E;
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_0_E;
    ledConfig.pulseStretch            = CPSS_LED_PULSE_STRETCH_1_E;
    ledConfig.ledStart                = 32;
    if (LED_PROFILE_97BIT_AC3X == ledProfile)
    {
        ledConfig.ledEnd                  = 97;
    }
    else
    {
        ledConfig.ledEnd                  = 65;
    }
    ledConfig.clkInvert               = GT_FALSE;
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E;
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E;
    ledConfig.invertEnable            = GT_FALSE;
    ledConfig.ledClockFrequency       = CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E;

    if (IS_DEVICE_FALCON(devType))
    {
        ledConfig.sip6LedConfig.ledClockFrequency   = 1627;
        ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */
        ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */
        ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */
        ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */

        for (ledUnit = 0; ledUnit < (4*4); ledUnit++)
        {
            int isCpuPortPresent = 0;
            //In  12.8 ledUnit 1 and 2 has CPU Port
            //In  6.4 ledUnit 0,3,4 and 7 has CPU Port
            if ((IS_DEVICE_FALCON_6_4(devType)) ||
                (IS_DEVICE_FALCON_3_2(devType)) ||
                (IS_DEVICE_FALCON_2(devType)))
            {
                if (!IS_DEVICE_FALCON_2(devType))
                {
                    ledStreamIndication = falcon_6_4_led_indications;
                }
                else
                {
                    ledStreamIndication = falcon_2T_4T_led_indications;
                }

                numOfTiles = 2;
                cpuPortMacFirst = 128;

                if (ledUnit == 0 || ledUnit == 3 || ledUnit == 4 || ledUnit == 7)
                {
                    isCpuPortPresent = 1;
                }
                if (ledUnit >= (numOfTiles * 4))
                {
                    break;
                }
            }
            else
            {
                ledStreamIndication = falcon_12_8_led_indications;
                numOfTiles = 4;
                cpuPortMacFirst     = 256;
                if (ledUnit == 1 || ledUnit == 2)
                {
                    isCpuPortPresent = 1;
                }

            }
            //TODO:
            //In 3.2 ledUnit 0,5 has CPU port
            ledConfig.sip6LedConfig.ledStart[ledUnit] = 64;
            ledConfig.sip6LedConfig.ledEnd[ledUnit] = 65 + (isCpuPortPresent?1:0);
            if (isCpuPortPresent)
            {
                cpuPorts[ledUnit] = cpuPortMacFirst + ledUnit;
            }
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "cpssDxChLedStreamConfigSet dev %d ledUnit %d Start %d End %d",
                  devNum, ledUnit, ledConfig.sip6LedConfig.ledStart[ledUnit],
                  ledConfig.sip6LedConfig.ledEnd[ledUnit]);
        }
    }
    else if (devType == ALDRIN2XL)
    {
        ledConfig.ledStart                =
            aldrin2xl_TG4810M_led_stream[ledInterfaceNum].ledStart;
        ledConfig.ledEnd                  =
            aldrin2xl_TG4810M_led_stream[ledInterfaceNum].ledEnd;
    }
    else if (devType == TG48M_P)
    {
        ledConfig.ledEnd = 65;
    }

    cpssRet = cpssDxChLedStreamConfigSet(devNum, ledInterfaceNum, &ledConfig);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChLedStreamConfigSet dev %d  failed(%d)", devNum, cpssRet);
        return cpssRet;
    }

    ledClassManip.invertEnable            = GT_FALSE;
    ledClassManip.blinkEnable             = GT_TRUE;
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable             = GT_FALSE;
    ledClassManip.forceData               = 0;
    ledClassManip.pulseStretchEnable      = GT_TRUE;
    ledClassManip.disableOnLinkDown       = GT_TRUE;

    portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;
    for (classNum = 0 ; classNum < 6; classNum++)
    {
        if (classNum == 2)
        {
            ledClassManip.disableOnLinkDown       = GT_TRUE;
            ledClassManip.blinkEnable             = GT_TRUE;
        }
        else
        {
            ledClassManip.disableOnLinkDown       = GT_FALSE;
            ledClassManip.blinkEnable             = GT_FALSE;
        }


        cpssRet = cpssDxChLedStreamClassManipulationSet(devNum, ledInterfaceNum,
                                                        portType, classNum, &ledClassManip);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChLedStreamClassManipulationSet dev %d  failed(%d)", devNum,  cpssRet);
            return cpssRet;
        }
    }

    ledClassManip.invertEnable            = GT_FALSE;
    ledClassManip.blinkEnable             = GT_FALSE;
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable             = GT_TRUE;
    ledClassManip.forceData               = 0;
    ledClassManip.pulseStretchEnable      = GT_TRUE;
    ledClassManip.disableOnLinkDown       = GT_TRUE;

    if ((LED_PROFILE_97BIT_AC3X == ledProfile) || (devType == ALDRIN2XL) ||
        (devType == TG48M_P))
    {
        classNum = 1;
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;
        if ((devType == ALDRIN2XL) || (devType == TG48M_P))
        {
            ledClassManip.pulseStretchEnable      = GT_FALSE;
            ledClassManip.disableOnLinkDown       = GT_FALSE;
        }

        cpssRet = cpssDxChLedStreamClassManipulationSet(devNum, ledInterfaceNum,
                                                        portType, classNum, &ledClassManip);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChLedStreamClassManipulationSet dev %d  failed(%d)", devNum,
                  cpssRet);
            return cpssRet;
        }
    }

    if (LED_PROFILE_97BIT_AC3X == ledProfile)
    {
        classNum = 3;
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        cpssRet = cpssDxChLedStreamClassManipulationSet(devNum, ledInterfaceNum,
                                                        portType, classNum, &ledClassManip);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChLedStreamClassManipulationSet dev %d  failed(%d)", devNum,
                  cpssRet);
            return cpssRet;
        }
    }

    return GT_OK;
}

GT_BOOL isCpuPort(GT_U32 portMacNum)
{
    GT_U32 ii;
    for (ii = 0; ii < 16; ii++)
    {
        if (cpuPorts[ii] == portMacNum && portMacNum !=0)
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

GT_BOOL firstPortRaven(GT_U32 portMacNum)
{
    GT_STATUS rc = (((portMacNum / 8) % 2) == 0) ? GT_TRUE : GT_FALSE;

    return rc;
}


GT_BOOL ravenWithCpuPort(GT_U32 portMacNum)
{
    GT_U32 raven = portMacNum / 16;
    /* current mac not in Raven with connected CPU port */
    if (cpuPorts[raven] == 0xFFFFFFFF)
    {
        return GT_FALSE;
    }
    return GT_TRUE;
}


GT_STATUS cpssHalLedPortConfig(GT_U8 devNum, XP_DEV_TYPE_T devType,
                               GT_PHYSICAL_PORT_NUM portNum, GT_U32 position, LED_PROFILE_TYPE_E ledProfile)
{
    GT_U32  classNum;
    GT_STATUS cpssRet;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadowPtr;
    GT_U32 macNumber = 0;
    GT_U32 firstLedPosition;
    GT_U32 secondLedPosition;

    if (-1 == position) // Calculate the position
    {
        cpssRet = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum,
                                                         &portMapShadowPtr);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortPhysicalPortDetailedMapGet dev %d Port %d failed(%d)", devNum,
                  portNum, cpssRet);
            return cpssRet;
        }
        macNumber = portMapShadowPtr.portMap.macNum;

        if (IS_DEVICE_FALCON(devType))
        {

            if (isCpuPort(macNumber))
            {
                if (numOfTiles == 4)
                {
                    position = 2;
                }
                else
                {
                    position = 0;
                }
            }
            else
            {
                if (macNumber % 8 == 0)
                {
                    if (ravenWithCpuPort(macNumber) == GT_FALSE)
                    {
                        if (numOfTiles == 4)
                        {
                            firstLedPosition =  0;
                            secondLedPosition = 1;
                        }
                        else
                        {
                            firstLedPosition =  1;
                            secondLedPosition = 0;
                        }
                    }
                    else
                    {
                        if (numOfTiles == 4)
                        {
                            firstLedPosition =  0;
                            secondLedPosition = 1;
                        }
                        else
                        {
                            firstLedPosition =  2;
                            secondLedPosition = 1;
                        }
                    }

                    position = firstPortRaven(macNumber) ? firstLedPosition : secondLedPosition;
                }

            }

            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
                  "cpssDxChLedStreamPortPositionSet dev %d Port %d MacNum %d isCpuPort %d ravenCpuPort %d Position %d",
                  devNum, portNum, macNumber, isCpuPort(macNumber), ravenWithCpuPort(macNumber),
                  position);

        }
        else if (devType == ALDRIN2XL)
        {
            /*
            LED connectivity for ALDRIN2
            * LED0 chain is connected to Ports 0-11
            * LED1 chain is connected to Ports 12-23
            * LED2 chain is connected to Ports 48-71
            * LED3 chain is connected to Ports 24-47 + CPU (10gig ports)
            */
            if (macNumber < 12)
            {
                /* LED interface - 0 */
                position = macNumber;
            }
            else if (macNumber < 24)
            {
                /* LED interface - 1 */
                position = macNumber - 12;
            }
            else if (macNumber < 48)
            {
                /* LED interface - 3 */
                position = macNumber - 24;
            }
            else if (macNumber < 72)
            {
                /* LED interface - 2 */
                position = macNumber - 48;
            }
            else if (macNumber == 72)
            {
                /* LED interface - 3 */
                position = 24;
            }
        }
        else if (IS_DEVICE_FUJITSU_LARGE(devType))
        {
            /*
            LED connectivity for Fujitsu MCU and ALDRIN2XL Evaluation board
            * LED0 chain is connected to Ports 0-11
            * LED1 chain is connected to Ports 12-23
            * LED2 chain is connected to Ports 52-72
            * LED3 chain is connected to Ports 28-47
            */
            if (macNumber < 12)
            {
                /* LED interface - 0 */
                position = macNumber;
            }
            else if (macNumber < 24)
            {
                /* LED interface - 1 */
                position = macNumber - 12;
            }
            else if (macNumber < 32)
            {
                /* LED interface - 3 */
                position = macNumber - 28;
            }
            else if (macNumber < 44)
            {
                /* LED interface - 3 */
                position = macNumber - 32;
            }
            else if (macNumber < 48)
            {
                /* LED interface - 2 */
                position = macNumber - 36;
            }
            else if (macNumber < 56)
            {
                /* LED interface - 2 */
                position = macNumber - 52;
            }
            else if (macNumber < 64)
            {
                /* LED interface - 2 */
                position = macNumber - 56;
            }
            else if (macNumber < 72)
            {
                /* LED interface - 2 */
                position = macNumber - 60;
            }
        }
    }

    LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_DEFAULT,
          "cpssDxChLedStreamPortPositionSet dev %d Port %d MacNum %d Position %d",
          devNum, portNum, macNumber, position);

    if (-1 != position)
    {
        cpssRet = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChLedStreamPortPositionSet dev %d Port %d failed(%d)", devNum, portNum,
                  cpssRet);
            return cpssRet;
        }

        classNum = 2;
        cpssRet = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum, portNum,
                                                                    classNum, GT_TRUE);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChLedStreamPortClassPolarityInvertEnableSet dev %d Port %d failed(%d)",
                  devNum, portNum, cpssRet);
            return cpssRet;
        }
    }

    return GT_OK;
}

GT_STATUS cpssHalLedPortForcedStatus(GT_U8 cpssDevNum,
                                     GT_PHYSICAL_PORT_NUM portNum, CPSS_PORT_SPEED_ENT cpssSpeed, int linkState)
{
    GT_STATUS cpssRet;
    CPSS_DXCH_LED_PORT_TYPE_ENT portType;
    GT_U32  classNum;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    static uint32_t forced10GLinkStatus = 0;
    static uint32_t forced1GLinkStatus = 0;
    LED_PROFILE_TYPE_E ledProfile = cpssHalLedProfileGet(cpssDevNum);
    XP_DEV_TYPE_T devType;
    GT_U32 bitPos = 0;
    GT_U32 ledInterfaceNum = 0;

    cpssHalGetDeviceType(0, &devType);

    if (IS_DEVICE_FALCON(devType))
    {
        return GT_OK;
    }

    if (IS_DEVICE_AC3X(devType))
    {
        /* Skip non 10G ports */
        if ((portNum <= 48) || (portNum >= 53))
        {
            return GT_OK;
        }
    }
    if ((devType == ALDB2B) &&
        (ledProfile != LED_PROFILE_97BIT_AC3X))
    {
        return GT_OK;
    }

    if (devType == ALDRIN2XL)
    {
        CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadowPtr;
        GT_U32 portMacNum = 0;

        cpssRet = cpssDxChPortPhysicalPortDetailedMapGet(cpssDevNum, portNum,
                                                         &portMapShadowPtr);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortPhysicalPortDetailedMapGet dev %d Port %d failed(%d)", cpssDevNum,
                  portNum, cpssRet);
            return cpssRet;
        }

        portMacNum = portMapShadowPtr.portMap.macNum;
        if (portMacNum < 12)
        {
            ledInterfaceNum = 0;
        }
        else if (portMacNum < 24)
        {
            ledInterfaceNum = 1;
        }
        else if ((portMacNum < 48) ||
                 (portMacNum == 72))
        {
            ledInterfaceNum = 3;
        }
        else if (portMacNum < 72)
        {
            ledInterfaceNum = 2;
        }
    }

    if (devType == ALDRIN2XLFL || devType == ALDRIN2EVAL)
    {
        /* return GT_OK; */
        CPSS_DXCH_DETAILED_PORT_MAP_STC portMapShadowPtr;
        GT_U32 portMacNum = 0;

        cpssRet = cpssDxChPortPhysicalPortDetailedMapGet(cpssDevNum, portNum,
                                                         &portMapShadowPtr);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChPortPhysicalPortDetailedMapGet dev %d Port %d failed(%d)", cpssDevNum,
                  portNum, cpssRet);
            return cpssRet;
        }

        portMacNum = portMapShadowPtr.portMap.macNum;
        if (portMacNum < 12)
        {
            ledInterfaceNum = 0;
        }
        else if (portMacNum < 24)
        {
            ledInterfaceNum = 1;
        }
        else if ((portMacNum < 48) ||
                 (portMacNum == 72))
        {
            ledInterfaceNum = 3;
        }
        else if (portMacNum < 72)
        {
            ledInterfaceNum = 2;
        }
    }

    if (IS_DEVICE_FUJITSU_SMALL(devType))
    {
        return GT_OK;
    }

    cpssRet = cpssDxChLedStreamPortPositionGet(cpssDevNum, portNum, &bitPos);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChLedStreamPortPositionGet dev %d Port %d failed(%d)", cpssDevNum,
              portNum, cpssRet);
        return cpssRet;
    }

    switch (cpssSpeed)
    {
        case CPSS_PORT_SPEED_10000_E:
            if (linkState)
            {
                forced10GLinkStatus |= (1UL << bitPos);
            }
            else
            {
                forced10GLinkStatus &= ~(1UL << bitPos);
            }
            forced1GLinkStatus &= ~(1UL << bitPos);
            break;
        case CPSS_PORT_SPEED_1000_E:
        default:
            if (linkState)
            {
                forced1GLinkStatus |= (1UL << bitPos);
            }
            else
            {
                forced1GLinkStatus &= ~(1UL << bitPos);
            }
            forced10GLinkStatus &= ~(1UL << bitPos);
            break;
    }

    ledClassManip.invertEnable            = GT_FALSE;
    ledClassManip.blinkEnable             = GT_FALSE;
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable             = GT_TRUE;
    ledClassManip.pulseStretchEnable      = GT_TRUE;
    ledClassManip.disableOnLinkDown       = GT_TRUE;
    if ((devType == ALDRIN2XL) || (devType == TG48M_P))
    {
        ledClassManip.pulseStretchEnable      = GT_FALSE;
        ledClassManip.disableOnLinkDown       = GT_FALSE;
    }

    classNum = 1;
    portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;
    if ((devType == ALDRIN2XL) || (devType == TG48M_P))
    {
        // TG4810M Class 1: 0 - 10g Green, 1 - 1G Amber
        // TG48M-P Class 1: 0 - 10g Green, 1 - 1G Amber
        ledClassManip.forceData  = forced1GLinkStatus;
    }
    else
    {
        // IPD6448M Class 1: Link 10g Green
        ledClassManip.forceData = forced10GLinkStatus;
    }

    cpssRet = cpssDxChLedStreamClassManipulationSet(cpssDevNum, ledInterfaceNum,
                                                    portType, classNum, &ledClassManip);
    if (cpssRet != GT_OK)
    {
        LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
              "cpssDxChLedStreamClassManipulationSet dev %d  failed(%d)", cpssDevNum,
              cpssRet);
        return cpssRet;
    }

    if (ledProfile == LED_PROFILE_97BIT_AC3X)
    {
        // IPD6448M Class 3: Link 1g Amber
        classNum = 3;
        ledClassManip.forceData               = forced1GLinkStatus;
        portType = CPSS_DXCH_LED_PORT_TYPE_XG_E;

        cpssRet = cpssDxChLedStreamClassManipulationSet(cpssDevNum, ledInterfaceNum,
                                                        portType, classNum, &ledClassManip);
        if (cpssRet != GT_OK)
        {
            LOGFN(xpLogModXps, XP_SUBMOD_MAIN, XP_LOG_ERROR,
                  "cpssDxChLedStreamClassManipulationSet dev %d  failed(%d)", cpssDevNum,
                  cpssRet);
            return cpssRet;
        }
    }

    return GT_OK;
}

