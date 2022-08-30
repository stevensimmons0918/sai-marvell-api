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
* @file mvHwsMtiUsxLowSpeedPcsRev2If.c
*
* @brief MTI USX Low Speed PCS interface API
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
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiLowSpeedPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>

static char* mvHwsMtiUsxLowSpeedPcsTypeGetFunc(void)
{
  return "MTI_USX_LOW_SPEED_PCS";
}

/**
* @internal mvHwsMtiUsxLowSpeedPcsMode function
* @endinternal
*
* @brief   Set the LPCS mode
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
static GT_STATUS mvHwsMtiUsxLowSpeedPcsMode
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    GT_STATUS rc;
    GT_U32 baseAddr, unitIndex, unitNum;
    MV_HWS_UNITS_ID unitId = MTI_USX_LPCS_UNIT;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_LPCS_UNITS_FIELDS_E  fieldName;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;
    GT_U32 regAddr, fieldStart, fieldLength;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*CHECK_STATUS(mvHwsMtiUsxmCfg(devNum, portGroup, portMacNum, portMode));*/

    /* configure low speed ports */
    switch(portMode)
    {
        case _1000Base_X:   /* single */
        case SGMII2_5:
        case SGMII:
        case _2500Base_X:
            /* m_RAL.lpcs_units_RegFile.GMODE.Lpcs_enable.set(lpcs_enable); */
            CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, portMacNum, &baseAddr, &unitIndex, &unitNum ));
            rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portMacNum, portMode, &convertIdx);
            CHECK_STATUS(rc);
            rc = genUnitRegDbEntryGet(devNum, portMacNum, unitId, portMode, LPCS_UNITS_GMODE_LPCS_ENABLE_E, &fieldReg, &convertIdx);
            CHECK_STATUS(rc);

            fieldLength = 1;
            regAddr = baseAddr + fieldReg.regOffset;
            fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit % 8;

            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, 1));
            if ((portMode == SGMII) || (portMode == SGMII2_5))
            {
                /* m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Sgmii_ena.set(1'b1); //Choses between 1000BASE-X and SGMII */
                fieldName = LPCS_UNITS_IF_MODE_P0_SGMII_ENA_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 1, NULL));

                /* m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Use_sgmii_an.set(1'b0); //Disable AutoNeg */
                fieldName = LPCS_UNITS_IF_MODE_P0_USE_SGMII_AN_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 0, NULL));
            }
            else
            {
                fieldName = LPCS_UNITS_IF_MODE_P0_SGMII_ENA_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 0, NULL));
            }

            if (portMode == _2500Base_X)
            {
                fieldName = LPCS_UNITS_IF_MODE_P0_IFMODE_SEQ_ENA_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 1, NULL));
            }
            /* m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Sgmii_speed.set(sgmii_speed);  - done in mvHwsMtiUsxReplicationSet*/
            break;


        case _5G_QUSGMII:   /* quad */
        case QSGMII:

            CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, portMacNum, &baseAddr, &unitIndex, &unitNum ));
            rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portMacNum, portMode, &convertIdx);
            CHECK_STATUS(rc);

            rc = genUnitRegDbEntryGet(devNum, portMacNum, unitId, portMode, LPCS_UNITS_GMODE_LPCS_ENABLE_E, &fieldReg, &convertIdx);
            CHECK_STATUS(rc);

            fieldLength = 1;
            regAddr = baseAddr + fieldReg.regOffset;
            fieldStart = fieldReg.fieldStart + convertIdx.ciderIndexInUnit % 8;

            CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, 0, regAddr, fieldStart, fieldLength, 1));

            /* m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Sgmii_ena.set(1'b1); //Choses between 1000BASE-X and SGMII */
            fieldName = LPCS_UNITS_IF_MODE_P0_SGMII_ENA_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 1, NULL));

            /* m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Use_sgmii_an.set(1'b0); //Disable AutoNeg */
            fieldName = LPCS_UNITS_IF_MODE_P0_USE_SGMII_AN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 0, NULL));

            fieldName = LPCS_UNITS_GMODE_QSGMII_0_ENABLE_E + (convertIdx.ciderIndexInUnit % 8 ) / 4;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 1, NULL));

            fieldName = LPCS_UNITS_GMODE_USGMII8_ENABLE_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 0, NULL));
            fieldName = LPCS_UNITS_GMODE_USGMII_SCRAMBLE_ENABLE_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 0, NULL));

            break;

        case _10G_OUSGMII:  /* octal */

            /* m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Sgmii_ena.set(1'b1); //Choses between 1000BASE-X and SGMII */
            fieldName = LPCS_UNITS_IF_MODE_P0_SGMII_ENA_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 1, NULL));

            /* m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Use_sgmii_an.set(1'b0); //Disable AutoNeg */
            fieldName = LPCS_UNITS_IF_MODE_P0_USE_SGMII_AN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 0, NULL));

            fieldName = LPCS_UNITS_GMODE_USGMII8_ENABLE_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 1, NULL));
            fieldName = LPCS_UNITS_GMODE_USGMII_SCRAMBLE_ENABLE_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 1, NULL));

            fieldName = LPCS_UNITS_GMODE_QSGMII_0_ENABLE_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 0, NULL));
            fieldName = LPCS_UNITS_GMODE_QSGMII_1_ENABLE_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, fieldName, 0, NULL));

            break;

        default:
            break;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxLowSpeedPcsReset function
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
static GT_STATUS mvHwsMtiUsxLowSpeedPcsReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_HWS_USXM_UNITS_FIELDS_E fieldName;

    GT_UNUSED_PARAM(portGroup);

    if(POWER_DOWN == action)
    {
        /*******************************************/
        /*  USX USXM part                          */
        /*******************************************/
        /* m_USXM.ConfigureActivePorts();  */
        fieldName = USXM_UNITS_PORTS_ENA_ACTIVE_PORTS_USED_E;
        CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_MULTIPLEXER_UNIT, portMode, fieldName, 0, NULL));
        /* m_USXM.Configure(); */
        fieldName = USXM_UNITS_VL_INTVL_PORT_CYCLE_INTERVAL_E;
        CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_MULTIPLEXER_UNIT, portMode, fieldName, 0x1004, NULL));

        /*******************************************/
        /*  USX LPCS part                          */
        /*******************************************/
        fieldName = LPCS_UNITS_IF_MODE_P0_SGMII_SPEED_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, fieldName, 0, NULL));
        fieldName = LPCS_UNITS_IF_MODE_P0_SGMII_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, fieldName, 0, NULL));
        fieldName = LPCS_UNITS_IF_MODE_P0_USE_SGMII_AN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, fieldName, 0, NULL));
        fieldName = LPCS_UNITS_IF_MODE_P0_IFMODE_SEQ_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, fieldName, 0, NULL));

        fieldName = LPCS_UNITS_GMODE_LPCS_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, fieldName, 0, NULL));
        fieldName = LPCS_UNITS_GMODE_USGMII8_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, fieldName, 0, NULL));
        fieldName = LPCS_UNITS_GMODE_USGMII_SCRAMBLE_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, fieldName, 0, NULL));
        fieldName = LPCS_UNITS_GMODE_QSGMII_0_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, fieldName, 0, NULL));
        fieldName = LPCS_UNITS_GMODE_QSGMII_1_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, fieldName, 0, NULL));


    }

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxLowSpeedPcsLoopBack function
* @endinternal
*
* @brief   Set PCS loopback.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiUsxLowSpeedPcsLoopBack
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
    MV_HWS_LPCS_UNITS_FIELDS_E  fieldName;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(lbType == TX_2_RX_LB)
    {
        data = 1;
    }
    else if(lbType == DISABLE_LB)
    {
        data = 0;
    }
    else
    {
        return GT_BAD_STATE;
    }

    fieldName = LPCS_UNITS_CONTROL_P0_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_USX_LPCS_UNIT, fieldName, data, NULL));

    extField = MTIP_USX_EXT_UNITS_CONTROL1_P0_FORCE_LINK_OK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_USX_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxLowSpeedPcsLoopBackGet function
* @endinternal
*
* @brief   Get the PCS loopback mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] lbTypePtr                - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiUsxLowSpeedPcsLoopBackGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbTypePtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;
    MV_HWS_LPCS_UNITS_FIELDS_E  fieldName;

    if(lbTypePtr == NULL)
    {
        return GT_BAD_PTR;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    fieldName = LPCS_UNITS_CONTROL_P0_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_USX_LPCS_UNIT, fieldName, &data, NULL));

    *lbTypePtr = (data != 0) ? TX_2_RX_LB : DISABLE_LB;
    return GT_OK;

}

/**
* @internal mmvHwsMtiUsxLowSpeedPcsFecConfigGet function
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
GT_STATUS mvHwsMtiUsxLowSpeedPcsFecConfigGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    return  mvHwsExtFecTypeGet(devNum, portGroup, phyPortNum, portMode, portFecTypePtr);
}

/**
* @internal mvHwsMtiUsxLowSpeedPcsRemoteFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending remote fault signals to partner.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] send                     - send/stop remote fault
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiUsxLowSpeedPcsRemoteFaultSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  GT_BOOL                 send
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

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

    extField = MTIP_USX_EXT_UNITS_CONTROL_P0_TX_REM_FAULT_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_USX_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxLowSpeedPcsLocalFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending local fault signals to partner.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] send                     - send/stop local fault
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiUsxLowSpeedPcsLocalFaultSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  GT_BOOL                 send
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

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

    extField = MTIP_USX_EXT_UNITS_CONTROL_P0_TX_LOC_FAULT_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_USX_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
 * @internal mvHwsMtiLowSpeedPcsAutoNeg1GSgmii function
 * @endinternal
 *
 * @brief   Configure Auto-negotiation for SGMII/1000BaseX port
 *          modes.
 *
 *
 * @param devNum
 * @param phyPortNum
 * @param portMode
 * @param autoNegotiationPtr
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsMtiUsxLowSpeedPcsAutoNeg1GSgmii
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portMacNum,
    IN  MV_HWS_PORT_STANDARD        portMode,
    IN  MV_HWS_PCS_AUTONEG_1G_SGMII *autoNegPtr
)
{
    GT_UREG_DATA    data;
    GT_BOOL enable = (autoNegPtr->inbandAnEnable) && !(autoNegPtr->byPassEnable);

        /*
         * 1.  For both 1000Base-X and SGMII, need to enable the AN by setting:
         *      /<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_CONTROL
         *      Bit[12] = 0x1.
         */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_CONTROL_P0_ANENABLE_E, enable, NULL));

    switch (portMode) {
    case _10G_OUSGMII:
    case _5G_QUSGMII:
    case QSGMII:
    case SGMII:
        /*
         * 2.  For SGMII, need to configure the
         *      link timer. Each with different value. Registers:
         *       /<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_LINK_TIMER_1
         *       /<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_LINK_TIMER_0
         *     For SGMII the standard requires 1.6ms, which means:
         *      Link_Timer_1[4:0] = 0x3.
         *      Link_Timer_0[15:0] = 0xd40.
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_LINK_TIMER_0_P0_TIMER0_E, 0, NULL));
        data = ((enable) ? (0xd40>>1) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_LINK_TIMER_0_P0_TIMER15_1_E, data, NULL));
        data = ((enable) ? (0x3) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_LINK_TIMER_1_P0_TIMER20_16_E, data, NULL));
        /*
         * 3.  For SGMII/QSGMII only:
         */
        if (SGMII == portMode || QSGMII == portMode)
        {
          /**
            *  We wish to let the HW set the speed automatically once AN is done.
            *  This is done by writing to following register:
            *  /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_IF_MODE
            *  Bit[1] = 0x1.
            *  In this case, bit[3:2] are don’t care.
            */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_IF_MODE_P0_USE_SGMII_AN_E, enable, NULL));
        }
        else
        {
           /**
             *      For OUSGMII we must use the SW set speed flow
             *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_IF_MODE
             *      Bit[1] = 0x0
             *      Set bit[3:2] in  mvHwsMtiUsxReplicationSet function
             *
             *      TBD QUSGMII
             */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_IF_MODE_P0_USE_SGMII_AN_E, 0, NULL));
        }

        /*
         * 4.  Configure device ability:
         *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
         *      Set bit[0] to 0x1 and set all other bits to 0.
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_DEV_ABILITY_P0_LD_ABILITY_RSV05_E, enable, NULL));
        break;

    case _1000Base_X:
       /*
         * 2.  For 1000Base-X the standard requires 10ms, which means:
         *      Link_Timer_1[4:0] = 0x13.
         *      Link_Timer_0[15:0] = 0x12d0. (bit[0] is part of the value although it is RO)
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_LINK_TIMER_0_P0_TIMER0_E, 0, NULL));
        data = ((enable) ? (0x12d0>>1) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_LINK_TIMER_0_P0_TIMER15_1_E, data, NULL));
        data = ((enable) ? (0x13) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_LINK_TIMER_1_P0_TIMER20_16_E, data, NULL));
        /*
         * 3.  Configure device ability:
         *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
         *      Since we support only Full-duplex, set bit[5] to 0x1 and bit[6] to 0x0.
         *      Set bit[7] 'PS1' - PAUSE and bit[8] 'PS2' - ASM_DIR
         *      Set all other bits to 0
         */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_DEV_ABILITY_P0_LD_FD_E, 1, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_DEV_ABILITY_P0_LD_HD_E, 0, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_DEV_ABILITY_P0_LD_PS1_E, ((enable) ? BOOL2BIT_MAC(autoNegPtr->flowCtrlPauseAdvertiseEnable) : (0x1)), NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_LPCS_UNIT, LPCS_UNITS_DEV_ABILITY_P0_LD_PS2_E, ((enable) ? BOOL2BIT_MAC(autoNegPtr->flowCtrlAsmAdvertiseEnable) : (0x1)), NULL));

        break;
    default:
        return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxLowSpeedPcsAlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiUsxPcs.
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
static GT_STATUS mvHwsMtiUsxLowSpeedPcsAlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    GT_UREG_DATA                  data;
    MV_HWS_PORT_INIT_PARAMS       curPortParams;
    MV_HWS_MTIP_USX_EXT_UNITS_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    extField = MTIP_USX_EXT_UNITS_STATUS_P0_LPCS_LINK_STATUS_E;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_USX_EXT_UNIT, extField, &data, NULL));

    *lockPtr = (data & 0x1);

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxLowSpeedPcsRev2IfInit function
* @endinternal
*
* @brief   Init MtiUsxPcsLowSpeed configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiUsxLowSpeedPcsRev2IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_USX_PCS_LOW_SPEED])
    {
        funcPtrArray[MTI_USX_PCS_LOW_SPEED] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_USX_PCS_LOW_SPEED])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_USX_PCS_LOW_SPEED], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_USX_PCS_LOW_SPEED]->pcsResetFunc               = mvHwsMtiUsxLowSpeedPcsReset;
    funcPtrArray[MTI_USX_PCS_LOW_SPEED]->pcsModeCfgFunc             = mvHwsMtiUsxLowSpeedPcsMode;
    funcPtrArray[MTI_USX_PCS_LOW_SPEED]->pcsLbCfgFunc               = mvHwsMtiUsxLowSpeedPcsLoopBack;
    funcPtrArray[MTI_USX_PCS_LOW_SPEED]->pcsLbCfgGetFunc            = mvHwsMtiUsxLowSpeedPcsLoopBackGet;
    funcPtrArray[MTI_USX_PCS_LOW_SPEED]->pcsTypeGetFunc             = mvHwsMtiUsxLowSpeedPcsTypeGetFunc;
    funcPtrArray[MTI_USX_PCS_LOW_SPEED]->pcsFecCfgGetFunc           = mvHwsMtiUsxLowSpeedPcsFecConfigGet;
    funcPtrArray[MTI_USX_PCS_LOW_SPEED]->pcsSendFaultSetFunc        = mvHwsMtiUsxLowSpeedPcsRemoteFaultSet;
    funcPtrArray[MTI_USX_PCS_LOW_SPEED]->pcsSendLocalFaultSetFunc   = mvHwsMtiUsxLowSpeedPcsLocalFaultSet;
    funcPtrArray[MTI_USX_PCS_LOW_SPEED]->pcsAutoNeg1GSgmiiFunc      = mvHwsMtiUsxLowSpeedPcsAutoNeg1GSgmii;
    funcPtrArray[MTI_USX_PCS_LOW_SPEED]->pcsAlignLockGetFunc        = mvHwsMtiUsxLowSpeedPcsAlignLockGet;
    return GT_OK;
}


