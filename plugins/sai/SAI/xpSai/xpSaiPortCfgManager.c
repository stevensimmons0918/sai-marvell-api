// xpSaiPortCfgManager.c

/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
********************************************************************************/

#include "xpSaiPort.h"
#include "xpSaiPortCfgManager.h"
#include "xpSaiSwitch.h"
#include "xpSaiDev.h"
#include "saitypes.h"
#include "xpsUmac.h"
#include "xpsMac.h"
#include "xpSaiUtil.h"
#include "cpssHalDevice.h"

XP_SAI_LOG_REGISTER_API(SAI_API_PORT);

#define XPSAI_PORT_SERDES_DFE_WAIT_DELAY    1500
#define XPSAI_PORT_SERDES_RECHECK_DELAY     2000
#define XPSAI_PORT_LINK_STATUS_CHECK_DELAY  (XPSAI_PORT_SERDES_RECHECK_DELAY - 300)
#define XPSAI_PORT_SERDES_FINE_TUNING_DELAY 36000   // 50msec * 36000 = 1800secs (30min)

#define XPSAI_PORT_SYSTEM_MAX               MAX_PORTNUM

#define XPSAI_ADMIN_PORT_UP                 0
#define XPSAI_ADMIN_PORT_DOWN               1

#define xpSaiPortMgrLockAquire()            pthread_mutex_lock(&gSaiPortXdkMutex);
#define xpSaiPortMgrLockRelease()           pthread_mutex_unlock(&gSaiPortXdkMutex);

static xpSaiPortMgrInfo_t g_xpSaiPortMgrInfo[XPSAI_PORT_SYSTEM_MAX];

static pthread_mutex_t gSaiPortXdkMutex = PTHREAD_MUTEX_INITIALIZER;


xpSaiPortMgrInfo_t *xpSaiPortInfoGet(uint32_t portNum)
{
    if (portNum < XPSAI_PORT_SYSTEM_MAX)
    {
        return &g_xpSaiPortMgrInfo[portNum];
    }

    return NULL;
}

/**
 * \brief Get number of HW lanes used by port break-out mode
 *
 * \param [in] uint8_t breakOutMode - Port break-out mode
 *
 * \return Number of HW lanes used by break-out mode
 */
static uint8_t xpSaiGetUsedLaneCnt(uint8_t breakOutMode)
{
    if (breakOutMode == XPSAI_PORT_BREAKOUT_MODE_2X)
    {
        return 2;
    }
    else if (breakOutMode == XPSAI_PORT_BREAKOUT_MODE_4X)
    {
        return 4;
    }
    else if (breakOutMode == XPSAI_PORT_BREAKOUT_MODE_8X)
    {
        return 8;
    }
    else
    {
        return 1;
    }

}

/**
 * \brief Conversion of XP MAC mode to break-out mode
 *
 * \param [in] xpMacConfigMode macMode - XP MAC mode
 * \param [out] uint8_t breakoutMode - Break-out mode value 1x, 2x or 4x
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiPortMacModeToBreakoutMode(xpMacConfigMode macMode,
                                                uint8_t* breakoutMode)
{
    if (breakoutMode == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    switch (macMode)
    {
#if 0
        case MAC_MODE_4X40GB:
        case MAC_MODE_4X100GB:
            *breakoutMode = XPSAI_PORT_BREAKOUT_MODE_1X;
            break;
        case MAC_MODE_2X50GB:
            *breakoutMode = XPSAI_PORT_BREAKOUT_MODE_2X;
            break;
        case MAC_MODE_1GB:
        case MAC_MODE_1X10GB:
        case MAC_MODE_4X1GB:
        case MAC_MODE_4X10GB:
        case MAC_MODE_4X25GB:
        case MAC_MODE_1X50GB:
            *breakoutMode = XPSAI_PORT_BREAKOUT_MODE_4X;
            break;
        default:
            XP_SAI_LOG_ERR("Unsupported MAC mode %u.", macMode);
            *breakoutMode = XPSAI_PORT_BREAKOUT_MODE_NONE;
            break;
#endif
        case MAC_MODE_1GB:
        case MAC_MODE_2_5GB:
        case MAC_MODE_1X10GB:
        case MAC_MODE_4X10GB:
        case MAC_MODE_1X25GB:
        case MAC_MODE_1X50GB:
        case MAC_MODE_10M:
        case MAC_MODE_100M:
        case MAC_MODE_1000BASE_R:
        case MAC_MODE_1000BASE_X:
        case MAC_MODE_MIX_SGMII:
        case MAC_MODE_MIX_1000BASE_R:
        case MAC_MODE_MIX_4_CHANNEL_10G:
            *breakoutMode = XPSAI_PORT_BREAKOUT_MODE_1X;
            break;
        case MAC_MODE_2X50GB:
        case MAC_MODE_2X100GB:
            *breakoutMode = XPSAI_PORT_BREAKOUT_MODE_2X;
            break;
        case MAC_MODE_4X40GB:
        case MAC_MODE_4X100GB:
        case MAC_MODE_4X200GB:
            *breakoutMode = XPSAI_PORT_BREAKOUT_MODE_4X;
            break;
        case MAC_MODE_8X400GB:
            *breakoutMode = XPSAI_PORT_BREAKOUT_MODE_8X;
            break;
        default:
            XP_SAI_LOG_ERR("Unsupported MAC mode %u.", macMode);
            *breakoutMode = XPSAI_PORT_BREAKOUT_MODE_NONE;
            break;

    }

    return XP_NO_ERR;
}

/**
 * \brief Conversion of port speed value to XP MAC mode
 *
 * \param [in] uint32_t speed - Port speed
 *
 * \return XP MAC mode value
 */
static xpMacConfigMode xpSaiPortSpeedToMacMode(uint32_t speed)
{
    xpMacConfigMode macMode = MAC_MODE_MAX_VAL;

    switch (speed)
    {
        case XPSAI_PORT_SPEED_10MB:
            macMode = MAC_MODE_10M;
            break;
        case XPSAI_PORT_SPEED_100MB:
            macMode = MAC_MODE_100M;
            break;
        case XPSAI_PORT_SPEED_1000MB:
            macMode = MAC_MODE_1GB;
            break;
        case XPSAI_PORT_SPEED_2500MB:
            macMode = MAC_MODE_2_5GB;
            break;
        case XPSAI_PORT_SPEED_10G:
            macMode = MAC_MODE_1X10GB;
            break;
        case XPSAI_PORT_SPEED_25G:
            macMode = MAC_MODE_1X25GB;
            break;
        case XPSAI_PORT_SPEED_40G:
            macMode = MAC_MODE_4X40GB;
            break;
        case XPSAI_PORT_SPEED_50G:
            if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
            {
                macMode = MAC_MODE_1X50GB;    //doubt (should we check which device it is and the assign?)
            }
            else
            {
                macMode = MAC_MODE_2X50GB;
            }
            break;
        case XPSAI_PORT_SPEED_200G:
            macMode = MAC_MODE_4X200GB;
            break;
        case XPSAI_PORT_SPEED_400G:
            macMode = MAC_MODE_8X400GB;
            break;
        case XPSAI_PORT_SPEED_100G:
            if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
            {
                macMode = MAC_MODE_2X100GB;
            }
            else
            {
                macMode = MAC_MODE_4X100GB;
            }
            break;
        default:
            break;
    }

    return macMode;
}

/**
 * \brief Conversion of port FEC value to XP FEC mode
 *
 * \param [in] uint8_t fec - Port FEC
 *
 * \return XP FEC mode value
 */
static xpFecMode xpSaiPortFecToFecMode(uint8_t fec)
{
    xpFecMode fecMode = MAX_FEC_MODE;

    switch (fec)
    {
        case XPSAI_PORT_FEC_FC:
            fecMode = FC_FEC_MODE;
            break;
        case XPSAI_PORT_FEC_RS:
            fecMode = RS_FEC_MODE;
            break;
        case XPSAI_PORT_FEC_RS_544_514:
            fecMode = RS_544_514_FEC_MODE;
            break;
    }

    return fecMode;
}

/**
 * \brief SAI AN abilities encode to XP AN abilities
 *
 * This API is used to encode SAI AN and FEC attributes to XP AN and FEC register values
 *
 * \param [in] uint32_t advertFdAbility - SAI speed FullDuplex advertisement attributes
 * \param [in] uint32_t advertHdAbility - SAI speed HalfDuplex advertisement attributes
 * \param [in] uint32_t advertFec - SAI FEC advertisement attributes
 * \param [out] uint16_t abilities - XP ANability register value
 * \param [out] uint16_t errorControl - XP ANCtrl register value
 *
 * \return void
 */
static void xpSaiAutonegAbilitiesEncode(uint32_t portNum,
                                        uint32_t advertFdAbility,
                                        uint32_t advertHdAbility,
                                        uint32_t advertFec, uint32_t *abilities,
                                        uint16_t *errorControl)
{
    xpMacConfigMode macMode = MAC_MODE_MAX_VAL;
    XP_STATUS status = XP_NO_ERR;

    *abilities = 0;
    *errorControl = 0;
    status = xpsMacConfigModeGet(0, portNum, &macMode);
    if (XP_NO_ERR != status)
    {
        XP_SAI_LOG_ERR("Could not get MAC mode of the port %u.\n", portNum);
        return ;
    }


    XP_SAI_LOG_NOTICE("xpSaiAutonegAbilitiesEncode %u Advert Speed 0x%x Fec 0x%x \n",
                      portNum, advertFdAbility, advertFec);

    if (advertFdAbility & XPSAI_PORT_ADVERT_SPEED_10G)
    {
        *abilities |= IEEE_TECH_ABILITY_10GBASE_KX4_BIT
                      | IEEE_TECH_ABILITY_10GBASE_KR_BIT;

        if (advertFec & XPSAI_PORT_ADVERT_FEC_FC)
        {
            *errorControl |= UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                             | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT
                             | FC_FEC_SUPPORT;
        }

        if (advertFec & XPSAI_PORT_ADVERT_FEC_RS)
        {
            *errorControl |= UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                             | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT
                             | RS_FEC_SUPPORT;
        }
        //check if serdes lane if of 10G and return
        if (macMode == MAC_MODE_1X10GB)
        {
            XP_SAI_LOG_NOTICE("xpSaiAutonegAbilitiesEncode MAC_MODE_1X10GB %u Advert Ability 0x%x ErrCtrl 0x%x \n",
                              portNum, *abilities, *errorControl);
            return;
        }


    }

    if (advertFdAbility & XPSAI_PORT_ADVERT_SPEED_25G)
    {
        *abilities |= IEEE_TECH_ABILITY_25GBASE_KR1_BIT
                      | IEEE_TECH_ABILITY_25GBASE_CR1_BIT;

        if (advertFec & XPSAI_PORT_ADVERT_FEC_FC)
        {
            *errorControl |= UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                             | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT
                             | FC_FEC_SUPPORT;
        }

        if (advertFec & XPSAI_PORT_ADVERT_FEC_RS)
        {
            *errorControl |= UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                             | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT
                             | RS_FEC_SUPPORT;
        }
        //check if serdes lane if of 25G and return
        if (macMode == MAC_MODE_1X25GB)
        {
            XP_SAI_LOG_NOTICE("xpSaiAutonegAbilitiesEncode MAC_MODE_1X25GB %u Advert Ability 0x%x ErrCtrl 0x%x \n",
                              portNum, *abilities, *errorControl);
            return;
        }

    }

    if (advertFdAbility & XPSAI_PORT_ADVERT_SPEED_40G)
    {
        *abilities |= IEEE_TECH_ABILITY_40GBASE_KR4_BIT
                      | IEEE_TECH_ABILITY_40GBASE_CR4_BIT;

        if (advertFec & XPSAI_PORT_ADVERT_FEC_FC)
        {
            *errorControl = 0; //UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
            //| UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT;
        }
        if (macMode == MAC_MODE_4X40GB)
        {
            XP_SAI_LOG_NOTICE("xpSaiAutonegAbilitiesEncode MAC_MODE_4X40GB %u Advert Ability 0x%x ErrCtrl 0x%x \n",
                              portNum, *abilities, *errorControl);
            return;
        }
    }

    if (advertFdAbility & XPSAI_PORT_ADVERT_SPEED_50G)
    {
        {
            *abilities |= IEEE_TECH_ABILITY_50GBASE_KR_CR_BIT |
                          IEEE_TECH_ABILITY_50GBASE_KR2_BIT | IEEE_TECH_ABILITY_50GBASE_CR2_BIT;

            if ((advertFec & XPSAI_PORT_ADVERT_FEC_RS_544_514) ||
                (advertFec & XPSAI_PORT_ADVERT_FEC_RS))
            {
                *errorControl |= UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                                 | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT
                                 | UMAC_BPAN_AN_CTRL_FEC_MODE_RS_544_514;
            }
            else if (advertFec & XPSAI_PORT_ADVERT_FEC_RS)
            {
                *errorControl |= UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                                 | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT
                                 | RS_FEC_SUPPORT;

            }
        }

        if ((macMode == MAC_MODE_1X50GB) || (macMode == MAC_MODE_2X50GB))
        {
            XP_SAI_LOG_NOTICE("xpSaiAutonegAbilitiesEncode MAC_MODE_1/2X50GB %u Advert Ability 0x%x ErrCtrl 0x%x \n",
                              portNum, *abilities, *errorControl);
            return;
        }
    }

    if (advertFdAbility & XPSAI_PORT_ADVERT_SPEED_100G)
    {
        if ((advertFec & XPSAI_PORT_ADVERT_FEC_RS_544_514) ||
            (advertFec & XPSAI_PORT_ADVERT_FEC_RS))
        {
            *errorControl |= UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                             | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT
                             | RS_FEC_SUPPORT;
        }
        if (macMode == MAC_MODE_4X100GB)
        {
            *abilities |= IEEE_TECH_ABILITY_100GBASE_KR4_BIT |
                          IEEE_TECH_ABILITY_100GBASE_CR4_BIT;

            XP_SAI_LOG_NOTICE("xpSaiAutonegAbilitiesEncode MAC_MODE_4X100GB %u Advert Ability 0x%x ErrCtrl 0x%x \n",
                              portNum, *abilities, *errorControl);
            return;
        }
        else if (macMode == MAC_MODE_2X100GB)
        {
            *abilities |= IEEE_TECH_ABILITY_100GBASE_KR2_CR2_BIT;
            *errorControl |= UMAC_BPAN_AN_CTRL_FEC_MODE_RS_544_514;
            XP_SAI_LOG_NOTICE("xpSaiAutonegAbilitiesEncode MAC_MODE_2X100GB %u Advert Ability 0x%x ErrCtrl 0x%x \n",
                              portNum, *abilities, *errorControl);
            return;
        }
        /* For modes above 100, enable both 4x and 2x 100g modes */
        *abilities |= IEEE_TECH_ABILITY_100GBASE_KR4_BIT |
                      IEEE_TECH_ABILITY_100GBASE_CR4_BIT;
        *abilities |= IEEE_TECH_ABILITY_100GBASE_KR2_CR2_BIT;
        *errorControl |= UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT |
                         UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT |
                         RS_FEC_SUPPORT | UMAC_BPAN_AN_CTRL_FEC_MODE_RS_544_514;
    }

    if (advertFdAbility & XPSAI_PORT_ADVERT_SPEED_200G)
    {
        *abilities |= IEEE_TECH_ABILITY_200GBASE_KR4_CR4_BIT;

        if ((advertFec & XPSAI_PORT_ADVERT_FEC_RS_544_514) ||
            (advertFec & XPSAI_PORT_ADVERT_FEC_RS))
        {
            *errorControl |= UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                             | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT | UMAC_BPAN_AN_CTRL_FEC_MODE_RS_544_514;
        }
        if (macMode == MAC_MODE_4X200GB)
        {
            XP_SAI_LOG_NOTICE("xpSaiAutonegAbilitiesEncode MAC_MODE_4X200GB %u Advert Ability 0x%x ErrCtrl 0x%x \n",
                              portNum, *abilities, *errorControl);
            return;
        }
    }

    if (advertFdAbility & XPSAI_PORT_ADVERT_SPEED_400G)
    {
        *abilities |= IEEE_TECH_ABILITY_400GBASE_KR8_BIT;
        if (advertFec & XPSAI_PORT_ADVERT_FEC_RS_544_514)
        {
            *errorControl |= UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                             | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT
                             | UMAC_BPAN_AN_CTRL_FEC_MODE_RS_544_514;
        }
        if (macMode == MAC_MODE_8X400GB)
        {
            XP_SAI_LOG_NOTICE("xpSaiAutonegAbilitiesEncode MAC_MODE_8X400GB %u Advert Ability 0x%x ErrCtrl 0x%x \n",
                              portNum, *abilities, *errorControl);
            return;
        }
    }

    // Should not come here
    XP_SAI_LOG_WARNING("xpSaiAutonegAbilitiesEncode macmode:%d port:%u Advert Ability 0x%x ErrCtrl 0x%x \n",
                       macMode, portNum, *abilities, *errorControl);
    return;
}

/**
 * \brief XP AN abilities decode to SAI AN abilities
 *
 * This API is used to decode XP AN and FEC register values to SAI AN and FEC attributes
 *
 * \param [in] uint16_t abilities - XP ANability register value
 * \param [in] uint16_t errorControl - XP ANCtrl register value
 * \param [out] uint32_t advertFdAbility - SAI speed FullDuplex advertisement attributes
 * \param [out] uint32_t advertHdAbility - SAI speed HalfDuplex advertisement attributes
 * \param [out] uint32_t advertFec - SAI FEC advertisement attributes
 *
 * \return void
 */

static void xpSaiAutonegAbilitiesDecode(uint16_t abilities,
                                        uint16_t errorControl,
                                        uint32_t *advertFdAbility, uint32_t *advertHdAbility,
                                        uint32_t *advertFec)
{
    *advertFdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
    *advertHdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
    *advertFec = XPSAI_PORT_ADVERT_FEC_NONE;

    if (abilities & (IEEE_TECH_ABILITY_10GBASE_KX4_BIT
                     | IEEE_TECH_ABILITY_10GBASE_KR_BIT))
    {
        *advertFdAbility |= XPSAI_PORT_ADVERT_SPEED_10G;

        if (errorControl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                            | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
        {
            *advertFec |= XPSAI_PORT_ADVERT_FEC_FC;
        }
    }

    if (abilities & (IEEE_TECH_ABILITY_25GBASE_KR1_BIT
                     | IEEE_TECH_ABILITY_25GBASE_CR1_BIT))
    {
        *advertFdAbility |= XPSAI_PORT_ADVERT_SPEED_25G;

        if (errorControl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                            | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
        {
            if (errorControl & UMAC_BPAN_AN_CTRL_FEC_MODE_FC)
            {
                *advertFec |= XPSAI_PORT_ADVERT_FEC_FC;
            }
            *advertFec |= XPSAI_PORT_ADVERT_FEC_RS;
        }
    }

    if (abilities & (IEEE_TECH_ABILITY_40GBASE_KR4_BIT
                     | IEEE_TECH_ABILITY_40GBASE_CR4_BIT))
    {
        *advertFdAbility |= XPSAI_PORT_ADVERT_SPEED_40G;

        if (errorControl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                            | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
        {
            *advertFec |= XPSAI_PORT_ADVERT_FEC_FC;
        }
    }

    if (abilities & (IEEE_TECH_ABILITY_50GBASE_KR2_BIT
                     | IEEE_TECH_ABILITY_50GBASE_CR2_BIT))
    {
        *advertFdAbility |= XPSAI_PORT_ADVERT_SPEED_50G;

        if (errorControl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                            | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
        {
            if (errorControl & UMAC_BPAN_AN_CTRL_FEC_MODE_FC)
            {
                *advertFec |= XPSAI_PORT_ADVERT_FEC_FC;
            }
            *advertFec |= XPSAI_PORT_ADVERT_FEC_RS;
        }
    }

    if (abilities & (IEEE_TECH_ABILITY_100GBASE_KR4_BIT
                     | IEEE_TECH_ABILITY_100GBASE_CR4_BIT))
    {
        *advertFdAbility |= XPSAI_PORT_ADVERT_SPEED_100G;

        if (errorControl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                            | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
        {
            *advertFec |= XPSAI_PORT_ADVERT_FEC_RS;
        }
    }

    if (abilities & IEEE_TECH_ABILITY_50GBASE_KR_CR_BIT)
    {
        *advertFdAbility |= XPSAI_PORT_ADVERT_SPEED_50G;

        if (errorControl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                            | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
        {
            *advertFec |= XPSAI_PORT_ADVERT_FEC_RS_544_514;
        }
    }

    if (abilities & IEEE_TECH_ABILITY_100GBASE_KR2_CR2_BIT)
    {
        *advertFdAbility |= XPSAI_PORT_ADVERT_SPEED_100G;

        if (errorControl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                            | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
        {
            *advertFec |= XPSAI_PORT_ADVERT_FEC_RS_544_514;
        }
    }

    if (abilities & IEEE_TECH_ABILITY_200GBASE_KR4_CR4_BIT)
    {
        *advertFdAbility |= XPSAI_PORT_ADVERT_SPEED_200G;

        if (errorControl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                            | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
        {
            *advertFec |= XPSAI_PORT_ADVERT_FEC_RS_544_514;
        }
    }
    if (abilities & (IEEE_TECH_ABILITY_400GBASE_KR8_BIT
                     | IEEE_TECH_ABILITY_400GBASE_CR8_BIT))    //doubt
    {
        *advertFdAbility |= XPSAI_PORT_ADVERT_SPEED_400G;

        if (errorControl & (UMAC_BPAN_AN_CTRL_FEC_SUPPORTED_BIT
                            | UMAC_BPAN_AN_CTRL_FEC_REQUEST_BIT))
        {
            *advertFec |= XPSAI_PORT_ADVERT_FEC_RS_544_514;
        }
    }

}

/**
 * \brief Reads oper speed configuration for a given port
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 *
 * \return Current speed configuration or the port
 */
uint32_t xpSaiPortGetOperSpeed(xpsDevice_t devId, uint32_t portNum)
{
    XP_STATUS status = XP_NO_ERR;
    xpSpeed xpSpeed = SPEED_MAX_VAL;

    status = xpsMacGetPortOperSpeed(devId, portNum, &xpSpeed);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_WARNING("Could not get %u port speed. Err=%d", portNum, status);
    }

    switch (xpSpeed)
    {
        case SPEED_10MB:
            return XPSAI_PORT_SPEED_10MB;
        case SPEED_100MB:
            return XPSAI_PORT_SPEED_100MB;
        case SPEED_1GB:
            return XPSAI_PORT_SPEED_1000MB;
        case SPEED_2_5GB:
            return XPSAI_PORT_SPEED_2500MB;
        case SPEED_10GB:
            return XPSAI_PORT_SPEED_10G;
        case SPEED_25GB:
            return XPSAI_PORT_SPEED_25G;
        case SPEED_40GB:
            return XPSAI_PORT_SPEED_40G;
        case SPEED_50GB:
            return XPSAI_PORT_SPEED_50G;
        case SPEED_100GB:
            return XPSAI_PORT_SPEED_100G;
        case SPEED_200GB:
            return XPSAI_PORT_SPEED_200G;
        case SPEED_400GB:
            return XPSAI_PORT_SPEED_400G;
        default:
            break;
    }

    return XPSAI_PORT_SPEED_INVALID;
}



/**
 * \brief Reads current speed configuration for a given port
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 *
 * \return Current speed configuration or the port
 */
static uint32_t xpSaiPortGetSpeed(xpsDevice_t devId, uint32_t portNum)
{
    XP_STATUS status = XP_NO_ERR;
    xpSpeed xpSpeed = SPEED_MAX_VAL;

    status = xpsMacGetPortSpeed(devId, portNum, &xpSpeed);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_WARNING("Could not get %u port speed. Err=%d", portNum, status);
    }

    switch (xpSpeed)
    {
        case SPEED_10MB:
            return XPSAI_PORT_SPEED_10MB;
        case SPEED_100MB:
            return XPSAI_PORT_SPEED_100MB;
        case SPEED_1GB:
            return XPSAI_PORT_SPEED_1000MB;
        case SPEED_2_5GB:
            return XPSAI_PORT_SPEED_2500MB;
        case SPEED_10GB:
            return XPSAI_PORT_SPEED_10G;
        case SPEED_25GB:
            return XPSAI_PORT_SPEED_25G;
        case SPEED_40GB:
            return XPSAI_PORT_SPEED_40G;
        case SPEED_50GB:
            return XPSAI_PORT_SPEED_50G;
        case SPEED_100GB:
            return XPSAI_PORT_SPEED_100G;
        case SPEED_200GB:
            return XPSAI_PORT_SPEED_200G;
        case SPEED_400GB:
            return XPSAI_PORT_SPEED_400G;
        default:
            break;
    }

    return XPSAI_PORT_SPEED_INVALID;
}

/**
 * \brief Reads current FEC configuration for a given port
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 *
 * \return Current FEC configuration or the port
 */
static uint8_t xpSaiPortGetFec(xpsDevice_t devId, uint32_t portNum)
{
    XP_STATUS status = XP_NO_ERR;
    xpFecMode fecMode = MAX_FEC_MODE;

    status = xpsMacFecModeGet(devId, portNum, &fecMode);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_WARNING("Could not get FEC mode for port %u. Err=%d",
                           portNum, status);
    }

    /* Apply additional handling in case if FEC is enabled */
    switch (fecMode)
    {
        case FC_FEC_MODE:
            return XPSAI_PORT_FEC_FC;
        case RS_FEC_MODE:
            return XPSAI_PORT_FEC_RS;
        case RS_544_514_FEC_MODE:
            return XPSAI_PORT_FEC_RS_544_514;
        default:
            break;
    }

    return XPSAI_PORT_FEC_NONE;
}

/**
 * \brief Set mac and port loopback mode
 *
 * This API is used to set mac loopback mode and port loopback state
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint8_t loopbackMode
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiLoopbackModeSet(xpsDevice_t devId, uint32_t portNum,
                                      uint8_t loopbackMode, uint8_t enable)
{
    XP_STATUS status = XP_NO_ERR;

    if (loopbackMode == XPSAI_PORT_LOOPBACK_MODE_MAC)
    {
        status = xpsMacMacLoopback(devId, portNum, MAC_PCS_LEVEL_LOOPBACK, 1);
        if (XP_NO_ERR != status)
        {
            XP_SAI_LOG_ERR("Failed to disable loopback MAC mode for port %u\n", portNum);
            return status;
        }
    }

    else if (loopbackMode == XPSAI_PORT_LOOPBACK_MODE_PHY)
    {
        status = xpsMacPortLoopbackEnable(devId, portNum, 1);
        if (XP_NO_ERR != status)
        {
            XP_SAI_LOG_ERR("Failed to disable loopback PHY mode for port %u\n", portNum);
            return status;
        }
    }

    return status;
}

/**
 * \brief Validate FEC and speed compatibility
 *
 * This API is used to check if FEC mode and port speed settings are compatible.
 *  - RS-FEC can be enabled on 25G, 50G, 100G ports
 *  - FC-FEC can be enabled on 10G, 25G, 40G, 50G ports
 *
 * \param [in] uint8_t fec
 * \param [in] uint8_t speed
 *
 * \return bool - true if setting are compatible
 */
static bool xpSaiFecModeAndSpeedCheck(uint8_t fec, uint32_t speed)
{
    if (fec == XPSAI_PORT_FEC_NONE)
    {
        return true;
    }
    else if (fec == XPSAI_PORT_FEC_FC)
    {
        if ((speed == XPSAI_PORT_SPEED_10G) ||
            (speed == XPSAI_PORT_SPEED_25G) ||
            (speed == XPSAI_PORT_SPEED_40G) ||
            (speed == XPSAI_PORT_SPEED_50G) ||
            (speed == XPSAI_PORT_SPEED_100G))
        {
            return true;
        }
    }
    else if (fec == XPSAI_PORT_FEC_RS)
    {
        //AC5X48x1G6x10G profile needs FEC support for some 1G ports.
        if ((speed == XPSAI_PORT_SPEED_1000MB) ||
            (speed == XPSAI_PORT_SPEED_2500MB) ||
            (speed == XPSAI_PORT_SPEED_25G) ||
            (speed == XPSAI_PORT_SPEED_50G) ||
            (speed == XPSAI_PORT_SPEED_100G))
        {
            return true;
        }
    }
    else if (fec == XPSAI_PORT_FEC_RS_544_514)
    {
        if ((speed == XPSAI_PORT_SPEED_50G) ||
            (speed == XPSAI_PORT_SPEED_100G)||
            (speed == XPSAI_PORT_SPEED_200G)||
            (speed == XPSAI_PORT_SPEED_400G))
        {
            return true;
        }
    }

    return false;
}

/**
 * \brief Perform port SerDes tunning
 *
 * This API runs port serdes tuning state machine for fixed speed
 * port configuration using bellow diagram:
 *
 * -------------------------------------------------------------------Time------------------------------------------------------------------------->
 *
 * |-----------------|                                                                                         |-------------|  |---------------- |
 * | Get Link Status |                                                                                         | Delay 300ms |->| Get Link Status |
 * |-----------------|                                                                                       ->|-------------|  |-----------------|
 *                   |                                                                                       |
 *                   ->|--------------------------------------|                     |------------------------|
 *                     | Check link status?                   |                     | Check if Serdes Dfe is |
 *                     | "0" - Serdes re-check delay running? |                     | running?               |
 *                     |       "Yes" -> Idle.                 |                     | "Yes" -> Idle.         |
 *                     |       "No"  -> Check and Tune Serdes |                     | "No"  -> Reset Serdes  |
 *                     | "1" - Check and Tune Serdes          |                     |          signalOk      |
 *                     |--------------------------------------|                   ->|------------------------|
 *                                                            |                   |
 *                                                            ->|-----------------|
 *                                                              | Check if Serdes |
 *                                                              |  Dfe is running |
 *                                                              |-----------------|
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum
 * \param [in] uint32_t pollCycleTimeMs - pooling cycle time is milliseconds
 * \param [out] int32_t serdesRecheckDelay - internal counter used to delay SerDes re-tune in case if port link is down
 * \param [out] int32_t serdesTuneWait - internal counter used for SerDes tunning state machine
 * \param [out] uint32_t serdesFineTuneWait - internal counter used for SerDes tunning state machine
 * \param [out] uint8_t linkStatus
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiPortSerdesTune(xpsDevice_t devId, uint32_t portNum,
                                     uint32_t pollCycleTimeMs,
                                     int32_t *serdesRecheckDelay, int32_t *serdesTuneWait, uint32_t
                                     *serdesFineTuneWait, uint8_t *linkStatus)
{
    XP_STATUS status = XP_NO_ERR;
    xpFecMode fecMode = MAX_FEC_MODE;
    uint8_t isDfeRunning = 1;

    *linkStatus = 0;

    //Step1:: Do not tune SerDes for 1G speed
    //    if (xpSaiPortInfoGet(portNum)->speed <= XPSAI_PORT_SPEED_1000MB) //Disabled port serdes tune as cpss portmanager is already taking care of it
    {
        status = xpsMacGetLinkStatus(devId, portNum, linkStatus);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_WARNING("Could not get port %u link status. Err=%d\n",
                               portNum, status);
            return status;
        }
        return XP_NO_ERR;
    }

    //Step2:: Check for DFE tune in progress
    status = xpsMacPortSerdesDfeRunning(devId, portNum, &isDfeRunning);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_WARNING("Serdes Dfe running failed for port %u. Err=%d\n",
                           portNum, status);
        return status;
    }
#if 0
    if (isDfeRunning ==
        1) //As CPSS is not returning proper DFE state, isDfeRunning is not checked.
    {
        if (*serdesTuneWait == 0)
        {
            // DFE tuning Timeout
            // Try port init till it recovers for the max of 15 retries.
            int i, retry = 15;
            xpFecMode fecMode = xpSaiPortFecToFecMode(xpSaiPortInfoGet(portNum)->fec);
            xpMacConfigMode macMode = xpSaiPortSpeedToMacMode(xpSaiPortInfoGet(
                                                                  portNum)->speed);
            XP_SAI_LOG_ERR("ERROR: DFE tuning timeout port: %d\n", portNum);
            do
            {
                for (i = 0; i < xpSaiGetUsedLaneCnt(xpSaiPortInfoGet(portNum)->breakOutMode);
                     i++)
                {
                    if (xpSaiPortInfoGet(portNum)->hwLaneList[i] != XPSAI_PORT_HW_LANE_INVALID)
                    {
                        status = xpsMacPortDeInit(devId, xpSaiPortInfoGet(portNum)->hwLaneList[i]);
                        if (status != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("ERROR: Failed to deinit port #%u [%u]. Err=%d", portNum,
                                           xpSaiPortInfoGet(portNum)->hwLaneList[i], status);
                        }
                    }
                }
                status = xpsMacPortInitWithLinkStatusControl(devId, portNum,
                                                             macMode, 1, 0, 0, fecMode, (fecMode != MAX_FEC_MODE),
                                                             XPSAI_ADMIN_PORT_UP);
                if (status == XP_NO_ERR)
                {
                    // portInit success, no retries ...
                    break;
                }
                retry --;
            }
            while (retry>0);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_WARNING("Port INIT failed for port %u. Err=%d\n",
                                   portNum, status);
                return status;
            }
        }
        else
        {
            // Wait for DFE Tuning completion
            return XP_NO_ERR;
        }
    }
#endif

    //Step3:: Get Link Status
    status = xpsMacGetLinkStatus(devId, portNum, linkStatus);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_WARNING("Could not get port %u link status. Err=%d\n",
                           portNum, status);
        return status;
    }

    if ((*linkStatus == XPSAI_PORT_LINK_STATUS_UP) && (*serdesTuneWait != 0) &&
        (*serdesFineTuneWait < XPSAI_PORT_SERDES_FINE_TUNING_DELAY))
    {
        return XP_NO_ERR;
    }

    //Step4:: Check to do COARSE tune
    if (*serdesTuneWait == 0)
    {
        uint8_t portTuneStatus = 0;

        xpsLinkManagerCheckStatusAndTunePort(devId, portNum, &portTuneStatus);
        if (portTuneStatus)
        {
            *linkStatus = 0;
            *serdesRecheckDelay = XPSAI_PORT_SERDES_RECHECK_DELAY;
            *serdesTuneWait = XPSAI_PORT_SERDES_DFE_WAIT_DELAY;
            *serdesFineTuneWait = XPSAI_PORT_SERDES_FINE_TUNING_DELAY;
        }
        return XP_NO_ERR;
    }
    //Step5:: Check port DOWN
    if (*linkStatus == XPSAI_PORT_LINK_STATUS_DOWN)
    {
        status = xpsMacFecModeGet(devId, portNum, &fecMode);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_WARNING("Could not get FEC mode for port %u. Err=%d\n",
                               portNum, status);
        }
        /* Apply additional handling in case if FEC is enabled */
        if (fecMode != MAX_FEC_MODE)
        {
            status = xpsMacGetLinkStatus(devId, portNum, linkStatus);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_WARNING("Could not get port %u link status. Err=%d\n",
                                   portNum, status);
            }
            if (*linkStatus == XPSAI_PORT_LINK_STATUS_DOWN)
            {
                xpsMacPortSerdesSignalOverride(devId, portNum, XP_SERDES_INVERTED);
                xpSaiSleepMsec(5);
                xpsMacPortSerdesSignalOverride(devId, portNum, XP_SERDES_OK_LIVE);
            }
        }
        status = xpsMacGetLinkStatus(devId, portNum, linkStatus);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_WARNING("Could not get port %u link status. Err=%d\n",
                               portNum, status);
            return status;
        }

        if (*linkStatus == XPSAI_PORT_LINK_STATUS_DOWN)
        {
            *serdesTuneWait = *serdesTuneWait - pollCycleTimeMs;
            return XP_NO_ERR;
        }
    }
    //Step6:: Check to do FINE tune
    if (*linkStatus == XPSAI_PORT_LINK_STATUS_UP)
    {
        if (*serdesRecheckDelay != 0)
        {
            // Fine tuning
            *serdesFineTuneWait = *serdesFineTuneWait + 1; // Every +1 is +50msec delay
            if (*serdesFineTuneWait >= XPSAI_PORT_SERDES_FINE_TUNING_DELAY)
            {
                xpsPort_t ports[1];
                ports[0] = portNum;

                status = xpsMacPortSerdesTune(devId, ports, 1, XP_DFE_PCAL, 0);
                if (status != XP_NO_ERR)
                {
                    XP_SAI_LOG_WARNING("Serdes Fine tuning failed for port %u. Err=%d\n",
                                       portNum, status);
                }
                *serdesFineTuneWait = 0;
            }
        }
    }
    return XP_NO_ERR;
}

/**
 * \brief Update current port configuration with requested parameters
 *
 * \param [in] uint32_t portNum - Port number
 * \param [in] xpSaiPortMgrInfo_t *info - requested port configuration parameters
 * \param [in/out] uint32_t actionMask - Mask specifying parameters to be updated
 * \param [out] xpSaiPortMgrInfo_t *newPortInfo - port configuration composed of current port configuration and requested
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiUpdatePortInfo(uint32_t portNum,
                                     const xpSaiPortMgrInfo_t *info, uint32_t *actionMask,
                                     xpSaiPortMgrInfo_t *newPortInfo)
{
    XP_STATUS status = XP_NO_ERR;
    xpSaiPortMgrInfo_t *portInfo = xpSaiPortInfoGet(portNum);

    if (portInfo == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    memcpy(newPortInfo, portInfo, sizeof(xpSaiPortMgrInfo_t));

    if (*actionMask & XPSAI_PORT_ATTR_ENABLE_MASK)
    {
        if (portInfo->enable != info->enable)
        {
            newPortInfo->enable = info->enable;
        }
        else
        {
            *actionMask &= ~XPSAI_PORT_ATTR_ENABLE_MASK;
        }
    }

    if (*actionMask & XPSAI_PORT_ATTR_DUPLEX_MASK)
    {
        if (portInfo->duplex != info->duplex)
        {
            newPortInfo->duplex = info->duplex;
        }
        else
        {
            *actionMask &= ~XPSAI_PORT_ATTR_DUPLEX_MASK;
        }
    }

    if (*actionMask & XPSAI_PORT_ATTR_SPEED_MASK)
    {
        if (portInfo->speed != info->speed)
        {
            newPortInfo->speed = info->speed;
        }
        else
        {
            *actionMask &= ~XPSAI_PORT_ATTR_SPEED_MASK;
        }
    }

    if (*actionMask & XPSAI_PORT_ATTR_FEC_MASK)
    {
        if (portInfo->fec != info->fec)
        {
            newPortInfo->fec = info->fec;
        }
        else
        {
            *actionMask &= ~XPSAI_PORT_ATTR_FEC_MASK;
        }
    }

    if (*actionMask & XPSAI_PORT_ATTR_AUTONEG_MASK)
    {
        if (portInfo->autoneg != info->autoneg)
        {
            newPortInfo->autoneg = info->autoneg;
        }
        else
        {
            *actionMask &= ~XPSAI_PORT_ATTR_AUTONEG_MASK;
        }
    }

    if (*actionMask & XPSAI_PORT_ATTR_LOCAL_ADVERT_FD_MASK)
    {
        if (portInfo->localAdvertFdAbility != info->localAdvertFdAbility)
        {
            newPortInfo->localAdvertFdAbility = info->localAdvertFdAbility;
        }
        else
        {
            *actionMask &= ~XPSAI_PORT_ATTR_LOCAL_ADVERT_FD_MASK;
        }
    }

    if (*actionMask & XPSAI_PORT_ATTR_LOCAL_ADVERT_HD_MASK)
    {
        if (portInfo->localAdvertHdAbility != info->localAdvertHdAbility)
        {
            newPortInfo->localAdvertHdAbility = info->localAdvertHdAbility;
        }
        else
        {
            *actionMask &= ~XPSAI_PORT_ATTR_LOCAL_ADVERT_HD_MASK;
        }
    }

    if (*actionMask & XPSAI_PORT_ATTR_LOCAL_ADVERT_FEC_MASK)
    {
        if (portInfo->localAdvertFec != info->localAdvertFec)
        {
            newPortInfo->localAdvertFec = info->localAdvertFec;
        }
        else
        {
            *actionMask &= ~XPSAI_PORT_ATTR_LOCAL_ADVERT_FEC_MASK;
        }
    }

    if (*actionMask & XPSAI_PORT_ATTR_LOOPBACK_MASK)
    {
        if (portInfo->loopbackMode != info->loopbackMode)
        {
            newPortInfo->loopbackMode = info->loopbackMode;
        }
        else
        {
            *actionMask &= ~XPSAI_PORT_ATTR_LOOPBACK_MASK;
        }
    }

    if (*actionMask & XPSAI_PORT_ATTR_INTF_TYPE_MASK)
    {
        if (portInfo->intfType != info->intfType)
        {
            newPortInfo->intfType = info->intfType;
        }
        else
        {
            *actionMask &= ~XPSAI_PORT_ATTR_INTF_TYPE_MASK;
        }
    }

    return status;
}

/**
 * \brief Validate port configuration
 *
 * This API is used for various validations of port configuration parameter set.
 * Check if break-out mode, speed, FEC, advertisement abilities, etc. settings are compatible.
 *
 * \param [in] xpSaiPortMgrInfo_t *portInfo - port configuration structure
 *
 * \return bool -- true if configuration set is valid
 */
static bool xpSaiValidatePortInfo(const xpSaiPortMgrInfo_t *portInfo)
{
    if ((portInfo->speed >= XPSAI_PORT_SPEED_1000MB) &&
        (portInfo->duplex == XPSAI_PORT_DUPLEX_HALF))
    {
        /* We do not support duplex change for speeds 1G and above */
        XP_SAI_LOG_ERR("Half duplex config not supported\n");
        return false;
    }

    /* Validate speed and FEC mode compatibility */
    if (!xpSaiFecModeAndSpeedCheck(portInfo->fec, portInfo->speed))
    {
        XP_SAI_LOG_ERR("Port does not support FEC mode %u for speed %u \n",
                       portInfo->fec, portInfo->speed);
        return false;
    }

    /* Validate local advertisement abilities based on current breakout mode */
    if (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_1X)
    {
        if ((portInfo->localAdvertFdAbility & ~XPSAI_PORT_ADVERT_FD_SPEED_1X_ALLOWED) !=
            0)
        {
            XP_SAI_LOG_ERR("Local full duplex speed advert not supported\n");
            return false;
        }

        if ((portInfo->localAdvertHdAbility & ~XPSAI_PORT_ADVERT_HD_SPEED_1X_ALLOWED) !=
            0)
        {
            XP_SAI_LOG_ERR("Local half duplex speed advert not supported\n");
            return false;
        }

        if ((portInfo->localAdvertFec & ~XPSAI_PORT_ADVERT_FEC_1X_ALLOWED) != 0)
        {
            XP_SAI_LOG_ERR("Local fec advert not supported\n");
            return false;
        }

        if ((portInfo->speed != XPSAI_PORT_SPEED_50G) &&
            (portInfo->speed != XPSAI_PORT_SPEED_25G) &&
            (portInfo->speed != XPSAI_PORT_SPEED_10G) &&
            (portInfo->speed != XPSAI_PORT_SPEED_1000MB) &&
            (portInfo->speed != XPSAI_PORT_SPEED_100MB) &&
            (portInfo->speed != XPSAI_PORT_SPEED_2500MB) &&
            (portInfo->speed != XPSAI_PORT_SPEED_10MB))
        {
            XP_SAI_LOG_ERR("Invalid speed %d setting for breakout mode 1x\n",
                           portInfo->speed);
            return false;
        }
    }
    else if (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_2X)
    {
        if ((portInfo->localAdvertFdAbility & ~XPSAI_PORT_ADVERT_FD_SPEED_2X_ALLOWED) !=
            0)
        {
            XP_SAI_LOG_ERR("Local full duplex speed advert not supported\n");
            return false;
        }

        if ((portInfo->localAdvertHdAbility & ~XPSAI_PORT_ADVERT_HD_SPEED_2X_ALLOWED) !=
            0)
        {
            XP_SAI_LOG_ERR("Local half duplex speed advert not supported\n");
            return false;
        }

        if ((portInfo->localAdvertFec & ~XPSAI_PORT_ADVERT_FEC_2X_ALLOWED) != 0)
        {
            XP_SAI_LOG_ERR("Local fec advert not supported\n");
            return false;
        }

        if ((portInfo->speed != XPSAI_PORT_SPEED_100G) &&
            (portInfo->speed != XPSAI_PORT_SPEED_50G))
        {
            XP_SAI_LOG_ERR("Invalid speed settings for breakout mode 2x\n");
            return false;
        }
    }
    else if (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_4X)
    {
        if ((portInfo->localAdvertFdAbility & ~XPSAI_PORT_ADVERT_FD_SPEED_4X_ALLOWED) !=
            0)
        {
            XP_SAI_LOG_ERR("Local full duplex speed advert not supported\n");
            return false;
        }

        if ((portInfo->localAdvertHdAbility & ~XPSAI_PORT_ADVERT_HD_SPEED_4X_ALLOWED) !=
            0)
        {
            XP_SAI_LOG_ERR("Local half duplex speed advert not supported\n");
            return false;
        }

        if ((portInfo->localAdvertFec & ~XPSAI_PORT_ADVERT_FEC_4X_ALLOWED) != 0)
        {
            XP_SAI_LOG_ERR("Local fec advert not supported\n");
            return false;
        }

        if ((portInfo->speed != XPSAI_PORT_SPEED_200G) &&
            (portInfo->speed != XPSAI_PORT_SPEED_40G) &&
            (portInfo->speed != XPSAI_PORT_SPEED_100G))
        {
            XP_SAI_LOG_ERR("Invalid speed settings for breakout mode 4x\n");
            return false;
        }
    }
    else if (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_8X)
    {
        if ((portInfo->localAdvertFdAbility & ~XPSAI_PORT_ADVERT_FD_SPEED_8X_ALLOWED) !=
            0)
        {
            XP_SAI_LOG_ERR("Local full duplex speed advert not supported\n");
            return false;
        }

        if ((portInfo->localAdvertHdAbility & ~XPSAI_PORT_ADVERT_HD_SPEED_8X_ALLOWED) !=
            0)
        {
            XP_SAI_LOG_ERR("Local half duplex speed advert not supported\n");
            return false;
        }

        if ((portInfo->localAdvertFec & ~XPSAI_PORT_ADVERT_FEC_8X_ALLOWED) != 0)
        {
            XP_SAI_LOG_ERR("Local fec advert not supported\n");
            return false;
        }

        if ((portInfo->speed != XPSAI_PORT_SPEED_400G))
        {
            XP_SAI_LOG_ERR("Invalid speed settings for breakout mode 8x\n");
            return false;
        }
    }
    return true;
}

static XP_STATUS xpSaiGetCPSSIntfType(uint32_t saiIntfType,
                                      CPSS_PORT_INTERFACE_MODE_ENT *cpssIntfType)
{
    XP_STATUS status = XP_NO_ERR;
    if (cpssIntfType == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    switch ((sai_port_interface_type_t)saiIntfType)
    {
        case SAI_PORT_INTERFACE_TYPE_CR:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_CR_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_CR2:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_CR2_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_CR4:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_CR4_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_SR:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_SR2:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_SR_LR2_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_SR4:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_SR_LR4_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_LR:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_LR4:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_SR_LR4_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_KR:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_KR_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_KR2:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_KR2_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_KR4:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_KR4_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_GMII:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_GMII_E;
            break;
        case SAI_PORT_INTERFACE_TYPE_XGMII:
            *cpssIntfType = CPSS_PORT_INTERFACE_MODE_XGMII_E;
            break;
        default:
            return XP_ERR_MODE_NOT_SUPPORTED;
            break;
    }
    return status;
}


/**
 * \brief Apply port configuration set
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum - Port number
 * \param [in] uint32_t actionMask - Mask specifying parameters that changed
 * \param [in/out] xpSaiPortMgrInfo_t *portInfo - port configuration to apply
 *
 * \return XP_STATUS
 */
static XP_STATUS xpSaiApplyPortInfo(xpsDevice_t devId, uint32_t portNum,
                                    uint32_t actionMask, xpSaiPortMgrInfo_t *portInfo)
{
    XP_STATUS status = XP_NO_ERR;
#if 0
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
#endif
    xpSaiPortStatisticDbEntryT *portStatistics = NULL;

    CPSS_PORT_INTERFACE_MODE_ENT cpssIntfType = CPSS_PORT_INTERFACE_MODE_NA_E;
    if (actionMask & XPSAI_PORT_ATTR_INTF_TYPE_MASK)
    {
        status = xpSaiGetCPSSIntfType(portInfo->intfType, &cpssIntfType);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("intfType %u is not supported. Err=%d\n", portInfo->intfType,
                           status);
            return status;
        }
    }

    if (actionMask & (XPSAI_PORT_ATTR_ENABLE_MASK | XPSAI_PORT_ATTR_AUTONEG_MASK))
    {
        if (portInfo->autoneg == XPSAI_PORT_AUTONEG_ENABLE)
        {
            xpsMacSetPortAnEnable(devId, portNum, 1);
        }
        else
        {
            xpsMacSetPortAnEnable(devId, portNum, 0);
        }
    }


    /* Apply loopback configuration with a highest priority */
    if (portInfo->loopbackMode != XPSAI_PORT_LOOPBACK_MODE_NONE)
    {
        /* Apply loopback together with a speed config here */
        if (actionMask & (XPSAI_PORT_ATTR_ENABLE_MASK |
                          XPSAI_PORT_ATTR_SPEED_MASK |
                          XPSAI_PORT_ATTR_INTF_TYPE_MASK |
                          XPSAI_PORT_ATTR_DUPLEX_MASK |
                          XPSAI_PORT_ATTR_FEC_MASK |
                          XPSAI_PORT_ATTR_LOOPBACK_MASK))
        {
            /* Decode speed/duplex/fec settings */
            xpMacConfigMode macMode = xpSaiPortSpeedToMacMode(portInfo->speed);
            if (portInfo->speed == XPSAI_PORT_SPEED_100G)
            {
                // Handle R4 and R2
                if (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_4X)
                {
                    macMode = MAC_MODE_4X100GB;
                }
            }
            xpFecMode fecMode = xpSaiPortFecToFecMode(portInfo->fec);

            // storing the port statistics in DB when disabling AN in internal loopback mode
#if 0
            saiRetVal = xpSaiPortStatisticsStoreDB(devId, portNum);
            if (saiRetVal != SAI_STATUS_SUCCESS)
            {
                XP_SAI_LOG_ERR("Failed to store port statistics in db %u\n", portNum);
            }
#endif
            // If autoneg is enabled, temporyly disable AnEnable flag for loopbackmode
            if (portInfo->autoneg == XPSAI_PORT_AUTONEG_ENABLE)
            {
                xpsMacSetPortAnEnable(devId, portNum, 0);
            }

            status = xpsMacPortInitWithLinkStatusControl(devId, portNum,
                                                         macMode, (uint8_t)cpssIntfType, 1, 0, 0, fecMode, (fecMode != MAX_FEC_MODE),
                                                         portInfo->enable ? XPSAI_ADMIN_PORT_UP : XPSAI_ADMIN_PORT_DOWN, 1);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_WARNING("Failed to re-init port %u. Err=%d",
                                   portNum, status);
            }

            status = xpSaiLoopbackModeSet(devId, portNum, portInfo->loopbackMode,
                                          portInfo->enable);
            if (XP_NO_ERR != status)
            {
                XP_SAI_LOG_ERR("Failed to set loopback mode for port %u\n", portNum);
                return status;
            }

            // Restore back the AnEnable flag
            if (portInfo->autoneg == XPSAI_PORT_AUTONEG_ENABLE)
            {
                xpsMacSetPortAnEnable(devId, portNum, 1);
            }
        }
    }
    else
    {
        /* Set loopback mode to NONE */
        if (actionMask & XPSAI_PORT_ATTR_LOOPBACK_MASK)
        {
            status = xpSaiLoopbackModeSet(devId, portNum, XPSAI_PORT_LOOPBACK_MODE_NONE,
                                          portInfo->enable);
            if (XP_NO_ERR != status)
            {
                XP_SAI_LOG_ERR("Failed to set loopback mode for port %u\n", portNum);
                return status;
            }
        }

        /* Two major cases if autoneg is ON or OFF, so check it first */
        if (portInfo->autoneg == XPSAI_PORT_AUTONEG_ENABLE)
        {
            if ((actionMask & (XPSAI_PORT_ATTR_ENABLE_MASK |
                               XPSAI_PORT_ATTR_AUTONEG_MASK |
                               XPSAI_PORT_ATTR_LOCAL_ADVERT_FD_MASK |
                               XPSAI_PORT_ATTR_LOCAL_ADVERT_HD_MASK |
                               XPSAI_PORT_ATTR_LOCAL_ADVERT_FEC_MASK |
                               XPSAI_PORT_ATTR_LOOPBACK_MASK)))
            {
                /* Either Autoneg is enabled or local advertisements
                 * are changed. Need to restart AN/LT anyway */
                if (actionMask & (XPSAI_PORT_ATTR_AUTONEG_MASK |
                                  XPSAI_PORT_ATTR_LOCAL_ADVERT_FD_MASK |
                                  XPSAI_PORT_ATTR_LOCAL_ADVERT_FEC_MASK |
                                  XPSAI_PORT_ATTR_LOOPBACK_MASK))
                {
                    uint32_t abilities = 0;
                    uint16_t errorControl = 0;

                    xpSaiAutonegAbilitiesEncode(portNum, portInfo->localAdvertFdAbility,
                                                portInfo->localAdvertHdAbility,
                                                portInfo->localAdvertFec,
                                                &abilities, &errorControl);

                    XP_SAI_LOG_DBG("Start autonego for port #%u with "
                                   "abilities 0x%x and error_control 0x%x\n",
                                   portNum, abilities, errorControl);

                    // storing the port statistics in DB before restarting the AN
#if 0
                    saiRetVal = xpSaiPortStatisticsStoreDB(devId, portNum);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to store port statistics in db %u\n", portNum);
                    }
#endif
                    status = xpsLinkManagerPortANLtEnable(devId, portNum,
                                                          abilities, errorControl,
                                                          (portInfo->localAdvertFdAbility & XPSAI_PORT_ADVERT_SPEED_25G)?1:0,
                                                          portInfo->enable ? XPSAI_ADMIN_PORT_UP : XPSAI_ADMIN_PORT_DOWN);
                    if (status != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("Failed to enable auto-negotiation on port %u. Err=%d\n",
                                           portNum, status);
                    }

                    portInfo->remoteAdvertFdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
                    portInfo->remoteAdvertHdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
                    portInfo->remoteAdvertFec = XPSAI_PORT_ADVERT_FEC_NONE;
                }

                if (actionMask & (XPSAI_PORT_ATTR_ENABLE_MASK |
                                  XPSAI_PORT_ATTR_LOOPBACK_MASK))
                {
                    /* Disable port and silently save autoneg abilities,
                     * so it can be used when port will be enabled  */
                    status = xpsMacPortEnable(devId, portNum, portInfo->enable);
                    if (status != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("Failed to apply admin state for port #%u. Err=%d\n",
                                           portNum, status);
                    }
                }

                status = XP_NO_ERR;
            }
            else
            {
                /* Autoneg is enabled, but none of AN related parameter
                 * changed. Skip any reconfigure actions */
                status = XP_NO_ERR;
            }
        }
        else
        {
            if (actionMask & (XPSAI_PORT_ATTR_ENABLE_MASK |
                              XPSAI_PORT_ATTR_SPEED_MASK |
                              XPSAI_PORT_ATTR_INTF_TYPE_MASK |
                              XPSAI_PORT_ATTR_DUPLEX_MASK |
                              XPSAI_PORT_ATTR_FEC_MASK |
                              XPSAI_PORT_ATTR_AUTONEG_MASK |
                              XPSAI_PORT_ATTR_LOOPBACK_MASK))
            {
                //uint16_t i;

                /* Decode speed/duplex/fec settings */
                xpMacConfigMode macMode = xpSaiPortSpeedToMacMode(portInfo->speed);
                if (portInfo->speed == XPSAI_PORT_SPEED_100G)
                {
                    // Handle R4 and R2
                    if (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_4X)
                    {
                        macMode = MAC_MODE_4X100GB;
                    }
                }
                xpFecMode fecMode = xpSaiPortFecToFecMode(portInfo->fec);

                xpMacConfigMode prevMacMode = MAC_MODE_MAX_VAL;
                prevMacMode = xpSaiPortSpeedToMacMode(portInfo->removePortSpeed);
                if (portInfo->speed == XPSAI_PORT_SPEED_100G)
                {
                    // Handle R4 and R2
                    if (portInfo->breakOutMode == XPSAI_PORT_BREAKOUT_MODE_4X)
                    {
                        prevMacMode = MAC_MODE_4X100GB;
                    }
                }

                if (((actionMask & XPSAI_PORT_ATTR_SPEED_MASK) && (macMode == prevMacMode)) ||
                    ((actionMask & XPSAI_PORT_ATTR_FEC_MASK) && (macMode == prevMacMode) &&
                     (fecMode != MAX_FEC_MODE)))
                {
                    // restore mac stats
#if 0
                    saiRetVal = xpSaiPortStatisticsStoreDB(devId, portNum);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to store port statistics in db %u\n", portNum);
                    }
#endif
                }
                else if (actionMask & XPSAI_PORT_ATTR_SPEED_MASK)
                {
                    // clear port statistics in DB
                    status = xpSaiGetPortStatisticInfo(portNum, &portStatistics);
                    if (status != XP_NO_ERR)
                    {
                        XP_SAI_LOG_ERR("Error: Failed to get the port statistic data, xpStatus: %d\n",
                                       status);
                        return status;
                    }

                    memset(portStatistics, 0, sizeof(xpSaiPortStatisticDbEntryT));
                    portStatistics->portNum = portNum;
                }

                if (actionMask & (XPSAI_PORT_ATTR_SPEED_MASK |
                                  XPSAI_PORT_ATTR_INTF_TYPE_MASK |
                                  XPSAI_PORT_ATTR_DUPLEX_MASK |
                                  XPSAI_PORT_ATTR_FEC_MASK |
                                  XPSAI_PORT_ATTR_AUTONEG_MASK |
                                  XPSAI_PORT_ATTR_LOOPBACK_MASK))
                {
                    status = xpSaiGetCPSSIntfType(portInfo->intfType, &cpssIntfType);
                    if (status != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("intfType not initialised.\n");
                    }

                    status = xpsMacPortInitWithLinkStatusControl(devId, portNum,
                                                                 macMode, (uint8_t)cpssIntfType, 1, 0, 0, fecMode, (fecMode != MAX_FEC_MODE),
                                                                 portInfo->enable ? XPSAI_ADMIN_PORT_UP : XPSAI_ADMIN_PORT_DOWN,
                                                                 ((actionMask & XPSAI_PORT_ATTR_AUTONEG_MASK) ||
                                                                  (actionMask & XPSAI_PORT_ATTR_LOOPBACK_MASK)));
                    if (status != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("Failed to re-init port %u with interfaceType %d. Err=%d\n",
                                           portNum, cpssIntfType, status);
                        return status;
                    }
                }
                else if (actionMask & XPSAI_PORT_ATTR_ENABLE_MASK)
                {
                    /* Apply port admin state and silently save autoneg abilities */
                    status = xpsMacPortEnable(devId, portNum, portInfo->enable);
                    if (status != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("Failed to apply admin state for port #%u. Err=%d\n",
                                           portNum, status);
                    }
                }
                /* This will force to tune port after apply new settings */
                portInfo->serdesTuneWait = 0;
                portInfo->serdesRecheckDelay = 0;
                portInfo->serdesFineTuneWait = 0;
                portInfo->removePortSpeed = xpSaiPortGetSpeed(devId, portNum);
                /* Set status to success at the end */
                status = XP_NO_ERR;
            }
            else
            {
                /* Autoneg is disabled, but none of fixed speed related
                 * parameters changed. Skip any reconfigure actions */
                status = XP_NO_ERR;
            }
        }
    }
    return status;
}

/**
 * \brief Initialize SAI port configuration manager
 *
 * During initialization process we iterate over all available ports
 * in the system and initialize port configuration manager internal
 * structures with current port break-out, speed, FEC, etc. configuration.
 *
 * The initial port break-out dependencies are built here ("isActive" and
 * "breakOutMember" values are set based on initial SKU mode)
 *
 * \param [in] xpsDevice_t devId
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiPortCfgMgrInit(xpsDevice_t devId)
{
    XP_STATUS status = XP_NO_ERR;
    uint32_t portNum;
    uint8_t maxPhyPortNum;
    xpSpeed speed;
    xpSaiPortMgrInfo_t *portInfo;
    xpMacConfigMode macMode = MAC_MODE_MAX_VAL;
    uint8_t maxMacChannel = XPSAI_PORT_HW_LANE_CNT_MAX;
    uint32_t portAdvertFdAbility;

    status = xpsGlobalSwitchControlGetMacMaxChannelNumber(devId, &maxMacChannel);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to get MAC max channel number\n");
        return status;
    }
    // Get max port num
    status = xpsGlobalSwitchControlGetMaxPorts(devId, &maxPhyPortNum);
    if (status != XP_NO_ERR)
    {
        XP_SAI_LOG_ERR("Failed to retrieve maximum port number\n");
        return status;
    }

    XPS_GLOBAL_PORT_ITER(portNum, maxPhyPortNum)
    {
        portInfo = xpSaiPortInfoGet(portNum);
        if (portInfo == NULL)
        {
            continue;
        }

        portInfo->isActive = 0;
        portInfo->serdesTuneWait = 0;
        portInfo->serdesRecheckDelay = 0;
        portInfo->serdesFineTuneWait = 0;
        portInfo->breakOutMember = XPSAI_PORT_HW_LANE_INVALID;
        portInfo->breakOutMode = XPSAI_PORT_BREAKOUT_MODE_NONE;
        portInfo->hwLaneList[0] = portNum;
        if (maxMacChannel == 8)
        {
            if ((portNum % maxMacChannel) == 0)
            {
                portInfo->hwLaneList[1] = portNum + 1;
                portInfo->hwLaneList[2] = portNum + 2;
                portInfo->hwLaneList[3] = portNum + 3;
                portInfo->hwLaneList[4] = portNum + 4;
                portInfo->hwLaneList[5] = portNum + 5;
                portInfo->hwLaneList[6] = portNum + 6;
                portInfo->hwLaneList[7] = portNum + 7;
            }
            else if ((portNum % maxMacChannel) == 4)
            {
                portInfo->hwLaneList[1] = portNum + 1;
                portInfo->hwLaneList[2] = portNum + 2;
                portInfo->hwLaneList[3] = portNum + 3;
                portInfo->hwLaneList[4] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[5] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[6] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[7] = XPSAI_PORT_HW_LANE_INVALID;
            }
            else if ((portNum % maxMacChannel) == 2 || (portNum % maxMacChannel) == 6)
            {
                portInfo->hwLaneList[1] = portNum + 1;
                portInfo->hwLaneList[2] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[3] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[4] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[5] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[6] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[7] = XPSAI_PORT_HW_LANE_INVALID;
            }
            else
            {
                portInfo->hwLaneList[1] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[2] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[3] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[4] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[5] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[6] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[7] = XPSAI_PORT_HW_LANE_INVALID;
            }
        }
        else if (maxMacChannel == 4)
        {
            if ((portNum % maxMacChannel) == 0)
            {
                portInfo->hwLaneList[1] = portNum + 1;
                portInfo->hwLaneList[2] = portNum + 2;
                portInfo->hwLaneList[3] = portNum + 3;
            }
            else if ((portNum % maxMacChannel) == 2)
            {
                portInfo->hwLaneList[1] = portNum + 1;
                portInfo->hwLaneList[2] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[3] = XPSAI_PORT_HW_LANE_INVALID;
            }
            else
            {
                portInfo->hwLaneList[1] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[2] = XPSAI_PORT_HW_LANE_INVALID;
                portInfo->hwLaneList[3] = XPSAI_PORT_HW_LANE_INVALID;
            }
        }
        portInfo->hwLaneList[1] = portNum + 1;
        portInfo->hwLaneList[2] = portNum + 2;
        portInfo->hwLaneList[3] = portNum + 3;
        portInfo->hwLaneList[4] = portNum + 4;
        portInfo->hwLaneList[5] = portNum + 5;
        portInfo->hwLaneList[6] = portNum + 6;
        portInfo->hwLaneList[7] = portNum + 7;

        portInfo->enable = XPSAI_PORT_DISABLE;
        portInfo->speed = XPSAI_PORT_SPEED_INVALID;
        portInfo->duplex = XPSAI_PORT_DUPLEX_FULL;
        portInfo->fec = XPSAI_PORT_FEC_NONE;
        portInfo->autoneg = XPSAI_PORT_AUTONEG_DISABLE;

        portInfo->localAdvertFdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
        portInfo->localAdvertHdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
        portInfo->localAdvertFec = XPSAI_PORT_ADVERT_FEC_NONE;
        portInfo->loopbackMode = XPSAI_PORT_LOOPBACK_MODE_NONE;
        portInfo->linkStatus = XPSAI_PORT_LINK_STATUS_DOWN;
        portInfo->countersWALinkState = 0;
        portInfo->remoteAdvertFdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
        portInfo->remoteAdvertHdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
        portInfo->remoteAdvertFec = XPSAI_PORT_ADVERT_FEC_NONE;

        if (xpsMacGetPortAnEnable(devId, portNum))
        {
            portInfo->autoneg = XPSAI_PORT_AUTONEG_ENABLE;
            status = xpsMacGetPortFdAbility(devId, portNum, &portAdvertFdAbility);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to get portAdvertFdAbility for  port #%u. Err=%d",
                               portNum, status);
            }
            status = xpsMacGetPortSpeed(devId, portNum, &speed);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to get speed for port #%u. Err=%d",
                               portNum, status);
            }
            portInfo->speed = speed;
            portInfo->autoneg = XPSAI_PORT_AUTONEG_ENABLE;
            portInfo->localAdvertFdAbility = portAdvertFdAbility;
            portInfo->remoteAdvertFdAbility = portAdvertFdAbility;


        }
        if (xpsIsPortInited(devId, portNum) == XP_NO_ERR)
        {
            uint8_t signalOk = 0;

            status = xpsMacPortEnable(devId, portNum, portInfo->enable);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_WARNING("Failed to apply admin state for port #%u. Err=%d\n",
                                   portNum, status);
            }

            status = xpsMacPortSerdesGetSignalOk(devId, portNum, &signalOk);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_WARNING("Serdes get signalOk failed for port %u. Err=%d\n",
                                   portNum, status);
            }

            status = xpsMacConfigModeGet(devId, portNum, &macMode);
            if (XP_NO_ERR != status)
            {
                XP_SAI_LOG_ERR("Could not get MAC mode of the port %u.\n", portNum);
                return status;
            }

            status = xpSaiPortMacModeToBreakoutMode(macMode, &portInfo->breakOutMode);
            if (XP_NO_ERR != status)
            {
                XP_SAI_LOG_ERR("Could not determine break-out mode of the port %u.\n", portNum);
                return status;
            }

            switch (portInfo->breakOutMode)
            {
                case XPSAI_PORT_BREAKOUT_MODE_1X:
                    portInfo->localAdvertFdAbility = XPSAI_PORT_ADVERT_FD_SPEED_1X_ALLOWED;
                    portInfo->localAdvertHdAbility = XPSAI_PORT_ADVERT_HD_SPEED_1X_ALLOWED;
                    break;
                case XPSAI_PORT_BREAKOUT_MODE_2X:
                    portInfo->localAdvertFdAbility = XPSAI_PORT_ADVERT_FD_SPEED_2X_ALLOWED;
                    portInfo->localAdvertHdAbility = XPSAI_PORT_ADVERT_HD_SPEED_2X_ALLOWED;
                    break;
                case XPSAI_PORT_BREAKOUT_MODE_4X:
                    portInfo->localAdvertFdAbility = XPSAI_PORT_ADVERT_FD_SPEED_4X_ALLOWED;
                    portInfo->localAdvertHdAbility = XPSAI_PORT_ADVERT_HD_SPEED_4X_ALLOWED;
                    break;
                case XPSAI_PORT_BREAKOUT_MODE_8X:
                    portInfo->localAdvertFdAbility = XPSAI_PORT_ADVERT_FD_SPEED_8X_ALLOWED;
                    portInfo->localAdvertHdAbility = XPSAI_PORT_ADVERT_HD_SPEED_8X_ALLOWED;
                    break;
                default:
                    break;
            }
            portInfo->speed = xpSaiPortGetSpeed(devId, portNum);
            portInfo->fec = xpSaiPortGetFec(devId, portNum);
            portInfo->breakOutMember = portNum;
            portInfo->isActive = 1;
        }
    }

    /* Update port break out membership for inactive lanes */
    XPS_GLOBAL_PORT_ITER(portNum, maxPhyPortNum)
    {
        portInfo = xpSaiPortInfoGet(portNum);
        if (portInfo != NULL)
        {
            if (portInfo->isActive &&
                (portInfo->breakOutMode != XPSAI_PORT_BREAKOUT_MODE_NONE))
            {
                uint16_t i;
                for (i = 0; i < xpSaiGetUsedLaneCnt(portInfo->breakOutMode); i++)
                {
                    xpSaiPortMgrInfo_t *hwLaneInfo = xpSaiPortInfoGet(portInfo->hwLaneList[i]);
                    if (hwLaneInfo != NULL)
                    {
                        hwLaneInfo->breakOutMember = portNum;
                    }
                }
            }
        }
    }

    return status;
}

/**
 * \brief Verify if port is active
 *
 * This API verifies port activity. Port is considered active when it is initialized and break-out mode is NOT none.
 *
 * \param [in] uint32_t portNum - SAI port number
 *
 * \return uint8_t - true if port is active
 */
uint8_t xpSaiIsPortActive(uint32_t portNum)
{
    xpSaiPortMgrInfo_t *portInfo; // Holds current settings
    uint8_t isActive = false;

    /* skip inband mgmt port */
    if (portNum == xpSaiGetSwitchInbandMgmtInf())
    {
        return false;
    }

    portInfo = xpSaiPortInfoGet(portNum);
    if (portInfo != NULL)
    {
        /* Port config/apply sync acquire */
        xpSaiPortMgrLockAquire();
        isActive = portInfo->isActive;
        /* Port config/apply sync release */
        xpSaiPortMgrLockRelease();
    }

    return isActive;
}

/**
 * \brief Initialize "xpSaiPortMgrInfo_t" structure
 *
 * API works like C++ "constructor". This API should be always called in prior to
 * xpSaiPortCfgSet()/xpSaiPortCfgGet() call.
 *
 * Example:
 *
 * \code
 *
 * xpSaiPortMgrInfo_t info;
 *
 * xpSaiPortInfoInit(&info);
 * info.enable = XPSAI_PORT_ENABLE;
 * xpSaiPortCfgSet(devId, portNum, &info, XPSAI_PORT_ATTR_ENABLE_MASK);
 *
 * \endcode
 *
 * \param [out] xpSaiPortMgrInfo_t info - Pointer to port info structure
 *
 * \return void
 */
void xpSaiPortInfoInit(xpSaiPortMgrInfo_t *info)
{
    info->breakOutMode = XPSAI_PORT_BREAKOUT_MODE_NONE;
    info->hwLaneList[0] = info->hwLaneList[1] =
                              info->hwLaneList[2] = info->hwLaneList[3] = info->hwLaneList[4] =
                                                                              info->hwLaneList[5] = info->hwLaneList[6] = info->hwLaneList[7] =
                                                                                      XPSAI_PORT_HW_LANE_INVALID;
    info->enable = XPSAI_PORT_DISABLE;
    info->speed = XPSAI_PORT_SPEED_INVALID;
    info->duplex = XPSAI_PORT_DUPLEX_FULL;
    info->fec = XPSAI_PORT_FEC_NONE;
    info->autoneg = XPSAI_PORT_AUTONEG_DISABLE;
    info->localAdvertFdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
    info->localAdvertHdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
    info->localAdvertFec = XPSAI_PORT_ADVERT_FEC_NONE;
    info->loopbackMode = XPSAI_PORT_LOOPBACK_MODE_NONE;
    info->linkStatus = XPSAI_PORT_LINK_STATUS_DOWN;
    info->remoteAdvertFdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
    info->remoteAdvertHdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
    info->remoteAdvertFec = XPSAI_PORT_ADVERT_FEC_NONE;
    info->removePortSpeed = XPSAI_PORT_SPEED_INVALID;
    info->operSpeed = XPSAI_PORT_SPEED_INVALID;
}

/**
 * \brief Apply port break-out mode configuration
 *
 * This API checks if port break-out mode can be applied to a given port.
 * If port break-out mode conditions are met then new break-out mode is
 * applied on a port.
 *
 * \param [in] xpsDevice_t devId
 * \param [in] xpsPort_t portNum - SAI port number
 * \param [in] uint8_t newBreakOutMode - Port break-out mode to apply
 *
 * \return uint8_t - true if port is active
 */
static XP_STATUS xpSaiUpdatePortBreakOutMode(xpsDevice_t devId,
                                             xpsPort_t portNum,
                                             uint8_t newBreakOutMode, uint32_t speed)
{
    XP_STATUS status = XP_NO_ERR;
#if 0
    sai_status_t saiRetVal = SAI_STATUS_SUCCESS;
#endif
    xpMacConfigMode macMode = MAC_MODE_MAX_VAL;
    xpSaiPortMgrInfo_t *portInfo = xpSaiPortInfoGet(portNum);
    uint8_t breakOutMode;
    uint32_t hwLaneList[8];
    uint16_t i = 0;
    xpFecMode fecMode = MAX_FEC_MODE;
    uint8_t enableFec = 0;

    if (portInfo == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    breakOutMode = portInfo->breakOutMode;
    memcpy(hwLaneList, portInfo->hwLaneList, 8);

    if (newBreakOutMode == breakOutMode)
    {
        /* We are in the same breakout mode so nothing to be changed */
        XP_SAI_LOG_DBG("Same breakOutMode %u selected on port %u\n", newBreakOutMode,
                       portNum);
        return XP_NO_ERR;
    }

    /* If we are switching from XPSAI_PORT_BREAKOUT_MODE_NONE to some other
     * then check if needed lanes are not used */
    if (newBreakOutMode != XPSAI_PORT_BREAKOUT_MODE_NONE)
    {
        uint32_t localAdvertFdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
        uint32_t localAdvertHdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
        uint32_t localAdvertFec = XPSAI_PORT_ADVERT_FEC_NONE;

        for (i = 0; i < xpSaiGetUsedLaneCnt(newBreakOutMode); i++)
        {
            xpSaiPortMgrInfo_t *hwLaneInfo = xpSaiPortInfoGet(hwLaneList[i]);
            if (hwLaneInfo != NULL)
            {
                if (hwLaneInfo->breakOutMember != XPSAI_PORT_HW_LANE_INVALID)
                {
                    /* Lane is currently used by another port. Return an error */
                    //FALCON WORKAROUND
                    //return XP_ERR_INIT_FAILED;
                }
            }
        }

        switch (newBreakOutMode)
        {
            case XPSAI_PORT_BREAKOUT_MODE_1X:
                if (XPSAI_PORT_SPEED_1000MB == speed)
                {
                    macMode = MAC_MODE_1GB;
                }
                else if (XPSAI_PORT_SPEED_2500MB == speed)
                {
                    macMode = MAC_MODE_2_5GB;
                }
                else if (XPSAI_PORT_SPEED_100MB == speed)
                {
                    macMode = MAC_MODE_100M;
                }
                else if (XPSAI_PORT_SPEED_10MB == speed)
                {
                    macMode = MAC_MODE_10M;
                }
                else if (XPSAI_PORT_SPEED_25G == speed)
                {
                    macMode = MAC_MODE_1X25GB;
                }
                else if (XPSAI_PORT_SPEED_50G == speed)
                {
                    macMode = MAC_MODE_1X50GB;
                    fecMode = RS_544_514_FEC_MODE;
                    enableFec = 1;
                }
                else if (XPSAI_PORT_SPEED_10G == speed)
                {
                    macMode = MAC_MODE_1X10GB;
                }
                localAdvertFdAbility = XPSAI_PORT_ADVERT_FD_SPEED_1X_ALLOWED;
                localAdvertHdAbility = XPSAI_PORT_ADVERT_HD_SPEED_1X_ALLOWED;
                break;
            case XPSAI_PORT_BREAKOUT_MODE_2X:
                if (XPSAI_PORT_SPEED_50G == speed)
                {
                    macMode = MAC_MODE_2X50GB;
                }
                else if (XPSAI_PORT_SPEED_100G == speed)
                {
                    macMode = MAC_MODE_2X100GB;
                    fecMode = RS_544_514_FEC_MODE;
                    enableFec = 1;
                }
                localAdvertFdAbility = XPSAI_PORT_ADVERT_FD_SPEED_2X_ALLOWED;
                localAdvertHdAbility = XPSAI_PORT_ADVERT_HD_SPEED_2X_ALLOWED;
                break;
            case XPSAI_PORT_BREAKOUT_MODE_4X:
                if (XPSAI_PORT_SPEED_100G == speed)
                {
                    macMode = MAC_MODE_4X100GB;
                }
                else if (XPSAI_PORT_SPEED_40G == speed)
                {
                    macMode = MAC_MODE_4X40GB;
                }
                else if (XPSAI_PORT_SPEED_200G == speed)
                {
                    macMode = MAC_MODE_4X200GB;
                    fecMode = RS_544_514_FEC_MODE;
                    enableFec = 1;
                }
                localAdvertFdAbility = XPSAI_PORT_ADVERT_FD_SPEED_4X_ALLOWED;
                localAdvertHdAbility = XPSAI_PORT_ADVERT_HD_SPEED_4X_ALLOWED;
                break;
            case XPSAI_PORT_BREAKOUT_MODE_8X:
                if (XPSAI_PORT_SPEED_400G == speed)
                {
                    macMode = MAC_MODE_8X400GB;
                    fecMode = RS_544_514_FEC_MODE;
                    enableFec = 1;
                }
                localAdvertFdAbility = XPSAI_PORT_ADVERT_FD_SPEED_8X_ALLOWED;
                localAdvertHdAbility = XPSAI_PORT_ADVERT_HD_SPEED_8X_ALLOWED;
                break;
            default:
                macMode = MAC_MODE_1X50GB;
                fecMode = RS_544_514_FEC_MODE;
                enableFec = 1;
                localAdvertFdAbility = XPSAI_PORT_ADVERT_FD_SPEED_1X_ALLOWED;
                localAdvertHdAbility = XPSAI_PORT_ADVERT_HD_SPEED_1X_ALLOWED;
                break;
        }

        /* Switch to new MAC mode */
        status = xpsMacPortInitWithLinkStatusControl(devId, portNum,
                                                     macMode, CPSS_PORT_INTERFACE_MODE_NA_E, 1, 0, 0, fecMode, enableFec,
                                                     XPSAI_ADMIN_PORT_DOWN, 1);
        if (status != XP_NO_ERR)
        {
            XP_SAI_LOG_ERR("Unable to init port %u with %u MAC mode. Err=%d",
                           portNum, macMode, status);
            return status;
        }

        portInfo->isActive = 1;
        portInfo->breakOutMode = newBreakOutMode;
        portInfo->localAdvertFdAbility = localAdvertFdAbility;
        portInfo->localAdvertHdAbility = localAdvertHdAbility;
        portInfo->localAdvertFec = localAdvertFec;
        portInfo->speed = xpSaiPortGetSpeed(devId, portNum);
        portInfo->fec = xpSaiPortGetFec(devId, portNum);

        for (i = 0; i < xpSaiGetUsedLaneCnt(newBreakOutMode); i++)
        {
            xpSaiPortMgrInfo_t *hwLaneInfo = xpSaiPortInfoGet(hwLaneList[i]);
            if (hwLaneInfo != NULL)
            {
                hwLaneInfo->breakOutMember = portNum;
            }
        }
    }
    else
    {
        /* We are switching to XPSAI_PORT_BREAKOUT_MODE_NONE here */
        for (i = 0; i < xpSaiGetUsedLaneCnt(breakOutMode); i++)
        {
            xpSaiPortMgrInfo_t *hwLaneInfo = xpSaiPortInfoGet(hwLaneList[i]);
            if (hwLaneInfo == NULL)
            {
                continue;
            }

            if (hwLaneInfo->breakOutMember == portNum)
            {
                if (xpsIsPortInited(devId, hwLaneList[i]) == XP_NO_ERR)
                {
#if 0
                    saiRetVal = xpSaiPortStatisticsStoreDB(devId, portNum);
                    if (saiRetVal != SAI_STATUS_SUCCESS)
                    {
                        XP_SAI_LOG_ERR("Failed to store port statistics in db %u\n", portNum);
                    }
#endif
                    // storing the port speed before portDeInit
                    if (hwLaneList[i] == portNum)
                    {
                        hwLaneInfo->removePortSpeed = xpSaiPortGetSpeed(devId, portNum);
                    }
                    else
                    {
                        hwLaneInfo->removePortSpeed = XPSAI_PORT_SPEED_INVALID;
                    }
                    status = xpsLinkManagerPortANLtDisable(devId, hwLaneList[i]);
                    if (status != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("Failed disable AN LT port #%u. Err=%d",
                                           hwLaneList[i], status);
                    }

                    status = xpsMacBackPlaneAutoNegProcessReset(devId, hwLaneList[i]);
                    if (status != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("Failed to reset autoneg on port #%u. Err=%d",
                                           hwLaneList[i], status);
                    }

                    status = xpsMacPortDeInit(devId, hwLaneList[i]);
                    if (status != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("Failed to deinit port #%u. Err=%d",
                                           hwLaneList[i], status);
                    }
                }

                hwLaneInfo->isActive = 0;
                hwLaneInfo->serdesTuneWait = 0;
                hwLaneInfo->serdesRecheckDelay = 0;
                hwLaneInfo->serdesFineTuneWait = 0;
                hwLaneInfo->breakOutMember = XPSAI_PORT_HW_LANE_INVALID;
                hwLaneInfo->breakOutMode = XPSAI_PORT_BREAKOUT_MODE_NONE;
                hwLaneInfo->enable = XPSAI_PORT_DISABLE;
                hwLaneInfo->speed = XPSAI_PORT_SPEED_INVALID;
                hwLaneInfo->intfType = SAI_PORT_INTERFACE_TYPE_NONE;
                hwLaneInfo->removePortSpeed = XPSAI_PORT_SPEED_INVALID;
                hwLaneInfo->duplex = XPSAI_PORT_DUPLEX_FULL;
                hwLaneInfo->fec = XPSAI_PORT_FEC_NONE;
                hwLaneInfo->autoneg = XPSAI_PORT_AUTONEG_DISABLE;
                hwLaneInfo->localAdvertFdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
                hwLaneInfo->localAdvertHdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
                hwLaneInfo->localAdvertFec = XPSAI_PORT_ADVERT_FEC_NONE;
                hwLaneInfo->loopbackMode = XPSAI_PORT_LOOPBACK_MODE_NONE;
                hwLaneInfo->linkStatus = XPSAI_PORT_LINK_STATUS_DOWN;
                hwLaneInfo->remoteAdvertFdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
                hwLaneInfo->remoteAdvertHdAbility = XPSAI_PORT_ADVERT_SPEED_NONE;
                hwLaneInfo->remoteAdvertFec = XPSAI_PORT_ADVERT_FEC_NONE;
            }
        }
    }

    return XP_NO_ERR;
}

/**
 * \brief Change port settings according to a given "actionMask"
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum - SAI port number
 * \param [in] xpSaiPortMgrInfo_t info - Points to a structure holding port parameters
 *                                       to update. Only fields specified by "actionMask"
 *                                       are used during update
 * \param [in] uint32_t actionMask - Bit encoded set of port parameters
 *                                   (pointed by "info" parameter) to update
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiPortCfgSet(xpsDevice_t devId, uint32_t portNum,
                          const xpSaiPortMgrInfo_t *info, uint32_t actionMask)
{
    XP_STATUS status = XP_NO_ERR;
    xpSaiPortMgrInfo_t *portInfo = xpSaiPortInfoGet(portNum);

    if (portInfo == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    if (actionMask & XPSAI_PORT_ATTR_ALL_RO_MASK)
    {
        /* We do not support setting for port read only attributes */
        XP_SAI_LOG_ERR("Can't update read only attributes for port %u\n", portNum);
        return XP_ERR_INVALID_ARG;
    }

    /* Port config/apply sync acquire */
    xpSaiPortMgrLockAquire();

    do
    {
        /* Handle port break-out mode change in prior to any other property to
         * give possibility to apply port speed, FEC, autoneg, etc. configuration
         * to a port withing the same call of xpSaiPortCfgSet() */
        if (actionMask & XPSAI_PORT_ATTR_BREAKOUT_MASK)
        {
            status = xpSaiUpdatePortBreakOutMode(devId, portNum, info->breakOutMode,
                                                 info->speed);
            if (status != XP_NO_ERR)
            {
                XP_SAI_LOG_ERR("Failed to apply breakout mode for port %u\n", portNum);
                break;
            }
        }

        /* Handle changes for various attributes with respect to current settings */
        if (actionMask & (XPSAI_PORT_ATTR_ENABLE_MASK | XPSAI_PORT_ATTR_SPEED_MASK |
                          XPSAI_PORT_ATTR_DUPLEX_MASK | XPSAI_PORT_ATTR_FEC_MASK |
                          XPSAI_PORT_ATTR_AUTONEG_MASK | XPSAI_PORT_ATTR_LOCAL_ADVERT_FD_MASK |
                          XPSAI_PORT_ATTR_LOCAL_ADVERT_HD_MASK | XPSAI_PORT_ATTR_LOCAL_ADVERT_FEC_MASK |
                          XPSAI_PORT_ATTR_LOOPBACK_MASK | XPSAI_PORT_ATTR_INTF_TYPE_MASK))
        {
            xpSaiPortMgrInfo_t newPortInfo;

            if (portInfo->isActive == 0)
            {
                XP_SAI_LOG_ERR("Failed to update. Port %u is not active\n", portNum);
                status = XP_ERR_NOT_INITED;
                break;
            }

            status = xpSaiUpdatePortInfo(portNum, info, &actionMask, &newPortInfo);
            if (XP_NO_ERR != status)
            {
                XP_SAI_LOG_ERR("Failed to update port information structure for port %u\n",
                               portNum);
                break;
            }

            if (!xpSaiValidatePortInfo(&newPortInfo))
            {
                XP_SAI_LOG_ERR("Requested port %u settings are invalid\n", portNum);
                status = XP_ERR_INVALID_VALUE;
                break;
            }

            status = xpSaiApplyPortInfo(devId, portNum, actionMask, &newPortInfo);
            if (XP_NO_ERR != status)
            {
                XP_SAI_LOG_ERR("Failed to apply parameters for port %u\n", portNum);
                break;
            }

            /* Save new configuration set */
            memcpy(portInfo, &newPortInfo, sizeof(xpSaiPortMgrInfo_t));
        }

        if (actionMask)
        {
            XP_SAI_LOG_DBG("Port #%u configuration has changed (mask 0x%08X)\n", portNum,
                           actionMask);
        }

    }
    while (0);

    /* Port config/apply sync release */
    xpSaiPortMgrLockRelease();

    return status;
}

/**
 * \brief Get current settings for a given port
 *
 * The "info" structure will be filled with current settings of the port.
 * If current "linkStatus" of the port is "DOWN" then "speed" and "fec" properties
 * holds configured speed and FEC.
 * If current  "linkStatus" of the port is "UP" then "speed" and "fec" properties
 * holds actual speed and FEC applied on a port.
 *
 * Example:
 *  - Port is configured to 1X with "40G" with FEC "OFF" and Autoneg "ON" with
 *    40G/100G + FC/RS FEC in local advertisements. While "linkStatus" is "DOWN"
 *    the xpSaiPortCfgGet() will return speed "40G" and FEC "OFF". When "linkStatus"
 *    is changed to "UP" (Autoneg is complete) the xpSaiPortCfgGet() can return
 *    speed "100G" and FEC "RS" (if negotiated with remote partner to use 100G).
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum - SAI port number
 * \param [out] xpSaiPortMgrInfo_t info - Is filled with current settings for a given port
 *
 * \return XP_STATUS
 */
XP_STATUS xpSaiPortCfgGet(xpsDevice_t devId, uint32_t portNum,
                          xpSaiPortMgrInfo_t *info)
{
    xpSaiPortMgrInfo_t *portInfo = xpSaiPortInfoGet(portNum);

    if (portInfo == NULL)
    {
        return XP_ERR_NULL_POINTER;
    }

    /* Port config/apply sync acquire */
    xpSaiPortMgrLockAquire();

    memcpy(info, portInfo, sizeof(xpSaiPortMgrInfo_t));

    /* If port is valid, then read speed and FEC values */
    if (portInfo->isActive)
    {
        info->speed = xpSaiPortGetSpeed(devId, portNum);
        info->fec = xpSaiPortGetFec(devId, portNum);
    }

    /* Port config/apply sync release */
    xpSaiPortMgrLockRelease();

    return XP_NO_ERR;
}

/**
 * \brief Update state for all active ports in the system
 *
 * This API has to be called periodically and poll period should be passed
 * as a parameter. The recommended poll interval is 1ms
 *
 * The API iterates all "active" ports ("isActive" set to 1) in the system
 * and performs various state updates depending on current configuration of
 * the port:
 *  - For ports with loopback configuration applied the port link status is read.
 *  - For ports with Autoneg "ON" the XP link manager Autoneg state machine is
 *    executed. The remote partner Autoneg abilities are read from HW and updated.
 *    The port link status is read.
 *  - For ports with fixed speed+FEC configuration the port SerDes tune state
 *    machine is executed. The port link status is read.
 *
 *  The API tracks port link status and calls SAI port link notifications in
 *  case link status is changed.
 *
 * \param [in] xpsDevice_t devId
 * \param [in] uint32_t portNum - SAI port number
 * \param [out] xpSaiPortMgrInfo_t info - Is filled with current settings for a given port
 *
 * \return void
 */
void xpSaiPortCfgMgrStateUpdate(xpsDevice_t devId, uint32_t pollCycleTimeMs)
{
    xpSaiPortMgrInfo_t *portInfo = NULL;
    XP_STATUS status = XP_NO_ERR;
    uint32_t portNum = 0;

    for (portNum = 0; portNum < XPSAI_PORT_SYSTEM_MAX; portNum++)
    {
        uint8_t linkStatusChanged = 0;
        uint8_t linkStatus = 0;

        portInfo = xpSaiPortInfoGet(portNum);

        if (portInfo == NULL)
        {
            continue;
        }

        if (portInfo->isActive)
        {
            /* Port config/apply sync acquire */
            xpSaiPortMgrLockAquire();

            /* We need to sync config and apply to HW only in case
             * port is enabled */
            if (portInfo->enable == XPSAI_PORT_ENABLE)
            {
                if (portInfo->loopbackMode == XPSAI_PORT_LOOPBACK_MODE_NONE)
                {
                    if (portInfo->autoneg == XPSAI_PORT_AUTONEG_ENABLE)
                    {
                        uint16_t portRemoteANAbility = 0, portRemoteANCtrl = 0;

                        status = xpsLinkManagerPortANLtRemoteAbilityGet(devId, portNum,
                                                                        &portRemoteANAbility, &portRemoteANCtrl);
                        if (status != XP_NO_ERR)
                        {
                            //XP_SAI_LOG_WARNING("Could not get remote partner ability %u. Err=%d\n",
                            //                   portNum, status);
                        }
                        else
                        {
                            xpSaiAutonegAbilitiesDecode(portRemoteANAbility, portRemoteANCtrl,
                                                        &portInfo->remoteAdvertFdAbility,
                                                        &portInfo->remoteAdvertHdAbility,
                                                        &portInfo->remoteAdvertFec);
                        }

                        status = xpsMacGetLinkStatus(devId, portNum, &linkStatus);
                        if (status != XP_NO_ERR)
                        {
                            XP_SAI_LOG_WARNING("Could not get port %u link status. Err=%d",
                                               portNum, status);
                        }
                    }
                    else
                    {
                        /* Process serdes tune state machine */
                        status = xpSaiPortSerdesTune(devId, portNum, pollCycleTimeMs,
                                                     &portInfo->serdesRecheckDelay,
                                                     &portInfo->serdesTuneWait, &portInfo->serdesFineTuneWait,
                                                     &linkStatus);
                        if (status != XP_NO_ERR)
                        {
                            XP_SAI_LOG_ERR("Port Serdes Tune failed port %u. Err=%d",
                                           portNum, status);
                        }
                    }
                }
                else
                {
                    status = xpsMacGetLinkStatus(devId, portNum, &linkStatus);
                    if (status != XP_NO_ERR)
                    {
                        XP_SAI_LOG_WARNING("Could not get port %u link status. Err=%d",
                                           portNum, status);
                    }
                }
            }

            /* Update current link status and call upper layer notifications */
            linkStatus = linkStatus ? XPSAI_PORT_LINK_STATUS_UP :
                         XPSAI_PORT_LINK_STATUS_DOWN;
            if (portInfo->linkStatus != linkStatus)
            {
                portInfo->linkStatus = linkStatus;
                if (portInfo->linkStatus == XPSAI_PORT_LINK_STATUS_UP)
                {
                    portInfo->countersWALinkState = 1;
                }

                linkStatusChanged = 1;
            }

            /* Port config/apply sync release */
            xpSaiPortMgrLockRelease();
        }

        if (linkStatusChanged)
        {
            if (linkStatus)
            {
                xpSaiSwitchLinkUpEventNotification(devId, portNum);
            }
            else
            {
                xpSaiSwitchLinkDownEventNotification(devId, portNum);
            }
        }
        // Slow poller for validity
        if (IS_DEVICE_FALCON(xpSaiSwitchDevTypeGet()))
        {
            if (linkStatusChanged)
            {
                XP_SAI_LOG_NOTICE("%s:SlowPoll dev %d port %d link %d\n",
                                  __func__, devId, portNum, linkStatus);
            }
            xpSaiSleepMsec(100);
        }
    }
}

/* No dedicated thread for Falcon port link status query.
 * This API is invoked from CPS HAL in portEvent Task Thread context.
 * Make sure NO  blocking call or loop is added in this logic.
 */
void xpSaiPortCfgMgrStatusUpdate(xpsDevice_t devId, uint32_t portNum,
                                 int linkStatus)
{
    xpSaiPortMgrInfo_t *portInfo = NULL;

    uint8_t linkStatusChanged = 0;

    portInfo = xpSaiPortInfoGet(portNum);

    if (portInfo == NULL)
    {
        return;
    }

    if (portInfo->isActive)
    {
        /* Port config/apply sync acquire */
        xpSaiPortMgrLockAquire();

        /* Update current link status and call upper layer notifications */
        linkStatus = linkStatus ? XPSAI_PORT_LINK_STATUS_UP :
                     XPSAI_PORT_LINK_STATUS_DOWN;
        if (portInfo->linkStatus != linkStatus)
        {
            portInfo->linkStatus = linkStatus;
            if (portInfo->linkStatus == XPSAI_PORT_LINK_STATUS_UP)
            {
                portInfo->countersWALinkState = 1;
            }

            linkStatusChanged = 1;
        }

        /* Port config/apply sync release */
        xpSaiPortMgrLockRelease();
    }

    if (linkStatusChanged)
    {
        if (linkStatus)
        {
            xpSaiSwitchLinkUpEventNotification(devId, portNum);
        }
        else
        {
            xpSaiSwitchLinkDownEventNotification(devId, portNum);
        }
    }
}
