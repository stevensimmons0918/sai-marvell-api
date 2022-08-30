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
* @file prvCpssDxChTxqTailDropUtils.h
*
* @brief CPSS SIP6 TXQ  tail drop functions
*
* @version   1
********************************************************************************

*/

#ifndef __prvCpssDxChTxqTailDropUtils
#define __prvCpssDxChTxqTailDropUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
* @internal prvCpssSip6TxqUtilsPbSizeGet function
* @endinternal
*
* @brief  Get PB size.
*
* @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                  physical device number
* @param[out] maxPbSizePtr                   -     size of PB for device
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssSip6TxqUtilsPbSizeGet
(
    IN GT_U8                                devNum,
    OUT GT_U32                              *maxPbSizePtr
);

/**
* @internal prvCpssFalconTxqUtilsInitTailDrop function
* @endinternal
*
* @brief  Initialize PREQ tail drop parameters to default.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] configureMulticast                   -         `        if equal GT_TRUE then default multicast threshold configured ,
*                                                                                               else no multicast default limits configured
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsInitTailDrop
(
    IN GT_U8                                devNum,
    IN GT_BOOL                              configureMulticast
);

/**
 * @internal prvCpssSip6TxQUtilsPoolConfigurationValidate function
 * @endinternal
 *
 * @brief  Validates that pool limit is within range and set reserved pool value if WA is enabled (CPSS-11271)
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X;Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       -physical device number
 * @param[out] initDonePtr                                 (pointer to)GT_TRUE if txQ library init is done,GT_FALSE otherwise
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxQUtilsPoolConfigurationValidate
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      poolNum,
    IN  GT_U32                      limit
);

GT_U32 prvCpssTxqUtilsPbSizeInBuffersGet
(
    IN GT_U8 devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqMemory */

