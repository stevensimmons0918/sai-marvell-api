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
* mvHwsMtiCpuMacRev2If.c
*
* DESCRIPTION: MTI CPU 10G MAC
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
#include <cpss/common/labServices/port/gop/port/mac/mtiCpuMac/mvHwsMtiCpuMacRev2If.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mac/mtiCpuMac/mvHwsMtiCpuMacDb.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMacCpuUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsPcsCpuUnits.h>
#include <cpss/common/labServices/port/gop/port/silicon/ac5p/regDb/mvHwsMtipCpuExtUnits.h>

static char* mvHwsMacRev2TypeGet(void)
{
  return "MTI_CPU_MAC_REV2";
}

/**
* @internal mvHwsMtiCpuMacRev2ModeCfg function
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
static GT_STATUS mvHwsMtiCpuMacRev2ModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{

    MV_HWS_PORT_INIT_PARAMS       curPortParams;
    MV_HWS_CPU_MAC_UNITS_FIELDS_E macField;
    GT_STATUS rc;
    GT_U32 markerInterval;
    MV_HWS_PORT_FEC_MODE    portFecMode;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams)) {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /** I: m_RAL.COMMAND_CONFIG.CNTL_FRAME_ENA.set(1);
          m_RAL.COMMAND_CONFIG.TX_PAD_EN.set(1); */
    macField = CPU_MAC_UNITS_COMMAND_CONFIG_CNTL_FRAME_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, 1, NULL));

    macField = CPU_MAC_UNITS_COMMAND_CONFIG_TX_PAD_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, 1, NULL));

    portFecMode = curPortParams.portFecMode;
    /* m_RAL.TX_IPG_LENGTH.Compensation.set(ipgBias);*/
    rc = mvHwsMarkerIntervalCalc(devNum, portMode, portFecMode, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }
    macField = CPU_MAC_UNITS_TX_IPG_LENGTH_COMPENSATION_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, markerInterval, NULL));

    /* m_RAL.TX_IPG_LENGTH.TxIPG.set(12);*/
    macField = CPU_MAC_UNITS_TX_IPG_LENGTH_TXIPG_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, 12, NULL));

    /** II: CRC_STRIP_ENABLED   : m_RAL.COMMAND_CONFIG.CRC_FWD.set(0);*/

    macField = CPU_MAC_UNITS_COMMAND_CONFIG_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, 0, NULL));

    /** III: m_RAL.COMMAND_CONFIG.PROMIS_EN.set(1);   */

    macField = CPU_MAC_UNITS_COMMAND_CONFIG_PROMIS_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, 1, NULL));

    macField = CPU_MAC_UNITS_TX_FIFO_SECTIONS_TX_SECTION_FULL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, 0x15, NULL));

    macField = CPU_MAC_UNITS_RX_FIFO_SECTIONS_RX_SECTION_FULL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, 0x1, NULL));

    macField = CPU_MAC_UNITS_XIF_MODE_ONESTEPENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, 1, NULL));

    macField = CPU_MAC_UNITS_XIF_MODE_XGMII_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, 1, NULL));

#if 0
    macField = CPU_MAC_UNITS_FRM_LENGTH_FRM_LENGTH_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_CPU_MAC_UNIT, macField, 0x3178, NULL));
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacRev2Reset function
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
static GT_STATUS mvHwsMtiCpuMacRev2Reset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    (void)devNum;
    (void)portGroup;
    (void)portMacNum;
    (void)portMode;
    (void)action;

    if (POWER_DOWN == action)
    {
        /*
            TBD
        */
    }
    else if (RESET == action)
    {
        /*
           TBD
        */
    }
    else if (UNRESET == action)
    {
        /*
            TBD
        */
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }
    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacRev2LinkStatus function
* @endinternal
*
* @brief   Get MTI CPU MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiCpuMacRev2LinkStatus
(
    GT_U8       devNum,
    GT_U32      portGroup,
    GT_U32      macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL     *linkStatus
)
{
    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E extField = MTIP_CPU_EXT_UNITS_STATUS_LINK_OK_E;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId = MTI_CPU_EXT_UNIT;

    (void)portMode;
    (void)portGroup;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, macNum, unitId, extField, &regValue, NULL));
    *linkStatus = (regValue == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacRev2PortEnable function
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
static GT_STATUS mvHwsMtiCpuMacRev2PortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    MV_HWS_CPU_MAC_UNITS_FIELDS_E macField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId = MTI_CPU_MAC_UNIT;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    regValue = (enable == GT_TRUE) ? 1 : 0;
    macField = CPU_MAC_UNITS_COMMAND_CONFIG_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, macNum, unitId, macField, regValue, NULL));
    macField = CPU_MAC_UNITS_COMMAND_CONFIG_RX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, macNum, unitId, macField, regValue, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacRev2PortEnableGet function
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
GT_STATUS mvHwsMtiCpuMacRev2PortEnableGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *enablePtr
)
{
    MV_HWS_CPU_MAC_UNITS_FIELDS_E macField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    macField = CPU_MAC_UNITS_COMMAND_CONFIG_TX_ENA_E;
    unitId = MTI_CPU_MAC_UNIT;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, unitId, macField, &regValue, NULL));

    *enablePtr = (regValue) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacRev2LoopbackStatusGet function
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
static GT_STATUS mvHwsMtiCpuMacRev2LoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{

    MV_HWS_MTIP_CPU_EXT_UNITS_FIELDS_E extField = MTIP_CPU_EXT_UNITS_CONTROL_LOOP_ENA_E;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId = MTI_CPU_EXT_UNIT;

    (void)portMode;
    (void)portGroup;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, macNum, unitId, extField, &regValue, NULL));
    *lbType = (regValue == 1) ? RX_2_TX_LB : DISABLE_LB;
    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacRev2LoopbackSet function
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
static GT_STATUS mvHwsMtiCpuMacRev2LoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  macNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_CPU_MAC_UNITS_FIELDS_E macField;
    GT_U32 regValue;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

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

    macField = CPU_MAC_UNITS_COMMAND_CONFIG_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, macNum, MTI_CPU_MAC_UNIT, macField, regValue, NULL));
    macField = CPU_MAC_UNITS_COMMAND_CONFIG_LOOPBACK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, macNum, MTI_CPU_MAC_UNIT, macField, regValue, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiCpuMacRev2IfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMtiCpuMacRev2IfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
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
    funcPtrArray[MTI_CPU_MAC]->macRestFunc    = mvHwsMtiCpuMacRev2Reset;
    funcPtrArray[MTI_CPU_MAC]->macModeCfgFunc = mvHwsMtiCpuMacRev2ModeCfg;
    funcPtrArray[MTI_CPU_MAC]->macLinkGetFunc = mvHwsMtiCpuMacRev2LinkStatus;
    funcPtrArray[MTI_CPU_MAC]->macLbCfgFunc   = mvHwsMtiCpuMacRev2LoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_CPU_MAC]->macLbStatusGetFunc = mvHwsMtiCpuMacRev2LoopbackStatusGet;
#endif
    funcPtrArray[MTI_CPU_MAC]->macTypeGetFunc = mvHwsMacRev2TypeGet;
    funcPtrArray[MTI_CPU_MAC]->macPortEnableFunc = mvHwsMtiCpuMacRev2PortEnable;
    funcPtrArray[MTI_CPU_MAC]->macPortEnableGetFunc = mvHwsMtiCpuMacRev2PortEnableGet;

    return GT_OK;
}
