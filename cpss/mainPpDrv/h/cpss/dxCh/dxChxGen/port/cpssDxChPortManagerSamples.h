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
* @file cpssDxChPortManager.h
*
* @brief CPSS implementation for Port management.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPortManagerSamples
#define __cpssDxChPortManagerSamples

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/port/private/prvCpssPortManager.h>



/**
* @internal cpssDxChSamplePortManagerMandatoryParamsSet function
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
GT_STATUS cpssDxChSamplePortManagerMandatoryParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode
);
/**
* @internal cpssDxChSamplePortManagerMandatoryParamsExtSet function
* @endinternal
*
* @brief   set the mandtory params in the pm database(with preemption support)
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] ifMode                   - port mode
* @param[in] speed                        - port speed
* @param[in] fecMode                    - port fec mode
* @param[in] preemptionType     - preemption feature parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerMandatoryParamsExtSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecMode,
    IN  CPSS_PM_MAC_PREEMPTION_TYPE_ENT preemptionType
);


/**
* @internal cpssDxChSamplePortManagerLoopbackSet function
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
GT_STATUS cpssDxChSamplePortManagerLoopbackSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT     loopbackType,
    IN  GT_BOOL                                 macLoopbackEnable,
    IN  CPSS_PORT_SERDES_LOOPBACK_MODE_ENT      serdesLoopbackMode,
    IN  GT_BOOL                                 enableRegularTrafic
);

/**
* @internal cpssDxChSamplePortManagerSerdesTxParamsSet function
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
GT_STATUS cpssDxChSamplePortManagerSerdesTxParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          laneNum,
    IN  CPSS_PORT_SERDES_TX_CONFIG_STC  *portParamsInputStcPtr
);

/**
* @internal cpssDxChSamplePortManagerSerdesRxParamsSet function
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
GT_STATUS cpssDxChSamplePortManagerSerdesRxParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          laneNum,
    IN  CPSS_PORT_SERDES_RX_CONFIG_STC  *portParamsInputStcPtr
);

/**
* @internal cpssDxChSamplePortManagerTrainModeSet function
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
GT_STATUS cpssDxChSamplePortManagerTrainModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_SERDES_AUTO_TUNE_MODE_ENT     trainModeType
);

/**
* @internal cpssDxChSamplePortManagerAdaptiveRxTrainSuppSet function
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
GT_STATUS cpssDxChSamplePortManagerAdaptiveRxTrainSuppSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 adaptRxTrainSupp
);

/**
* @internal cpssDxChSamplePortManagerEdgeDetectSuppSet function
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
GT_STATUS cpssDxChSamplePortManagerEdgeDetectSuppSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_BOOL                                 edgeDetectSupported
);

/**
* @internal cpssDxChSamplePortManagerFecModeSet function
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
GT_STATUS cpssDxChSamplePortManagerFecModeSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_FEC_MODE_ENT                  fecMode
);

/**
* cpssDxChSamplePortManagerEnhanceTuneOverrideSet function
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

GT_STATUS cpssDxChSamplePortManagerEnhanceTuneOverrideSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  GT_U8                                   minLF,
    IN  GT_U8                                   maxLF
);

/**
* cpssDxChSamplePortManagerOpticalCalibrationSet function
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
* @note   APPLICABLE DEVICES:      xCat3; AC5; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; AC3X.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS cpssDxChSamplePortManagerOpticalCalibrationSet
(
    IN  GT_U8                                   devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PORT_MANAGER_CALIBRATION_TYPE_ENT  calibrationType,
    IN  GT_U32                                  minEoThreshold,
    IN  GT_U32                                  maxEoThreshold,
    IN  GT_U32                                  bitMapEnable
);

/**
* cpssDxChSamplePortManagerUnMaskModeSet function
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
GT_STATUS cpssDxChSamplePortManagerUnMaskModeSet
(
    IN  GT_U8                                     devNum,
    IN  GT_PHYSICAL_PORT_NUM                      portNum,
    IN  CPSS_PORT_MANAGER_UNMASK_EVENTS_MODE_ENT  unMaskMode
);

/**
* cpssDxChSamplePortManagerBwModeSet function
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
GT_STATUS cpssDxChSamplePortManagerBwModeSet
(
    IN  GT_U8                     devNum,
    IN  GT_PHYSICAL_PORT_NUM      portNum,
    IN  CPSS_PORT_PA_BW_MODE_ENT  bwMode
);

/**
* @internal
*           cpssDxChSamplePortManagerOverrideInterconnectProfileSet
*           function
* @endinternal
*
* @brief   set the interconnect profile in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] interconnectProfile    - interconnect profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerOverrideInterconnectProfileSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT  interconnectProfile
);

/**
* @internal
*           cpssDxChSamplePortManagerAutoNegotiationSet
*           function
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
GT_STATUS cpssDxChSamplePortManagerAutoNegotiationSet
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
);

/**
* @internal
*           cpssDxChSamplePortManagerPmOverFwSet function
* @endinternal
*
* @brief   set PM over FW enable in the pm database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] enablePmOverFw        - enable enablePmOverFw
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerPmOverFwSet
(
    IN  GT_U8                  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL                enablePmOverFw
);

/**
* @internal cpssDxChSamplePortManagerApMandatoryParamsSet function
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
GT_STATUS cpssDxChSamplePortManagerApMandatoryParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecSupported,
    IN  CPSS_PORT_FEC_MODE_ENT          fecRequested
);

/**
* @internal cpssDxChSamplePortManagerApAddMultiSpeedParamsSet
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
* cpssDxChSamplePortManagerApMandatoryParamsSet must be called
* first
*/
GT_STATUS cpssDxChSamplePortManagerApAddMultiSpeedParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_PORT_SPEED_ENT             speed,
    IN  CPSS_PORT_FEC_MODE_ENT          fecSupported,
    IN  CPSS_PORT_FEC_MODE_ENT          fecRequested
);

/**
* @internal cpssDxChSamplePortManagerApAttributeParamsSet
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
GT_STATUS cpssDxChSamplePortManagerApAttributeParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             nonceDisable,
    IN  GT_BOOL                             fcPause,
    IN  CPSS_PORT_AP_FLOW_CONTROL_ENT       fcAsmDir,
    IN  GT_U32                              negotiationLaneNum
);

/**
* @internal cpssDxChSamplePortManagerAttrExtOperationParamsSet
*           function
* @endinternal
*
* @brief   set the port extra operations in the pm database.
           supported for ap and non-ap ports.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] portExtraOperation    - extra operations bitmap
*           supported CPSS_PM_AP_PORT_ATTR_OPERATION_PRECODING_CNS
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerAttrExtOperationParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              portExtraOperation
);

/**
* @internal cpssDxChSamplePortManagerApOverrideTxLaneParamsSet
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
GT_STATUS cpssDxChSamplePortManagerApOverrideTxLaneParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    IN  CPSS_PM_AP_LANE_SERDES_SPEED        laneSpeed,
    IN  CPSS_PORT_SERDES_TX_CONFIG_STC      *portParamsInputStcPtr
);

/**
* @internal cpssDxChSamplePortManagerApOverrideRxLaneParamsSet
*           function
* @endinternal
*
* @brief   set the rx params in the pm database
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
GT_STATUS cpssDxChSamplePortManagerApOverrideRxLaneParamsSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              laneNum,
    IN  CPSS_PM_AP_LANE_SERDES_SPEED        laneSpeed,
    IN  CPSS_PORT_SERDES_RX_CONFIG_STC      *portParamsInputStcPtr
);

/**
* @internal
*           cpssDxChSamplePortManagerApOverrideInterconnectProfileSet
*           function
* @endinternal
*
* @brief   set the interconnect profile in the pm
*          database
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] interconnectProfile    - interconnect profile
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApOverrideInterconnectProfileSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PORT_MANAGER_INTERCONNECT_PROFILE_ENT  interconnectProfile
);

/**
* @internal cpssDxChSamplePortManagerUpd function
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

GT_STATUS cpssDxChSamplePortManagerUpd
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_PORT_PA_BW_MODE_ENT            bwMode
);

/**
* @internal cpssDxChSamplePortManagerApSkipResSet function
* @endinternal
*
* @brief   Skip resolution
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] skipRes               - skipRes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApSkipResSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_BOOL                             skipRes
);

/**
* @internal cpssDxChSamplePortManagerApParallelDetectSet function
* @endinternal
*
* @brief   set Parallel Detect parameters
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
* @param[in] restartMaxNum         - restartMaxNum
* @param[in] enable                - enable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChSamplePortManagerApParallelDetectSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U8                               restartMaxNum,
    IN  GT_BOOL                             enable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortManager */
