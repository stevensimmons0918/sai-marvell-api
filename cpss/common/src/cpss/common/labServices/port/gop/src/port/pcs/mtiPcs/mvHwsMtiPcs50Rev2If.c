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
* @file mvHwsMtiPcs50Rev2If.c
*
* @brief MTI PCS50 interface API
*
* @version   1
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs50If.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>

static char* mvHwsMtiPcs50Rev2TypeGetFunc(void)
{
  return "MTI_PCS50";
}

/**
* @internal mvHwsMtiPcs50Rev2Reset function
* @endinternal
*
* @brief   Set MTI50 PCS RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiPcs50Rev2Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  pcsNum,value ;
    MV_HWS_PCS50_UNIT_FIELDS_E pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    if((POWER_DOWN == action) || (RESET == action) || (PARTIAL_POWER_DOWN == action))
    {
        value = 0;
        pcsField = PCS50_UNITS_CONTROL1_P1_RESET_E;
        if((HWS_DEV_SILICON_TYPE(devNum) == AC5X) && (curPortParams.numOfActLanes == 1))
        {
            value = 1;
        }
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_PCS50_UNIT, pcsField, value, NULL));

    }
    else if (UNRESET == action)
    {
        pcsField = PCS50_UNITS_CONTROL1_P1_RESET_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_PCS50_UNIT, pcsField, 1, NULL));
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs50Rev2Mode function
* @endinternal
*
* @brief   Set the MtiPcs50 mode
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
static GT_STATUS mvHwsMtiPcs50Rev2Mode
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    GT_STATUS               rc;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_FEC_MODE    portFecMode;
    GT_U32 markerInterval;
    MV_HWS_PCS50_UNIT_FIELDS_E pcsField;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 regValue;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    portFecMode = curPortParams.portFecMode;

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType > _50GMII_E)
    {
        hwsOsPrintf("mvHwsMtiPCS50Rev2Mode: bad MII type for port mode %d\n", portMode);
        return GT_BAD_PARAM;
    }

    rc = mvHwsMarkerIntervalCalc(devNum, portMode, portFecMode, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(markerInterval != 0)
    {
        pcsField = PCS50_UNITS_VENDOR_VL_INTVL_P1_MARKER_COUNTER_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, markerInterval, NULL));

        if((hwsDeviceSpecInfo[devNum].devType == AC5X) && (miiType >= _40GMII_E ))
        {
            pcsField = PCS25_UNITS_PORT_VL_INTVL_ALIGNMENT_MARKERS_INTERVAL_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, (portMacNum+1), MTI_PCS25_UNIT, pcsField, markerInterval, NULL));
        }

    }

    if((miiType == _5GMII_E) || (miiType == _10GMII_E) || (miiType == _25GMII_E))
    {
        regValue = (miiType == _25GMII_E) ? 1 : 0;
        pcsField = PCS50_UNITS_VENDOR_PCS_MODE_P1_HI_BER25_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, regValue, NULL));

        regValue = (miiType == _5GMII_E) ? 1 : 0;
        pcsField = PCS50_UNITS_VENDOR_PCS_MODE_P1_HI_BER5_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, regValue, NULL));

        if(markerInterval == 0)
        {
            pcsField = PCS50_UNITS_VENDOR_PCS_MODE_P1_DISABLE_MLD_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 1, NULL));
        }
        else
        {
            pcsField = PCS50_UNITS_VENDOR_PCS_MODE_P1_DISABLE_MLD_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0, NULL));

            if(hwsDeviceSpecInfo[devNum].devType == AC5X)
            {
                pcsField = PCS50_UNITS_VENDOR_MIRROR_VL0_0_P1_MIRROR_M0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0xc1, NULL));
                pcsField = PCS50_UNITS_VENDOR_MIRROR_VL0_0_P1_MIRROR_M1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x68, NULL));
            }
            else
            {
                pcsField = PCS50_UNITS_VL0_0_P1_VL0_0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x68c1, NULL));
            }

            if(hwsDeviceSpecInfo[devNum].devType == AC5X)
            {
                pcsField = PCS50_UNITS_VENDOR_MIRROR_VL0_1_P1_MIRROR_M2_E;
            }
            else
            {
                pcsField = PCS50_UNITS_VL0_1_P1_VL0_1_E;
            }
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x21, NULL));

        }
        pcsField = PCS50_UNITS_VENDOR_PCS_MODE_P1_ENA_CLAUSE49_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 1, NULL));
    }
    else if((miiType == _40GMII_E) || (miiType == _50GMII_E))
    {
        pcsField = PCS50_UNITS_VENDOR_PCS_MODE_P1_HI_BER25_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0, NULL));

        pcsField = PCS50_UNITS_VENDOR_PCS_MODE_P1_HI_BER5_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0, NULL));

        pcsField = PCS50_UNITS_VENDOR_PCS_MODE_P1_DISABLE_MLD_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0, NULL));

        pcsField = PCS50_UNITS_VENDOR_PCS_MODE_P1_ENA_CLAUSE49_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0, NULL));

        /* Marker values for 40G_R1,50G_R1 (no FEC dependency)*/

        if(hwsDeviceSpecInfo[devNum].devType == AC5X)
        {
            pcsField = PCS50_UNITS_VENDOR_MIRROR_VL0_0_P1_MIRROR_M0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x90, NULL));
            pcsField = PCS50_UNITS_VENDOR_MIRROR_VL0_0_P1_MIRROR_M1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x76, NULL));
        }
        else
        {
            pcsField = PCS50_UNITS_VL0_0_P1_VL0_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x7690, NULL));
        }

        if(hwsDeviceSpecInfo[devNum].devType == AC5X)
        {
            pcsField = PCS50_UNITS_VENDOR_MIRROR_VL0_1_P1_MIRROR_M2_E;
        }
        else
        {
            pcsField = PCS50_UNITS_VL0_1_P1_VL0_1_E;
        }
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x47, NULL));

    }
    else
    {
        return GT_BAD_PARAM;
    }
    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        pcsField = PCS50_UNITS_VENDOR_MIRROR_VL1_0_P1_MIRROR_M0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0xf0, NULL));
        pcsField = PCS50_UNITS_VENDOR_MIRROR_VL1_0_P1_MIRROR_M1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0xc4, NULL));
    }
    else
    {
        pcsField = PCS50_UNITS_VL1_0_P1_VL1_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0xc4f0, NULL));
    }

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        pcsField = PCS50_UNITS_VENDOR_MIRROR_VL1_1_P1_MIRROR_M2_E;
    }
    else
    {
        pcsField = PCS50_UNITS_VL1_1_P1_VL1_1_E;
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0xe6, NULL));

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        pcsField = PCS50_UNITS_VENDOR_MIRROR_VL2_0_P1_MIRROR_M0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0xc5, NULL));
        pcsField = PCS50_UNITS_VENDOR_MIRROR_VL2_0_P1_MIRROR_M1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x65, NULL));
    }
    else
    {
        pcsField = PCS50_UNITS_VL2_0_P1_VL2_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x65c5, NULL));
    }

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        pcsField = PCS50_UNITS_VENDOR_MIRROR_VL2_1_P1_MIRROR_M2_E;
    }
    else
    {
        pcsField = PCS50_UNITS_VL2_1_P1_VL2_1_E;
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x9b, NULL));

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        pcsField = PCS50_UNITS_VENDOR_MIRROR_VL3_0_P1_MIRROR_M0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0xa2, NULL));
        pcsField = PCS50_UNITS_VENDOR_MIRROR_VL3_0_P1_MIRROR_M1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x79, NULL));
    }
    else
    {
        pcsField = PCS50_UNITS_VL3_0_P1_VL3_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x79a2, NULL));
    }

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        pcsField = PCS50_UNITS_VENDOR_MIRROR_VL3_1_P1_MIRROR_M2_E;
    }
    else
    {
        pcsField = PCS50_UNITS_VL3_1_P1_VL3_1_E;
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS50_UNIT, pcsField, 0x3d, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50Rev2LoopBack function
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
static GT_STATUS mvHwsMtiPcs50Rev2LoopBack
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;
    MV_HWS_PCS50_UNIT_FIELDS_E pcsField;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = PCS50_UNITS_CONTROL1_P1_LOOPBACK_E;
    data = (lbType == TX_2_RX_LB) ? 1 : 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS50_UNIT, pcsField, data, NULL));

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        extField = PHOENIX_MTIP_EXT_UNITS_CONTROL1_P0_FORCE_LINK_OK_EN_E;
    }
    else
    {
        extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_P0_FORCE_LINK_OK_EN_E;
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50Rev2LoopBackGet function
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
static GT_STATUS mvHwsMtiPcs50Rev2LoopBackGet
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
    MV_HWS_PCS50_UNIT_FIELDS_E pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = PCS50_UNITS_CONTROL1_P1_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS50_UNIT, pcsField, &data, NULL));

    *lbType = (data != 0) ? TX_2_RX_LB : DISABLE_LB;
    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50Rev2AlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiPcs50.
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
static GT_STATUS mvHwsMtiPcs50Rev2AlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_UOPT                 portGroup,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    OUT GT_BOOL                 *lock
)
{
    GT_UREG_DATA                  data;
    MV_HWS_PORT_INIT_PARAMS       curPortParams;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        extField = PHOENIX_MTIP_EXT_UNITS_STATUS_P0_LINK_STATUS_E;
    }
    else
    {
        extField = HAWK_MTIP_EXT_BR_UNITS_STATUS_P0_LINK_STATUS_E;
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_EXT_UNIT, extField, &data, NULL));

    *lock = (data != 0);

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50Rev2SendFaultSet function
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
static GT_STATUS mvHwsMtiPcs50Rev2SendFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;

    portGroup = portGroup;
    portMode = portMode;

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

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        extField = PHOENIX_MTIP_EXT_UNITS_CONTROL_P0_TX_REM_FAULT_E;
    }
    else
    {
        extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_P0_TX_REM_FAULT_E;
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50Rev2SendLocalFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending local fault signals to partner.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - pcs number
* @param[in  portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs50Rev2SendLocalFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;

    portGroup = portGroup;
    portMode = portMode;

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

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        extField = PHOENIX_MTIP_EXT_UNITS_CONTROL_P0_TX_LOC_FAULT_E;
    }
    else
    {
        extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_P0_TX_LOC_FAULT_E;
    }
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs50Rev2FecConfigGet function
* @endinternal
*
* @brief   Return the FEC mode status on PCS.
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
static GT_STATUS mvHwsMtiPcs50Rev2FecConfigGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    devNum = devNum;
    portGroup = portGroup;
    phyPortNum = phyPortNum;
    portMode = portMode;
    if(portFecTypePtr == NULL)
    {
        return GT_BAD_PTR;
    }

    return  mvHwsExtFecTypeGet(devNum, portGroup, phyPortNum, portMode, portFecTypePtr);
}

/**
* @internal mvHwsMtiPcs50Rev2CheckGearBox function
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
static GT_STATUS mvHwsMtiPcs50Rev2CheckGearBox
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_UREG_DATA                data;
    MV_HWS_PCS50_UNIT_FIELDS_E  pcsField;
    MV_HWS_HAWK_CONVERT_STC     convertIdx;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* for RS_FEC modes 'block_lock' field is not relevant */
    if((curPortParams.portFecMode == RS_FEC) || (curPortParams.portFecMode == RS_FEC_544_514))
    {
        CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portMode, &convertIdx));
        if(hwsDeviceSpecInfo[devNum].devType == AC5X)
        {
            pcsField = PHOENIX_MTIP_EXT_UNITS_GLOBAL_AMPS_LOCK_STATUS_AMPS_LOCK_E;
        }
        else
        {
            pcsField = HAWK_MTIP_EXT_BR_UNITS_GLOBAL_AMPS_LOCK_STATUS_AMPS_LOCK_E;
        }

        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_EXT_UNIT, pcsField, &data, NULL));

        data = (data >> convertIdx.ciderIndexInUnit) & ((1 << curPortParams.numOfActLanes) - 1);

        *lockPtr = ((GT_U16)data == ((1 << curPortParams.numOfActLanes) - 1)) ? GT_TRUE : GT_FALSE;
    }
    else
    {
        pcsField = PCS50_UNITS_BASER_STATUS1_P1_BLOCK_LOCK_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS50_UNIT, pcsField, &data, NULL));
        *lockPtr = (data & 0x1) ? GT_TRUE : GT_FALSE;
    }

#ifdef ASIC_SIMULATION
    *lockPtr = GT_TRUE;
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50Rev2IfInit function
* @endinternal
*
* @brief   Init MtiPcs50 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs50Rev2IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_PCS_50])
    {
        funcPtrArray[MTI_PCS_50] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_PCS_50])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_PCS_50], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_PCS_50]->pcsResetFunc     = mvHwsMtiPcs50Rev2Reset;
    funcPtrArray[MTI_PCS_50]->pcsModeCfgFunc   = mvHwsMtiPcs50Rev2Mode;
    funcPtrArray[MTI_PCS_50]->pcsLbCfgFunc     = mvHwsMtiPcs50Rev2LoopBack;
    funcPtrArray[MTI_PCS_50]->pcsLbCfgGetFunc = mvHwsMtiPcs50Rev2LoopBackGet;
    funcPtrArray[MTI_PCS_50]->pcsTypeGetFunc   = mvHwsMtiPcs50Rev2TypeGetFunc;
    funcPtrArray[MTI_PCS_50]->pcsFecCfgGetFunc = mvHwsMtiPcs50Rev2FecConfigGet;
    funcPtrArray[MTI_PCS_50]->pcsCheckGearBoxFunc = mvHwsMtiPcs50Rev2CheckGearBox;
    funcPtrArray[MTI_PCS_50]->pcsAlignLockGetFunc = mvHwsMtiPcs50Rev2AlignLockGet;
    funcPtrArray[MTI_PCS_50]->pcsSendFaultSetFunc = mvHwsMtiPcs50Rev2SendFaultSet;
    funcPtrArray[MTI_PCS_50]->pcsSendLocalFaultSetFunc = mvHwsMtiPcs50Rev2SendLocalFaultSet;

    return GT_OK;
}

