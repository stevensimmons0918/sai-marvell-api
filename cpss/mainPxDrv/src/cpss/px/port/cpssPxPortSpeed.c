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
* @file cpssPxPortSpeed.c
*
* @brief CPSS implementation for Port speed configuration.
*
* The following APIs can run now "preliminary stage" and "post stage"
* callbacks, if the bind with port MAC object pointer engaged:
* - cpssPxPortSpeedSet;
* - cpssPxPortSpeedGet;
*
* @version   62
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
/*#define CPSS_LOG_IN_MODULE_ENABLE*/
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/PizzaArbiter/cpssPxPortPizzaArbiter.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpssDriver/pp/interrupts/generic/prvCpssDrvInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsLion2.h>

#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/px/port/cpssPxPortAp.h>

/**
* @internal prvCpssPxPortSpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*
* @note 1. If the port is enabled then the function disables the port before
*       the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*       configured before port's speed, see cpssPxPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*
*/
GT_STATUS prvCpssPxPortSpeedSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_STATUS            rc = GT_OK;            /* return code */
    GT_U32               portMacNum;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }
    if((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        && (PRV_CPSS_PX_PP_MAC(devNum)->genInfo.hitlessWriteMethodEnable))
    {/* do only if we during recovery */
        CPSS_PORT_SPEED_ENT   currentSpeed;

        rc = prvCpssPxPortSpeedGet(devNum, portNum, &currentSpeed);
        if(rc != GT_OK)
        {
            return rc;
        }
        if(speed == currentSpeed)
        {
            return GT_OK;
        }
    }

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,portMacNum);


    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum) != PRV_CPSS_PORT_GE_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* disable port if we need */
    rc = prvCpssPxGeMacUnitSpeedSet(devNum, portNum, speed);
    if(rc != GT_OK)
    {
        return rc;
    }


    return rc;
}

/**
* @internal internal_cpssPxPortSpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*
* @note 1. If the port is enabled then the function disables the port before
*       the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*       configured before port's speed, see cpssPxPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*
*/
static GT_STATUS internal_cpssPxPortSpeedSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_PHYSICAL_PORT_NUM portMacNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,portMacNum);

    if(PRV_CPSS_PORT_NOT_EXISTS_E == PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* check input parameter */
    if (speed >= CPSS_PORT_SPEED_NA_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssPxPortSpeedSet(devNum, portNum, speed);
}

/**
* @internal cpssPxPortSpeedSet function
* @endinternal
*
* @brief   Sets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number or CPU port
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or speed
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_SUPPORTED         - on not supported speed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - Speed of the port group member is different
*                                       from speed for setting.
*
* @note 1. If the port is enabled then the function disables the port before
*       the operation and re-enables it at the end.
*       2. For Flex-Link ports the interface mode should be
*       configured before port's speed, see cpssPxPortInterfaceModeSet.
*       3.This API also checks all SERDES per port initialization. If serdes was
*       not initialized, proper init will be done
*
*/
GT_STATUS cpssPxPortSpeedSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSpeedSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, speed));

    rc = internal_cpssPxPortSpeedSet(devNum, portNum, speed);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, speed));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortGePortTypeSet function
* @endinternal
*
* @brief   Set port type and inband auto-neg. mode of GE MAC of port
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] ifMode                   - port interface mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device, ifMode
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPortGePortTypeSet
(
    IN GT_SW_DEV_NUM                devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN CPSS_PORT_INTERFACE_MODE_ENT ifMode
)
{
    GT_STATUS   rc;     /* return code */
    GT_U32      value;  /* value to set in register */
    GT_U32      regAddr; /* register address */
    GT_U32      portGroupId;    /* port group ID - for multi-port-group devices */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PORT_MAC_CHECK_MAC(devNum,portNum);

    /* Set interface mode */
    switch(ifMode)
    {
        case CPSS_PORT_INTERFACE_MODE_QSGMII_E: /*No break*/
            /* coverity[fallthrough] */
        case CPSS_PORT_INTERFACE_MODE_SGMII_E:
            value = 0;
            break;
        case CPSS_PORT_INTERFACE_MODE_1000BASE_X_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_PX_PORT_MAC_CTRL_REG_MAC(devNum, portNum, PRV_CPSS_PORT_GE_E,
                                        &regAddr);
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(
                                                                CAST_SW_DEVNUM(devNum), portNum);
    rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, 1, 1,
                                            value);
    if(rc != GT_OK)
    {
        return rc;
    }
    return rc;
}

/**
* @internal prvCpssPxGeMacUnitSpeedSet function
* @endinternal
*
* @brief   Configure GE MAC unit of specific device and port to required speed
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] speed                    - port speed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong speed
*/
GT_STATUS prvCpssPxGeMacUnitSpeedSet
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speed
)
{
    GT_STATUS rc;               /* return code */
    PRV_CPSS_PX_PORT_STATE_STC    portStateStc;        /* current port state */
    GT_U32    gmiiOffset;       /* gmii speed bit offset */
    GT_U32    miiOffset;        /* mii speed bit offset */
    GT_U32    portSpeedRegAddr; /* address of GE port speed register */
    GT_U32    portGroupId;/*the port group Id - support multi-port-groups device */

    GT_U32 portMacNum;      /* MAC number */

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* convert the 'Physical port' to portGroupId,local port -- supporting multi-port-groups device */
    portGroupId = PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portNum);

    /* disable port if we need */
    rc = prvCpssPxPortStateDisableAndGet(devNum,portNum,&portStateStc);
    if(rc != GT_OK)
        return rc;

    PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum,portMacNum,&portSpeedRegAddr);

    gmiiOffset = 6;
    miiOffset  = 5;

    switch (speed)
    {
        case CPSS_PORT_SPEED_10_E:
            if(prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId, portSpeedRegAddr, gmiiOffset, 1, 0) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId, portSpeedRegAddr, miiOffset, 1, 0);
            if(rc != GT_OK)
                return rc;
            break;

        case CPSS_PORT_SPEED_100_E:
            if (prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId, portSpeedRegAddr, gmiiOffset, 1, 0) != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
            }
            rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId, portSpeedRegAddr, miiOffset, 1, 1);
            if(rc != GT_OK)
                return rc;
            break;
        case CPSS_PORT_SPEED_1000_E:
        case CPSS_PORT_SPEED_2500_E:
            if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_GE_E)
            {
                rc = prvCpssDrvHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), portGroupId, portSpeedRegAddr, gmiiOffset, 1, 1);
                if(rc != GT_OK)
                    return rc;

                if((CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == PRV_CPSS_PX_PORT_IFMODE_MAC(devNum,portMacNum) ||
                   (CPSS_PORT_SPEED_2500_E == speed)))
                {
                    GT_BOOL linkDownStatus;

                    rc = prvCpssPxPortForceLinkDownEnable(devNum, portNum, &linkDownStatus);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* configure 1000BaseX Port type, although the
                        interface mode called SGMII from historic reasons */
                    rc = prvCpssPxPortGePortTypeSet(devNum, portNum,
                                                    CPSS_PORT_INTERFACE_MODE_1000BASE_X_E);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = prvCpssPxPortInbandAutonegMode(devNum,portNum, CPSS_PORT_INTERFACE_MODE_1000BASE_X_E);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    /* usually it's application's decision enable/disable inband auto-neg.,
                        but to make 1000BaseX and 2.5G establish link it must be enabled */
                    rc = cpssPxPortInbandAutoNegEnableSet(devNum,portNum,GT_TRUE);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = prvCpssPxPortForceLinkDownDisable(devNum,portNum, linkDownStatus);
                    if(rc != GT_OK)
                    {
                        return rc;
                    }
                }

                break;
            }
            /* else
                continue toward failure */
            GT_ATTR_FALLTHROUGH;
        default:
            /* enable port if we need */
            rc = prvCpssPxPortStateRestore(devNum,portNum,&portStateStc);
            if(rc != GT_OK)
                return rc;
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG); /* GT_BAD_PARAM; */
    }

    /* enable port if we need */
    rc = prvCpssPxPortStateRestore(devNum,portNum,&portStateStc);

    return rc;
}

/**
* @internal prvCpssPxPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*
*/
GT_STATUS prvCpssPxPortSpeedGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_STATUS rc;   /* return code */


    CPSS_PORT_INTERFACE_MODE_ENT ifMode;    /* interface mode */
    GT_U32      regValue;   /* register value */
    GT_U32      portGroupId;    /* port group Id for multi-port-group support */
    GT_U32      regAddr;    /* register address */
    GT_U32      portMacNum; /* MAC number connected to port */
    GT_BOOL     apEnable;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(speedPtr);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,portMacNum);

    rc = cpssPxPortInterfaceModeGet(devNum,portNum,&ifMode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* NA interface */
    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_E)
    {
        *speedPtr = CPSS_PORT_SPEED_NA_E;
        return GT_OK;
    }
    /* NA interface */
    if (ifMode == CPSS_PORT_INTERFACE_MODE_NA_HCD_E)
    {
        *speedPtr = CPSS_PORT_SPEED_NA_HCD_E;
        return GT_OK;
    }

    *speedPtr = PRV_CPSS_PX_PORT_SPEED_MAC(devNum, portMacNum);

    if(*speedPtr <= CPSS_PORT_SPEED_1000_E)
    {
        rc = cpssPxPortApPortEnableGet(devNum,portNum,&apEnable);
        if (rc != GT_OK)
        {
           apEnable = GT_FALSE;
        }
         if (!apEnable)
         {
            /* for all types of ASIC's for FE/GE ports get speed from HW for case
            auto-negotiation is enabled */
            portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacNum);
            PRV_CPSS_PX_PORT_STATUS_CTRL_REG_MAC(devNum, portMacNum, &regAddr);
            rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, 1,
                                                    2, &regValue);
            if (rc != GT_OK)
            {
                return rc;
            }

            if ((regValue & 0x1) == 0)
            {
                *speedPtr = ((regValue >> 1) == 1) ? CPSS_PORT_SPEED_100_E:
                    CPSS_PORT_SPEED_10_E;
            }
            else
            {
                *speedPtr = CPSS_PORT_SPEED_1000_E;
            }
         }
    }

    return rc;
}

/**
* @internal internal_cpssPxPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*
*/
static GT_STATUS internal_cpssPxPortSpeedGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_STATUS rc = GT_OK;               /* return code */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(speedPtr);

    if(PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_NOT_EXISTS_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);


        rc = prvCpssPxPortSpeedGet(devNum,portNum,speedPtr);
        if(rc!=GT_OK)
        {
            return rc;
        }

    return rc;
}

/**
* @internal cpssPxPortSpeedGet function
* @endinternal
*
* @brief   Gets speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number (or CPU port)
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_INITIALIZED       - on no initialized SERDES per port
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.This API also checks if at least one serdes per port was initialized.
*       In case there was no initialized SERDES per port GT_NOT_INITIALIZED is
*       returned.
*
*/
GT_STATUS cpssPxPortSpeedGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSpeedGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, speedPtr));

    rc = internal_cpssPxPortSpeedGet(devNum, portNum, speedPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, speedPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortSpeedHwGet function
* @endinternal
*
* @brief   Gets from HW speed for specified port on specified device.
*
* @note   APPLICABLE DEVICES:      pipe.
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] speedPtr                 - pointer to actual port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - speed not appropriate for interface mode
*/
GT_STATUS prvCpssPxPortSpeedHwGet
(
    IN  GT_SW_DEV_NUM         devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_PORT_SPEED_ENT   *speedPtr
)
{
    GT_STATUS               rc; /* return code */
    GT_U32                  portGroupId;    /* port group Id for multi-port-group support */
    GT_U32                  portMacMap; /* number of mac mapped to this physical port */
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    MV_HWS_PORT_STANDARD    portMode;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum,portMacMap);
    CPSS_NULL_PTR_CHECK_MAC(speedPtr);

    *speedPtr = CPSS_PORT_SPEED_NA_E;
    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E == portMapShadowPtr->portMap.mappingType)
    {/* return dummy speed to support legacy behavior */
        *speedPtr = CPSS_PORT_SPEED_1000_E;
        return GT_OK;
    }
    portGroupId = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), portMacMap);

    if(PRV_CPSS_GE_PORT_GE_ONLY_E == PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum,portMacMap))
    {
        CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortInterfaceGet(devNum[%d], portGroup[%d], phyPortNum[%d], *portModePtr)", devNum, portGroupId, portMacMap&0xFFFFFFFC);
        rc = mvHwsPortInterfaceGet(CAST_SW_DEVNUM(devNum), portGroupId, portMacMap&0xFFFFFFFC, &portMode);
        if(rc != GT_OK)
        {
            CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
            return rc;
        }
        if (QSGMII == portMode)
        {
            goto speedGet;
        }
    }

    CPSS_LOG_INFORMATION_MAC("Calling: mvHwsPortInterfaceGet(devNum[%d], portGroup[%d], phyPortNum[%d], *portModePtr)", devNum, portGroupId, portMacMap);
    rc = mvHwsPortInterfaceGet(CAST_SW_DEVNUM(devNum), portGroupId, portMacMap, &portMode);
    if(rc != GT_OK)
    {
        CPSS_LOG_INFORMATION_MAC("Hws return code is %d", rc);
        return rc;
    }

speedGet:
    switch(portMode)
    {
        case _100Base_FX:
            *speedPtr = CPSS_PORT_SPEED_100_E;
            break;

        case SGMII:
        case QSGMII:
            {/* HWS doesn't support 10/100M configuration */
                GT_U32 regAddr;
                GT_U32 value;

                /* read configured speed - not status */
                PRV_CPSS_PX_PORT_AUTO_NEG_CTRL_REG_MAC(devNum, portMacMap, &regAddr);
                rc = prvCpssDrvHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum), portGroupId, regAddr, 5,
                                                                            2, &value);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if ((value & 0x2) == 0)
                {
                    *speedPtr = ((value & 0x1) == 1) ? CPSS_PORT_SPEED_100_E:
                        CPSS_PORT_SPEED_10_E;
                }
                else
                {
                    *speedPtr = CPSS_PORT_SPEED_1000_E;
                }
            }

            break;

        case SGMII2_5:
            *speedPtr = CPSS_PORT_SPEED_2500_E;
            break;

        case _1000Base_X:
            *speedPtr = CPSS_PORT_SPEED_1000_E;
            break;

        case _5GBaseR:
           *speedPtr = CPSS_PORT_SPEED_5000_E;
            break;

        case _10GBase_KX4:
        case _10GBase_KX2:
        case _10GBase_KR:
        case RXAUI:
        case _10GBase_SR_LR:
        case INTLKN_4Lanes_3_125G:
            *speedPtr = CPSS_PORT_SPEED_10000_E;
            break;

        case _20GBase_KX4:
        case _20GBase_KR2:
        case _20GBase_SR_LR2:
            *speedPtr = CPSS_PORT_SPEED_20000_E;
            break;

        case _40GBase_KR4:
        case _40GBase_KR2:
        case _40GBase_CR4:
        case XLHGL_KR4:
        case _40GBase_SR_LR4:
        case INTLKN_8Lanes_6_25G:
            *speedPtr = CPSS_PORT_SPEED_40000_E;
            break;

        case _100GBase_KR10:
        case _100GBase_SR10:
        case CHGL:
            *speedPtr = CPSS_PORT_SPEED_100G_E;
            break;

        case _12_1GBase_KR:
            *speedPtr = CPSS_PORT_SPEED_12000_E;
            break;

        case _12_5GBase_KR:
            *speedPtr = CPSS_PORT_SPEED_12500_E;
            break;

        case _24GBase_KR2:
            *speedPtr = CPSS_PORT_SPEED_23600_E;
            break;

        case _25GBase_KR2:
            *speedPtr = CPSS_PORT_SPEED_25000_E;
            break;

        case _50GBase_KR4:
            *speedPtr = CPSS_PORT_SPEED_50000_E;
            break;

        case _25GBase_KR:
        case _25GBase_KR_C:
        case _25GBase_CR:
        case _25GBase_CR_C:
        case _25GBase_KR_S:
        case _25GBase_CR_S:
            *speedPtr = CPSS_PORT_SPEED_25000_E;
            break;

        case _26_7GBase_KR:
            *speedPtr =  CPSS_PORT_SPEED_26700_E;
            break;

        case _50GBase_KR2:
        case _50GBase_KR2_C:
        case _50GBase_CR2:
            *speedPtr = CPSS_PORT_SPEED_50000_E;
            break;

        case _52_5GBase_KR2:
            *speedPtr = CPSS_PORT_SPEED_52500_E;
            break;

        case _100GBase_KR4:
        case _100GBase_CR4:
            *speedPtr = CPSS_PORT_SPEED_100G_E;
            break;

        case _102GBase_KR4:
            *speedPtr = CPSS_PORT_SPEED_102G_E;
            break;

        case _107GBase_KR4:
            *speedPtr = CPSS_PORT_SPEED_107G_E;
            break;

        case _25GBase_SR:
            *speedPtr = CPSS_PORT_SPEED_25000_E;
            break;

        case _50GBase_SR2:
            *speedPtr = CPSS_PORT_SPEED_50000_E;
            break;

        case _100GBase_SR4:
            *speedPtr = CPSS_PORT_SPEED_100G_E;
            break;

        case _100GBase_MLG:
            if ((portMacMap % 2) == 0) /* portMacNum is even */
            {
                *speedPtr = CPSS_PORT_SPEED_40000_E;
            }
            else if ((portMacMap % 2) == 1) /* portMacNum is odd */
            {
                *speedPtr = CPSS_PORT_SPEED_10000_E;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
            }
            break;

        case HGL:
            *speedPtr = CPSS_PORT_SPEED_15000_E;
            break;

        case HGL16G:
            *speedPtr = CPSS_PORT_SPEED_16000_E;
            break;

        case _12GBase_SR:
            *speedPtr = CPSS_PORT_SPEED_11800_E;
            break;

        case _48GBase_SR4:
            *speedPtr = CPSS_PORT_SPEED_47200_E;
            break;

        case _22GBase_SR:
            *speedPtr = CPSS_PORT_SPEED_22000_E;
            break;

        default:
            *speedPtr = CPSS_PORT_SPEED_NA_E;
            break;
    }

    return GT_OK;
}

