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
* mvHwsMtiCpuMacIf.c
*
* DESCRIPTION: MTI CPU 10G MAC
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
#include <cpss/common/labServices/port/gop/port/mac/mtiCpuMac/mvHwsMtiCpuMacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mtiCpuMac/mvHwsMtiCpuMacDb.h>



static char* mvHwsMacTypeGet(void)
{
  return "MTI_CPU_MAC";
}



/**
* @internal mvHwsMtiCpuMacModeCfg function
* @endinternal
*
* @brief   Set the MTI CPU MAC in GOP.
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
static GT_STATUS mvHwsMtiCpuMacModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_MAC_PCS_CFG_SEQ_PTR seq;
    GT_STATUS  st;

    attributesPtr = attributesPtr;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (curPortParams.portStandard)
    {
        case _10GBase_KR:
        case _10GBase_SR_LR:
        case _25GBase_KR:
            seq = &hwsMtiCpuMacSeqDb[MTI_CPU_MAC_XLG_MODE_SEQ];
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    st = mvCfgMacPcsSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"));

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacReset function
* @endinternal
*
* @brief   Set MTI CPU MAC RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiCpuMacReset
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
    GT_U32  unitAddr, unitIndex, unitNum, address;

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, portMacNum, &unitAddr, &unitIndex,&unitNum));
    if (unitAddr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = MTIP_CPU_EXT_PORT_RESET + unitAddr;

    if (POWER_DOWN == action)
    {
        switch(portMode)
        {
            case _10GBase_KR:
            case _10GBase_SR_LR:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, 2, 1, 0));
                seq = &hwsMtiCpuMacSeqDb[MTI_CPU_MAC_XLG_POWER_DOWN_SEQ];
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "FULL_RESET not supported for portMode = %d \n", portMode);
        }
    }
    else if (RESET == action)
    {
        switch(portMode)
        {
            case _10GBase_SR_LR:
            case _10GBase_KR:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, 2, 1, 0));
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "RESET not supported for portMode = %d \n", portMode);
        }
    }
    else if (UNRESET == action)
    {
        switch(portMode)
        {
            case _10GBase_SR_LR:
            case _10GBase_KR:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, 2, 1, 1));
                seq = &hwsMtiCpuMacSeqDb[MTI_CPU_MAC_XLG_UNRESET_SEQ];

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
* @internal mvHwsMtiCpuMacLinkStatus function
* @endinternal
*
* @brief   Get MTI CPU MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuMacLinkStatus
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

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));

    if (unitAddr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    address = MTIP_CPU_EXT_PORT_STATUS + unitAddr;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 1));

    *linkStatus = (data == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacPortEnable function
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
static GT_STATUS mvHwsMtiCpuMacPortEnable
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

    CHECK_STATUS(genUnitRegisterSet(devNum,
                                    portGroup,
                                    MTI_CPU_MAC_UNIT,
                                    macNum,
                                    MTI_MAC_COMMAND_CONFIG /* same address for regular and CPU port*/,
                                    data,
                                    3));

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacPortEnableGet function
* @endinternal
*
* @brief   Get port enable status on the a port MAC.
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
static GT_STATUS mvHwsMtiCpuMacPortEnableGet
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

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_CPU_MAC_UNIT, macNum, MTI_MAC_COMMAND_CONFIG, &data, 3));
    *enablePtr = (data) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacLoopbackStatusGet function
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
static GT_STATUS mvHwsMtiCpuMacLoopbackStatusGet
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

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_MAC_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));

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
* @internal mvHwsMtiCpuMacLoopbackSet function
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
static GT_STATUS mvHwsMtiCpuMacLoopbackSet
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
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_EXT_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));
    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = MTIP_CPU_EXT_PORT_CONTROL + unitAddr;;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum,portGroup,address,14,1,data));

    /* MTI_MAC_COMMAND_CONFIG register set */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_CPU_MAC_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));

    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = MTI_MAC_COMMAND_CONFIG + unitAddr;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum,portGroup,address,6,1,data));

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMtiCpuMacIfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_CPU_MAC])
    {
        funcPtrArray[MTI_CPU_MAC] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
        if(!funcPtrArray[MTI_CPU_MAC])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_CPU_MAC], 0, sizeof(MV_HWS_MAC_FUNC_PTRS));
    }
    funcPtrArray[MTI_CPU_MAC]->macRestFunc    = mvHwsMtiCpuMacReset;
    funcPtrArray[MTI_CPU_MAC]->macModeCfgFunc = mvHwsMtiCpuMacModeCfg;
    funcPtrArray[MTI_CPU_MAC]->macLinkGetFunc = mvHwsMtiCpuMacLinkStatus;
    funcPtrArray[MTI_CPU_MAC]->macLbCfgFunc   = mvHwsMtiCpuMacLoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_CPU_MAC]->macLbStatusGetFunc = mvHwsMtiCpuMacLoopbackStatusGet;
#endif
    funcPtrArray[MTI_CPU_MAC]->macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_CPU_MAC]->macPortEnableFunc = mvHwsMtiCpuMacPortEnable;
    funcPtrArray[MTI_CPU_MAC]->macPortEnableGetFunc = mvHwsMtiCpuMacPortEnableGet;

    return GT_OK;
}
