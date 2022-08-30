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
* @file cpssPxPortEcn.c
*
* @brief CPSS Pipe Port ECN APIs implementation.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortEcn.h>

/* Maximal Tail Drop profile index */
#define PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_MAX_E     15
/**
* @internal internal_cpssPxPortEcnMarkingEnableSet function
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
static GT_STATUS internal_cpssPxPortEcnMarkingEnableSet
(
    IN GT_SW_DEV_NUM                       devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    IN GT_BOOL                             enable
)
{
    GT_U32                      regAddr;        /* register address */
    GT_U32                      fieldValue;     /* field Value      */
    GT_U32                      fieldOffset;    /* field offset     */
    GT_U32                      fieldSize;      /* field size       */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPECNEnableConfig;

    fieldValue = BOOL2BIT_MAC(enable);
    fieldSize = 1;
    switch (protocolStack)
    {
       case CPSS_IP_PROTOCOL_IPV4_E:
          /*Enable ECN marking for IPv4*/
            fieldOffset = 0;
            break;
       case CPSS_IP_PROTOCOL_IPV6_E:
          /*Enable ECN marking for IPv6*/
            fieldOffset = 1;
            break;
       case CPSS_IP_PROTOCOL_IPV4V6_E:
          /*Enable ECN marking for dual stack*/
            fieldOffset = 0;
            fieldValue *= 3; /* assign 0,1 bits to be b00 or b11 */
            fieldSize = 2;
            break;
       default:
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldSize, fieldValue);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEcnMarkingEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, enable));

    rc = internal_cpssPxPortEcnMarkingEnableSet(devNum, protocolStack, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortEcnMarkingEnableGet function
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
static GT_STATUS internal_cpssPxPortEcnMarkingEnableGet
(
    IN GT_SW_DEV_NUM                       devNum,
    IN CPSS_IP_PROTOCOL_STACK_ENT          protocolStack,
    OUT GT_BOOL                            *enablePtr
)
{
    GT_U32                      regAddr;        /* register address */
    GT_U32                      bitValue;       /* bit value        */
    GT_STATUS                   rc;             /* return code      */
    GT_U32                      fieldOffset;    /* register field offset */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_PCP_MAC(devNum).PRS.PCPECNEnableConfig;

    switch (protocolStack)
    {
       case CPSS_IP_PROTOCOL_IPV4_E:
          /*Get status of ECN assignment for IPv4 header*/
           fieldOffset = 0;
           break;
       case CPSS_IP_PROTOCOL_IPV6_E:
          /*Get status of ECN assignment for IPv6 header*/
           fieldOffset = 1;
           break;
       default:
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, 1, &bitValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(bitValue);

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEcnMarkingEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, protocolStack, enablePtr));

    rc = internal_cpssPxPortEcnMarkingEnableGet(devNum, protocolStack, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, protocolStack, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortEcnMarkingTailDropProfileEnableSet function
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
static GT_STATUS internal_cpssPxPortEcnMarkingTailDropProfileEnableSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    IN  CPSS_PX_PORT_ECN_ENABLERS_STC               *enablersPtr
)
{
    GT_U32 regAddr;             /* register address     */
    GT_U32 regVal = 0;          /* register value       */
    
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablersPtr);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profileSet);

    /* <Profile <%p> ECN Mark En> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.profileECNMarkEn[profileSet];

    U32_SET_FIELD_MAC(regVal, 0, 1, enablersPtr->tcDpLimit);
    U32_SET_FIELD_MAC(regVal, 1, 1, enablersPtr->portLimit);
    U32_SET_FIELD_MAC(regVal, 2, 1, enablersPtr->sharedPoolLimit);
    U32_SET_FIELD_MAC(regVal, 3, 1, enablersPtr->tcLimit);

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 4, regVal);
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEcnMarkingTailDropProfileEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, enablersPtr));

    rc = internal_cpssPxPortEcnMarkingTailDropProfileEnableSet(devNum, profileSet, enablersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, enablersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortEcnMarkingTailDropProfileEnableGet function
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
static GT_STATUS internal_cpssPxPortEcnMarkingTailDropProfileEnableGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    OUT CPSS_PX_PORT_ECN_ENABLERS_STC               *enablersPtr
)
{
    GT_U32 regAddr;    /* register address     */
    GT_U32 regData;    /* register value       */
    GT_STATUS rc;      /* return code */
    
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablersPtr);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profileSet);

    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.profileECNMarkEn[profileSet];
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 4, &regData);
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortEcnMarkingTailDropProfileEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, enablersPtr));

    rc = internal_cpssPxPortEcnMarkingTailDropProfileEnableGet(devNum, profileSet, enablersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, enablersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


