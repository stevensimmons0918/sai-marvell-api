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
* mvHwsMti400MacRev2If.c
*
* DESCRIPTION: MTI 400G MAC
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>


static char* mvHwsMacTypeGet(void)
{
  return "MTI400_MAC";
}

/**
* @internal mvHwsMti400MacRev2ModeCfg function
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
static GT_STATUS mvHwsMti400MacRev2ModeCfg
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_STATUS  rc;
    GT_U32 markerInterval;
    MV_HWS_MTIP_MAC400_UNIT_FIELDS_E macField;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = mvHwsMarkerIntervalCalc(devNum, portMode, curPortParams.portFecMode, &markerInterval);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(curPortParams.portMacType != MTI_MAC_400)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    macField = MAC400_UNITS_XIF_MODE_P0_ONE_STEP_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x1, NULL));

    macField = MAC400_UNITS_COMMAND_CONFIG_P0_CMD_FRAME_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x1, NULL));

    macField = MAC400_UNITS_COMMAND_CONFIG_P0_TX_PAD_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x1, NULL));

    macField = MAC400_UNITS_COMMAND_CONFIG_P0_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0, NULL));

    macField = MAC400_UNITS_COMMAND_CONFIG_P0_PROMIS_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x1, NULL));

    macField = MAC400_UNITS_TX_IPG_LENGTH_P0_TXIPGCOMPENSATION_E;
    markerInterval = (markerInterval + 1) / 8;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, markerInterval, NULL));

    macField = MAC400_UNITS_TX_FIFO_SECTIONS_P0_TX_SECTION_AVAIL_THRESHOLD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0xa, NULL));

    macField = MAC400_UNITS_RX_FIFO_SECTIONS_P0_RX_SECTION_AVAIL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x1, NULL));

#if 0
    macField = MAC400_UNITS_FRM_LENGTH_P0_FRM_LENGTH_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x3178, NULL));
#endif

    return GT_OK;
}

/**
* @internal mvHwsMti400MacRev2Reset function
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
static GT_STATUS mvHwsMti400MacRev2Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    devNum = devNum;
    portGroup = portGroup;
    portMacNum = portMacNum;
    portMode = portMode;
    action = action;

    if (POWER_DOWN == action)
    {
        /*
            DO NOTHING:

            1. Port power down sequence support is redundant because in
               mvHwsMti400MacRev2ModeCfg there is no conditional "write"
               operation. So next port mode will overwrite all fields.

            2. MAC reset sequence is implemented in mvHwsExtMacResetRelease.
        */
    }
    else if (RESET == action)
    {
        /*
            DO NOTHING:

            1. MAC reset sequence is implemented in mvHwsExtMacResetRelease.
        */
    }
    else if (UNRESET == action)
    {
        /*
            DO NOTHING:

            1. MAC unreset sequence is implemented in mvHwsExtMacResetRelease.
        */
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}

/**
* @internal mvHwsMti400MacRev2LinkStatus function
* @endinternal
*
* @brief   Get CG MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti400MacRev2LinkStatus
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroup,
    IN  GT_U32      portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL     *linkStatus
)
{
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;
    GT_U32 regValue;

    portMode = portMode;
    portGroup = portGroup;

    extField = HAWK_MTIP_EXT_BR_UNITS_STATUS_SEG_P0_LINK_OK_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portMacNum, MTI_EXT_UNIT, portMode, extField, &regValue, NULL));

    *linkStatus = (regValue == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMti400MacRev2PortEnable function
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
static GT_STATUS mvHwsMti400MacRev2PortEnable
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  GT_BOOL                 enable
)
{
    MV_HWS_MTIP_MAC400_UNIT_FIELDS_E macField;
    GT_U32 regValue;

    portGroup = portGroup;
    portMode = portMode;

    regValue = (enable == GT_TRUE) ? 1 : 0;
    macField = MAC400_UNITS_COMMAND_CONFIG_P0_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, regValue, NULL));
    macField = MAC400_UNITS_COMMAND_CONFIG_P0_RX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, regValue, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMti400MacRev2PortEnableGet function
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
GT_STATUS mvHwsMti400MacRev2PortEnableGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *enablePtr
)
{
    MV_HWS_MTIP_MAC400_UNIT_FIELDS_E macField;
    GT_U32 regValue;
    portGroup = portGroup;
    portMode = portMode;

    macField = MAC400_UNITS_COMMAND_CONFIG_P0_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, &regValue, NULL));

    *enablePtr = (regValue) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal mvHwsMti400MacRev2LoopbackStatusGet function
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
static GT_STATUS mvHwsMti400MacRev2LoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;
    GT_U32 regValue;

    portGroup = portGroup;
    portMode = portMode;

    extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_SEG_P0_LOOP_ENA_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portMacNum, MTI_EXT_UNIT, portMode, extField, &regValue, NULL));

    *lbType = (regValue == 1) ? RX_2_TX_LB : DISABLE_LB;

    return GT_OK;
}

/**
* @internal mvHwsMti400MacRev2LoopbackSet function
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
static GT_STATUS mvHwsMti400MacRev2LoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_MTIP_MAC400_UNIT_FIELDS_E macField;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;
    GT_U32 regValue;

    portGroup = portGroup;
    portMode = portMode;

    if(lbType == RX_2_TX_LB)
    {
        regValue = 1;
    }
    else if(lbType == DISABLE_LB)
    {
        regValue = 0;
    }
    else
    {
        return GT_BAD_STATE;
    }

    macField = MAC400_UNITS_COMMAND_CONFIG_P0_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, regValue, NULL));
    extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_SEG_P0_LOOP_ENA_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_EXT_UNIT, portMode, extField, regValue, NULL));

    /*
        Please pay attention that
        bit_10(MAC400_UNITS_COMMAND_CONFIG_P0_LOOPBACK_EN_E)
        is not connected to any logic - don't use it
    */

    /*
       In order to disable regular traffic to continue to the next units in
       pipe (MIF) it is needed to configure <loop_rx_block_out> bit, but
       default value already do it
    */

    return GT_OK;
}

/**
* @internal mvHwsMti
* 400MacRev2IfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMti400MacRev2IfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
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
    funcPtrArray[MTI_MAC_400]->macRestFunc    = mvHwsMti400MacRev2Reset;
    funcPtrArray[MTI_MAC_400]->macModeCfgFunc = mvHwsMti400MacRev2ModeCfg;
    funcPtrArray[MTI_MAC_400]->macLinkGetFunc = mvHwsMti400MacRev2LinkStatus;
    funcPtrArray[MTI_MAC_400]->macLbCfgFunc   = mvHwsMti400MacRev2LoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_MAC_400]->macLbStatusGetFunc = mvHwsMti400MacRev2LoopbackStatusGet;
#endif
    funcPtrArray[MTI_MAC_400]->macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_MAC_400]->macPortEnableFunc = mvHwsMti400MacRev2PortEnable;
    funcPtrArray[MTI_MAC_400]->macPortEnableGetFunc = mvHwsMti400MacRev2PortEnableGet;

    return GT_OK;
}


