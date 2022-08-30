/********************************************************************************
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
* @file cpssPxPortManagerSamples.c
*
* @brief CPSS implementation for Port management.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/port/private/prvCpssPortManager.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortManager.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>


/*-----Regular-Samples-----*/

/**
* @internal cpssPxSamplePortManagerMandatoryParamsSet function
* @endinternal
*
* @brief   set the mandtory params in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - port mode
* @param[in] speed                    - port speed
* @param[in] speed                    - port fec mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerMandatoryParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssPxPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
    portParams.portParamsType.regPort.portAttributes.fecMode = fecMode;

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_CALIBRATION_E);
    /* in order to select optinal calibration need to call the function with CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.calibrationType = CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E;
    /* set min Eo Threshold */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMinThreshold = 130;
    /* set max Eo Threshold */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMaxThreshold = 800;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.calibrationTimeOutSec = 10;

    /* portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = 0x30; */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = 0x30;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxSamplePortManagerLoopbackSet function
* @endinternal
*
* @brief   set the Loopback params in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] loopbackType             - loopback type
* @param[in] macLoopbackEnable        - MAC loopback
*       enable/disable
* @param[in] serdesLoopbackMode       - SerDes loopback type
* @param[in] enableRegularTrafic      - enable trafic
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerLoopbackSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT     loopbackType,
    IN  GT_BOOL                                 macLoopbackEnable,
    IN  CPSS_PORT_SERDES_LOOPBACK_MODE_ENT      serdesLoopbackMode,
    IN  GT_BOOL                                 enableRegularTrafic
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_LOOPBACK_E);

    portParams.portParamsType.regPort.portAttributes.loopback.loopbackType = loopbackType;

    switch (loopbackType)
    {
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
            portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.macLoopbackEnable = macLoopbackEnable;
            break;
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
            portParams.portParamsType.regPort.portAttributes.loopback.loopbackMode.serdesLoopbackMode = serdesLoopbackMode;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portParams.portParamsType.regPort.portAttributes.loopback.enableRegularTrafic = enableRegularTrafic;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxSamplePortManagerLKBSet function
* @endinternal
*
* @brief   set the Link Binding attribute on port in the pm database
*          when the LKB enabled - if the pair port fails
*          the remote fault enabled for the port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pairPortNum              - pair port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerLKBSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_PHYSICAL_PORT_NUM    pairPortNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    /* get the port params from pm db */
    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
        return rc;

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "portNum not inialized");
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_LKB_E);
    portParams.portParamsType.regPort.portAttributes.linkBinding.enabled = GT_TRUE;
    portParams.portParamsType.regPort.portAttributes.linkBinding.pairPortNum = pairPortNum;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* @internal cpssPxSamplePortManagerSerdesTxParamsSet function
* @endinternal
*
* @brief   set the serdes tx params in the pm database
*
* @param[in] devNum               - device number
* @param[in] portNum              - port number
* @param[in] laneNum              - lane number
* @param[in] portParamsInputSt    - a structure contains port
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerSerdesTxParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          laneNum,
    IN  CPSS_PORT_SERDES_TX_CONFIG_STC  *portParamsInputStcPtr
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC         portParams;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U32                          portMacNum;
    MV_HWS_PORT_STANDARD            portMode;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    if(portParamsInputStcPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(portParamsInputStcPtr->type != CPSS_PORT_SERDES_AVAGO_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,portParams.portParamsType.regPort.ifMode,portParams.portParamsType.regPort.speed, &portMode);
    if (rc!=GT_OK)
    {
        return rc;
    }

    rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if (laneNum >= curPortParams.numOfActLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_LANE_PARAM(&portParams, laneNum, CPSS_PM_LANE_PARAM_TX_E);

    portParams.portParamsType.regPort.laneParams[laneNum].txParams.type = CPSS_PORT_SERDES_AVAGO_E;
    cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].txParams.txTune.avago),
         &(portParamsInputStcPtr->txTune.avago),
         sizeof(CPSS_PORT_AVAGO_SERDES_TX_CONFIG_STC));


    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssPxSamplePortManagerSerdesRxParamsSet function
* @endinternal
*
* @brief   set the serdes rx params in the pm database
*
* @param[in] devNum               - device number
* @param[in] portNum              - port number
* @param[in] laneNum              - lane number
* @param[in] portParamsInputSt    - a structure contains port
*                                    configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerSerdesRxParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          laneNum,
    IN  CPSS_PORT_SERDES_RX_CONFIG_STC  *portParamsInputStcPtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;
    MV_HWS_PORT_INIT_PARAMS         curPortParams;
    GT_U32                          portMacNum;
    MV_HWS_PORT_STANDARD            portMode;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssCommonPortIfModeToHwsTranslate(devNum,portParams.portParamsType.regPort.ifMode,portParams.portParamsType.regPort.speed, &portMode);
    if (rc!=GT_OK)
    {
        return rc;
    }

    rc = hwsPortModeParamsGetToBuffer(CAST_SW_DEVNUM(devNum), 0, portMacNum, portMode, &curPortParams);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if (laneNum >= curPortParams.numOfActLanes)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_LANE_PARAM(&portParams, laneNum, CPSS_PM_LANE_PARAM_RX_E);


    portParams.portParamsType.regPort.laneParams[laneNum].rxParams.type = CPSS_PORT_SERDES_AVAGO_E;
    cpssOsMemCpy(&(portParams.portParamsType.regPort.laneParams[laneNum].rxParams.rxTune.avago),
            &(portParamsInputStcPtr->rxTune.avago),
            sizeof(CPSS_PORT_AVAGO_SERDES_RX_CONFIG_STC));

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssPxSamplePortManagerTrainModeSet function
* @endinternal
*
* @brief   set the TrainMode attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] trainMode                - trainMode type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerTrainModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT     trainModeType
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (trainModeType >=  CPSS_PORT_SERDES_AUTO_TUNE_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_TRAIN_MODE_E);
    portParams.portParamsType.regPort.portAttributes.trainMode = trainModeType;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* @internal cpssPxSamplePortManagerAdaptiveRxTrainSuppSet function
* @endinternal
*
* @brief   set the AdaptiveRxTrainSupp attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] adaptRxTrainSupp         - boolean whether adaptive rx tune is supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerAdaptiveRxTrainSuppSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 adaptRxTrainSupp
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if ((adaptRxTrainSupp != GT_TRUE) && (adaptRxTrainSupp != GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_RX_TRAIN_SUPP_E);
    portParams.portParamsType.regPort.portAttributes.adaptRxTrainSupp = adaptRxTrainSupp;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}


/**
* @internal cpssPxSamplePortManagerEdgeDetectSuppSet function
* @endinternal
*
* @brief   set the EdgeDetectSupp attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] edgeDetectSupported      - boolean whether edge detect is supported
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerEdgeDetectSuppSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 edgeDetectSupported
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if ((edgeDetectSupported != GT_TRUE) && (edgeDetectSupported != GT_FALSE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_EDGE_DETECT_SUPP_E);
    portParams.portParamsType.regPort.portAttributes.edgeDetectSupported = edgeDetectSupported;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* @internal cpssPxSamplePortManagerFecModeSet function
* @endinternal
*
* @brief   set the FecMode attr in the pm database
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] fecMode                  - FEC mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerFecModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_FEC_MODE_ENT                  fecMode
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (fecMode >= CPSS_PORT_FEC_MODE_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams, CPSS_PM_ATTR_FEC_MODE_E);
    portParams.portParamsType.regPort.portAttributes.fecMode = fecMode;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* cpssPxSamplePortManagerEnhanceTuneOverrideSet function
*
* @brief   Enhanced Tune Override Params Set fn.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] minLF                    - minimum LF
* @param[in] maxLF                    - max LF
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssPxSamplePortManagerEnhanceTuneOverrideSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U8                                   minLF,
    IN  GT_U8                                   maxLF
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_ET_OVERRIDE_E);
    portParams.portParamsType.regPort.portAttributes.etOverride.minLF = minLF;
    portParams.portParamsType.regPort.portAttributes.etOverride.maxLF = maxLF;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* cpssPxSamplePortManagerOpticalCalibrationSet function
*
* @brief   pre defined calibration type set command for port
*          manager
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] calibrationType          - Cu or Optical
* @param[in] minEoThreshold           - minimum EO threshold
* @param[in] maxEoThreshold           - max EO threshold
* @param[in] bitMapEnable             - BM enable
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssPxSamplePortManagerOpticalCalibrationSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT  calibrationType,
    IN  GT_U32                                  minEoThreshold,
    IN  GT_U32                                  maxEoThreshold,
    IN  GT_U32                                  bitMapEnable
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (CPSS_PORT_MANAGER_CALIBRATION_TYPE_LAST_E <= calibrationType)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* standard */
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_CALIBRATION_E);

    /* in order to select optinal calibration need to call the function with CPSS_PORT_MANAGER_OPTICAL_CALIBRATION_TYPE_E */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.calibrationType = calibrationType;
    /* setting confidence interval parameters */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.lfMinThreshold = 12;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.lfMaxThreshold = 15;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.hfMinThreshold = 0;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.hfMaxThreshold = 4;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMinThreshold = minEoThreshold;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.eoMaxThreshold = maxEoThreshold;
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.calibrationTimeOutSec = 10;

    /* for now the requirment is to use only EO as filtring until CI data will be collected */
    /* portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = 0x30; */
    portParams.portParamsType.regPort.portAttributes.calibrationMode.confidenceCfg.confidenceEnableBitMap = bitMapEnable;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* cpssPxSamplePortManagerUnMaskModeSet function
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] unMaskMode               - un-mask mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssPxSamplePortManagerUnMaskModeSet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT  unMaskMode
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_LAST_E <= unMaskMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }


    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_UNMASK_EV_MODE_E);
    portParams.portParamsType.regPort.portAttributes.unMaskEventsMode = unMaskMode;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* cpssPxSamplePortManagerBwModeSet function
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] bwMode                   - bw mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssPxSamplePortManagerBwModeSet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  CPSS_PORT_PA_BW_MODE_ENT  bwMode
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS               rc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    if (CPSS_PORT_PA_BW_MODE_LAST_E <= bwMode)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }


    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_BW_MODE_E);
    portParams.portParamsType.regPort.portAttributes.bwMode = bwMode;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* @internal
*           cpssPxSamplePortManagerAutoNegotiationSet function
* @endinternal
*
* @brief   set auto negotiation enable in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] enableInband          - enable inband
* @param[in] enableDuplex          - enable duplex
* @param[in] enableSpeed           - enable speed
* @param[in] enableByPass          - enable bypass
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerAutoNegotiationSet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL                enableInband,
    IN  GT_BOOL                enableDuplex,
    IN  GT_BOOL                enableSpeed,
    IN  GT_BOOL                enableFlowCtrl,
    IN  GT_BOOL                enableFlowCtrlPauseAdvertise,
    IN  GT_BOOL                enableFlowCtrlAsmAdvertise,
    IN  GT_BOOL                enableByPass
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandatory params */
    if ((portParams.portParamsType.regPort.ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.regPort.speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_ATTR(&portParams,CPSS_PM_PORT_ATTR_AUTO_NEG_ENABLE);

    portParams.portParamsType.regPort.portAttributes.autoNegotiation.inbandEnable  = enableInband;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.duplexEnable  = enableDuplex;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.speedEnable   = enableSpeed;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlEnable = enableFlowCtrl;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlPauseAdvertiseEnable = enableFlowCtrlPauseAdvertise;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.flowCtrlAsmAdvertiseEnable = enableFlowCtrlAsmAdvertise;
    portParams.portParamsType.regPort.portAttributes.autoNegotiation.byPassEnable  = enableByPass;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/*-----AP-Samples-----*/

/**
* @internal cpssPxSamplePortManagerApMandatoryParamsSet function
* @endinternal
*
* @brief   set the mandtory params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - port mode
* @param[in] speed                 - port speed
* @param[in] fecSupported          - fec supported
* @param[in] fecRequested          - fec requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerApMandatoryParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecSupported,
    IN  CPSS_PORT_FEC_MODE_ENT          fecRequested
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E;
    rc = cpssPxPortManagerPortParamsStructInit(portParams.portType, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    portParams.portParamsType.apPort.numOfModes++;

    /* configure the first port mode and speed */
    portParams.portParamsType.apPort.modesArr[0].ifMode = ifMode;
    portParams.portParamsType.apPort.modesArr[0].speed = speed;
    portParams.portParamsType.apPort.modesArr[0].fecRequested = fecRequested;
    portParams.portParamsType.apPort.modesArr[0].fecSupported = fecSupported;

    /* checking mandtory params */
    if ((portParams.portParamsType.apPort.modesArr[0].ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
    (portParams.portParamsType.apPort.modesArr[0].speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxSamplePortManagerApAddMultiSpeedParamsSet
*           function
* @endinternal
*
* @brief   set the multi speed params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] ifMode                - port mode
* @param[in] speed                 - port speed
* @param[in] fecSupported          - fec supported
* @param[in] fecRequested          - fec requested
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* Hint: this is for multi speed set,
* cpssPxSamplePortManagerApMandatoryParamsSet must be called
* first
*/
GT_STATUS cpssPxSamplePortManagerApAddMultiSpeedParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecSupported,
    IN  CPSS_PORT_FEC_MODE_ENT          fecRequested
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;
    GT_U32 numOfModes;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    numOfModes = portParams.portParamsType.apPort.numOfModes;
    if (numOfModes == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (numOfModes >= CPSS_PM_AP_MAX_MODES)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    portParams.portParamsType.apPort.modesArr[numOfModes].ifMode = ifMode;
    portParams.portParamsType.apPort.modesArr[numOfModes].speed = speed;
    portParams.portParamsType.apPort.modesArr[numOfModes].fecRequested = fecRequested;
    portParams.portParamsType.apPort.modesArr[numOfModes].fecSupported = fecSupported;

    portParams.portParamsType.apPort.numOfModes++;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal cpssPxSamplePortManagerApAttributeParamsSet
*           function
* @endinternal
*
* @brief   set the mandtory params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] nonceDisable          - nonceDisable
* @param[in] fcPause               - fcPause
* @param[in] fcAsmDir              - fcAsmDir
* @param[in] negotiationLaneNum    - negotiation lane number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerApAttributeParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             nonceDisable,
    IN  GT_BOOL                             fcPause,
    IN  CPSS_PORT_AP_FLOW_CONTROL_ENT       fcAsmDir,
    IN  GT_U32                              negotiationLaneNum
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_NONCE_E);
    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_FC_PAUSE_E);
    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_FC_ASM_DIR_E);
    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_NUM_E);

    portParams.portParamsType.apPort.apAttrs.nonceDisable = nonceDisable;
    portParams.portParamsType.apPort.apAttrs.fcPause = fcPause;
    portParams.portParamsType.apPort.apAttrs.fcAsmDir = fcAsmDir;
    portParams.portParamsType.apPort.apAttrs.negotiationLaneNum = negotiationLaneNum;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxSamplePortManagerUpd function
* @endinternal
*
* @brief   update port manager params without deleting the port
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] bwMode                - bw mode to update
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS cpssPxSamplePortManagerUpd
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_PA_BW_MODE_ENT            bwMode
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_UPDATE_PARAMS_STC updateParamsStc;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    updateParamsStc.updateParamsBitmapType = CPSS_PORT_MANAGER_UPDATE_PARAMS_BW_TYPE_E;
    updateParamsStc.bwMode = bwMode;

    rc = cpssPxPortManagerPortParamsUpdate(devNum, portNum, &updateParamsStc);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling cpssPxPortManagerPortParamsUpdate returned=%d, portNum=%d", rc, portNum);
    }

    return GT_OK;
}

/**
* @internal cpssPxSamplePortManagerApLKBSet function
* @endinternal
*
* @brief   set the Link Binding on port in the pm database
*          when the LKB enabled - if the pair port fails
*          the remote fault enabled for the port
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pairPortNum              - pair port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerApLKBSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_PHYSICAL_PORT_NUM    pairPortNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_PM_PORT_PARAMS_STC portParams;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    /* get the port params from pm db */
    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
        return rc;

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, "portNum not inialized");
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.apPort.modesArr[0].ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
    (portParams.portParamsType.apPort.modesArr[0].speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams, CPSS_PM_AP_PORT_ATTR_LKB_E);
    portParams.portParamsType.apPort.apAttrs.linkBinding.enabled = GT_TRUE;
    portParams.portParamsType.apPort.apAttrs.linkBinding.pairPortNum = pairPortNum;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);

    return rc;
}

/**
* @internal cpssPxSamplePortManagerApOverrideTxLaneParamsSet
*           function
* @endinternal
*
* @brief   set the tx params in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] laneNum               - lane number
* @param[in] laneSpeed             - lane speed
* @param[in] portParamsInputStc    - structe with attributes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxSamplePortManagerApOverrideTxLaneParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    IN  CPSS_PM_AP_LANE_SERDES_SPEED        laneSpeed,
    IN  GT_U8                               atten,
    IN  GT_U8                               post,
    IN  GT_U8                               pre
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    rc = cpssPxPortManagerPortParamsGet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(portParams.portType != CPSS_PORT_MANAGER_PORT_TYPE_802_3AP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* checking mandtory params */
    if ((portParams.portParamsType.apPort.modesArr[0].ifMode == CPSS_PORT_INTERFACE_MODE_NA_E) ||
        (portParams.portParamsType.apPort.modesArr[0].speed == CPSS_PORT_SPEED_NA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if ((CPSS_PM_MAX_LANES <= laneNum) || (CPSS_PM_AP_LANE_SERDES_SPEED_NONE_E <= laneSpeed))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    CPSS_PM_SET_VALID_AP_ATTR(&portParams,CPSS_PM_AP_PORT_ATTR_LANE_OVR_PARAMS_E);

    CPSS_PM_SET_VALID_AP_ATTR_OVERRIDE(&portParams, laneSpeed, laneNum, CPSS_PM_AP_LANE_OVERRIDE_TX_OFFSET_E);

    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txParams.type = CPSS_PORT_SERDES_AVAGO_E;

    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txParams.txTune.avago.atten = atten;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txParams.txTune.avago.post = post;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txParams.txTune.avago.pre = pre;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txParams.txTune.avago.pre2 = 0;
    portParams.portParamsType.apPort.apAttrs.overrideLaneParams[laneSpeed][laneNum].txParams.txTune.avago.pre3 = 0;

    rc = cpssPxPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

