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
* @file prvCpssDxChTxqFcUtils.h
*
* @brief CPSS SIP6 TXQ flow control functions
*
* @version   1
********************************************************************************

*/

#ifndef __prvCpssDxChFcUtils
#define __prvCpssDxChFcUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @internal prvCpssFalconTxqUtilsPfcEnable function
 * @endinternal
 *
 * @brief   Set PFC  enable status for both generation and responce  at
 *               TxQ and MAC
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 * @param[in] portNum                                                         physical port number
 * @param[out] pfcRespBitMapPtr                                         (pointer to)current PFC responce mode
 * @param[out] pfcGenerationModePtr                      -        (pointer to)current PFC generation mode
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsPfcEnable
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32                               *pfcRespBitMapPtr,
    IN CPSS_DXCH_PORT_FC_MODE_ENT           *pfcGenerationModePtr
);

/**
* @internal prvCpssFalconTxqUtilsFlowControlInit function
* @endinternal
*
* @brief   This function  initialize PFC quanta and PFC threshold for all mapped ports
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                         PP's device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqUtilsFlowControlInit
(
    IN GT_U8 devNum
);


/**
* @internal prvCpssFalconTxqUtilsGlobalFlowControlEnableSet  function
* @endinternal
*
* @brief   Enable/disble flow control globally.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] fcEnable                   - GT_TRUE,  enable flow control globally,
*                                                         GT_FALSE, disable flow control globally
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
*                                                contain latency queue.
*/
GT_STATUS prvCpssFalconTxqUtilsGlobalFlowControlEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL fcEnable
);
/**
* @internal prvCpssFalconTxqUtilsGlobalFlowControlEnableGet  function
* @endinternal
*
* @brief   Get enable/disble flow control globally.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[out] fcEnablePtr                   -(pointer to) GT_TRUE,  enable flow control globally,
*                                                         GT_FALSE, disable flow control globally
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE - in case of enabling low latency mode on port  that already
*                                                contain latency queue.
*/
GT_STATUS prvCpssFalconTxqUtilsGlobalFlowControlEnableGet
(
    IN GT_U8   devNum,
    OUT GT_BOOL *fcEnablePtr
);

/**
* @internal prvCpssFalconTxqUtilsPfcDisable function
* @endinternal
*
* @brief   Read PFC  enable status for both generation and responce and then disable PFC on RX/TX at
*               TxQ and MAC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] portNum                                                         physical port number
* @param[out] pfcRespBitMapPtr                                         (pointer to)current PFC responce mode
* @param[out] pfcGenerationModePtr                      -        (pointer to)current PFC generation mode
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsPfcDisable
(
    IN GT_U8                                devNum,
    IN GT_PHYSICAL_PORT_NUM                 portNum,
    IN GT_U32                               *pfcRespBitMapPtr,
    IN CPSS_DXCH_PORT_FC_MODE_ENT           *pfcGenerationModePtr
);
/**
* @internal prvCpssFalconTxqPfcValidGet function
* @endinternal
*
* @brief   This function check if PFC is configured per port or port/TC
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                          PP's device number.
* @param[in] tileNum                           tile number
* @param[in] dpNum         -                 data path[0..7]
* @param[in] localPort                         local port number[0..8]
* @param[in] tcBmpPtr                           (pointer to) traffic class that is set for flow control,0x0 mean no flow control is set,0xFF mean flow control on port
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssFalconTxqPfcValidGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNum,
    IN  GT_U32                  dpNum,
    IN  GT_U32                  localPort,
    OUT  GT_U32                 *tcBmpPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqMemory */

