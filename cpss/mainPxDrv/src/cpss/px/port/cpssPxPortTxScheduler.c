/*******************************************************************************
*              (c), Copyright 2017, Marvell International Ltd.                 *
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
* @file cpssPxPortTxScheduler.c
*
* @brief CPSS implementation for configuring the Physical Port Tx Traffic Class
* Queues. This APIs cover scheduling algorithms and bandwidth management.
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortTxScheduler.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>
#include <cpss/px/port/private/prvCpssPxPortLog.h>


/* Check the Tx Queue scheduler profile set range */
#define PRV_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_CHECK_MAC(profileSet)            \
    if ((GT_U32)(profileSet) > (GT_U32)CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E) \
    {                                                                          \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);         \
    }


/**
* @internal prvUpdateSchedVarTriggerBitWaitAndSet function
* @endinternal
*
* @brief   Wait for <UpdateSchedVarTrigger> to be ready for triggering and when
*         ready --> trigger new action.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
*/
static GT_STATUS prvUpdateSchedVarTriggerBitWaitAndSet
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_U32      regAddr;    /* register address */
    GT_STATUS   rc;         /* return code */

    /* <Scheduler Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.schedulerConfig.
              schedulerConfig;

    /* Each of the the scheduling parameters need to be set by the    */
    /* <UpdateSchedVarTrigger> in the Transmit Queue Control Register */
    /* in order to take effect.                                       */

    /* wait for bit <UpdateSchedVarTrigger> to clear */
    rc = prvCpssPortGroupBusyWait(CAST_SW_DEVNUM(devNum),
            CPSS_PORT_GROUP_UNAWARE_MODE_CNS, regAddr, 6, GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Trigger the new setting */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 6, 1, 1);
}

/**
* @internal internal_cpssPxPortTxSchedulerProfileIdSet function
* @endinternal
*
* @brief   Bind a port to scheduler profile set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] profileSet               - The Profile Set in which the scheduler's parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerProfileIdSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet
)
{
    GT_STATUS   rc;             /* return code */
    GT_U32      txqPortNum;     /* TXQ port number */
    GT_U32      regAddr;        /* register address */
    GT_U32      regValue;       /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    PRV_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_CHECK_MAC(profileSet);

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.
              portSchedulerProfile[txqPortNum];

    regValue = (GT_U32) profileSet;

    /* Set <TxSched Port<n> Profile> field of <Port<n> Scheduler Profile> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 4, regValue);

    return rc;
}

/**
* @internal cpssPxPortTxSchedulerProfileIdSet function
* @endinternal
*
* @brief   Bind a port to scheduler profile set.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] profileSet               - The Profile Set in which the scheduler's parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerProfileIdSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerProfileIdSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileSet));

    rc = internal_cpssPxPortTxSchedulerProfileIdSet(devNum, portNum, profileSet);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileSet));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerProfileIdGet function
* @endinternal
*
* @brief   Get scheduler profile set that is binded to the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] profileSetPtr            - The Profile Set in which the scheduler's parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerProfileIdGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   *profileSetPtr
)
{
    GT_STATUS   rc;             /* return status */
    GT_U32      txqPortNum;     /* TXQ port number */
    GT_U32      regAddr;        /* register address */
    GT_U32      regValue;       /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    CPSS_NULL_PTR_CHECK_MAC(profileSetPtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.
              portSchedulerProfile[txqPortNum];

    /* Get <TxSched Port<n> Profile> field of <Port<n> Scheduler Profile> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 4, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *profileSetPtr = (CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT) regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxSchedulerProfileIdGet function
* @endinternal
*
* @brief   Get scheduler profile set that is binded to the port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] profileSetPtr            - The Profile Set in which the scheduler's parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerProfileIdGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                        portNum,
    OUT CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   *profileSetPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerProfileIdGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileSetPtr));

    rc = internal_cpssPxPortTxSchedulerProfileIdGet(devNum, portNum, profileSetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileSetPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerWrrMtuSet function
* @endinternal
*
* @brief   Set global parameters for WRR scheduler.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] wrrMtu                   - MTU for DSWRR, resolution of WRR weights for byte based
*                                      Transmit Scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerWrrMtuSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   wrrMtu
)
{
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue = 0; /* register value */
    GT_U32      wrrMtuValue; /* MTU HW value */
    GT_U32      mask;       /* register value mask */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch (wrrMtu)
    {
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64_E:
            wrrMtuValue = 0x00;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_128_E:
            wrrMtuValue = 0x01;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_256_E:
            wrrMtuValue = 0x02;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_512_E:
            wrrMtuValue = 0x03;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_1K_E:
            wrrMtuValue = 0x04;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_2K_E:
            wrrMtuValue = 0x05;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_4K_E:
            wrrMtuValue = 0x06;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_8K_E:
            wrrMtuValue = 0x07;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_16K_E:
            wrrMtuValue = 0x08;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_32K_E:
            wrrMtuValue = 0x09;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64K_E:
            wrrMtuValue = 0x0A;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.schedulerConfig.
              schedulerConfig;

    U32_SET_FIELD_MAC(regValue, 2,  4, wrrMtuValue);
    U32_SET_FIELD_MAC(regValue, 26, 4, wrrMtuValue);
    mask = FIELD_MASK_MAC(2, 4) | FIELD_MASK_MAC(26, 4);

    /* Set <TxSchdMTU> field of <Scheduler Configuration> register */
    return prvCpssHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, mask,
        regValue);
}

/**
* @internal cpssPxPortTxSchedulerWrrMtuSet function
* @endinternal
*
* @brief   Set global parameters for WRR scheduler.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*                                      wrrMode - transmit Scheduler count mode.
* @param[in] wrrMtu                   - MTU for DSWRR, resolution of WRR weights for byte based
*                                      Transmit Scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerWrrMtuSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   wrrMtu
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerWrrMtuSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, wrrMtu));

    rc = internal_cpssPxPortTxSchedulerWrrMtuSet(devNum, wrrMtu);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, wrrMtu));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerWrrMtuGet function
* @endinternal
*
* @brief   Get global parameters for WRR scheduler.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] wrrMtuPtr                - (pointer to) MTU for DSWRR, resolution of WRR weights
*                                      for byte based Transmit Scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
static GT_STATUS internal_cpssPxPortTxSchedulerWrrMtuGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   *wrrMtuPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(wrrMtuPtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.schedulerConfig.
              schedulerConfig;

    /* Get <TxSchdMTU> field of <Scheduler Configuration> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 4, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (regValue)
    {
        case 0x00:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64_E;
            break;
        case 0x01:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_128_E;
            break;
        case 0x02:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_256_E;
            break;
        case 0x03:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_512_E;
            break;
        case 0x04:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_1K_E;
            break;
        case 0x05:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_2K_E;
            break;
        case 0x06:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_4K_E;
            break;
        case 0x07:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_8K_E;
            break;
        case 0x08:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_16K_E;
            break;
        case 0x09:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_32K_E;
            break;
        case 0x0A:
            *wrrMtuPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_64K_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortTxSchedulerWrrMtuGet function
* @endinternal
*
* @brief   Get global parameters for WRR scheduler.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] wrrMtuPtr                - (pointer to) MTU for DSWRR, resolution of WRR weights
*                                      for byte based Transmit Scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*/
GT_STATUS cpssPxPortTxSchedulerWrrMtuGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT CPSS_PX_PORT_TX_SCHEDULER_WRR_MTU_ENT   *wrrMtuPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerWrrMtuGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, wrrMtuPtr));

    rc = internal_cpssPxPortTxSchedulerWrrMtuGet(devNum, wrrMtuPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, wrrMtuPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerWrrProfileSet function
* @endinternal
*
* @brief   Set Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
* @param[in] tcQueue                  - traffic class queue on this Port.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] wrrWeight                - proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this
*                                      Arbitration Group - resolution is 1/255.
*                                      (APPLICABLE RANGES: 0..0xFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If weight will be less than port's MTU (maximum transmit unit) there is
*       possibility for empty WRR loops for given TC queue, but to not tight
*       user and for future ASIC's where this problem will be solved check of
*       wrrWeight 256 > MTU not implemented.
*
*/
static GT_STATUS internal_cpssPxPortTxSchedulerWrrProfileSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    IN  GT_U32                                      wrrWeight
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* field offset */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_CHECK_MAC(profileSet);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    if (wrrWeight > 255)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (tcQueue < 4)
    {
        /* <Profile p SDWRR Weights Configuration Register 0> register */
        regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.
                  profileSDWRRWeightsConfigReg0[profileSet];
        offset = tcQueue * 8;
    }
    else
    {
        /* <Profile p SDWRR Weights Configuration Register 1> register */
        regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.
                  profileSDWRRWeightsConfigReg1[profileSet];
        offset = (tcQueue - 4) * 8;
    }

    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 8, wrrWeight);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Wait for <UpdateSchedVarTrigger> bit of <Scheduler Configuration> register */
    rc = prvUpdateSchedVarTriggerBitWaitAndSet(devNum);

    return rc;
}

/**
* @internal cpssPxPortTxSchedulerWrrProfileSet function
* @endinternal
*
* @brief   Set Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the wrrWeight
*                                      Parameter is associated.
* @param[in] tcQueue                  - traffic class queue on this Port.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] wrrWeight                - proportion of bandwidth assigned to this queue
*                                      relative to the other queues in this
*                                      Arbitration Group - resolution is 1/255.
*                                      (APPLICABLE RANGES: 0..0xFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If weight will be less than port's MTU (maximum transmit unit) there is
*       possibility for empty WRR loops for given TC queue, but to not tight
*       user and for future ASIC's where this problem will be solved check of
*       wrrWeight 256 > MTU not implemented.
*
*/
GT_STATUS cpssPxPortTxSchedulerWrrProfileSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    IN  GT_U32                                      wrrWeight
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerWrrProfileSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, wrrWeight));

    rc = internal_cpssPxPortTxSchedulerWrrProfileSet(devNum, profileSet, tcQueue,
                                                     wrrWeight);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, wrrWeight));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerWrrProfileGet function
* @endinternal
*
* @brief   Get Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue on this Port.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which
*                                      the wrrWeight parameter is associated.
*
* @param[out] wrrWeightPtr             - Pointer to proportion of bandwidth assigned to
*                                      this queue relative to the other queues in this
*                                      Arbitration Group - resolution is 1/255.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerWrrProfileGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    OUT GT_U32                                      *wrrWeightPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      offset;     /* field offset */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_CHECK_MAC(profileSet);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(wrrWeightPtr);

    if (tcQueue < 4)
    {
        /* <Profile p SDWRR Weights Configuration Register 0> register */
        regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.
                  profileSDWRRWeightsConfigReg0[profileSet];
        offset = tcQueue * 8;
    }
    else
    {
        /* <Profile p SDWRR Weights Configuration Register 1> register */
        regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.
                  profileSDWRRWeightsConfigReg1[profileSet];
        offset = (tcQueue - 4) * 8;
    }

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 8, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *wrrWeightPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxSchedulerWrrProfileGet function
* @endinternal
*
* @brief   Get Weighted Round Robin profile on the specified port's
*         Traffic Class Queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue on this Port.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which
*                                      the wrrWeight parameter is associated.
*
* @param[out] wrrWeightPtr             - Pointer to proportion of bandwidth assigned to
*                                      this queue relative to the other queues in this
*                                      Arbitration Group - resolution is 1/255.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerWrrProfileGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    OUT GT_U32                                      *wrrWeightPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerWrrProfileGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, wrrWeightPtr));

    rc = internal_cpssPxPortTxSchedulerWrrProfileGet(devNum, profileSet, tcQueue,
                                                     wrrWeightPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, wrrWeightPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerArbitrationGroupSet function
* @endinternal
*
* @brief   Set Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] arbGroup                 - scheduling arbitration group.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device or arbGroup
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerArbitrationGroupSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    IN  CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     arbGroup
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      wrrArbGroup;            /* WRR arbitration group */
    GT_BOOL     isWrrArbGroupEnabled;   /* WRR arbitration group enable state */
    GT_U32      regAddr;                /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_CHECK_MAC(profileSet);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    switch (arbGroup)
    {
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_0_E:
            isWrrArbGroupEnabled = GT_TRUE;
            wrrArbGroup = 0;
            break;

        case CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_1_E:
            isWrrArbGroupEnabled = GT_TRUE;
            wrrArbGroup = 1;
            break;

        case CPSS_PX_PORT_TX_SCHEDULER_SP_ARB_GROUP_E:
            isWrrArbGroupEnabled = GT_FALSE;
            wrrArbGroup = 0xFF;     /* not used - needed to prevent warning */
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* <Profile p SDWRR Enable> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.
              profileSDWRREnable[profileSet];

    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, tcQueue, 1,
                                BOOL2BIT_MAC(isWrrArbGroupEnabled));
    if (rc != GT_OK)
    {
        return rc;
    }

    if (isWrrArbGroupEnabled)
    {
        /* <Profile p SDWRR Group> register */
        regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.
                  profileSDWRRGroup[profileSet];

        rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, tcQueue, 1, wrrArbGroup);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* Wait for <UpdateSchedVarTrigger> bit of <Scheduler Configuration> register */
    rc = prvUpdateSchedVarTriggerBitWaitAndSet(devNum);

    return rc;
}

/**
* @internal cpssPxPortTxSchedulerArbitrationGroupSet function
* @endinternal
*
* @brief   Set Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] arbGroup                 - scheduling arbitration group.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device or arbGroup
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - previous updated operation not yet completed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerArbitrationGroupSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    IN  CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     arbGroup
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerArbitrationGroupSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, arbGroup));

    rc = internal_cpssPxPortTxSchedulerArbitrationGroupSet(devNum, profileSet,
                                                           tcQueue, arbGroup);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, arbGroup));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerArbitrationGroupGet function
* @endinternal
*
* @brief   Get Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] arbGroupPtr              - Pointer to scheduling arbitration group.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerArbitrationGroupGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    OUT CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     *arbGroupPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      wrrArbGroupEnableBit;   /* WRR Arbitration enable bit */
    GT_U32      wrrArbGroupBit;         /* WRR Arbitration group bit */
    GT_U32      regAddr;                /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_CHECK_MAC(profileSet);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(arbGroupPtr);

    /* <Profile p SDWRR Enable> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.
              priorityArbiterWeights.profileSDWRREnable[profileSet];

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, tcQueue, 1,
                                &wrrArbGroupEnableBit);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (wrrArbGroupEnableBit == 1)
    {
        /* <Profile p SDWRR Group> register */
        regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.
                  priorityArbiterWeights.profileSDWRRGroup[profileSet];

        rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, tcQueue, 1,
                                    &wrrArbGroupBit);
        if (rc != GT_OK)
        {
            return rc;
        }

        if (wrrArbGroupBit == 0)
        {
            *arbGroupPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_0_E;
        }
        else
        {
            *arbGroupPtr = CPSS_PX_PORT_TX_SCHEDULER_WRR_ARB_GROUP_1_E;
        }
    }
    else
    {
        *arbGroupPtr = CPSS_PX_PORT_TX_SCHEDULER_SP_ARB_GROUP_E;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortTxSchedulerArbitrationGroupGet function
* @endinternal
*
* @brief   Get Traffic Class Queue scheduling arbitration group on
*         specificed profile of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Tx Queue scheduler Profile Set in which the arbGroup
*                                      parameter is associated.
* @param[in] tcQueue                  - traffic class queue.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] arbGroupPtr              - Pointer to scheduling arbitration group.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong profile number, device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerArbitrationGroupGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    OUT CPSS_PX_PORT_TX_SCHEDULER_ARB_GROUP_ENT     *arbGroupPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerArbitrationGroupGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, arbGroupPtr));

    rc = internal_cpssPxPortTxSchedulerArbitrationGroupGet(devNum, profileSet,
                                                           tcQueue, arbGroupPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, arbGroupPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerProfileByteCountChangeEnableSet function
* @endinternal
*
* @brief   Enables/Disables profile Byte Count Change of the packet length by per
*         port constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - scheduler  index
* @param[in] bcMode                   - byte count change enable options
* @param[in] bcOp                     - addition or subtraction of the per port value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal profile, bcMode, bcOp
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerProfileByteCountChangeEnableSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile,
    IN  CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  bcMode,
    IN  CPSS_ADJUST_OPERATION_ENT                       bcOp
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_CHECK_MAC(profile);

    /* Set <Enable_conf_sched_bc profile<p>> and       */
    /* <Enable conf shaping bc profile <p>> fields of  */
    /* <Profile <p> Byte Count Modification> register  */
    switch (bcMode)
    {
        case CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_DISABLE_ALL_E:
            regValue = 0x00;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_ONLY_E:
            regValue = 0x01;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SCHEDULER_ONLY_E:
            regValue = 0x02;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_AND_SCHEDULER_E:
            regValue = 0x03;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regValue = regValue << 7;

    /* Set <BC sub en<p>> field of <Profile <p> Byte Modification> register */
    switch (bcOp)
    {
        case CPSS_ADJUST_OPERATION_SUBSTRUCT_E:
            regValue |= 0x01 << 6;
            break;
        case CPSS_ADJUST_OPERATION_ADD_E:
            /* need to set bit to zero - stay as is */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* <Profile <p> Byte Count Modification> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.
              profileByteCountModification[profile];

    rc = prvCpssHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr,
                                    (0x07 << 6), regValue);

    return rc;
}

/**
* @internal cpssPxPortTxSchedulerProfileByteCountChangeEnableSet function
* @endinternal
*
* @brief   Enables/Disables profile Byte Count Change of the packet length by per
*         port constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - scheduler  index
* @param[in] bcMode                   - byte count change enable options
* @param[in] bcOp                     - addition or subtraction of the per port value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal profile, bcMode, bcOp
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerProfileByteCountChangeEnableSet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile,
    IN  CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  bcMode,
    IN  CPSS_ADJUST_OPERATION_ENT                       bcOp
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerProfileByteCountChangeEnableSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, bcMode, bcOp));

    rc = internal_cpssPxPortTxSchedulerProfileByteCountChangeEnableSet(devNum,
            profile, bcMode, bcOp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, bcMode, bcOp));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerProfileByteCountChangeEnableGet function
* @endinternal
*
* @brief   Get profile Byte Count Change mode.
*         The byte count change mode changes the length of a packet by per port
*         constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - scheduler  index
*
* @param[out] bcModePtr                - (pointer to) byte count change enable options.
* @param[out] bcOpPtr                  - (pointer to) addition or subtraction of the per port value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerProfileByteCountChangeEnableGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile,
    OUT CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  *bcModePtr,
    OUT CPSS_ADJUST_OPERATION_ENT                       *bcOpPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_CHECK_MAC(profile);
    CPSS_NULL_PTR_CHECK_MAC(bcModePtr);
    CPSS_NULL_PTR_CHECK_MAC(bcOpPtr);

    /* <Profile <p> Byte Count Modification> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.
              profileByteCountModification[profile];

    rc = prvCpssHwPpReadRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, (0x07 << 6), &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Get <BC sub en<p>> field of <Profile <p> Byte Modification> register */
    *bcOpPtr = (((regValue >> 6) & 0x01) == 1)
               ? CPSS_ADJUST_OPERATION_SUBSTRUCT_E
               : CPSS_ADJUST_OPERATION_ADD_E;

    regValue = (regValue >> 7) & 0x03;

    /* Get <Enable_conf_sched_bc profile<p>> and <Enable conf shaping bc profile <p>> */
    /* fields of <Profile <p> Byte Count Modification> register                       */
    switch (regValue)
    {
        case 0x00:
            *bcModePtr = CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_DISABLE_ALL_E;
            break;
        case 0x01:
            *bcModePtr = CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_ONLY_E;
            break;
        case 0x02:
            *bcModePtr = CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SCHEDULER_ONLY_E;
            break;
        case 0x03:
            *bcModePtr = CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_SHAPER_AND_SCHEDULER_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortTxSchedulerProfileByteCountChangeEnableGet function
* @endinternal
*
* @brief   Get profile Byte Count Change mode.
*         The byte count change mode changes the length of a packet by per port
*         constant for shaping and/or scheduling rate calculation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profile                  - scheduler  index
*
* @param[out] bcModePtr                - (pointer to) byte count change enable options.
* @param[out] bcOpPtr                  - (pointer to) addition or subtraction of the per port value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on illegal profile
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerProfileByteCountChangeEnableGet
(
    IN  GT_SW_DEV_NUM                                   devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT       profile,
    OUT CPSS_PX_PORT_TX_SCHEDULER_BC_CHANGE_ENABLE_ENT  *bcModePtr,
    OUT CPSS_ADJUST_OPERATION_ENT                       *bcOpPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerProfileByteCountChangeEnableGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, bcModePtr, bcOpPtr));

    rc = internal_cpssPxPortTxSchedulerProfileByteCountChangeEnableGet(devNum,
            profile, bcModePtr, bcOpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, bcModePtr, bcOpPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerShaperByteCountChangeValueSet function
* @endinternal
*
* @brief   Sets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] bcValue                  - number of bytes added to the length of every frame
*                                      for shaping/scheduling rate calculation.
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerShaperByteCountChangeValueSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  bcValue
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      txqPortNum; /* TxQ port number */
    GT_U32      regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);

    if (bcValue >= BIT_6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.
              portBCConstantValue[txqPortNum];

    /* Set <Port <n> BC Constant Value> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 6, bcValue);

    return rc;
}

/**
* @internal cpssPxPortTxSchedulerShaperByteCountChangeValueSet function
* @endinternal
*
* @brief   Sets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] bcValue                  - number of bytes added to the length of every frame
*                                      for shaping/scheduling rate calculation.
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerShaperByteCountChangeValueSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  bcValue
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerShaperByteCountChangeValueSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, bcValue));

    rc = internal_cpssPxPortTxSchedulerShaperByteCountChangeValueSet(devNum,
            portNum, bcValue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, bcValue));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerShaperByteCountChangeValueGet function
* @endinternal
*
* @brief   Gets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] bcValuePtr               - (pointer to) number of bytes added to the length of
*                                      every frame for shaping/scheduling rate calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerShaperByteCountChangeValueGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *bcValuePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */
    GT_U32      txqPortNum; /* TxQ port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    CPSS_NULL_PTR_CHECK_MAC(bcValuePtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.
              portBCConstantValue[txqPortNum];

    /* Get <Port <n> BC Constant Value> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 6, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *bcValuePtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortTxSchedulerShaperByteCountChangeValueGet function
* @endinternal
*
* @brief   Gets the number of bytes added/subtracted to the length of every frame
*         for shaping/scheduling rate calculation for given port when
*         Byte Count Change is enabled.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] bcValuePtr               - (pointer to) number of bytes added to the length of
*                                      every frame for shaping/scheduling rate calculation.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerShaperByteCountChangeValueGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *bcValuePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerShaperByteCountChangeValueGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, bcValuePtr));

    rc = internal_cpssPxPortTxSchedulerShaperByteCountChangeValueGet(devNum,
            portNum, bcValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, bcValuePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerProfileCountModeSet function
* @endinternal
*
* @brief   Sets the counting mode for scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - scheduler profile
* @param[in] wrrMode                  - scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong wrrMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxSchedulerProfileCountModeSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile,
    IN  CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      wrrMode
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_CHECK_MAC(profile);

    switch (wrrMode)
    {
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_PACKET_MODE_E:
            regValue = 0x00;
            break;
        case CPSS_PX_PORT_TX_SCHEDULER_WRR_BYTE_MODE_E:
            regValue = 0x01;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* <Profile <p> Byte Count Modification> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.
              profileByteCountModification[profile];

    /* Set <TxSchedCountModeProfile<p>> field of      */
    /* <Profile <p> Byte Count Modification> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 1, regValue);

    return rc;
}

/**
* @internal cpssPxPortTxSchedulerProfileCountModeSet function
* @endinternal
*
* @brief   Sets the counting mode for scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - scheduler profile
* @param[in] wrrMode                  - scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong wrrMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxSchedulerProfileCountModeSet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile,
    IN  CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      wrrMode
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerProfileCountModeSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, wrrMode));

    rc = internal_cpssPxPortTxSchedulerProfileCountModeSet(devNum, profile, wrrMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, wrrMode));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxSchedulerProfileCountModeGet function
* @endinternal
*
* @brief   Sets the counting mode for scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - scheduler profile
*
* @param[out] wrrModePtr               - (pointer to) scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong wrrMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
static GT_STATUS internal_cpssPxPortTxSchedulerProfileCountModeGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile,
    OUT CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      *wrrModePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_TX_SCHEDULER_PROFILE_CHECK_MAC(profile);
    CPSS_NULL_PTR_CHECK_MAC(wrrModePtr);

    /* <Profile <p> Byte Count Modification> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.
              profileByteCountModification[profile];

    /* Get <TxSchedCountModeProfile<p>> field of      */
    /* <Profile <p> Byte Count Modification> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 1, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *wrrModePtr = (regValue == 0)
                  ? CPSS_PX_PORT_TX_SCHEDULER_WRR_PACKET_MODE_E
                  : CPSS_PX_PORT_TX_SCHEDULER_WRR_BYTE_MODE_E;

    return GT_OK;
}

/**
* @internal cpssPxPortTxSchedulerProfileCountModeGet function
* @endinternal
*
* @brief   Sets the counting mode for scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profile                  - scheduler profile
*
* @param[out] wrrModePtr               - (pointer to) scheduler count mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device, profile or wrong wrrMode
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
*/
GT_STATUS cpssPxPortTxSchedulerProfileCountModeGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT   profile,
    OUT CPSS_PX_PORT_TX_SCHEDULER_WRR_MODE_ENT      *wrrModePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxSchedulerProfileCountModeGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profile, wrrModePtr));

    rc = internal_cpssPxPortTxSchedulerProfileCountModeGet(devNum, profile, wrrModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profile, wrrModePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

