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
* @file mvHwsMtiLowSpeedPcsRev2If.c
*
* @brief MTI Low Speed PCS interface API
*
* @version   1
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiLowSpeedPcsRev2If.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

static char* mvHwsMtiLowSpeedPcsRev2TypeGetFunc(void)
{
  return "MTI_LOW_SP_PCS";
}

/**
* @internal mvHwsMtiLowSpeedPcsRev2LoopBack function
* @endinternal
*
* @brief   Set PCS loopback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiLowSpeedPcsRev2LoopBack
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;
    MV_HWS_LPCS_UNITS_FIELDS_E pcsField;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = LPCS_UNITS_CONTROL_P0_LOOPBACK_E;
    data = (lbType == TX_2_RX_LB) ? 1 : 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_LOW_SP_PCS_UNIT, pcsField, data, NULL));

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        extField = PHOENIX_MTIP_EXT_UNITS_CONTROL1_P0_FORCE_LINK_OK_EN_E;
    }
    else
    {
        extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_P0_FORCE_LINK_OK_EN_E;
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}


/**
 * @internal mvHwsMtiLowSpeedPcsAutoNeg1GSgmii function
 * @endinternal
 *
 * @brief   Configure Auto-negotiation for SGMII/1000BaseX port
 *          modes.
 *
 *
 * @param devNum
 * @param phyPortNum
 * @param portMode
 * @param autoNegotiationPtr
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsMtiLowSpeedPcsAutoNeg1GSgmii
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portMacNum,
    IN  MV_HWS_PORT_STANDARD        portMode,
    IN  MV_HWS_PCS_AUTONEG_1G_SGMII *autoNegPtr
)
{
    GT_UREG_DATA    data;
    GT_BOOL enable = (autoNegPtr->inbandAnEnable) && !(autoNegPtr->byPassEnable);

        /*
         * 1.  For both 1000Base-X and SGMII, need to enable the AN by setting:
         *      /<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_CONTROL
         *      Bit[12] = 0x1.
         */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_CONTROL_P0_ANENABLE_E, enable, NULL));

    switch (portMode) {
    case SGMII:
    case QSGMII:
        /*
         * 2.  For SGMII, need to configure the
         *      link timer. Each with different value. Registers:
         *       /<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_LINK_TIMER_1
         *       /<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_LINK_TIMER_0
         *     For SGMII the standard requires 1.6ms, which means:
         *      Link_Timer_1[4:0] = 0x3.
         *      Link_Timer_0[15:0] = 0xd40.
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_LINK_TIMER_0_P0_TIMER0_E, 0, NULL));
        data = ((enable) ? (0xd40>>1) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_LINK_TIMER_0_P0_TIMER15_1_E, data, NULL));
        data = ((enable) ? (0x3) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_LINK_TIMER_1_P0_TIMER20_16_E, data, NULL));
        /*
         * 3.  For SGMII Only:
         *      We wish to let the HW set the speed automatically once AN is done.
         *      This is done by writing to following register:
         *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_IF_MODE
         *      Bit[1] = 0x1.
         *      In this case, bit[3:2] are don’t care.
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_IF_MODE_P0_USE_SGMII_AN_E, enable, NULL));
        /*
         * 4.  Configure device ability:
         *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
         *      Set bit[0] to 0x1 and set all other bits to 0.
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_DEV_ABILITY_P0_LD_ABILITY_RSV05_E, enable, NULL));
        break;

    case _1000Base_X:
       /*
         * 2.  For 1000Base-X the standard requires 10ms, which means:
         *      Link_Timer_1[4:0] = 0x13.
         *      Link_Timer_0[15:0] = 0x12d0. (bit[0] is part of the value although it is RO)
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_LINK_TIMER_0_P0_TIMER0_E, 0, NULL));
        data = ((enable) ? (0x12d0>>1) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_LINK_TIMER_0_P0_TIMER15_1_E, data, NULL));
        data = ((enable) ? (0x13) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_LINK_TIMER_1_P0_TIMER20_16_E, data, NULL));
        /*
         * 3.  Configure device ability:
         *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
         *      Since we support only Full-duplex, set bit[5] to 0x1 and bit[6] to 0x0.
         *      Set bit[7] 'PS1' - PAUSE and bit[8] 'PS2' - ASM_DIR
         *      Set all other bits to 0
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_DEV_ABILITY_P0_LD_FD_E, 1, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_DEV_ABILITY_P0_LD_HD_E, 0, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_DEV_ABILITY_P0_LD_PS1_E, ((enable) ? BOOL2BIT_MAC(autoNegPtr->flowCtrlPauseAdvertiseEnable) : (0x1)), NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_DEV_ABILITY_P0_LD_PS2_E, ((enable) ? BOOL2BIT_MAC(autoNegPtr->flowCtrlAsmAdvertiseEnable) : (0x1)), NULL));

        break;
    default:
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiLowSpeedPcsRev2Reset function
* @endinternal
*
* @brief   Set MTILowSpeed PCS RESET/UNRESET or FULL_RESET
*          action.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - RESET/UNRESET or FULL_RESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiLowSpeedPcsRev2Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    GT_U32 baseAddr, regAddr, unitIndex, unitNum, fieldStart, fieldLength;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

    GT_UNUSED_PARAM(portGroup);

    if ((POWER_DOWN == action) || (PARTIAL_POWER_DOWN == action))
    {
        CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_LOW_SP_PCS_UNIT, (portMacNum), &baseAddr, &unitIndex, &unitNum));

        /* I: m_RAL.lpcs_units_RegFile.GMODE.Lpcs_enable.set(lpcs_enable); */
        CHECK_STATUS(genUnitRegDbEntryGet(devNum, portMacNum, MTI_LOW_SP_PCS_UNIT, portMode, LPCS_UNITS_GMODE_LPCS_ENABLE_E, &fieldReg, &convertIdx));
        fieldLength = 1; /* CIDER field length is 8 - here we need to configure 1 port */
        fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
        regAddr = baseAddr + fieldReg.regOffset;
        /* need to reset in Power Down */
        CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, 0));

        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_IF_MODE_P0_SGMII_ENA_E, 0, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_IF_MODE_P0_USE_SGMII_AN_E, 0, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_IF_MODE_P0_SGMII_SPEED_E, 0, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_LOW_SP_PCS_UNIT, LPCS_UNITS_IF_MODE_P0_IFMODE_SEQ_ENA_E, 0, NULL));
    }
    else if (RESET == action)
    {
        /*do nothing*/
    }
    else if (UNRESET == action)
    {
        /*do nothing*/
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiLowSpeedPcsRev2LoopBackGet function
* @endinternal
*
* @brief   Get the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiLowSpeedPcsRev2LoopBackGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_UREG_DATA                data;
    MV_HWS_LPCS_UNITS_FIELDS_E  pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = LPCS_UNITS_CONTROL_P0_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_LOW_SP_PCS_UNIT, pcsField, &data, NULL));

    *lbType = (data != 0) ? TX_2_RX_LB : DISABLE_LB;
    return GT_OK;
}


/**
* @internal mvHwsMtiLowSpeedPcsRev2Mode function
* @endinternal
*
* @brief   Set the MtiLowSpeedPcs mode
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiLowSpeedPcsRev2Mode
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_LPCS_UNITS_FIELDS_E pcsField;
    GT_U32 sgmiiSpeed, sgmiiEnable;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    GT_U32 baseAddr, regAddr, unitIndex, unitNum, fieldStart, fieldLength;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_STATUS rc;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_LOW_SP_PCS_UNIT, (portMacNum), &baseAddr, &unitIndex, &unitNum));

    /* I: m_RAL.lpcs_units_RegFile.GMODE.Lpcs_enable.set(lpcs_enable); */
    CHECK_STATUS(genUnitRegDbEntryGet(devNum, portMacNum, MTI_LOW_SP_PCS_UNIT, portMode, LPCS_UNITS_GMODE_LPCS_ENABLE_E, &fieldReg, &convertIdx));
    fieldLength = 1; /* CIDER field length is 8 - here we need to configure 1 port */
    fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit;
    regAddr = baseAddr + fieldReg.regOffset;

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, 1));

    /*II:   m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Sgmii_ena.set(is_sgmii_en);     //Choses between 1000BASE-X and SGMII
            m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Use_sgmii_an.set(0);            //Currently not supporting AN
            m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Sgmii_speed.set(sgmii_speed);   //In case of SGMII, choose speed {10M,100M,1000M}
            m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_ifmode_tx_preamble_sync.set(preambleLength != PREAMBLE_8B);
            m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_ifmode_rx_preamble_sync.set(preambleLength != PREAMBLE_8B);*/

    switch (portMode)
    {
        case _1000Base_X:
        case _2500Base_X:
            sgmiiEnable = 0;
            break;

        case SGMII:
        case SGMII2_5:
            sgmiiEnable = 1;
            break;

        default:
            return GT_BAD_PARAM;
    }

    switch(attributesPtr->portSpeed)
    {
        case MV_HWS_PORT_SPEED_10M_E:
            sgmiiSpeed = 0;
            break;

        case MV_HWS_PORT_SPEED_100M_E:
            sgmiiSpeed = 1;
            break;

        case MV_HWS_PORT_SPEED_1G_E:
        case MV_HWS_PORT_SPEED_2_5G_E:
            sgmiiSpeed = 2;
            break;

        default:
            return GT_BAD_PARAM;
    }

    pcsField = LPCS_UNITS_IF_MODE_P0_SGMII_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_LOW_SP_PCS_UNIT, pcsField, sgmiiEnable, NULL));

    pcsField = LPCS_UNITS_IF_MODE_P0_USE_SGMII_AN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_LOW_SP_PCS_UNIT, pcsField, 0, NULL));

    pcsField = LPCS_UNITS_IF_MODE_P0_SGMII_SPEED_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_LOW_SP_PCS_UNIT, pcsField, sgmiiSpeed, NULL));

    if (portMode == _2500Base_X)
    {
        pcsField = LPCS_UNITS_IF_MODE_P0_IFMODE_SEQ_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_LOW_SP_PCS_UNIT, pcsField, 1, NULL));
    }

    /* III: m_RAL.lpcs_units_RegFile.p_CONTROL[portIndex].p_Anenable.set(0);
            m_RAL.lpcs_units_RegFile.p_CONTROL[portIndex].p_Reset.set(1);
            WaitFewCycles();
            m_RAL.lpcs_units_RegFile.p_CONTROL[portIndex].p_Reset.set(0)*/

    pcsField = LPCS_UNITS_CONTROL_P0_ANENABLE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_LOW_SP_PCS_UNIT, pcsField, 0, NULL));

#if 0
    pcsField = LPCS_UNITS_CONTROL_P0_RESET_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_LOW_SP_PCS_UNIT, pcsField, 1, NULL));
#endif

    /*
        LPCS CONTROL1.Reset is not used since:
            1.  It doesn't clean the TX buffers.
            2.  It affects neighbor ports when operating QSGMII/USGMII.

        PCS CONTROL1.Reset is used since:
            Until we configure to 'low rate' (1000BASE-X/SGMII/QSGMII,etc.) the TX buffer gets filled from the Base-R PCS.
            If we don't assert this reset in order to clean the buffer, we might have power up side effect (we saw link toggle in simulation).
            For QSGMII/USGMII need to assert this reset only for first port, though it should be harmless to do it for all ports.
    */

    pcsField = PCS25_UNITS_PORT_CONTROL1_PORT_RESET_E;
    rc = genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS25_UNIT, pcsField, 1, NULL);
    if(rc == GT_OK)
    {
        return rc;
    }
    pcsField = PCS50_UNITS_CONTROL1_P1_RESET_E;
    rc = genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS50_UNIT, pcsField, 1, NULL);
    if(rc == GT_OK)
    {
        return rc;
    }
    pcsField = PCS100_UNITS_CONTROL1_P0_RESET_E;
    rc = genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS100_UNIT, pcsField, 1, NULL);

    return rc;
}

/**
* @internal mvHwsMtiUsxLowSpeedPcsAlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiUsxPcs.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - port number
* @param[in] portMode                 - port mode
*
* @param[out] lock                     - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiLowSpeedPcsAlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    GT_UREG_DATA                  data;
    MV_HWS_PORT_INIT_PARAMS       curPortParams;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        extField = PHOENIX_MTIP_EXT_UNITS_STATUS_P0_LPCS_LINK_STATUS_E;
    }
    else
    {
        extField = HAWK_MTIP_EXT_BR_UNITS_STATUS_P0_LPCS_LINK_STATUS_E;
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_EXT_UNIT, extField, &data, NULL));

    *lockPtr = (data & 0x1);

    return GT_OK;
}

/**
* @internal mvHwsMtiLowSpeedPcsRev2IfInit function
* @endinternal
*
* @brief   Init MtiLowSpeedPcs configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiLowSpeedPcsRev2IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_PCS_LOW_SPEED])
    {
        funcPtrArray[MTI_PCS_LOW_SPEED] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_PCS_LOW_SPEED])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_PCS_LOW_SPEED], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_PCS_LOW_SPEED]->pcsResetFunc           = mvHwsMtiLowSpeedPcsRev2Reset;
    funcPtrArray[MTI_PCS_LOW_SPEED]->pcsModeCfgFunc         = mvHwsMtiLowSpeedPcsRev2Mode;
    funcPtrArray[MTI_PCS_LOW_SPEED]->pcsLbCfgFunc           = mvHwsMtiLowSpeedPcsRev2LoopBack;
    funcPtrArray[MTI_PCS_LOW_SPEED]->pcsLbCfgGetFunc        = mvHwsMtiLowSpeedPcsRev2LoopBackGet;
    funcPtrArray[MTI_PCS_LOW_SPEED]->pcsTypeGetFunc         = mvHwsMtiLowSpeedPcsRev2TypeGetFunc;
    funcPtrArray[MTI_PCS_LOW_SPEED]->pcsAutoNeg1GSgmiiFunc  = mvHwsMtiLowSpeedPcsAutoNeg1GSgmii;
    funcPtrArray[MTI_PCS_LOW_SPEED]->pcsAlignLockGetFunc    = mvHwsMtiLowSpeedPcsAlignLockGet;

    return GT_OK;
}
