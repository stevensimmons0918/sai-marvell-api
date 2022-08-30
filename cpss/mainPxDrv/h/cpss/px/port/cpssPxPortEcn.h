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
* @file cpssPxPortEcn.h
*
* @brief CPSS Pipe Port ECN APIs.
*
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxPortEcn
#define __cpssPxPortEcn

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/px/port/cpssPxPortTxTailDrop.h>





/**
* @struct CPSS_PX_PORT_ECN_ENABLERS_STC
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

} CPSS_PX_PORT_ECN_ENABLERS_STC;

/**
* @internal cpssPxPortEcnMarkingEnableSet function
* @endinternal
*
* @brief   Enable/Disable ECN marking.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
GT_STATUS cpssPxPortEcnMarkingEnableSet
(
    IN GT_SW_DEV_NUM                       devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    IN GT_BOOL                             enable
);

/**
* @internal cpssPxPortEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get status of ECN marking.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
GT_STATUS cpssPxPortEcnMarkingEnableGet
(
    IN GT_SW_DEV_NUM                       devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    OUT GT_BOOL                            *enablePtr
);


/**
* @internal cpssPxPortEcnMarkingTailDropProfileEnableSet function
* @endinternal
*
* @brief   Enable/Disable ECN marking per profile according to Tail Drop limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
GT_STATUS cpssPxPortEcnMarkingTailDropProfileEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    IN  CPSS_PX_PORT_ECN_ENABLERS_STC               *enablersPtr
);


/**
* @internal cpssPxPortEcnMarkingTailDropProfileEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable ECN marking per profile according to Tail Drop limits.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
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
GT_STATUS cpssPxPortEcnMarkingTailDropProfileEnableGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    OUT CPSS_PX_PORT_ECN_ENABLERS_STC               *enablersPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxPortEcn */


