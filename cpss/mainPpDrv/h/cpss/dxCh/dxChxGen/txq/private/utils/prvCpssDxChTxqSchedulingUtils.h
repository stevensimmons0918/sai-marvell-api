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
* @file prvCpssDxChTxqSchedulingUtils.h
*
* @brief CPSS SIP6 TXQ  scheduling operation functions
*
* @version   1
********************************************************************************

*/

#ifndef __prvCpssDxChTxqSchedulingUtils
#define __prvCpssDxChTxqSchedulingUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @internal prvCpssFalconTxqUtilsInitSchedProfilesDb function
 * @endinternal
 *
 * @brief   Initialize schedProfiles database . All the queues are in WRR gropu 0. TC 0-3 weight 1 , TC 0-3 weight 5
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -PP's device number.
 */
GT_VOID  prvCpssFalconTxqUtilsInitSchedProfilesDb
(
    IN GT_U8 devNum
);
/**
 * @internal prvCpssFalconTxqUtilsBindPortToSchedulerProfile function
 * @endinternal
 *
 * @brief  Bind a port to scheduler profile set.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                  -  device number
 * @param[in] physicalPortNum           -port number
 * @param[in] profileSet  -The Profile Set in which the scheduler's parameters are
 *                                      associated.
* @param[in] forceUpdate           -sync HW anyway even if correct profile at SW
*
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssFalconTxqUtilsBindPortToSchedulerProfile
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT profileSet,
    IN GT_BOOL  forceUpdate
);

/**
* @internal prvCpssFalconTxqUtilsWrrProfileSet function
* @endinternal
 *
* @brief   Set Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue on this Port (0..7)
* @param[in] wrrWeight                - proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this Arbitration Group
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
*/

GT_STATUS prvCpssFalconTxqUtilsWrrProfileSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  GT_U8                                   wrrWeight,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
);
/**
* @internal prvCpssFalconTxqUtilsWrrProfileGet function
* @endinternal
 *
* @brief   Get Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue on this Port (0..7)
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
*
* @param[out] wrrWeightPtr             - Pointer to proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this  Arbitration Group
*/
GT_STATUS prvCpssFalconTxqUtilsWrrProfileGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT  GT_U8                                  *wrrWeightPtr
);
/**
* @internal prvCpssFalconTxqUtilsArbGroupSet function
* @endinternal
 *
* @brief   Set Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
 *
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue (0..7)
* @param[in] arbGroup                 - scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
*/
GT_STATUS prvCpssFalconTxqUtilsArbGroupSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_Q_ARB_GROUP_ENT            arbGroup,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet
);
/**
* @internal prvCpssFalconTxqUtilsArbGroupGet function
* @endinternal
 *
* @brief   Get Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @param[in] devNum                   - physical device number
* @param[in] tcQueue                  - traffic class queue (0..7)
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
*
* @param[out] arbGroupPtr              - Pointer to scheduling arbitration group:
*                                      1) Strict Priority
*                                      2) WRR Group 1
*                                      3) WRR Group 0
*/
GT_STATUS prvCpssFalconTxqUtilsArbGroupGet
(
    IN  GT_U8                                   devNum,
    IN  GT_U8                                   tcQueue,
    IN  CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT CPSS_PORT_TX_Q_ARB_GROUP_ENT            *arbGroupPtr
);

/**
* @internal prvCpssFalconTxqUtilsBindPortToSchedulerProfile function
* @endinternal
 *
* @brief   Get scheduler profile set that is binded to the port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @param[in] devNum                  -  device number
* @param[in] physicalPortNum           -port number
* @param[out] profileSetPtr  -The Profile Set in which the scheduler's parameters are
*                                      associated.
*/

GT_STATUS prvCpssFalconTxqUtilsPortSchedulerProfileGet
(
  IN GT_U8 devNum,
  IN GT_U32 physicalPortNum,
  OUT CPSS_PORT_TX_SCHEDULER_PROFILE_SET_ENT  * profileSetPtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqMemory */

