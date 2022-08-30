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
* @file cpssDxChPortEcn.c
*
* @brief CPSS DxCh Port ECN APIs implementation.
*
* @version   3
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortEcn.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/* Feature specific includes. */
#include <cpss/common/cpssTypes.h>

/**
* @internal internal_cpssDxChPortEcnMarkingEnableSet function
* @endinternal
*
* @brief   Enable/Disable ECN marking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
static GT_STATUS internal_cpssDxChPortEcnMarkingEnableSet
(
    IN GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    IN GT_BOOL                             enable
)
{
    GT_U32                      regAddr;        /* register address */
    GT_U32                      fieldValue;     /* field Value      */
    GT_U32                      fieldOffset;    /* field offset     */
    GT_U32                      fieldSize;      /* field size     */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;

    fieldValue = BOOL2BIT_MAC(enable);
    fieldSize = 1;
    switch (protocolStack)
    {
       case CPSS_IP_PROTOCOL_IPV4_E:
          /*Enable ECN marking for IPv4*/
            fieldOffset = 22;
            break;
       case CPSS_IP_PROTOCOL_IPV6_E:
          /*Enable ECN marking for IPv6*/
            fieldOffset = 23;
            break;
       case CPSS_IP_PROTOCOL_IPV4V6_E:
          /*Enable ECN marking for dual stack*/
            fieldOffset = 22;
            fieldValue *= 3; /* assign 22,23 bits to be b00 or b11 */
            fieldSize = 2;
            break;
       default:
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldSize, fieldValue);
}

/**
* @internal cpssDxChPortEcnMarkingEnableSet function
* @endinternal
*
* @brief   Enable/Disable ECN marking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEcnMarkingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, enable));

    rc = internal_cpssDxChPortEcnMarkingEnableSet(devNum, protocolStack, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get status of ECN marking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2.
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
static GT_STATUS internal_cpssDxChPortEcnMarkingEnableGet
(
    IN GT_U8                               devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    OUT GT_BOOL                            *enablePtr
)
{
    GT_U32                      regAddr;        /* register address */
    GT_U32                      bitValue;       /* bit value        */
    GT_STATUS                   rc;             /* return code      */
    GT_U32                      fieldOffset;    /* register field offset */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
          CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_TTI_MAC(devNum).TTIUnitGlobalConfigs.TTIUnitGlobalConfig;

    switch (protocolStack)
    {
       case CPSS_IP_PROTOCOL_IPV4_E:
          /*Get status of ECN assignment for IPv4 header*/
           fieldOffset = 22;
           break;
       case CPSS_IP_PROTOCOL_IPV6_E:
          /*Get status of ECN assignment for IPv6 header*/
           fieldOffset = 23;
           break;
       default:
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &bitValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(bitValue);

    return GT_OK;
}

/**
* @internal cpssDxChPortEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get status of ECN marking.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEcnMarkingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, enablePtr));

    rc = internal_cpssDxChPortEcnMarkingEnableGet(devNum, protocolStack, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortEcnMarkingTailDropProfileEnableSet function
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
static GT_STATUS internal_cpssDxChPortEcnMarkingTailDropProfileEnableSet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    IN  CPSS_DXCH_PORT_ECN_ENABLERS_STC     *enablersPtr
)
{
    GT_U32 regAddr;    /* register address     */
    GT_U32 regData;    /* register value       */
    GT_STATUS rc;      /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);
    CPSS_NULL_PTR_CHECK_MAC(enablersPtr);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet, devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.profileECNMarkEn[profileSet];

    regData = (BOOL2BIT_MAC(enablersPtr->sharedPoolLimit) << 2) |
              (BOOL2BIT_MAC(enablersPtr->tcLimit) << 3) |
              (BOOL2BIT_MAC(enablersPtr->portLimit) << 1) |
              (BOOL2BIT_MAC(enablersPtr->tcDpLimit));

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 4, regData);
    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEcnMarkingTailDropProfileEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, enablersPtr));

    rc = internal_cpssDxChPortEcnMarkingTailDropProfileEnableSet(devNum, profileSet, enablersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, enablersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortEcnMarkingTailDropProfileEnableGet function
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
static GT_STATUS internal_cpssDxChPortEcnMarkingTailDropProfileEnableGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   profileSet,
    OUT CPSS_DXCH_PORT_ECN_ENABLERS_STC     *enablersPtr
)
{
    GT_U32 regAddr;    /* register address     */
    GT_U32 regData;    /* register value       */
    GT_STATUS rc;      /* return code */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E);

    CPSS_NULL_PTR_CHECK_MAC(enablersPtr);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet, devNum);

    regAddr = PRV_DXCH_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.profileECNMarkEn[profileSet];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 4, &regData);
    if(GT_OK != rc)
    {
        return rc;
    }

    enablersPtr->tcDpLimit = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData, 0, 1));
    enablersPtr->portLimit = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData, 1, 1));
    enablersPtr->sharedPoolLimit = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData, 2, 1));
    enablersPtr->tcLimit =  BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData, 3, 1));

    return GT_OK;
}

/**
* @internal cpssDxChPortEcnMarkingTailDropProfileEnableGet function
* @endinternal
*
* @brief   Get Enable/Disable ECN marking per profile according to Tail Drop limits.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortEcnMarkingTailDropProfileEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, enablersPtr));

    rc = internal_cpssDxChPortEcnMarkingTailDropProfileEnableGet(devNum, profileSet, enablersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, enablersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal  internal_cpssDxChPortTx4TcTailDropEcnMarkingProfileSet function
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
static GT_STATUS internal_cpssDxChPortTx4TcTailDropEcnMarkingProfileSet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    IN    CPSS_PORT_QUEUE_ECN_PARAMS_STC          *tailDropEcnProfileParamsPtr
)
{
    GT_STATUS rc;
    GT_U32 regData[4],i;
    CPSS_PORT_QUEUE_DP_ECN_PARAMS_STC  *dpXEcnAttributesPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
        CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(tailDropEcnProfileParamsPtr);
    PRV_CPSS_DXCH_COS_CHECK_4BIT_TC_MAC(devNum,trafficClass);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet, devNum);

    for(i=0;i<3;i++)
    {
        switch (i)
        {
        case 0:
            dpXEcnAttributesPtr = &(tailDropEcnProfileParamsPtr->dp0EcnAttributes);
            break;
        case 1:
            dpXEcnAttributesPtr = &(tailDropEcnProfileParamsPtr->dp1EcnAttributes);
            break;
        default:
            dpXEcnAttributesPtr = &(tailDropEcnProfileParamsPtr->dp2EcnAttributes);
            break;
        }

        if(((dpXEcnAttributesPtr->ecnOffset) >= BIT_20))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

    }

    for(i=0;i<3;i++)
    {
       switch (i)
        {
        case 0:
            dpXEcnAttributesPtr = &(tailDropEcnProfileParamsPtr->dp0EcnAttributes);
            break;
        case 1:
            dpXEcnAttributesPtr = &(tailDropEcnProfileParamsPtr->dp1EcnAttributes);
            break;
        default:
            dpXEcnAttributesPtr = &(tailDropEcnProfileParamsPtr->dp2EcnAttributes);
            break;
        }

        rc = prvCpssDxChReadTableEntry(devNum,
            CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E,
        (i<<8)+(profileSet<<4) + trafficClass,regData);
        if(rc !=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        U32_SET_FIELD_IN_ENTRY_MAC(regData,47,1,BOOL2BIT_MAC(dpXEcnAttributesPtr->ecnEnable));
        U32_SET_FIELD_IN_ENTRY_MAC(regData,48,20,dpXEcnAttributesPtr->ecnOffset);

        rc = prvCpssDxChWriteTableEntry(devNum,
            CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E,
        (i<<8)+(profileSet<<4) + trafficClass,
            regData);

        if(rc !=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}



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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTx4TcTailDropEcnMarkingProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, trafficClass,tailDropEcnProfileParamsPtr));

    rc = internal_cpssDxChPortTx4TcTailDropEcnMarkingProfileSet(devNum, profileSet,trafficClass,tailDropEcnProfileParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, trafficClass,tailDropEcnProfileParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal  internal_cpssDxChPortTx4TcTailDropEcnMarkingProfileGet function
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
* @param[out] tailDropEcnProfileParamsPtr -
*                                      the Drop Profile ECN Parameters to associate
*                                      with the Traffic Class in this Profile.
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
static GT_STATUS internal_cpssDxChPortTx4TcTailDropEcnMarkingProfileGet
(
    IN    GT_U8                                   devNum,
    IN    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet,
    IN    GT_U8                                   trafficClass,
    OUT    CPSS_PORT_QUEUE_ECN_PARAMS_STC         *tailDropEcnProfileParamsPtr
)
{
    GT_STATUS rc;
    GT_U32 regData[4],i,tmp;
    CPSS_PORT_QUEUE_DP_ECN_PARAMS_STC  *dpXEcnAttributesPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E |
        CPSS_AC3X_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(tailDropEcnProfileParamsPtr);
    PRV_CPSS_DXCH_COS_CHECK_4BIT_TC_MAC(devNum,trafficClass);
    PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet, devNum);

    for(i=0;i<3;i++)
    {
       switch (i)
        {
        case 0:
            dpXEcnAttributesPtr = &(tailDropEcnProfileParamsPtr->dp0EcnAttributes);
            break;
        case 1:
            dpXEcnAttributesPtr = &(tailDropEcnProfileParamsPtr->dp1EcnAttributes);
            break;
        default:
            dpXEcnAttributesPtr = &(tailDropEcnProfileParamsPtr->dp2EcnAttributes);
            break;
        }

        rc = prvCpssDxChReadTableEntry(devNum,
            CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E,
        (i<<8)+(profileSet<<4) + trafficClass,regData);
        if(rc !=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        U32_GET_FIELD_IN_ENTRY_MAC(regData, 47,1, tmp);
        dpXEcnAttributesPtr->ecnEnable = BIT2BOOL_MAC(tmp);
        U32_GET_FIELD_IN_ENTRY_MAC(regData, 48,20, dpXEcnAttributesPtr->ecnOffset);


    }

    return GT_OK;
}


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
* @param[out] tailDropEcnProfileParamsPtr -(pointer to)
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTx4TcTailDropEcnMarkingProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, trafficClass,tailDropEcnProfileParamsPtr));

    rc = internal_cpssDxChPortTx4TcTailDropEcnMarkingProfileGet(devNum, profileSet,trafficClass,tailDropEcnProfileParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, trafficClass,tailDropEcnProfileParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortTxTailDropPoolEcnMarkingEnableSet function
* @endinternal
*
* @brief   Set Enable ECN marking based on Pool limit.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] enable               -        Enable/disable ECN marking based on Pool limit
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
static GT_STATUS internal_cpssDxChPortTxTailDropPoolEcnMarkingEnableSet
(
    IN    GT_U8                                   devNum,
    IN    GT_BOOL                                 enable
)
{
    GT_U32    regAddr;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_SIP6_ONLY_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).globalConfig;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,12,1,BOOL2BIT_MAC(enable));

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortTxTailDropPoolEcnMarkingEnableSet function
* @endinternal
*
* @brief   Set Enable ECN marking based on Pool limit.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[in] enable               -        Enable/disable ECN marking based on Pool limit
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxTailDropPoolEcnMarkingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortTxTailDropPoolEcnMarkingEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxTailDropPoolEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get Enable ECN marking based on Pool limit.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[out] enable               -     (pointer to)Enable/disable ECN marking based on Pool limit
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
static GT_STATUS internal_cpssDxChPortTxTailDropPoolEcnMarkingEnableGet
(
    IN    GT_U8                                   devNum,
    OUT    GT_BOOL                                *enablePtr
)
{
    GT_U32    regAddr,tmp;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_APPLICABLE_DEV_SIP6_ONLY_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).globalConfig;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,12,1,&tmp);
    *enablePtr = BIT2BOOL_MAC(tmp);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortTxTailDropPoolEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get Enable ECN marking based on Pool limit.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[out] enable               -     (pointer to)Enable/disable ECN marking based on Pool limit
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
    OUT    GT_BOOL                                *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxTailDropPoolEcnMarkingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortTxTailDropPoolEcnMarkingEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortTxTailDropMcEcnMarkingEnableSet function
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
static GT_STATUS internal_cpssDxChPortTxTailDropMcEcnMarkingEnableSet
(
    IN    GT_U8                                   devNum,
    IN    GT_BOOL                                 enable
)
{
    GT_U32    regAddr;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_SIP6_ONLY_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).globalConfig;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,13,1,BOOL2BIT_MAC(enable));

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}


/**
* @internal cpssDxChPortTxTailDropPoolEcnMarkingEnableSet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxTailDropMcEcnMarkingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortTxTailDropMcEcnMarkingEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChPortTxTailDropMcEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get Enable ECN marking based on multicasr limit.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[out] enable               -     (pointer to)Enable/disable ECN marking based on multicast limit
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
static GT_STATUS internal_cpssDxChPortTxTailDropMcEcnMarkingEnableGet
(
    IN    GT_U8                                   devNum,
    OUT    GT_BOOL                                *enablePtr
)
{
    GT_U32    regAddr,tmp;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    PRV_CPSS_APPLICABLE_DEV_SIP6_ONLY_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).globalConfig;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,13,1,&tmp);
    *enablePtr = BIT2BOOL_MAC(tmp);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChPortTxTailDropMcEcnMarkingEnableGet function
* @endinternal
*
* @brief   Get Enable ECN marking based on multicasr limit.
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum                   - physical device number
* @param[out] enable               -     (pointer to)Enable/disable ECN marking based on multicast limit
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
    OUT    GT_BOOL                                *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxTailDropMcEcnMarkingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortTxTailDropMcEcnMarkingEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableSet function
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
static GT_STATUS internal_cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableSet
(
    IN    GT_U8                                   devNum,
    IN    GT_BOOL                                 enable
)
{
    GT_U32    regAddr;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_SIP6_ONLY_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).globalConfig;

    rc = prvCpssHwPpSetRegField(devNum,regAddr,11,1,BOOL2BIT_MAC(enable));

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableGet function
* @endinternal
*
* @brief   Get enable marking ECN for packets which are mirrored-on-congestion
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;.
*
* @param[in] devNum             - physical device number
* @param[out] enable               -  Enable/disable marking
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
static GT_STATUS internal_cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableGet
(
    IN    GT_U8                                   devNum,
    OUT   GT_BOOL                                 *enablePtr
)
{
    GT_U32    regAddr,tmp;
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_APPLICABLE_DEV_SIP6_ONLY_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).globalConfig;

    rc = prvCpssHwPpGetRegField(devNum,regAddr,11,1,&tmp);
    *enablePtr = BIT2BOOL_MAC(tmp);

    if(rc!=GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

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
* @param[out] enable               -  Enable/disable marking
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChPortTxTailDropEcnMarkingOnCongestionEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


