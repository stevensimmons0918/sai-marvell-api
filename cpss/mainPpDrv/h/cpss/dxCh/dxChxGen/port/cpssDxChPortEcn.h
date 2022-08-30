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
* @file cpssDxChPortEcn.h
*
* @brief CPSS DxCh Port ECN APIs.
*
*
* @version   3
********************************************************************************
*/
#ifndef __cpssDxChPortEcn
#define __cpssDxChPortEcn

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>

/**
* @struct CPSS_PORT_QUEUE_DP_ECN_PARAMS_STC
 *
 * @brief Define ECN parameters per table entry
*/

typedef struct{

   /** @brief  Define whether ECN feature is enabled
    */
   GT_BOOL                                ecnEnable;
  /** @brief  Defines the additional buffers for the ECN triggering threshold calculation.
           When queue length exceeds Gti + Dti + Ecn Offset packets will be tail dropped without ECN triggering
    */
   GT_U32                                ecnOffset;
} CPSS_PORT_QUEUE_DP_ECN_PARAMS_STC;



/* @struct CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC
 *
 * @brief Define ECN parameters per Queue (3 entries - DP0,DP1,DP2)
*/
typedef struct{
  /** @brief drop precedence 0 ECN attributes
    */
   CPSS_PORT_QUEUE_DP_ECN_PARAMS_STC dp0EcnAttributes;
  /** @brief drop precedence 1 ECN attributes
    */
   CPSS_PORT_QUEUE_DP_ECN_PARAMS_STC dp1EcnAttributes;
  /** @brief drop precedence 2 ECN attributes
    */
   CPSS_PORT_QUEUE_DP_ECN_PARAMS_STC dp2EcnAttributes;
} CPSS_PORT_QUEUE_ECN_PARAMS_STC;


/**
* @struct CPSS_DXCH_PORT_ECN_ENABLERS_STC
 *
 * @brief ECN marking enablers structure.
*/
typedef struct{

    /** enable/disable ECN marking for (Queue,DP) descriptor/buffer limits. */
    GT_BOOL tcDpLimit;

    /** enable/disable ECN marking for Port descriptor/buffer limits. */
    GT_BOOL portLimit;

    /** enable/disable ECN marking for Queue descriptor/buffer limits. */
    GT_BOOL tcLimit;

    /** enable/disable ECN marking for Shared Pool descriptor/buffer limits. */
    GT_BOOL sharedPoolLimit;

} CPSS_DXCH_PORT_ECN_ENABLERS_STC;



/**
* @internal cpssDxChPortEcnMarkingEnableSet function
* @endinternal
*
* @brief   Enable/Disable ECN marking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - type of IP stack
* @param[in] enable                   - GT_TRUE: ECN marking enable
*                                      GT_FALSE: ECN marking disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or protocolStack parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEcnMarkingEnableSet
(
    IN GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    IN GT_BOOL                             enable
);

/**
* @internal cpssDxChPortEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get status of ECN marking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] protocolStack            - type of IP stack
*
* @param[out] enablePtr                - GT_TRUE: ECN marking enable
*                                      GT_FALSE: ECN marking disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or protocolStack parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEcnMarkingEnableGet
(
    IN GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    OUT GT_BOOL                            *enablePtr
);


/**
* @internal cpssDxChPortEcnMarkingTailDropProfileEnableSet function
* @endinternal
*
* @brief   Enable/Disable ECN marking per profile according to Tail Drop limits.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - tail drop profile ID
* @param[in] enablersPtr              - (pointer to) struct of enablers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or profileSet
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEcnMarkingTailDropProfileEnableSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  CPSS_DXCH_PORT_ECN_ENABLERS_STC     *enablersPtr
);


/**
* @internal cpssDxChPortEcnMarkingTailDropProfileEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable ECN marking per profile according to Tail Drop limits.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - tail drop profile ID
*
* @param[out] enablersPtr              - (pointer to) struct of enablers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on other error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum or profileSet
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortEcnMarkingTailDropProfileEnableGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    OUT CPSS_DXCH_PORT_ECN_ENABLERS_STC     *enablersPtr
);


/**
* @internal cpssDxChPortTx4TcTailDropEcnMarkingProfileSet function
* @endinternal
*
* @brief   Set tail drop ECN  profile parameters  for particular TC.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..15).
* @param[in] tailDropEcnProfileParamsPtr -
*                                      the Drop Profile ECN Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTx4TcTailDropEcnMarkingProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    IN    CPSS_PORT_QUEUE_ECN_PARAMS_STC          *tailDropEcnProfileParamsPtr
);

/**
* @internal cpssDxChPortTx4TcTailDropEcnMarkingProfileGet function
* @endinternal
*
* @brief   Get tail drop ECN  profile parameters  for particular TC.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] profileSet               - the Profile Set in which the Traffic
*                                      Class Drop Parameters is associated.
* @param[in] trafficClass             - the Traffic Class associated with this set of
*                                      Drop Parameters (0..15).
* @param[out] tailDropEcnProfileParamsPtr -
*                                      the Drop Profile ECN Parameters to associate
*                                      with the Traffic Class in this Profile set.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTx4TcTailDropEcnMarkingProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    OUT   CPSS_PORT_QUEUE_ECN_PARAMS_STC          *tailDropEcnProfileParamsPtr
);

/**
* @internal cpssDxChPortTxTailDropPoolEcnMarkingEnableSet function
* @endinternal
*
* @brief   Set Enable ECN marking based on Pool limit.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] enable               -         Enable/disable ECN marking based on Pool limit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropPoolEcnMarkingEnableSet
(
    IN    GT_U8                                   devNum,
    IN    GT_BOOL                                 enable
);

/**
* @internal cpssDxChPortTxTailDropPoolEcnMarkingEnableGet  function
* @endinternal
*
* @brief   Set Enable ECN marking based on Pool limit.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[out] enablePtr               -  (pointer to)Enable/disable marking based on  Pool limit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropPoolEcnMarkingEnableGet
(
    IN    GT_U8                                   devNum,
    OUT   GT_BOOL                                 *enablePtr
);

/**
* @internal cpssDxChPortTxTailDropMcEcnMarkingEnableSet function
* @endinternal
*
* @brief   Set Enable ECN marking based on Multicast  limit.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] enable               -         Enable/disable ECN marking based on Multicast  limit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropMcEcnMarkingEnableSet
(
    IN    GT_U8                                   devNum,
    IN    GT_BOOL                                 enable
);

/**
* @internal cpssDxChPortTxTailDropPoolEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get Enable ECN marking based on Multicast  limit.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[out] enablePtr               -  (pointer to)Enable/disable marking based on Multicast  limit
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropMcEcnMarkingEnableGet
(
    IN    GT_U8                                   devNum,
    OUT   GT_BOOL                                 *enablePtr
);


/**
* @internal cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableSet function
* @endinternal
*
* @brief   Enable marking ECN for packets which are mirrored-on-congestion
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum             - physical device number
* @param[in] enable               -  Enable/disable marking
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM       on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableSet
(
    IN    GT_U8                                   devNum,
    IN    GT_BOOL                                 enable
);

/**
* @internal cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableGet function
* @endinternal
*
* @brief   Get enable marking ECN for packets which are mirrored-on-congestion
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum             - physical device number
* @param[out] enablePtr               -  (pointer to)Enable/disable marking
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM       on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableGet
(
    IN    GT_U8                                   devNum,
    OUT   GT_BOOL                                 *enablePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortEcn */


