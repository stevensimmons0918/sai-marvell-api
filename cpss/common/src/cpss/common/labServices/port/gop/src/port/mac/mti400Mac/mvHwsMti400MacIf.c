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
* mvHwsMti400MacIf.c
*
* DESCRIPTION: MTI 400G MAC
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
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacDb.h>

#if 0
#include <cpss/common/labServices/port/gop/port/silicon/falcon/mvHwsFalconPortIf.h>
#endif


static char* mvHwsMacTypeGet(void)
{
  return "MTI400_MAC";
}

/**
* @internal mvHwsMti400MacIfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMti400MacIfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_MAC_400])
    {
        funcPtrArray[MTI_MAC_400] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
        if(!funcPtrArray[MTI_MAC_400])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_MAC_400], 0, sizeof(MV_HWS_MAC_FUNC_PTRS));
    }
    funcPtrArray[MTI_MAC_400]->macRestFunc    = mvHwsMti400MacReset;
    funcPtrArray[MTI_MAC_400]->macModeCfgFunc = mvHwsMti400MacModeCfg;
    funcPtrArray[MTI_MAC_400]->macLinkGetFunc = mvHwsMti400MacLinkStatus;
    funcPtrArray[MTI_MAC_400]->macLbCfgFunc   = mvHwsMti400MacLoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_MAC_400]->macLbStatusGetFunc = mvHwsMti400MacLoopbackStatusGet;
#endif
    funcPtrArray[MTI_MAC_400]->macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_MAC_400]->macPortEnableFunc = mvHwsMti400MacPortEnable;
    funcPtrArray[MTI_MAC_400]->macPortEnableGetFunc = mvHwsMti400MacPortEnableGet;

    return GT_OK;
}

/**
* @internal mvHwsMti400MacModeCfg function
* @endinternal
*
* @brief   Set the MTI400 MAC in GOP.
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
GT_STATUS mvHwsMti400MacModeCfg
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
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
        case _200GBase_CR4:
        case _200GBase_KR4:
        case _200GBase_KR8:
        case _200GBase_CR8:
        case _400GBase_KR8:
        case _400GBase_CR8:
        case _200GBase_SR_LR4:
        case _200GBase_SR_LR8:
        case _400GBase_SR_LR8:
        case _424GBase_KR8:
            seq = &hwsMti400MacSeqDb[HWS_MTI_MAC400_SEGMENTED_4_LANES_MODE_SEQ];
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    st = mvCfgMacPcsSeqExec(devNum, portGroup, curPortParams.portMacNumber, seq->cfgSeq, seq->cfgSeqSize);
    CHECK_STATUS_EXT(st, LOG_ARG_MAC_IDX_MAC(portMacNum), LOG_ARG_STRING_MAC("calling mvCfgSeqExec"));

    return GT_OK;
}

/**
* @internal mvHwsMti400MacReset function
* @endinternal
*
* @brief   Set MTI400 MAC RESET/UNRESET or FULL_RESET action.
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
GT_STATUS mvHwsMti400MacReset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
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
    localMacIndex = localMacIndex / 4;  /* bit_24 for 200G_KR4 on port 0, bit_25 for 200G_KR4 on port 4 */

    if (POWER_DOWN == action)
    {
        switch(portMode)
        {
            case _200GBase_CR4:
            case _200GBase_KR4:
            case _200GBase_SR_LR4:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (24+localMacIndex), 1, 0));
                seq = &hwsMti400MacSeqDb[HWS_MTI_MAC400_SEGMENTED_4_LANES_POWER_DOWN_SEQ];
                break;

            case _400GBase_CR8:
            case _400GBase_KR8:
            case _400GBase_SR_LR8:
            case _424GBase_KR8:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, 24, 1, 0));
                seq = &hwsMti400MacSeqDb[HWS_MTI_MAC400_SEGMENTED_8_LANES_POWER_DOWN_SEQ];
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "FULL_RESET not supported for portMode = %d \n", portMode);
        }
    }
    else if (RESET == action)
    {
        switch(portMode)
        {
            case _200GBase_CR4:
            case _200GBase_KR4:
            case _400GBase_KR8:
            case _400GBase_CR8:
            case _200GBase_SR_LR4:
            case _400GBase_SR_LR8:
            case _424GBase_KR8:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (24 + localMacIndex), 1, 0));
                break;

            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "RESET not supported for portMode = %d \n", portMode);
        }
    }
    else if (UNRESET == action)
    {
        switch(portMode)
        {
            case _200GBase_CR4:
            case _200GBase_KR4:
            case _200GBase_SR_LR4:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, (24 + localMacIndex), 1, 1));
                seq = &hwsMti400MacSeqDb[HWS_MTI_MAC400_SEGMENTED_4_LANES_UNRESET_SEQ];
                break;

            case _400GBase_CR8:
            case _400GBase_KR8:
            case _400GBase_SR_LR8:
            case _424GBase_KR8:
                CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, 24, 1, 1));
                seq = &hwsMti400MacSeqDb[HWS_MTI_MAC400_SEGMENTED_8_LANES_UNRESET_SEQ];
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
* @internal mvHwsMti400MacLinkStatus function
* @endinternal
*
* @brief   Get CG MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMti400MacLinkStatus
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroup,
    IN  GT_U32      macNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL     *linkStatus
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

    address = MTIP_EXT_SEG_PORT_STATUS + ((macNum %8) / 4) * 0x14 + unitAddr;

    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &data, 1));

    *linkStatus = (data == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMti400MacPortEnable function
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
GT_STATUS mvHwsMti400MacPortEnable
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  macNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  GT_BOOL                 enable
)
{
    GT_U32 data = (enable==GT_TRUE) ? 3 : 0;
    portMode = portMode;

    CHECK_STATUS(genUnitRegisterSet(devNum, portGroup, MTI_MAC400_UNIT, macNum, MTI_MAC_COMMAND_CONFIG, data, 3));

    return GT_OK;
}

/**
* @internal mvHwsMti400MacPortEnableGet function
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
GT_STATUS mvHwsMti400MacPortEnableGet
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

    CHECK_STATUS(genUnitRegisterGet(devNum, portGroup, MTI_MAC400_UNIT, macNum, MTI_MAC_COMMAND_CONFIG, &data, 3));
    *enablePtr = (data) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal mvHwsMti400MacLoopbackStatusGet function
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
GT_STATUS mvHwsMti400MacLoopbackStatusGet
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

    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_MAC400_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));

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
* @internal mvHwsMti400MacLoopbackSet function
* @endinternal
*
* @brief   Set MAC loopback .
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
GT_STATUS mvHwsMti400MacLoopbackSet
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

    /* MTIP_EXT_SEG_PORT_CONTROL register set */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_EXT_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));
    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = MTIP_EXT_SEG_PORT_CONTROL + ((macNum % 8) / 4) * 0x14 + unitAddr;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum,portGroup,address,9,1,data));

    /* MTI_MAC_COMMAND_CONFIG register set */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, MTI_MAC400_UNIT, macNum, &unitAddr, &unitIndex,&unitNum));

    if (unitAddr == 0)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = MTI_MAC_COMMAND_CONFIG + unitAddr;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum,portGroup,address,6,1,data));


    /* In order to disable regular traffic to continue to the next units in pipe (MPFS/MSDB/D2D and later to Eagle)
       it is needed to stop MPFS RX enable bit */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, RAVEN_MPFS_UNIT,( macNum & 0xfffffff8), &unitAddr, &unitIndex, &unitNum));
    if (macNum%8 == 0)     /*in seg mode port 0 is taken from mpf 8 */
    {
        unitAddr += 8*HWS_MPF_CHANNEL_OFFSET;
    }
    else /*in seg mode port 4 is taken from mpf 9 */
    {
        unitAddr += 9*HWS_MPF_CHANNEL_OFFSET;
    }
    address = unitAddr + MPFS_PAU_CONTROL;
    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum,portGroup,address,0,1,(1-data)));

    return GT_OK;
}


