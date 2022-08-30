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
* @file mvHwsMtiUsxPcsRev2If.c
*
* @brief MTI USX PCS interface API
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
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiUsxPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>

static char* mvHwsMtiUsxPcsTypeGetFunc(void)
{
  return "MTI_USX_LOW_SPEED_PCS";
}


/**
* @internal mvHwsMtiUsxPcsMode function
* @endinternal
*
* @brief   Set the USX PCS mode
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
static GT_STATUS mvHwsMtiUsxPcsMode
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_USX_PCS_UNITS_FIELDS_E  fieldName;
    GT_U32 regValue;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 markerInterval;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*CHECK_STATUS(mvHwsMtiUsxmCfg(devNum, portGroup, portMacNum, portMode));*/

    if (hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        if((portMode == _25GBase_KR) || (portMode == _10GBase_KR) || (portMode == _5GBaseR))
        {
            MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);

            CHECK_STATUS(mvHwsMarkerIntervalCalc(devNum, portMode, curPortParams.portFecMode, &markerInterval));

            regValue = (miiType == _25GMII_E) ? 1 : 0;
            fieldName = USX_PCS_UNITS_VENDOR_PCS_MODE_P0_HI_BER25_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_PCS_UNIT, fieldName, regValue, NULL));

            regValue = (miiType == _5GMII_E) ? 1 : 0;
            fieldName = USX_PCS_UNITS_VENDOR_PCS_MODE_P0_HI_BER5_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_PCS_UNIT, fieldName, regValue, NULL));

            fieldName = USX_PCS_UNITS_VENDOR_PCS_MODE_P0_DISABLE_MLD_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_PCS_UNIT, fieldName, ((markerInterval == 0)?1:0), NULL));

            fieldName = USX_PCS_UNITS_VENDOR_PCS_MODE_P0_ENA_CLAUSE49_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_PCS_UNIT, fieldName, 1, NULL));

            return GT_OK;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxPcsReset function
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
static GT_STATUS mvHwsMtiUsxPcsReset
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
        /*  USX PCS part                           */
        /*******************************************/
        fieldName = USX_PCS_UNITS_VENDOR_PCS_MODE_P0_HI_BER25_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_PCS_UNIT, fieldName, 0, NULL));
        fieldName = USX_PCS_UNITS_VENDOR_PCS_MODE_P0_HI_BER5_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_PCS_UNIT, fieldName, 0, NULL));
        fieldName = USX_PCS_UNITS_VENDOR_PCS_MODE_P0_DISABLE_MLD_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_PCS_UNIT, fieldName, 1, NULL));
        fieldName = USX_PCS_UNITS_VENDOR_PCS_MODE_P0_ENA_CLAUSE49_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_PCS_UNIT, fieldName, 1, NULL));


        if((portMode == _2_5G_SXGMII) || (portMode == _5G_SXGMII) || (portMode == _10G_SXGMII))
        {
            /* m_RSFEC.ConfigureToggleEn(); */
            fieldName = USX_RSFEC_UNITS_RSFEC_VENDOR_TX_FIFO_THRESHOLD_TOGGLE_EN_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_USX_RSFEC_UNIT, portMode, fieldName, 0, NULL));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxPcsLoopBack function
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
static GT_STATUS mvHwsMtiUsxPcsLoopBack
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
    MV_HWS_USX_PCS_UNITS_FIELDS_E  fieldName;
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

    fieldName = USX_PCS_UNITS_CONTROL1_P0_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_USX_PCS_UNIT, fieldName, data, NULL));

    extField = MTIP_USX_EXT_UNITS_CONTROL1_P0_FORCE_LINK_OK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_USX_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxPcsCheckGearBox function
* @endinternal
*
* @brief   check if gear box is locked on MtiUsxPcs.
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
static GT_STATUS mvHwsMtiUsxPcsCheckGearBox
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    MV_HWS_USX_PCS_UNITS_FIELDS_E       pcsField;
    MV_HWS_PORT_INIT_PARAMS             curPortParams;
    GT_UREG_DATA                        data;
    MV_HWS_UNITS_ID                     unitId;

    if(lockPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* for RS_FEC modes 'block_lock' field is not relevant */
    if(curPortParams.portFecMode == RS_FEC)
    {
        pcsField = USX_RSFEC_UNITS_RSFEC_STATUS_AMPS_LOCK_E;
        unitId = MTI_USX_RSFEC_UNIT;
    }
    else
    {
        pcsField = MTIP_USX_EXT_UNITS_STATUS_P0_LINK_STATUS_E;
        unitId = MTI_USX_EXT_UNIT;
    }

    CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, portGroup, curPortParams.portPcsNumber, unitId, portMode, pcsField, &data, NULL));

    *lockPtr = (data != 0);

#ifdef ASIC_SIMULATION
    *lockPtr = GT_TRUE;
#endif

    return GT_OK;
}


/**
* @internal mvHwsMtiUsxPcsLoopBackGet function
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
static GT_STATUS mvHwsMtiUsxPcsLoopBackGet
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
    MV_HWS_USX_PCS_UNITS_FIELDS_E  fieldName;

    if(lbTypePtr == NULL)
    {
        return GT_BAD_PTR;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    fieldName = USX_PCS_UNITS_CONTROL1_P0_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_USX_PCS_UNIT, fieldName, &data, NULL));

    *lbTypePtr = (data != 0) ? TX_2_RX_LB : DISABLE_LB;
    return GT_OK;

}

/**
* @internal mvHwsMtiUsxPcsAlignLockGet function
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
static GT_STATUS mvHwsMtiUsxPcsAlignLockGet
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
    MV_HWS_HAWK_MTIP_EXT_UNIT_FIELDS_E extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    extField = MTIP_USX_EXT_UNITS_STATUS_P0_LINK_STATUS_E;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_USX_EXT_UNIT, extField, &data, NULL));

    *lockPtr = (data & 0x1);
#ifdef ASIC_SIMULATION
    *lockPtr = GT_TRUE;
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiUsxPcsFecConfigGet function
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
GT_STATUS mvHwsMtiUsxPcsFecConfigGet
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
* @internal mvHwsMtiUsxPcsRemoteFaultSet function
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
static GT_STATUS mvHwsMtiUsxPcsRemoteFaultSet
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
* @internal mvHwsMtiUsxPcsLocalFaultSet function
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
static GT_STATUS mvHwsMtiUsxPcsLocalFaultSet
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
* @internal mvHwsMtiUsxPcsRev2IfInit function
* @endinternal
*
* @brief   Init MtiUsxPcs configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiUsxPcsRev2IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_USX_PCS])
    {
        funcPtrArray[MTI_USX_PCS] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_USX_PCS])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_USX_PCS], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_USX_PCS]->pcsResetFunc           = mvHwsMtiUsxPcsReset;
    funcPtrArray[MTI_USX_PCS]->pcsModeCfgFunc         = mvHwsMtiUsxPcsMode;
    funcPtrArray[MTI_USX_PCS]->pcsLbCfgFunc           = mvHwsMtiUsxPcsLoopBack;
    funcPtrArray[MTI_USX_PCS]->pcsLbCfgGetFunc        = mvHwsMtiUsxPcsLoopBackGet;
    funcPtrArray[MTI_USX_PCS]->pcsCheckGearBoxFunc    = mvHwsMtiUsxPcsCheckGearBox;
    funcPtrArray[MTI_USX_PCS]->pcsAlignLockGetFunc    = mvHwsMtiUsxPcsAlignLockGet;
    funcPtrArray[MTI_USX_PCS]->pcsTypeGetFunc         = mvHwsMtiUsxPcsTypeGetFunc;
    funcPtrArray[MTI_USX_PCS]->pcsFecCfgGetFunc       = mvHwsMtiUsxPcsFecConfigGet;
    funcPtrArray[MTI_USX_PCS]->pcsSendFaultSetFunc    = mvHwsMtiUsxPcsRemoteFaultSet;
    funcPtrArray[MTI_USX_PCS]->pcsSendLocalFaultSetFunc = mvHwsMtiUsxPcsLocalFaultSet;

    return GT_OK;
}


