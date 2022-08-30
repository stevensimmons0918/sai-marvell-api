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
* @file mvHwsMtiPcs100Rev2If.c
*
* @brief MTI PCS100 interface API
*
* @version   1
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs100If.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>

static char* mvHwsMtiPcs100Rev2TypeGetFunc(void)
{
  return "MTI_PCS100";
}

/**
* @internal mvHwsMtiPcs100Rev2Reset function
* @endinternal
*
* @brief   Set MTI100 PCS RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiPcs100Rev2Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  pcsNum;
    MV_HWS_PCS100_UNIT_FIELDS_E pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    if((POWER_DOWN == action) || (RESET == action) || (PARTIAL_POWER_DOWN == action))
    {
        pcsField = PCS100_UNITS_CONTROL1_P0_RESET_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_PCS100_UNIT, pcsField, 0, NULL));
    }
    else if (UNRESET == action)
    {
        pcsField = PCS100_UNITS_CONTROL1_P0_RESET_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_PCS100_UNIT, pcsField, 1, NULL));
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs100Rev2Mode function
* @endinternal
*
* @brief   Set the MtiPCS100 mode
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
static GT_STATUS mvHwsMtiPcs100Rev2Mode
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
    MV_HWS_PCS100_UNIT_FIELDS_E pcsField;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 regValue;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    portFecMode = curPortParams.portFecMode;

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType > _100GMII_E)
    {
        hwsOsPrintf("mvHwsMtiPCS100Rev2Mode: bad MII type for port mode %d\n", portMode);
        return GT_BAD_PARAM;
    }

    rc = mvHwsMarkerIntervalCalc(devNum, portMode, portFecMode, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(markerInterval != 0)
    {
        pcsField = PCS100_UNITS_VENDOR_VL_INTVL_P0_MARKER_COUNTER_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, markerInterval, NULL));
    }

    if (HWS_DEV_SILICON_TYPE(devNum) == AC5X)
    {
        return GT_OK;
    }

    if((miiType == _5GMII_E) || (miiType == _10GMII_E) || (miiType == _25GMII_E))
    {
        regValue = (miiType == _25GMII_E) ? 1 : 0;
        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_HI_BER25_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, regValue, NULL));

        regValue = (miiType == _5GMII_E) ? 1 : 0;
        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_HI_BER5_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, regValue, NULL));

        if(markerInterval == 0)
        {
            pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_DISABLE_MLD_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 1, NULL));
        }
        else
        {
            pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_DISABLE_MLD_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0, NULL));

            pcsField = PCS100_UNITS_VL0_0_P0_VL0_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x68c1, NULL));

            pcsField = PCS100_UNITS_VL0_1_P0_VL0_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x21, NULL));

            pcsField = PCS100_UNITS_VL1_0_P0_VL1_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xc4f0, NULL));

            pcsField = PCS100_UNITS_VL1_1_P0_VL1_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xe6, NULL));

            pcsField = PCS100_UNITS_VL2_0_P0_VL2_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x65c5, NULL));

            pcsField = PCS100_UNITS_VL2_1_P0_VL2_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x9b, NULL));

            pcsField = PCS100_UNITS_VL3_0_P0_VL3_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x79a2, NULL));

            pcsField = PCS100_UNITS_VL3_1_P0_VL3_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x3d, NULL));
        }
        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_ENA_CLAUSE49_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 1, NULL));
    }
    else if((miiType == _40GMII_E) || (miiType == _50GMII_E))
    {
        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_HI_BER25_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0, NULL));

        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_HI_BER5_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0, NULL));

        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_DISABLE_MLD_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0, NULL));

        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_ENA_CLAUSE49_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0, NULL));

        /* Marker values for 50G_R2,50G_R1 and 40G_R4 (no FEC dependency)*/
        pcsField = PCS100_UNITS_VL0_0_P0_VL0_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x7690, NULL));

        pcsField = PCS100_UNITS_VL0_1_P0_VL0_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x47, NULL));

        pcsField = PCS100_UNITS_VL1_0_P0_VL1_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xc4f0, NULL));

        pcsField = PCS100_UNITS_VL1_1_P0_VL1_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xe6, NULL));

        pcsField = PCS100_UNITS_VL2_0_P0_VL2_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x65c5, NULL));

        pcsField = PCS100_UNITS_VL2_1_P0_VL2_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x9b, NULL));

        pcsField = PCS100_UNITS_VL3_0_P0_VL3_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x79a2, NULL));

        pcsField = PCS100_UNITS_VL3_1_P0_VL3_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x3d, NULL));
    }
    else if(miiType == _100GMII_E)
    {
        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_HI_BER25_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0, NULL));

        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_HI_BER5_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0, NULL));

        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_DISABLE_MLD_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0, NULL));

        pcsField = PCS100_UNITS_VENDOR_PCS_MODE_P0_ENA_CLAUSE49_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0, NULL));

        /* Marker values for 100G-KR4 / 100G-KR2 (no FEC dependency) */
        pcsField = PCS100_UNITS_VL0_0_P0_VL0_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x68c1, NULL));
        pcsField = PCS100_UNITS_VL0_1_P0_VL0_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x21, NULL));

        pcsField = PCS100_UNITS_VL1_0_P0_VL1_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x719d, NULL));
        pcsField = PCS100_UNITS_VL1_1_P0_VL1_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x8e, NULL));

        pcsField = PCS100_UNITS_VL2_0_P0_VL2_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x4b59, NULL));
        pcsField = PCS100_UNITS_VL2_1_P0_VL2_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xe8, NULL));

        pcsField = PCS100_UNITS_VL3_0_P0_VL3_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x954d, NULL));
        pcsField = PCS100_UNITS_VL3_1_P0_VL3_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x7b, NULL));

        pcsField = PCS100_UNITS_VL4_0_P0_VL4_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x7f5, NULL));
        pcsField = PCS100_UNITS_VL4_1_P0_VL4_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x9, NULL));

        pcsField = PCS100_UNITS_VL5_0_P0_VL5_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x14dd, NULL));
        pcsField = PCS100_UNITS_VL5_1_P0_VL5_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xc2, NULL));

        pcsField = PCS100_UNITS_VL6_0_P0_VL6_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x4a9a, NULL));
        pcsField = PCS100_UNITS_VL6_1_P0_VL6_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x26, NULL));

        pcsField = PCS100_UNITS_VL7_0_P0_VL7_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x457b, NULL));
        pcsField = PCS100_UNITS_VL7_1_P0_VL7_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x66, NULL));

        pcsField = PCS100_UNITS_VL8_0_P0_VL8_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x24a0, NULL));
        pcsField = PCS100_UNITS_VL8_1_P0_VL8_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x76, NULL));

        pcsField = PCS100_UNITS_VL9_0_P0_VL9_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xc968, NULL));
        pcsField = PCS100_UNITS_VL9_1_P0_VL9_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xfb, NULL));

        pcsField = PCS100_UNITS_VL10_0_P0_VL10_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x6cfd, NULL));
        pcsField = PCS100_UNITS_VL10_1_P0_VL10_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x99, NULL));

        pcsField = PCS100_UNITS_VL11_0_P0_VL11_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x91b9, NULL));
        pcsField = PCS100_UNITS_VL11_1_P0_VL11_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x55, NULL));

        pcsField = PCS100_UNITS_VL12_0_P0_VL12_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xb95c, NULL));
        pcsField = PCS100_UNITS_VL12_1_P0_VL12_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xb2, NULL));

        pcsField = PCS100_UNITS_VL13_0_P0_VL13_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xf81a, NULL));
        pcsField = PCS100_UNITS_VL13_1_P0_VL13_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xbd, NULL));

        pcsField = PCS100_UNITS_VL14_0_P0_VL14_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xc783, NULL));
        pcsField = PCS100_UNITS_VL14_1_P0_VL14_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xca, NULL));

        pcsField = PCS100_UNITS_VL15_0_P0_VL15_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x3635, NULL));
        pcsField = PCS100_UNITS_VL15_1_P0_VL15_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xcd, NULL));

        pcsField = PCS100_UNITS_VL16_0_P0_VL16_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x31c4, NULL));
        pcsField = PCS100_UNITS_VL16_1_P0_VL16_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x4c, NULL));

        pcsField = PCS100_UNITS_VL17_0_P0_VL17_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xd6ad, NULL));
        pcsField = PCS100_UNITS_VL17_1_P0_VL17_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xb7, NULL));

        pcsField = PCS100_UNITS_VL18_0_P0_VL18_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x665f, NULL));
        pcsField = PCS100_UNITS_VL18_1_P0_VL18_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0x2a, NULL));

        pcsField = PCS100_UNITS_VL19_0_P0_VL19_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xf0c0, NULL));
        pcsField = PCS100_UNITS_VL19_1_P0_VL19_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS100_UNIT, pcsField, 0xe5, NULL));
    }
    else
    {
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs100Rev2LoopBack function
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
static GT_STATUS mvHwsMtiPcs100Rev2LoopBack
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
    MV_HWS_PCS100_UNIT_FIELDS_E pcsField;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = PCS100_UNITS_CONTROL1_P0_LOOPBACK_E;
    data = (lbType == TX_2_RX_LB) ? 1 : 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS100_UNIT, pcsField, data, NULL));

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
* @internal mvHwsMtiPcs100Rev2LoopBackGet function
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
static GT_STATUS mvHwsMtiPcs100Rev2LoopBackGet
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
    MV_HWS_PCS100_UNIT_FIELDS_E pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = PCS100_UNITS_CONTROL1_P0_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS100_UNIT, pcsField, &data, NULL));

    *lbType = (data != 0) ? TX_2_RX_LB : DISABLE_LB;
    return GT_OK;
}

/**
* @internal mvHwsMtiPcs100Rev2AlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiPCS100.
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
static GT_STATUS mvHwsMtiPcs100Rev2AlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_UOPT                 portGroup,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
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
* @internal mvHwsMtiPcs100Rev2SendFaultSet function
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
static GT_STATUS mvHwsMtiPcs100Rev2SendFaultSet
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
* @internal mvHwsMtiPcs100Rev2FecConfigGet function
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
static GT_STATUS mvHwsMtiPcs100Rev2FecConfigGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    if(portFecTypePtr == NULL)
    {
        return GT_BAD_PTR;
    }
    return  mvHwsExtFecTypeGet(devNum, portGroup, phyPortNum, portMode, portFecTypePtr);
}

/**
* @internal mvHwsMtiPcs100Rev2CheckGearBox function
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
static GT_STATUS mvHwsMtiPcs100Rev2CheckGearBox
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_UREG_DATA                    data;
    MV_HWS_PCS100_UNIT_FIELDS_E     pcsField;
    MV_HWS_HAWK_CONVERT_STC         convertIdx;


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
        pcsField = PCS100_UNITS_BASER_STATUS1_P0_BLOCK_LOCK_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS100_UNIT, pcsField, &data, NULL));
        *lockPtr = (data & 0x1) ? GT_TRUE : GT_FALSE;
    }


    return GT_OK;
}

/**
* @internal mvHwsMtiPcs100Rev2SendLocalFaultSet function
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
static GT_STATUS mvHwsMtiPcs100Rev2SendLocalFaultSet
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
* @internal mvHwsMtiPcs100Rev2IfInit function
* @endinternal
*
* @brief   Init MtiPCS100Rev2 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs100Rev2IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
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
    funcPtrArray[MTI_PCS_100]->pcsResetFunc     = mvHwsMtiPcs100Rev2Reset;
    funcPtrArray[MTI_PCS_100]->pcsModeCfgFunc   = mvHwsMtiPcs100Rev2Mode;
    funcPtrArray[MTI_PCS_100]->pcsLbCfgFunc     = mvHwsMtiPcs100Rev2LoopBack;
    funcPtrArray[MTI_PCS_100]->pcsLbCfgGetFunc = mvHwsMtiPcs100Rev2LoopBackGet;
    funcPtrArray[MTI_PCS_100]->pcsTypeGetFunc   = mvHwsMtiPcs100Rev2TypeGetFunc;
    funcPtrArray[MTI_PCS_100]->pcsFecCfgGetFunc = mvHwsMtiPcs100Rev2FecConfigGet;
    funcPtrArray[MTI_PCS_100]->pcsCheckGearBoxFunc = mvHwsMtiPcs100Rev2CheckGearBox;
    funcPtrArray[MTI_PCS_100]->pcsAlignLockGetFunc = mvHwsMtiPcs100Rev2AlignLockGet;
    funcPtrArray[MTI_PCS_100]->pcsSendFaultSetFunc = mvHwsMtiPcs100Rev2SendFaultSet;
    funcPtrArray[MTI_PCS_100]->pcsSendLocalFaultSetFunc = mvHwsMtiPcs100Rev2SendLocalFaultSet;

    return GT_OK;
}

