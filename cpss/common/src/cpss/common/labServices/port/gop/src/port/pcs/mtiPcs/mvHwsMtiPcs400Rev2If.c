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
* @file mvHwsMtiPcs400Rev2If.c
*
* @brief MTI PCS400 interface API
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

static char* mvHwsMtiPcs400Rev2TypeGetFunc(void)
{
  return "MTI_PCS400";
}

/**
* @internal mvHwsMtiPcs400Rev2Reset function
* @endinternal
*
* @brief   Set MTI400 PCS RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiPcs400Rev2Reset
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
    MV_HWS_PCS400_UNITS_FIELDS_E pcsField;
    MV_HWS_UNITS_ID unitId;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    if(HWS_DEV_SILICON_TYPE(devNum) == Harrier)
    {
        if ((pcsNum % MV_HWS_HARRIER_GOP_PORT_NUM_CNS) == 0)
        {
            pcsField = PCS400_UNITS_CONTROL1_RESET_E;
            unitId = MTI_PCS400_UNIT;
        }
        else
        {
            pcsField = PCS200_UNITS_CONTROL1_RESET_E;
            unitId = MTI_PCS200_UNIT;
        }
    }
    else if (pcsNum % MV_HWS_AC5P_GOP_PORT_NUM_CNS == 0)
    {
        pcsField = PCS400_UNITS_CONTROL1_RESET_E;
        unitId = MTI_PCS400_UNIT;
    }
    else
    {
        pcsField = PCS200_UNITS_CONTROL1_RESET_E;
        unitId = MTI_PCS200_UNIT;
    }
    if ((POWER_DOWN == action) || (RESET == action) || (PARTIAL_POWER_DOWN == action))
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, unitId, pcsField, 0, NULL));
    }
    else if (UNRESET == action)
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, unitId, pcsField, 1, NULL));
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs400Rev2Mode function
* @endinternal
*
* @brief   Set the MtiPCS400 mode
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
static GT_STATUS mvHwsMtiPcs400Rev2Mode
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
    MV_HWS_PCS400_UNITS_FIELDS_E pcsField;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 regValue, txCdmiiVal, controlVal;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    portFecMode = curPortParams.portFecMode;

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType > _400GMII_E)
    {
        hwsOsPrintf("mvHwsMtiPCS400Rev2Mode: bad MII type for port mode %d\n", portMode);
        return GT_BAD_PARAM;
    }

    rc = mvHwsMarkerIntervalCalc(devNum, portMode, portFecMode, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }
    if(miiType == _400GMII_E)
    {
        markerInterval = ((2 * (markerInterval + 1)) / 5);
        txCdmiiVal = 4;
        controlVal = 0;
    }
    else
    {
        markerInterval = ((markerInterval + 1) / 5);
        txCdmiiVal = 8;
        controlVal = 1;
    }

   /* m_RAL.VENDOR_TX_LANE_THRESH.Tx_lanethresh.set(9);
    m_RAL.VENDOR_TX_LANE_THRESH.update(status);

    m_RAL.VENDOR_VL_INTVL.Vl_intvl.set(MarkerInterval);
    m_RAL.VENDOR_VL_INTVL.update(status);

    // Common Marker values same as 400G
    m_RAL.VENDOR_AM_0.Cm0.set(8'h9a);
    m_RAL.VENDOR_AM_0.Cm1.set(8'h4a);
    m_RAL.VENDOR_AM_0.update(status);

    m_RAL.VENDOR_AM_1.Cm2.set(8'h26);
    m_RAL.VENDOR_AM_1.update(status);*/

    regValue = 9;
    pcsField = PCS400_UNITS_VENDOR_TX_LANE_THRESH_TX_LANETHRESH_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS400_UNIT, pcsField, regValue, NULL));

    regValue = markerInterval;
    pcsField = PCS400_UNITS_VENDOR_VL_INTVL_VL_INTVL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS400_UNIT, pcsField, regValue, NULL));


    regValue = 0x9a;
    pcsField = PCS400_UNITS_VENDOR_AM_0_CM0_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS400_UNIT, pcsField, regValue, NULL));

    regValue = 0x4a;
    pcsField = PCS400_UNITS_VENDOR_AM_0_CM1_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS400_UNIT, pcsField, regValue, NULL));

    regValue = 0x26;
    pcsField = PCS400_UNITS_VENDOR_AM_1_CM2_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS400_UNIT, pcsField, regValue, NULL));

    pcsField = PCS400_UNITS_VENDOR_TX_CDMII_PACE_TX_CDMII_PACE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS400_UNIT, pcsField, txCdmiiVal, NULL));

    pcsField = PCS400_UNITS_CONTROL1_SPEED_SELECTION_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS400_UNIT, pcsField, controlVal, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400Rev2LoopBack function
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
static GT_STATUS mvHwsMtiPcs400Rev2LoopBack
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
    MV_HWS_PCS400_UNITS_FIELDS_E pcsField;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = PCS400_UNITS_CONTROL1_LOOPBACK_E;
    data = (lbType == TX_2_RX_LB) ? 1 : 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS400_UNIT, pcsField, data, NULL));

    extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_SEG_P0_FORCE_LINK_OK_EN_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_EXT_UNIT, portMode, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400Rev2LoopBackGet function
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
static GT_STATUS mvHwsMtiPcs400Rev2LoopBackGet
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
    MV_HWS_PCS400_UNITS_FIELDS_E pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = PCS400_UNITS_CONTROL1_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS400_UNIT, pcsField, &data, NULL));

    *lbType = (data != 0) ? TX_2_RX_LB : DISABLE_LB;
    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400Rev2AlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiPCS400.
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
static GT_STATUS mvHwsMtiPcs400Rev2AlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_UOPT                 portGroup,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lock
)
{
    GT_UREG_DATA                  data;
    MV_HWS_HAWK_MTIP_EXT_BR_UNIT_FIELDS_E extField = HAWK_MTIP_EXT_BR_UNITS_STATUS_SEG_P0_LINK_STATUS_E;
    MV_HWS_PORT_INIT_PARAMS       curPortParams;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_EXT_UNIT, portMode, extField, &data, NULL));

    *lock = (data != 0);

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400Rev2SendFaultSet function
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
static GT_STATUS mvHwsMtiPcs400Rev2SendFaultSet
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

    extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_SEG_P0_TX_REM_FAULT_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, pcsNum, MTI_EXT_UNIT, portMode, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400Rev2SendLocalFaultSet function
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
static GT_STATUS mvHwsMtiPcs400Rev2SendLocalFaultSet
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

    extField = HAWK_MTIP_EXT_BR_UNITS_CONTROL_SEG_P0_TX_LOC_FAULT_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, pcsNum, MTI_EXT_UNIT, portMode, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400Rev2FecConfigGet function
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
static GT_STATUS mvHwsMtiPcs400Rev2FecConfigGet
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
* @internal mvHwsMtiPcs400Rev2IfInit function
* @endinternal
*
* @brief   Init MtiPCS400Rev2 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs400Rev2IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_PCS_400])
    {
        funcPtrArray[MTI_PCS_400] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_PCS_400])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_PCS_400], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_PCS_400]->pcsResetFunc     = mvHwsMtiPcs400Rev2Reset;
    funcPtrArray[MTI_PCS_400]->pcsModeCfgFunc   = mvHwsMtiPcs400Rev2Mode;
    funcPtrArray[MTI_PCS_400]->pcsLbCfgFunc     = mvHwsMtiPcs400Rev2LoopBack;
    funcPtrArray[MTI_PCS_400]->pcsLbCfgGetFunc = mvHwsMtiPcs400Rev2LoopBackGet;
    funcPtrArray[MTI_PCS_400]->pcsTypeGetFunc   = mvHwsMtiPcs400Rev2TypeGetFunc;
    funcPtrArray[MTI_PCS_400]->pcsFecCfgGetFunc = mvHwsMtiPcs400Rev2FecConfigGet;
    funcPtrArray[MTI_PCS_400]->pcsAlignLockGetFunc = mvHwsMtiPcs400Rev2AlignLockGet;
    funcPtrArray[MTI_PCS_400]->pcsSendFaultSetFunc = mvHwsMtiPcs400Rev2SendFaultSet;
    funcPtrArray[MTI_PCS_400]->pcsSendLocalFaultSetFunc = mvHwsMtiPcs400Rev2SendLocalFaultSet;

    return GT_OK;
}

