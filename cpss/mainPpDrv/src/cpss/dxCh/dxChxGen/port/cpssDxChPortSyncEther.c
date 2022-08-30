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
* @file cpssDxChPortSyncEther.c
*
* @brief CPSS implementation for Sync-E (Synchronous Ethernet)
*
* @version   36
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortSyncEther.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal prvCpssDxChPortSyncEtherPortLaneCheck function
* @endinternal
*
* @brief   Function check if the lane number is within the allowable range for
*         the port interface mode.
*
* @note   APPLICABLE DEVICES:      Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - SERDES lane number within port.
*                                      Allowable ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*
* @param[out] ifModePtr                - (pointer to) port interface mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum or lane number
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvCpssDxChPortSyncEtherPortLaneCheck
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          laneNum,
    OUT CPSS_PORT_INTERFACE_MODE_ENT    *ifModePtr
)
{
    GT_STATUS                       rc;     /* return code */
    GT_U32                          maxLaneNum; /* lanes number according to */
                                                /* port interface mode       */
    CPSS_PORT_SPEED_ENT             speed; /* current port speed */
    GT_U32                          startSerdes;

    rc = cpssDxChPortInterfaceModeGet(devNum, portNum, ifModePtr);
    if( GT_OK != rc )
    {
        return rc;
    }

    rc = cpssDxChPortSpeedGet(devNum, portNum, &speed);
    if( GT_OK != rc )
    {
        return rc;
    }

    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, *ifModePtr, &startSerdes, &maxLaneNum);
    if( GT_OK != rc )
    {
        return rc;
    }

    if( laneNum >= maxLaneNum )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSyncEtherRecoveryClkConfigSetForLion2 function
* @endinternal
*
* @brief   Function configures the recovery clock enable/disable state and sets
*         its source portNum.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
* @param[in] enable                   - enable/disable recovered clock1/clock2
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType, portNum
*                                       or laneNum.
* @retval GT_BAD_STATE             - another SERDES already enabled
* @retval GT_HW_ERROR              - on hardware error
*
* @note If SERDES enabling is requesed and another SERDES in the port group
*       is already enabled, GT_BAD_STATE is returned.
*
*/
static GT_STATUS prvCpssDxChPortSyncEtherRecoveryClkConfigSetForLion2
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL enable,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum
)
{
    GT_STATUS rc;           /* returned status */
    GT_U32  regAddr;        /* register address */
    GT_U32  regValue = 0;   /* register value */
    GT_U32  regOffset;      /* register offset */
    GT_U32  portGroup;      /* port group */
    GT_U32  serdes;         /* loop index */
    GT_U32  startSerdes;    /* start serdes index in port */
    GT_U32  serdesIdx;      /* current serdes index */
    GT_U32  maxSerdesNumber;/* maximal serdes number */
    GT_U32  maxLaneNum;     /* max serdes number per port */
    GT_U32  recoveryClkTypeU32  = recoveryClkType; /* auxiliary variable to avoid casting */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */

    /*
            Port Group <i>                    Port Group <i + 1>
         __________________                   __________________
        |                  |                 |                  |
        | port(SERDES) <x> |________     ____| port(SERDES) <z> |
        |______CLK0________|        |   |    |_______CLK0_______|
                                    |   |
         __________________         |   |     __________________
        |                  |        |   |    |                  |
        | port(SERDES) <y> |  ______|___|____| port(SERDES) <w> |
        |______CLK1________| |      |   |    |_______CLK1_______|
                    |        |      |   |
                    |        |      |   |
                    |        |      |   |
                   _|________|_    _|___|_____
                   \MUX <i+1>/     \ MUX <i> /
                    \_______/       \_______/
                        |               |
                Recovered Clock    Recovered Clock
                 Output <i+1>        Output <i>

    Note: i = 2N, where N = 0-3
    */

    rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* HGL port not supported for clock recovery */
    if(CPSS_PORT_INTERFACE_MODE_HGL_E == ifMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPortIfModeSerdesNumGet(devNum, portNum, ifMode, &startSerdes, &maxLaneNum);
    if( GT_OK != rc )
    {
        return rc;
    }

    if( laneNum >= maxLaneNum )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    serdesIdx = startSerdes + laneNum;

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);

    switch(recoveryClkType)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
            if(( 0 != portGroup ) && ( 1 != portGroup ))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK2_E:
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK3_E:
            if(( 2 != portGroup ) && ( 3 != portGroup ))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK4_E:
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK5_E:
            if(( 4 != portGroup ) && ( 5 != portGroup ))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK6_E:
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK7_E:
            if(( 6 != portGroup ) && ( 7 != portGroup ))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Find the right offset for Clock0 select or Clock1 select */
    if(((portGroup == recoveryClkTypeU32) && (portGroup % 2 == 0)) ||
       ((portGroup != recoveryClkTypeU32) && (portGroup % 2 != 0)))
    {
        /* Clock0 Select */
        regOffset = 11;
    }
    else
    {
        /* Clock1 Select */
        regOffset = 15;
    }

    if( GT_FALSE == enable )
    {
        /* Serdes External Configuration 1 Register */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIdx].serdesExternalReg2;

        /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroup, regAddr, regOffset, 1, 1);
    }
    else /* GT_TRUE == enable */
    {
        switch (PRV_CPSS_PP_MAC(devNum)->devType)
        {

            case CPSS_LION_2_THREE_MINI_GOPS_DEVICES_CASES_MAC:
                maxSerdesNumber = PRV_CPSS_LION2_SERDES_NUM_CNS - 6;
                break;
            case CPSS_LION_2_FOUR_MINI_GOPS_DEVICES_CASES_MAC:
                maxSerdesNumber = PRV_CPSS_LION2_SERDES_NUM_CNS;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        for( serdes = 0 ; serdes < maxSerdesNumber ; serdes++ )
        {
            if ( serdes == serdesIdx )
            {
                continue;
            }

            /* Serdes External Configuration 1 Register */
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].serdesExternalReg2;

            /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
            rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroup, regAddr, regOffset, 1, &regValue);
            if( GT_OK != rc )
            {
                return rc;
            }

            if( 0 == regValue )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

        /* Serdes External Configuration 1 Register */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIdx].serdesExternalReg2;

        /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroup, regAddr, regOffset, 1, 0);

        /******************************************/
        /* Clock0/Clock1 Output mux configuration */
        /******************************************/

        /* The configuration for cores 2N and 2N+1 resides in the XLG unit
           of port 0 in core 2N. */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[0].xlgExternalControlRegister;


        /* Set Recovered Clock Output <i> / <i+1> */
        if((portGroup == recoveryClkTypeU32) && (portGroup % 2 == 0))
        {
            /* Port Group Recovered Clock 0 Select*/
            regOffset = 0;
            /* Use core 2N */
            regValue = 0;
        }
        else if((portGroup != recoveryClkTypeU32) && (portGroup % 2 != 0))
        {
            /* Port Group Recovered Clock 0 Select*/
            regOffset = 0;
            /* Use core 2N+1 */
            regValue = 1;
        }
        else if((portGroup == recoveryClkTypeU32) && (portGroup % 2 != 0))
        {
            /* Port Group Recovered Clock 1 Select*/
            regOffset = 1;
            /* Use core 2N+1 */
            regValue = 1;
        }
        else
        {   /*  portGroup != recoveryClkType && portGroup % 2 == 0 */

            /* Port Group Recovered Clock 1 Select*/
            regOffset = 1;
            /* Use core 2N */
            regValue = 0;
        }

        /* The configuration for cores 2N and 2N+1 resides in the XLG unit
           of port 0 in core 2N. */

        portGroup &= ~1;

        rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroup, regAddr, regOffset, 1, regValue);
    }

    return rc;
}


/**
* @internal prvCpssDxChPortSyncEtherRecoveryClkConfigSetForBobcat2 function
* @endinternal
*
* @brief   Function configures the recovery clock enable/disable state and sets
*         its source portNum.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
* @param[in] enable                   - enable/disable recovered clock0/clock1
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType, portNum
*                                       or laneNum.
* @retval GT_BAD_STATE             - another SERDES already enabled
* @retval GT_HW_ERROR              - on hardware error
*
* @note If SERDES enabling is requesed and another SERDES in the port group
*       is already enabled, GT_BAD_STATE is returned.
*
*/
static GT_STATUS prvCpssDxChPortSyncEtherRecoveryClkConfigSetForBobcat2
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL enable,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum
)
{
    GT_STATUS rc;           /* returned status */
    GT_U32  regAddr;        /* register address */
    GT_U32  dfxRegAddr = 0; /* DFX server register address */
    GT_U32  dfxRegAddr2 = 0; /* DFX server register address */
    GT_U32  dfxRegValue = 0;/* DFX register value */
    GT_U32  dfxRegValue2 = 0;/* DFX register value */
    GT_U32  regValue = 0;   /* register value */
    GT_U32  regOffset;      /* register offset */
    GT_U32  regOffset1;     /* register offset */
    GT_U32  regOffset2 = 0; /* register offset */
    GT_U32  dfxRegOffset2 = 0; /* register offset */
    GT_U32  serdes;         /* loop index */
    GT_U32  serdesIdx;      /* current serdes index */
    GT_U32  maxSerdesNumber;/* maximal serdes number */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */
    CPSS_PORT_SPEED_ENT speed;
    CPSS_DXCH_PORT_FEC_MODE_ENT fecMode;
    CPSS_DXCH_ACTIVE_LANES_STC  portParams;

    rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
    if( GT_OK != rc )
    {
        return rc;
    }

    rc = cpssDxChPortSpeedGet(devNum, portNum, &speed);
    if( GT_OK != rc )
    {
        return rc;
    }

    rc = prvCpssDxChPortParamsGet(devNum,portNum,&portParams);
    if( GT_OK != rc )
    {
        return rc;
    }

    if( laneNum >= portParams.numActiveLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    serdesIdx = portParams.activeLaneList[laneNum];

    if((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE) && (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_TRUE))
    {
        dfxRegAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl16;

        if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_TRUE)  /* Aldrin 2 */
        {
            if (serdesIdx <= 23)
            {
                dfxRegValue = 0;
            }
            else if(serdesIdx <= 72)
            {
                dfxRegValue = 1;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else                                                /* Bobcat3 */
        {
            if (serdesIdx <= 71)
            {
                dfxRegValue = (serdesIdx / 36);
            }
            else if ((serdesIdx == 72) || (serdesIdx == 73))
            {
                dfxRegValue = (serdesIdx % 2);
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* Find the right offset for Clock0 select or Clock1 select */
    switch (recoveryClkType)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
            regOffset = 15;
            regOffset1 = 6;
            if((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE) && (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_TRUE))
            {
                regOffset2 = 5;
            }
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
            regOffset = 11;
            regOffset1 = 5;
            if((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE) && (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_TRUE))
            {
                regOffset2 = 4;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if( GT_FALSE == enable )
    {
        /* Serdes External Configuration 1 Register */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIdx].serdesExternalReg2;

        /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset, 1, 1);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT3_100G_SYNC_ETHERNET_WA_E))
        {
            if((ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR4_E) ||
               (ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR2_E) ||
               (ifMode == CPSS_PORT_INTERFACE_MODE_KR4_E) ||
               (ifMode == CPSS_PORT_INTERFACE_MODE_KR2_E) ||
               (ifMode == CPSS_PORT_INTERFACE_MODE_CR4_E && speed !=  CPSS_PORT_SPEED_40000_E))
            {
                /* <rcvrd_clk0_auto_mask_en> or <rcvrd_clk1_auto_mask_en> */
                rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset1, 1, 0);
                if(GT_OK != rc)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            if (ifMode == CPSS_PORT_INTERFACE_MODE_KR_E && speed ==  CPSS_PORT_SPEED_25000_E)
            {
                rc = cpssDxChPortFecModeGet(devNum, portNum, &fecMode);
                if( GT_OK != rc )
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
                if (fecMode == CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E)
                {
                    /* <rcvrd_clk0_auto_mask_en> or <rcvrd_clk1_auto_mask_en> */
                    rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset1, 1, 0);
                    if(GT_OK != rc)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                }
            }
        }
    }
    else /* GT_TRUE == enable */
    {
        maxSerdesNumber = prvCpssDxChHwInitNumOfSerdesGet(devNum);

        for( serdes = 0 ; serdes < maxSerdesNumber ; serdes++ )
        {
            if ( serdes == serdesIdx )
            {
                continue;
            }

            /* skip not valid SERDESes in the loop */
            PRV_CPSS_DXCH_SKIP_NON_VALID_SERDES_IN_LOOP_MAC(devNum,serdes);

            /* Serdes External Configuration 1 Register */
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].serdesExternalReg2;

            /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
            rc = prvCpssHwPpGetRegField(devNum, regAddr, regOffset, 1, &regValue);
            if( GT_OK != rc )
            {
                return rc;
            }

            if( 0 == regValue )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

        /* Serdes External Configuration 1 Register */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdesIdx].serdesExternalReg2;

        /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
        rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset, 1, 0);
        if(GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_BOBCAT3_100G_SYNC_ETHERNET_WA_E))
        {
            if((ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR4_E) ||
               (ifMode == CPSS_PORT_INTERFACE_MODE_SR_LR2_E) ||
               (ifMode == CPSS_PORT_INTERFACE_MODE_KR4_E) ||
               (ifMode == CPSS_PORT_INTERFACE_MODE_KR2_E) ||
               (ifMode == CPSS_PORT_INTERFACE_MODE_CR4_E && speed !=  CPSS_PORT_SPEED_40000_E))
            {
                /* <rcvrd_clk0_auto_mask_en> or <rcvrd_clk1_auto_mask_en> */
                rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset1, 1, 1);
                if(GT_OK != rc)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            if (ifMode == CPSS_PORT_INTERFACE_MODE_KR_E && speed ==  CPSS_PORT_SPEED_25000_E)
            {
                rc = cpssDxChPortFecModeGet(devNum, portNum, &fecMode);
                if( GT_OK != rc )
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
                if (fecMode == CPSS_DXCH_PORT_RS_FEC_MODE_ENABLED_E)
                {
                    /* <rcvrd_clk0_auto_mask_en> or <rcvrd_clk1_auto_mask_en> */
                    rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset1, 1, 1);
                    if(GT_OK != rc)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                    }
                }
            }
        }

        /*  To determine which port group the recovered clock outputs are selected from, configure the
            2-bit <rcvr_clk_out_sel> field in the Device General Control 16 Register:
                - The lower bit is <Recovered Clock0 Select>, corresponding to RCVR_CLK_OUT<0>.
                - The higher bit is <Recovered Clock1 Select>, corresponding to RCVR_CLK_OUT<1>.
        */
        if((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE) && (PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) == GT_TRUE))
        {
            if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_TRUE)  /* Aldrin 2 */
            {
                /* Muxing between MPP and SyncE
                 * clkOvrdOffset[18] Enable - Indicates GPP[1] & GPP[2]
                 *                            are used as RCVR_CLK_OUT[1:0]
                 */
                dfxRegAddr2 = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl33;
                dfxRegOffset2 = 18;
                dfxRegValue2 = 1;

                rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, dfxRegAddr2, dfxRegOffset2, 1, dfxRegValue2);
                if(GT_OK != rc)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }
            rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, dfxRegAddr, regOffset2, 1, dfxRegValue);
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSyncEtherRecoveryClkConfigSetForSip6_10 function
* @endinternal
*
* @brief   Function configures the recovery clock enable/disable state and sets
*         its source portNum.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
* @param[in] enable                   - enable/disable recovered clock0/clock1
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] portNum                  - port number.
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType, portNum
*                                       or laneNum.
* @retval GT_BAD_STATE             - another SERDES already enabled
* @retval GT_HW_ERROR              - on hardware error
*
* @note If SERDES enabling is requesed and another SERDES in the port group
*       is already enabled, GT_BAD_STATE is returned.
*
*/
static GT_STATUS prvCpssDxChPortSyncEtherRecoveryClkConfigSetForSip6_10
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL enable,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum
)
{
    GT_STATUS rc;           /* returned status */
    GT_U32  regAddr;        /* register address */
    GT_U32  regValue = 0;   /* register value */
    GT_U32  regValue4 = 0;  /* register value */
    GT_U32  regOffset;      /* register offset */
    GT_U32  regOffset1 = 0; /* register offset */
    GT_U32  regOffset2 = 0; /* register offset */
    GT_U32  regOffset3 = 0; /* register offset */
    GT_U32  regOffset4 = 0; /* register offset */
    GT_U32  regMask;        /* register field mask */
    GT_U32  serdes;         /* loop index */
    GT_U32  serdesIdx;      /* current serdes index */
    GT_U32  maxSerdesNumber;/* maximal serdes number */
    CPSS_DXCH_ACTIVE_LANES_STC  portParams;

    rc = prvCpssDxChPortParamsGet(devNum,portNum,&portParams);
    if( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if( laneNum >= portParams.numActiveLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    serdesIdx = portParams.activeLaneList[laneNum];

    maxSerdesNumber = prvCpssDxChHwInitNumOfSerdesGet(devNum);

    /* Find the right offset for Clock0 select or Clock1 select */
    switch (recoveryClkType)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
            regOffset = 8;      /*mux_sel*/
            regOffset1 = 10;    /*lane_sel*/
            regOffset2 = 16;    /*clk_disabled*/
            regOffset3 = 18;    /*clk_div_reset_*/
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
            {
                /*
                    Select between the Ethernet COMPHYs' Recovery clocks:
                        0x0 = select clk0 of chain0 and clk1 of chain0
                        0x1 = select clk0 of chain1 and clk1 of chain 0
                        0x2 = select clk0 of chain0 and clk1 of chain1
                        0x3 = select clk0 of chain1 and clk1 of chain1
                        chain0 is composed by COMPHY1 -> COMPHY0
                        chain1 is composed by COMPHY3 -> COMPHY2 -> COMPHY4
                */
                regValue4 = (serdesIdx < 8) ? 0 : 1;
                regOffset4 = 0;    /*rcvr_clk_out_sel[0]*/
                regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl30;
            }
            else /* Hawk */
            {
                regValue4 = ((serdesIdx < maxSerdesNumber/2) || (serdesIdx == maxSerdesNumber-1)) ? 1 : 0;
                regOffset4 = 8;    /*rcvr_clk_out_sel[0]*/
                regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl13;
            }
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
            regOffset = 9;      /*mux_sel*/
            regOffset1 = 12;    /*lane_sel*/
            regOffset2 = 17;    /*clk_disabled*/
            regOffset3 = 19;    /*clk_div_reset_*/
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
            {
                /*
                    Select between the Ethernet COMPHYs' Recovery clocks:
                        0x0 = select clk0 of chain0 and clk1 of chain0
                        0x1 = select clk0 of chain1 and clk1 of chain 0
                        0x2 = select clk0 of chain0 and clk1 of chain1
                        0x3 = select clk0 of chain1 and clk1 of chain1
                        chain0 is composed by COMPHY1 -> COMPHY0
                        chain1 is composed by COMPHY3 -> COMPHY2 -> COMPHY4
                */
                regValue4 = (serdesIdx < 8) ? 0 : 1;
                regOffset4 = 1;    /*rcvr_clk_out_sel[1]*/
                regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl30;
            }
            else
            {
                regValue4 = ((serdesIdx < maxSerdesNumber/2) || (serdesIdx == maxSerdesNumber-1)) ? 1 : 0;
                regOffset4 = 9;    /*rcvr_clk_out_sel[1]*/
                regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl13;
            }
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if( GT_FALSE == enable )
    {
        /* <sync_e0_mux_sel> or <sync_e1_mux_sel> */
        rc = prvCpssGenPortGroupSerdesWriteRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                      serdesIdx,
                                                      SDW_GENERAL_CONTROL_3,
                                                      0, (1 << regOffset));
        if( GT_OK != rc )
        {
            return rc;
        }

        regMask = (3 << regOffset1);
        /* <sync_e0_lane_sel> or <sync_e1_lane_sel> */
        rc = prvCpssGenPortGroupSerdesWriteRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                      serdesIdx,
                                                      SDW_GENERAL_CONTROL_3,
                                                      0, regMask);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* <rcvrd_clk0_local_clk_div_reset_> or <rcvrd_clk1_local_clk_div_reset_> */
        rc = prvCpssGenPortGroupSerdesWriteRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                      serdesIdx,
                                                      SDW_GENERAL_CONTROL_3,
                                                      0, (1 << regOffset3));
        if( GT_OK != rc )
        {
            return rc;
        }

        /* <rcvrd_clk0_local_clk_disabled> or <rcvrd_clk1_local_clk_disabled> */
        rc = prvCpssGenPortGroupSerdesWriteRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                      serdesIdx,
                                                      SDW_GENERAL_CONTROL_3,
                                                      (1 << regOffset2), (1 << regOffset2));
        if( GT_OK != rc )
        {
            return rc;
        }

    }
    else /* GT_TRUE == enable */
    {
        for( serdes = 0 ; serdes < maxSerdesNumber ; serdes++ )
        {
            if(serdes == serdesIdx)
            {
                continue;
            }

            /* <sync_e0_mux_sel> or <sync_e1_mux_sel> */
            rc = prvCpssGenPortGroupSerdesReadRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                         serdes,
                                                         SDW_GENERAL_CONTROL_3,
                                                         &regValue, (1 << regOffset));
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            if(regValue != 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }

        /* <rcvrd_clk0_local_clk_disabled> or <rcvrd_clk1_local_clk_disabled> */
        rc = prvCpssGenPortGroupSerdesWriteRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                      serdesIdx,
                                                      SDW_GENERAL_CONTROL_3,
                                                      0, (1 << regOffset2));
        if( GT_OK != rc )
        {
            return rc;
        }

        /* <rcvrd_clk0_local_clk_div_reset_> or <rcvrd_clk1_local_clk_div_reset_> */
        rc = prvCpssGenPortGroupSerdesWriteRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                      serdesIdx,
                                                      SDW_GENERAL_CONTROL_3,
                                                      (1 << regOffset3), (1 << regOffset3));
        if( GT_OK != rc )
        {
            return rc;
        }

        /* <sync_e0_mux_sel> or <sync_e1_mux_sel> */
        rc = prvCpssGenPortGroupSerdesWriteRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                      serdesIdx,
                                                      SDW_GENERAL_CONTROL_3,
                                                      (1 << regOffset), (1 << regOffset));
        if( GT_OK != rc )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        regValue = (hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[serdesIdx].internalLane << regOffset1);
        regMask = (3 << regOffset1);
        /* <sync_e0_lane_sel> or <sync_e1_lane_sel> */
        rc = prvCpssGenPortGroupSerdesWriteRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                      serdesIdx,
                                                      SDW_GENERAL_CONTROL_3,
                                                      regValue, regMask);
        if( GT_OK != rc )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) ||
            (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E))
        {
            rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset4, 1, regValue4);
            if( GT_OK != rc )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortSyncEtherRecoveryClkConfigSet function
* @endinternal
*
* @brief   Function configures the recovery clock enable/disable state and sets
*         its source portNum.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
* @param[in] enable                   - enable/disable recovered clock1/clock2
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] portNum                  - port number.
*                                      For xCat3 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType, portNum
*                                       or laneNum.
* @retval GT_BAD_STATE             - another SERDES already enabled
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChPortSyncEtherRecoveryClkConfigSet
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL enable,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */
    GT_U32  regOffset;  /* register offset */
    GT_U32  portGroup;  /* port group */
    GT_STATUS rc;       /* returned status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_ENHANCED_PHY_PORT_CHECK_MAC(devNum, portNum);


    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        return prvCpssDxChPortSyncEtherRecoveryClkConfigSetForLion2(devNum,
                                                                    recoveryClkType,
                                                                    enable,
                                                                    portNum,
                                                                    laneNum);
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortSyncEtherRecoveryClkConfigSetForSip6_10(devNum,
                                                                      recoveryClkType,
                                                                      enable,
                                                                      portNum,
                                                                      laneNum);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        return prvCpssDxChPortSyncEtherRecoveryClkConfigSetForBobcat2(devNum,
                                                                      recoveryClkType,
                                                                      enable,
                                                                      portNum,
                                                                      laneNum);
    }


    /* Check port number */
    switch (portNum)
    {
        case 0:     /* Group index for Giga ports = 4-9 */
        case 4:
        case 8:
        case 12:
        case 16:
        case 20:
            portGroup = (4 + portNum / 4);
            break;
        case 24:    /* Group index for Stacking ports = 0-3 */
        case 25:
        case 26:
        case 27:
            portGroup = (portNum - 24);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* calculate register offset */
    switch(recoveryClkType)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
            regOffset = 10;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
            regOffset = 14;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* set register value */
    regValue = (enable == GT_TRUE) ? portGroup : 0xF;

    /* Extended Global Control2 register address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.extendedGlobalControl2;

    /* set <rcvr_clk0_ctrl> or <rcvr_clk1_ctrl> in extended global register2 */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, regOffset, 4, regValue);

    return rc;
}

/**
* @internal cpssDxChPortSyncEtherRecoveryClkConfigSet function
* @endinternal
*
* @brief   Function configures the recovery clock enable/disable state and sets
*         its source portNum.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
* @param[in] enable                   - enable/disable recovered clock1/clock2
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
* @param[in] portNum                  - port number.
*                                      For xCat3 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType, portNum
*                                       or laneNum.
* @retval GT_BAD_STATE             - another SERDES already enabled
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortSyncEtherRecoveryClkConfigSet
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    IN  GT_BOOL enable,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSyncEtherRecoveryClkConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, recoveryClkType, enable, portNum, laneNum));

    rc = internal_cpssDxChPortSyncEtherRecoveryClkConfigSet(devNum, recoveryClkType, enable, portNum, laneNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, recoveryClkType, enable, portNum, laneNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortSyncEtherRecoveryClkConfigGetForLion2 function
* @endinternal
*
* @brief   Function gets the recovery clock enable/disable state and its source
*         portNum.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type
*
* @param[out] enablePtr                - (pointer to) port state as reference.
*                                      GT_TRUE -  enabled
*                                      GT_FALSE - disbled
* @param[out] portNumPtr               - (pointer to) port number.
* @param[out] laneNumPtr               - (pointer to) selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad port interface state
*/
static GT_STATUS prvCpssDxChPortSyncEtherRecoveryClkConfigGetForLion2
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *enablePtr,
    OUT GT_PHYSICAL_PORT_NUM    *portNumPtr,
    OUT GT_U32   *laneNumPtr
)
{
    GT_STATUS rc;       /* returned status */
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */
    GT_U32  portGroup;  /* port group */
    GT_U32  regOffset;  /* register offset */
    GT_U32  regOffsetMux;  /* register offset for clock mux*/
    GT_U32   localPort;  /* local port - support multi-port-groups device */
    GT_U32  serdes;     /* loop index */
    GT_U32  maxSerdesNumber;/* maximal serdes number */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* port interface mode */
    CPSS_PORT_SPEED_ENT             speed; /* current port speed */


    /*
            Port Group <i>                    Port Group <i + 1>
         __________________                   __________________
        |                  |                 |                  |
        | port(SERDES) <x> |________     ____| port(SERDES) <z> |
        |______CLK0________|        |   |    |_______CLK0_______|
                                    |   |
         __________________         |   |     __________________
        |                  |        |   |    |                  |
        | port(SERDES) <y> |  ______|___|____| port(SERDES) <w> |
        |______CLK1________| |      |   |    |_______CLK1_______|
                    |        |      |   |
                    |        |      |   |
                    |        |      |   |
                   _|________|_    _|___|_____
                   \MUX <i+1>/     \ MUX <i> /
                    \_______/       \_______/
                        |               |
                Recovered Clock    Recovered Clock
                 Output <i+1>        Output <i>

    Note: i = 2N, where N = 0-3
    */

    switch(recoveryClkType)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK2_E:
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK4_E:
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK6_E:
            regOffset = 11;
            regOffsetMux = 0;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK3_E:
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK5_E:
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK7_E:
            regOffset = 15;
            regOffsetMux = 1;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* The configuration for cores 2N and 2N+1 resides in the XLG unit
       of port 0 in core 2N. */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->macRegs.perPortRegs[0].xlgExternalControlRegister;

    /* portGroup should be even (2N) */
    portGroup = (GT_U32)recoveryClkType & ~1;

    /* read XLG external control register */
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroup, regAddr, regOffsetMux, 1, &regValue);

    /* check current MUX configuration in order to choose appropriate portGroup */
    if(regValue != 0) /* 2N+1 case */
    {
        ++portGroup;
    }

    /* get actual number of SERDESes */
    switch (PRV_CPSS_PP_MAC(devNum)->devType)
    {

        case CPSS_LION_2_THREE_MINI_GOPS_DEVICES_CASES_MAC:
            maxSerdesNumber = PRV_CPSS_LION2_SERDES_NUM_CNS - 6;
            break;
        case CPSS_LION_2_FOUR_MINI_GOPS_DEVICES_CASES_MAC:
            maxSerdesNumber = PRV_CPSS_LION2_SERDES_NUM_CNS;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    for( serdes = 0 ; serdes < maxSerdesNumber ; serdes++ )
    {
        /* Serdes External Configuration 1 Register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[serdes].serdesExternalReg2;

        /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
        rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroup, regAddr, regOffset, 1, &regValue);
        if( GT_OK != rc )
        {
            return rc;
        }

        if( 0 == regValue )
        {
            break;
        }
    }

    if( maxSerdesNumber == serdes )
    {
        *enablePtr = GT_FALSE;
    }
    else /* SERDES which was configured for recovery clock was found */
    {
        /* find port interface to calculate lane number */

        /* check is SGMII / 10G-KR port */
        localPort = (GT_U8)(serdes - 2*(serdes/6));
        *portNumPtr = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum, portGroup, localPort);
        rc = cpssDxChPortInterfaceModeGet(devNum, *portNumPtr, &ifMode);
        rc |= cpssDxChPortSpeedGet(devNum, *portNumPtr, &speed);

        if( (GT_OK == rc) &&  ((CPSS_PORT_INTERFACE_MODE_SGMII_E == ifMode)
                                || (CPSS_PORT_INTERFACE_MODE_1000BASE_X_E == ifMode)
                                || ((CPSS_PORT_INTERFACE_MODE_KR_E == ifMode) && (CPSS_PORT_SPEED_10000_E == speed))
                                || ((CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode) && (CPSS_PORT_SPEED_10000_E == speed))
                                || ((CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode) && (CPSS_PORT_SPEED_11800_E == speed))
                                || ((CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode) && (CPSS_PORT_SPEED_11800_E == speed))
                               ))
        {
            *enablePtr = GT_TRUE;
            *laneNumPtr = 0 ;
            return GT_OK;
        }

        /* check is RXAUI / 20G-KR2 port */
        /*  SERDES  |   PORT
             0-1    |    0
             2-3    |    2
             6-7    |    4
             8-9    |    6
             12-13  |    8
             14-15  |    10
             18-19  |    9
             20-21  |    11
        */
        if(serdes < 18)
        {
            localPort = (GT_U8)((serdes&~1) - 2*(serdes/6));
        }
        else if((serdes == 18) || (serdes == 19))
        {
            localPort = 9;
        }
        else
        {   /* serdes == 20 || serdes == 21*/
            localPort = 11;
        }

        *portNumPtr = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum, portGroup, localPort);
        rc = cpssDxChPortInterfaceModeGet(devNum, *portNumPtr, &ifMode);
        rc |= cpssDxChPortSpeedGet(devNum, *portNumPtr, &speed);

        if( (GT_OK == rc) &&  ((CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode)
                                || (CPSS_PORT_INTERFACE_MODE_HX_E == ifMode)
                                || ((CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode) && (CPSS_PORT_SPEED_20000_E == speed))
                                || ((CPSS_PORT_INTERFACE_MODE_SR_LR2_E == ifMode) && (CPSS_PORT_SPEED_20000_E == speed))))
        {
            *enablePtr = GT_TRUE;
            *laneNumPtr = serdes & 1;
            return GT_OK;
        }

        /* check is XAUI / 40G-KR4 port */
        /*  SERDES  |   PORT
             0-3    |    0
             6-9    |    4
             12-15  |    8
             18-21  |    9
        */
        if(serdes < 18)
        {
            localPort = (GT_U8)((serdes/6)*4);
        }
        else
        {
            localPort = 9;
        }
        *portNumPtr = PRV_CPSS_LOCAL_PORT_TO_GLOBAL_PORT_CONVERT_MAC(devNum, portGroup, localPort);
        rc = cpssDxChPortInterfaceModeGet(devNum, *portNumPtr, &ifMode);
        rc |= cpssDxChPortSpeedGet(devNum, *portNumPtr, &speed);

        if( (GT_OK == rc) && ((CPSS_PORT_INTERFACE_MODE_XGMII_E == ifMode)
                             ||((CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode) && (CPSS_PORT_SPEED_40000_E == speed))
                             ||((CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode) && (CPSS_PORT_SPEED_40000_E == speed))
                             ||((CPSS_PORT_INTERFACE_MODE_SR_LR4_E == ifMode) && (CPSS_PORT_SPEED_40000_E == speed))
                             || ((CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode) && (CPSS_PORT_SPEED_47200_E == speed))
                             || ((CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode) && (CPSS_PORT_SPEED_47200_E == speed))
                              ))
        {
            *enablePtr = GT_TRUE;
            *laneNumPtr = (serdes - 2*(serdes/6)) & 3;
            return GT_OK;
        }

        /* no port interface to lane number match found */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChPortSyncEtherRecoveryClkConfigGetForBobcat2 function
* @endinternal
*
* @brief   Function gets the recovery clock enable/disable state and its source
*         portNum.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
*
* @param[out] enablePtr                - (pointer to) port state as reference.
*                                      GT_TRUE -  enabled
*                                      GT_FALSE - disbled
* @param[out] portNumPtr               - (pointer to) port number.
* @param[out] laneNumPtr               - (pointer to) selected SERDES lane number within port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType, portNum
*                                       or laneNum.
* @retval GT_BAD_STATE             - another SERDES already enabled
* @retval GT_HW_ERROR              - on hardware error
*
* @note If SERDES enabling is requesed and another SERDES in the port group
*       is already enabled, GT_BAD_STATE is returned.
*
*/
static GT_STATUS prvCpssDxChPortSyncEtherRecoveryClkConfigGetForBobcat2
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *enablePtr,
    OUT GT_PHYSICAL_PORT_NUM    *portNumPtr,
    OUT GT_U32   *laneNumPtr
)
{
    GT_STATUS rc;           /* returned status */
    GT_U32  regAddr;        /* register address */
    GT_U32  regValue = 0;   /* register value */
    GT_U32  regOffset;      /* register offset */
    GT_U32  serdes;         /* loop index */
    CPSS_DXCH_ACTIVE_LANES_STC  portParams;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */
    GT_PHYSICAL_PORT_NUM portNum; /* loop iterator */
    GT_BOOL isValid;        /* port mapping status */

    /* Find the right offset for Clock0 select or Clock1 select */
    switch (recoveryClkType)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
            regOffset = 15;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
            regOffset = 11;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        if(PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum) == GT_TRUE)
        {
            rc = cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
            if((rc != GT_OK) || (isValid != GT_TRUE))
                continue;
        }
        else
        {
            PRV_CPSS_DXCH_SKIP_NOT_EXIST_PORT_MAC(devNum, portNum);
        }

        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if((ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) || ( GT_OK != rc ))
        {
            continue;
        }

        rc = prvCpssDxChPortParamsGet(devNum,portNum,&portParams);
        if( GT_OK != rc )
        {
            return rc;
        }

        for( serdes = 0; serdes < portParams.numActiveLanes; serdes++ )
        {
            /* Serdes External Configuration 1 Register */
            regAddr =
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[serdes]].serdesExternalReg2;

            /* <rcvrd_clk0_sel> or <rcvrd_clk1_sel> */
            rc = prvCpssHwPpGetRegField(devNum, regAddr, regOffset, 1, &regValue);
            if( GT_OK != rc )
            {
                return rc;
            }

            if( 0 == regValue )
            {
                *enablePtr = GT_TRUE;
                *laneNumPtr = serdes;
                *portNumPtr = portNum;

                return GT_OK;
            }
        }
    }

    *enablePtr = GT_FALSE;
    *laneNumPtr = 0;
    *portNumPtr = CPSS_MAX_PORTS_NUM_CNS;

    return GT_OK;
}

/**
* @internal prvCpssDxChPortSyncEtherRecoveryClkConfigGetForSip6_10 function
* @endinternal
*
* @brief   Function gets the recovery clock enable/disable state and its source
*         portNum.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type.
*
* @param[out] enablePtr                - (pointer to) port state as reference.
*                                      GT_TRUE -  enabled
*                                      GT_FALSE - disbled
* @param[out] portNumPtr               - (pointer to) port number.
* @param[out] laneNumPtr               - (pointer to) selected SERDES lane number within port.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType, portNum
*                                       or laneNum.
* @retval GT_BAD_STATE             - another SERDES already enabled
* @retval GT_HW_ERROR              - on hardware error
*
* @note If SERDES enabling is requesed and another SERDES in the port group
*       is already enabled, GT_BAD_STATE is returned.
*
*/
static GT_STATUS prvCpssDxChPortSyncEtherRecoveryClkConfigGetForSip6_10
(
    IN  GT_U8   devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *enablePtr,
    OUT GT_PHYSICAL_PORT_NUM    *portNumPtr,
    OUT GT_U32   *laneNumPtr
)
{
    GT_STATUS rc;           /* returned status */
    GT_U32  regValue = 0;   /* register value */
    GT_U32  regOffset;      /* register offset */
    GT_U32  regOffset1;     /* register offset */
    GT_U32  serdes;         /* loop index */
    CPSS_DXCH_ACTIVE_LANES_STC  portParams;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */
    GT_PHYSICAL_PORT_NUM portNum; /* loop iterator */
    GT_BOOL isValid;        /* port mapping status */

    *enablePtr = GT_FALSE;

    /* Find the right offset for Clock0 select or Clock1 select */
    switch (recoveryClkType)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
            regOffset = 8;
            regOffset1 = 10;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
            regOffset = 9;
            regOffset1 = 12;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
    {
        rc = cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
        if((rc != GT_OK) || (isValid != GT_TRUE))
            continue;

        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if((ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) || ( GT_OK != rc ))
        {
            continue;
        }

        rc = prvCpssDxChPortParamsGet(devNum,portNum,&portParams);
        if( GT_OK != rc )
        {
            return rc;
        }

        for( serdes = 0; serdes < portParams.numActiveLanes; serdes++ )
        {
            /* <sync_e0_mux_sel> or <sync_e1_mux_sel> */
            rc = prvCpssGenPortGroupSerdesReadRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                         portParams.activeLaneList[serdes],
                                                         SDW_GENERAL_CONTROL_3,
                                                         &regValue, (1 << regOffset));
            if(GT_OK != rc)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            if(regValue != 0)
            {
                /* <sync_e0_lane_sel> or <sync_e1_lane_sel> */
                rc = prvCpssGenPortGroupSerdesReadRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                             portParams.activeLaneList[serdes],
                                                             SDW_GENERAL_CONTROL_3,
                                                             &regValue, (3 << regOffset1));

                *enablePtr = GT_TRUE;
                /* global serdes index calculate */
                regValue = (regValue >> regOffset1) + (portParams.activeLaneList[serdes] & 0xFFFFFFFC);

                break;
            }
        }
        if(*enablePtr == GT_TRUE)
        {
            break;
        }
    }

    if(*enablePtr == GT_TRUE)
    {
        for (portNum = 0; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
        {
            rc = cpssDxChPortPhysicalPortMapIsValidGet(devNum, portNum, &isValid);
            if((rc != GT_OK) || (isValid != GT_TRUE))
                continue;

            rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
            if((ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) || ( GT_OK != rc ))
            {
                continue;
            }

            rc = prvCpssDxChPortParamsGet(devNum,portNum,&portParams);
            if( GT_OK != rc )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            for( serdes = 0; serdes < portParams.numActiveLanes; serdes++ )
            {
                if(portParams.activeLaneList[serdes] == regValue)
                {
                    *portNumPtr = portNum;
                    *laneNumPtr = serdes;
                    return GT_OK;
                }
            }
        }
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }
    else
    {
        *enablePtr = GT_FALSE;
        *laneNumPtr = 0;
        *portNumPtr = CPSS_MAX_PORTS_NUM_CNS;
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortSyncEtherRecoveryClkConfigGet function
* @endinternal
*
* @brief   Function gets the recovery clock enable/disable state and its source
*         portNum.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type
*
* @param[out] enablePtr                - (pointer to) port state as reference.
*                                      GT_TRUE -  enabled
*                                      GT_FALSE - disbled
* @param[out] portNumPtr               - (pointer to) port number.
* @param[out] laneNumPtr               - (pointer to) selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on bad state of register
*/
static GT_STATUS internal_cpssDxChPortSyncEtherRecoveryClkConfigGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *enablePtr,
    OUT GT_PHYSICAL_PORT_NUM    *portNumPtr,
    OUT GT_U32   *laneNumPtr
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regValue;   /* register value */
    GT_U32  regOffset;  /* register offset */
    GT_STATUS rc;       /* returned status */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);
    CPSS_NULL_PTR_CHECK_MAC(laneNumPtr);

    if(CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily)
    {
        return prvCpssDxChPortSyncEtherRecoveryClkConfigGetForLion2(
                            devNum,recoveryClkType,enablePtr,portNumPtr,laneNumPtr);
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortSyncEtherRecoveryClkConfigGetForSip6_10(
                            devNum,recoveryClkType,enablePtr,portNumPtr,laneNumPtr);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        return prvCpssDxChPortSyncEtherRecoveryClkConfigGetForBobcat2(
                            devNum,recoveryClkType,enablePtr,portNumPtr,laneNumPtr);
    }

    /* calculate register offset */
    switch(recoveryClkType)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E:
            regOffset = 10;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK1_E:
            regOffset = 14;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Extended Global Control2 register address */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->globalRegs.extendedGlobalControl2;

    /* get <rcvr_clk0_ctrl> or <rcvr_clk1_ctrl> in extended global register2 */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, regOffset, 4, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (regValue < 10)
    {
        *enablePtr = GT_TRUE;
    }
    else if (regValue == 0xF)
    {
        *enablePtr = GT_FALSE;
        return GT_OK;
    }
    else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

    /* recovery clock output pin port group number */
    switch (regValue)
    {
        /* Stacking ports */
        case 0:
        case 1:
        case 2:
        case 3:
            *portNumPtr = (GT_U8)(24 + regValue);
            break;
        /* Giga ports */
        default:
            *portNumPtr = (GT_U8)((regValue - 4) * 4);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSyncEtherRecoveryClkConfigGet function
* @endinternal
*
* @brief   Function gets the recovery clock enable/disable state and its source
*         portNum.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] recoveryClkType          - recovered clock output type
*
* @param[out] enablePtr                - (pointer to) port state as reference.
*                                      GT_TRUE -  enabled
*                                      GT_FALSE - disbled
* @param[out] portNumPtr               - (pointer to) port number.
* @param[out] laneNumPtr               - (pointer to) selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, recoveryClkType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - on bad state of register
*/
GT_STATUS cpssDxChPortSyncEtherRecoveryClkConfigGet
(
    IN  GT_U8    devNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_TYPE_ENT  recoveryClkType,
    OUT GT_BOOL  *enablePtr,
    OUT GT_PHYSICAL_PORT_NUM    *portNumPtr,
    OUT GT_U32   *laneNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSyncEtherRecoveryClkConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, recoveryClkType, enablePtr, portNumPtr, laneNumPtr));

    rc = internal_cpssDxChPortSyncEtherRecoveryClkConfigGet(devNum, recoveryClkType, enablePtr, portNumPtr, laneNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, recoveryClkType, enablePtr, portNumPtr, laneNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortSyncEtherRecoveryClkDividerValueSetForSip6_10 function
* @endinternal
*
* @brief   Function sets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] value                    - recovery clock divider  to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
static GT_STATUS prvCpssDxChPortSyncEtherRecoveryClkDividerValueSetForSip6_10
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value
)
{
    GT_STATUS rc;           /* returned status */
    GT_U32  regValue = 0;   /* register value */
    GT_U32  regOffset;      /* register offset */
    GT_U32  regMask;        /* register mask */
    GT_U32  serdesIdx;      /* global SD index */
    CPSS_DXCH_ACTIVE_LANES_STC  portParams;

    rc = prvCpssDxChPortParamsGet(devNum,portNum,&portParams);
    if( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if( laneNum >= portParams.numActiveLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    serdesIdx = portParams.activeLaneList[laneNum];

    switch (clockSelect)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
            regOffset = 0;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
            regOffset = 4;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(value)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E:
            regValue = 0;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_E:
            regValue = 1;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_3_E:
            regValue = 2;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_4_E:
            regValue = 3;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_5_E:
            regValue = 4;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E:
            regValue = 5;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E:
            regValue = 6;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regMask = (0x7 << regOffset);
    regValue = (regValue << regOffset);
    /* <rcvrd_clk0_div> or <rcvrd_clk1_div> */
    rc = prvCpssGenPortGroupSerdesWriteRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                  serdesIdx,
                                                  SDW_GENERAL_CONTROL_3,
                                                  regValue, regMask);
    return rc;
}

/**
* @internal internal_cpssDxChPortSyncEtherRecoveryClkDividerValueSet function
* @endinternal
*
* @brief   Function sets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] value                    - recovery clock divider  to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
static GT_STATUS internal_cpssDxChPortSyncEtherRecoveryClkDividerValueSet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value
)
{
    GT_STATUS   rc;         /* return code      */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register data    */
    GT_U32      regOffset;  /* offset in register */
    GT_U32      portGroup;  /* port group */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */
    CPSS_DXCH_ACTIVE_LANES_STC  portParams;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);


    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_FE_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    portGroup = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    rc = prvCpssDxChPortParamsGet(devNum,portNum,&portParams);
    if( GT_OK != rc )
    {
        return rc;
    }

    if( laneNum >= portParams.numActiveLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortSyncEtherRecoveryClkDividerValueSetForSip6_10(
                            devNum,portNum,laneNum,clockSelect,value);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* Serdes External Configuration 2 Register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[laneNum]].serdesExternalReg2;

        switch (clockSelect)
        {
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
                regOffset = 12;
                break;
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
                regOffset = 8;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else /* Lion2 */
    {
        rc = prvCpssDxChPortSyncEtherPortLaneCheck(devNum, portNum, laneNum, &ifMode);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* Serdes External Configuration 1 Register */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[laneNum]].
                                                                serdesExternalReg2;

        switch(clockSelect)
        {
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
                regOffset = 12;
                break;
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
                regOffset = 8;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    switch(value)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E:    regValue = 0;
                                                                    break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_E:    regValue = 1;
                                                                    break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_3_E:    regValue = 2;
                                                                    break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_4_E:    regValue = 3;
                                                                    break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_5_E:    regValue = 4;
                                                                    break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E:    regValue = 5;
                                                                    break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E:

            if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||
               ((PRV_CPSS_SIP_5_CHECK_MAC(devNum)) && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))) ||
               (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            else
            {
                regValue = 6;
            }
            break;

        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_5_E:

            if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||
               ((PRV_CPSS_SIP_5_CHECK_MAC(devNum)) && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))) ||
               (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
            {
                regValue = 6;
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E:  regValue = 7;
                                                                    break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    /* <rcvrd_clk0_div> or <rcvrd_clk1_div> */
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroup, regAddr, regOffset, 3, regValue);

    return rc;
}

/**
* @internal cpssDxChPortSyncEtherRecoveryClkDividerValueSet function
* @endinternal
*
* @brief   Function sets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[in] value                    - recovery clock divider  to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
GT_STATUS cpssDxChPortSyncEtherRecoveryClkDividerValueSet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  value
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSyncEtherRecoveryClkDividerValueSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, clockSelect, value));

    rc = internal_cpssDxChPortSyncEtherRecoveryClkDividerValueSet(devNum, portNum, laneNum, clockSelect, value);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, clockSelect, value));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChPortSyncEtherRecoveryClkDividerValueGetForSip6_10 function
* @endinternal
*
* @brief   Function gets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
* @param[out] value                   - recovery clock divider  to set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
static GT_STATUS prvCpssDxChPortSyncEtherRecoveryClkDividerValueGetForSip6_10
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    OUT CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  *valuePtr
)
{
    GT_STATUS rc;           /* returned status */
    GT_U32  regValue = 0;   /* register value */
    GT_U32  regOffset;      /* register offset */
    GT_U32  regMask;        /* register mask */
    GT_U32  serdesIdx;      /* global SD index */
    CPSS_DXCH_ACTIVE_LANES_STC  portParams;

    rc = prvCpssDxChPortParamsGet(devNum,portNum,&portParams);
    if( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if( laneNum >= portParams.numActiveLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    serdesIdx = portParams.activeLaneList[laneNum];

    switch (clockSelect)
    {
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
            regOffset = 0;
            break;
        case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
            regOffset = 4;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regMask = (0x7 << regOffset);
    /* <rcvrd_clk0_div> or <rcvrd_clk1_div> */
    rc = prvCpssGenPortGroupSerdesReadRegBitMask(devNum, 0, 0,/*EXTERNAL_REG*/
                                                  serdesIdx,
                                                  SDW_GENERAL_CONTROL_3,
                                                  &regValue, regMask);


    switch(regValue >> regOffset)
    {
        case 0:
            *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E;
            break;
        case 1:
            *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_E;
            break;
        case 2:
            *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_3_E;
            break;
        case 3:
            *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_4_E;
            break;
        case 4:
            *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_5_E;
            break;
        case 5:
            *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E;
            break;
        case 6:
            *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortSyncEtherRecoveryClkDividerValueGet function
* @endinternal
*
* @brief   Function gets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] valuePtr                 - (pointer to) recovery clock divider value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
static GT_STATUS internal_cpssDxChPortSyncEtherRecoveryClkDividerValueGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    OUT CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  *valuePtr
)
{
    GT_STATUS   rc;         /* return code      */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register data    */
    GT_U32      regOffset;  /* offset in register */
    GT_U32      portGroup;  /* port group */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */
    CPSS_DXCH_ACTIVE_LANES_STC  portParams;
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);


    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_FE_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    portGroup = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    rc = prvCpssDxChPortParamsGet(devNum, portNum, &portParams);
    if (GT_OK != rc)
    {
        return rc;
    }

    if (laneNum >= portParams.numActiveLanes) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        return prvCpssDxChPortSyncEtherRecoveryClkDividerValueGetForSip6_10(
                            devNum,portNum,laneNum,clockSelect,valuePtr);
    }


    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* Serdes External Configuration 2 Register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[laneNum]].serdesExternalReg2;

        switch (clockSelect)
        {
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
                regOffset = 12;
                break;
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
                regOffset = 8;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    else /* Lion2 */
    {
        rc = prvCpssDxChPortSyncEtherPortLaneCheck(devNum, portNum, laneNum, &ifMode);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* Serdes External Configuration 1 Register */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[laneNum]].
                                                                serdesExternalReg2;

        switch(clockSelect)
        {
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
                regOffset = 12;
                break;
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
                regOffset = 8;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    /* <rcvrd_clk0_div> or <rcvrd_clk1_div> */
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroup, regAddr, regOffset,
                                            3, &regValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    switch(regValue)
    {
        case 0: *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_1_E;
                break;
        case 1: *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_E;
                break;
        case 2: *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_3_E;
                break;
        case 3: *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_4_E;
                break;
        case 4: *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_5_E;
                break;
        case 5: *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E;
                break;
        case 6: if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) ||
                   ((PRV_CPSS_SIP_5_CHECK_MAC(devNum)) && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum))) ||
                   (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
                {
                    *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_5_E;
                }
                else
                {
                    *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_16_E;
                }
                break;
        case 7: *valuePtr = CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_2_5_E;
                break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortSyncEtherRecoveryClkDividerValueGet function
* @endinternal
*
* @brief   Function gets recovery clock divider value.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] valuePtr                 - (pointer to) recovery clock divider value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
GT_STATUS cpssDxChPortSyncEtherRecoveryClkDividerValueGet
(
    IN  GT_U8                                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                                portNum,
    IN  GT_U32                                              laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    OUT CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_ENT  *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSyncEtherRecoveryClkDividerValueGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, clockSelect, valuePtr));

    rc = internal_cpssDxChPortSyncEtherRecoveryClkDividerValueGet(devNum, portNum, laneNum, clockSelect, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, clockSelect, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet function
* @endinternal
*
* @brief   Function sets Recovered Clock Automatic Masking enabling.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[in] enable                   - enable/disable Recovered Clock Automatic Masking
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
static GT_STATUS internal_cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    IN  GT_BOOL enable
)
{
    GT_STATUS   rc;         /* return code      */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register data    */
    GT_U32      regOffset = 0;  /* register offset  */
    GT_U32      portGroup;  /* port group */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */
    GT_U32      portMacNum;  /* MAC number */
    CPSS_DXCH_ACTIVE_LANES_STC  portParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_FE_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    portGroup = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    rc = prvCpssDxChPortParamsGet(devNum, portNum, &portParams);
    if (GT_OK != rc)
    {
        return rc;
    }

    if (laneNum >= portParams.numActiveLanes) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if( GT_OK != rc )
        {
            return rc;
        }

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            /* Serdes External Configuration 1 Register */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[laneNum]].serdesExternalReg2;

            switch (clockSelect)
            {
                case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
                    regOffset = 6;
                    break;
                case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
                    regOffset = 5;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            /* Serdes External Configuration 2 Register */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[laneNum]].serdesExternalReg3;

            switch (clockSelect)
            {
                case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
                    regOffset = 1;
                    break;
                case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
                    regOffset = 0;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }
    else /* Lion2 */
    {
        rc = prvCpssDxChPortSyncEtherPortLaneCheck(devNum, portNum, laneNum, &ifMode);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* Serdes External Configuration 2 Register */
        regAddr =
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[laneNum]].
                                                                serdesExternalReg3;

        switch(clockSelect)
        {
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
                regOffset = 1;
                break;
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
                regOffset = 0;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* set register value */
    regValue = BOOL2BIT_MAC(enable);

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    /* <rcvrd_clk0_auto_mask> or <rcvrd_clk1_auto_mask> */
    rc = prvCpssHwPpPortGroupSetRegField(devNum, portGroup, regAddr, regOffset, 1, regValue);

    return rc;
}

/**
* @internal cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet function
* @endinternal
*
* @brief   Function sets Recovered Clock Automatic Masking enabling.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
* @param[in] enable                   - enable/disable Recovered Clock Automatic Masking
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
GT_STATUS cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, clockSelect, enable));

    rc = internal_cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet(devNum, portNum, laneNum, clockSelect, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, clockSelect, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet function
* @endinternal
*
* @brief   Function gets Recovered Clock Automatic Masking status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Recovered Clock Automatic Masking state
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
static GT_STATUS internal_cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS   rc;         /* return code      */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register data    */
    GT_U32      regOffset = 0;  /* register offset  */
    GT_U32      portGroup;  /* port group */
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode; /* interface mode */
    GT_U32      portMacNum;  /* MAC number */
    CPSS_DXCH_ACTIVE_LANES_STC  portParams;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    PRV_CPSS_DXCH_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_DXCH_PORT_TYPE_MAC(devNum,portMacNum) == PRV_CPSS_PORT_FE_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);

    portGroup = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    rc = prvCpssDxChPortParamsGet(devNum, portNum, &portParams);
    if (GT_OK != rc)
    {
        return rc;
    }

    if (laneNum >= portParams.numActiveLanes) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) ||
       (PRV_CPSS_DXCH_IS_AC3_BASED_DEVICE_MAC(devNum)))
    {
        rc = cpssDxChPortInterfaceModeGet(devNum, portNum, &ifMode);
        if( GT_OK != rc )
        {
            return rc;
        }

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            /* Serdes External Configuration 2 Register */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[laneNum]].serdesExternalReg2;

            switch (clockSelect)
            {
                case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
                    regOffset = 6;
                    break;
                case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
                    regOffset = 5;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            /* Serdes External Configuration 2 Register */
            regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[laneNum]].serdesExternalReg3;

            switch (clockSelect)
            {
                case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
                    regOffset = 1;
                    break;
                case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
                    regOffset = 0;
                    break;
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }
    else /* Lion2 */
    {
        rc = prvCpssDxChPortSyncEtherPortLaneCheck(devNum, portNum, laneNum, &ifMode);
        if( GT_OK != rc )
        {
            return rc;
        }

        /* Serdes External Configuration 2 Register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->serdesConfig[portParams.activeLaneList[laneNum]].serdesExternalReg3;

        switch(clockSelect)
        {
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_1_E:
                regOffset = 1;
                break;
            case CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E:
                regOffset = 0;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    portGroup = PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum, portNum);
    /* <rcvrd_clk0_auto_mask> or <rcvrd_clk1_auto_mask>*/
    rc = prvCpssHwPpPortGroupGetRegField(devNum, portGroup, regAddr, regOffset, 1, &regValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* set return value */
    *enablePtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}

/**
* @internal cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet function
* @endinternal
*
* @brief   Function gets Recovered Clock Automatic Masking status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*                                      For xCat2 valid ports for configuration:
*                                      Giga Ports: 0,4,8,12,16,20
*                                      Stacking Ports: 24,25,26,27
* @param[in] laneNum                  - selected SERDES lane number within port.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3)
*                                      Ranges according to port interface:
*                                      SGMII/1000BaseX - 0
*                                      RXAUI/HX        - 0..1
*                                      XAUII           - 0..3
*                                      XLG             - 0..7
*                                      10GBASE-R       - 0
*                                      20GBASE-R2      - 0..1
*                                      XLAUI           - 0..3
* @param[in] clockSelect              - clock select - each SERDES may be connected.
*                                      (APPLICABLE DEVICES Lion2, Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
*
* @param[out] enablePtr                - (pointer to) Recovered Clock Automatic Masking state
*                                      GT_TRUE - enable
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong devNum, portNum, lane number or clockSelect
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_SUPPORTED         - on not supported port type
*/
GT_STATUS cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  GT_U32  laneNum,
    IN  CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_ENT clockSelect,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneNum, clockSelect, enablePtr));

    rc = internal_cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableGet(devNum, portNum, laneNum, clockSelect, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneNum, clockSelect, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

