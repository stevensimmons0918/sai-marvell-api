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
* @file prvCpssDxChTxqShapingUtils.h
*
* @brief CPSS SIP6 TXQ  shaping operation functions
*
* @version   1
********************************************************************************

*/

#ifndef __prvCpssDxChTxqShapingUtils
#define __prvCpssDxChTxqShapingUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssFalconTxqUtilsShapingParametersSet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and write shaping parameters to specific node
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] burstSize                - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[in,out] maxRatePtr               - (pointer to)Requested Rate in Kbps
* @param[in,out] maxRatePtr               -(pointer to) the actual Rate value in Kbps.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqUtilsShapingParametersSet
(
    IN    GT_U8                               devNum,
    IN    GT_U32                              physicalPortNum,
    IN    GT_U32                              queueNumber,
    IN    GT_BOOL                             perQueue,
    IN    PRV_QUEUE_SHAPING_ACTION_ENT        rateType,
    IN    GT_U16                              burstSize,
    INOUT GT_U32                              *maxRatePtr
);
/**
* @internal prvCpssFalconTxqUtilsShapingParametersGet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and read shaping parameters from specific node(SW)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] burstSizePtr             - burst size in units of 4K bytes
*                                      (max value is 4K which results in 16K burst size)
* @param[out] maxRatePtr               -(pointer to) the requested shaping Rate value in Kbps.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsShapingParametersGet
(
    IN    GT_U8                               devNum,
    IN    GT_U32                              physicalPortNum,
    IN    GT_U32                              queueNumber,
    IN    GT_BOOL                             perQueue,
    IN    PRV_QUEUE_SHAPING_ACTION_ENT        rateType,
    OUT    GT_U16                             *burstSizePtr,
    OUT GT_U32                                *maxRatePtr
);
/**
* @internal prvCpssSip6TxqUtilsShapingEnableSet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssSip6TxqUtilsShapingEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    IN    GT_BOOL    enable
);
/**
* @internal prvCpssSip6TxqUtilsShapingEnableGet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and get Enable/Disable Token Bucket rate shaping on specified port or queue of specified device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] enablePtr                - GT_TRUE, enable Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssSip6TxqUtilsShapingEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    IN  GT_BOOL  *  enablePtr
);


/**
* @internal prvCpssFalconTxqUtilsMinimalBwEnableSet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and
 *             Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *             Once token bucket is empty the priority is lowered to lowest.
*
* @note   APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port
* @param[in] enable                   - GT_TRUE,  Set lowest  priority once token bucket is empty
*                                      GT_FALSE, otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqUtilsMinimalBwEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      physicalPortNum,
    IN  GT_U32      queueNumber,
    IN  GT_BOOL     enable
);
/**
* @internal prvCpssFalconTxqUtilsMinimalBwEnableGet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and get
 *             Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *             Once token bucket is empty the priority is lowered to lowest.
*
* @note   APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:    xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port
* @param[in] enablePtr                   - (pointer to)GT_TRUE,  Set lowest  priority once token bucket is empty
*                                                       GT_FALSE, otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqUtilsMinimalBwEnableGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      physicalPortNum,
    IN  GT_U32      queueNumber,
    IN  GT_BOOL     *enablePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqMemory */

