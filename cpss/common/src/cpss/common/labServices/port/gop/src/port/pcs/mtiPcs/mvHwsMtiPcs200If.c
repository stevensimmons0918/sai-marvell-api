/******************************************************************************
*              Copyright (c) Marvell International Ltd. and its affiliates
*
* This software file (the "File") is owned and distributed by Marvell
* International Ltd. and/or its affiliates ("Marvell") under the following
* alternative licensing terms.
* If you received this File from Marvell, you may opt to use, redistribute
* and/or modify this File under the following licensing terms.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*  -   Redistributions of source code must retain the above copyright notice,
*       this list of conditions and the following disclaimer.
*  -   Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*  -    Neither the name of Marvell nor the names of its contributors may be
*       used to endorse or promote products derived from this software without
*       specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************
*/
/**
********************************************************************************
* @file mvHwsMtiPcs200If.c
*
* @brief MTI PCS200 interface API
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
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs200If.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

#if defined(HWS_DEBUG) ||!defined(RAVEN_DEV_SUPPORT)
static char* mvHwsMtiPcs200TypeGetFunc(void)
{
  return "MTI_PCS200";
}
#endif

/**
* @internal mvHwsMtiPcs200Reset function
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
static GT_STATUS mvHwsMtiPcs200Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_MAC_PCS_CFG_SEQ_PTR          seq = NULL;
    MV_MAC_PCS_CFG_SEQ_PTR          seq1 = NULL;
    GT_STATUS               st;
    GT_U32                  pcsNum;
    GT_U32                  activePcsNum;
    GT_U32                  quadModes;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsNum = curPortParams.portPcsNumber;

    if (POWER_DOWN == action)
    {
        /*
            check if another 200G port exists (port 0) -
            if yes - power down seq should not be called.
        */

        /* read {gc_pcs400_ena_in} field */
        CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_EXT_UNIT, pcsNum, MTIP_EXT_GLOBAL_CHANNEL_CONTROL, &quadModes, (0x3 << 6)));
        quadModes = (quadModes >> 6) & 0x3;
        if((quadModes & 0x1) == 0)
        {
            seq = &hwsMtiPcs200SeqDb[MTI_PCS_200G_R4_POWER_DOWN_SEQ];
        }
        activePcsNum = pcsNum & 0xFFFFFFF8;
        seq1 = &hwsMtiPcs200SeqDb[MTI_PCS_RESET_SEQ];
    }
    else if (RESET == action)
    {
        seq = &hwsMtiPcs200SeqDb[MTI_PCS_RESET_SEQ];
        activePcsNum = pcsNum;
    }
    else if (UNRESET == action)
    {
        seq = &hwsMtiPcs200SeqDb[MTI_PCS_UNRESET_SEQ];
        activePcsNum = pcsNum;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }


    /* power down seq is called for PCS400 index 0,8,16 ... */
    if(seq != NULL)
    {
        st = mvCfgMacPcsSeqExec(devNum, portGroup, activePcsNum, seq->cfgSeq, seq->cfgSeqSize);
        CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))
    }

    /* reset seq is called for PCS200 index 4,12,20 ... */
    if(seq1 != NULL)
    {
        st = mvCfgMacPcsSeqExec(devNum, portGroup, pcsNum, seq1->cfgSeq, seq1->cfgSeqSize);
        CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))
    }

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs200Mode function
* @endinternal
*
* @brief   Set the MtiPcs200 mode
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
static GT_STATUS mvHwsMtiPcs200Mode
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

    attributesPtr = attributesPtr;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (portMode)
    {
        case _200GBase_CR4:
        case _200GBase_KR4:
        case _200GBase_SR_LR4:
            subSeq = MTI_PCS_200R4_MODE_SEQ;
             break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    seq = &hwsMtiPcs200SeqDb[subSeq];

    /**********************************************************************
     **********************************************************************
        200G_BaseR4 on offset 4 port mode defined via PCS400 unit on
        offset 0, so PCS index should be 0,8,16...
     **********************************************************************
     **********************************************************************
     */
    st = mvCfgMacPcsSeqExec(devNum, portGroup, (curPortParams.portPcsNumber & 0xFFFFFFF8), seq->cfgSeq, seq->cfgSeqSize);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))

    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsMtiPcs200LoopBack function
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
static GT_STATUS mvHwsMtiPcs200LoopBack
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
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT,( phyPortNum & 0xfffffff8), &baseAddr, &unitIndex, &unitNum));
    if (phyPortNum%8 == 0) { /*in seg mode port 0 is taken from mpf 8 */
        baseAddr += 8*HWS_MPF_CHANNEL_OFFSET;
    }
    else /*in seg mode port 4 is taken from mpf 9 */
    {
        baseAddr += 9*HWS_MPF_CHANNEL_OFFSET;
    }
    regAddr = baseAddr + MPFS_SAU_CONTROL;

    switch (lbType)
    {
        case DISABLE_LB:
            seq = &hwsMtiPcs200SeqDb[MTI_PCS_LPBK_NORMAL_SEQ];
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 30, 2, 0));
            break;
        case TX_2_RX_LB:
            seq = &hwsMtiPcs200SeqDb[MTI_PCS_LPBK_TX2RX_SEQ];
            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, 30, 2, 0x3));
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvCfgMacPcsSeqExec(devNum, portGroup, curPortParams.portPcsNumber, seq->cfgSeq, seq->cfgSeqSize));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs200LoopBackGet function
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
static GT_STATUS mvHwsMtiPcs200LoopBackGet
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

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_PCS200_UNIT, curPortParams.portPcsNumber, MTI_PCS_CONTROL1, &data, (0x1 << 14)));

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
* @internal mvHwsMtiPcs200AlignLockGet function
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
static GT_STATUS mvHwsMtiPcs200AlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_UOPT                 portGroup,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lock
)
{
    GT_UREG_DATA              data;
    GT_U32                    unitAddr, unitIndex, unitNum, address;
    MV_HWS_PORT_INIT_PARAMS   curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, curPortParams.portPcsNumber, &unitAddr, &unitIndex,&unitNum));

    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    address = MTIP_EXT_SEG_PORT_STATUS + ((curPortParams.portPcsNumber % 8)/4) * 0x14 + unitAddr;
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 2));
    *lock = (data != 0);
#ifdef ASIC_SIMULATION
    *lock = GT_TRUE;
#endif
    return GT_OK;
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsMtiPcs200SendFaultSet function
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
static GT_STATUS mvHwsMtiPcs200SendFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    MV_MAC_PCS_CFG_SEQ_PTR seq;

    portMode = portMode;
    if (send==GT_TRUE)
    {
        /*  start sending fault signals  */
        seq = &hwsMtiPcs200SeqDb[MTI_PCS_START_SEND_FAULT_SEQ];
        CHECK_STATUS_EXT(mvCfgMacPcsSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize), LOG_ARG_PCS_IDX_MAC(pcsNum));
    }
    else
    {
        /*  stop sending fault signals */
        seq = &hwsMtiPcs200SeqDb[MTI_PCS_STOP_SEND_FAULT_SEQ];
        CHECK_STATUS_EXT(mvCfgMacPcsSeqExec(devNum, portGroup, pcsNum, seq->cfgSeq, seq->cfgSeqSize), LOG_ARG_PCS_IDX_MAC(pcsNum));
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs200FecConfigGet function
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
static GT_STATUS mvHwsMtiPcs200FecConfigGet
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
#endif

/**
* @internal mvHwsMtiPcs200IfInit function
* @endinternal
*
* @brief   Init MtiPcs200 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs200IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_PCS_200])
    {
        funcPtrArray[MTI_PCS_200] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_PCS_200])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_PCS_200], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_PCS_200]->pcsResetFunc     = mvHwsMtiPcs200Reset;
    funcPtrArray[MTI_PCS_200]->pcsModeCfgFunc   = mvHwsMtiPcs200Mode;
    funcPtrArray[MTI_PCS_200]->pcsAlignLockGetFunc = mvHwsMtiPcs200AlignLockGet;
#if defined(HWS_DEBUG) ||!defined(RAVEN_DEV_SUPPORT)
    funcPtrArray[MTI_PCS_200]->pcsTypeGetFunc   = mvHwsMtiPcs200TypeGetFunc;
#endif
#ifndef RAVEN_DEV_SUPPORT
    funcPtrArray[MTI_PCS_200]->pcsLbCfgFunc     = mvHwsMtiPcs200LoopBack;
    funcPtrArray[MTI_PCS_200]->pcsLbCfgGetFunc  = mvHwsMtiPcs200LoopBackGet;
    funcPtrArray[MTI_PCS_200]->pcsFecCfgGetFunc = mvHwsMtiPcs200FecConfigGet;
    funcPtrArray[MTI_PCS_200]->pcsSendFaultSetFunc = mvHwsMtiPcs200SendFaultSet;
#endif
    return GT_OK;
}

