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
* @file mvHwsMtiCpuPcsIf.c
*
* @brief MTI CPU PCS interface API
*
* @version   1
********************************************************************************
*/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiCpuPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

#if defined(HWS_DEBUG) ||!defined(RAVEN_DEV_SUPPORT)
static char* mvHwsMtiCpuPcsTypeGetFunc(void)
{
  return "MTI_CPU_PCS";
}
#endif

/**
* @internal mvHwsMtiCpuPcsReset function
* @endinternal
*
* @brief   Set CG PCS RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiCpuPcsReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    MV_MAC_PCS_CFG_SEQ_PTR              seq;
    GT_STATUS                   st;
    MV_HWS_MTI_CPU_PCS_SUB_SEQ  subSeq;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (POWER_DOWN == action)
    {
        subSeq = MTI_CPU_PCS_POWER_DOWN_SEQ;
    }
    else if (RESET == action)
    {
        subSeq = MTI_CPU_PCS_RESET_SEQ;
    }
    else if (UNRESET == action)
    {
        subSeq = MTI_CPU_PCS_UNRESET_SEQ;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }
    seq = &hwsMtiCpuPcsSeqDb[subSeq];
    st = mvCfgMacPcsSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))

    return GT_OK;
}
/**
* @internal mvHwsMtiCpuPcsMode function
* @endinternal
*
* @brief   Set the MtiCpuPcs mode
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

static GT_STATUS mvHwsMtiCpuPcsMode
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_MAC_PCS_CFG_SEQ_PTR              seq;
    GT_STATUS                   st;
    MV_HWS_MTI_CPU_PCS_SUB_SEQ  subSeq;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;

    attributesPtr = attributesPtr;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*portFecMode = curPortParams.portFecMode;*/

    switch (portMode)
    {
        case _10GBase_KR:
        case _10GBase_SR_LR:
            subSeq = MTI_CPU_PCS_XG_MODE_SEQ;
            break;
        case _25GBase_KR:
            subSeq = MTI_CPU_PCS_XLG_MODE_SEQ;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    seq = &hwsMtiCpuPcsSeqDb[subSeq];

    st = mvCfgMacPcsSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsMtiCpuPcsLoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
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
static GT_STATUS mvHwsMtiCpuPcsLoopBack
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_MAC_PCS_CFG_SEQ_PTR seq;
    GT_U32 baseAddr, unitIndex, unitNum;
    GT_U32 regAddr;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*
        In MAC/PCS loopback there is no link indication from MTI to MPF.
        Without link up indication the MPF drain all the packets that received from Eagle.
        As a result, we will need to force those links up indication in MPF interface
    */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MTI_CPU_MPFS_UNIT, phyPortNum, &baseAddr, &unitIndex, &unitNum));
    regAddr = baseAddr + MPFS_SAU_CONTROL;

    switch (lbType)
    {
        case DISABLE_LB:
            seq = &hwsMtiCpuPcsSeqDb[MTI_CPU_PCS_LPBK_NORMAL_SEQ];
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 30, 2, 0));
            break;
        case TX_2_RX_LB:
            seq = &hwsMtiCpuPcsSeqDb[MTI_CPU_PCS_LPBK_TX2RX_SEQ];
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 30, 2, 0x3));
            break;
        default:
            return GT_NOT_SUPPORTED;
    }
    CHECK_STATUS(mvCfgMacPcsSeqExec(devNum, portGroup, phyPortNum, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuPcsLoopBackGet function
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
static GT_STATUS mvHwsMtiCpuPcsLoopBackGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_CPU_PCS_UNIT, phyPortNum, MTI_PCS_CONTROL1, &data, (0x1 << 14)));

    switch(data>>14)
    {
        case 0:
            *lbType = DISABLE_LB;
            break;
        case 1:
            *lbType = TX_2_RX_LB;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}
#endif
/**
* @internal mvHwsMtiCpuPcsAlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiPcs100.
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
static GT_STATUS mvHwsMtiCpuPcsAlignLockGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *lock
)
{
    GT_UREG_DATA            data;
    GT_U32                  unitAddr, unitIndex, unitNum, address;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, curPortParams.portPcsNumber, &unitAddr, &unitIndex, &unitNum));

    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    address = MTIP_CPU_EXT_PORT_STATUS + unitAddr;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 2));
    *lock = (data != 0);

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsMtiCpuPcsSendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - pcs number
* @param[in  portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsSendFaultSet
(
    GT_U8                devNum,
    GT_U32               portGroup,
    GT_U32               pcsNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    GT_U32          unitAddr, unitIndex, unitNum, address;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, pcsNum, &unitAddr, &unitIndex,&unitNum));
    portMode = portMode;

    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    address = MTIP_CPU_EXT_PORT_CONTROL + unitAddr;

    if (send == GT_TRUE)
    {
        /*  start sending fault signals  */
        data = 0x20;
    }
    else
    {
        /*  stop sending fault signals */
        data = 0;
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, data, 0x20));

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuPcsFecConfigGet function
* @endinternal
*
* @brief   Return the FEC mode  status on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] portFecTypePtr           - pointer to fec mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsFecConfigGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    return  mvHwsMtipExtFecTypeGet(devNum, portGroup, phyPortNum, portMode, portFecTypePtr);
}

/**
* @internal mvHwsMtiCpuPcsCheckGearBox function
* @endinternal
*
* @brief   check if gear box is locked on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] lockPtr                 - pointer lock value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuPcsCheckGearBox
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_CPU_PCS_UNIT, curPortParams.portPcsNumber, MTI_PCS_BASER_STATUS1, &data, 0x1));
    if (data & 0x1)
    {
        *lockPtr = GT_TRUE;
    }
    else
    {
        *lockPtr = GT_FALSE;
    }
#ifdef ASIC_SIMULATION
    *lockPtr = GT_TRUE;
#endif

    return GT_OK;
}

/**
 * @internal mvHwsMtiCpuPcsAutoNeg1GSgmii function
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
static GT_STATUS mvHwsMtiCpuPcsAutoNeg1GSgmii
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      phyPortNum,
    IN  MV_HWS_PORT_STANDARD        portMode,
    IN  MV_HWS_PCS_AUTONEG_1G_SGMII *autoNegPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;
    GT_U32  unitAddr, unitIndex, unitNum, address;
    GT_U8 portGroup = 0;
    GT_BOOL enable = (autoNegPtr->inbandAnEnable) && !(autoNegPtr->byPassEnable);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_PCS_UNIT, phyPortNum, &unitAddr, &unitIndex,&unitNum));
    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

        /*
         * 1.  For both 1000Base-X and SGMII, need to enable the AN by setting:
         *      /Cider/EBU/Falcon/Falcon {Current}/Raven_Full/<Raven>Raven_A0/<Raven> Device Units/<GOP TAP 0>GOP TAP 0/<GOP TAP 0> <MTIP IP WRAPPER>MTIP IP WRAPPER/<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_CONTROL
         *      Bit[12] = 0x1.
         */
    address = MTI_LPCS_CONTROL + unitAddr;
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, (enable << 12), (0x1 << 12)));

    switch (portMode) {
    case SGMII:
    case QSGMII:
        /*
         * 2.  For SGMII, need to configure the
         *      link timer. Each with different value. Registers:
         *       /Cider/EBU/Falcon/Falcon {Current}/Raven_Full/<Raven>Raven_A0/<Raven> Device Units/<GOP TAP 0>GOP TAP 0/<GOP TAP 0> <MTIP IP WRAPPER>MTIP IP WRAPPER/<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_LINK_TIMER_1
         *       /Cider/EBU/Falcon/Falcon {Current}/Raven_Full/<Raven>Raven_A0/<Raven> Device Units/<GOP TAP 0>GOP TAP 0/<GOP TAP 0> <MTIP IP WRAPPER>MTIP IP WRAPPER/<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_LINK_TIMER_0
         *     For SGMII the standard requires 1.6ms, which means:
         *      Link_Timer_1[4:0] = 0x3.
         *      Link_Timer_0[15:0] = 0xd40.
         */
        address = MTI_LPCS_LINK_TIMER_1 + unitAddr;
        data = ((enable) ? (0x3) : (0x0));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, data, 0x1F));

        address = MTI_LPCS_LINK_TIMER_0 + unitAddr;
        data = ((enable) ? (0xd40) : (0x0));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, data, 0xFFFF));
        /*
         * 3.  For SGMII Only:
         *      We wish to let the HW set the speed automatically once AN is done.
         *      This is done by writing to following register:
         *      /Cider/EBU/Falcon/Falcon {Current}/Raven_Full/<Raven>Raven_A0/<Raven> Device Units/<GOP TAP 0>GOP TAP 0/<GOP TAP 0> <MTIP IP WRAPPER>MTIP IP WRAPPER/<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_IF_MODE
         *      Bit[1] = 0x1.
         *      In this case, bit[3:2] are don’t care.
         */
        address = MTI_LPCS_IF_MODE + unitAddr;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, (enable << 1), (0x1 << 1)));
        /*
         * 4.  Configure device ability:
         *      /Cider/EBU/Falcon/Falcon {Current}/Raven_Full/<Raven>Raven_A0/<Raven> Device Units/<GOP TAP 0>GOP TAP 0/<GOP TAP 0> <MTIP IP WRAPPER>MTIP IP WRAPPER/<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
         *      Set bit[0] to 0x1 and set all other bits to 0.
         */
        address = MTI_LPCS_DEV_ABILITY + unitAddr;
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, enable, (0xFFFF)));

        break;

    case _1000Base_X:
        /*
         * 2.  For 1000Base-X the standard requires 10ms, which means:
         *      Link_Timer_1[4:0] = 0x13.
         *      Link_Timer_0[15:0] = 0x12d0. (bit[0] is part of the value although it is RO)
         */
        address = MTI_LPCS_LINK_TIMER_1 + unitAddr;
        data = (enable) ? (0x13) : (0x0);
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, data, 0x1F));

        address = MTI_LPCS_LINK_TIMER_0 + unitAddr;
        data = (enable) ? (0x12d0) : (0x0);
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, data, 0xFFFF));
        /*
         * 3.  Configure device ability:
         *      /Cider/EBU/Falcon/Falcon {Current}/Raven_Full/<Raven>Raven_A0/<Raven> Device Units/<GOP TAP 0>GOP TAP 0/<GOP TAP 0> <MTIP IP WRAPPER>MTIP IP WRAPPER/<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
         *      Since we support only Full-duplex, set bit[5] to 0x1 and bit[6] to 0x0.
         *      Set bit[7] 'PS1' - PAUSE and bit[8] 'PS2' - ASM_DIR
         *      Set all other bits to 0
         */
        address = MTI_LPCS_DEV_ABILITY + unitAddr;
        data = (0x1 << 5) | (((enable) ? (BOOL2BIT_MAC(autoNegPtr->flowCtrlPauseAdvertiseEnable)) : 0x1) << 7) | (((enable) ? (BOOL2BIT_MAC(autoNegPtr->flowCtrlAsmAdvertiseEnable)) : 0x1) << 8);
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, data, (0xFFFF)));

        break;
    default:
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

#endif


/**
* @internal mvHwsMtiPcs100IfInit function
* @endinternal
*
* @brief   Init MTI CPU PCS configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiCpuPcsIfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_CPU_PCS])
    {
        funcPtrArray[MTI_CPU_PCS] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_CPU_PCS])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_CPU_PCS], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_CPU_PCS]->pcsResetFunc           = mvHwsMtiCpuPcsReset;
    funcPtrArray[MTI_CPU_PCS]->pcsModeCfgFunc         = mvHwsMtiCpuPcsMode;
    funcPtrArray[MTI_CPU_PCS]->pcsAlignLockGetFunc    = mvHwsMtiCpuPcsAlignLockGet;
#if defined(HWS_DEBUG) ||!defined(RAVEN_DEV_SUPPORT)
    funcPtrArray[MTI_CPU_PCS]->pcsTypeGetFunc         = mvHwsMtiCpuPcsTypeGetFunc;
#endif
#ifndef RAVEN_DEV_SUPPORT
    funcPtrArray[MTI_CPU_PCS]->pcsLbCfgFunc           = mvHwsMtiCpuPcsLoopBack;
    funcPtrArray[MTI_CPU_PCS]->pcsLbCfgGetFunc        = mvHwsMtiCpuPcsLoopBackGet;
    funcPtrArray[MTI_CPU_PCS]->pcsCheckGearBoxFunc    = mvHwsMtiCpuPcsCheckGearBox;
    funcPtrArray[MTI_CPU_PCS]->pcsFecCfgGetFunc       = mvHwsMtiCpuPcsFecConfigGet;
    funcPtrArray[MTI_CPU_PCS]->pcsSendFaultSetFunc    = mvHwsMtiCpuPcsSendFaultSet;
    funcPtrArray[MTI_CPU_PCS]->pcsAutoNeg1GSgmiiFunc  = mvHwsMtiCpuPcsAutoNeg1GSgmii;
#endif

    return GT_OK;
}
