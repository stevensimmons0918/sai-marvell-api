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
* @file mvHwsMtiPcs100If.c
*
* @brief MTI PCS100 interface API
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
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs100If.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

#if defined(HWS_DEBUG) ||!defined(RAVEN_DEV_SUPPORT)
static char* mvHwsMtiPcs100TypeGetFunc(void)
{
  return "MTI_PCS100";
}
#endif

/**
* @internal mvHwsMtiPcs100Reset function
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
static GT_STATUS mvHwsMtiPcs100Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_MAC_PCS_CFG_SEQ_PTR          seq;
    GT_STATUS               st;
    MV_HWS_MTI_PCS_SUB_SEQ  subSeq;
    GT_U32                  pcsNum;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    if (POWER_DOWN == action)
    {
        if ((curPortParams.portFecMode != RS_FEC ) && (curPortParams.portFecMode != RS_FEC_544_514))
        {
            /* RS-FEC unit for these modes is in reset
               (with clock disable) and can not accessed */
            subSeq = MTI_PCS_POWER_DOWN_NO_RS_FEC_SEQ;
        }
        else
        {
            subSeq = MTI_PCS_POWER_DOWN_SEQ;
        }
    }
    else if (RESET == action)
    {
        subSeq = MTI_PCS_RESET_SEQ;
    }
    else if (UNRESET == action)
    {
        subSeq = MTI_PCS_UNRESET_SEQ;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }
    seq = &hwsMtiPcs100SeqDb[subSeq];
    st = mvCfgMacPcsSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs100Mode function
* @endinternal
*
* @brief   Set the MtiPcs100 mode
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
static GT_STATUS mvHwsMtiPcs100Mode
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_MAC_PCS_CFG_SEQ_PTR          seq;
    GT_STATUS               st;
    MV_HWS_MTI_PCS_SUB_SEQ  subSeq;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_FEC_MODE    portFecMode;

    attributesPtr = attributesPtr;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portFecMode = curPortParams.portFecMode;
    switch (portMode)
    {
        case _5GBaseR:
        case _10GBase_KR:
        case _10GBase_SR_LR:
            subSeq = MTI_PCS_XG_MODE_SEQ;
            break;
        case _25GBase_KR:
        case _25GBase_CR:
        case _25GBase_SR:
        case _25GBase_KR_S:
        case _25GBase_CR_S:
        case _25GBase_CR_C:
        case _25GBase_KR_C:
            if (portFecMode == RS_FEC) {
                subSeq = MTI_PCS_XG_25_RS_FEC_MODE_SEQ;
            }
            else
            {
                subSeq = MTI_PCS_XG_25_MODE_SEQ;
            }
            break;
        case _40GBase_KR4:
        case _40GBase_CR4:
        case _40GBase_SR_LR4:
        case _42GBase_KR4:
        case _48GBaseR4:
        case _48GBase_SR4:
            subSeq = MTI_PCS_XLG_MODE_SEQ;
            break;
        case _50GBase_CR:
        case _50GBase_KR:
        case _50GBase_SR_LR:
        case _40GBase_KR:
            subSeq = MTI_PCS_XLG_50R1_MODE_SEQ;
            break;
        case _40GBase_KR2:
        case _50GBase_KR2:
        case _50GBase_CR2:
        case _50GBase_SR2:
        case _50GBase_KR2_C:
        case _50GBase_CR2_C:
        case _53GBase_KR2:
            if (portFecMode == RS_FEC) {
                subSeq = MTI_PCS_XLG_50R2_RS_FEC_MODE_SEQ;
            }
            else
            {
                subSeq = MTI_PCS_XLG_MODE_SEQ;
            }
            break;
        case _100GBase_CR2:
        case _100GBase_KR2:
        case _102GBase_KR2:
        case _100GBase_SR_LR2:
        case _100GBase_KR4:
        case _106GBase_KR4:
        case _100GBase_CR4:
        case _100GBase_SR4:
            subSeq = MTI_PCS_CG_MODE_SEQ;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    seq = &hwsMtiPcs100SeqDb[subSeq];

    st = mvCfgMacPcsSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsMtiPcs100LoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*                                      pcsType   - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs100LoopBack
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_LB_TYPE     lbType
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
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, phyPortNum, &baseAddr, &unitIndex, &unitNum));
    regAddr = baseAddr + MPFS_SAU_CONTROL;

    switch (lbType)
    {
        case DISABLE_LB:
            seq = &hwsMtiPcs100SeqDb[MTI_PCS_LPBK_NORMAL_SEQ];
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 30, 2, 0));
            break;
        case TX_2_RX_LB:
#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3)) || defined (FALCON_DEV_SUPPORT)
            if(portMode == _5GBaseR || portMode == _10GBase_KR ||
               portMode == _10GBase_SR_LR || portMode == _25GBase_KR ||
               portMode == _25GBase_CR || portMode == _25GBase_SR)
            {
                seq = &hwsMtiPcs100SeqDb[MTI_PCS_LPBK_TX2RX_WA_SEQ];
            }
            else
#endif
            {
                seq = &hwsMtiPcs100SeqDb[MTI_PCS_LPBK_TX2RX_SEQ];
            }
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 30, 2, 0x3));
            break;
        default:
            return GT_NOT_SUPPORTED;
    }
    CHECK_STATUS(mvCfgMacPcsSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}
/**
* @internal mvHwsMtiPcs100LoopBackGet function
* @endinternal
*
* @brief   Get the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs100LoopBackGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS100_UNIT, curPortParams.portPcsNumber, MTI_PCS_CONTROL1, &data, (0x1 << 14)));

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
* @internal mvHwsMtiPcs100AlignLockGet function
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
static GT_STATUS mvHwsMtiPcs100AlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_UOPT                 portGroup,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lock
)
{
    GT_UREG_DATA            data;
    GT_U32                  unitAddr, unitIndex, unitNum, address;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, curPortParams.portPcsNumber, &unitAddr, &unitIndex, &unitNum));

    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    address = MTIP_EXT_PORT_STATUS + (curPortParams.portPcsNumber % 8) * 0x18 + unitAddr;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 2));

    *lock = (data != 0);
#ifdef ASIC_SIMULATION
    *lock = GT_TRUE;
#endif

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsMtiPcs100SendFaultSet function
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
static GT_STATUS mvHwsMtiPcs100SendFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    GT_U32          unitAddr, unitIndex, unitNum, address;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, pcsNum, &unitAddr, &unitIndex,&unitNum));
    portMode = portMode;

    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    address = MTIP_EXT_PORT_CONTROL + (pcsNum %8) * 0x18 + unitAddr;

    if (send == GT_TRUE)
    {
        /*  start sending fault signals  */
        data = 2;
    }
    else
    {
        /*  stop sending fault signals */
        data = 0;
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, data, 2));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs100SendLocalFaultSet function
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
static GT_STATUS mvHwsMtiPcs100SendLocalFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    GT_U32          unitAddr, unitIndex, unitNum, address;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, pcsNum, &unitAddr, &unitIndex,&unitNum));
    portMode = portMode;

    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    address = MTIP_EXT_PORT_CONTROL + (pcsNum %8) * 0x18 + unitAddr;

    if (send == GT_TRUE)
    {
        /*  start sending fault signals  */
        data = 1;
    }
    else
    {
        /*  stop sending fault signals */
        data = 0;
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, data, 1));

    return GT_OK;
}
/**
* @internal mvHwsMtiPcs100FecConfigGet function
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
static GT_STATUS mvHwsMtiPcs100FecConfigGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    return  mvHwsMtipExtFecTypeGet(devNum, portGroup, phyPortNum, portMode, portFecTypePtr);
}

/**
* @internal mvHwsMtiPcs100CheckGearBox function
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
static GT_STATUS mvHwsMtiPcs100CheckGearBox
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

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS100_UNIT, curPortParams.portPcsNumber, MTI_PCS_BASER_STATUS1, &data, 0x1));
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
#endif
/**
* @internal mvHwsMtiPcs100IfInit function
* @endinternal
*
* @brief   Init MtiPcs100 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs100IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_PCS_100])
    {
        funcPtrArray[MTI_PCS_100] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_PCS_100])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_PCS_100], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_PCS_100]->pcsResetFunc     = mvHwsMtiPcs100Reset;
    funcPtrArray[MTI_PCS_100]->pcsModeCfgFunc   = mvHwsMtiPcs100Mode;
    funcPtrArray[MTI_PCS_100]->pcsAlignLockGetFunc = mvHwsMtiPcs100AlignLockGet;
#if defined(HWS_DEBUG) ||!defined(RAVEN_DEV_SUPPORT)
    funcPtrArray[MTI_PCS_100]->pcsTypeGetFunc   = mvHwsMtiPcs100TypeGetFunc;
#endif
#ifndef RAVEN_DEV_SUPPORT
    funcPtrArray[MTI_PCS_100]->pcsLbCfgFunc     = mvHwsMtiPcs100LoopBack;
    funcPtrArray[MTI_PCS_100]->pcsLbCfgGetFunc  = mvHwsMtiPcs100LoopBackGet;
    funcPtrArray[MTI_PCS_100]->pcsCheckGearBoxFunc = mvHwsMtiPcs100CheckGearBox;
    funcPtrArray[MTI_PCS_100]->pcsFecCfgGetFunc = mvHwsMtiPcs100FecConfigGet;
    funcPtrArray[MTI_PCS_100]->pcsSendFaultSetFunc = mvHwsMtiPcs100SendFaultSet;
    funcPtrArray[MTI_PCS_100]->pcsSendLocalFaultSetFunc = mvHwsMtiPcs100SendLocalFaultSet;
#endif
    return GT_OK;
}


