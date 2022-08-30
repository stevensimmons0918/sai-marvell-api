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
* @file cpssPxPortSyncEther.c
*
* @brief CPSS implementation for Sync-E in Pipe devices (Synchronous Ethernet)
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortSyncEther.h>
#include <cpss/px/port/private/prvCpssPxPort.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>

/**
* @internal prvCpssPxSyncEClockOutputSerdesChainSet function
* @endinternal
*
* @brief   Selects the SyncE output RCVR_CLOCK_OUT[0] or RCVR_CLOCK_OUT[1],
*         between the 25G and the 10G SERDES chains.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
* @param[in] portNum                  - physical port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
*/
static GT_STATUS prvCpssPxSyncEClockOutputSerdesChainSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  regOffset;      /* register offset */
    GT_U32  regValue;       /* register value */
    GT_U32  portMacNum;     /* MAC number */

    GT_STATUS rc;           /* return status */

    switch (recoveryClkType)
    {
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
            regOffset = 17;
            break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
            regOffset = 18;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* Select SERDES chain according to port MAC number.
       Ports MACs 0..11 belong to 10G SREDES chain, MACs 12..15 are 25G SREDES chain */
    regValue = (portMacNum < 12) ? 0 : 1;

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
        DFXServerUnitsDeviceSpecificRegs.deviceCtrl20;

    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, regOffset, 1, regValue);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxPortSyncEtherRecoveryClkConfigSet function
* @endinternal
*
* @brief   Function configures the recovery clock enable/disable state and sets
*         its source portNum.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
* @param[in] enable                   - enable/disable recovered clock1/clock2:
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..3
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_STATE             - on bad state
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortSyncEtherRecoveryClkConfigSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL                 enable,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  regValue;       /* register value */
    GT_U32  regOffset;      /* register offset */
    GT_U32  serdes;         /* loop index */
    GT_U32  startSerdes;    /* start serdes index in port */
    GT_U32  serdesIdx;      /* current serdes index */
    GT_U32  maxSerdesNumber;/* maximal serdes number */
    GT_U32  maxLaneNum;     /* max serdes number per port */

    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */
    CPSS_PORT_SPEED_ENT speed;


    GT_STATUS rc;       /* returned status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    rc = cpssPxPortInterfaceModeGet(devNum, portNum, &ifMode);
    if( GT_OK != rc )
    {
        return rc;
    }

    rc = cpssPxPortSpeedGet(devNum, portNum, &speed);
    if( GT_OK != rc )
    {
        return rc;
    }

    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode, &startSerdes, &maxLaneNum);
    if( GT_OK != rc )
    {
        return rc;
    }

    if( laneNum >= maxLaneNum )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    serdesIdx = startSerdes + laneNum;

    /* Find the right offset for Clock0 select or Clock1 select */
    switch (recoveryClkType)
    {
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
            regOffset = 15;
            break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
            regOffset = 11;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if( GT_FALSE == enable )
    {
        /* SERDES External Configuration 1 Register */
        regAddr =
            PRV_PX_REG1_UNIT_SERDES_MAC(devNum, serdesIdx).serdesExternalReg2;

        /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),  regAddr, regOffset, 1, 1);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }
    else /* GT_TRUE == enable */
    {
        maxSerdesNumber = prvCpssPxHwInitNumOfSerdesGet(devNum);

        for(serdes = 0; serdes < maxSerdesNumber; serdes++)
        {
            if (serdes == serdesIdx)
            {
                continue;
            }

            /* SERDES External Configuration 1 Register */
            regAddr =
                PRV_PX_REG1_UNIT_SERDES_MAC(devNum, serdes).serdesExternalReg2;

            /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
            rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),  regAddr, regOffset, 1, &regValue);
            if( GT_OK != rc )
            {
                return rc;
            }

            if( 0 == regValue )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

        /* SERDES External Configuration 1 Register */
        regAddr =
            PRV_PX_REG1_UNIT_SERDES_MAC(devNum, serdesIdx).serdesExternalReg2;

        /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),  regAddr, regOffset, 1, 0);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    /* Set <rcvrd_clk0_sel> or <rcvrd_clk1_sel> SERDES chain */
    rc = prvCpssPxSyncEClockOutputSerdesChainSet(devNum, recoveryClkType, portNum);

    return rc;
}

/**
* @internal cpssPxPortSyncEtherRecoveryClkConfigSet function
* @endinternal
*
* @brief   Function configures the recovery clock enable/disable state and sets
*         its source portNum.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
* @param[in] enable                   - enable/disable recovered clock1/clock2:
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..3
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_STATE             - on bad state
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSyncEtherRecoveryClkConfigSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL                 enable,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  laneNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSyncEtherRecoveryClkConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, recoveryClkType, enable, portNum, laneNum));

    rc = internal_cpssPxPortSyncEtherRecoveryClkConfigSet(devNum, recoveryClkType, enable, portNum, laneNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, recoveryClkType, enable, portNum, laneNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSyncEtherRecoveryClkConfigGet function
* @endinternal
*
* @brief   Function gets the recovery clock enable/disable state and its source
*         portNum.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
*
* @param[out] enablePtr                - (pointer to) port state as reference.
*                                      GT_TRUE -  enabled
*                                      GT_FALSE - disabled
* @param[out] portNumPtr               - (pointer to) port number.
* @param[out] laneNumPtr               - (pointer to) selected SERDES lane number within port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on bad state of register
*/
static GT_STATUS internal_cpssPxPortSyncEtherRecoveryClkConfigGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *              enablePtr,
    OUT GT_PHYSICAL_PORT_NUM    *portNumPtr,
    OUT GT_U32                  *laneNumPtr
)
{
    GT_U32  regAddr;            /* register address */
    GT_U32  regValue;           /* register value */
    GT_U32  regOffset;          /* register offset */
    GT_STATUS rc;               /* returned status */
    GT_U32  serdes;             /* loop index */
    GT_U32  startSerdes;        /* start serdes index in port */
    GT_U32  maxLaneNum;         /* max serdes number per port */
    CPSS_PORT_INTERFACE_MODE_ENT  ifMode; /* interface mode */
    GT_PHYSICAL_PORT_NUM portNum; /* loop iterator */
    GT_BOOL isValid;            /* port mapping status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(laneNumPtr);

    /* Find the right offset for Clock0 select or Clock1 select */
    switch (recoveryClkType)
    {
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
            regOffset = 15;
            break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
            regOffset = 11;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    for (portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
    {
        rc = cpssPxPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
        if((rc != GT_OK) || (isValid != GT_TRUE))
        {
            continue;
        }

        rc = cpssPxPortInterfaceModeGet(devNum, portNum, &ifMode);
        if((ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) || ( GT_OK != rc ))
        {
            continue;
        }

        rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode, &startSerdes, &maxLaneNum);
        if( GT_OK != rc )
        {
            return rc;
        }

        for(serdes = startSerdes; serdes < (startSerdes + maxLaneNum); serdes++)
        {
            /* SERDES External Configuration 1 Register */
            regAddr =
                PRV_PX_REG1_UNIT_SERDES_MAC(devNum, serdes).serdesExternalReg2;

            /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
            rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),  regAddr, regOffset, 1, &regValue);
            if( GT_OK != rc )
            {
                return rc;
            }

            if( 0 == regValue )
            {
                *enablePtr = GT_TRUE;
                *laneNumPtr = serdes - startSerdes;
                *portNumPtr = portNum;

                return GT_OK;
            }
        }
    }

    *enablePtr = GT_FALSE;
    *laneNumPtr = 0;
    *portNumPtr = PRV_CPSS_PX_PORTS_NUM_CNS;

    return GT_OK;
}

/**
* @internal cpssPxPortSyncEtherRecoveryClkConfigGet function
* @endinternal
*
* @brief   Function gets the recovery clock enable/disable state and its source
*         portNum.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
*
* @param[out] enablePtr                - (pointer to) port state as reference.
*                                      GT_TRUE -  enabled
*                                      GT_FALSE - disabled
* @param[out] portNumPtr               - (pointer to) port number.
* @param[out] laneNumPtr               - (pointer to) selected SERDES lane number within port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on bad state of register
*/
GT_STATUS cpssPxPortSyncEtherRecoveryClkConfigGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *              enablePtr,
    OUT GT_PHYSICAL_PORT_NUM    *portNumPtr,
    OUT GT_U32                  *laneNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSyncEtherRecoveryClkConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, recoveryClkType, enablePtr, portNumPtr, laneNumPtr));

    rc = internal_cpssPxPortSyncEtherRecoveryClkConfigGet(devNum, recoveryClkType, enablePtr, portNumPtr, laneNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, recoveryClkType, enablePtr, portNumPtr, laneNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSyncEtherRecoveryClkDividerValueSet function
* @endinternal
*
* @brief   Function sets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..3
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
* @param[in] value                    - recovery clock divider  to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
static GT_STATUS internal_cpssPxPortSyncEtherRecoveryClkDividerValueSet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT   clockSelect,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT    value
)
{
    GT_STATUS   rc;             /* return code      */
    GT_U32      regAddr;        /* register address */
    GT_U32      regValue;       /* register data    */
    GT_U32      regOffset;      /* offset in register */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */
    GT_U32      startSerdes;    /* first serdes used by port */
    GT_U32      maxLaneNum;     /* number of SERDESes used by port now */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    rc = cpssPxPortInterfaceModeGet(devNum, portNum, &ifMode);
    if( GT_OK != rc )
    {
        return rc;
    }

    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                         &startSerdes, &maxLaneNum);
    if( GT_OK != rc )
    {
        return rc;
    }

    if( laneNum >= maxLaneNum )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* SERDES External Configuration 1 Register */
    regAddr =
        PRV_PX_REG1_UNIT_SERDES_MAC(devNum, (startSerdes + laneNum)).serdesExternalReg2;

    switch (clockSelect)
    {
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
            regOffset = 12;
            break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
            regOffset = 8;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(value)
    {
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E:    regValue = 0;
                                                                    break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_E:    regValue = 1;
                                                                    break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_3_E:    regValue = 2;
                                                                    break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_4_E:    regValue = 3;
                                                                    break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_5_E:    regValue = 4;
                                                                    break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E:    regValue = 5;
                                                                    break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E:   regValue = 6;
                                                                    break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E:  regValue = 7;
                                                                    break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    /* <rcvrd_clk0_div> or <rcvrd_clk1_div> */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, regOffset, 3, regValue);
}

/**
* @internal cpssPxPortSyncEtherRecoveryClkDividerValueSet function
* @endinternal
*
* @brief   Function sets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..3
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
* @param[in] value                    - recovery clock divider  to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortSyncEtherRecoveryClkDividerValueSet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT   clockSelect,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT    value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSyncEtherRecoveryClkDividerValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, clockSelect, value));

    rc = internal_cpssPxPortSyncEtherRecoveryClkDividerValueSet(devNum, portNum, laneNum, clockSelect, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, clockSelect, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortSyncEtherRecoveryClkDividerValueGet function
* @endinternal
*
* @brief   Function gets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..3
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*
* @param[out] valuePtr                 - (pointer to) recovery clock divider value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssPxPortSyncEtherRecoveryClkDividerValueGet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT   clockSelect,
    OUT CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT    *valuePtr
)
{
    GT_STATUS   rc;             /* return code      */
    GT_U32      regAddr;        /* register address */
    GT_U32      regValue;       /* register data    */
    GT_U32      regOffset;      /* offset in register */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */
    GT_U32      startSerdes;    /* first serdes used by port */
    GT_U32      maxLaneNum;     /* number of SERDESes used by port now */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);


    rc = cpssPxPortInterfaceModeGet(devNum, portNum, &ifMode);
    if( GT_OK != rc )
    {
        return rc;
    }

    rc = prvCpssPxPortIfModeSerdesNumGet(devNum, portNum, ifMode,
                                           &startSerdes, &maxLaneNum);
    if( GT_OK != rc )
    {
        return rc;
    }

    if( laneNum >= maxLaneNum )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* SERDES External Configuration 1 Register */
    regAddr =
        PRV_PX_REG1_UNIT_SERDES_MAC(devNum, (startSerdes + laneNum)).serdesExternalReg2;

    switch (clockSelect)
    {
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
            regOffset = 12;
            break;
        case CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
            regOffset = 8;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* <rcvrd_clk0_div> or <rcvrd_clk1_div> */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),  regAddr, regOffset, 3, &regValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    switch(regValue)
    {
        case 0: *valuePtr = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E;
                break;
        case 1: *valuePtr = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_E;
                break;
        case 2: *valuePtr = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_3_E;
                break;
        case 3: *valuePtr = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_4_E;
                break;
        case 4: *valuePtr = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_5_E;
                break;
        case 5: *valuePtr = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E;
                break;
        case 6: *valuePtr = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E;
                break;
        case 7: *valuePtr = CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E;
                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortSyncEtherRecoveryClkDividerValueGet function
* @endinternal
*
* @brief   Function gets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..3
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*
* @param[out] valuePtr                 - (pointer to) recovery clock divider value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
GT_STATUS cpssPxPortSyncEtherRecoveryClkDividerValueGet
(
    IN  GT_SW_DEV_NUM                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT   clockSelect,
    OUT CPSS_PX_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT    *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortSyncEtherRecoveryClkDividerValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, clockSelect, valuePtr));

    rc = internal_cpssPxPortSyncEtherRecoveryClkDividerValueGet(devNum, portNum, laneNum, clockSelect, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, clockSelect, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


