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
* mvHwsMtiCpuSgPcsIf.c
*
* DESCRIPTION:
*       CGPCS interface API
*
* FILE REVISION NUMBER:
*       $Revision: 9 $
*
*******************************************************************************/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiCpuSgPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

static char* mvHwsMtiCpuSgPcsTypeGetFunc(void)
{
  return "SGPCS";
}

/**
* @internal mvHwsMtiCpuSgPcsMode function
* @endinternal
*
* @brief  Set the CPU SGPCS mode
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
GT_STATUS mvHwsMtiCpuSgPcsMode
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr

)
{
    GT_U32                     regValue = 0, sgmiiEnable = 0;
    MV_HWS_USXM_UNITS_FIELDS_E fieldName;

    GT_UNUSED_PARAM(attributesPtr);
    GT_UNUSED_PARAM(portGroup);

    fieldName = CPU_SGPCS_UNIT_PORT_VENDOR_CONTROL_PORT_SGPCS_ENA_R_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, fieldName, 0x1, NULL));

    /* configure low speed ports */
    switch(portMode)
    {
        case _1000Base_X:
        case _2500Base_X:
            sgmiiEnable = 0;
            regValue = 0x2;
            break;
        case SGMII:
        case SGMII2_5:
            sgmiiEnable = 1;
            regValue = 0x2;
            break;

        default:
            break;
    }

    fieldName = CPU_SGPCS_UNIT_PORT_IF_MODE_PORT_SGMII_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, fieldName, sgmiiEnable, NULL));

    fieldName = CPU_SGPCS_UNIT_PORT_IF_MODE_PORT_SGMII_SPEED_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, fieldName, regValue, NULL));

    fieldName = CPU_SGPCS_UNIT_PORT_IF_MODE_PORT_USE_SGMII_AN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, fieldName, 0x0, NULL));

    fieldName = CPU_SGPCS_UNIT_PORT_CONTROL_PORT_AN_ENABLE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, fieldName, 0x0, NULL));

    fieldName = CPU_SGPCS_UNIT_PORT_CONTROL_PORT_RESET_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, fieldName, 0x1, NULL));


    return GT_OK;
}

/**
* @internal mmvHwsMtiCpuSgPcsSpeedPcsReset function
* @endinternal
*
* @brief   Set CPU GS PCS RESET/UNRESET or FULL_RESET action.
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
GT_STATUS mvHwsMtiCpuSgPcsSpeedPcsReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_HWS_USXM_UNITS_FIELDS_E fieldName;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    if(POWER_DOWN == action)
    {
        fieldName = CPU_SGPCS_UNIT_PORT_VENDOR_CONTROL_PORT_SGPCS_ENA_R_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum,MTI_CPU_SGPCS_UNIT, fieldName, 0x0, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuSgPcsLoopBack function
* @endinternal
*
* @brief   Set CPU SGPCS loopback.
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
GT_STATUS mvHwsMtiCpuSgPcsLoopBack
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS             curPortParams;
    GT_UREG_DATA                        data;
    MV_HWS_CPU_PCS_UNITS_FIELDS_E       pcsField;
    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E  extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = CPU_SGPCS_UNIT_PORT_CONTROL_PORT_LOOPBACK_E;
    data = (lbType == TX_2_RX_LB) ? 1 : 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_SGPCS_UNIT, pcsField, data, NULL));

    extField = MTIP_CPU_EXT_UNITS_CONTROL_FORCE_LINK_OK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuSgPcsLoopBackGet function
* @endinternal
*
* @brief   Get the SGPCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[out] lbType                  - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuSgPcsLoopBackGet
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
    MV_HWS_CPU_PCS_UNITS_FIELDS_E pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = CPU_SGPCS_UNIT_PORT_CONTROL_PORT_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_SGPCS_UNIT, pcsField, &data, NULL));

    *lbType = (data != 0) ? TX_2_RX_LB : DISABLE_LB;

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
static GT_STATUS mvHwsMtiCpuSgPcsAutoNeg1GSgmii
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portMacNum,
    IN  MV_HWS_PORT_STANDARD        portMode,
    IN  MV_HWS_PCS_AUTONEG_1G_SGMII *autoNegPtr
)
{
    GT_UREG_DATA data;
    GT_BOOL enable = (autoNegPtr->inbandAnEnable) && !(autoNegPtr->byPassEnable);

        /*
         * 1.  For both 1000Base-X and SGMII, need to enable the AN by setting:
         *      /<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_CONTROL
         *      Bit[12] = 0x1.
         */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_CONTROL_PORT_AN_ENABLE_E, enable, NULL));

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
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_LINK_TIMER_0_PORT_TIMER0_E, 0, NULL));
        data = ((enable) ? (0xd40>>1) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_LINK_TIMER_0_PORT_TIMER15_1_E, data, NULL));
        data = ((enable) ? (0x3) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_LINK_TIMER_1_PORT_TIMER20_16_E, data, NULL));
        /*
         * 3.  For SGMII Only:
         *      We wish to let the HW set the speed automatically once AN is done.
         *      This is done by writing to following register:
         *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_IF_MODE
         *      Bit[1] = 0x1.
         *      In this case, bit[3:2] are don’t care.
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_IF_MODE_PORT_USE_SGMII_AN_E, enable, NULL));
        /*
         * 4.  Configure device ability:
         *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
         *      Set bit[0] to 0x1 and set all other bits to 0.
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_DEV_ABILITY_PORT_LD_ABILITY_RSV05_E, enable, NULL));

        break;

    case _1000Base_X:
       /*
         * 2.  For 1000Base-X the standard requires 10ms, which means:
         *      Link_Timer_1[4:0] = 0x13.
         *      Link_Timer_0[15:0] = 0x12d0. (bit[0] is part of the value although it is RO)
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_LINK_TIMER_0_PORT_TIMER0_E, 0, NULL));
        data = ((enable) ? (0x12d0>>1) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_LINK_TIMER_0_PORT_TIMER15_1_E, data, NULL));
        data = ((enable) ? (0x13) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_LINK_TIMER_1_PORT_TIMER20_16_E, data, NULL));
        /*
         * 3.  Configure device ability:
         *      /Cider/EBU/Falcon/Falcon {Current}/Raven_Full/<Raven>Raven_A0/<Raven> Device Units/<GOP TAP 0>GOP TAP 0/<GOP TAP 0> <MTIP IP WRAPPER>MTIP IP WRAPPER/<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
         *      Since we support only Full-duplex, set bit[5] to 0x1 and bit[6] to 0x0.
         *      Set bit[7] 'PS1' - PAUSE and bit[8] 'PS2' - ASM_DIR
         *      Set all other bits to 0
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_DEV_ABILITY_PORT_LD_FD_E, 1, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_DEV_ABILITY_PORT_LD_HD_E, 0, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_DEV_ABILITY_PORT_LD_PS1_E, ((enable) ? BOOL2BIT_MAC(autoNegPtr->flowCtrlPauseAdvertiseEnable) : (0x1)), NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_SGPCS_UNIT, CPU_SGPCS_UNIT_PORT_DEV_ABILITY_PORT_LD_PS2_E, ((enable) ? BOOL2BIT_MAC(autoNegPtr->flowCtrlAsmAdvertiseEnable) : (0x1)), NULL));

        break;
    default:
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuSgPcsAlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiCpuPcs.
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
static GT_STATUS mvHwsMtiCpuSgPcsAlignLockGet
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
    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E  extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* in CPU port there is a single link_status which is driven by a MUX and knows to take the link from Base-X when needed and from Base-R when needed.
       So we can look at it as "link_status || lpcs_link_status" */
    extField = MTIP_CPU_EXT_UNITS_STATUS_LINK_STATUS_E;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_CPU_EXT_UNIT, extField, &data, NULL));

    *lockPtr = (data & 0x1);
#ifdef ASIC_SIMULATION
    *lockPtr = GT_TRUE;
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuSgPcsIfInit function
* @endinternal
*
* @brief   Init CPU SGPCS configuration sequences and IF functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiCpuSgPcsIfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_CPU_SGPCS])
    {
        funcPtrArray[MTI_CPU_SGPCS] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_CPU_SGPCS])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_CPU_SGPCS], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }

    funcPtrArray[MTI_CPU_SGPCS]->pcsResetFunc           = mvHwsMtiCpuSgPcsSpeedPcsReset;
    funcPtrArray[MTI_CPU_SGPCS]->pcsModeCfgFunc         = mvHwsMtiCpuSgPcsMode;
    funcPtrArray[MTI_CPU_SGPCS]->pcsTypeGetFunc         = mvHwsMtiCpuSgPcsTypeGetFunc;
    funcPtrArray[MTI_CPU_SGPCS]->pcsLbCfgFunc           = mvHwsMtiCpuSgPcsLoopBack;
    funcPtrArray[MTI_CPU_SGPCS]->pcsLbCfgGetFunc        = mvHwsMtiCpuSgPcsLoopBackGet;
    funcPtrArray[MTI_CPU_SGPCS]->pcsAutoNeg1GSgmiiFunc  = mvHwsMtiCpuSgPcsAutoNeg1GSgmii;
    funcPtrArray[MTI_CPU_SGPCS]->pcsAlignLockGetFunc  = mvHwsMtiCpuSgPcsAlignLockGet;

    return GT_OK;
}
