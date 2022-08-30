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
* mvHwsMti100MacRev2If.c
*
* DESCRIPTION: MTI100 1G-100G MAC
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
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>



static char* mvHwsMacTypeGet(void)
{
  return "MTI100_MAC";
}

/**
* @internal mvHwsMti100MacRev2ModeCfg function
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
static GT_STATUS mvHwsMti100MacRev2ModeCfg
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
    GT_STATUS  rc;
    GT_U32 markerInterval;
    MV_HWS_MTIP_MAC100_UNIT_FIELDS_E macField;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    portFecMode = curPortParams.portFecMode;

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType > _100GMII_E)
    {
        hwsOsPrintf("mvHwsMti100MacRev2ModeCfg: bad MII type for port mode %d\n", portMode);
        return GT_BAD_PARAM;
    }

    rc = mvHwsMarkerIntervalCalc(devNum, portMode, portFecMode, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(mvHwsUsxModeCheck(devNum, portMacNum, portMode))
    {
        unitId =  MTI_USX_MAC_UNIT;
        if((markerInterval != 0) && (markerInterval != 0x4FFF))
        {
            markerInterval = markerInterval / 4;
        }
    }
    else
    {
        unitId =  MTI_MAC100_UNIT;
    }


    macField = MAC100_UNITS_TX_IPG_LENGTH_P0_COMPENSATION_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, markerInterval, NULL));

    if(mvHwsUsxModeCheck(devNum, portMacNum, portMode) &&
                         ((markerInterval != 0x4FFF) && (markerInterval != 0x0)))
    {
        regValue = 14;
    }
    else
    {
        regValue = 12;
    }
    macField = MAC100_UNITS_TX_IPG_LENGTH_P0_TXIPG_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

    macField = MAC100_UNITS_CRC_MODE_P0_CRC_0BYTE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));
    macField = MAC100_UNITS_CRC_MODE_P0_CRC_1BYTE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));
    macField = MAC100_UNITS_CRC_MODE_P0_CRC_2BYTE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));

    if(mvHwsUsxModeCheck(devNum, portMacNum, portMode))
    {
        macField = MAC100_UNITS_COMMAND_CONFIG_P0_RX_SFD_ANY_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));
    }

    macField = MAC100_UNITS_COMMAND_CONFIG_P0_CNTL_FRAME_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));

    macField = MAC100_UNITS_COMMAND_CONFIG_P0_TX_PAD_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));

    macField = MAC100_UNITS_COMMAND_CONFIG_P0_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));

    macField = MAC100_UNITS_COMMAND_CONFIG_P0_PROMIS_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));

    macField = MAC100_UNITS_COMMAND_CONFIG_P0_SHORT_PREAMBLE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));

    macField = MAC100_UNITS_COMMAND_CONFIG_P0_NO_PREAMBLE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        if(mvHwsUsxModeCheck(devNum, portMacNum, portMode))
            {regValue = 21;}
        else
            {regValue = 0x9;}
    }
    else
    {
        if(mvHwsUsxModeCheck(devNum, portMacNum, portMode))
            {regValue = 21;}
        else if(HWS_100G_R2_MODE_CHECK(portMode))
            {regValue = 0xB;}
        else
            {regValue = 0xA;}
    }
    macField = MAC100_UNITS_TX_FIFO_SECTIONS_P0_TX_SECTION_FULL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

    macField = MAC100_UNITS_RX_FIFO_SECTIONS_P0_RX_SECTION_FULL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

    macField = MAC100_UNITS_XIF_MODE_P0_ONESTEPENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

    regValue = ((miiType == _GMII_E) || (miiType == _5GMII_E) || (miiType == _10GMII_E) || (miiType == _25GMII_E)) ? 1 : 0;
    macField = MAC100_UNITS_XIF_MODE_P0_XGMII_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

#if 0
    regValue = (miiType == _GMII_E) ? 1 : 0;
    macField = MAC100_UNITS_XIF_MODE_P0_PAUSETIMERX8_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

    macField = MAC100_UNITS_FRM_LENGTH_P0_FRM_LENGTH_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x3178, NULL));
#endif

    return GT_OK;
}

/**
* @internal mvHwsMti100MacRev2Reset function
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
static GT_STATUS mvHwsMti100MacRev2Reset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
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
               mvHwsMti100MacRev2ModeCfg there is no conditional "write"
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
* @internal mvHwsMti100MacRev2LinkStatus function
* @endinternal
*
* @brief   Get MTI100 MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti100MacRev2LinkStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
)
{
    MV_HWS_HAWK_MTIP_EXT_UNIT_FIELDS_E extField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    portMode = portMode;
    portGroup = portGroup;

    if(mvHwsUsxModeCheck(devNum, portMacNum, portMode))
    {
        /*
         * If only Auto-negotiation is enabled,
         * for USGMII modes use the Copper Link Status from PHY
         * because the MAC link status is allways "UP"
         */
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_CONTROL_P0_ANENABLE_E, &regValue, NULL));
        if ( (0x1 == regValue) && (portMode == _10G_OUSGMII || portMode == _5G_QUSGMII || portMode == QSGMII) )
        {
            extField = LPCS_UNITS_PARTNER_ABILITY_P0_LP_NP_E;
            unitId = MTI_USX_LPCS_UNIT;
        }
        else
        {
            extField = MTIP_USX_EXT_UNITS_STATUS_P0_LINK_OK_E;
            unitId = MTI_USX_EXT_UNIT;
        }
    }
    else
    {
        if(hwsDeviceSpecInfo[devNum].devType == AC5X)
        {
            extField = PHOENIX_MTIP_EXT_UNITS_STATUS_P0_LINK_OK_E;
        }
        else
        {
            extField = HAWK_MTIP_EXT_UNITS_STATUS_P0_LINK_OK_E;
        }
        unitId = MTI_EXT_UNIT;
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, unitId, extField, &regValue, NULL));

    *linkStatus = (regValue == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMti100MacRev2PortEnable function
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
static GT_STATUS mvHwsMti100MacRev2PortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    MV_HWS_MTIP_MAC100_UNIT_FIELDS_E macField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    portGroup = portGroup;

    unitId = (mvHwsUsxModeCheck(devNum, portMacNum, portMode)) ? MTI_USX_MAC_UNIT : MTI_MAC100_UNIT;

    regValue = (enable == GT_TRUE) ? 1 : 0;
    macField = MAC100_UNITS_COMMAND_CONFIG_P0_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));
    macField = MAC100_UNITS_COMMAND_CONFIG_P0_RX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMti100MacRev2PortEnableGet function
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
GT_STATUS mvHwsMti100MacRev2PortEnableGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *enablePtr
)
{
    MV_HWS_MTIP_MAC100_UNIT_FIELDS_E macField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    portGroup = portGroup;

    unitId = (mvHwsUsxModeCheck(devNum, portMacNum, portMode)) ? MTI_USX_MAC_UNIT : MTI_MAC100_UNIT;
    macField = MAC100_UNITS_COMMAND_CONFIG_P0_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, unitId, macField, &regValue, NULL));

    *enablePtr = (regValue) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal mvHwsMti100MacRev2LoopbackStatusGet function
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
static GT_STATUS mvHwsMti100MacRev2LoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_HAWK_MTIP_EXT_UNIT_FIELDS_E extField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    portGroup = portGroup;

    if(mvHwsUsxModeCheck(devNum, portMacNum, portMode))
    {
        extField = MTIP_USX_EXT_UNITS_CONTROL_P0_LOOP_ENA_E;
        unitId = MTI_USX_EXT_UNIT;
    }
    else
    {
        if(hwsDeviceSpecInfo[devNum].devType == AC5X)
        {
            extField = PHOENIX_MTIP_EXT_UNITS_CONTROL_P0_LOOP_ENA_E;
        }
        else
        {
            extField = HAWK_MTIP_EXT_UNITS_CONTROL_P0_LOOP_ENA_E;
        }
        unitId = MTI_EXT_UNIT;
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, unitId, extField, &regValue, NULL));

    *lbType = (regValue == 1) ? RX_2_TX_LB : DISABLE_LB;

    return GT_OK;
}

/**
* @internal mvHwsMti100MacRev2LoopbackSet function
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
static GT_STATUS mvHwsMti100MacRev2LoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_MTIP_MAC100_UNIT_FIELDS_E macField;
    MV_HWS_HAWK_MTIP_EXT_UNIT_FIELDS_E extField;
    GT_U32 regValue;

    portGroup = portGroup;

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

    if(mvHwsUsxModeCheck(devNum, portMacNum, portMode))
    {
        macField = MAC100_UNITS_COMMAND_CONFIG_P0_CRC_FWD_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_MAC_UNIT, macField, regValue, NULL));
        extField = MTIP_USX_EXT_UNITS_CONTROL_P0_LOOP_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_EXT_UNIT, extField, regValue, NULL));

    }
    else
    {
        macField = MAC100_UNITS_COMMAND_CONFIG_P0_CRC_FWD_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC100_UNIT, macField, regValue, NULL));
        if(hwsDeviceSpecInfo[devNum].devType == AC5X)
        {
            extField = PHOENIX_MTIP_EXT_UNITS_CONTROL_P0_LOOP_ENA_E;
        }
        else
        {
            extField = HAWK_MTIP_EXT_UNITS_CONTROL_P0_LOOP_ENA_E;
        }
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_EXT_UNIT, extField, regValue, NULL));
    }

    /*
        Please pay attention that
        bit_10(MAC100_UNITS_COMMAND_CONFIG_P0_LOOPBACK_EN_E)
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
* @internal mvHwsMti100MacRev2IfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMti100MacRev2IfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
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
    funcPtrArray[MTI_MAC_100]->macRestFunc    = mvHwsMti100MacRev2Reset;
    funcPtrArray[MTI_MAC_100]->macModeCfgFunc = mvHwsMti100MacRev2ModeCfg;
    funcPtrArray[MTI_MAC_100]->macLinkGetFunc = mvHwsMti100MacRev2LinkStatus;
    funcPtrArray[MTI_MAC_100]->macLbCfgFunc   = mvHwsMti100MacRev2LoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_MAC_100]->macLbStatusGetFunc = mvHwsMti100MacRev2LoopbackStatusGet;
#endif
    funcPtrArray[MTI_MAC_100]->macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_MAC_100]->macPortEnableFunc = mvHwsMti100MacRev2PortEnable;
    funcPtrArray[MTI_MAC_100]->macPortEnableGetFunc = mvHwsMti100MacRev2PortEnableGet;

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxMacRev2IfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMtiUsxMacRev2IfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_USX_MAC])
    {
        funcPtrArray[MTI_USX_MAC] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
        if(!funcPtrArray[MTI_USX_MAC])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_USX_MAC], 0, sizeof(MV_HWS_MAC_FUNC_PTRS));
    }
    funcPtrArray[MTI_USX_MAC]->macRestFunc    = mvHwsMti100MacRev2Reset;
    funcPtrArray[MTI_USX_MAC]->macModeCfgFunc = mvHwsMti100MacRev2ModeCfg;
    funcPtrArray[MTI_USX_MAC]->macLinkGetFunc = mvHwsMti100MacRev2LinkStatus;
    funcPtrArray[MTI_USX_MAC]->macLbCfgFunc   = mvHwsMti100MacRev2LoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_USX_MAC]->macLbStatusGetFunc = mvHwsMti100MacRev2LoopbackStatusGet;
#endif
    funcPtrArray[MTI_USX_MAC]->macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_USX_MAC]->macPortEnableFunc = mvHwsMti100MacRev2PortEnable;

    return GT_OK;
}

