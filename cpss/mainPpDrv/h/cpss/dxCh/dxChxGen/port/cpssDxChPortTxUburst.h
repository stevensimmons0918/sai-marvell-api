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
* @file cpssDxChPortTxUburst.h
*
* @brief CPSS implementation for TX micro burst detection.
*
* @version  1
********************************************************************************

*/

#ifndef __cpssDxChPortTxUbursth
#define __cpssDxChPortTxUbursth

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
/*
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
*/

/**
* @enum CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT
 *
 * @brief Micro burst triggering event
*/
typedef enum{

    /**Uburst trigger for interrupt is not valid */
    CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_NOT_VALID_E,

    /** Uburst trigger for interrupt is when resource is filling */
    CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_FILL_E,

   /** Uburst trigger for interrupt is when resource is draining */
    CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_DRAIN_E,

    /** Uburst trigger for interrupt is when resource is filling or draining */
    CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_EITHER_E

} CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT;

/**
* @struct CPSS_DXCH_UBURST_TIME_STAMP_STC
 *
 * @brief  Time of uburst event
 *
 */
typedef struct
{
     /** @brief    seconds part of timestamp
     */
    GT_U32   seconds;
     /** @brief  nano seconds part of timestamp
     */
    GT_U32    nanoseconds;

} CPSS_DXCH_UBURST_TIME_STAMP_STC;


/**
* @struct CPSS_DXCH_UBURST_INFO_STC
 *
 * @brief  Uburst event information
 *
 */
typedef struct
{
    /** @brief    Physical port that triggered  the event
     */
    GT_PHYSICAL_PORT_NUM   portNum;
     /** @brief  Queue offset of the queue that triggered  the event
     */
    GT_U32                 queueOffset;

   /** @brief  Represent ID of threshold that was crossed [0..1]
     */
    GT_U32                 thresholdId;

    /** @brief  Trigger that caused the interrupt.
                Valid options:CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_FILL_E or
                CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_DRAIN_E
     */
   CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT trigger;

   /** @brief  Timestamp of the event
     */
   CPSS_DXCH_UBURST_TIME_STAMP_STC        timestamp;

} CPSS_DXCH_UBURST_INFO_STC;

/**
* @struct CPSS_DXCH_UBURST_THRESHOLD_STC
 *
 * @brief  Uburst threshold information
 *
 */
typedef struct
{
    /** @brief  Threshold trigger
     */
  CPSS_DXCH_PORT_UBURST_TRIGGER_TYPE_ENT trigger;
   /** @brief  Threshold value
     */
  GT_U32                                 value;
}CPSS_DXCH_UBURST_THRESHOLD_STC;


/**
* @struct CPSS_DXCH_UBURST_PROFILE_STC
 *
 * @brief  Uburst profile information
 *
 */
typedef struct
{
    /** @brief  Threshold  0 attributes
      */
  CPSS_DXCH_UBURST_THRESHOLD_STC threshold0;
    /** @brief  Threshold  1 attributes
      */
  CPSS_DXCH_UBURST_THRESHOLD_STC threshold1;
}CPSS_DXCH_UBURST_PROFILE_STC;


/**
* @internal cpssDxChPortTxUburstEventInfoGet function
* @endinternal
*
* @brief  Get next uburst event information
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[out] uBurstDataPtr                   -         `               (pointer to)micro  burst event
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NO_MORE - no more new events found
*
*
*/
GT_STATUS cpssDxChPortTxUburstEventInfoGet
(
    IN  GT_U8                                devNum,
    INOUT GT_U32                             *uBurstEventsSizePtr,
    OUT CPSS_DXCH_UBURST_INFO_STC            uBurstDataPtr[] /*arrSizeVarName=uBurstEventsSizePtr*/
);

/**
* @internal cpssDxChPortTxUburstDetectionEnableSet
* @endinternal
*
* @brief   Enable/disable micro burst event generation per port/queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] physicalPortNumber                  - physical port number
* @param[in] queueOffset                                   local  queue offset (APPLICABLE RANGES:0..15).
* @param[in] enable                                             Enable/disable micro burst feature
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortTxUburstDetectionEnableSet
(
   IN  GT_U8                    devNum,
   IN  GT_PHYSICAL_PORT_NUM     physicalPortNumber,
   IN  GT_U32                   queueOffset,
   IN  GT_BOOL                  enable
);
/**
* @internal cpssDxChPortTxUburstDetectionEnableGet
* @endinternal
*
* @brief   Get enable/disable configuration micro burst event generation per port/queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                     physical device number
* @param[in] physicalPortNumber                  - physical port number
* @param[in] queueOffset                                   local  queue offset (APPLICABLE RANGES:0..15).
* @param[out] enable                                             Enable/disable micro burst feature
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortTxUburstDetectionEnableGet
(
   IN  GT_U8                    devNum,
   IN  GT_PHYSICAL_PORT_NUM     physicalPortNumber,
   IN  GT_U32                   queueOffset,
   OUT  GT_BOOL                 *enablePtr
);
/**
* @internal cpssDxChPortTxUburstDetectionProfileBindSet
* @endinternal
*
* @brief  Set queue micro burst profile binding
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                                       - physical device number
* @param[in] physicalPortNumber                  - physical port number
* @param[in] queueOffset                                   port queue offset (APPLICABLE RANGES:0..15).
* @param[in] profileNum                                      profile number[0..31]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS  cpssDxChPortTxUburstDetectionProfileBindSet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               physicalPortNumber,
    IN  GT_U32                             queueOffset,
    IN  GT_U32                             profileNum
);

/**
* @internal cpssDxChPortTxUburstDetectionProfileBindGet
* @endinternal
*
* @brief  Get queue micro burst profile binding configuration
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                                       - physical device number
* @param[in] physicalPortNumber                  - physical port number
* @param[in] queueOffset                                   port queue offset (APPLICABLE RANGES:0..15).
* @param[out] profileNumPtr                              (pointer to)profile number[0..31]
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortTxUburstDetectionProfileBindGet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               physicalPortNumber,
    IN  GT_U32                             queueOffset,
    OUT GT_U32                             *profileNumPtr
);
/**
* @internal cpssDxChPortTxUburstDetectionProfileSet
* @endinternal
*
* @brief  Set  micro burst profile attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] profileNum                  Profile number[0..31]
* @param[in] profilePtr                    (pointer to)Micro burst profile
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortTxUburstDetectionProfileSet
(
     IN  GT_U8                                  devNum,
     IN  GT_U32                                 profileNum,
     IN  CPSS_DXCH_UBURST_PROFILE_STC           *profilePtr
);
/**
* @internal cpssDxChPortTxUburstDetectionProfileGet
* @endinternal
*
* @brief  Get  micro burst profile attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] profileNum                  Profile number[0..31]
* @param[out] profilePtr                 (pointer to)Micro burst profile
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS cpssDxChPortTxUburstDetectionProfileGet
(
     IN  GT_U8                                  devNum,
     IN  GT_U32                                 profileNum,
     OUT CPSS_DXCH_UBURST_PROFILE_STC           *profilePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortTxUbursth */

