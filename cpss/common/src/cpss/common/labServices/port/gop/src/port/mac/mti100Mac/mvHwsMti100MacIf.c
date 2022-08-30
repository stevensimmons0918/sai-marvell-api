/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsMti100MacIf.c
*
* DESCRIPTION: MTI100 1G-100G MAC
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacDb.h>

#if 0
#include <cpss/common/labServices/port/gop/port/silicon/falcon/mvHwsFalconPortIf.h>
#endif


static char* mvHwsMacTypeGet(void)
{
  return "MTI100_MAC";
}

/**
* @internal mvHwsMti100MacIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMti100MacIfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_MAC_100])
    {
        funcPtrArray[MTI_MAC_100] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
        if(!funcPtrArray[MTI_MAC_100])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_MAC_100], 0, sizeof(MV_HWS_MAC_FUNC_PTRS));
    }
    funcPtrArray[MTI_MAC_100]->macRestFunc    = mvHwsMti100MacReset;
    funcPtrArray[MTI_MAC_100]->macModeCfgFunc = mvHwsMti100MacModeCfg;
    funcPtrArray[MTI_MAC_100]->macLinkGetFunc = mvHwsMti100MacLinkStatus;
    funcPtrArray[MTI_MAC_100]->macLbCfgFunc   = mvHwsMti100MacLoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_MAC_100]->macLbStatusGetFunc = mvHwsMti100MacLoopbackStatusGet;
#endif
    funcPtrArray[MTI_MAC_100]->macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_MAC_100]->macPortEnableFunc = mvHwsMti100MacPortEnable;
    funcPtrArray[MTI_MAC_100]->macPortEnableGetFunc = mvHwsMti100MacPortEnableGet;

    return GT_OK;
}

/**
* @internal mvHwsMti100MacModeCfg function
* @endinternal
*
* @brief   Set the MTI100 MAC in GOP.
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
GT_STATUS mvHwsMti100MacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_FEC_MODE    portFecMode;
    MV_MAC_PCS_CFG_SEQ_PTR seq;
    GT_STATUS  st;

    attributesPtr = attributesPtr;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portFecMode = curPortParams.portFecMode;

    switch (curPortParams.portStandard)
    {
        case _1000Base_X:
        case _2500Base_X:
        case _5GBaseR:
        case _10GBase_KR:
        case _10GBase_SR_LR:
        case SGMII:
            seq = &hwsMti100MacSeqDb[HWS_MTI_MAC100_XG_MODE_SEQ];
            break;

        case _25GBase_KR:
        case _25GBase_CR:
        case _25GBase_SR:
        case _25GBase_KR_S:
        case _25GBase_CR_S:
        case _25GBase_KR_C:
        case _25GBase_CR_C:
            if (portFecMode == RS_FEC) {
                seq = &hwsMti100MacSeqDb[HWS_MTI_MAC100_XG_25RSFEC_MODE_SEQ];
            }
            else
            {
                seq = &hwsMti100MacSeqDb[HWS_MTI_MAC100_XG_MODE_SEQ];
            }
            break;

        case _40GBase_KR:
        case _50GBase_KR:
        case _50GBase_CR:
        case _50GBase_SR_LR:
            seq = &hwsMti100MacSeqDb[HWS_MTI_MAC100_XLG_MODE_SEQ];
            break;

        case _40GBase_KR4:
        case _40GBase_CR4:
        case _40GBase_SR_LR4:
        case _42GBase_KR4:
        case _40GBase_KR2:
        case _50GBase_CR2:
        case _50GBase_KR2_C:
        case _50GBase_CR2_C:
        case _50GBase_SR2:
        case _50GBase_KR2:
        case _53GBase_KR2:
        case _100GBase_KR2:
        case _102GBase_KR2:
        case _100GBase_CR2:
        case _100GBase_KR4:
        case _106GBase_KR4:
        case _100GBase_SR_LR2:
        case _100GBase_CR4:
        case _100GBase_SR4:
        case _48GBaseR4:
        case _48GBase_SR4:
            if (/* 106G, 53G */_27_34375G == curPortParams.serdesSpeed || /* 42G */_10_9375G == curPortParams.serdesSpeed)
                {
                  seq = &hwsMti100MacSeqDb[HWS_MTI_MAC100_XLG_NRZ_SPECIAL_SPEEDS_SEQ];
                }
                else
                {
                  seq = &hwsMti100MacSeqDb[HWS_MTI_MAC100_XLG_NRZ_MODE_SEQ];
                }
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

#if 0
    mvHwsCgMac28nmAccessLock(devNum, portMacNum);
#endif

    st = mvCfgMacPcsSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize);

#if 0
    mvHwsCgMac28nmAccessUnlock(devNum, portMacNum);
#endif

    /* Fine tuning for TX_FIFO_THRESHOLD for 100G_R2
        Port rate [Gb/s]    TH[entries]
              25                10
              50                10
              100               11
              200               7
              400               7
    */
    if(curPortParams.portStandard == _100GBase_KR2 || curPortParams.portStandard == _102GBase_KR2 || curPortParams.portStandard == _100GBase_CR2 || curPortParams.portStandard == _100GBase_SR_LR2)
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MTI_MAC100_UNIT, curPortParams.portMacNumber, MTI_MAC_TX_FIFO_SECTIONS, 0xB, 0xFFFF));
    }

    /*
        MTI_MAC_XIF_MODE:
            Onestepena = 1
            Pausetimerx8 = 1
            Xgmii = 0
    */
    if(curPortParams.portStandard == _1000Base_X || curPortParams.portStandard == _2500Base_X || curPortParams.portStandard == SGMII)
    {
        CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MTI_MAC100_UNIT, curPortParams.portMacNumber, MTI_MAC_XIF_MODE, 0x30, 0x31));
    }

    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"));

    return GT_OK;
}

/**
* @internal mvHwsMti100MacReset function
* @endinternal
*
* @brief   Set MTI100 MAC RESET/UNRESET or FULL_RESET action.
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
GT_STATUS mvHwsMti100MacReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_MAC_PCS_CFG_SEQ_PTR seq = NULL;
    GT_STATUS st;
    GT_U32  localMacIndex;
    GT_U32  unitAddr, unitIndex, unitNum, address;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, portMacNum, &unitAddr, &unitIndex,&unitNum));
    if (unitAddr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = MTIP_EXT_GLOBAL_RESET_CONTROL + unitAddr;
    localMacIndex = portMacNum % 8;

    if (POWER_DOWN == action)
    {
        switch(portMode)
        {
            case _1000Base_X:
            case _2500Base_X:
            case SGMII:
            case _5GBaseR:
            case _10GBase_KR:
            case _10GBase_SR_LR:
            case _25GBase_KR:
            case _25GBase_CR:
            case _25GBase_KR_C:
            case _25GBase_CR_C:
            case _25GBase_SR:
            case _25GBase_KR_S:
            case _25GBase_CR_S:
            case _40GBase_KR4:
            case _40GBase_CR4:
            case _40GBase_SR_LR4:
            case _42GBase_KR4:
            case _40GBase_KR:
            case _40GBase_KR2:
            case _50GBase_KR:
            case _50GBase_CR:
            case _50GBase_SR_LR:
            case _50GBase_KR2:
            case _50GBase_CR2:
            case _50GBase_KR2_C:
            case _50GBase_CR2_C:
            case _50GBase_SR2:
            case _53GBase_KR2:
            case _100GBase_KR2:
            case _102GBase_KR2:
            case _100GBase_CR2:
            case _100GBase_SR_LR2:
            case _100GBase_KR4:
            case _106GBase_KR4:
            case _100GBase_CR4:
            case _100GBase_SR4:
            case _48GBaseR4:
            case _48GBase_SR4:

                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (16 + localMacIndex), 1, 0));

                seq = &hwsMti100MacSeqDb[HWS_MTI_MAC100_XLG_POWER_DOWN_SEQ];
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "FULL_RESET not supported for portMode = %d \n", portMode);
        }
    }
    else if (RESET == action)
    {
        switch(portMode)
        {
            case _1000Base_X:
            case _2500Base_X:
            case SGMII:
            case _5GBaseR:
            case _10GBase_KR:
            case _10GBase_SR_LR:
            case _25GBase_KR:
            case _25GBase_KR_C:
            case _25GBase_CR:
            case _25GBase_CR_C:
            case _25GBase_SR:
            case _25GBase_KR_S:
            case _25GBase_CR_S:
            case _40GBase_KR4:
            case _40GBase_CR4:
            case _40GBase_SR_LR4:
            case _40GBase_KR:
            case _42GBase_KR4:
            case _40GBase_KR2:
            case _50GBase_KR:
            case _50GBase_CR:
            case _50GBase_SR_LR:
            case _50GBase_KR2:
            case _50GBase_CR2:
            case _50GBase_KR2_C:
            case _50GBase_CR2_C:
            case _50GBase_SR2:
            case _53GBase_KR2:
            case _100GBase_KR2:
            case _102GBase_KR2:
            case _100GBase_CR2:
            case _100GBase_SR_LR2:
            case _100GBase_KR4:
            case _106GBase_KR4:
            case _100GBase_CR4:
            case _100GBase_SR4:
            case _48GBaseR4:
            case _48GBase_SR4:

                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (16 + localMacIndex), 1, 0));
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "RESET not supported for portMode = %d \n", portMode);
        }
    }
    else if (UNRESET == action)
    {
        switch(portMode)
        {
            case _1000Base_X:
            case _2500Base_X:
            case SGMII:
            case _5GBaseR:
            case _10GBase_KR:
            case _10GBase_SR_LR:
            case _25GBase_KR:
            case _25GBase_KR_C:
            case _25GBase_CR:
            case _25GBase_CR_C:
            case _25GBase_SR:
            case _25GBase_KR_S:
            case _25GBase_CR_S:
            case _40GBase_KR4:
            case _40GBase_CR4:
            case _40GBase_SR_LR4:
            case _40GBase_KR:
            case _40GBase_KR2:
            case _42GBase_KR4:
            case _50GBase_KR:
            case _50GBase_CR:
            case _50GBase_SR_LR:
            case _50GBase_KR2:
            case _50GBase_CR2:
            case _50GBase_KR2_C:
            case _50GBase_CR2_C:
            case _50GBase_SR2:
            case _53GBase_KR2:
            case _100GBase_KR2:
            case _102GBase_KR2:
            case _100GBase_CR2:
            case _100GBase_SR_LR2:
            case _100GBase_KR4:
            case _106GBase_KR4:
            case _100GBase_CR4:
            case _100GBase_SR4:
            case _48GBaseR4:
            case _48GBase_SR4:

                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (16 + localMacIndex), 1, 1));

                seq = &hwsMti100MacSeqDb[HWS_MTI_MAC100_XLG_UNRESET_SEQ];
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "UNRESET not supported for portMode = %d \n", portMode);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    if(seq != NULL)
    {
        st = mvCfgMacPcsSeqExec(devNum, portGroup, portMacNum, seq->cfgSeq, seq->cfgSeqSize);
        CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"))
    }

    return GT_OK;
}

/**
* @internal mvHwsMti100MacLinkStatus function
* @endinternal
*
* @brief   Get MTI100 MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacLinkStatus
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL     *linkStatus
)
{
    GT_U32  data;
    GT_U32  unitAddr, unitIndex, unitNum, address;
    portMode = portMode;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));

    if (unitAddr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    address = MTIP_EXT_PORT_STATUS + (macNum %8) * 0x18 + unitAddr;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 1));

    *linkStatus = (data == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMti100MacPortEnable function
* @endinternal
*
* @brief   Perform port enable on the a port MAC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacPortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    GT_U32 data = (enable==GT_TRUE) ? 3 : 0;
    portMode = portMode;

    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MTI_MAC100_UNIT, macNum, MTI_MAC_COMMAND_CONFIG, data, 3));

    return GT_OK;
}

/**
* @internal mvHwsMti100MacPortEnableGet function
* @endinternal
*
* @brief   Get port enable status on the a port MAC..
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[out] enablePtr               - port enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacPortEnableGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *enablePtr
)
{
    GT_U32 data;
    portMode = portMode;

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_MAC100_UNIT, macNum, MTI_MAC_COMMAND_CONFIG, &data, 3));
    *enablePtr = (data) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}
/**
* @internal mvHwsMti100MacLoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacLoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_UREG_DATA            data;
    GT_U32                  unitAddr, unitIndex, unitNum, address;
    portMode = portMode;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_MAC100_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));

    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = MTI_MAC_COMMAND_CONFIG + unitAddr;
    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum,portGroup,address,6,1,&data));

    *lbType = (data == 1) ? RX_2_TX_LB : DISABLE_LB;
    return GT_OK;
}

/**
* @internal mvHwsMti100MacLoopbackSet function
* @endinternal
*
* @brief   Set MAC loopback .
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti100MacLoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    GT_UREG_DATA            data;
    GT_U32                  unitAddr, unitIndex, unitNum, address;
    portMode = portMode;

    switch (lbType)
    {
        case DISABLE_LB:
            data = 0;
            break;
        case RX_2_TX_LB:
            data = 1;
            break;
        default:
            return GT_FAIL;
    }

    /* MTIP_EXT_PORT_CONTROL register set */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));
    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = MTIP_EXT_PORT_CONTROL + (macNum % 8) * 0x18 + unitAddr;;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum,portGroup,address,7,1,data));

    /* MTI_MAC_COMMAND_CONFIG register set */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_MAC100_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));

    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = MTI_MAC_COMMAND_CONFIG + unitAddr;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum,portGroup,address,6,1,data));

    /* In order to disable regular traffic to continue to the next units in pipe (MPFS/MSDB/D2D and later to Eagle)
       it is needed to stop MPFS RX enable bit */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT, macNum, &unitAddr, &unitIndex, &unitNum));
    address = unitAddr + MPFS_PAU_CONTROL;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum,portGroup,address,0,1,(1-data)));

    return GT_OK;
}

