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
* @file cpssPxPortPfc.c
*
* @brief CPSS implementation for Priority Flow Control functionality.
*
* @version   41
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/cos/private/prvCpssPxCoS.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/cpssPxPortPfc.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>


#define PRV_CPSS_PX_PFC_THRESHOLD_MAX_VALUE   0x1FFFFF
#define PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM      21

/* Convert CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT to HW value */
#define PRV_CPSS_PX_PORT_PFC_DBA_ALPHA_TO_HW_CONVERT_MAC(alpha, hwValue) \
    switch (alpha)                                                            \
    {                                                                         \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:                         \
            hwValue = 0;                                                      \
            break;                                                            \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:                      \
            hwValue = 1;                                                      \
            break;                                                            \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:                       \
            hwValue = 2;                                                      \
            break;                                                            \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:                         \
            hwValue = 3;                                                      \
            break;                                                            \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:                         \
            hwValue = 4;                                                      \
            break;                                                            \
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:                         \
            hwValue = 5;                                                      \
            break;                                                            \
        default:                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);    \
    }

/* Convert HW value to CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT */
#define PRV_CPSS_PX_PORT_PFC_DBA_HW_TO_ALPHA_CONVERT_MAC(hwValue, alpha) \
    switch (hwValue)                                                          \
    {                                                                         \
        case 0:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;                  \
            break;                                                            \
        case 1:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E;               \
            break;                                                            \
        case 2:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E;                \
            break;                                                            \
        case 3:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;                  \
            break;                                                            \
        case 4:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E;                  \
            break;                                                            \
        case 5:                                                               \
            alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;                  \
            break;                                                            \
        default:                                                              \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);         \
    }

/* Check the profile set range */
#define PRV_CPSS_PX_PORT_PFC_PROFILE_SET_CHECK_MAC(profile)          \
    if ((GT_U32)(profile) > (GT_U32) CPSS_PX_PORT_TX_SCHEDULER_PROFILE_15_E)  \
    {                                                                         \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);        \
    }


/**
* @internal internal_cpssPxPortPfcEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) response functionality.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcEnable                - PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If PFC response is enabled, the shaper’s baseline must be
*       at least 0x3FFFC0, see:
*       cpssPxPortTxShaperBaselineSet.
*       Note: Triggering cannot be disabled by this API.
*
*/
static GT_STATUS internal_cpssPxPortPfcEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN CPSS_PX_PORT_PFC_ENABLE_ENT pfcEnable
)
{
    GT_U32 regAddr;       /* register address */
    GT_STATUS rc;         /* function return value */
    GT_U32 value ;         /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (pfcEnable==CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_ONLY_E)
    {
        value=0;
    }
    else if (pfcEnable==CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E)
    {
        value=1;
    }
    else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* To globally enable triggering PFC events - set <PfcEn> field ( bit 0) in the PFC Trigger Global Configuration Register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCTriggerGlobalConfig;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set value in Flow Control Response Config reg. */
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.flowCtrlResponseConfig;
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, value);
}

/**
* @internal cpssPxPortPfcEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) response functionality.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcEnable                - PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note If PFC response is enabled, the shaper’s baseline must be
*       at least 0x3FFFC0, see:
*       cpssPxPortTxShaperBaselineSet.
*       Note: Triggering cannot be disabled by this API.
*
*/
GT_STATUS cpssPxPortPfcEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN CPSS_PX_PORT_PFC_ENABLE_ENT pfcEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcEnable));

    rc = internal_cpssPxPortPfcEnableSet(devNum, pfcEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcEnable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcEnableGet function
* @endinternal
*
* @brief   Get the status of PFC response functionality.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcEnablePtr             - (pointer to) PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering is enabled by default.
*
*/
static GT_STATUS internal_cpssPxPortPfcEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PX_PORT_PFC_ENABLE_ENT *pfcEnablePtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 responseValue; /* registers value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(pfcEnablePtr);
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.flowCtrlResponseConfig;

    rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &responseValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (responseValue)
    {
        *pfcEnablePtr = CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E;
    }
    else
        *pfcEnablePtr = CPSS_PX_PORT_PFC_ENABLE_TRIGGERING_ONLY_E;

   return GT_OK;
}

/**
* @internal cpssPxPortPfcEnableGet function
* @endinternal
*
* @brief   Get the status of PFC response functionality.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcEnablePtr             - (pointer to) PFC enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Triggering is enabled by default.
*
*/
GT_STATUS cpssPxPortPfcEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PX_PORT_PFC_ENABLE_ENT *pfcEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcEnablePtr));

    rc = internal_cpssPxPortPfcEnableGet(devNum, pfcEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcProfileIndexSet function
* @endinternal
*
* @brief   Binds a source port to a PFC profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcProfileIndexSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   profileIndex
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 fieldOffset; /* the start bit number in the register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if (profileIndex >= BIT_3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCPortProfile[(portNum >> 3)];
    fieldOffset = (portNum % 8) * 3;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, 3, profileIndex);
}

/**
* @internal cpssPxPortPfcProfileIndexSet function
* @endinternal
*
* @brief   Binds a source port to a PFC profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range profile index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcProfileIndexSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32   profileIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcProfileIndexSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileIndex));

    rc = internal_cpssPxPortPfcProfileIndexSet(devNum, portNum, profileIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileIndex));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcProfileIndexGet function
* @endinternal
*
* @brief   Gets the port's PFC profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] profileIndexPtr          - (pointer to) profile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcProfileIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *profileIndexPtr
)
{
    GT_U32 regAddr,index;     /* register address */
    GT_U32 fieldOffset; /* the start bit number in the register */
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    CPSS_NULL_PTR_CHECK_MAC(profileIndexPtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCPortProfile[(portNum >> 3)];
    fieldOffset = (portNum % 8) * 3;

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, 3, &index);
    *profileIndexPtr = index;
    return rc;
}

/**
* @internal cpssPxPortPfcProfileIndexGet function
* @endinternal
*
* @brief   Gets the port's PFC profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] profileIndexPtr          - (pointer to) profile index.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcProfileIndexGet
(
    IN  GT_U8   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *profileIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcProfileIndexGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileIndexPtr));

    rc = internal_cpssPxPortPfcProfileIndexGet(devNum, portNum, profileIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssPxPortPfcDbaAvailableBuffersSet function
* @endinternal
*
* @brief   Define the number of buffers that are Available for the PFC Dynamic threshold calculations.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*                                      buffNum      - number of buffers available for DBA.
*                                      (APPLICABLE RANGE: 0..0xFFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range buffers number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcDbaAvailableBuffersSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32         buffsNum
)
{
    GT_U32 regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (buffsNum >= BIT_24)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).dba.PFCAvailableBuffers;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 24, buffsNum);
}

/**
* @internal cpssPxPortPfcDbaAvailableBuffersSet function
* @endinternal
*
* @brief   Define the number of buffers that are Available for the PFC Dynamic threshold calculations.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*                                      buffNum      - number of buffers available for DBA.
*                                      (APPLICABLE RANGE: 0..0xFFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range buffers number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcDbaAvailableBuffersSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32         buffsNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcDbaAvailableBuffersSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, buffsNum));

    rc = internal_cpssPxPortPfcDbaAvailableBuffersSet(devNum, buffsNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, buffsNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcDbaAvailableBuffersGet function
* @endinternal
*
* @brief   Get the number of buffers that are Available for the PFC Dynamic threshold calculations.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] buffsNumPtr              - pointer to number of buffers available for DBA.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range buffers number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcDbaAvailableBuffersGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_U32         *buffsNumPtr
)
{
    GT_U32 regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(buffsNumPtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).dba.PFCAvailableBuffers;

    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 24, buffsNumPtr);
}

/**
* @internal cpssPxPortPfcDbaAvailableBuffersGet function
* @endinternal
*
* @brief   Get the number of buffers that are Available for the PFC Dynamic threshold calculations.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*                                      OUTPUTS:
* @param[in] buffsNumPtr              - pointer to number of buffers available for DBA.
*
* @param[out] buffsNumPtr              - pointer to number of buffers available for DBA.
*                                      OUTPUTS:
*                                      None
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_OUT_OF_RANGE          - on out of range buffers number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcDbaAvailableBuffersGet
(
    IN  GT_SW_DEV_NUM  devNum,
    OUT GT_U32         *buffsNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcDbaAvailableBuffersGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, buffsNumPtr));

    rc = internal_cpssPxPortPfcDbaAvailableBuffersGet(devNum, buffsNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, buffsNumPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssPxPortPfcProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcProfileCfgPtr         - pointer to PFC Profile configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
static GT_STATUS internal_cpssPxPortPfcProfileQueueConfigSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32   profileIndex,
    IN GT_U32   tcQueue,
    IN CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 data;
    GT_STATUS rc;     /* function return value */
    GT_U32    xonAlphaHwValue,xoffAlphaHwValue;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(pfcProfileCfgPtr);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    PRV_CPSS_PX_PORT_PFC_DBA_ALPHA_TO_HW_CONVERT_MAC(pfcProfileCfgPtr->xonAlpha, xonAlphaHwValue);
    PRV_CPSS_PX_PORT_PFC_DBA_ALPHA_TO_HW_CONVERT_MAC(pfcProfileCfgPtr->xoffAlpha, xoffAlphaHwValue);

    if (profileIndex > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((pfcProfileCfgPtr->xoffThreshold > PRV_CPSS_PX_PFC_THRESHOLD_MAX_VALUE) ||
        (pfcProfileCfgPtr->xonThreshold  > PRV_CPSS_PX_PFC_THRESHOLD_MAX_VALUE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).FCModeProfileTCXOffThresholds[profileIndex][tcQueue];
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM, pfcProfileCfgPtr->xoffThreshold);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).FCModeProfileTCXOnThresholds[profileIndex][tcQueue];
    rc =  prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM, pfcProfileCfgPtr->xonThreshold);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).dba.PFCProfileTCAlphaThresholds[profileIndex][tcQueue];
    data = xonAlphaHwValue | (xoffAlphaHwValue << 4);

    return   prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 8, data);

}

/**
* @internal cpssPxPortPfcProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcProfileCfgPtr         - pointer to PFC Profile configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
GT_STATUS cpssPxPortPfcProfileQueueConfigSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32   profileIndex,
    IN GT_U32   tcQueue,
    IN CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcProfileQueueConfigSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, tcQueue, pfcProfileCfgPtr));

    rc = internal_cpssPxPortPfcProfileQueueConfigSet(devNum, profileIndex, tcQueue, pfcProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, tcQueue, pfcProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] pfcProfileCfgPtr         - pointer to PFC Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
static GT_STATUS internal_cpssPxPortPfcProfileQueueConfigGet
(
    IN GT_SW_DEV_NUM    devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U32    tcQueue,
    OUT CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 data;
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(pfcProfileCfgPtr);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    if (profileIndex > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

   regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).FCModeProfileTCXOffThresholds[profileIndex][tcQueue];
   rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM, &(pfcProfileCfgPtr->xoffThreshold));
   if (rc != GT_OK)
   {
       return rc;
   }

   regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).FCModeProfileTCXOnThresholds[profileIndex][tcQueue];
   rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM, &(pfcProfileCfgPtr->xonThreshold));
   if (rc != GT_OK)
   {
       return rc;
   }
   regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).dba.PFCProfileTCAlphaThresholds[profileIndex][tcQueue];
   rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 4, &(data));
   if (rc != GT_OK)
   {
       return rc;
   }

   PRV_CPSS_PX_PORT_PFC_DBA_HW_TO_ALPHA_CONVERT_MAC(data, pfcProfileCfgPtr->xonAlpha);
   rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 4, 4, &(data));
   if (rc != GT_OK)
   {
       return rc;
   }

   PRV_CPSS_PX_PORT_PFC_DBA_HW_TO_ALPHA_CONVERT_MAC(data, pfcProfileCfgPtr->xoffAlpha);

   return rc;

}

/**
* @internal cpssPxPortPfcProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] profileIndex             - profile index (APPLICABLE RANGES: 0..7)
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] pfcProfileCfgPtr         - pointer to PFC Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
GT_STATUS cpssPxPortPfcProfileQueueConfigGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32   profileIndex,
    IN  GT_U32    tcQueue,
    OUT CPSS_PX_PORT_PFC_PROFILE_CONFIG_STC     *pfcProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcProfileQueueConfigGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileIndex, tcQueue, pfcProfileCfgPtr));

    rc = internal_cpssPxPortPfcProfileQueueConfigGet(devNum, profileIndex, tcQueue, pfcProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileIndex, tcQueue, pfcProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcCountingModeSet function
* @endinternal
*
* @brief   Sets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcCountMode             - PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcCountingModeSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN CPSS_PX_PORT_PFC_COUNT_MODE_ENT  pfcCountMode
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    switch (pfcCountMode)
    {
        case CPSS_PX_PORT_PFC_COUNT_BUFFERS_MODE_E:
            value = 0;
            break;
        case CPSS_PX_PORT_PFC_COUNT_PACKETS_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCTriggerGlobalConfig;
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 1, value);

}

/**
* @internal cpssPxPortPfcCountingModeSet function
* @endinternal
*
* @brief   Sets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcCountMode             - PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcCountingModeSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN CPSS_PX_PORT_PFC_COUNT_MODE_ENT  pfcCountMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcCountingModeSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcCountMode));

    rc = internal_cpssPxPortPfcCountingModeSet(devNum, pfcCountMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcCountMode));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcCountingModeGet function
* @endinternal
*
* @brief   Gets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcCountModePtr          - (pointer to) PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcCountingModeGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PX_PORT_PFC_COUNT_MODE_ENT  *pfcCountModePtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    CPSS_PX_PORT_PFC_COUNT_MODE_ENT mode;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(pfcCountModePtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCTriggerGlobalConfig;
    rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    mode = (value == 0) ? CPSS_PX_PORT_PFC_COUNT_BUFFERS_MODE_E : CPSS_PX_PORT_PFC_COUNT_PACKETS_E;
    *pfcCountModePtr = mode;
    return GT_OK;
}

/**
* @internal cpssPxPortPfcCountingModeGet function
* @endinternal
*
* @brief   Gets PFC counting mode.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] pfcCountModePtr          - (pointer to) PFC counting mode.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcCountingModeGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PX_PORT_PFC_COUNT_MODE_ENT  *pfcCountModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcCountingModeGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcCountModePtr));

    rc = internal_cpssPxPortPfcCountingModeGet(devNum, pfcCountModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcCountModePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcGlobalDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC global drop.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: Enable PFC global drop.
*                                      GT_FALSE: Disable PFC global drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To configure drop threshold use cpssPxPortPfcGlobalQueueConfigSet.
*
*/
static GT_STATUS internal_cpssPxPortPfcGlobalDropEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_BOOL  enable
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_U32 bitIndex;  /* bit index */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    value = BOOL2BIT_MAC(enable);

    bitIndex = 10;
    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCTriggerGlobalConfig;
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, bitIndex, 1, value);
}

/**
* @internal cpssPxPortPfcGlobalDropEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC global drop.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: Enable PFC global drop.
*                                      GT_FALSE: Disable PFC global drop.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To configure drop threshold use cpssPxPortPfcGlobalQueueConfigSet.
*
*/
GT_STATUS cpssPxPortPfcGlobalDropEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcGlobalDropEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPortPfcGlobalDropEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcGlobalDropEnableGet function
* @endinternal
*
* @brief   Gets the current status of PFC global drop.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of PFC functionality
*                                      GT_TRUE:  PFC global drop enabled.
*                                      GT_FALSE: PFC global drop disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcGlobalDropEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32 bitIndex;  /* bit index */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    bitIndex = 10;
    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCTriggerGlobalConfig;
    rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, bitIndex, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssPxPortPfcGlobalDropEnableGet function
* @endinternal
*
* @brief   Gets the current status of PFC global drop.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of PFC functionality
*                                      GT_TRUE:  PFC global drop enabled.
*                                      GT_FALSE: PFC global drop disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcGlobalDropEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_BOOL  *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcGlobalDropEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPortPfcGlobalDropEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcGlobalQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*         APPLICABLE DEVICES:
*         Pipe
*         NOT APPLICABLE DEVICES:
*         None
*         INPUTS:
*         devNum    - device number
*         tcQueue    - traffic class queue
*         APPLICABLE RANGES: 0..7
*         xoffThreshold - XOFF threshold
*         APPLICABLE RANGES: 0..0x1FFFFF
*         dropThreshold - Drop threshold. When a global counter with given tcQueue
*         crosses up the dropThreshold the packets are dropped
*         APPLICABLE RANGES: 0..0x1FFFFF
*         xonThreshold - XON threshold
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue
*                                      APPLICABLE RANGES: 0..7
* @param[in] xoffThreshold            - XOFF threshold
*                                      APPLICABLE RANGES:  0..0x1FFFFF
* @param[in] dropThreshold            - Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped
*                                      APPLICABLE RANGES:  0..0x1FFFFF
* @param[in] xonThreshold             - XON threshold
*                                      APPLICABLE RANGES: 0..0x1FFFFF
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. dropThreshold is used when PFC global drop is enabled.
*       See cpssPxPortPfcGlobalDropEnableSet.
*       2. All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
static GT_STATUS internal_cpssPxPortPfcGlobalQueueConfigSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32   tcQueue,
    IN GT_U32   xoffThreshold,
    IN GT_U32   dropThreshold,
    IN GT_U32   xonThreshold
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    if (xoffThreshold > PRV_CPSS_PX_PFC_THRESHOLD_MAX_VALUE ||
        dropThreshold > PRV_CPSS_PX_PFC_THRESHOLD_MAX_VALUE ||
        xonThreshold  > PRV_CPSS_PX_PFC_THRESHOLD_MAX_VALUE)
    {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCXoffThresholds[tcQueue][0];
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0,
                                PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM, xoffThreshold);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCDropThresholds[tcQueue][0];
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0,
                                PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM, dropThreshold);
    if (rc != GT_OK)
    {
            return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCXonThresholds[tcQueue][0];
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0,
                                PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM, xonThreshold);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortPfcGlobalQueueConfigSet function
* @endinternal
*
* @brief   Sets PFC profile configurations for given tc queue.
*         APPLICABLE DEVICES:
*         Pipe
*         NOT APPLICABLE DEVICES:
*         None
*         INPUTS:
*         devNum    - device number
*         tcQueue    - traffic class queue (APPLICABLE RANGES: 0..7)
*         xoffThreshold - XOFF threshold
*         APPLICABLE RANGES: 0..0x1FFFFF
*         dropThreshold - Drop threshold. When a global counter with given tcQueue
*         crosses up the dropThreshold the packets are dropped
*         APPLICABLE RANGES: 0..0x1FFFFF
*         xonThreshold - XON threshold
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] xoffThreshold            - XOFF threshold
*                                      APPLICABLE RANGES:  0..0x1FFFFF
* @param[in] dropThreshold            - Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped
*                                      APPLICABLE RANGES:  0..0x1FFFFF
* @param[in] xonThreshold             - XON threshold
*                                      APPLICABLE RANGES:  0..0x1FFFFF
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. dropThreshold is used when PFC global drop is enabled.
*       See cpssPxPortPfcGlobalDropEnableSet.
*       2. All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
GT_STATUS cpssPxPortPfcGlobalQueueConfigSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_U32    tcQueue,
    IN GT_U32   xoffThreshold,
    IN GT_U32   dropThreshold,
    IN GT_U32   xonThreshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcGlobalQueueConfigSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, xoffThreshold, dropThreshold, xonThreshold));

    rc = internal_cpssPxPortPfcGlobalQueueConfigSet(devNum, tcQueue, xoffThreshold, dropThreshold, xonThreshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, xoffThreshold, dropThreshold, xonThreshold));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcGlobalQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] xoffThresholdPtr         - (pointer to) Xoff threshold.
* @param[out] dropThresholdPtr         - (pointer to) Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped.
* @param[out] xonThresholdPtr          - (pointer to) Xon threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
static GT_STATUS internal_cpssPxPortPfcGlobalQueueConfigGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32    tcQueue,
    OUT GT_U32   *xoffThresholdPtr,
    OUT GT_U32   *dropThresholdPtr,
    OUT GT_U32   *xonThresholdPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(xoffThresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(dropThresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(xonThresholdPtr);

    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCXoffThresholds[tcQueue][0];
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0,
                                PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM, xoffThresholdPtr);
    if (rc != GT_OK)
    {
            return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCDropThresholds[tcQueue][0];
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0,
                                PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM, dropThresholdPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCXonThresholds[tcQueue][0];
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0,
                                PRV_CPSS_PX_PFC_THRESHOLD_BITS_NUM, xonThresholdPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssPxPortPfcGlobalQueueConfigGet function
* @endinternal
*
* @brief   Gets PFC profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
*
* @param[out] xoffThresholdPtr         - (pointer to) Xoff threshold.
* @param[out] dropThresholdPtr         - (pointer to) Drop threshold. When a global counter with given tcQueue
*                                      crosses up the dropThreshold the packets are dropped.
* @param[out] xonThresholdPtr          - (pointer to) Xon threshold.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile index
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All thresholds are set in buffers or packets.
*       See cpssPxPortPfcCountingModeSet.
*
*/
GT_STATUS cpssPxPortPfcGlobalQueueConfigGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_U32    tcQueue,
    OUT GT_U32   *xoffThresholdPtr,
    OUT GT_U32   *dropThresholdPtr,
    OUT GT_U32   *xonThresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcGlobalQueueConfigGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, xoffThresholdPtr, dropThresholdPtr, xonThresholdPtr));

    rc = internal_cpssPxPortPfcGlobalQueueConfigGet(devNum, tcQueue, xoffThresholdPtr, dropThresholdPtr, xonThresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, xoffThresholdPtr, dropThresholdPtr, xonThresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssPxPortPfcSourcePortToPfcCounterSet function
* @endinternal
*
* @brief   Set PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcSourcePortToPfcCounterSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32  pfcCounterNum
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    if (pfcCounterNum >= BIT_7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCSourcePortToPFCIndexMap[portNum];

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 7, pfcCounterNum);
}

/**
* @internal cpssPxPortPfcSourcePortToPfcCounterSet function
* @endinternal
*
* @brief   Set PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..127)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcSourcePortToPfcCounterSet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_U32  pfcCounterNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcSourcePortToPfcCounterSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pfcCounterNum));

    rc = internal_cpssPxPortPfcSourcePortToPfcCounterSet(devNum, portNum, pfcCounterNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pfcCounterNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcSourcePortToPfcCounterGet function
* @endinternal
*
* @brief   Get PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] pfcCounterNumPtr         (pointer to) PFC counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcSourcePortToPfcCounterGet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *pfcCounterNumPtr
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(pfcCounterNumPtr);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCSourcePortToPFCIndexMap[portNum];

    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 7, pfcCounterNumPtr);
}

/**
* @internal cpssPxPortPfcSourcePortToPfcCounterGet function
* @endinternal
*
* @brief   Get PFC mapping of a physical port to a PFC counter.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
*
* @param[out] pfcCounterNumPtr         (pointer to) PFC counter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcSourcePortToPfcCounterGet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32  *pfcCounterNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcSourcePortToPfcCounterGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, pfcCounterNumPtr));

    rc = internal_cpssPxPortPfcSourcePortToPfcCounterGet(devNum, portNum, pfcCounterNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, pfcCounterNumPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortPfcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per PFC counter and traffic class.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..17)
*
* @param[out] pfcCounterValuePtr       - (pointer to) PFC counter value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcCounterGet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_U32  tcQueue,
    IN  GT_U32  pfcCounterNum,
    OUT GT_U32  *pfcCounterValuePtr
)
{
    GT_STATUS rc;  /* return code */
    CPSS_PX_TABLE_ENT tableType;
    GT_U32  maxNum;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(pfcCounterValuePtr);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    maxNum = 17;

    if (pfcCounterNum >= maxNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "pfcCounterNum[%d] is out of range [0..%d]",
                                      pfcCounterNum, maxNum - 1);
    }

    tableType = CPSS_PX_TABLE_PFC_LLFC_COUNTERS_E;

    rc = prvCpssPxReadTableEntry(devNum, tableType, ((pfcCounterNum << 3) + tcQueue), pfcCounterValuePtr);

    return rc;
}

/**
* @internal cpssPxPortPfcCounterGet function
* @endinternal
*
* @brief   Get PFC counter value per PFC counter and traffic class.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue (APPLICABLE RANGES: 0..7)
* @param[in] pfcCounterNum            - PFC counter (APPLICABLE RANGES: 0..17)
*
* @param[out] pfcCounterValuePtr       - (pointer to) PFC counter value
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - on wrong pfcCounterNum
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcCounterGet
(
    IN  GT_SW_DEV_NUM  devNum,
    IN  GT_U32  tcQueue,
    IN  GT_U32  pfcCounterNum,
    OUT GT_U32  *pfcCounterValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcCounterGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, pfcCounterNum, pfcCounterValuePtr));

    rc = internal_cpssPxPortPfcCounterGet(devNum, tcQueue, pfcCounterNum, pfcCounterValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, pfcCounterNum, pfcCounterValuePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcTimerMapEnableSet function
* @endinternal
*
* @brief   Enables mapping of PFC timer to priority queue for given scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] enable                   - Determines whether PFC timer to Priority Queue map
*                                      is used.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssPxPortTxBindPortToSchedulerProfileSet.
*
*/
static GT_STATUS internal_cpssPxPortPfcTimerMapEnableSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_BOOL                                 enable

)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_PFC_PROFILE_SET_CHECK_MAC(profileSet);

    value = BOOL2BIT_MAC(enable);
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.schedulerProfilePFCTimerToPriorityQueueMapEnable[profileSet];

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, value);
}

/**
* @internal cpssPxPortPfcTimerMapEnableSet function
* @endinternal
*
* @brief   Enables mapping of PFC timer to priority queue for given scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] enable                   - Determines whether PFC timer to Priority Queue map
*                                      is used.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssPxPortTxBindPortToSchedulerProfileSet.
*
*/
GT_STATUS cpssPxPortPfcTimerMapEnableSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_BOOL                                 enable

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcTimerMapEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, enable));

    rc = internal_cpssPxPortPfcTimerMapEnableSet(devNum, profileSet, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcTimerMapEnableGet function
* @endinternal
*
* @brief   Get the status of PFS timer to priority queue mapping for given
*         scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile Set
*
* @param[out] enablePtr                - (pointer to) status of PFC timer to Priority Queue
*                                      mapping.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcTimerMapEnableGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT GT_BOOL                                *enablePtr

)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_PX_PORT_PFC_PROFILE_SET_CHECK_MAC(profileSet);

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.schedulerProfilePFCTimerToPriorityQueueMapEnable[profileSet];

    rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &value);

    if (rc != GT_OK)
    {
        return rc;
    }
    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssPxPortPfcTimerMapEnableGet function
* @endinternal
*
* @brief   Get the status of PFS timer to priority queue mapping for given
*         scheduler profile.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile Set
*
* @param[out] enablePtr                - (pointer to) status of PFC timer to Priority Queue
*                                      mapping.
*                                      GT_TRUE: PFC timer to Priority Queue map used.
*                                      GT_FALSE: PFC timer to Priority Queue map bypassed.
*                                      1:1 mapping between a timer in PFC frame to an egress queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcTimerMapEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    OUT GT_BOOL                                *enablePtr

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcTimerMapEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, enablePtr));

    rc = internal_cpssPxPortPfcTimerMapEnableGet(devNum, profileSet, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcTimerToQueueMapSet function
* @endinternal
*
* @brief   Sets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7)
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_OUT_OF_RANGE          - on out of traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcTimerToQueueMapSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_U32    pfcTimer,
    IN  GT_U32    tcQueue

)
{
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    if (pfcTimer >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.PFCTimerToPriorityQueueMap[pfcTimer];
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 3, tcQueue);
}

/**
* @internal cpssPxPortPfcTimerToQueueMapSet function
* @endinternal
*
* @brief   Sets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7)
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_OUT_OF_RANGE          - on out of traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcTimerToQueueMapSet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_U32    pfcTimer,
    IN  GT_U32    tcQueue

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcTimerToQueueMapSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcTimer, tcQueue));

    rc = internal_cpssPxPortPfcTimerToQueueMapSet(devNum, pfcTimer, tcQueue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcTimer, tcQueue));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcTimerToQueueMapGet function
* @endinternal
*
* @brief   Gets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7).
*
* @param[out] tcQueuePtr               - (pointer to) traffic class queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortPfcTimerToQueueMapGet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_U32    pfcTimer,
    OUT GT_U32    *tcQueuePtr

)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(tcQueuePtr);
    if (pfcTimer >= 8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.PFCTimerToPriorityQueueMap[pfcTimer];

    rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 3, tcQueuePtr);

    return rc;
}

/**
* @internal cpssPxPortPfcTimerToQueueMapGet function
* @endinternal
*
* @brief   Gets PFC timer to priority queue map.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] pfcTimer                 - PFC timer (0..7).
*
* @param[out] tcQueuePtr               - (pointer to) traffic class queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or PFC timer
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPxPortPfcTimerToQueueMapGet
(
    IN  GT_SW_DEV_NUM     devNum,
    IN  GT_U32    pfcTimer,
    OUT GT_U32    *tcQueuePtr

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcTimerToQueueMapGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pfcTimer, tcQueuePtr));

    rc = internal_cpssPxPortPfcTimerToQueueMapGet(devNum, pfcTimer, tcQueuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pfcTimer, tcQueuePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcShaperToPortRateRatioSet function
* @endinternal
*
* @brief   Sets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] shaperToPortRateRatio    - shaper rate to port speed ratio
*                                      in percentage (0..100).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range shaper rate to port speed ratio
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To bind port to scheduler profile use:
*       cpssPxPortTxBindPortToSchedulerProfileSet.
*
*/
static GT_STATUS internal_cpssPxPortPfcShaperToPortRateRatioSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U32                                   tcQueue,
    IN  GT_U32                                  shaperToPortRateRatio

)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_PFC_PROFILE_SET_CHECK_MAC(profileSet);

    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    if (shaperToPortRateRatio > 100)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (shaperToPortRateRatio == 0)
    {
        value = 0;
    }
    else
    {
        /* PFCFactor = ROUNDUP(shaperToPortRateRatio*128/100) - 1 */
        value = (shaperToPortRateRatio * 128) / 100 -(((shaperToPortRateRatio * 128) % 100 == 0) ? 1 : 0);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.schedulerProfilePriorityQueuePFCFactor[profileSet][tcQueue];
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 7, value);
}

/**
* @internal cpssPxPortPfcShaperToPortRateRatioSet function
* @endinternal
*
* @brief   Sets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] shaperToPortRateRatio    - shaper rate to port speed ratio
*                                      in percentage (0..100).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range shaper rate to port speed ratio
* @retval GT_HW_ERROR              - on hardware error
*
* @note To bind port to scheduler profile use:
*       cpssPxPortTxBindPortToSchedulerProfileSet.
*
*/
GT_STATUS cpssPxPortPfcShaperToPortRateRatioSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U32                                  tcQueue,
    IN  GT_U32                                  shaperToPortRateRatio

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcShaperToPortRateRatioSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, shaperToPortRateRatio));

    rc = internal_cpssPxPortPfcShaperToPortRateRatioSet(devNum, profileSet, tcQueue, shaperToPortRateRatio);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, shaperToPortRateRatio));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcShaperToPortRateRatioGet function
* @endinternal
*
* @brief   Gets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] shaperToPortRateRatioPtr - (pointer to)shaper rate to port speed ratio
*                                      in percentage.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssPxPortPfcShaperToPortRateRatioGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U32                                 tcQueue,
    OUT GT_U32                                 *shaperToPortRateRatioPtr

)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(shaperToPortRateRatioPtr);

    PRV_CPSS_PX_PORT_PFC_PROFILE_SET_CHECK_MAC(profileSet);

    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.schedulerProfilePriorityQueuePFCFactor[profileSet][tcQueue];

    rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 7, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (value == 0)
    {
        *shaperToPortRateRatioPtr = 0;
    }
    else
    {
        /* shaperToPortRateRatio = ROUNDDOWN(PFCFactor*100/128) + 1 */
        *shaperToPortRateRatioPtr = (value * 100) / 128 + 1;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortPfcShaperToPortRateRatioGet function
* @endinternal
*
* @brief   Gets shaper rate to port speed ratio on given scheduler profile
*         and traffic class queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - the Tx Queue scheduler profile.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] shaperToPortRateRatioPtr - (pointer to)shaper rate to port speed ratio
*                                      in percentage.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number ,profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxPortPfcShaperToPortRateRatioGet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  CPSS_PX_PORT_TX_SCHEDULER_PROFILE_SET_ENT  profileSet,
    IN  GT_U32                                  tcQueue,
    OUT GT_U32                                 *shaperToPortRateRatioPtr

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcShaperToPortRateRatioGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, shaperToPortRateRatioPtr));

    rc = internal_cpssPxPortPfcShaperToPortRateRatioGet(devNum, profileSet, tcQueue, shaperToPortRateRatioPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, shaperToPortRateRatioPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxPortPfcCascadeEnableSet function
* @endinternal
*
* @brief   Enable/Disable insertion of DSA tag for PFC frames
*         transmitted on given port.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number, CPU port number.
* @param[in] enable                   - GT_TRUE - PFC packets are transmitted with a DSA tag.
*                                      GT_FALSE - PFC packets are transmitted without DSA tag.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortPfcCascadeEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 value;      /* register value */
    GT_U32 portMacNum; /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    value = BOOL2BIT_MAC(enable);

    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).FCACtrl;

    /* set PFC Cascade Port Enable */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 9, 1, value);
}
/**
* @internal internal_cpssPxPortPfcForwardEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (0..15)
* @param[in] enable                   - GT_TRUE:  forward PFC frames to the ingress pipe,
*                                      GT_FALSE: do not forward PFC frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
static GT_STATUS internal_cpssPxPortPfcForwardEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS   rc;
    GT_U32 regAddr;    /* register address */
    GT_U32 value;      /* register value */
    GT_U32 portMacNum;      /* MAC number */
    PRV_CPSS_PORT_REG_CONFIG_STC   regDataArray[PRV_CPSS_PORT_NOT_APPLICABLE_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    value = BOOL2BIT_MAC(enable);


    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].serialParameters1;

    if (prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);

    regDataArray[PRV_CPSS_PORT_GE_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_GE_E].fieldOffset = 1;


    PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XG_E, &regAddr);
    regDataArray[PRV_CPSS_PORT_XG_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_XG_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_XG_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_XG_E].fieldOffset = 6;


    PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, PRV_CPSS_PORT_XLG_E, &regAddr);
    regDataArray[PRV_CPSS_PORT_XLG_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_XLG_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_XLG_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_XLG_E].fieldOffset = 6;

    /* support for CG port */
    regAddr = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portMacNum).CGMAConvertersFcControl0;
    regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
    regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
    regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
    regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 17; /*<Forward PFC Enable>*/

    rc = prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
    if (rc != GT_OK)
    {
        return rc;
    }


    if (prvCpssPxPortMacConfigurationClear(regDataArray) != GT_OK) CPSS_LOG_ERROR_AND_RETURN_MAC(GT_INIT_ERROR, LOG_ERROR_NO_MSG);


    /* support for CG port */
    /*  pfc response in 100G ports require enabling
        of bit 19 in COMMAND_CONFIG register */
    regAddr = PRV_PX_REG1_UNIT_CG_PORT_MAC_MAC(devNum, portMacNum).CGPORTMACCommandConfig;
    if (regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        regDataArray[PRV_CPSS_PORT_CG_E].regAddr = regAddr;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldData = value;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldLength = 1;
        regDataArray[PRV_CPSS_PORT_CG_E].fieldOffset = 19; /*<PFC Mode>*/
    }
    else
    {
        return GT_OK;
    }

    return prvCpssPxPortMacConfiguration(devNum, portNum, regDataArray);
}

/**
* @internal cpssPxPortPfcForwardEnableSet function
* @endinternal
*
* @brief   Enable/disable forwarding of PFC frames to the ingress
*         pipeline of a specified port.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
* @param[in] enable                   - GT_TRUE:  forward PFC frames to the ingress pipe,
*                                      GT_FALSE: do not forward PFC frames to the ingress pipe.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
GT_STATUS cpssPxPortPfcForwardEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    IN  GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcForwardEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortPfcForwardEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortPfcForwardEnableGet function
* @endinternal
*
* @brief   Get status of PFC frames forwarding
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - current forward PFC frames status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
static GT_STATUS internal_cpssPxPortPfcForwardEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32 regAddr;    /* register address */
    GT_U32 value;      /* register value */
    GT_STATUS rc;      /* return code */
    GT_U32 offset;     /* bit number inside register       */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* type of mac unit of port */
    GT_U32          portMacNum;      /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum, portMacNum);
    if (portMacType == PRV_CPSS_PORT_CG_E) {
        regAddr = PRV_PX_REG1_UNIT_CG_CONVERTERS_MAC(devNum, portMacNum).CGMAConvertersFcControl0;
        offset  = 17; /*<Forward PFC Enable>*/
    }
    else
    {
        PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, portMacType, &regAddr);
        if (portMacType >= PRV_CPSS_PORT_XG_E)
        {
            offset = 6;
        }
        else
        {
             regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.perPortRegs[portNum].serialParameters1;
             offset = 1;
        }
    }
    if ((rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, offset, 1, &value)) != GT_OK)
        return rc;

    *enablePtr = BIT2BOOL_MAC(value);
    return GT_OK;
}

/**
* @internal cpssPxPortPfcForwardEnableGet function
* @endinternal
*
* @brief   Get status of PFC frames forwarding
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number (CPU port not supported)
*
* @param[out] enablePtr                - current forward PFC frames status
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or profile set
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note A packet is considered as a PFC frame if all of the following are true:
*       - Packet’s Length/EtherType field is 88-08
*       - Packet’s OpCode field is 01-01
*       - Packet’s MAC DA is 01-80-C2-00-00-01 or the port’s configured MAC Address
*
*/
GT_STATUS cpssPxPortPfcForwardEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcForwardEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortPfcForwardEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcPacketClassificationEnableSet function
* @endinternal
*
* @brief   Enable/Disable the ingress control pipe to parse and terminate received
*         flow control frames (PRC/802.3x)
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.(APPLICABLE RANGES: 0..31)
* @param[in] udbpIndex                - user defined byte pair index, where opcode will be set (APPLICABLE RANGES: 0..3)
* @param[in] enable                   -  or disable PFC packet classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If the configuration is disabled, flow control frames are processed by
*       the control pipe as regular data frames (that is they are subject to
*       learning, forwarding, filtering and mirroring) but they are not
*       terminated and the PFC timers are not extracted.
*
*/
static GT_STATUS internal_cpssPxPortPfcPacketClassificationEnableSet
(

    IN GT_SW_DEV_NUM                devNum,
    IN CPSS_PX_PACKET_TYPE          packetType,
    IN GT_U32                       udbpIndex,
    IN GT_BOOL                      enable
)
{

    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;

    GT_U32      regAddr;
    GT_U32      data,i;
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (udbpIndex > 3)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    /* Clear keyData & keyMask needed to configure packet type */
    cpssOsMemSet(&keyData, 0, sizeof(keyData));
    cpssOsMemSet(&keyMask, 0, sizeof(keyMask));
    cpssOsMemSet(&portKey, 0, sizeof(portKey));
    if (enable)
    {
        keyData.macDa.arEther[0] = 0x01;
        keyData.macDa.arEther[1] = 0x80;
        keyData.macDa.arEther[2] = 0xC2;
        keyData.macDa.arEther[3] = 0x0;
        keyData.macDa.arEther[4] = 0x0;
        keyData.macDa.arEther[5] = 0x01;
        keyData.etherType = 0x8808;

        keyData.udbPairsArr[udbpIndex].udb[0] = 0x01;
        keyData.udbPairsArr[udbpIndex].udb[1] = 0x01;

        keyMask.udbPairsArr[udbpIndex].udb[0] = 0xFF;
        keyMask.udbPairsArr[udbpIndex].udb[1] = 0xFF;


        keyMask.etherType = 0xFFFF;
        for (i = 0; i < 6; i++)
        {
            keyMask.macDa.arEther[i] = 0xFF;
        }

        rc = cpssPxIngressPacketTypeKeyEntrySet(devNum, packetType, &keyData, &keyMask);
        if (rc != GT_OK)
        {
            return rc;
        }


        for (i = 0; i < PRV_CPSS_PX_GOP_PORTS_NUM_CNS; i++)
        {
            rc = cpssPxIngressPortPacketTypeKeyGet(devNum, i, &portKey);

            if (rc != GT_OK)
                return rc;

            portKey.portUdbPairArr[udbpIndex].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;
            portKey.portUdbPairArr[udbpIndex].udbByteOffset = 2;

            rc = cpssPxIngressPortPacketTypeKeySet(devNum, i, &portKey);

            if (rc != GT_OK)
                return rc;

        }
    }
    rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, enable);

    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).config.PFCConfig;
    if (enable)
    {
        data = packetType;
    }
    else
    {
        data = 0x1f; /*reset value*/
    }
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 5, data);
}

/**
* @internal cpssPxPortPfcPacketClassificationEnableSet function
* @endinternal
*
* @brief   Enable/Disable the ingress control pipe to parse and terminate received
*         flow control frames (PRC/802.3x)
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] packetType               - index of the packet type key in the table.(APPLICABLE RANGES: 0..31)
* @param[in] udbpIndex                - user defined byte pair index, where opcode will be set (APPLICABLE RANGES: 0..3)
* @param[in] enable                   -  or disable PFC packet classification
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If the configuration is disabled, flow control frames are processed by
*       the control pipe as regular data frames (that is they are subject to
*       learning, forwarding, filtering and mirroring) but they are not
*       terminated and the PFC timers are not extracted.
*
*/
GT_STATUS cpssPxPortPfcPacketClassificationEnableSet
(

    IN GT_SW_DEV_NUM                devNum,
    IN CPSS_PX_PACKET_TYPE          packetType,
    IN GT_U32                       udbpIndex,
    IN GT_BOOL                      enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcPacketClassificationEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetType, udbpIndex, enable));

    rc = internal_cpssPxPortPfcPacketClassificationEnableSet(devNum, packetType, udbpIndex, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetType, udbpIndex, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcPacketTypeGet function
* @endinternal
*
* @brief   Get packet type
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_STATUS internal_cpssPxPortPfcPacketTypeGet
(
    IN  GT_SW_DEV_NUM            devNum,
    OUT CPSS_PX_PACKET_TYPE      *packetTypePtr
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;
    /* check parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(packetTypePtr);

    regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).config.PFCConfig;

    /* packetType = index of the packet type key in the table */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 5, packetTypePtr);

    return rc;
}

/**
* @internal cpssPxPortPfcPacketTypeGet function
* @endinternal
*
* @brief   Get packet type
*
* @param[in] devNum                   - device number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS cpssPxPortPfcPacketTypeGet
(
    IN  GT_SW_DEV_NUM            devNum,
    OUT CPSS_PX_PACKET_TYPE      *packetTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcPacketTypeGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetTypePtr));

    rc = internal_cpssPxPortPfcPacketTypeGet(devNum, packetTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetTypePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcQueueCounterGet function
* @endinternal
*
* @brief   Get the number buffers/packets per proirity queue.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue on the port
*
* @param[out] cntPtr                   - (pointer to) number buffers/packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, tcQueue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Buffer or packet counting mode is set by cpssPxPortPfcCountingModeSet.
*
*/
static GT_STATUS internal_cpssPxPortPfcQueueCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32   tcQueue,
    OUT GT_U32  *cntPtr
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cntPtr);

    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);

    regAddr = PRV_PX_REG1_UNIT_TXQ_PFC_MAC(devNum).PFCGroupOfPortsTCCntr[0][tcQueue];
    rc = prvCpssPortGroupsCounterSummary(CAST_SW_DEVNUM(devNum), regAddr, 0, 29, cntPtr, NULL);

    return rc;
}

/**
* @internal cpssPxPortPfcQueueCounterGet function
* @endinternal
*
* @brief   Get the number buffers/packets per proirity queue.
*
* @param[in] devNum                   - device number
* @param[in] tcQueue                  - traffic class queue on the port
*
* @param[out] cntPtr                   - (pointer to) number buffers/packets
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong devNum, tcQueue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Buffer or packet counting mode is set by cpssPxPortPfcCountingModeSet.
*
*/
GT_STATUS cpssPxPortPfcQueueCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32   tcQueue,
    OUT GT_U32  *cntPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcQueueCounterGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tcQueue, cntPtr));

    rc = internal_cpssPxPortPfcQueueCounterGet(devNum, tcQueue, cntPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tcQueue, cntPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortPfcXonMessageFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE:  Enable  PFC XON Message Filter.
*                                      GT_FALSE: Disable PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Enable this feature in a system with periodic flow control XON messages.
*
*/
GT_STATUS internal_cpssPxPortPfcXonMessageFilterEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_BOOL  enable
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 value;     /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    value = BOOL2BIT_MAC(enable);
    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.flowCtrlResponseConfig;

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 1, value);
}

/**
* @internal cpssPxPortPfcXonMessageFilterEnableSet function
* @endinternal
*
* @brief   Enable/Disable PFC (Priority Flow Control) filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE:  Enable  PFC XON Message Filter.
*                                      GT_FALSE: Disable PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or enable option
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Enable this feature in a system with periodic flow control XON messages.
*
*/
GT_STATUS cpssPxPortPfcXonMessageFilterEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN GT_BOOL  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcXonMessageFilterEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPortPfcXonMessageFilterEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortPfcXonMessageFilterEnableGet function
* @endinternal
*
* @brief   Gets current status of the filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) Enable  PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPxPortPfcXonMessageFilterEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 value;     /* register value */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_DQ_MAC(devNum).flowCtrlResponse.flowCtrlResponseConfig;

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 1, &value);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssPxPortPfcXonMessageFilterEnableGet function
* @endinternal
*
* @brief   Gets current status of the filtering of
*         continuous flow control XON messages to priority shaper.
*
* @note   APPLICABLE DEVICES:      Pipe
* @note   NOT APPLICABLE DEVICES:  None
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) Enable  PFC XON Message Filter.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortPfcXonMessageFilterEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPfcXonMessageFilterEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPortPfcXonMessageFilterEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


