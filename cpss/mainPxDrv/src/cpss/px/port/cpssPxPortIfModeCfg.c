/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxPortIfModeCfg.c
*
* @brief CPSS implementation for Port interface mode configuration.
*
*
* @version   116
********************************************************************************
*/
/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE


#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxHwInitLedCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/PizzaArbiter/prvCpssPxPortPizzaArbiter.h>
#include <cpss/px/cpssHwInit/cpssPxHwInitLedCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortIfModeCfgPipeResource.h>
#include <cpss/px/port/cpssPxPortAp.h>

#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/common/port/cpssPortCtrl.h>

#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsLion2.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



/* array defining serdes speed used in given interface mode for given port data speed */
/* APPLICABLE DEVICES:  pipe. */
extern CPSS_PORT_SERDES_SPEED_ENT serdesPxFrequency[CPSS_PORT_INTERFACE_MODE_NA_E][CPSS_PORT_SPEED_NA_E];

extern GT_STATUS prvCpssPxHwRegAddrPortMacUpdate
(
    IN  GT_U32                  devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
);

#if 0
/* array defining possible interfaces/ports modes configuration options */
/* APPLICABLE DEVICES:  pipe */
GT_BOOL supportedPortsModes[PRV_CPSS_XG_PORT_OPTIONS_MAX_E][CPSS_PORT_INTERFACE_MODE_NA_E] =
{                                  /* R_10BIT  R_GMII    MII       SGMII     XGMII     MGMII   1000BASE_X    GMII    MII_PHY    QX          HX      RXAUI   100BASE_FX  QSGMII       XLG   LOCAL_XGMII  KR      HGL       CHGL_12    ILK      SR_LR */
/* PRV_CPSS_XG_PORT_XG_ONLY_E */    {GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE },
/* PRV_CPSS_XG_PORT_HX_QX_ONLY_E */ {GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE },
/* PRV_CPSS_XG_PORT_XG_HX_QX_E */   {GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_FALSE, GT_TRUE , GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_TRUE , GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE },
/* PRV_CPSS_GE_PORT_GE_ONLY_E */    {GT_FALSE, GT_TRUE , GT_TRUE , GT_TRUE , GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE },
/* PRV_CPSS_XG_PORT_XLG_SGMII_E */  {GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_FALSE, GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE, GT_FALSE },
/* PRV_CPSS_XG_PORT_CG_SGMII_E */   {GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_FALSE, GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE , GT_TRUE , GT_TRUE , GT_TRUE , GT_FALSE, GT_TRUE  }
};
#endif

typedef struct prvIfModeSupportedPxPortsModes_STC
{
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT  portType;
    CPSS_PORT_INTERFACE_MODE_ENT   *supportedIfList;
}prvIfModeSupportedPxPortsModes_STC;



CPSS_PORT_INTERFACE_MODE_ENT supportedPxPortsModes_XG_PORT_XG_ONLY[] =
{
    CPSS_PORT_INTERFACE_MODE_XGMII_E,
    CPSS_PORT_INTERFACE_MODE_NA_E
};

CPSS_PORT_INTERFACE_MODE_ENT supportedPxPortsModes_XG_PORT_HX_QX_ONLY[] =
{
    CPSS_PORT_INTERFACE_MODE_SGMII_E,
    CPSS_PORT_INTERFACE_MODE_QX_E,
    CPSS_PORT_INTERFACE_MODE_HX_E,
    CPSS_PORT_INTERFACE_MODE_NA_E
};

CPSS_PORT_INTERFACE_MODE_ENT supportedPxPortsModes_XG_PORT_XG_HX_QX[] =
{
    CPSS_PORT_INTERFACE_MODE_SGMII_E,
    CPSS_PORT_INTERFACE_MODE_XGMII_E,
    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
    CPSS_PORT_INTERFACE_MODE_QX_E,
    CPSS_PORT_INTERFACE_MODE_HX_E,
    CPSS_PORT_INTERFACE_MODE_RXAUI_E,
    CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,
    CPSS_PORT_INTERFACE_MODE_NA_E
};

CPSS_PORT_INTERFACE_MODE_ENT supportedPxPortsModes_GE_PORT_GE_ONLY[] =
{
    CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E,
    CPSS_PORT_INTERFACE_MODE_MII_E,
    CPSS_PORT_INTERFACE_MODE_SGMII_E,

    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
    CPSS_PORT_INTERFACE_MODE_GMII_E,
    CPSS_PORT_INTERFACE_MODE_MII_PHY_E,

    CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,
    CPSS_PORT_INTERFACE_MODE_QSGMII_E,
    CPSS_PORT_INTERFACE_MODE_NA_E
};

CPSS_PORT_INTERFACE_MODE_ENT supportedPxPortsModes_XG_PORT_XLG_SGMII[] =  /* Lion */
{
    CPSS_PORT_INTERFACE_MODE_SGMII_E,
    CPSS_PORT_INTERFACE_MODE_XGMII_E,

    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
    CPSS_PORT_INTERFACE_MODE_HX_E,
    CPSS_PORT_INTERFACE_MODE_RXAUI_E,

    CPSS_PORT_INTERFACE_MODE_XLG_E,
    CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,
    CPSS_PORT_INTERFACE_MODE_NA_E
};

CPSS_PORT_INTERFACE_MODE_ENT supportedPxPortsModes_XG_PORT_CG_SGMII[] = /* lion2 and above , flex link */
{
    CPSS_PORT_INTERFACE_MODE_SGMII_E,
    CPSS_PORT_INTERFACE_MODE_XGMII_E,

    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,
    CPSS_PORT_INTERFACE_MODE_HX_E,
    CPSS_PORT_INTERFACE_MODE_RXAUI_E,

    CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,
    CPSS_PORT_INTERFACE_MODE_KR_E,
    CPSS_PORT_INTERFACE_MODE_HGL_E,
    CPSS_PORT_INTERFACE_MODE_CHGL_12_E,
    CPSS_PORT_INTERFACE_MODE_SR_LR_E,
    CPSS_PORT_INTERFACE_MODE_XHGS_E,
    CPSS_PORT_INTERFACE_MODE_XHGS_SR_E,
    CPSS_PORT_INTERFACE_MODE_KR4_E,
    CPSS_PORT_INTERFACE_MODE_KR2_E,
    CPSS_PORT_INTERFACE_MODE_SR_LR2_E,
    CPSS_PORT_INTERFACE_MODE_NA_E
};



prvIfModeSupportedPxPortsModes_STC supportedPxPortsModesList[] =
{
    {  PRV_CPSS_XG_PORT_XG_ONLY_E,     &supportedPxPortsModes_XG_PORT_XG_ONLY[0]        }
   ,{  PRV_CPSS_XG_PORT_HX_QX_ONLY_E,  &supportedPxPortsModes_XG_PORT_HX_QX_ONLY[0]     }
   ,{  PRV_CPSS_XG_PORT_XG_HX_QX_E,    &supportedPxPortsModes_XG_PORT_XG_HX_QX[0]       }
   ,{  PRV_CPSS_GE_PORT_GE_ONLY_E,     &supportedPxPortsModes_GE_PORT_GE_ONLY[0]        }
   ,{  PRV_CPSS_XG_PORT_XLG_SGMII_E,   &supportedPxPortsModes_XG_PORT_XLG_SGMII[0]      }
   ,{  PRV_CPSS_XG_PORT_CG_SGMII_E,    &supportedPxPortsModes_XG_PORT_CG_SGMII[0]       }
   ,{  PRV_CPSS_XG_PORT_OPTIONS_MAX_E, (CPSS_PORT_INTERFACE_MODE_ENT *)NULL           }
};

static GT_STATUS prvCpssPxPortIsAlreadyConfigured
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    prevIfMode,
    IN  CPSS_PORT_SPEED_ENT             prevSpeed,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    newIfMode,
    IN  CPSS_PORT_SPEED_ENT             newSpeed,
    OUT GT_BOOL                        *isPortAlreadyConfigured
)
{
    GT_STATUS rc;
    GT_BOOL   linkUp;

    if(NULL == isPortAlreadyConfigured)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    *isPortAlreadyConfigured = GT_FALSE;

    rc = cpssPxPortLinkStatusGet(devNum, portNum, &linkUp);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(linkUp)
    {
        if((GT_TRUE == powerUp) && (prevIfMode == newIfMode) && (prevSpeed == newSpeed))
        {
            *isPortAlreadyConfigured = GT_TRUE;
        }
    }
    return GT_OK;
}

CPSS_PORT_INTERFACE_MODE_ENT * prvCpssPxPortTypeSupportedModeListGet
(
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT portType
)
{
    prvIfModeSupportedPxPortsModes_STC *listPtr;
    GT_U32 i;

    listPtr = &supportedPxPortsModesList[0];
    for (i = 0 ; listPtr->supportedIfList != NULL; i++,listPtr++)
    {
        if (listPtr->portType == portType)
        {
            return listPtr->supportedIfList;
        }
    }
    return (CPSS_PORT_INTERFACE_MODE_ENT *)NULL;
}


GT_BOOL prvCpssPxPortTypeSupportedModeCheck
(
    IN PRV_CPSS_PORT_TYPE_OPTIONS_ENT portType,
    IN CPSS_PORT_INTERFACE_MODE_ENT   ifMode
)
{
    CPSS_PORT_INTERFACE_MODE_ENT * listPtr;
    GT_U32 i;

    listPtr = prvCpssPxPortTypeSupportedModeListGet(portType);
    if (listPtr == NULL)
    {
        return GT_FALSE;
    }
    for (i = 0 ; *listPtr != CPSS_PORT_INTERFACE_MODE_NA_E ; i++, listPtr++)
    {
        if (*listPtr == ifMode)
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

GT_STATUS prvCpssPxPortSupportedModeCheck
(
    IN  GT_SW_DEV_NUM                 devNum,
    IN  GT_PHYSICAL_PORT_NUM          portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT  ifMode,
    OUT GT_BOOL                      *isSupportedPtr
)
{
    PRV_CPSS_PORT_TYPE_OPTIONS_ENT portType;
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* check if interface mode supported by given port */
    portType = PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum,portMacNum);

    /* resOld = supportedPortsModes[PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum,portNum)][ifMode]; */
    *isSupportedPtr = prvCpssPxPortTypeSupportedModeCheck(portType,ifMode);
    return GT_OK;
}

#ifndef GM_USED
/**
* @internal prvCpssPxLedPortTypeConfig function
* @endinternal
*
* @brief   Configures the type of the port connected to the LED.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] powerUp                  - port is powered Up(GT_TRUE) or powered down(GT_FALSE)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssPxLedPortTypeConfig
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         powerUp
)
{
    GT_U32      portMacNum;      /* port MAC number */
    GT_U32      regAddr;         /* register address */
    GT_STATUS   rc;              /* return status*/
    GT_U32      ledPort;         /* LED port number */
    GT_U32      bitOffset;       /* register field offset */
    GT_U32      fieldValue;      /* register field value */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* port MAC type */

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = cpssPxLedStreamPortPositionGet(devNum, portNum, &ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (ledPort == 0x3f)
    {
        /* LED port not connected */
        return GT_OK;
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).portTypeConfig[0];
    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);
    switch (portMacType)
    {
        case PRV_CPSS_PORT_GE_E:
            fieldValue = 0;
            break;
        case PRV_CPSS_PORT_XG_E:
        case PRV_CPSS_PORT_XLG_E:
            fieldValue = 1;
            break;
        case PRV_CPSS_PORT_CG_E:
            fieldValue = 2;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    bitOffset = (ledPort % 16) * 2;

    /* Ports type configuration */
    rc =  prvCpssDrvHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, bitOffset, 2, fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* for Pipe if FE is applicable */
    if (GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum,
        PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        PRV_CPSS_PORT_TYPE_ENT macType;

        if (powerUp == GT_TRUE)
        {
            macType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);
        }
        else
        {
            /* disconnect Mac led indication */
            macType = PRV_CPSS_PORT_NOT_EXISTS_E;
        }
        rc = prvCpssPxLedErrataPortLedMacTypeSet(devNum, portNum, macType);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}
#endif /* GM_USED */

/**
* @internal prvCpssPxPortSerdesLoopbackActivate function
* @endinternal
*
* @brief   Set internal loopback enable and
*         Set power down/up of Tx and Rx on Serdeses.
*         Not Valid for port speed 10G and 20G.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - system device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failed
* @retval GT_NOT_SUPPORTED         - not supported
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortSerdesLoopbackActivate
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN CPSS_PORT_SPEED_ENT      speed
)
{
    GT_STATUS                    rc;

    rc = prvCpssPxPortInternalLoopbackEnableSet(devNum, portNum, GT_TRUE);
    if (rc != GT_OK)
    {
         return rc;
    }
    if(CPSS_PORT_SPEED_1000_E == speed)
    {
        rc = prvCpssPxPortSerdesPartialPowerDownSet(devNum, portNum, GT_TRUE, GT_TRUE);
        if (rc != GT_OK)
        {
             return rc;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortSerdesPartialPowerDownSet function
* @endinternal
*
* @brief   Set power down/up of Tx and Rx on Serdeses.
*         Not Valid for port speed 10G and 20G
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - system device number
* @param[in] portNum                  - physical port number
* @param[in] powerDownRx              - Status of Serdes Rx (TRUE - power down, FALSE - power up).
* @param[in] powerDownTx              - Status of Serdes Tx (TRUE - power down, FALSE - power up).
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failed.
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortSerdesPartialPowerDownSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  powerDownRx,
    IN GT_BOOL                  powerDownTx
)
{
    GT_STATUS                    rc;
    GT_UOPT                      portGroup;
    MV_HWS_PORT_STANDARD         portMode;/* interface modes of port which use this configuration in Black Box format */
    CPSS_PORT_SPEED_ENT          speed; /* current speed of port */
    CPSS_PORT_INTERFACE_MODE_ENT ifMode; /* port interface mode */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);
    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portNum);
    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portNum);
    rc = prvCpssCommonPortIfModeToHwsTranslate(
        CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
    if (rc != GT_OK)
    {
        return rc;
    }
    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortPartialPowerDown(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], powerDownRx[%d], powerDownTx[%d])", devNum, portGroup, portNum, portMode, powerDownRx, powerDownTx);
    rc = mvHwsPortPartialPowerDown(CAST_SW_DEVNUM(devNum), portGroup, portNum, portMode, powerDownRx, powerDownTx);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
    }

    return rc;
}

/**
* @internal prvCpssPxPortTypeSet function
* @endinternal
*
* @brief   Sets port type (mostly means which mac unit used) on a specified port.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
*                                      macNum  - port MAC number
* @param[in] ifMode                   - Interface mode
* @param[in] speed                    - port speed
*                                       None.
*/
GT_VOID prvCpssPxPortTypeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_U32 macNum;

    /* port number already checked in caller function */
    (GT_VOID)prvCpssPxPortPhysicalPortMapCheckAndConvert(devNum, portNum, PRV_CPSS_PX_PORT_TYPE_MAC_E, &macNum);

    switch (ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E:
            switch(speed)
            {
                case CPSS_PORT_SPEED_1000_E:
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_GE_E;
                    break;
                case CPSS_PORT_SPEED_10000_E:
                case CPSS_PORT_SPEED_20000_E:
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XLG_E;
                    break;
                default:
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XLG_E;
                    break;
            }
            break;
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E:
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
        case CPSS_PORT_INTERFACE_MODE_100BASE_FX_E:
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_GE_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_QX_E:
        case CPSS_PORT_INTERFACE_MODE_HX_E:
        case CPSS_PORT_INTERFACE_MODE_XGMII_E:
        case CPSS_PORT_INTERFACE_MODE_RXAUI_E:
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XLG_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_SR_LR4_E:
        case CPSS_PORT_INTERFACE_MODE_KR4_E:
            if ((speed == CPSS_PORT_SPEED_100G_E)
                || (speed == CPSS_PORT_SPEED_107G_E)
                || (speed == CPSS_PORT_SPEED_102G_E))
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_CG_E;
                break;
            }
            if (speed == CPSS_PORT_SPEED_40000_E)
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XLG_E;
                break;
            }
            GT_ATTR_FALLTHROUGH;
        case CPSS_PORT_INTERFACE_MODE_CR4_E:
            if (speed == CPSS_PORT_SPEED_100G_E)
            {
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_CG_E;
                break;
            }
            GT_ATTR_FALLTHROUGH;
        case CPSS_PORT_INTERFACE_MODE_XLG_E:
        case CPSS_PORT_INTERFACE_MODE_KR_E:
        case CPSS_PORT_INTERFACE_MODE_KR2_E:
        case CPSS_PORT_INTERFACE_MODE_CR_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR_E:
        case CPSS_PORT_INTERFACE_MODE_SR_LR2_E:
        case CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E:
        case CPSS_PORT_INTERFACE_MODE_HGL_E: /* pay attention this definition for
        HGL relevant for Bobcat2 XLHGL_KR4 mode, in other cases could be required
        PRV_CPSS_PORT_HGL_E */
        case CPSS_PORT_INTERFACE_MODE_XHGS_E: /* If with Serdes   of 12.1875Ghz */
        case CPSS_PORT_INTERFACE_MODE_XHGS_SR_E:
        case CPSS_PORT_INTERFACE_MODE_KR_C_E:
        case CPSS_PORT_INTERFACE_MODE_KR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR_C_E:
        case CPSS_PORT_INTERFACE_MODE_CR2_C_E:
        case CPSS_PORT_INTERFACE_MODE_KR_S_E:
        case CPSS_PORT_INTERFACE_MODE_CR_S_E:
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_XLG_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_ILKN4_E:
        case CPSS_PORT_INTERFACE_MODE_ILKN8_E:
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_ILKN_E;
            break;
        case CPSS_PORT_INTERFACE_MODE_NA_E:
            /* leave CPSS init default value, see hwPpPhase1Part1 */
            break;
        default:
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[macNum].portType = PRV_CPSS_PORT_FE_E;
            break;
    }

    return;
}

/**
* @internal internal_cpssPxPortInterfaceModeGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
static GT_STATUS internal_cpssPxPortInterfaceModeGet
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32  portMacNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(ifModePtr);

    *ifModePtr = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacNum);
    return rc;
}

/**
* @internal cpssPxPortInterfaceModeGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port.
*
* @note   APPLICABLE DEVICES:      pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
GT_STATUS cpssPxPortInterfaceModeGet
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortInterfaceModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ifModePtr));

    rc = internal_cpssPxPortInterfaceModeGet(devNum, portNum, ifModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ifModePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssPxPortForceLinkDownEnable function
* @endinternal
*
* @brief   Enable Force link down on a specified port on specified device and
*         read current force link down state of it.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
*
* @param[out] linkDownStatusPtr        - (ptr to) current force link down state of port:
*                                      GT_TRUE  - enable force link down on port,
*                                      GT_FALSE - disable force link down on port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortForceLinkDownEnable
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL *linkDownStatusPtr
)
{
    GT_STATUS rc; /* return code */

    /* Get force link down status */
    if((rc = cpssPxPortForceLinkDownEnableGet(devNum, portNum, linkDownStatusPtr)) != GT_OK)
        return rc;

    if(*linkDownStatusPtr == GT_FALSE)
    {/* Set force link down */
        if((rc = cpssPxPortForceLinkDownEnableSet(devNum,portNum,GT_TRUE)) != GT_OK)
            return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortForceLinkDownDisable function
* @endinternal
*
* @brief   Disable Force link down on a specified port on specified device if
*         it was previously disabled.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number, CPU port number
* @param[in] linkDownStatus           - previous force link down state:
*                                      GT_TRUE  - enable force link down on port,
*                                      GT_FALSE - disable force link down on port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortForceLinkDownDisable
(
    IN GT_SW_DEV_NUM   devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL linkDownStatus
)
{
    GT_STATUS rc;   /* return code */

    if(linkDownStatus == GT_FALSE)
    {
        if((rc = cpssPxPortForceLinkDownEnableSet(devNum,portNum,GT_FALSE)) != GT_OK)
            return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortBusWidthSet function
* @endinternal
*
* @brief   Set bus interface width for ptp and fca.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not CPU port)
* @param[in] speed                    - port data speed, or CPSS_PORT_SPEED_10_E for deleted port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssPxPortBusWidthSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_U32      regAddr;            /* register address */
    GT_U32      ptpInterfaceWidth;     /* PTP interface width */
    GT_U32      fcaInterfaceWidth;     /* PTP interface width */
    GT_U32      portMacNum;
    GT_U32      fieldOffset;
    GT_U32      fieldLength;
    GT_STATUS   rc;
    PRV_CPSS_PX_PIPE_PORT_RESOURCE_STC  resource;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (speed == CPSS_PORT_SPEED_10_E)
    {
        /* Port has been deleted, use minimum widths */
        ptpInterfaceWidth = 1; /*64 bit*/
        fcaInterfaceWidth = 1;
    }
    else
    {
        /* Port is alive and well, use same widths as TXFIFO */
        cpssOsMemSet(&resource, 0, sizeof(resource));
        rc = prvCpssPxPipePortResourceConfigGet(
                devNum, portNum,/*OUT*/ &resource);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Interface width select */
        switch(resource.txfifoScdmaShiftersOutgoingBusWidth)
        {
            case PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_8B_E:
                ptpInterfaceWidth = 1; /*64 bit*/
                fcaInterfaceWidth = 1;
                break;

            case PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_32B_E:
                fcaInterfaceWidth = 3;/*256 bit*/
                ptpInterfaceWidth = 2;
                break;

            case PRV_CPSS_PX_TxFIFO_OutGoingBusWidth_64B_E:
                fcaInterfaceWidth = 4;/*512 bit*/
                ptpInterfaceWidth = 3;
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    fieldOffset = 4;
    fieldLength = 3;
    rc =  prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, fcaInterfaceWidth);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
    fieldOffset = 2;
    fieldLength = 2;
    rc =  prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, ptpInterfaceWidth);

    return rc;
}

/**
* @internal prvCpssPxPortPtpInterfaceWidthGet function
* @endinternal
*
* @brief   Get PTP interface width in bits
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number (not CPU port)
*
* @param[out] busWidthBitsPtr          - pointer to bus with in bits.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong speed
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssPxPortPtpInterfaceWidthGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                         *busWidthBitsPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;            /* register address */
    GT_U32      interfaceWidth;     /* PTP interface width */
    GT_U32      portMacNum;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    static GT_U32 busWidthArr[] =
    {
        /* 0x0 = 8BIT;  */   8
        /* 0x1 = 64BIT; */ ,64
        /* 0x2 = 256BIT */,256
        /* 0x3 = 512BIT */,512
    };

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(busWidthBitsPtr);

    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum,portNum,/*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);


    regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;

    /* Set interface width select bits[3:2] */
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 2, /*OUT*/&interfaceWidth);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    *busWidthArr = (GT_U32)(~0);
    if (interfaceWidth >=  sizeof(busWidthArr)/sizeof(busWidthArr[0]))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    *busWidthBitsPtr = busWidthArr[interfaceWidth];
    return GT_OK;
}


/**
* @internal prvCpssPxPortReset function
* @endinternal
*
* @brief   Power down port.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - interface mode according to which power down port
* @param[in] speed                    - port data  according to which power down port
* @param[in] qsgmiiFullDelete         - GT_TRUE - power down QSGMII interface
*                                      GT_FALSE - just free resources of this specific
*                                      member of QSGMII quadruplet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortReset
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         qsgmiiFullDelete
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    MV_HWS_PORT_STANDARD    portMode;   /* port interface/speed in terms of HWS */

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacMap);

    rc = prvCpssCommonPortIfModeToHwsTranslate(
        CAST_SW_DEVNUM(devNum),ifMode, speed, &portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

#ifndef GM_USED
    if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
    {
        /* reset just units used for loopback */
        if(CPSS_PORT_SPEED_1000_E == speed)
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, 0, portMacMap, GEMAC_X, RESET);
            rc = mvHwsMacReset(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, GEMAC_X, RESET);
        }
        else
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsMacReset(devNum[%d], portGroup[%d], macNum[%d], macType[%d], action[%d])", devNum, 0, portMacMap, XLGMAC, RESET);
            rc = mvHwsMacReset(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, XLGMAC, RESET);
        }
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in mvHwsMacReset, code %d for port[%d]\n", rc, portMacMap);
        }

        if(CPSS_PORT_SPEED_1000_E == speed)
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], portMacMap[%d], pcsType[%d], action[%d])", devNum, 0, portMacMap, GPCS, RESET);
            rc = mvHwsPcsReset(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, GPCS, RESET);
        }
        else
        {
            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPcsReset(devNum[%d], portGroup[%d], portMacMap[%d], pcsType[%d], action[%d])", devNum, 0, portMacMap, MMPCS, RESET);
            rc = mvHwsPcsReset(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, MMPCS, RESET);
        }
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in mvHwsPcsReset, code %d for port[%d]\n", rc, portMacMap);
        }
    }
    else
    {
        if((qsgmiiFullDelete) && (portMode != NON_SUP_MODE))
        {
            if(ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
            {
                /* the QSGMII sets 4 ports on the same SERDES and those 4 MACs should be aligned */
                /* NOTE: the qsgmiiFullDelete makes sure that 'last' port 'valid' in the 4 ports
                   will get into this code */
                portMacMap = portMacMap & (~0x3);/* clear the 2 LSBits */
            }


            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortReset(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], action[%d])", devNum, 0, portMacMap, portMode, PORT_POWER_DOWN);
            rc = mvHwsPortReset(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, PORT_POWER_DOWN);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Error in mvHwsPortReset, code %d for port[%d], portMode[%d]\n", rc, portMacMap, portMode);
            }
        }
    }
#else
    /* to prevent warnings in GM compilation */
    (GT_VOID)qsgmiiFullDelete;
#endif

    return GT_OK;
}

/**
* @internal prvCpssPxPortFcaBusWidthGet function
* @endinternal
*
* @brief   FCA bus width configuration.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed for given port
*/
GT_STATUS prvCpssPxPortFcaBusWidthGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_U32                         *widthBitsPtr
)
{
    GT_STATUS rc; /* return code */
    GT_U32 regAddr; /* register address */
    GT_U32 regValue; /* register value */
    GT_U32 fieldOffset; /* register value */
    GT_U32 fieldLength; /* register value */
    GT_U32 portMacNum; /* number of mac mapped to this physical port */
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;

    static GT_U32 fcaBusWidthBits[] =
    {
        /*   0x0 = Type A; BUS_8_BIT; 1G port         */   8
        /*   0x1 = Type B; BUS_64_BIT; 2.5G-10G port  */, 64
        /*   0x2 = Type C; BUS_128_BIT; No such port  */,128
        /*   0x3 = Type D; BUS_256_BIT; 40G port      */,256
        /*   0x4 = Type E; BUS_512_BIT; 100G port     */,512
    };
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(widthBitsPtr);

    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum,portNum,/*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (portMapShadowPtr->portMap.mappingType != CPSS_PX_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;
    fieldOffset = 4;
    fieldLength = 3;


    *widthBitsPtr = (GT_U32)(~0);
    rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, /*OUT*/&regValue);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,  LOG_ERROR_NO_MSG);
    }

    if (regValue >=  sizeof(fcaBusWidthBits)/sizeof(fcaBusWidthBits[0]))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);/* out of array boundary */
    }
    *widthBitsPtr = fcaBusWidthBits[regValue];
    return GT_OK;
}


/**
* @internal bcat2QsgmiiConfig function
* @endinternal
*
* @brief   Check if port needs configuration on MAC/PCS/Serdes levels,
*         force link down on ports of its quadruplet if application not
*         configured them yet.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] fullConfigPtr            - call port delete/create or just configure pizza
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*
* @note In QSGMII can't configure only one port, because of internal physical
*       synchronization issues, but we don't want application get linkup interrupts
*       from ports it
*
*/
static GT_STATUS bcat2QsgmiiConfig
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 powerUp,
    OUT GT_BOOL                 *fullConfigPtr
)
{

   GT_STATUS   rc; /* return code */
    GT_U32      i;  /* iterator */
    GT_PHYSICAL_PORT_NUM    portMacNum; /* MAC number mapped to given port */
    GT_U32  firstInQuadruplet;  /* mac number of first port in quadruplet */
    GT_U32  regAddr;    /* address of register */

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,portMacNum);

    if(powerUp == GT_FALSE)
    {
        /* the call is on 'physical port' (not on 'MAC port') */
        rc = cpssPxIngressPortTargetEnableSet(devNum, portNum, powerUp);
    }
    else
    {
        /* we can not force link UP ... we need to wait for interrupt
           from the MAC to know that ports is actually 'link UP'

           FIX JIRA : CPSS-4899 : Ports 0-11 in link up when working with higher ports
        */
        rc = GT_OK;
    }

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *fullConfigPtr = GT_TRUE;
    if((GT_TRUE == powerUp) && (CPSS_PORT_INTERFACE_MODE_QSGMII_E == PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacNum)))
    {
        *fullConfigPtr = GT_FALSE;
        return GT_OK;
    }

    firstInQuadruplet = portMacNum&0xFFFFFFFC;
    if(GT_TRUE == powerUp)
    {
        for(i = firstInQuadruplet; i < firstInQuadruplet+4; i++)
        {
            if(i == portMacNum)
            {
                /* set application configuration for force link down */
                rc = prvCpssPxPortForceLinkDownEnableSet(devNum,portNum,
                            CPSS_PORTS_BMP_IS_PORT_SET_MAC(&PRV_CPSS_PX_PP_MAC(devNum)->port.portForceLinkDownBmp,portNum));
                if(rc != GT_OK)
                {
                    return rc;
                }
                continue;
            }
            if(CPSS_PORT_INTERFACE_MODE_NA_E == PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, i))
            {
                /* because HWS configures all four ports of QSGMII quadruplet
                   when application requires to configure one of them,
                   force link down on GE MAC of port to prevent link on ports
                   not configured yet by application */
                PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,i,&regAddr);
                if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum),
                                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                       regAddr, 0, 1, 1) != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
                }
            }
            else if(CPSS_PORT_INTERFACE_MODE_QSGMII_E ==
                                        PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, i))
            {
                *fullConfigPtr = GT_FALSE;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    {
        for(i = firstInQuadruplet; i < firstInQuadruplet+4; i++)
        {
            if(i != portMacNum)
            {
                if(CPSS_PORT_INTERFACE_MODE_NA_E !=
                                            PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, i))
                {
                    *fullConfigPtr = GT_FALSE;
                    break;
                }
            }
        }

        if(*fullConfigPtr == GT_TRUE)
        {
            for(i = firstInQuadruplet; i < firstInQuadruplet+4; i++)
            {
                /* set application configuration for force link down */
                rc = prvCpssPxPortForceLinkDownEnableSet(devNum,portNum,
                            CPSS_PORTS_BMP_IS_PORT_SET_MAC(&PRV_CPSS_PX_PP_MAC(devNum)->port.portForceLinkDownBmp, portNum));
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
        else
        {
            /* set force link down enable in case that SERDES is still in power up state*/
            rc = prvCpssPxPortForceLinkDownEnableSet(devNum,portNum,GT_TRUE);
            if(rc != GT_OK)
            {
                return rc;
            }

        }
    }

    return GT_OK;
}

/**
* @internal checkMLGonePortConfigured function
* @endinternal
*
* @brief   Check if port one of four ports already configured to MLG mode.
*         force link down on ports of its quadruplet if application not
*         configured them yet.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
*
* @param[out] fullConfigPtr            - call port delete/create or just configure pizza
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS checkMLGonePortConfigured
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 powerUp,
    OUT GT_BOOL                 *fullConfigPtr,
    OUT GT_U8                   *configuredPortsBmpPtr
)
{
    GT_PHYSICAL_PORT_NUM    portMacNum; /* MAC number mapped to given port */
    GT_U32      firstPortInGop;  /* mac number of first port in GOP */
    GT_U32      i;

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);


    *fullConfigPtr = GT_TRUE;
    *configuredPortsBmpPtr = 0;
    firstPortInGop = (portMacNum & 0xFFFFFFFC);

    if(powerUp == GT_TRUE)
    {
        /* check the port's mode in all four ports in the GOP */
        for(i = firstPortInGop; i < (firstPortInGop + 4); i++)
        {
            if(PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, i) == CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E)
            {
                /* if one port in the GOP already configured to MLG mode,
                   no need to configure other ports */
                *fullConfigPtr = GT_FALSE;
                *configuredPortsBmpPtr = *configuredPortsBmpPtr | (1 << (i-firstPortInGop));
            }
            else if(PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, i) != CPSS_PORT_INTERFACE_MODE_NA_E)
            {
                /* if one port in the GOP already configured to other mode, the GOP can't be configured to
                   MLG mode for any port in this GOP */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }
    else
    { /* power Down */
        for(i = firstPortInGop; i < (firstPortInGop + 4); i++)
        {
            if(PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, i) == CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E)
            {
                *configuredPortsBmpPtr = *configuredPortsBmpPtr | (1 << (i-firstPortInGop));
                /* if other ports in the GOP already configured to MLG mode, the GOP configuraition will be saved
                    for these ports */
                if(i != portMacNum)
                {
                    *fullConfigPtr = GT_FALSE;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortSerdesFreeCheck function
* @endinternal
*
* @brief   Check that the Serdes for port Configuration is not free and not used
*         by another port
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_STATE             - on port cannot be configured when the serdes, that port need
*                                       for configuration is not free
* @retval GT_NOT_SUPPORTED         - i/f mode/speed pair not supported
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortSerdesFreeCheck
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS               rc;           /* return code */
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U8                   sdVecSize;
    MV_HWS_PORT_STANDARD    portMode;
    GT_U32                  portRef;
    GT_U32                  divider;
    GT_U32                  difference;
    GT_U32                  portGroup;
    GT_U32                  portMacMap;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    if(ifMode == CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E)
    {
        return GT_OK;
    }

    if(PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum,portMacMap) != PRV_CPSS_GE_PORT_GE_ONLY_E)
    {
        for(divider = 2; divider <= 4; divider += 2)
        {
            portRef = (portMacMap / divider) * divider;
            difference = portMacMap - portRef;
            if(difference > 0)
            {
                /* skip not initialized ports */
                if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E)
                {
                    continue;
                }
                rc = prvCpssCommonPortIfModeToHwsTranslate(
                    CAST_SW_DEVNUM(devNum), PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portIfMode,
                    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portRef].portSpeed, &portMode);

                if(GT_NOT_INITIALIZED == rc)
                {
                    continue;
                }

                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portRef);

                rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portRef, portMode, &curPortParams);
                if (GT_OK != rc)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
                sdVecSize   = curPortParams.numOfActLanes;

                if(sdVecSize > difference)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }
        }

        /* Checking is serdes, needs for current port don't used already by port with mac number AFTER current mac number */
        rc = prvCpssCommonPortIfModeToHwsTranslate(
            CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }


        portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

        rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), portGroup, portMacMap, portMode, &curPortParams);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        sdVecSize   = curPortParams.numOfActLanes;

        for(divider = 1; divider < sdVecSize; divider++)
        {
            if(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap + divider].portIfMode == CPSS_PORT_INTERFACE_MODE_NA_E &&
               PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap + divider].portSpeed  == CPSS_PORT_SPEED_NA_E )
            {
                /* no need to get rc = GT_NOT_INITIALIZED from prvCpssCommonPortIfModeToHwsTranslate(...)
                   that will cause 'CPSS ERROR LOG' to indicate error !
                   as the code below will ignore anyway !
                */
                continue;
            }

            rc = prvCpssCommonPortIfModeToHwsTranslate(
                CAST_SW_DEVNUM(devNum), PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap + divider].portIfMode,
                PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap + divider].portSpeed, &portMode);

            if(GT_NOT_INITIALIZED == rc)
            {
                continue;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxIsCgUnitInUse function
* @endinternal
*
* @brief   Check if given pair portNum and portMode use CG MAC
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port MAC number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - interface speed
*
* @param[out] isCgUnitInUsePtr         - GT_TRUE ? GC MAC in use
*                                      GT_FALSE ? GC MAC not in use
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxIsCgUnitInUse
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *isCgUnitInUsePtr
)
{
    GT_STATUS rc;
    GT_U32 portMacNum;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_STANDARD    portMode;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(
        devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(isCgUnitInUsePtr);

    *isCgUnitInUsePtr = GT_FALSE;

    rc = prvCpssCommonPortIfModeToHwsTranslate(
        CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
    if((rc != GT_OK) || (portMode == NON_SUP_MODE))
    {
        return GT_OK;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((curPortParams.portMacType == CGMAC) && (curPortParams.portPcsType == CGPCS))
    {
        *isCgUnitInUsePtr = GT_TRUE;
    }

    return GT_OK;
}


/**
* @internal prvCpssPxPortLoopbackDisableRestore function
* @endinternal
*
* @brief   Takes care to Set loopback to disable if it was
*         previousely set to enable.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portsBmp                 - physical port number
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortLoopbackDisableRestore
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORTS_BMP_STC              portsBmp,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      i;  /* iterator */
    CPSS_PORT_INTERFACE_MODE_ENT prevIfMode; /* interface mode before setting */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    for(i = 0; i < PRV_CPSS_PP_MAC(devNum)->numOfPorts; i++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp, i))
        {
            if(powerUp == GT_TRUE)
            {
                prevIfMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, i);
                if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == prevIfMode)
                {
                    rc = prvCpssPxPortInternalLoopbackEnableSet(devNum, i, GT_FALSE);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
            else
            {
                if(CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode)
                {
                    rc = prvCpssPxPortInternalLoopbackEnableSet(devNum, i, GT_FALSE);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }
    return GT_OK;
}

GT_STATUS prvCpssPxPortRefClockUpdate
(
    IN  GT_SW_DEV_NUM                   devNum,
    MV_HWS_PORT_STANDARD                portMode,
    IN  GT_U16                          *serdesArrPtr,
    IN  GT_U8                           serdesSize,
    IN  MV_HWS_REF_CLOCK_SOURCE         refClockSource,
    OUT MV_HWS_REF_CLOCK_SUP_VAL        *refClockPtr
)
{
    GT_STATUS                       rc;
    MV_HWS_CPLL_OUTPUT_FREQUENCY    cpllOutFreq;
    GT_U32                          i;
    GT_U32                          dividerValue = 2;
    GT_U32                          regAddr;


    CPSS_NULL_PTR_CHECK_MAC(serdesArrPtr);

    if(serdesSize == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    cpllOutFreq = prvCpssPxPortCpllConfigArr[serdesArrPtr[0]].outFreq;


    if(prvCpssPxPortCpllConfigArr[serdesArrPtr[0]].valid == GT_TRUE)
    {
        /**************************************************************************/
        /*  Dividers values are calculated according to cpllOutFreq               */
        /**************************************************************************/

        if(refClockSource == PRIMARY_LINE_SRC)
        {   /* CPLL as ref clock source */
            if(cpllOutFreq == MV_HWS_312MHz_OUT)
            {
                switch(portMode)
                {
                    case _5GBaseR:
                    case _12_5GBase_KR:
                    case _25GBase_KR2:
                    case _50GBase_KR4:
                        *refClockPtr = MHz_78;
                        dividerValue = 3;       /*div_by_4 (312.5 / 4 = 78.125) --> regValue = 3*/
                        break;

                    case _107GBase_KR4:
                        *refClockPtr = MHz_312;
                        dividerValue = 1;       /*div_by_1 (312.5 / 1 = 312.5) --> regValue = 1*/
                        break;

                    default:
                        *refClockPtr = MHz_156;
                        dividerValue = 2;       /*div_by_2 (312.5 / 2 = 156.25) --> regValue = 2*/
                        break;
                }
            }
            else if(cpllOutFreq == MV_HWS_164MHz_OUT && portMode == _52_5GBase_KR2)
            {
                *refClockPtr = MHz_164;
                dividerValue = 1;           /*div_by_1 (164.24 / 1 = 164.24) --> regValue = 1*/
            }

        }
        else /* SECONDARY */
        {
                switch(portMode)
                {
                    case _5GBaseR:
                    case _12_5GBase_KR:
                    case _25GBase_KR2:
                    case _50GBase_KR4:
                        *refClockPtr = MHz_78;
                        dividerValue = 2;               /*div_by_2 (156.25 / 2 = 78.125) --> regValue = 2*/
                        break;
                    default:
                        *refClockPtr = MHz_156;
                        dividerValue = 1;               /*div_by_1 (156.25 / 1 = 156.25) --> regValue = 1*/
                        break;
                }
        }

        for(i = 0; i < serdesSize; i++)
        {
            regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->serdesConfig[serdesArrPtr[i]].serdesExternalReg1;
            rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 11, 2, dividerValue);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }
    return GT_OK;
}

/**
* @internal internal_cpssPxPortInterfaceSpeedSupportGet function
* @endinternal
*
* @brief   Check if given pair ifMode and speed supported by given port on
*         given device
*
* @note   APPLICABLE DEVICES:      pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @param[out] supportedPtr             ? GT_TRUE ? (ifMode; speed) supported
*                                      GT_FALSE ? (ifMode; speed) not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortInterfaceSpeedSupportGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr
)
{
    GT_STATUS rc;
    GT_U32  portMacNum;      /* MAC number */
    GT_BOOL ifIsSupported;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(supportedPtr);

    if(ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E || speed >= CPSS_PORT_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    *supportedPtr = GT_FALSE;

    rc = prvCpssPxPortSupportedModeCheck(devNum,portNum,ifMode,/*OUT*/&ifIsSupported);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (ifIsSupported == GT_FALSE)
    {
        return GT_OK;
    }

    return prvCpssCommonPortInterfaceSpeedGet(CAST_SW_DEVNUM(devNum), portMacNum,
                                                ifMode, speed,
                                                supportedPtr);
}

/**
* @internal cpssPxPortInterfaceSpeedSupportGet function
* @endinternal
*
* @brief   Check if given pair ifMode and speed supported by given port on
*         given device
*
* @note   APPLICABLE DEVICES:      pipe
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (not CPU port)
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - port data speed
*
* @param[out] supportedPtr             ? GT_TRUE ? (ifMode; speed) supported
*                                      GT_FALSE ? (ifMode; speed) not supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if one of input parameters wrong
* @retval GT_BAD_PTR               - if supportedPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortInterfaceSpeedSupportGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    OUT GT_BOOL                         *supportedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortInterfaceSpeedSupportGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, ifMode, speed, supportedPtr));

    rc = internal_cpssPxPortInterfaceSpeedSupportGet(devNum, portNum, ifMode, speed, supportedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, ifMode, speed, supportedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssPxPortFecModeSet function
* @endinternal
*
* @brief   Configure Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortFecModeSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PORT_FEC_MODE_ENT   mode
)
{
    GT_STATUS   rc;         /* return code */
    MV_HWS_PORT_FEC_MODE    fecMode; /* FEC state on port */
    GT_U32      portMacNum; /* MAC number */
    MV_HWS_PORT_STANDARD portMode;  /* HWS port interface mode */
    CPSS_PORT_INTERFACE_MODE_ENT    portIfMode;
    CPSS_PORT_SPEED_ENT             portSpeed;
    CPSS_PORTS_BMP_STC              portsBmp;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    switch(mode)
    {
        case CPSS_PORT_FEC_MODE_ENABLED_E:
            fecMode = FC_FEC;   /* FC_FEC */
            break;

        case CPSS_PORT_FEC_MODE_DISABLED_E:
            fecMode = FEC_OFF;
            break;

        case CPSS_PORT_RS_FEC_MODE_ENABLED_E:
            fecMode = RS_FEC;
            break;

        case CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portIfMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacNum);
    portSpeed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum,portMacNum);

    if((CPSS_PORT_INTERFACE_MODE_NA_E == portIfMode)
       || (CPSS_PORT_SPEED_NA_E == portSpeed))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(
        CAST_SW_DEVNUM(devNum), portIfMode,portSpeed,&portMode);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(GT_FALSE == hwsIsFecModeSupported(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, fecMode))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    /* store current port configuration */
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsBmp);
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, portNum);

    /* HWS Elements DB update */
    if(GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, NON_SUP_MODE, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if(curPortParams.portFecMode == fecMode)
    {
        return GT_OK;
    }

    rc = cpssPxPortModeSpeedSet(devNum, &portsBmp, GT_FALSE, portIfMode, portSpeed);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* update HWS elements DB */
    rc = hwsPortModeParamsSetFec(CAST_SW_DEVNUM(devNum),0,portMacNum,portMode,fecMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, portIfMode, portSpeed);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }



    return GT_OK;
}

/**
* @internal cpssPxPortFecModeSet function
* @endinternal
*
* @brief   Configure Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortFecModeSet
(
    IN  GT_SW_DEV_NUM             devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  CPSS_PORT_FEC_MODE_ENT mode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortFecModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, mode));

    rc = internal_cpssPxPortFecModeSet(devNum, portNum, mode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, mode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortFecModeGet function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - current Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortFecModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_FEC_MODE_ENT   *modePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      portGroupId;/* iterator of port groups */
    MV_HWS_PORT_FEC_MODE    fecMode; /* FEC state on port */
    GT_U32      portMacNum; /* MAC number */
    MV_HWS_PORT_STANDARD portMode;  /* port i/f mode and speed translated to
                                        BlackBox enum */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(modePtr);

    rc = prvCpssCommonPortIfModeToHwsTranslate(
        CAST_SW_DEVNUM(devNum),
        PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacNum),
        PRV_CPSS_PX_PORT_SPEED_MAC(devNum,portMacNum),
        &portMode);
    if(rc != GT_OK)
    {
        *modePtr = CPSS_PORT_FEC_MODE_DISABLED_E;
        return GT_OK;
    }

    switch(portMode)
    {
        case _10GBase_KR:
        case _10GBase_SR_LR:
        case _12GBaseR:
        case _20GBase_KR2:
        case _20GBase_SR_LR2:
        case _25GBase_KR:
        case _25GBase_CR:
        case _25GBase_SR:
        case _25GBase_KR_C:
        case _25GBase_KR_S:
        case _25GBase_CR_S:
        case _25GBase_CR_C:
        case _26_7GBase_KR:
        case _40GBase_SR_LR4:
        case _40GBase_KR4:
        case _40GBase_KR2:
        case _40GBase_CR4:
        case _48GBaseR4:
        case _50GBase_KR2:
        case _50GBase_SR2:
        case _50GBase_KR4:
        case _50GBase_CR2_C:
        case _50GBase_KR2_C:
        case _50GBase_CR2:
        case _52_5GBase_KR2:
        case _100GBase_KR4:
        case _100GBase_CR4:
        case _100GBase_SR4:
        case _102GBase_KR4:
        case _107GBase_KR4:
            break;
        default:
            *modePtr = CPSS_PORT_FEC_MODE_DISABLED_E;
            return GT_OK;
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum),portMacNum);

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFecCofigGet(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], *portFecEn)", devNum, portGroupId, portMacNum, portMode);
    rc = mvHwsPortFecCofigGet(CAST_SW_DEVNUM(devNum), portGroupId, portMacNum, portMode, &fecMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

    switch(fecMode)
    {
        case FC_FEC:
            *modePtr = CPSS_PORT_FEC_MODE_ENABLED_E;
            break;

        case FEC_OFF:
            *modePtr = CPSS_PORT_FEC_MODE_DISABLED_E;
            break;

        case RS_FEC:
            *modePtr = CPSS_PORT_RS_FEC_MODE_ENABLED_E;
            break;

        case RS_FEC_544_514:
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssPxPortFecModeGet function
* @endinternal
*
* @brief   Read current Forward error correction (FEC) mode of port
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] modePtr                  - current Forward error correction mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_INITIALIZED       - if port not configured
* @retval GT_BAD_PTR               - modePtr == NULL
* @retval GT_NOT_SUPPORTED         - if FEC not supported on interface mode defined now on port
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortFecModeGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PORT_FEC_MODE_ENT *modePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortFecModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, modePtr));

    rc = internal_cpssPxPortFecModeGet(devNum, portNum, modePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, modePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal prvCpssPxPortInterfaceModeHwGet function
* @endinternal
*
* @brief   Gets Interface mode on a specified port from HW.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] ifModePtr                - interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - wrong media interface mode value received
*/
GT_STATUS prvCpssPxPortInterfaceModeHwGet
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT   *ifModePtr
)
{
    GT_U32      portMacNum; /* MAC number */
    GT_STATUS   rc;         /* return code */
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32 portGroupId; /*the port group Id - support multi-port-groups device */
    MV_HWS_PORT_STANDARD    portMode; /* port mode in HWS format */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(ifModePtr);

    *ifModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E == portMapShadowPtr->portMap.mappingType)
    {/* return dummy ifMode to support legacy behavior */
        *ifModePtr = CPSS_PORT_INTERFACE_MODE_MII_E;
        return GT_OK;
    }

    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum);

    if((PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum,portMacNum))
       && (portMacNum%4 != 0))
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortInterfaceGet(devNum[%d], portGroup[%d], phyPortNum[%d], *portModePtr)", devNum, portGroupId, portMacNum);
        rc = mvHwsPortInterfaceGet(CAST_SW_DEVNUM(devNum), portGroupId, portMacNum&0xFFFFFFFC, &portMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        if (QSGMII == portMode)
        {
            goto hws2cpss;
        }
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortInterfaceGet(devNum[%d], portGroup[%d], phyPortNum[%d], *portModePtr)", devNum, portGroupId, portMacNum);
    rc = mvHwsPortInterfaceGet(CAST_SW_DEVNUM(devNum), portGroupId, portMacNum, &portMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

hws2cpss:
    switch(portMode)
    {
        case _100Base_FX:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_100BASE_FX_E;
            break;

        case SGMII:
        case SGMII2_5:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_SGMII_E;
            break;

        case _1000Base_X:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_1000BASE_X_E;
            break;

        case QSGMII:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_QSGMII_E;
            break;

        case _10GBase_KX4:
        case _20GBase_KX4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_XGMII_E;
            break;

        case _10GBase_KX2:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_HX_E;
            break;

        case _10GBase_KR:
        case _100GBase_KR10:
        case _12_1GBase_KR:
        case _12_5GBase_KR:
        case _25GBase_KR:
        case _5GBaseR:
        case _26_7GBase_KR:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR_E;
            break;

        case _25GBase_CR:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR_E;
            break;

        case _25GBase_KR_S:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR_S_E;
            break;

        case _25GBase_CR_S:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR_S_E;
            break;

        case _20GBase_KR2:
        case _25GBase_KR2:
        case _50GBase_KR2:
        case _40GBase_KR2:
        case _52_5GBase_KR2:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR2_E;
            break;

        case _50GBase_CR2:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR2_E;
            break;

        case _40GBase_KR4:
        case _50GBase_KR4:
        case _100GBase_KR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR4_E;
            break;
        case _107GBase_KR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR4_E;
            break;

        case _40GBase_CR4:
        case _100GBase_CR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR4_E;
            break;

        case HGL:
        case HGL16G:
        case XLHGL_KR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_HGL_E;
            break;

        case RXAUI:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_RXAUI_E;
            break;

        case _10GBase_SR_LR:
        case _100GBase_SR10:
        case _5_625GBaseR:
        case _25GBase_SR:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_SR_LR_E;
            break;

        case _20GBase_SR_LR2:
        case _50GBase_SR2:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_SR_LR2_E;
            break;
        case _40GBase_SR_LR4:
        case _100GBase_SR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_SR_LR4_E;
            break;

        case _100GBase_MLG:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E;
            break;

        case _12GBaseR:
        case _24GBase_KR2:
        case _48GBaseR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_XHGS_E;
            break;
        case _12GBase_SR:
        case _48GBase_SR4:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_XHGS_SR_E;
            break;
        case CHGL:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CHGL_12_E;
            break;

        case _25GBase_KR_C:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR_C_E;
            break;

        case _50GBase_KR2_C:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_KR2_C_E;
            break;

        case _25GBase_CR_C:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR_C_E;
            break;

        case _50GBase_CR2_C:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_CR2_C_E;
            break;

        default:
            *ifModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
            break;
    }

    return GT_OK;

}


static GT_STATUS internal_cpssPxPortRefClockSourceOverrideEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         overrideEnable,
    IN  CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource
)
{
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum, portMacNum);

    if(overrideEnable == GT_FALSE)
    {
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.enableOverride = overrideEnable;
        /* invalidate entry */
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.portRefClockSource = CPSS_PORT_REF_CLOCK_SOURCE_LAST_E;
    }
    else
    {
        if((refClockSource == CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E) || (refClockSource == CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E))
        {
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.enableOverride = overrideEnable;
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.portRefClockSource = refClockSource;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxPortRefClockSourceOverrideEnableSet function
* @endinternal
*
* @brief   Enables/disables reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] overrideEnable           - override
*                                      GT_TRUE - use preconfigured refClock source;
*                                      GT_FALSE - use default refClock source;
* @param[in] refClockSource           - reference clock source.
*                                      Not relevant when overrideEnable is false.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, refClockSource
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortRefClockSourceOverrideEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         overrideEnable,
    IN  CPSS_PORT_REF_CLOCK_SOURCE_ENT  refClockSource
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortRefClockSourceOverrideEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, overrideEnable, refClockSource));

    rc = internal_cpssPxPortRefClockSourceOverrideEnableSet(devNum, portNum, overrideEnable, refClockSource);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, overrideEnable, refClockSource));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

static GT_STATUS internal_cpssPxPortRefClockSourceOverrideEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *overrideEnablePtr,
    OUT CPSS_PORT_REF_CLOCK_SOURCE_ENT  *refClockSourcePtr
)
{
    GT_U32  portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(overrideEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(refClockSourcePtr);

    *overrideEnablePtr = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.enableOverride;
    *refClockSourcePtr = PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portRefClock.portRefClockSource;

    return GT_OK;
}

/**
* @internal cpssPxPortRefClockSourceOverrideEnableGet function
* @endinternal
*
* @brief   Gets status of reference clock source override for port create.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] overrideEnablePtr        - (pointer to) override enable status
* @param[out] refClockSourcePtr        - (pointer to) reference clock source.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortRefClockSourceOverrideEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *overrideEnablePtr,
    OUT CPSS_PORT_REF_CLOCK_SOURCE_ENT  *refClockSourcePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortRefClockSourceOverrideEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, overrideEnablePtr, refClockSourcePtr));

    rc = internal_cpssPxPortRefClockSourceOverrideEnableGet(devNum, portNum, overrideEnablePtr, refClockSourcePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, overrideEnablePtr, refClockSourcePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}




/**
* @internal prvCpssPxPortDelete function
* @endinternal
*
* @brief   Power down port and free all allocated for it resources.
*
* @note   APPLICABLE DEVICES:      pipe
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
* @param[in] ifMode                   - interface mode according to which power down port
* @param[in] speed                    - port data  according to which power down port
* @param[in] qsgmiiFullDelete         - GT_TRUE - power down QSGMII interface
*                                      GT_FALSE - just free resources of this specific
*                                      member of QSGMII quadruplet
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortDelete
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         qsgmiiFullDelete
)
{
    GT_STATUS               rc;         /* return code */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;/* interface configured on port now */
    CPSS_PORT_SPEED_ENT          *portSpeedPtr; /* speed configured on port now */

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    portIfModePtr = &(PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap));
    portSpeedPtr = &(PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap));

    /* if on port was configured interface - reset physical layer and free resources */
    if(*portIfModePtr != CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        rc = prvCpssPxPortReset(devNum, portNum, *portIfModePtr,
                                           *portSpeedPtr, qsgmiiFullDelete);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPxPortDelete: error in prvCpssPxPortReset, portNum = %d\n", portNum);
        }

        rc = prvCpssPxPortResourcesRelease(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPxPortDelete: error in prvCpssPxPortResourcesRelease, portNum = %d\n", portNum);
        }

        /* Set PTP interface width */
        rc = prvCpssPxPortBusWidthSet(devNum, portNum, CPSS_PORT_SPEED_10_E);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPxPortDelete: error in prvCpssPxPortBusWidthSet, portNum = %d\n", portNum);
        }

        rc = prvCpssPxPortPizzaArbiterIfDelete(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPxPortDelete: error in prvCpssPxPortPizzaArbiterIfDelete, portNum = %d\n", portNum);
        }
    }

    if(!((*portIfModePtr == ifMode) && (*portSpeedPtr == speed)))
    {
        /* reset port according to new interface */
        rc = prvCpssPxPortReset(devNum, portNum, ifMode, speed,
                                           qsgmiiFullDelete);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPxPortDelete: error in prvCpssPxPortReset, portNum = %d\n", portNum);
        }
    }

    return GT_OK;
}


/**
* @internal internal_cpssPxPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmpPtr              - (pointer to) physical ports bitmap (or CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if port was not initialized
*                                     (does not exist in port
*                                     DB)
*
* @note Pay attention! Before configure CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E
*       MUST execute power down for port with this interface.
*
*/
static GT_STATUS internal_cpssPxPortModeSpeedSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORTS_BMP_STC              *portsBmpPtr,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS   rc; /* return code */
    MV_HWS_PORT_STANDARD    portMode;
    CPSS_PORT_INTERFACE_MODE_ENT *portIfModePtr;
    CPSS_PORT_SPEED_ENT          *portSpeedPtr;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_BOOL                 lbPort;
    MV_HWS_REF_CLOCK_SUP_VAL refClock;
    MV_HWS_REF_CLOCK_SOURCE  refClockSource;
    GT_PHYSICAL_PORT_NUM    portNum;    /* iterator */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    GT_BOOL                 supported;  /* is ifMode/speed supported on port */
    GT_BOOL                 fullConfig; /* for QSGMII mode:
                                                    one serdes serves four ports, when one port of
                                                    the quadruplete configured three others configured too,
                                                    so when application asks to configure another port from
                                                    quadruplete we don't need to execute full configuretion of new port
                                           for MLG mode:
                                                    four Serdeses serves four MLG ports, when one port of the quadruplete
                                                    configured, three others ports share the same four Serdeses.
                                                    So when application asks to configure another port from quadruplete,
                                                    we don't need to execute full configuretion of new port */
    GT_U8                   configuredPortsBmp;     /*for MLG mode:
                                                    the first port in the quadruplete should be
                                                    configured first on powerUp and last on powerDown */
    PRV_CPSS_PX_PORT_STATE_STC      portStateStc;  /* current port state */
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U8   sdVecSize;
    GT_U16  *sdVectorPtr;
    GT_BOOL isPortAlreadyConfigured;
    GT_BOOL isCgUnitInUse = GT_FALSE;
    GT_BOOL apEnabled = GT_FALSE; /* indicate ap enabled and ap port enabled */
    CPSS_PX_PORT_AP_PARAMS_STC  apParams;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;
/* to support RM-9725147 */
    GT_U32  regAddr;
    GT_U32  txFifoMin;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* check if AP port - in case of AP port cpssPxPortModeSpeedSet is called with ifMode and speed with addition of _NA_E value to the values requested*/
    if (ifMode > CPSS_PORT_INTERFACE_MODE_NA_E && speed > CPSS_PORT_SPEED_NA_E)
    {
        /* check if AP is enabled */
        rc = cpssPxPortApEnableGet(devNum, &apEnabled);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "cpssPxPortModeSpeedSet - try to set AP port but can not get FW status");
        }

        if (apEnabled)
        {
            for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
            {
                if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, portNum))
                {
                    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
                    break;
                }
            }
            if(portNum < PRV_CPSS_PX_PORTS_NUM_CNS)
            {
                rc = prvCpssPxPortApHighLevelConfigurationSet(devNum, portNum, powerUp, (ifMode - CPSS_PORT_INTERFACE_MODE_NA_E), (speed - CPSS_PORT_SPEED_NA_E));
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortModeSpeedSet - fail to set pizza for AP port");
                }
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "cpssPxPortModeSpeedSet - try to set AP port but wrong port number");
            }

            /* send ack to SrvCPU that Pizza was configure*/
            if(powerUp)
            {
                if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
                {
                    rc = prvCpssPxPortApPortModeAckSet(devNum, portNum, (ifMode - CPSS_PORT_INTERFACE_MODE_NA_E), (speed - CPSS_PORT_SPEED_NA_E));
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortModeSpeedSet - fail to send ack to srvCPU that pizza was set");
                    }
                }
            }

            rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, ifMode - CPSS_PORT_INTERFACE_MODE_NA_E, speed - CPSS_PORT_SPEED_NA_E, &portMode);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortModeSpeedSet - failed to translate port mode");
            }
            rc = hwsPortModeParamsSetMode(devNum,0, portMacMap, powerUp ? portMode : NON_SUP_MODE);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssPxPortModeSpeedSet - failed to update Elements databse on host CPU");
            }

            return rc;
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "cpssPxPortModeSpeedSet - try to set AP port but FW was not loaded");
        }
    }

    /* before configuring cpssPxPortModeSpeedSet check that ap is not running on port */
    for(portNum = 0; portNum < PRV_CPSS_PP_MAC(devNum)->numOfPorts; portNum++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, portNum))
        {
            PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

            rc = cpssPxPortApPortConfigGet(devNum, portNum, &apEnabled, &apParams);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (apEnabled == GT_TRUE) {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    if(powerUp == GT_TRUE)
    {
        if(ifMode >= CPSS_PORT_INTERFACE_MODE_NA_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(speed >= CPSS_PORT_SPEED_NA_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* fix JIRA: CPSS-6430 : 287Mhz 1x50Gb Interface should be blocked */
        /* check limitations of speed related to coreClock of the device */
        /* according to section "Port Speed Per Core Clock Support" in the
           Pipe Family Hardware Specifications document */
        if(PRV_CPSS_PP_MAC(devNum)->coreClock < 350)/*288*/
        {
            switch(speed)
            {
                /* relevant speeds from prv_PIPE_speedPlaceArr[] */
                case CPSS_PORT_SPEED_50000_E:
                case CPSS_PORT_SPEED_52500_E:
                case CPSS_PORT_SPEED_47200_E:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "The coreClock[%d MHz] not supports the speed [%d]",
                        PRV_CPSS_PP_MAC(devNum)->coreClock,
                        speed);
                default:
                    /* NOTE: the speeds of 100G,107G are blocked by the check that
                             relevant also to 350CC */
                    break;

            }
        }
        /* not 'else if' */
        if(PRV_CPSS_PP_MAC(devNum)->coreClock < 450)/*350,288*/
        {
            switch(speed)
            {
                /* relevant speeds from prv_PIPE_speedPlaceArr[] */
                case CPSS_PORT_SPEED_100G_E:
                case CPSS_PORT_SPEED_102G_E:
                case CPSS_PORT_SPEED_107G_E:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                        "The coreClock[%d MHz] not supports the speed [%d]",
                        PRV_CPSS_PP_MAC(devNum)->coreClock,
                        speed);
                default:
                    break;

            }
        }
    }
    else
    {
        /* in MLG mode we want to power down only the specific port number
           from the group */
        if (ifMode != CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E)
        {
            /* for all other port modes: we not care about those ifMode,
               speed from the caller ! as the port is going down
               set dummy values */
            ifMode = CPSS_PORT_INTERFACE_MODE_SGMII_E;
            speed  = CPSS_PORT_SPEED_1000_E;
        }
    }

    if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        rc = prvCpssPxPortLoopbackDisableRestore(devNum, *portsBmpPtr, powerUp, ifMode);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* return prvCpssPxPortModeSpeedSet(devNum, *portsBmpPtr, powerUp, ifMode,speed);*/

    if(powerUp == GT_TRUE)
    {
        rc = prvCpssCommonPortIfModeToHwsTranslate(
            CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = prvCpssPxSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
        if (rc != GT_OK)
        {
            return rc;
        }

        lbPort = (CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E == ifMode) ?
                                                        GT_TRUE : GT_FALSE;
    }
    else
    {/* to prevent warnings */
        portMode = NON_SUP_MODE;
        refClock = MHz_156;
        lbPort = GT_FALSE;
    }

    for(portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
    {
        if(CPSS_PORTS_BMP_IS_PORT_SET_MAC(portsBmpPtr, portNum))
        {
            PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);
        }
        else
        {
            continue;
        }

        portIfModePtr = &(PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portMacMap));
        portSpeedPtr = &(PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacMap));

        rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssPxPortCaelumModeSpeedSet: error in prvCpssPxPortPhysicalPortMapShadowDBGet, portNum = %d\n", portNum);
        }

        rc = prvCpssPxIsCgUnitInUse(devNum, portNum, ifMode, speed, &isCgUnitInUse);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(isCgUnitInUse == GT_TRUE)
        {
            /* init CG port register database */
            rc = prvCpssPxCgPortDbInvalidate(devNum, portMacMap, GT_FALSE);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

        rc = prvCpssPxPortIsAlreadyConfigured(devNum, portNum, powerUp, *portIfModePtr, *portSpeedPtr, ifMode, speed, &isPortAlreadyConfigured);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(isPortAlreadyConfigured)
        {
            continue;
        }


        if(powerUp == GT_FALSE)
        {
            /* we remove the ports, so need to use current interface and speed
             * if configured, not the values from the caller */
            if((CPSS_PORT_INTERFACE_MODE_NA_E != *portIfModePtr) &&
                                        (CPSS_PORT_SPEED_NA_E != *portSpeedPtr))
            {
                ifMode = *portIfModePtr;
                speed =  *portSpeedPtr;
            }
            else
            {
                continue; /* port is already power down , go to next port */
            }
        }
        else
        {
            rc = prvCpssPxPortSerdesFreeCheck(devNum,portNum,ifMode,speed);
            if(rc != GT_OK)
            {
                 return rc;
            }
        }

        rc = prvCpssCommonPortInterfaceSpeedGet(CAST_SW_DEVNUM(devNum), portMacMap,
                                                ifMode, speed, &supported);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(!supported)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
        rc = prvCpssCommonPortIfModeToHwsTranslate(
            CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = hwsPortModeParamsSetMode(CAST_SW_DEVNUM(devNum),0, portMacMap, powerUp ? portMode : NON_SUP_MODE);
        if(rc != GT_OK)
        {
            return rc;
        }
        /****************************************************************************************
            PART 1 - port disable

            This configuration will trigger:
                  a. The MAC will issue an indication towards the EGF
                     to stop sending packets to it.
                  b. The MAC will issue ready to the TXFIFO and thus the
                     traffic on the way to the port will be flushed.

            PART 2 - Check all traffic to the port was flushed

            1. Read Port<%n>DescCounter in the TXQ and wait for it to reach 0.
               This counter indicates how many descriptors are allocated for the port in the TXQ queues.
            2. Read TxDMA Port <%n> Credit Counter register in the TXQ and wait for it to reach the corresponding configured descriptor credits threshold in the TXDMA (Descriptor Credits SCDMA %p).
            3. In the TXDMA regfile - read the TXFIFO credits counters status and wait for it to reach zero.
               There is a separate counter for headers (scdma_%p_header_txfifo_counter) and payloads (scdma_%p_payload_txfifo_counter).
               These counters count the amount of data that is inside the TXDMA/TXFIFO on its way to the port.

        *****************************************************************************************/

        rc = prvCpssPxPortStateDisableAndGet(devNum, portNum, &portStateStc);
        if (rc != GT_OK)
        {
            return rc;
        }

        fullConfig = GT_TRUE;
        if(CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode)
        {
            rc = bcat2QsgmiiConfig(devNum, portNum, powerUp, &fullConfig);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in bcat2QsgmiiConfig, portNum = %d, powerUp = %d\n", portNum, powerUp);
            }
        }

        /* if one Port in the same GOP already configured to MLG mode, fullConfig=GT_FALSE */
        if(ifMode == CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E)
        {
            rc = checkMLGonePortConfigured(devNum, portNum, powerUp, &fullConfig, &configuredPortsBmp);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in checkMLGonePortConfigured, portNum = %d, powerUp = %d\n", portNum, powerUp);
            }

            if((portNum & 0xFFFFFFFC) == portNum) /* the current port is the first port (0) of the quad */
            {
                if((GT_FALSE == powerUp) && (0 != (configuredPortsBmp & (~0x1)))/* ports 1,2,3 of the quad are configured */)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Cannot power-down MLG's first port since some of the other ports are not deleted (bitmap view): [1,2,3]=[%d,%d,%d]\n",
                                                    (configuredPortsBmp&2)?1:0 ,(configuredPortsBmp&4)?1:0, (configuredPortsBmp&8)?1:0);
                }
            }
            else /* the current ports is port 1,2 or 3 of the quad */
            {
                if((GT_TRUE == powerUp) && (0 == (configuredPortsBmp & 0x1))/* first port (0) of the quad is not configured*/)
                {
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Cannot power-up MLG port since port %d is not configured\n", (portNum & 0xFFFFFFFC));
                }
            }
        }

        /****************************************************************************************
            PART 3 - Remove resources allocated to the port

        *****************************************************************************************/

        rc = prvCpssPxPortDelete(devNum, portNum, ifMode, speed,
                                            fullConfig);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortDelete, portNum = %d, ifMode = %d, speed = %d\n", portNum, ifMode, speed);
        }


        /* save new interface mode in DB after prvCpssPxPortDelete */
        *portIfModePtr = CPSS_PORT_INTERFACE_MODE_NA_E;
        *portSpeedPtr = CPSS_PORT_SPEED_NA_E;

        if(!powerUp)
        {
            /* calling to prvCpssDxChPortStateRestore here will cause to BC3 port get stuck with TXQ buffers,
               so EGF filer state will be enabled during next link up event */
            rc = cpssPxPortEnableSet(devNum, portNum, portStateStc.portEnableState);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in cpssPxPortEnableSet, portNum = %d\n", portNum);
            }
            if (GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum, PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
            {
                /* just detach MAC from Led */
                rc = prvCpssPxLedErrataPortLedMacTypeSet(devNum, portNum, PRV_CPSS_PORT_NOT_EXISTS_E);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxLedErrataPortLedMacTypeSet() at power down, portNum = %d\n", portNum);
                }
            }

            /* update port DB with the default port type value in order to prevent using CG MAC
               when it is disabled  */
            PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portType = PRV_CPSS_PORT_XLG_E;

            if(isCgUnitInUse == GT_TRUE)
            {
                /* invalidate CG port register database */
                rc = prvCpssPxCgPortDbInvalidate(devNum, portMacMap, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            continue;
        }

        rc = prvCpssPxPortPizzaArbiterIfConfigure(devNum, portNum,  ((speed < CPSS_PORT_SPEED_1000_E) ? CPSS_PORT_SPEED_1000_E : speed));
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortPizzaArbiterIfConfigure, portNum = %d\n", portNum);
        }
#ifndef GM_USED
        rc = prvCpssPxPortResourcesConfig(devNum, portNum, ifMode, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortResourcesConfig, portNum = %d\n", portNum);
        }
#endif
        /* Set  interface width */
        rc = prvCpssPxPortBusWidthSet(devNum, portNum, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortBusWidthSet, portNum = %d\n", portNum);
        }

        rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortPhysicalPortMapShadowDBGet, portNum = %d\n", portNum);
        }

        if(fullConfig)
        {
            if((CPSS_PORT_INTERFACE_MODE_QSGMII_E == ifMode) || (CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E == ifMode))
            {/* provide to HWS first port in quadruplet and it will configure
                all other ports if needed */
                portMacMap &= 0xFFFFFFFC;
            }

            refClockSource = PRIMARY_LINE_SRC;
            if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.enableOverride == GT_TRUE)
            {
                switch(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.portRefClockSource)
                {
                    case CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E:
                        refClockSource = PRIMARY_LINE_SRC;
                        break;
                    case CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E:
                        refClockSource = SECONDARY_LINE_SRC;
                        break;
                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }
            }

            rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, &curPortParams);
            if (GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in hwsPortModeParamsGetToBuffer, portMacMap = %d\n", portMacMap);
            }

            sdVecSize   = curPortParams.numOfActLanes;
            sdVectorPtr = curPortParams.activeLanesList;

            /* CPLL ref clock update */
            rc = prvCpssPxPortRefClockUpdate(devNum, portMode, sdVectorPtr, sdVecSize, refClockSource, &refClock);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortRefClockUpdate, portNum = %d\n", portNum);
            }

            CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d])", devNum, 0, portMacMap, portMode, lbPort, refClock, PRIMARY_LINE_SRC);

            cpssOsMemSet(&portInitInParam, 0,sizeof(portInitInParam));
            portInitInParam.lbPort = lbPort;
            portInitInParam.refClock = refClock;
            portInitInParam.refClockSource = refClockSource;
            rc = mvHwsPortFlavorInit(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, &portInitInParam);
            if (rc != GT_OK)
            {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"mvHwsPortFlavorInit: error for portNum = %d\n", portNum);
            }
            CPSS_LOG_INFORMATION_MAC("Exit: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d])", devNum, 0, portMacMap, portMode, lbPort, refClock, PRIMARY_LINE_SRC);

            if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
                /* set serdes Tx/Rx Tuning values if SW DB values initialized */
                rc = prvCpssPxPortSerdesTuning(devNum, 0, portNum, portMode);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortSerdesTuning, portNum = %d\n", portNum);
                }

                /* set the Polarity values on Serdeses if SW DB values initialized */
                rc = prvCpssPxPortSerdesPolaritySet(devNum, 0, sdVectorPtr, sdVecSize);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortSerdesPolaritySet, portNum = %d\n", portNum);
                }

                /* Enable the Tx signal, the signal was disabled during Serdes init */
                rc = mvHwsPortTxEnable(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, GT_TRUE);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in mvHwsPortTxEnable, portNum = %d\n", portNum);
                }
                CPSS_LOG_INFORMATION_MAC("Exit: mvHwsPortTxEnable(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], enable[%d]})", devNum, 0, portMacMap, portMode, GT_TRUE);
            }
        }

        prvCpssPxPortTypeSet(devNum, portNum, ifMode, speed);

        /* update addresses of mac registers accordingly to used MAC GE/XLG/etc. */
        rc = prvCpssPxHwRegAddrPortMacUpdate(devNum, portNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxHwRegAddrPortMacUpdate, portNum = %d\n", portNum);
        }

        if((CPSS_PORT_SPEED_10_E == speed) || (CPSS_PORT_SPEED_100_E == speed))
        {/* HWS doesn't support 10/100M, so after 1G configured by HWS, fix speed here */
            rc = prvCpssPxGeMacUnitSpeedSet(devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxGeMacUnitSpeedSet, portNum = %d\n", portNum);
            }
        }

        /* RM-9725147 implementation */
        if((speed == CPSS_PORT_SPEED_1000_E) ||
           (speed == CPSS_PORT_SPEED_2500_E))
        {
            regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacMap].gePortFIFOConfigReg1;
            txFifoMin = 130;

            rc = prvCpssHwPpSetRegField(devNum, regAddr, 6, 8, txFifoMin);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            /* Watermarks configuration */
            regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portMacMap].gePortFIFOConfigReg0;
            txFifoMin = (txFifoMin + 4) | ((txFifoMin + 2) << 8);
            rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, txFifoMin);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }

#ifndef GM_USED
        /* Configures type of the port connected to the LED */
        rc = prvCpssPxLedPortTypeConfig(devNum, portNum, powerUp);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxLedPortTypeConfig, portNum = %d\n", portNum);
        }
#endif

        /* save new interface mode in DB */
        *portIfModePtr = ifMode;
        *portSpeedPtr = speed;

        /*Creating Port LoopBack*/
        if(lbPort)
        {
            rc = prvCpssPxPortSerdesLoopbackActivate (devNum, portNum, speed);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortSerdesLoopbackActivate, portNum = %d\n", portNum);
            }
        }

        if (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
        {
            rc = prvCpssPxPortStateRestore(devNum, portNum, &portStateStc);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortStateRestore, portNum = %d\n", portNum);
            }
        }

    } /* for(portNum = 0; */

    return GT_OK;

}

/**
* @internal cpssPxPortModeSpeedSet function
* @endinternal
*
* @brief   Configure Interface mode and speed on a specified port and execute
*         on port's serdeses power up sequence; or configure power down on port's
*         serdeses.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portsBmpPtr              - (pointer to) physical ports bitmap (or CPU port)
* @param[in] powerUp                  - serdes power:
*                                      GT_TRUE - up;
*                                      GT_FALSE - down;
* @param[in] ifMode                   - interface mode (related only for serdes power up [powerUp==GT_TRUE])
* @param[in] speed                    - port data  (related only for serdes power up [powerUp==GT_TRUE])
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported interface for given port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - if port was not initialized
*                                     (does not exist in port
*                                     DB)
*
* @note The API rolls back a port's mode and speed to their last values
*       if they cannot be set together on the device.
*       Pay attention! Before configure CPSS_PORT_INTERFACE_MODE_NO_SERDES_PORT_E
*       MUST execute power down for port with this interface.
*
*/
GT_STATUS cpssPxPortModeSpeedSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORTS_BMP_STC              *portsBmpPtr,
    IN  GT_BOOL                         powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortModeSpeedSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portsBmpPtr, powerUp, ifMode, speed));

    rc = internal_cpssPxPortModeSpeedSet(devNum, portsBmpPtr, powerUp, ifMode, speed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portsBmpPtr, powerUp, ifMode, speed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortModeSpeedSetWrapper function
* @endinternal
*
* @brief   The Wrapper is using to set fec and other pre
*          configure before using portmodespeedset in some
*          devices that can use pre configuration
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portsBmp                 - physical port number
* @param[in] powerUp                  - power up/down bool
* @param[in] ifMode                   - interface mode
* @param[in] speed                    - interface speed
* @param[in] portMgrDbPtr             - PM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - fír not supported actions
* @retval GT_BAD_STATE             - nan speed/mode
*
*/
GT_STATUS prvCpssPxPortModeSpeedSetWrapper
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PORTS_BMP_STC                  portsBmp,
    IN  GT_BOOL                             powerUp,
    IN  CPSS_PORT_INTERFACE_MODE_ENT        ifMode,
    IN  CPSS_PORT_SPEED_ENT                 speed,
    IN  PRV_CPSS_PORT_MNG_PORT_SM_DB_STC    *portMgrDbPtr
)
{
    MV_HWS_PORT_FEC_MODE    fecMode;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32                  portMacNum;
    MV_HWS_PORT_STANDARD    portMode;
    GT_STATUS               rc;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    CPSS_PORT_INTERFACE_MODE_ENT    ifModeTmp = ifMode;
    CPSS_PORT_SPEED_ENT             speedTmp = speed;
    CPSS_PORT_FEC_MODE_ENT  fecAbility;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portMgrDbPtr);

    /* Set CPSS_PORT_FEC_MODE_LAST_E in case of handling power down or AP port, otherwise from port database */
    fecAbility = ((!powerUp) || ((speed > CPSS_PORT_SPEED_NA_E) && (ifMode > CPSS_PORT_INTERFACE_MODE_NA_E))) ? (CPSS_PORT_FEC_MODE_LAST_E) : (portMgrDbPtr->fecAbility);

    if (fecAbility != CPSS_PORT_FEC_MODE_LAST_E)
    {
        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            if (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp, portNum))
            {
                switch (fecAbility) {
                    case CPSS_PORT_FEC_MODE_ENABLED_E:
                        fecMode = FC_FEC;
                        break;

                    case CPSS_PORT_FEC_MODE_DISABLED_E:
                        fecMode = FEC_OFF;
                        break;

                    case CPSS_PORT_RS_FEC_MODE_ENABLED_E:
                        fecMode = RS_FEC;
                        break;

                    case CPSS_PORT_RS_FEC_544_514_MODE_ENABLED_E:
                        fecMode = RS_FEC_544_514;
                        break;

                    default:
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }

                PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

                if ((CPSS_PORT_INTERFACE_MODE_NA_E == ifMode) || (CPSS_PORT_SPEED_NA_E == speed))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
                }

                if (ifMode > CPSS_PORT_INTERFACE_MODE_NA_E && speed > CPSS_PORT_SPEED_NA_E)
                {
                    ifModeTmp -= CPSS_PORT_INTERFACE_MODE_NA_E;
                    speedTmp -= CPSS_PORT_SPEED_NA_E;
                }


                rc = prvCpssCommonPortIfModeToHwsTranslate(devNum, ifModeTmp, speedTmp, &portMode);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (GT_FALSE == hwsIsFecModeSupported(devNum, 0, portMacNum, portMode, fecMode))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                }


                if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portMacNum, NON_SUP_MODE, &curPortParams))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
                }
                if (curPortParams.portFecMode == fecMode)
                {
                    break;
                }

                /* update HWS elements DB */
                rc = hwsPortModeParamsSetFec(devNum, 0, portMacNum, portMode, fecMode);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
        }
    }
    return cpssPxPortModeSpeedSet(CAST_SW_DEVNUM(devNum), &portsBmp, powerUp, ifMode, speed);
}

/**
* @internal prvCpssPxPortHwResetWrapper function
* @endinternal
*
* @brief   HW port reset. This API reset the HW port compenent Mac, PCS and SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] qsgmiiFullDelete         - qsgmii full config boolean
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently, API not supported for QSGMII ports.
*
*/
GT_STATUS prvCpssPxPortHwResetWrapper
(
    IN  GT_SW_DEV_NUM                  devNum,
    IN  GT_PHYSICAL_PORT_NUM           portNum,
    GT_BOOL                            qsgmiiFullDelete
)
{
    GT_STATUS   rc;
    GT_U32      portMacMap;
    CPSS_PORT_INTERFACE_MODE_ENT ifMode;
    CPSS_PORT_SPEED_ENT speed;

    CPSS_LOG_INFORMATION_MAC("inside prvCpssPxPortHwResetWrapper with portNum=%d", portNum);

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portNum);
    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portNum);

    if (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PortHwReset API not supported for QSGMII port interface mode" );
    }


    rc =  prvCpssPxPortReset(devNum,portNum,ifMode,speed,qsgmiiFullDelete);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"calling prvCpssPxPortReset on DISABLE on port %d"
                                  " returned error code %d ",
                                  portNum, rc);
    }

    return rc;
}

/**
* @internal prvCpssPxPortHwUnresetWrapper function
* @endinternal
*
* @brief   HW port unreset. This API unreset the HW port compenent Mac, PCS and SerDes.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None;
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, physicalPortNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently, API not supported for QSGMII ports.
*       Currently, API not supported for Serdes Loopback state restore.
*
*/
GT_STATUS prvCpssPxPortHwUnresetWrapper
(
    IN GT_SW_DEV_NUM        devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    MV_HWS_REF_CLOCK_SUP_VAL    refClock;
    MV_HWS_REF_CLOCK_SOURCE     refClockSource;
    GT_U32      portMacMap;
    GT_STATUS   rc;
    GT_U8  sdVecSize;
    GT_U16  *sdVectorPtr;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    MV_HWS_PORT_STANDARD            portMode;
    MV_HWS_PORT_INIT_INPUT_PARAMS   portInitInParam;

    CPSS_LOG_INFORMATION_MAC("inside prvCpssPxPortHwUnresetWrapper with portNum=%d. ", portNum);

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacMap);

    /* get current ifMode and speed from CPSS db - they were not deleted in HwReset */
    speed = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portNum);
    ifMode = PRV_CPSS_PX_PORT_IFMODE_MAC(devNum, portNum);

    if (ifMode == CPSS_PORT_INTERFACE_MODE_QSGMII_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PortHwUnreset API not supported for QSGMII port interface mode" );
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(
        CAST_SW_DEVNUM(devNum), ifMode, speed, &portMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "PortHwUnreset API - "
                                      " prvCpssCommonPortIfModeToHwsTranslate returned error=%d", rc );
    }

    CPSS_LOG_INFORMATION_MAC("inside prvCpssPxPortHwUnreset with portNum=%d. Device is sip5_15 ", portNum);

    if (GT_OK != hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "hwsPortModeParamsGet returned null " );
    }

    sdVecSize = curPortParams.numOfActLanes;
    sdVectorPtr = curPortParams.activeLanesList;
    /* secondary means external ref clock */
    refClockSource = PRIMARY_LINE_SRC;

    rc = prvCpssPxSerdesRefClockTranslateCpss2Hws(devNum, &refClock);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.enableOverride == GT_TRUE)
    {
        switch(PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacMap].portRefClock.portRefClockSource)
        {
            case CPSS_PORT_REF_CLOCK_SOURCE_PRIMARY_E:
                refClockSource = PRIMARY_LINE_SRC;
                break;
            case CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E:
                refClockSource = SECONDARY_LINE_SRC;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    rc = prvCpssPxPortRefClockUpdate(devNum, portMode, sdVectorPtr, sdVecSize, refClockSource, &refClock);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "FATAL handling clock #2 on portNum %d returned error code %d ",portNum,rc);
    }
    cpssOsMemSet(&portInitInParam, 0,sizeof(portInitInParam));
    portInitInParam.lbPort = GT_FALSE;
    portInitInParam.refClock = refClock;
    portInitInParam.refClockSource = refClockSource;

    rc = mvHwsPortFlavorInit(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, &portInitInParam);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "FATAL handling mvHwsPortFlavorInit on portNum %d returned error code %d ",
                                                   portNum,rc );
    }

    CPSS_LOG_INFORMATION_MAC("Exit: mvHwsPortFlavorInit(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], lbPort[%d], refClock[%d], refClockSource[%d])", devNum, 0, portMacMap, portMode, 0/*lbPort*/, refClock, PRIMARY_LINE_SRC);

    /* set serdes Tx/Rx Tuning values if SW DB values initialized */
    rc = prvCpssPxPortSerdesTuning(devNum, 0, portNum, portMode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortSerdesTuning, portNum = %d\n", portNum);
    }

    /* set the Polarity values on Serdeses if SW DB values initialized */
    rc = prvCpssPxPortSerdesPolaritySet(devNum, 0, sdVectorPtr, sdVecSize);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortSerdesPolaritySet, portNum = %d\n", portNum);
    }

    /* Enable the Tx signal, the signal was disabled during Serdes init */
    rc = mvHwsPortTxEnable(CAST_SW_DEVNUM(devNum), 0, portMacMap, portMode, GT_TRUE);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in mvHwsPortTxEnable, portNum = %d\n", portNum);
    }
    CPSS_LOG_INFORMATION_MAC("Exit: mvHwsPortTxEnable(devNum[%d], portGroup[%d], phyPortNum[%d], portMode[%d], enable[%d]})", devNum, 0, portMacMap, portMode, GT_TRUE);

    if((CPSS_PORT_SPEED_10_E == speed) || (CPSS_PORT_SPEED_100_E == speed))
    {/* HWS doesn't support 10/100M, so after 1G configured by HWS, fix speed here */
        rc = prvCpssPxGeMacUnitSpeedSet(devNum, portNum, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxGeMacUnitSpeedSet, portNum = %d\n", portNum);
        }
    }

#ifndef GM_USED
    /* Configures type of the port connected to the LED */
    rc = prvCpssPxLedPortTypeConfig(devNum, portNum,GT_TRUE/*powerUp*/);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxLedPortTypeConfig, portNum = %d\n", portNum);
    }
#endif

    /*Creating Port LoopBack*/
    if(0/*lbPort*/)
    {
        rc = prvCpssPxPortSerdesLoopbackActivate (devNum, portNum, speed);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"error in prvCpssPxPortSerdesLoopbackActivate, portNum = %d\n", portNum);
        }
    }

    return GT_OK;
}


GT_STATUS prvCpssPxPortIfFunctionsObjInit
(
    IN GT_SW_DEV_NUM devNum
)
{

    /* common functions bind */
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortModeSpeedSetFunc = prvCpssPxPortModeSpeedSetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortHwUnresetFunc = prvCpssPxPortHwUnresetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortHwResetFunc = prvCpssPxPortHwResetWrapper;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppFecModeSetFunc = cpssPxPortFecModeSet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppFecModeGetFunc = cpssPxPortFecModeGet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppIfModeGetFromExtFunc = cpssPxPortInterfaceModeGet;
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[devNum]->ppCommonPortFuncPtrsStc).ppPortBwTxFifoSetFunc = NULL;

    return GT_OK;
}


