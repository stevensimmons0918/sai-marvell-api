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
* @file cpssPxPortTxDebug.c
*
* @brief CPSS implementation for configuring the Physical Port Tx Traffic Class
* Queues.
* This covers:
* - queuing enable/disable
* - transmit enable/disable
* - limits of total descriptors in egress queues
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/px/port/cpssPxPortTxDebug.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxRegsVer1.h>
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>


/**
* @internal internal_cpssPxPortTxDebugQueueingEnableSet function
* @endinternal
*
* @brief   Enable/Disable enqueuing to a Traffic Class queue on the specified port
*         of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this device.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE,  enqueuing to the queue
*                                      GT_FALSE, disable enqueuing to the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxDebugQueueingEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      value;      /* temporary variable */
    GT_U32      txqPortNum; /* TxQ port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    value = BOOL2BIT_MAC(enable);

    /* Set <Port<n> Enqueue Enable> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.portEnqueueEnable[txqPortNum],
            tcQueue, 1, value);

    return rc;
}

/**
* @internal cpssPxPortTxDebugQueueingEnableSet function
* @endinternal
*
* @brief   Enable/Disable enqueuing to a Traffic Class queue on the specified port
*         of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this device.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE,  enqueuing to the queue
*                                      GT_FALSE, disable enqueuing to the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugQueueingEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugQueueingEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, enable));

    rc = internal_cpssPxPortTxDebugQueueingEnableSet(devNum, portNum, tcQueue, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxDebugQueueingEnableGet function
* @endinternal
*
* @brief   Get the status of enqueuing to a Traffic Class queue on the specified
*         port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this device.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - GT_TRUE, enqueuing to the queue enabled
*                                      GT_FALSE, enqueuing to the queue disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxDebugQueueingEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      value;      /* temporary variable */
    GT_U32      txqPortNum; /* TxQ port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Get <Port<n> Enqueue Enable> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.portEnqueueEnable[txqPortNum],
            tcQueue, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssPxPortTxDebugQueueingEnableGet function
* @endinternal
*
* @brief   Get the status of enqueuing to a Traffic Class queue on the specified
*         port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this device.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - GT_TRUE, enqueuing to the queue enabled
*                                      GT_FALSE, enqueuing to the queue disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugQueueingEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugQueueingEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, enablePtr));

    rc = internal_cpssPxPortTxDebugQueueingEnableGet(devNum, portNum, tcQueue, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxDebugQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission from a Traffic Class queue on the specified
*         port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE,  transmission from the queue
*                                      GT_FALSE, disable transmission from the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxDebugQueueTxEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      value;      /* temporary variable */
    GT_U32      txqPortNum; /* TxQ port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    value = BOOL2BIT_MAC(enable);

    /* Set <Port<n> Dequeue Enable> register */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.portDequeueEnable[txqPortNum],
            tcQueue, 1, value);

    return rc;
}

/**
* @internal cpssPxPortTxDebugQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission from a Traffic Class queue on the specified
*         port of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port.
*                                      (APPLICABLE RANGES: 0..7).
* @param[in] enable                   - GT_TRUE,  transmission from the queue
*                                      GT_FALSE, disable transmission from the queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugQueueTxEnableSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugQueueTxEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, enable));

    rc = internal_cpssPxPortTxDebugQueueTxEnableSet(devNum, portNum, tcQueue, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxDebugQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission from a Traffic Class queue on the
*         specified port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - GT_TRUE, transmission from the queue enabled
*                                      GT_FALSE, transmission from the queue disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxDebugQueueTxEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      value;      /* temporary variable */
    GT_U32      txqPortNum; /* TxQ port number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Get <Port<n> Dequeue Enable> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).shaper.portDequeueEnable[txqPortNum],
            tcQueue, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return rc;
}

/**
* @internal cpssPxPortTxDebugQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission from a Traffic Class queue on the
*         specified port of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] tcQueue                  - traffic class queue on this Physical Port.
*                                      (APPLICABLE RANGES: 0..7).
*
* @param[out] enablePtr                - GT_TRUE, transmission from the queue enabled
*                                      GT_FALSE, transmission from the queue disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugQueueTxEnableGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  tcQueue,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugQueueTxEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, enablePtr));

    rc = internal_cpssPxPortTxDebugQueueTxEnableGet(devNum, portNum, tcQueue, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxDebugResourceHistogramThresholdSet function
* @endinternal
*
* @brief   Sets threshold for Histogram counter increment.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrNum                  - Histogram Counter number.
*                                      (APPLICABLE RANGES: 0..3).
* @param[in] threshold                - If the Global Descriptors Counter exceeds this Threshold,
*                                      the Histogram Counter is incremented by 1.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxDebugResourceHistogramThresholdSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cntrNum,
    IN  GT_U32          threshold
)
{
    GT_U32  offset;     /* register offset */
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (threshold > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    switch(cntrNum)
    {
        case 0:
            /* <Resource Histogram Limit 0> field of <Resource Histogram Limit 1> register */
            offset = 0;
            regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.resourceHistogram.
                      resourceHistogramLimits.resourceHistogramLimitReg1;
            break;

        case 1:
            /* <Resource Histogram Limit 1> field of <Resource Histogram Limit 1> register */
            offset = 16;
            regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.resourceHistogram.
                      resourceHistogramLimits.resourceHistogramLimitReg1;
            break;

        case 2:
            /* <Resource Histogram Limit 2> field of <Resource Histogram Limit 2> register */
            offset = 0;
            regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.resourceHistogram.
                      resourceHistogramLimits.resourceHistogramLimitReg2;
            break;

        case 3:
            /* <Resource Histogram Limit 3> field of <Resource Histogram Limit 2> register */
            offset = 16;
            regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.resourceHistogram.
                      resourceHistogramLimits.resourceHistogramLimitReg2;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 16, threshold);
}

/**
* @internal cpssPxPortTxDebugResourceHistogramThresholdSet function
* @endinternal
*
* @brief   Sets threshold for Histogram counter increment.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrNum                  - Histogram Counter number.
*                                      (APPLICABLE RANGES: 0..3).
* @param[in] threshold                - If the Global Descriptors Counter exceeds this Threshold,
*                                      the Histogram Counter is incremented by 1.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugResourceHistogramThresholdSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cntrNum,
    IN  GT_U32          threshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugResourceHistogramThresholdSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrNum, threshold));

    rc = internal_cpssPxPortTxDebugResourceHistogramThresholdSet(devNum, cntrNum, threshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrNum, threshold));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxDebugResourceHistogramThresholdGet function
* @endinternal
*
* @brief   Gets threshold for Histogram counter increment.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrNum                  - Histogram Counter number.
*                                      (APPLICABLE RANGES: 0..3).
*
* @param[out] thresholdPtr             - (pointer to) if the Global Descriptors Counter
*                                      exceeds this Threshold, the Histogram Counter
*                                      is incremented by 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxDebugResourceHistogramThresholdGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cntrNum,
    OUT GT_U32          *thresholdPtr
)
{
    GT_U32  offset;     /* register offset */
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(thresholdPtr);

    switch(cntrNum)
    {
        case 0:
            /* <Resource Histogram Limit 0> field of <Resource Histogram Limit 1> register */
            offset = 0;
            regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.resourceHistogram.
                      resourceHistogramLimits.resourceHistogramLimitReg1;
            break;

        case 1:
            /* <Resource Histogram Limit 1> field of <Resource Histogram Limit 1> register */
            offset = 16;
            regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.resourceHistogram.
                      resourceHistogramLimits.resourceHistogramLimitReg1;
            break;

        case 2:
            /* <Resource Histogram Limit 2> field of <Resource Histogram Limit 2> register */
            offset = 0;
            regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.resourceHistogram.
                      resourceHistogramLimits.resourceHistogramLimitReg2;
            break;

        case 3:
            /* <Resource Histogram Limit 3> field of <Resource Histogram Limit 2> register */
            offset = 16;
            regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.resourceHistogram.
                      resourceHistogramLimits.resourceHistogramLimitReg2;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 16, thresholdPtr);
}

/**
* @internal cpssPxPortTxDebugResourceHistogramThresholdGet function
* @endinternal
*
* @brief   Gets threshold for Histogram counter increment.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrNum                  - Histogram Counter number.
*                                      (APPLICABLE RANGES: 0..3).
*
* @param[out] thresholdPtr             - (pointer to) if the Global Descriptors Counter
*                                      exceeds this Threshold, the Histogram Counter
*                                      is incremented by 1.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugResourceHistogramThresholdGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cntrNum,
    OUT GT_U32          *thresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugResourceHistogramThresholdGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrNum, thresholdPtr));

    rc = internal_cpssPxPortTxDebugResourceHistogramThresholdGet(devNum, cntrNum, thresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrNum, thresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxDebugResourceHistogramCounterGet function
* @endinternal
*
* @brief   Gets Histogram Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrNum                  - Histogram Counter number.
*                                      (APPLICABLE RANGES: 0..3).
*
* @param[out] cntrPtr                  - (pointer to) Histogram Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Histogram Counter is cleared on read.
*
*/
static GT_STATUS internal_cpssPxPortTxDebugResourceHistogramCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cntrNum,
    OUT GT_U32          *cntrPtr
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cntrPtr);

    if (cntrNum > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.resourceHistogram.
              resourceHistogramCntrs.resourceHistogramCntr[cntrNum];

    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 32, cntrPtr);
}

/**
* @internal cpssPxPortTxDebugResourceHistogramCounterGet function
* @endinternal
*
* @brief   Gets Histogram Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrNum                  - Histogram Counter number.
*                                      (APPLICABLE RANGES: 0..3).
*
* @param[out] cntrPtr                  - (pointer to) Histogram Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, cntrNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Histogram Counter is cleared on read.
*
*/
GT_STATUS cpssPxPortTxDebugResourceHistogramCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          cntrNum,
    OUT GT_U32          *cntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugResourceHistogramCounterGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrNum, cntrPtr));

    rc = internal_cpssPxPortTxDebugResourceHistogramCounterGet(devNum, cntrNum, cntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrNum, cntrPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxDebugGlobalDescLimitSet function
* @endinternal
*
* @brief   Set limits of total descriptors in all egress queues
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] limit                    - total descriptor  value.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on limit out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Improper configuration of descriptors limit may affect entire system
*       behavior.
*
*/
static GT_STATUS internal_cpssPxPortTxDebugGlobalDescLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          limit
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (limit > 0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Set <TotalDescLimit> field of <Global Descriptors Limit> register */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.globalDescsLimit,
            0, 16, limit);
}

/**
* @internal cpssPxPortTxDebugGlobalDescLimitSet function
* @endinternal
*
* @brief   Set limits of total descriptors in all egress queues
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] limit                    - total descriptor  value.
*                                      (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - on limit out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Improper configuration of descriptors limit may affect entire system
*       behavior.
*
*/
GT_STATUS cpssPxPortTxDebugGlobalDescLimitSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          limit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugGlobalDescLimitSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, limit));

    rc = internal_cpssPxPortTxDebugGlobalDescLimitSet(devNum, limit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, limit));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxDebugGlobalDescLimitGet function
* @endinternal
*
* @brief   Get limits of total descriptors in all egress queues
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] limitPtr                 - pointer to total descriptor limit value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxDebugGlobalDescLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *limitPtr
)
{
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(limitPtr);

    /* Get <TotalDescLimit> field of <Global Descriptors Limit> register */
    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropLimits.globalDescsLimit,
            0, 16, limitPtr);
}

/**
* @internal cpssPxPortTxDebugGlobalDescLimitGet function
* @endinternal
*
* @brief   Get limits of total descriptors in all egress queues
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] limitPtr                 - pointer to total descriptor limit value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on invalid input paramters value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugGlobalDescLimitGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *limitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugGlobalDescLimitGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, limitPtr));

    rc = internal_cpssPxPortTxDebugGlobalDescLimitGet(devNum, limitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, limitPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxDebugGlobalQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE,  transmission
*                                      GT_FALSE, disable transmission
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxDebugGlobalQueueTxEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32  value;  /* temporary variable */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    value = BOOL2BIT_MAC(enable);

    /* Set <PortGroupDequeueEn> field of <Global Dequeue Configuration> register */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.globalDequeueConfig,
            8, 1, value);
}

/**
* @internal cpssPxPortTxDebugGlobalQueueTxEnableSet function
* @endinternal
*
* @brief   Enable/Disable transmission of specified device.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE,  transmission
*                                      GT_FALSE, disable transmission
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugGlobalQueueTxEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugGlobalQueueTxEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPortTxDebugGlobalQueueTxEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortTxDebugGlobalQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) transmission state:
*                                      GT_TRUE, transmission enabled
*                                      GT_FALSE, transmission disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxDebugGlobalQueueTxEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS   rc;     /* return code */
    GT_U32      value;  /* temporary variable */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Get <PortGroupDequeueEn> field of <Global Dequeue Configuration> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum),
            PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).global.globalDQConfig.globalDequeueConfig,
            8, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssPxPortTxDebugGlobalQueueTxEnableGet function
* @endinternal
*
* @brief   Get the status of transmission of specified device (Enable/Disable).
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) transmission state:
*                                      GT_TRUE, transmission enabled
*                                      GT_FALSE, transmission disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters in NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugGlobalQueueTxEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugGlobalQueueTxEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPortTxDebugGlobalQueueTxEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssPxPortTxDebugDescNumberGet function
* @endinternal
*
* @brief   Gets the current number of descriptors allocated per specified port.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortTxDebugDescNumberGet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_U16      *numberPtr
)
{
    GT_U32      value;      /* value to write into register     */
    GT_U32      regAddr;    /* register address                 */
    GT_U32      txqPortNum; /* TXQ port number */
    GT_U32      fieldLength;/* the number of bits to be written to register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    CPSS_NULL_PTR_CHECK_MAC(numberPtr);

    PRV_CPSS_PX_PORT_DESCR_COUNT_REG_MAC(devNum,txqPortNum,&regAddr);
    fieldLength = 16;

    if (prvCpssHwPpPortGroupGetRegField(CAST_SW_DEVNUM(devNum),
                PRV_CPSS_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(CAST_SW_DEVNUM(devNum), txqPortNum),
                regAddr, 0, fieldLength, &value) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    *numberPtr = (GT_U16)value;
    return GT_OK;
}

/**
* @internal cpssPxPortTxDebugDescNumberGet function
* @endinternal
*
* @brief   Gets the current number of descriptors allocated per specified port.
*
* @note   APPLICABLE DEVICES:     Pipe.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numberPtr                - number of descriptors
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortTxDebugDescNumberGet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT GT_U16      *numberPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugDescNumberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, numberPtr));

    rc = internal_cpssPxPortTxDebugDescNumberGet(devNum, portNum, numberPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, numberPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortTxDebugRegisterAddrGet function
* @endinternal
*
* @brief   get register address for 'txq per port' .
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] primaryIndex             - primary index , can be used as :
*                                      port number (also CPU port)
*                                      or as profile number
* @param[in] secondaryIndex           - secondary index , can be used as TC
* @param[in] registerType             - register type
*
* @param[out] regAddrPtr               - (pointer to) register address
*                                       GT_OK
*/
GT_STATUS prvCpssPxPortTxDebugRegisterAddrGet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           primaryIndex,
    IN  GT_U32           secondaryIndex,
    IN  PRV_CPSS_PX_PORT_TX_REG_TYPE_ENT  registerType,
    OUT GT_U32           *regAddrPtr
)
{
    secondaryIndex = secondaryIndex;
    switch(registerType)
    {
        case PRV_CPSS_PX_PORT_TX_REG_TYPE_TX_CONFIG_E:
            /* this register spread to many registers in txqVer1 */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);

        case PRV_CPSS_PX_PORT_TX_REG_TYPE_TOKEN_BUCK_E:
            /* use tables engine , with table:
            CPSS_PX_LOGICAL_TABLE_TXQ_SHAPER_PER_PORT_TOKEN_BUCKET_CONFIG_E
            */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);

        case PRV_CPSS_PX_PORT_TX_REG_TYPE_TOKEN_BUCK_LEN_E:
            /* not used in cpss yet */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);

        case PRV_CPSS_PX_PORT_TX_REG_TYPE_PRIO_TOKEN_BUCK_E:
            /* use tables engine , with table:
            CPSS_PX_LOGICAL_TABLE_TXQ_SHAPER_PER_PORT_PER_PRIO_TOKEN_BUCKET_CONFIG_E
            */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);

        case PRV_CPSS_PX_PORT_TX_REG_TYPE_PRIO_TOKEN_BUCK_LEN_E:
            /* not used in cpss yet */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);

        case PRV_CPSS_PX_PORT_TX_REG_TYPE_DESCR_COUNTER_E:
            *(regAddrPtr) =
                PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropCntrs.portDescCntr[primaryIndex];
        break;

        case PRV_CPSS_PX_PORT_TX_REG_TYPE_BUFFER_COUNTER_E:
            *(regAddrPtr) =
                PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropCntrs.portBuffersCntr[primaryIndex];
        break;

        case PRV_CPSS_PX_PORT_TX_REG_TYPE_TC_DESCR_COUNTER_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
        break;

        case PRV_CPSS_PX_PORT_TX_REG_TYPE_TC_BUFFER_COUNTER_E:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
        break;

        case PRV_CPSS_PX_PROFILE_TX_REG_TYPE_WRR_WEIGHTS_0_E:
            *(regAddrPtr) =
                PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg0[primaryIndex];
        break;

        case PRV_CPSS_PX_PROFILE_TX_REG_TYPE_WRR_WEIGHTS_1_E:
            *(regAddrPtr) =
                PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.profileSDWRRWeightsConfigReg1[primaryIndex];
        break;

        case PRV_CPSS_PX_PROFILE_TX_REG_TYPE_WRR_STRICT_PRIO_E:
            *(regAddrPtr) =
                PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.profileSDWRRGroup[primaryIndex];
        break;

        case PRV_CPSS_PX_PROFILE_TX_REG_TYPE_WRR_STRICT_PRIO_EN_E:
            *(regAddrPtr) =
                PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).scheduler.priorityArbiterWeights.profileSDWRREnable[primaryIndex];
        break;

        case PRV_CPSS_PX_PROFILE_TX_REG_TYPE_DESCR_LIMIT_E:
            /* use explicit register - Port Descriptor Limits or Port Buffer Limits registers. */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortTxDebugTailDropGlobalConfigurationGet function
* @endinternal
*
* @brief   Dumps global tail drop configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
static GT_STATUS prvCpssPxPortTxDebugTailDropGlobalConfigurationGet
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS   rc;
    GT_U32      descNumber;

    cpssOsPrintf("\n");

    rc = cpssPxPortTxDebugGlobalDescLimitGet(devNum, &descNumber);
    if (rc == GT_NOT_APPLICABLE_DEVICE)
    {
        cpssOsPrintf("Global number of Desc available in the System:    [  N/A  ]\n");
    }
    else if (rc != GT_OK)
    {
        cpssOsPrintf("cpssPxPortTxDebugGlobalDescLimitGet failed rc = %x\n", rc);
    }
    else
    {
        cpssOsPrintf("Global number of Desc available in the System:    [ %5d ]\n",
                     descNumber);
    }

    cpssOsPrintf("\n");

    return GT_OK;
}

/**
* @internal prvCpssPxPortTxDebugTxQStatusGet function
* @endinternal
*
* @brief   Dumps the related TxQ parameters in the system.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
static GT_STATUS prvCpssPxPortTxDebugTxQStatusGet
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS               rc;
    GT_U32                  tcQueue;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_BOOL                 txQueueingEnable;
    GT_BOOL                 txQueueEnable;

    cpssOsPrintf("\n");

    cpssOsPrintf("+--------------------------------------------------------------------------------------------------------------+\n");
    cpssOsPrintf("|                        T x Q    E n q u e u e    a n d    T r a s m i t    S T A T U S                       |\n");
    cpssOsPrintf("+------+------------+------------+------------+------------+------------+------------+------------+------------+\n");
    cpssOsPrintf("| Port |     TC0    |     TC1    |     TC2    |     TC3    |     TC4    |     TC5    |     TC6    |     TC7    |\n");
    cpssOsPrintf("|      | EnqEn:TxEn | EnqEn:TxEn | EnqEn:TxEn | EnqEn:TxEn | EnqEn:TxEn | EnqEn:TxEn | EnqEn:TxEn | EnqEn:TxEn |\n");
    cpssOsPrintf("+------+------------+------------+------------+------------+------------+------------+------------+------------+\n");

    for (portNum = 0; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
    {
        if ((PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].valid != GT_TRUE) ||
            (PRV_CPSS_PX_PP_MAC(devNum)->port.portsMapInfoShadowArr[portNum].portMap.txqNum == GT_PX_NA))
        {
            continue;
        }

        cpssOsPrintf("| %3d  |", portNum);

        for (tcQueue = 0; tcQueue < CPSS_TC_RANGE_CNS; tcQueue++)
        {
            rc = cpssPxPortTxDebugQueueingEnableGet(devNum, portNum, tcQueue, &txQueueingEnable);
            if (rc != GT_OK)
            {
                cpssOsPrintf("     N/A    |");
                continue;
            }

            rc = cpssPxPortTxDebugQueueTxEnableGet(devNum, portNum, tcQueue, &txQueueEnable);
            if (rc != GT_OK)
            {
                cpssOsPrintf("     N/A    |");
                continue;
            }

            cpssOsPrintf("  %3d : %-2d  |", txQueueingEnable, txQueueEnable);
        }

        cpssOsPrintf("\n");
    }

    cpssOsPrintf("+------+------------+------------+------------+------------+------------+------------+------------+------------+\n");

    cpssOsPrintf("\n");

    return GT_OK;
}

/**
* @internal internal_cpssPxPortTxDebugQueueDumpAll function
* @endinternal
*
* @brief   Dumps all related Tx Queue information.
*         That includes the static configuration and resources allocated at run-time
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
* @param[in] tailDropDumpBmp          - bitmap to indicate what info to show (0xff for all)
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_BAD_PARAM             - on tailDropDumpBmp not in range.
*/
static GT_STATUS internal_cpssPxPortTxDebugQueueDumpAll
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          tailDropDumpBmp
)
{
    GT_STATUS  rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* check if bitmap value is within range */
    if (tailDropDumpBmp == 0 || tailDropDumpBmp > 0xFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (tailDropDumpBmp & 0x1F)
    {
        cpssOsPrintf("+=============================================================================+\n");
        cpssOsPrintf("|                                                                             |\n");
        cpssOsPrintf("|                       T  A  I  L    D  R  O  P                              |\n");
        cpssOsPrintf("|           R  E  S  O  U  R  C  E    T  H  R  E  S  H  O  L  D  S            |\n");
        cpssOsPrintf("|                                                                             |\n");
        cpssOsPrintf("+=============================================================================+\n");

        if (tailDropDumpBmp & CPSS_PX_PORT_TX_DEBUG_DUMP_TAIL_DROP_GLOBAL_CONFIG)
        {
            rc = prvCpssPxPortTxDebugTailDropGlobalConfigurationGet(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

        cpssOsPrintf("\n\n");
    }

    if (tailDropDumpBmp & 0xE0)
    {
        cpssOsPrintf("+=============================================================================+\n");
        cpssOsPrintf("|                                                                             |\n");
        cpssOsPrintf("|                       T  A  I  L    D  R  O  P                              |\n");
        cpssOsPrintf("|           R  E  S  O  U  R  C  E    A  L  L  O  C  A  T  I  O  N            |\n");
        cpssOsPrintf("|                                                                             |\n");
        cpssOsPrintf("+=============================================================================+\n");

        if (tailDropDumpBmp & CPSS_PX_PORT_TX_DEBUG_DUMP_TX_Q_STATUS)
        {
            rc = prvCpssPxPortTxDebugTxQStatusGet(devNum);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxPortTxDebugQueueDumpAll function
* @endinternal
*
* @brief   Dumps all related Tx Queue information.
*         That includes the static configuration and resources allocated at run-time
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - SW device number
* @param[in] tailDropDumpBmp          - bitmap to indicate what info to show (0xff for all)
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_BAD_PARAM             - on tailDropDumpBmp not in range.
*/
GT_STATUS cpssPxPortTxDebugQueueDumpAll
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          tailDropDumpBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortTxDebugQueueDumpAll);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssPxPortTxDebugQueueDumpAll(devNum, tailDropDumpBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

