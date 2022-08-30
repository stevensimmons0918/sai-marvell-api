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
* mvHwsMti100MacBrRev2If.c
*
* DESCRIPTION: MTI100 BR 1G-100G MAC
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
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>



static char* mvHwsMacTypeGet(void)
{
  return "MTI100_MAC_BR";
}

/**
* @internal mvHwsMtiMac100BrRev2ModeCfg function
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
static GT_STATUS mvHwsMtiMac100BrRev2ModeCfg
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
    MV_HWS_MTIP_MAC100_BR_UNIT_FIELDS_E macField;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 regValue, txFifoFullRegValue;
    MV_HWS_UNITS_ID unitId;
    GT_BOOL isPreemptionEnabled = GT_FALSE;

    if(!HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portFecMode = curPortParams.portFecMode;

    if (attributesPtr != NULL)
    {
        isPreemptionEnabled = attributesPtr->preemptionEnable;
    }

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType > _100GMII_E)
    {
        hwsOsPrintf("mvHwsMtiMac100BrRev2ModeCfg: bad MII type for port mode %d\n", portMode);
        return GT_BAD_PARAM;
    }

    rc = mvHwsMarkerIntervalCalc(devNum, portMode, portFecMode, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }

    unitId =  MTI_MAC100_UNIT;

    /* Common COMPENSATION */
    macField = MAC100_BR_UNITS_TX_IPG_LENGTH_P0_COMPENSATION_E;
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
    /* Common TXIPG */
    macField = MAC100_BR_UNITS_TX_IPG_LENGTH_P0_TXIPG_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

    /* Common CRC_0BYTE */
    macField = MAC100_BR_UNITS_CRC_MODE_P0_CRC_0BYTE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));
    /* Common CRC_1BYTE */
    macField = MAC100_BR_UNITS_CRC_MODE_P0_CRC_1BYTE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));
    /* Common CRC_2BYTE */
    macField = MAC100_BR_UNITS_CRC_MODE_P0_CRC_2BYTE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));

    /* EMAC CNTL_FRAME_ENA */
    macField = MAC100_BR_UNITS_EMAC_COMMAND_CONFIG_P0_CNTL_FRAME_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));

    /* Common TX_PAD_EN */
    macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_TX_PAD_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));

    /* Common CRC_FWD */
    macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));

    /* EMAC PROMIS_EN */
    macField = MAC100_BR_UNITS_EMAC_COMMAND_CONFIG_P0_PROMIS_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));

    /* Common SHORT_PREAMBLE */
    macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_SHORT_PREAMBLE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));

    /* Common NO_PREAMBLE */
    macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_NO_PREAMBLE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));

    if(HWS_100G_R2_MODE_CHECK(portMode))
        {txFifoFullRegValue = 0xB;}
    else
        {txFifoFullRegValue = 0xA;}

    /* EMAC TX_SECTION_FULL */
    macField = MAC100_BR_UNITS_EMAC_TX_FIFO_SECTIONS_P0_TX_SECTION_FULL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, txFifoFullRegValue, NULL));

    /* EMAC RX_SECTION_FULL */
    macField = MAC100_BR_UNITS_EMAC_RX_FIFO_SECTIONS_P0_RX_SECTION_FULL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

    /* Common ONESTEPENA */
    macField = MAC100_BR_UNITS_XIF_MODE_P0_ONESTEPENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

    /* Common XGMII */
    regValue = ((miiType == _GMII_E) || (miiType == _5GMII_E) || (miiType == _10GMII_E) || (miiType == _25GMII_E)) ? 1 : 0;
    macField = MAC100_BR_UNITS_XIF_MODE_P0_XGMII_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

#if 0
    /* Common PAUSETIMERX8 */
    regValue = (miiType == _GMII_E) ? 1 : 0;
    macField = MAC100_BR_UNITS_XIF_MODE_P0_PAUSETIMERX8_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

    /* EMAC FRM_LENGTH */
    macField = MAC100_BR_UNITS_EMAC_FRM_LENGTH_P0_FRM_LENGTH_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x3178, NULL));
#endif

    /* If preemption MAC feature is enabled */
    if(isPreemptionEnabled == GT_TRUE)
    {
        /* PMAC CNTL_FRAME_ENA */
        macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_CNTL_FRAME_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));

        /* PMAC PROMIS_EN */
        macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_PROMIS_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));

#if 0
        /* FLT_TX_STOP */
        /*
            allows the TX MAC to continue transmission in the presence of a
            link fault. This is also a behavior more aligned to the 802.3
            specification as faults are handled by the reconciliation sublayer,
            and in the event of a fault on the RX direction, the TX should
            either send IDLEs or remote faults, and discard any incoming data.
            This has a different side-effect: the MAC will continue draining
            frames, and when the fault is no longer present on the receive
            side the TX path will open up, and resume potentially mid-frame
            or even mid-preempted frame. If your switching logic detects
            the link down and removes the port from a valid destination,
            this is a good alternative as it prevents filling up the
            egress queues with frames.
        */
        macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_FLT_TX_STOP_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));
#endif

        /* PMAC TX_SECTION_FULL */
        macField = MAC100_BR_UNITS_PMAC_TX_FIFO_SECTIONS_P0_TX_SECTION_FULL_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, txFifoFullRegValue, NULL));

        /* PMAC RX_SECTION_FULL */
        macField = MAC100_BR_UNITS_PMAC_RX_FIFO_SECTIONS_P0_RX_SECTION_FULL_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

#if 0
        /* PMAC FRM_LENGTH */
        macField = MAC100_BR_UNITS_PMAC_FRM_LENGTH_P0_FRM_LENGTH_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x3178, NULL));
#endif

        /* Preemption MAC enable */
        macField = MAC100_BR_UNITS_BR_CONTROL_P0_TX_PREEMPT_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

        /* When set to 0, PMAC frames are not transmitted unless preemption verification has succeeded. */
        macField = MAC100_BR_UNITS_BR_CONTROL_P0_TX_ALLOW_PMAC_IF_NVERIF_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

        /* When set to 0, EMAC frames are not transmitted unless preemption verification has succeeded. */
        macField = MAC100_BR_UNITS_BR_CONTROL_P0_TX_ALLOW_EMAC_IF_NVERIF_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));
    }
    else
    {
#if 0
        macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_FLT_TX_STOP_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));
#endif
        /* Preemption MAC disable */
        macField = MAC100_BR_UNITS_BR_CONTROL_P0_TX_PREEMPT_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x0, NULL));

        /* When set to 0, PMAC frames are not transmitted unless preemption verification has succeeded. */
        macField = MAC100_BR_UNITS_BR_CONTROL_P0_TX_ALLOW_PMAC_IF_NVERIF_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

        /* When set to 0, EMAC frames are not transmitted unless preemption verification has succeeded. */
        macField = MAC100_BR_UNITS_BR_CONTROL_P0_TX_ALLOW_EMAC_IF_NVERIF_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiMac100BrRev2Reset function
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
static GT_STATUS mvHwsMtiMac100BrRev2Reset
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
               mvHwsMtiMac100BrRev2ModeCfg there is no conditional "write"
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
* @internal mvHwsMtiMac100BrRev2LinkStatus function
* @endinternal
*
* @brief   Get MTI100 MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiMac100BrRev2LinkStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
)
{
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    portMode = portMode;
    portGroup = portGroup;

    extField = HAWK_MTIP_EXT_BR_UNITS_STATUS_P0_LINK_OK_E;
    unitId = MTI_EXT_UNIT;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, unitId, extField, &regValue, NULL));
    *linkStatus = (regValue == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMtiMac100BrRev2PortEnable function
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
static GT_STATUS mvHwsMtiMac100BrRev2PortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    MV_HWS_MTIP_MAC100_BR_UNIT_FIELDS_E macField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    unitId = MTI_MAC100_UNIT;

    regValue = (enable == GT_TRUE) ? 1 : 0;

    macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));
    macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_RX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

    macField = MAC100_BR_UNITS_EMAC_COMMAND_CONFIG_P0_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));
    macField = MAC100_BR_UNITS_EMAC_COMMAND_CONFIG_P0_RX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiMac100BrRev2PortEnableGet function
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
GT_STATUS mvHwsMtiMac100BrRev2PortEnableGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *enablePtr
)
{
    MV_HWS_MTIP_MAC100_BR_UNIT_FIELDS_E macField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    unitId = MTI_MAC100_UNIT;
    macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, unitId, macField, &regValue, NULL));

    *enablePtr = (regValue) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal mvHwsMtiMac100BrRev2LoopbackStatusGet function
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
static GT_STATUS mvHwsMtiMac100BrRev2LoopbackStatusGet
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
    MV_HWS_UNITS_ID unitId;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_P0_LOOP_ENA_E;
    unitId = MTI_EXT_UNIT;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, unitId, extField, &regValue, NULL));

    *lbType = (regValue == 1) ? RX_2_TX_LB : DISABLE_LB;

    return GT_OK;
}

/**
* @internal mvHwsMtiMac100BrRev2LoopbackSet function
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
static GT_STATUS mvHwsMtiMac100BrRev2LoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_MTIP_MAC100_BR_UNIT_FIELDS_E macField;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;
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

    macField = MAC100_BR_UNITS_COMMAND_CONFIG_P0_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC100_UNIT, macField, regValue, NULL));
    extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_P0_LOOP_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_EXT_UNIT, extField, regValue, NULL));
    extField = HAWK_MTIP_EXT_BR_UNITS_PMAC_CONTROL_P0_PREEMPTION_LOOP_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_EXT_UNIT, extField, regValue, NULL));

    /*
        Please pay attention that
        bit_10(MAC100_BR_UNITS_COMMAND_CONFIG_P0_LOOPBACK_EN_E)
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
* @internal mvHwsMtiMac100BrRev2IfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMtiMac100BrRev2IfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_MAC_100_BR])
    {
        funcPtrArray[MTI_MAC_100_BR] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
        if(!funcPtrArray[MTI_MAC_100_BR])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_MAC_100_BR], 0, sizeof(MV_HWS_MAC_FUNC_PTRS));
    }
    funcPtrArray[MTI_MAC_100_BR]->macRestFunc    = mvHwsMtiMac100BrRev2Reset;
    funcPtrArray[MTI_MAC_100_BR]->macModeCfgFunc = mvHwsMtiMac100BrRev2ModeCfg;
    funcPtrArray[MTI_MAC_100_BR]->macLinkGetFunc = mvHwsMtiMac100BrRev2LinkStatus;
    funcPtrArray[MTI_MAC_100_BR]->macLbCfgFunc   = mvHwsMtiMac100BrRev2LoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_MAC_100_BR]->macLbStatusGetFunc = mvHwsMtiMac100BrRev2LoopbackStatusGet;
#endif
    funcPtrArray[MTI_MAC_100_BR]->macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_MAC_100_BR]->macPortEnableFunc = mvHwsMtiMac100BrRev2PortEnable;
    funcPtrArray[MTI_MAC_100_BR]->macPortEnableGetFunc = mvHwsMtiMac100BrRev2PortEnableGet;

    return GT_OK;
}

