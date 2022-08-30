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
* @file cpssDxChPortTxUburst.c
*
* @brief CPSS implementation for TX micro burst detection.
*
* @version  1
********************************************************************************
*/


#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqQfc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/**
* @internal internal_cpssDxChPortTxUburstEventInfoGet function
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
static GT_STATUS internal_cpssDxChPortTxUburstEventInfoGet
(
    IN  GT_U8                                devNum,
    INOUT GT_U32                             *uBurstEventsSizePtr,
    OUT CPSS_DXCH_UBURST_INFO_STC            *uBurstDataPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
        CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(uBurstEventsSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(uBurstDataPtr);

    return prvCpssFalconTxqUtilsUburstEventInfoGet(devNum,uBurstEventsSizePtr,uBurstDataPtr);
}
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxUburstEventInfoGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,uBurstEventsSizePtr,uBurstDataPtr));

    rc = internal_cpssDxChPortTxUburstEventInfoGet(devNum,uBurstEventsSizePtr,uBurstDataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,devNum,uBurstEventsSizePtr,uBurstDataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxUburstDetectionEnableSet
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
static GT_STATUS internal_cpssDxChPortTxUburstDetectionEnableSet
(
   IN  GT_U8                    devNum,
   IN  GT_PHYSICAL_PORT_NUM     physicalPortNumber,
   IN  GT_U32                   tcQueue,
   IN  GT_BOOL                  enable
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
        CPSS_AC3X_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_COS_CHECK_16_TC_MAC(tcQueue);

    return prvCpssFalconTxqUtilsUburstEnableSet(devNum,physicalPortNumber,tcQueue,enable);
}
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
   IN  GT_U32                   tcQueue,
   IN  GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxUburstDetectionEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,physicalPortNumber,tcQueue,enable));

    rc = internal_cpssDxChPortTxUburstDetectionEnableSet(devNum,physicalPortNumber,tcQueue,enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,devNum,physicalPortNumber,tcQueue,enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxUburstDetectionEnableGet
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
static GT_STATUS internal_cpssDxChPortTxUburstDetectionEnableGet
(
   IN  GT_U8                    devNum,
   IN  GT_PHYSICAL_PORT_NUM     physicalPortNumber,
   IN  GT_U32                   queueOffset,
   OUT  GT_BOOL                 *enablePtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
        CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_COS_CHECK_16_TC_MAC(queueOffset);

    return prvCpssFalconTxqUtilsUburstEnableGet(devNum,physicalPortNumber,queueOffset,enablePtr);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxUburstDetectionEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,physicalPortNumber,queueOffset,enablePtr));

    rc = internal_cpssDxChPortTxUburstDetectionEnableGet(devNum,physicalPortNumber,queueOffset,enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,devNum,physicalPortNumber,queueOffset,enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxUburstDetectionProfileBindSet
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
static GT_STATUS internal_cpssDxChPortTxUburstDetectionProfileBindSet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               physicalPortNumber,
    IN  GT_U32                             queueOffset,
    IN  GT_U32                             profileNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
        CPSS_AC3X_E | CPSS_ALDRIN2_E);

    PRV_CPSS_DXCH_COS_CHECK_16_TC_MAC(queueOffset);

    return prvCpssFalconTxqUtilsUburstProfileBindSet(devNum,physicalPortNumber,queueOffset,profileNum);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxUburstDetectionProfileBindSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,physicalPortNumber,queueOffset,profileNum));

    rc = internal_cpssDxChPortTxUburstDetectionProfileBindSet(devNum,physicalPortNumber,queueOffset,profileNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,devNum,physicalPortNumber,queueOffset,profileNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChPortTxUburstDetectionProfileBindGet
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
static GT_STATUS internal_cpssDxChPortTxUburstDetectionProfileBindGet
(
    IN  GT_U8                              devNum,
    IN  GT_PHYSICAL_PORT_NUM               physicalPortNumber,
    IN  GT_U32                             queueOffset,
    OUT GT_U32                             *profileNumPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
        CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(profileNumPtr);

    PRV_CPSS_DXCH_COS_CHECK_16_TC_MAC(queueOffset);

    return prvCpssFalconTxqUtilsUburstProfileBindGet(devNum,physicalPortNumber,queueOffset,profileNumPtr);
}
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxUburstDetectionProfileBindGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,physicalPortNumber,queueOffset,profileNumPtr));

    rc = internal_cpssDxChPortTxUburstDetectionProfileBindGet(devNum,physicalPortNumber,queueOffset,profileNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,devNum,physicalPortNumber,queueOffset,profileNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChPortTxUburstDetectionProfileSet
* @endinternal
*
* @brief  Set  micro burst profile attributes.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] profileNum                  Profile number[0..31]
* @param[in] profilePtr                 (pointer to)Micro burst profile
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
static GT_STATUS internal_cpssDxChPortTxUburstDetectionProfileSet
(
     IN  GT_U8                                  devNum,
     IN  GT_U32                                 profileNum,
     IN  CPSS_DXCH_UBURST_PROFILE_STC           *profilePtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
        CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    return prvCpssFalconTxqQfcUburstProfileSet(devNum,profileNum,profilePtr->threshold0.trigger,
        profilePtr->threshold0.value,profilePtr->threshold1.trigger,profilePtr->threshold1.value);
}
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxUburstDetectionProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,profileNum,profilePtr));

    rc = internal_cpssDxChPortTxUburstDetectionProfileSet(devNum,profileNum,profilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,devNum,profileNum,profilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxUburstDetectionProfileGet
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
static GT_STATUS internal_cpssDxChPortTxUburstDetectionProfileGet
(
     IN  GT_U8                                  devNum,
     IN  GT_U32                                 profileNum,
     OUT CPSS_DXCH_UBURST_PROFILE_STC           *profilePtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
        CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    return prvCpssFalconTxqQfcUburstProfileGet(devNum,profileNum,&(profilePtr->threshold0.trigger),
        &(profilePtr->threshold0.value),&(profilePtr->threshold1.trigger),&(profilePtr->threshold1.value));
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxUburstDetectionProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,profileNum,profilePtr));

    rc = internal_cpssDxChPortTxUburstDetectionProfileGet(devNum,profileNum,profilePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc,devNum,profileNum,profilePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


