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
* @file cpssDxChPortManager.c
*
* @brief CPSS implementation for Port management.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/common/port/private/prvCpssPortManager.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#if 0
#define IS_FALCON_Z2_PHY_CONNECTED_PORT(devNum,portNum) \
            ((PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG[devNum]->mngInterfaceType == CPSS_CHANNEL_PEX_FALCON_Z_E) && /* Falcon Z2 */ \
            ((portNum < 32) || ((portNum > 228) && (portNum < 261))))
#endif

/**
* @internal cpssDxChPortManagerEventSet function
* @endinternal
*
* @brief   Configure the port according to the given event. This API performs actions based on
*         the given event and will possible transfer the port to a new state. For example, when
*         port is in PORT_MANAGER_STATE_RESET_E state and event PORT_MANAGER_EVENT_CREATE_E is
*         called, this API will perform port creation operations and upon success, port state
*         will be changed to PORT_MANAGER_STATE_LINK_DOWN_E state.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X (Not Include 98DX3255,98DX3258,98DX3265,98DX3268).
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portEventStcPtr          - (pointer to) structure containing the event data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortManagerEventSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_MANAGER_STC   *portEventStcPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerEventSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portEventStcPtr));

    if (devNum < PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        PRV_CPSS_PP_MAC(devNum)->lockLowLevelApi = GT_FALSE;
        rc = internal_cpssPortManagerEventSet(devNum, portNum, portEventStcPtr);
        PRV_CPSS_PP_MAC(devNum)->lockLowLevelApi = GT_TRUE;
    }
    else
        rc = GT_BAD_PARAM;

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portEventStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPortManagerStatusGet function
* @endinternal
*
* @brief   Get the status of the port. This API will return (1) State of the port
*         within the port manager state machine (2) Whether or not port is currently
*         disabled (using PORT_MANAGER_EVENT_DISABLE_E event) (3) Failure status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortManagerStatusGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MANAGER_STATUS_STC    *portStagePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerStatusGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portStagePtr));

    rc = internal_cpssPortManagerStatusGet(devNum, portNum, portStagePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portStagePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPortManagerPortParamsSet function
* @endinternal
*
* @brief   Set the given attributes to a port. Those configurations will take place
*         during the port management. This API is applicable only when port is in
*         reset (PORT_MANAGER_STATE_RESET_E state).
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortManagerPortParamsSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PM_PORT_PARAMS_STC                *portParamsStcPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerPortParamsSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portParamsStcPtr));

    rc = internal_cpssPortManagerPortParamsSet(devNum, portNum, portParamsStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portParamsStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPortManagerPortParamsGet function
* @endinternal
*
* @brief   Get the attributes of a port. In case port was created, this API will return
*         all the current attributes of the port. In case the port was not created yet
*         this API will return all the attributes the port will be configured with upon
*         creation. This API is not applicable if port is in
*         CPSS_PORT_MANAGER_STATE_RESET_E state and previous call to
*         cpssDxChPortManagerPortParamsSet have not been made.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently, On devices below SIP_5_15, if RX parameters not set and
*       port is in reset, the RX values are not availble to be fetched, and
*       filled with zeros.
*
*/
GT_STATUS cpssDxChPortManagerPortParamsGet
(
    IN  GT_U8                                  devNum,
    IN  GT_PHYSICAL_PORT_NUM                   portNum,
    OUT CPSS_PM_PORT_PARAMS_STC               *portParamsStcPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerPortParamsGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portParamsStcPtr));

    rc = internal_cpssPortManagerPortParamsGet(devNum, portNum, portParamsStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portParamsStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPortManagerPortParamsStructInit function
* @endinternal
*
* @brief  This API resets the Port Manager Parameter structure
*         so once the application update the structure with
*         relevant data - all other feilds are clean for sure.
*         Application must call this API before calling
*         cpssDxChPortManagerPortParamsSet.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in,out] portParamsStcPtr   - (pointer to) a structure
*       contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none
*
*/
GT_STATUS cpssDxChPortManagerPortParamsStructInit
(
    IN  CPSS_PORT_MANAGER_PORT_TYPE_ENT portType,
    INOUT  CPSS_PM_PORT_PARAMS_STC *portParamsStcPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerPortParamsStructInit);
    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, portType, portParamsStcPtr));

    rc = internal_cpssPortManagerInitParamsStruct(portType, portParamsStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, portType, portParamsStcPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);


    return rc;
}

/**
* cpssDxChPortManagerParametersSetExt function
*
* @brief   extended parmaeter set port manager command
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                    - interface mode
* @param[in] speed                    - speed
* @param[in] adaptiveMode             - wheater to run adaptive
*       or not
* @param[in] trainMode                - training mode
* @param[in] overrideElectricalParams - override TX parameters
* @param[in] overridePolarityParams   - override polarity
*       parameters
* @param[in] -                        minLF
* @param[in] -                        maxLF
* @param[in] fecMode                  = fecMode
*
* @param[out] portParamsStcPtr         - (pointer to) a structure contains port configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssDxChPortManagerParametersSetExt
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  GT_BOOL                         adaptiveMode,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT  trainMode,
    IN  GT_BOOL                              overrideElectricalParams,
    IN  GT_BOOL                              overridePolarityParams,
    IN  GT_U8                            minLF,
    IN  GT_U8                            maxLF,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    CPSS_PM_PORT_PARAMS_STC portParamsFromDb;

    GT_STATUS    rc;
    GT_U32       laneNum;

    GT_BOOL      overrideElec;

    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct :rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParamsFromDb);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct :rc=%d\n", rc);
        return rc;
    }

    /* standard */
    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    /* init get strctures */
    portParamsFromDb.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParamsFromDb.portParamsType.regPort.ifMode = ifMode;
    portParamsFromDb.portParamsType.regPort.speed = speed;

    /* set Adaptive mode */
/*

  CPSS_PM_ATTR_TRAIN_MODE_E,
  CPSS_PM_ATTR_RX_TRAIN_SUPP_E,
  CPSS_PM_ATTR_EDGE_DETECT_SUPP_E,
  CPSS_PM_ATTR_LOOPBACK_E,
  CPSS_PM_ATTR_ET_OVERRIDE_E,
  CPSS_PM_ATTR_FEC_MODE_E,
  CPSS_PM_ATTR_CALIBRATION_E,
  CPSS_PM_ATTR_UNMASK_EV_MODE_E,
*/
    portParams.portParamsType.regPort.portAttributes.adaptRxTrainSupp = adaptiveMode;
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_RX_TRAIN_SUPP_E);

    /* set train mode */
    portParams.portParamsType.regPort.portAttributes.trainMode = trainMode;
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_TRAIN_MODE_E);

    /* ETL values */
    portParams.portParamsType.regPort.portAttributes.etOverride.minLF = minLF;
    portParams.portParamsType.regPort.portAttributes.etOverride.maxLF = maxLF;
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_ET_OVERRIDE_E);

    /* insert additionals non-AP parameters to parameters structure */
    portParams.portParamsType.regPort.portAttributes.fecMode = fecMode;
    CPSS_PM_SET_VALID_ATTR (&portParams, CPSS_PM_ATTR_FEC_MODE_E);

    if (overrideElectricalParams == GT_TRUE || overridePolarityParams == GT_TRUE)
    {
        rc = cpssDxChPortManagerPortParamsGet(devNum, portNum, &portParamsFromDb);
        if (rc!=GT_OK)
        {
            cpssOsPrintf("calling cpssDxChPortManagerPortParamsGet returned=%d, portNum=%d", rc, portNum);
            return rc;
        }

    }

    for (laneNum=0; laneNum<CPSS_PM_MAX_LANES; laneNum++)
    {
           overrideElec = CPSS_PM_LANE_PARAM_IS_VALID(&portParamsFromDb, laneNum, CPSS_PM_LANE_PARAM_TX_E);

           if (overrideElectricalParams && overrideElec)
           {
               CPSS_PM_SET_VALID_LANE_PARAM(&portParams, laneNum, CPSS_PM_LANE_PARAM_TX_E);
               portParams.portParamsType.regPort.laneParams[laneNum].txParams.type =
                   portParamsFromDb.portParamsType.regPort.laneParams[laneNum].txParams.type;

               portParams.portParamsType.regPort.laneParams[laneNum].txParams.txTune.avago.atten =
                   portParamsFromDb.portParamsType.regPort.laneParams[laneNum].txParams.txTune.avago.atten;
               portParams.portParamsType.regPort.laneParams[laneNum].txParams.txTune.avago.post =
                   portParamsFromDb.portParamsType.regPort.laneParams[laneNum].txParams.txTune.avago.post;
               portParams.portParamsType.regPort.laneParams[laneNum].txParams.txTune.avago.pre =
                   portParamsFromDb.portParamsType.regPort.laneParams[laneNum].txParams.txTune.avago.pre;
           }
    }


    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);

    if (rc!=GT_OK)
    {
        cpssOsPrintf("calling cpssDxChPortManagerParametersSetExt returned=%d, portNum=%d", rc, portNum);
        return rc;
    }

    return GT_OK;
}

/**
* prvCpssDxChPortManagerParamsSet function
*
* @brief  parmaeters set command for port manager
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortManagerParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT        ifMode,
    IN  CPSS_PORT_SPEED_ENT                 speed
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc;

    /* standard */
    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct (portNum=%d):rc=%d\n", portNum, rc);
        return rc;
    }

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling PortManagerPortParamsSet returned=%d, portNum=%d", rc, portNum);
    }

    return GT_OK;
}

/**
* prvCpssDxChPortManagerStatusGet function
*
* @brief  Get the Status of a port
*
* @note   APPLICABLE DEVICES:   Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   Lion2; Bobcat2; AC3X
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortManagerStatusGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum
)
{
    CPSS_PORT_MANAGER_STATUS_STC portStatus;
    GT_STATUS rc;

    /* standard */
    rc = cpssDxChPortManagerStatusGet(devNum, portNum, &portStatus);
    if(rc != GT_OK)
    {
        cpssOsPrintf("prvCpssDxChPortManagerStatusGet (portNum=%d):rc=%d\n", portNum, rc);
        return rc;
    }

    cpssOsPrintf("Status for port %d:\n", portNum);
    cpssOsPrintf("  State           %d\n", portStatus.portState);
    cpssOsPrintf("  Oper Mode       %d\n", portStatus.portUnderOperDisable);
    cpssOsPrintf("  Failure state   %d\n", portStatus.failure);
    if (portStatus.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E)
    {
        cpssOsPrintf("  IfMode          %d:\n", portStatus.ifMode);
        cpssOsPrintf("  Speed           %d:\n", portStatus.speed);
        cpssOsPrintf("  FEC             %d:\n", portStatus.fecType);
    }

    return GT_OK;
}

/**
* prvCpssDxChPortManagerOpticalCalibrationSet function
*
* @brief   pre defined calibration type set command for port
*          manager
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortManagerOpticalCalibrationSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT            ifMode,
    IN  CPSS_PORT_SPEED_ENT                     speed,
    IN  CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT  calibrationType,
    IN  GT_U32                                  minEoThreshold,
    IN  GT_U32                                  maxEoThreshold,
    IN  GT_U32                                  bitMapEnable
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc;

    /* standard */
    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct (portNum=%d):rc=%d\n", portNum, rc);
        return rc;
    }

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    CPSS_PM_SET_VALID_ATTR (&portParams,CPSS_PM_ATTR_CALIBRATION_E);

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

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling PortManagerPortParamsSet returned=%d, portNum=%d", rc, portNum);
    }

    return GT_OK;
}

/**
* prvCpssDxChPortManagerUnMaskModeSet function
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortManagerUnMaskModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT            ifMode,
    IN  CPSS_PORT_SPEED_ENT                     speed,
    IN  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT  unMaskMode
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc;

    /* standard */
    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct (portNum=%d):rc=%d\n", portNum, rc);
        return rc;
    }


    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    CPSS_PM_SET_VALID_ATTR (&portParams,CPSS_PM_ATTR_UNMASK_EV_MODE_E);
    portParams.portParamsType.regPort.portAttributes.unMaskEventsMode = unMaskMode;

    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling PortManagerPortParamsSet returned=%d, portNum=%d", rc, portNum);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortManagerGlobalParamsOverride function
* @endinternal
*
* @brief  To use this function for overide, need to give it a
*         struct that include parameters to override, and to
*         enable override flag to 1, else it's will not override
*         and use defualts values.
*         Don't forget to fill the globalParamstType enum with
*         the match enum of what you want to override.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] globalParamsStc          - stc with the parameters
*                                       to override
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To add other globals parameter to the struct, make sure
*       to add enum and enable flag for the set of values.
*       *See CPSS_PORT_MANAGER_SIGNAL_DETECT_CONFIG_STC as a
*        good format to add other parameters.
*
*/
GT_STATUS cpssDxChPortManagerGlobalParamsOverride
(
    IN  GT_U8                                  devNum,
    IN  CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC    *globalParamsStcPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerGlobalParamsOverride);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, globalParamsStcPtr));

    rc = internal_cpssPortManagerGlobalParamsOverride(devNum, globalParamsStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, globalParamsStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/* @internal cpssDxChPortManagerPortParamsUpdate function
* @endinternal
*
* @brief  To use this function for update the PA mode also when port is active. In order to configure the new mode,
* application should call the port manager update API (called cpssDxChPortManagerPortParamsUpdate ) with the new
* specified mode
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] updateParamsStc          - stc with the parameters to update
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortManagerPortParamsUpdate
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_MANAGER_UPDATE_PARAMS_STC *updateParamsStcPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerPortParamsUpdate);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, updateParamsStcPtr));

    rc = internal_cpssPortManagerPortParamsUpdate(devNum, portNum, updateParamsStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, updateParamsStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* prvCpssDxChPortManagerGlobalParamsSet function
*
* @brief   Rx parmaeters set command for port manager
*
* @note   APPLICABLE DEVICES:      Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Caelum; Aldrin; Lion2; Bobcat2; AC3X.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssDxChPortManagerGlobalParamsSet
(
    IN  GT_U8    devNum,
    IN  GT_U32   changeInterval,
    IN  GT_U32   changeWindow,
    IN  GT_U32   checkTimeExpired,
    IN  GT_BOOL  highSpeed
)
{
    CPSS_PORT_MANAGER_GLOBAL_PARAMS_STC globalParams;
    GT_STATUS rc;
    GT_BOOL stabilityConfig = GT_FALSE;

    globalParams.globalParamsBitmapType = 0;
    globalParams.signalDetectDbCfg.sdChangeInterval = PORT_MANAGER_SIGNAL_DETECT_INTERVAL_CNS;
    globalParams.signalDetectDbCfg.sdChangeMinWindowSize = PORT_MANAGER_SIGNAL_DETECT_WINDOW_SIZE_CNS;
    globalParams.signalDetectDbCfg.sdCheckTimeExpired = PORT_MANAGER_SIGNAL_DETECT_TIME_EXPIRED_CNS;
    globalParams.propHighSpeedPortEnabled = GT_FALSE;

    if (changeInterval != 0)
    {
        globalParams.signalDetectDbCfg.sdChangeInterval = changeInterval;
        stabilityConfig = GT_TRUE;
    }
    if (changeWindow != 0)
    {
        globalParams.signalDetectDbCfg.sdChangeMinWindowSize = changeWindow;
        stabilityConfig = GT_TRUE;
    }
    if (checkTimeExpired != 0)
    {
        globalParams.signalDetectDbCfg.sdCheckTimeExpired = checkTimeExpired;
        stabilityConfig = GT_TRUE;
    }

    globalParams.propHighSpeedPortEnabled = highSpeed;
    globalParams.globalParamsBitmapType |= CPSS_PORT_MANAGER_GLOBAL_PARAMS_HIGH_SPEED_PORTS_E;

    if (stabilityConfig)
    {
        globalParams.globalParamsBitmapType |= CPSS_PORT_MANAGER_GLOBAL_PARAMS_SIGNAL_DETECT_E;
    }


    rc = cpssDxChPortManagerGlobalParamsOverride(devNum, &globalParams);
    if (rc!=GT_OK)
    {
        cpssOsPrintf("calling cpssDxChPortManagerGlobalParamsOverride returned=%d", rc);
        return rc;
    }

    return GT_OK;
}

/**
* prvCpssDxChPortManagerBwModeSet function
*
* @brief   defines whether to allow port manager bandwidth
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortManagerBwModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT            ifMode,
    IN  CPSS_PORT_SPEED_ENT                     speed,
    IN  CPSS_PORT_PA_BW_MODE_ENT                bwMode
)
{
    CPSS_PM_PORT_PARAMS_STC portParams;
    GT_STATUS rc;

    /* standard */
    rc = cpssDxChPortManagerPortParamsStructInit(CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E, &portParams);
    if(rc != GT_OK)
    {
        cpssOsPrintf("internal_cpssPortManagerInitParamsStruct (portNum=%d):rc=%d\n", portNum, rc);
        return rc;
    }

    portParams.portType = CPSS_PORT_MANAGER_PORT_TYPE_REGULAR_E;
    portParams.portParamsType.regPort.ifMode = ifMode;
    portParams.portParamsType.regPort.speed = speed;

    CPSS_PM_SET_VALID_ATTR (&portParams,CPSS_PM_ATTR_BW_MODE_E);

    portParams.portParamsType.regPort.portAttributes.bwMode = bwMode;


    rc = cpssDxChPortManagerPortParamsSet(devNum, portNum, &portParams);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling PortManagerPortParamsSet returned=%d, portNum=%d", rc, portNum);
    }

    return GT_OK;
}

/**
* prvCpssDxChPortManagerBwModeSet function
*
* @brief   defines whether to allow port manager bandwidth
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPortManagerUpd
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_PA_BW_MODE_ENT                bwMode
)
{
    GT_STATUS rc;
    CPSS_PORT_MANAGER_UPDATE_PARAMS_STC updateParamsStc;


    updateParamsStc.updateParamsBitmapType = CPSS_PORT_MANAGER_UPDATE_PARAMS_BW_TYPE_E;
    updateParamsStc.bwMode = bwMode;

    rc = cpssDxChPortManagerPortParamsUpdate(devNum, portNum, &updateParamsStc);
    if (rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "calling cpssDxChPortManagerPortParamsUpdate returned=%d, portNum=%d", rc, portNum);
    }

    return GT_OK;
}

/**
* @internal cpssDxChPortManagerInit function
* @endinternal
*
* @brief  Port Manager Init system - this function is
*         responsible on all configurations that till now was
*         application responsiblity.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/

GT_STATUS cpssDxChPortManagerInit
(
    IN  GT_U8   devNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerInit);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssPortManagerInit(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPortManagerEnableGet function
* @endinternal
*
* @brief  This function is returns if port manager enabled/disabled
*
* @note   APPLICABLE DEVICES:      AC3X;Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3
*
* @param[in] devNum                - physical device number
* @param[out] enablePtr            - (pointer to)port manager enable/disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/

GT_STATUS cpssDxChPortManagerEnableGet
(
    IN  GT_U8   devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerEnableGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPortManagerEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChPortManagerStatGet function
* @endinternal
*
* @brief  This function is returns the port manager statistics
*
* @note   APPLICABLE DEVICES:      AC3X;Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3
*
* @param[in] devNum             - physical device number
* @param[in] portNum            - port number
* @param[out] portStatStcPtr    - statistics structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChPortManagerStatGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    OUT CPSS_PORT_MANAGER_STATISTICS_STC    *portStatStcPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerStatGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portStatStcPtr));

    rc = internal_cpssPortManagerStatGet(devNum, portNum, portStatStcPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portStatStcPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
GT_STATUS cpssDxChPortManagerStatPrint
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum
)
{
    CPSS_PORT_MANAGER_STATISTICS_STC    portStatStc;
    GT_STATUS rc;

    rc = cpssDxChPortManagerStatGet(devNum,portNum,&portStatStc);
    if ( rc == GT_OK)
    {
        cpssOsPrintf("Port manager statistics for port %d:\n", portNum);
        cpssOsPrintf("rxTrainingTime        : %d\n", portStatStc.rxTrainingTime);
        cpssOsPrintf("alignLockTime         : %d\n", portStatStc.alignLockTime);
        cpssOsPrintf("linkUpTime            : %d\n", portStatStc.linkUpTime);
        cpssOsPrintf("linkFailToSignalTime  : %d\n", portStatStc.linkFailToSignalTime);
        cpssOsPrintf("linkFailCnt           : %d\n", portStatStc.linkFailCnt);
        cpssOsPrintf("linkOkCnt             : %d\n", portStatStc.linkOkCnt);
        cpssOsPrintf("alignLockFailCnt      : %d\n", portStatStc.alignLockFailCnt);
    }
    else
    {
        cpssOsPrintf("Port manager statistics - port %d not valid\n", portNum);

    }
    return GT_OK;
}
/**
* @internal cpssDxChPortManagerStatClear function
* @endinternal
*
* @brief  This function clear the port manager statistics.
*
* @note   APPLICABLE DEVICES:      AC3X;Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3
*
* @param[in] devNum             - physical device number
* @param[in] portNum            - port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS cpssDxChPortManagerStatClear
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortManagerStatClear);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssPortManagerStatClear(devNum, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
