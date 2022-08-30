/*******************************************************************************
* Copyright (c) 2021 Marvell. All rights reserved. The following file is       *
* subject to the limited use license agreement by and between Marvell and you, *
* your employer or other entity on behalf of whom you act. In the absence of   *
* such license agreement the following file is subject to Marvell’s standard   *
* Limited Use License Agreement.                                               *
*******************************************************************************/

/**
********************************************************************************
* @file cpssHalStp.h
*
* @brief Private API declarations which can be used in XPS layer.
*
* @version   01
********************************************************************************
*/


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include  <gtOs/gtGenTypes.h>
#include <cpss/common/cpssTypes.h>

#define CPSSHAL_ERSPAN_EPLR_FLOW_ID_BASE 512
#define CPSSHAL_ERSPAN_EPLR_FLOW_ID_MIN  0
#define CPSSHAL_ERSPAN_EPLR_FLOW_ID_MAX  10

#define CPSSHAL_ERSPAN_EPCL_FLOW_ID_START 0

#define CPSSHAL_ERSPAN_OAM_PROFILE_INDEX 1

/**
* @internal cpssHalSetMirrorTruncateState function
* @endinternal
*
* @brief  This function sets if to truncate TO_ANALYZER packets per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[in] truncate                 - Indicates if truncate TO_ANALYZER packets to 128B
*                                       GT_TRUE  truncate, GT_FALSE  do not truncate.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalSetMirrorTruncateState
(
    int                     devId,
    GT_U32                  index,
    GT_BOOL                 truncate
);

/**
* @internal cpssHalMirrorAnalyzerDpTcSet function
* @endinternal
*
* @brief   The TC/DP assigned to the packet forwarded to the inress/egress analyzer port due
*         to egress mirroring to the analyzer port.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devId                    - device number.
* @param[in] analyzerDp               - the Drop Precedence to be set
* @param[in] analyzerTc               - traffic class on analyzer port (APPLICABLE RANGES: 0..7)
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on error.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalMirrorAnalyzerDpTcSet
(
    IN GT_U8              devId,
    IN GT_BOOL            isRx,
    IN CPSS_DP_LEVEL_ENT  analyzerDp,
    IN GT_U8              analyzerTc
);

/**
* @internal cpssHalAnalyzerPortSet function
* @endinternal
*
* @brief   This function sets analyzer interface.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] analyzerIndex            - of analyzer interface. (APPLICABLE RANGES: 0..6)
* @param[in] interfacePort            - Destination Port.
* @param[in] type                     - Interface type port/trunk
* @param[in] ePort                    - Destination virtual port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index, interface type.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on wrong port or device number in interfacePtr.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalAnalyzerPortSet
(
    IN GT_U8                   devId,
    IN GT_U32                  analyzerIndex,
    IN CPSS_INTERFACE_TYPE_ENT type,
    IN GT_U32                  analyzerPort,
    IN GT_TRUNK_ID             analyzerTrunkId,
    IN GT_PORT_NUM             ePort
);

GT_STATUS cpssHalAnalyzerEPortMacSet
(
    IN GT_U8              devId,
    IN GT_U32             portNum,
    IN GT_PORT_NUM        ePort
);

/**
* cpssHalAnalyzerPortClear function
*
*
* @brief   This function clears analyzer interface.
*
*/
GT_STATUS cpssHalAnalyzerPortClear
(
    IN GT_U8              devId,
    IN GT_U32             analyzerIndex,
    IN GT_U32             analyzerPort,
    IN GT_PORT_NUM        ePort
);

/**
* @internal cpssHalBindAnalyzerToTunnel function
* @endinternal
*
* @brief   This function sets analyzer binding.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                - device number.
* @param[in] analyzerIndex         - of analyzer interface. (APPLICABLE RANGES: 0..6)
* @param[in] tunnelStartIndex      - tunnel start index.
* @param[in] ePort                 - destination interface.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalBindAnalyzerToTunnel
(
    IN GT_U8              devId,
    IN GT_U32             analyzerIndex,
    IN GT_U32             tunnelStartIndex,
    IN GT_PORT_NUM        ePort
);

/**
* @internal cpssHalUnBindAnalyzerFromTunnel function
* @endinternal
*
* @brief   This function unbind tunnel index from eport.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                - device number.
* @param[in] analyzerIndex         - of analyzer interface. (APPLICABLE RANGES: 0..6)
* @param[in] eport                 - eport
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalUnBindAnalyzerFromTunnel
(
    IN GT_U8              devId,
    IN GT_U32             analyzerIndex,
    IN GT_PORT_NUM        ePort
);

/**
* @internal cpssHalMirrorAnalyzerSampleRateSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Rx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                  - device number.
* @param[in] index                   - Ananyze Index.
* @param[in] sampleRate              - Indicates the  of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
* @param[in] isIngress               - Indicate analyzer position - ingress or egress
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Rx mirroring
*       ratio 1 -> all Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       ratio 10 -> 1 in 10 Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssHalMirrorAnalyzerSampleRateSet
(
    IN GT_U8    devId,
    IN GT_U32   index,
    IN GT_U32   sampleRate,
    IN GT_BOOL  isIngress
);

/**
* @internal cpssHalMirrorSampleRateSet function
* @endinternal
*
* @brief   Set the statistical mirroring rate in the Rx Analyzer port
*         Set Statistic mirroring to analyzer port statistical ratio configuration.
*
* @note   APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  Falcon.
*
* @param[in] devNum                  - device number.
* @param[in] sampleRate              - Indicates the  of egress mirrored to analyzer port packets
*                                      forwarded to the analyzer port. 1 of every 'ratio' packets are
*                                      forwarded to the analyzer port.
* @param[in] isIngress               - Indicate analyzer position - ingress or egress
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or ratio.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Examples:
*       ratio 0 -> no Rx mirroring
*       ratio 1 -> all Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       ratio 10 -> 1 in 10 Rx mirrored packets are forwarded out the Rx
*       Analyzer port
*       The maximum ratio value is 2047.
*
*/
GT_STATUS cpssHalMirrorSampleRateSet
(
    IN GT_U8    devId,
    IN GT_U32   sampleRate,
    IN GT_BOOL  isIngress
);

/**
* @internal cpssDxChMirrorAnalyzerMirrorOnDropEnableSet function
* @endinternal
*
* @brief   Enable / Disable mirroring of dropped packets.
*
* @note   APPLICABLE DEVICES:      xCat; xCat3; Lion; xCat2; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
* @note   NOT APPLICABLE DEVICES:  DxCh1; DxCh1_Diamond; DxCh2; DxCh3.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer destination interface index. (APPLICABLE RANGES: 0..6)
* @param[in] enable                   - GT_TRUE  - dropped packets are mirrored to analyzer interface.
*                                      GT_FALSE - dropped packets are not mirrored to analyzer interface.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device, index.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalMirrorAnalyzerMirrorOnDropEnableSet
(
    IN GT_U8     devNum,
    IN GT_U32    analyzerIndex,
    IN GT_BOOL   enable
);

GT_STATUS cpssHalMirrorInit
(
    IN GT_U8 devId
);

GT_STATUS cpssHalMirrorErspanOAMFlowConfig
(
    IN GT_U8 devId
);

GT_STATUS cpssHalMirrorErspanOAMFlowCounterClear
(
    IN GT_U8     devId,
    IN GT_U32    analyzerIndex
);
GT_STATUS cpssHalMirrorErspan2V6DummyTagSet(GT_U8 devId, GT_PORT_NUM ePort);
GT_STATUS cpssHalMirrorErspan2V6DummyTagClear(GT_U8 devId, GT_PORT_NUM ePort);


/**
* @internal cpssHalMirrorSampleRateGet function
* @endinternal
*
* @brief  This function gets Ingress statistical Mirroring
*         to the Ingress Analyzer Port including ratio per analyzer index.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number.
* @param[in] index                    - Analyzer interface index. (APPLICABLE RANGES: 0..6)
* @param[out] enablePtr               - (pointer to) enable/disable statistical mirroring.
* @param[out] ratioPtr                - (pointer to) Indicates the  ratio of  mirrored traffic to analyzer port
*                                       ratio 0 -> no Rx mirroring.
*                                       ratio 1 -> all Rx mirrored packets are forwarded out the Rx Analyzer port.
*                                       ratio 31 -> 1 in 31 Rx mirrored packets are forwarded out the Rx  Analyzer port.
*                                                                                     32
*                                                         The maximum ratio value is 2    - 1
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device or index.
* @retval GT_HW_ERROR              - reading HW error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssHalMirrorSampleRateGet
(
    IN GT_U8      devId,
    IN GT_U32     index,
    IN GT_BOOL    isIngress,
    OUT GT_U32    *ratioPtr
);
#ifdef __cplusplus
}
#endif
