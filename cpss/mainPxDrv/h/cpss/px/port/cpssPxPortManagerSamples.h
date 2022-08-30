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
* @file cpssPxPortManager.h
*
* @brief CPSS implementation for Port management.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxPortManagerSamples
#define __cpssPxPortManagerSamples

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/port/private/prvCpssPortManager.h>

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
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
);

/**
* cpssPxSamplePortManagerUnMaskModeSet function
*
* @brief   defines whether to allow port manager unmask LOW/MAC
*          level events or not
*
* @note   APPLICABLE DEVICES:      xCat3; Caelum; Aldrin; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; Lion; Lion2; xCat2; Bobcat2; AC3X.
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
);

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
);

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
);

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
);

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
);

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
);
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
);


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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortManager */

