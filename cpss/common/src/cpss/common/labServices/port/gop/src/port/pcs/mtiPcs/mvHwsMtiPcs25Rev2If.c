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
* @file mvHwsMtiPcs25Rev2If.c
*
* @brief MTI PCS25 interface API
*
* @version   1
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs25If.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>

static char* mvHwsMtiPcs25Rev2TypeGetFunc(void)
{
  return "MTI_PCS25";
}

/**
* @internal mvHwsMtiPcs25Rev2Reset function
* @endinternal
*
* @brief   Set MTI25 PCS RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiPcs25Rev2Reset
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
    MV_HWS_PCS25_UNIT_FIELDS_E pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    pcsNum = curPortParams.portPcsNumber;

    if((POWER_DOWN == action) || (RESET == action) || (PARTIAL_POWER_DOWN == action))
    {
        value = 0;
        pcsField = PCS25_UNITS_PORT_CONTROL1_PORT_RESET_E;
        if((HWS_DEV_SILICON_TYPE(devNum) == AC5X) && (curPortParams.numOfActLanes == 1))
        {
            value = 1;
        }
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_PCS25_UNIT, pcsField, value, NULL));
    }
    else if (UNRESET == action)
    {
        pcsField = PCS25_UNITS_PORT_CONTROL1_PORT_RESET_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_PCS25_UNIT, pcsField, 1, NULL));
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs25Rev2Mode function
* @endinternal
*
* @brief   Set the MtiPcs25 mode
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
static GT_STATUS mvHwsMtiPcs25Rev2Mode
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
    MV_HWS_PCS25_UNIT_FIELDS_E pcsField;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 regValue;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    portFecMode = curPortParams.portFecMode;

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType >= _50GMII_E)
    {
        hwsOsPrintf("mvHwsMtiPCS25Rev2Mode: bad MII type for port mode %d\n", portMode);
        return GT_BAD_PARAM;
    }

    rc = mvHwsMarkerIntervalCalc(devNum, portMode, portFecMode, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(markerInterval != 0)
    {
        pcsField = PCS25_UNITS_PORT_VL_INTVL_ALIGNMENT_MARKERS_INTERVAL_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, markerInterval, NULL));
    }

    if((miiType == _5GMII_E) || (miiType == _10GMII_E) || (miiType == _25GMII_E))
    {
        regValue = (miiType == _25GMII_E) ? 1 : 0;
        pcsField = PCS25_UNITS_PORT_VENDOR_PCS_MODE_HI_BER25_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, regValue, NULL));

        if(markerInterval == 0)
        {
            pcsField = PCS25_UNITS_PORT_VENDOR_PCS_MODE_DISABLE_MLD_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 1, NULL));
        }
        else
        {
            pcsField = PCS25_UNITS_PORT_VENDOR_PCS_MODE_DISABLE_MLD_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0, NULL));

            pcsField = PCS25_UNITS_PORT_VL0_0_M0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0xc1, NULL));
            pcsField = PCS25_UNITS_PORT_VL0_0_M1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0x68, NULL));
            pcsField = PCS25_UNITS_PORT_VL0_1_M2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0x21, NULL));

            pcsField = PCS25_UNITS_PORT_VL1_0_M0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0xf0, NULL));
            pcsField = PCS25_UNITS_PORT_VL1_0_M1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0xc4, NULL));
            pcsField = PCS25_UNITS_PORT_VL1_1_M2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0xe6, NULL));

            pcsField = PCS25_UNITS_PORT_VL2_0_M0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0xc5, NULL));
            pcsField = PCS25_UNITS_PORT_VL2_0_M1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0x65, NULL));
            pcsField = PCS25_UNITS_PORT_VL2_1_M2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0x9b, NULL));

            pcsField = PCS25_UNITS_PORT_VL3_0_M0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0xa2, NULL));
            pcsField = PCS25_UNITS_PORT_VL3_0_M1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0x79, NULL));
            pcsField = PCS25_UNITS_PORT_VL3_1_M2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 0x3d, NULL));

        }
        pcsField = PCS25_UNITS_PORT_VENDOR_PCS_MODE_ENA_CLAUSE49_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_PCS25_UNIT, pcsField, 1, NULL));
    }
    else
    {
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs25Rev2LoopBack function
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
static GT_STATUS mvHwsMtiPcs25Rev2LoopBack
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
    MV_HWS_PCS25_UNIT_FIELDS_E pcsField;
    MV_HWS_PHOENIX_MTIP_EXT_UNIT_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = PCS25_UNITS_PORT_CONTROL1_PORT_LOOPBACK_E;
    data = (lbType == TX_2_RX_LB) ? 1 : 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS25_UNIT, pcsField, data, NULL));

    extField = PHOENIX_MTIP_EXT_UNITS_CONTROL1_P0_FORCE_LINK_OK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs25Rev2LoopBackGet function
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
static GT_STATUS mvHwsMtiPcs25Rev2LoopBackGet
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
    MV_HWS_PCS25_UNIT_FIELDS_E pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = PCS25_UNITS_PORT_CONTROL1_PORT_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS25_UNIT, pcsField, &data, NULL));

    *lbType = (data != 0) ? TX_2_RX_LB : DISABLE_LB;
    return GT_OK;
}

/**
* @internal mvHwsMtiPcs25Rev2AlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiPcs25.
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
static GT_STATUS mvHwsMtiPcs25Rev2AlignLockGet
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
    MV_HWS_PHOENIX_MTIP_EXT_UNIT_FIELDS_E extField = PHOENIX_MTIP_EXT_UNITS_STATUS_P0_LINK_STATUS_E;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_EXT_UNIT, extField, &data, NULL));

    *lock = (data != 0);
#ifdef ASIC_SIMULATION
    *lock = GT_TRUE;
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs25Rev2SendFaultSet function
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
static GT_STATUS mvHwsMtiPcs25Rev2SendFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    MV_HWS_PHOENIX_MTIP_EXT_UNIT_FIELDS_E extField;

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

    extField = PHOENIX_MTIP_EXT_UNITS_CONTROL_P0_TX_REM_FAULT_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs25Rev2FecConfigGet function
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
static GT_STATUS mvHwsMtiPcs25Rev2FecConfigGet
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
* @internal mvHwsMtiPcs25Rev2CheckGearBox function
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
static GT_STATUS mvHwsMtiPcs25Rev2CheckGearBox
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
    MV_HWS_PCS25_UNIT_FIELDS_E  pcsField;
    MV_HWS_HAWK_CONVERT_STC     convertIdx;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* for RS_FEC modes 'block_lock' field is not relevant */
    if(curPortParams.portFecMode == RS_FEC)
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
        pcsField = PCS25_UNITS_PORT_BASER_STATUS1_BLOCK_LOCK_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS25_UNIT, pcsField, &data, NULL));
        *lockPtr = (data & 0x1) ? GT_TRUE : GT_FALSE;
    }

#ifdef ASIC_SIMULATION
    *lockPtr = GT_TRUE;
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs25Rev2SendLocalFaultSet function
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
static GT_STATUS mvHwsMtiPcs25Rev2SendLocalFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    MV_HWS_PHOENIX_MTIP_EXT_UNIT_FIELDS_E extField;

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

    extField = PHOENIX_MTIP_EXT_UNITS_CONTROL_P0_TX_LOC_FAULT_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs25Rev2IfInit function
* @endinternal
*
* @brief   Init MtiPcs25 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs25Rev2IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_PCS_25])
    {
        funcPtrArray[MTI_PCS_25] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_PCS_25])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_PCS_25], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_PCS_25]->pcsResetFunc     = mvHwsMtiPcs25Rev2Reset;
    funcPtrArray[MTI_PCS_25]->pcsModeCfgFunc   = mvHwsMtiPcs25Rev2Mode;
    funcPtrArray[MTI_PCS_25]->pcsLbCfgFunc     = mvHwsMtiPcs25Rev2LoopBack;
    funcPtrArray[MTI_PCS_25]->pcsLbCfgGetFunc = mvHwsMtiPcs25Rev2LoopBackGet;
    funcPtrArray[MTI_PCS_25]->pcsTypeGetFunc   = mvHwsMtiPcs25Rev2TypeGetFunc;
    funcPtrArray[MTI_PCS_25]->pcsFecCfgGetFunc = mvHwsMtiPcs25Rev2FecConfigGet;
    funcPtrArray[MTI_PCS_25]->pcsCheckGearBoxFunc = mvHwsMtiPcs25Rev2CheckGearBox;
    funcPtrArray[MTI_PCS_25]->pcsAlignLockGetFunc = mvHwsMtiPcs25Rev2AlignLockGet;
    funcPtrArray[MTI_PCS_25]->pcsSendFaultSetFunc = mvHwsMtiPcs25Rev2SendFaultSet;
    funcPtrArray[MTI_PCS_25]->pcsSendLocalFaultSetFunc = mvHwsMtiPcs25Rev2SendLocalFaultSet;

    return GT_OK;
}

