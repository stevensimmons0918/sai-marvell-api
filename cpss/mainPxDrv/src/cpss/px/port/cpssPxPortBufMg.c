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
* @file cpssPxPortBufMg.c
*
* @brief CPSS implementation for Buffer Management facility.
*
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/port/private/prvCpssPxPortLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortBufMg.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/config/cpssPxCfgInit.h>

#define PRV_CPSS_PX_RX_FC_PROFILE_CHECK_MAC(_devNum, _profileSet)            \
    if ((GT_U32)(_profileSet) > (GT_U32)CPSS_PORT_RX_FC_PROFILE_8_E)         \
        {CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);}                                                        \

/**
* @internal internal_cpssPxPortBufMgGlobalXonLimitSet function
* @endinternal
*
* @brief   Sets the Global Xon limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xon limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xonLimit), all ports that support 802.3x Flow Control send an Xon
*         frame.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] xonLimit                 - X-ON limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong xonLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Global Xon limit must be not be greater than Global Xoff Limit.
*
*/
static GT_STATUS internal_cpssPxPortBufMgGlobalXonLimitSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           xonLimit
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      fieldOffset; /* field offset     */
    GT_U32      fieldLength; /* field length     */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (xonLimit >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->BM.BMGlobalConfigs.BMPauseConfig;
    fieldOffset = 0;
    fieldLength = 16;

    return prvCpssHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, xonLimit);
}

/**
* @internal cpssPxPortBufMgGlobalXonLimitSet function
* @endinternal
*
* @brief   Sets the Global Xon limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xon limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xonLimit), all ports that support 802.3x Flow Control send an Xon
*         frame.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] xonLimit                 - X-ON limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong xonLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The Global Xon limit must be not be greater than Global Xoff Limit.
*
*/
GT_STATUS cpssPxPortBufMgGlobalXonLimitSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           xonLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgGlobalXonLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xonLimit));

    rc = internal_cpssPxPortBufMgGlobalXonLimitSet(devNum, xonLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xonLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgGlobalXonLimitGet function
* @endinternal
*
* @brief   Gets the Global Xon limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgGlobalXonLimitGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *xonLimitPtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      fieldOffset; /* field offset     */
    GT_U32      fieldLength; /* field length     */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(xonLimitPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->BM.BMGlobalConfigs.BMPauseConfig;
    fieldOffset = 0;
    fieldLength = 16;

    return prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, xonLimitPtr);
}

/**
* @internal cpssPxPortBufMgGlobalXonLimitGet function
* @endinternal
*
* @brief   Gets the Global Xon limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgGlobalXonLimitGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *xonLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgGlobalXonLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xonLimitPtr));

    rc = internal_cpssPxPortBufMgGlobalXonLimitGet(devNum, xonLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xonLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgGlobalXoffLimitSet function
* @endinternal
*
* @brief   Sets the Global Xoff limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xoff limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xoffLimit), all ports that support 802.3x Flow Control send a pause
*         frame.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] xoffLimit                - X-OFF limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong xoffLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgGlobalXoffLimitSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           xoffLimit
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      fieldOffset; /* field offset     */
    GT_U32      fieldLength; /* field length     */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (xoffLimit >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->BM.BMGlobalConfigs.BMPauseConfig;
    fieldOffset = 16;
    fieldLength = 16;

    return prvCpssHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, xoffLimit);
}

/**
* @internal cpssPxPortBufMgGlobalXoffLimitSet function
* @endinternal
*
* @brief   Sets the Global Xoff limit value. Enables global control over the
*         number of buffers allocated for all ports by Global Xoff limit setting.
*         When the total number of buffers allocated reaches this threshold
*         (xoffLimit), all ports that support 802.3x Flow Control send a pause
*         frame.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] xoffLimit                - X-OFF limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_OUT_OF_RANGE          - on wrong xoffLimit.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgGlobalXoffLimitSet
(
    IN GT_SW_DEV_NUM    devNum,
    IN GT_U32           xoffLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgGlobalXoffLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xoffLimit));

    rc = internal_cpssPxPortBufMgGlobalXoffLimitSet(devNum, xoffLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xoffLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgGlobalXoffLimitGet function
* @endinternal
*
* @brief   Gets the Global Xoff limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgGlobalXoffLimitGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *xoffLimitPtr
)
{
    GT_U32      regAddr;     /* register address */
    GT_U32      fieldOffset; /* field offset     */
    GT_U32      fieldLength; /* field length     */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(xoffLimitPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->BM.BMGlobalConfigs.BMPauseConfig;
    fieldOffset = 16;
    fieldLength = 16;

    return prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, xoffLimitPtr);
}

/**
* @internal cpssPxPortBufMgGlobalXoffLimitGet function
* @endinternal
*
* @brief   Gets the Global Xoff limit value.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_HW_ERROR              - on hardware error.
* @retval GT_BAD_PARAM             - on wrong devNum.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgGlobalXoffLimitGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *xoffLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgGlobalXoffLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, xoffLimitPtr));

    rc = internal_cpssPxPortBufMgGlobalXoffLimitGet(devNum, xoffLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, xoffLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgRxProfileSet function
* @endinternal
*
* @brief   Bind a port to a flow control profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or profileSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgRxProfileSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet
)
{
    GT_U32      regAddr;      /* register address  */
    GT_U32      portRxDmaNum; /* port RxDMA number */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);
    PRV_CPSS_PX_RX_FC_PROFILE_CHECK_MAC(devNum, profileSet);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.singleChannelDMAConfigs.SCDMAConfig0[portRxDmaNum];
    fieldOffset = 9;
    fieldLength = 3;

    return prvCpssHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, (GT_U32)profileSet);
}

/**
* @internal cpssPxPortBufMgRxProfileSet function
* @endinternal
*
* @brief   Bind a port to a flow control profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device or profileSet
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgRxProfileSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN GT_PHYSICAL_PORT_NUM             portNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgRxProfileSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileSet));

    rc = internal_cpssPxPortBufMgRxProfileSet(devNum, portNum, profileSet);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileSet));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgRxProfileGet function
* @endinternal
*
* @brief   Get the port's flow control profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - (pointer to) the Profile Set in which the Flow Control
*                                      Parameters are associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPxPortBufMgRxProfileGet
(
    IN  GT_SW_DEV_NUM                    devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    OUT CPSS_PORT_RX_FC_PROFILE_SET_ENT  *profileSetPtr
)
{
    GT_STATUS   rc;           /* return code       */
    GT_U32      regAddr;      /* register address  */
    GT_U32      portRxDmaNum; /* port RxDMA number */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */
    GT_U32      hwData;       /* HW data           */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);
    CPSS_NULL_PTR_CHECK_MAC(profileSetPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.singleChannelDMAConfigs.SCDMAConfig0[portRxDmaNum];
    fieldOffset = 9;
    fieldLength = 3;

    hwData = 0;
    rc = prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, &hwData);

    *profileSetPtr = (CPSS_PORT_RX_FC_PROFILE_SET_ENT)hwData;
    return rc;
}

/**
* @internal cpssPxPortBufMgRxProfileGet function
* @endinternal
*
* @brief   Get the port's flow control profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @param[out] profileSetPtr            - (pointer to) the Profile Set in which the Flow Control
*                                      Parameters are associated.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgRxProfileGet
(
    IN  GT_SW_DEV_NUM                    devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    OUT CPSS_PORT_RX_FC_PROFILE_SET_ENT  *profileSetPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgRxProfileGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, profileSetPtr));

    rc = internal_cpssPxPortBufMgRxProfileGet(devNum, portNum, profileSetPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, profileSetPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgProfileXonLimitSet function
* @endinternal
*
* @brief   Sets X-ON port limit. When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xonLimit                 - X-ON limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong xonLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgProfileXonLimitSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xonLimit
)
{
    GT_U32      regAddr;      /* register address  */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_RX_FC_PROFILE_CHECK_MAC(devNum, profileSet);
    if (xonLimit >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.globalRxDMAConfigs.buffersLimitProfiles.
            portsBuffersLimitProfileXONXOFFConfig[profileSet];
    fieldOffset = 0;
    fieldLength = 16;

    return prvCpssHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, xonLimit);
}

/**
* @internal cpssPxPortBufMgProfileXonLimitSet function
* @endinternal
*
* @brief   Sets X-ON port limit. When Flow Control is enabled, X-ON packet sent
*         when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xonLimit                 - X-ON limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong xonLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileXonLimitSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xonLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgProfileXonLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, xonLimit));

    rc = internal_cpssPxPortBufMgProfileXonLimitSet(devNum, profileSet, xonLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, xonLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgProfileXonLimitGet function
* @endinternal
*
* @brief   Gets the X-ON port limit. When Flow Control is enabled, X-ON packet
*         sent when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters of
*                                      are associated.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgProfileXonLimitGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xonLimitPtr
)
{
    GT_U32      regAddr;      /* register address  */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_RX_FC_PROFILE_CHECK_MAC(devNum, profileSet);
    CPSS_NULL_PTR_CHECK_MAC(xonLimitPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.globalRxDMAConfigs.buffersLimitProfiles.
            portsBuffersLimitProfileXONXOFFConfig[profileSet];
    fieldOffset = 0;
    fieldLength = 16;

    return prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, xonLimitPtr);
}

/**
* @internal cpssPxPortBufMgProfileXonLimitGet function
* @endinternal
*
* @brief   Gets the X-ON port limit. When Flow Control is enabled, X-ON packet
*         sent when the number of buffers for this port is less than xonLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters of
*                                      are associated.
*
* @param[out] xonLimitPtr              - (pointer to) the X-ON limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileXonLimitGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xonLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgProfileXonLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, xonLimitPtr));

    rc = internal_cpssPxPortBufMgProfileXonLimitGet(devNum, profileSet, xonLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, xonLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgProfileXoffLimitSet function
* @endinternal
*
* @brief   Sets X-OFF port limit. When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xoffLimit                - X-OFF limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong xoffLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgProfileXoffLimitSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xoffLimit
)
{
    GT_U32      regAddr;      /* register address  */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_RX_FC_PROFILE_CHECK_MAC(devNum, profileSet);
    if (xoffLimit >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.globalRxDMAConfigs.buffersLimitProfiles.
            portsBuffersLimitProfileXONXOFFConfig[profileSet];
    fieldOffset = 16;
    fieldLength = 16;

    return prvCpssHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, xoffLimit);
}

/**
* @internal cpssPxPortBufMgProfileXoffLimitSet function
* @endinternal
*
* @brief   Sets X-OFF port limit. When Flow Control is enabled, X-OFF packet sent
*         when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] xoffLimit                - X-OFF limit in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong xoffLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileXoffLimitSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           xoffLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgProfileXoffLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, xoffLimit));

    rc = internal_cpssPxPortBufMgProfileXoffLimitSet(devNum, profileSet, xoffLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, xoffLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgProfileXoffLimitGet function
* @endinternal
*
* @brief   Gets the X-OFF port limit. When Flow Control is enabled, X-OFF packet
*         sent when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgProfileXoffLimitGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xoffLimitPtr
)
{
    GT_U32      regAddr;      /* register address  */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_RX_FC_PROFILE_CHECK_MAC(devNum, profileSet);
    CPSS_NULL_PTR_CHECK_MAC(xoffLimitPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.globalRxDMAConfigs.buffersLimitProfiles.
            portsBuffersLimitProfileXONXOFFConfig[profileSet];
    fieldOffset = 16;
    fieldLength = 16;

    return prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, xoffLimitPtr);
}

/**
* @internal cpssPxPortBufMgProfileXoffLimitGet function
* @endinternal
*
* @brief   Gets the X-OFF port limit. When Flow Control is enabled, X-OFF packet
*         sent when the number of buffers for this port is less than xoffLimit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] xoffLimitPtr             - (pointer to) the X-OFF limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileXoffLimitGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *xoffLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgProfileXoffLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, xoffLimitPtr));

    rc = internal_cpssPxPortBufMgProfileXoffLimitGet(devNum, profileSet, xoffLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, xoffLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgProfileRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] rxBufLimit               - buffer limit threshold in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong rxBufLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgProfileRxBufLimitSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           rxBufLimit
)
{
    GT_U32      regAddr;      /* register address  */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_RX_FC_PROFILE_CHECK_MAC(devNum, profileSet);
    if (rxBufLimit >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.globalRxDMAConfigs.buffersLimitProfiles.
            portsBuffersLimitProfileRxlimitConfig[profileSet];
    fieldOffset = 0;
    fieldLength = 16;

    return prvCpssHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, rxBufLimit);
}

/**
* @internal cpssPxPortBufMgProfileRxBufLimitSet function
* @endinternal
*
* @brief   Sets receive buffer limit threshold for the specified profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
* @param[in] rxBufLimit               - buffer limit threshold in resolution of 1 buffer.
*                                      (APPLICABLE RANGES: 0..0xFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval OUT_OF_RANGE             - on wrong rxBufLimit
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileRxBufLimitSet
(
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PORT_RX_FC_PROFILE_SET_ENT  profileSet,
    IN GT_U32                           rxBufLimit
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgProfileRxBufLimitSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, rxBufLimit));

    rc = internal_cpssPxPortBufMgProfileRxBufLimitSet(devNum, profileSet, rxBufLimit);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, rxBufLimit));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgProfileRxBufLimitGet function
* @endinternal
*
* @brief   Gets the receive buffer limit threshold for the specified profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] rxBufLimitPtr            - (pointer to) the rx buffer limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgProfileRxBufLimitGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *rxBufLimitPtr
)
{
    GT_U32      regAddr;      /* register address  */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_RX_FC_PROFILE_CHECK_MAC(devNum, profileSet);
    CPSS_NULL_PTR_CHECK_MAC(rxBufLimitPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.globalRxDMAConfigs.buffersLimitProfiles.
            portsBuffersLimitProfileRxlimitConfig[profileSet];
    fieldOffset = 0;
    fieldLength = 16;

    return prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, rxBufLimitPtr);
}

/**
* @internal cpssPxPortBufMgProfileRxBufLimitGet function
* @endinternal
*
* @brief   Gets the receive buffer limit threshold for the specified profile.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] profileSet               - the Profile Set in which the Flow Control Parameters are
*                                      associated.
*
* @param[out] rxBufLimitPtr            - (pointer to) the rx buffer limit value in resolution
*                                      of 1 buffer.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong profileSet or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgProfileRxBufLimitGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PORT_RX_FC_PROFILE_SET_ENT profileSet,
    OUT GT_U32                          *rxBufLimitPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgProfileRxBufLimitGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, rxBufLimitPtr));

    rc = internal_cpssPxPortBufMgProfileRxBufLimitGet(devNum, profileSet, rxBufLimitPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, rxBufLimitPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgGlobalRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgGlobalRxBufNumberGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *numOfBuffersPtr
)
{
    GT_U32      regAddr;      /* register address  */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfBuffersPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->BM.BMCntrs.BMGlobalBufferCntr;
    fieldOffset = 0;
    fieldLength = 16;

    return prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, numOfBuffersPtr);
}

/**
* @internal cpssPxPortBufMgGlobalRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgGlobalRxBufNumberGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *numOfBuffersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgGlobalRxBufNumberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfBuffersPtr));

    rc = internal_cpssPxPortBufMgGlobalRxBufNumberGet(devNum, numOfBuffersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfBuffersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated for the specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgRxBufNumberGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *numOfBuffersPtr
)
{
    GT_U32      regAddr;      /* register address  */
    GT_U32      portRxDmaNum; /* port RxDMA number */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_RXDMA_NUM_GET_MAC(devNum, portNum, portRxDmaNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfBuffersPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.scdmaDebug.portBuffAllocCnt[portRxDmaNum];
    fieldOffset = 0;
    fieldLength = 16;

    return prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, numOfBuffersPtr);
}

/**
* @internal cpssPxPortBufMgRxBufNumberGet function
* @endinternal
*
* @brief   Gets total number of buffers currently allocated for the specified port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical or CPU port number
*
* @param[out] numOfBuffersPtr          - (pointer to) number of buffers
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgRxBufNumberGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *numOfBuffersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgRxBufNumberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, numOfBuffersPtr));

    rc = internal_cpssPxPortBufMgRxBufNumberGet(devNum, portNum, numOfBuffersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, numOfBuffersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgGlobalPacketNumberGet function
* @endinternal
*
* @brief   Gets total number of unique packets currently in the system.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numOfPacketsPtr          - (pointer to) number of packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgGlobalPacketNumberGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *numOfPacketsPtr
)
{
    GT_U32      regAddr;      /* register address  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfPacketsPtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
            rxDMA.debug.countersAndCountersStatuses.currentNumOfPacketsStoredInMppm;

    return prvCpssHwPpReadRegister(
        CAST_SW_DEVNUM(devNum), regAddr, numOfPacketsPtr);
}

/**
* @internal cpssPxPortBufMgGlobalPacketNumberGet function
* @endinternal
*
* @brief   Gets total number of unique packets currently in the system.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] numOfPacketsPtr          - (pointer to) number of packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgGlobalPacketNumberGet
(
    IN  GT_SW_DEV_NUM    devNum,
    OUT GT_U32           *numOfPacketsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgGlobalPacketNumberGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, numOfPacketsPtr));

    rc = internal_cpssPxPortBufMgGlobalPacketNumberGet(devNum, numOfPacketsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, numOfPacketsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgRxMcCntrGet function
* @endinternal
*
* @brief   Gets multicast counter of a certain buffer.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrIdx                  - counter index
*                                      (APPLICABLE RANGES 0..16383)
*
* @param[out] mcCntrPtr                - (pointer to) multicast counter of a certain buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or cntrIdx
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgRxMcCntrGet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           cntrIdx,
    OUT GT_U32           *mcCntrPtr
)
{
    GT_STATUS rc;         /* return code             */
    GT_U32    numEntries; /* amount of table entries */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(mcCntrPtr);
    rc = cpssPxCfgTableNumEntriesGet(
        devNum, CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E, &numEntries);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (cntrIdx >= numEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssPxReadTableEntry(
        devNum, CPSS_PX_TABLE_BMA_MULTICAST_COUNTERS_E,
        cntrIdx, mcCntrPtr);
}

/**
* @internal cpssPxPortBufMgRxMcCntrGet function
* @endinternal
*
* @brief   Gets multicast counter of a certain buffer.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cntrIdx                  - counter index
*                                      (APPLICABLE RANGES 0..16383)
*
* @param[out] mcCntrPtr                - (pointer to) multicast counter of a certain buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device or cntrIdx
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgRxMcCntrGet
(
    IN  GT_SW_DEV_NUM    devNum,
    IN  GT_U32           cntrIdx,
    OUT GT_U32           *mcCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgRxMcCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cntrIdx, mcCntrPtr));

    rc = internal_cpssPxPortBufMgRxMcCntrGet(devNum, cntrIdx, mcCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cntrIdx, mcCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgTxDmaBurstLimitEnableSet function
* @endinternal
*
* @brief   For a given port Enable/Disable TXDMA burst limit thresholds use.
*         When enabled: instruct the TxDMA to throttle the Transmit Queue Scheduler
*         as a function of the TxDMA FIFOs fill level measured in bytes and descriptor.
*         When disabled: the TxDMA throttles the Transmit Queue Scheduler
*         based on FIFOs fill level measured in descriptors only.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] enable                   - GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgTxDmaBurstLimitEnableSet
(
    IN GT_SW_DEV_NUM          devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL                enable
)
{
    GT_U32      regAddr;      /* register address  */
    GT_U32      portTxdmaNum; /* TX DMA number     */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */
    GT_U32      hwValue;      /* Hardware value    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXDMA_NUM_GET_MAC(devNum, portNum, portTxdmaNum);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
        txDMA.txDMAPerSCDMAConfigs.SCDMAConfigs[portTxdmaNum];
    fieldOffset = 30;
    fieldLength = 1;
    hwValue     = BOOL2BIT_MAC(enable);

    return prvCpssHwPpSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, hwValue);
}

/**
* @internal cpssPxPortBufMgTxDmaBurstLimitEnableSet function
* @endinternal
*
* @brief   For a given port Enable/Disable TXDMA burst limit thresholds use.
*         When enabled: instruct the TxDMA to throttle the Transmit Queue Scheduler
*         as a function of the TxDMA FIFOs fill level measured in bytes and descriptor.
*         When disabled: the TxDMA throttles the Transmit Queue Scheduler
*         based on FIFOs fill level measured in descriptors only.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] enable                   - GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgTxDmaBurstLimitEnableSet
(
    IN GT_SW_DEV_NUM          devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_BOOL                enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgTxDmaBurstLimitEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortBufMgTxDmaBurstLimitEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgTxDmaBurstLimitEnableGet function
* @endinternal
*
* @brief   Gets the current status for a given port of TXDMA burst limit thresholds use.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgTxDmaBurstLimitEnableGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL                *enablePtr
)
{
    GT_STATUS   rc;           /* return code       */
    GT_U32      regAddr;      /* register address  */
    GT_U32      portTxdmaNum; /* TX DMA number     */
    GT_U32      fieldOffset;  /* field offset      */
    GT_U32      fieldLength;  /* field length      */
    GT_U32      hwValue;      /* Hardware value    */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXDMA_NUM_GET_MAC(devNum, portNum, portTxdmaNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
        txDMA.txDMAPerSCDMAConfigs.SCDMAConfigs[portTxdmaNum];
    fieldOffset = 30;
    fieldLength = 1;

    rc = prvCpssHwPpGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, fieldLength, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr   = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal cpssPxPortBufMgTxDmaBurstLimitEnableGet function
* @endinternal
*
* @brief   Gets the current status for a given port of TXDMA burst limit thresholds use.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: TXDMA burst limit thresholds are used.
*                                      GT_FALSE: TXDMA burst limit thresholds are not used.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgTxDmaBurstLimitEnableGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_BOOL                *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgTxDmaBurstLimitEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortBufMgTxDmaBurstLimitEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgTxDmaBurstLimitThresholdsSet function
* @endinternal
*
* @brief   Sets the TXDMA burst limit thresholds for a given port.
*         The TxDMA supports three throttling levels: Normal, Slow and OFF.
*         The levels are controlled using two thresholds (almostFullThreshold and
*         fullThreshold) measuring the FIFOs fill level.
*         Normal - the Transmit Queue scheduler is not throttled.
*         Slow - the Transmit Queue scheduler is throttled.
*         OFF - the Transmit Queue scheduler is paused.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] almostFullThreshold      - almost full threshold in Bytes (0..(0x80  0xFFFF)).
* @param[in] fullThreshold            - full threshold in Bytes (0..(0x80  0xFFFF)).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of almostFullThreshold and fullThreshold fields in hardware
*       is 128 Bytes.
*       The value is rounded down before write to HW.
*
*/
static GT_STATUS internal_cpssPxPortBufMgTxDmaBurstLimitThresholdsSet
(
    IN GT_SW_DEV_NUM          devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32                 almostFullThreshold,
    IN GT_U32                 fullThreshold
)
{
    GT_U32      regAddr;          /* register address    */
    GT_U32      portTxdmaNum;     /* TX DMA number       */
    GT_U32      resolution;       /* resolution in bytes */
    GT_U32      maxSwValue;       /* maximal SW value    */
    GT_U32      hwAlmostFullThr;  /* HW value for param0 */
    GT_U32      hwFullThr;        /* HW value for param1 */
    GT_U32      hwValue;          /* Hardware value      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXDMA_NUM_GET_MAC(devNum, portNum, portTxdmaNum);

    resolution = 128;
    maxSwValue = (resolution * (BIT_16 - 1));
    if ((almostFullThreshold > maxSwValue) || (fullThreshold > maxSwValue))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
        txDMA.txDMAPerSCDMAConfigs.burstLimiterSCDMA[portTxdmaNum];
    hwAlmostFullThr = (almostFullThreshold / resolution);
    hwFullThr       = (fullThreshold / resolution);
    hwValue         = ((hwAlmostFullThr << 16) | hwFullThr);

    return prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, hwValue);
}

/**
* @internal cpssPxPortBufMgTxDmaBurstLimitThresholdsSet function
* @endinternal
*
* @brief   Sets the TXDMA burst limit thresholds for a given port.
*         The TxDMA supports three throttling levels: Normal, Slow and OFF.
*         The levels are controlled using two thresholds (almostFullThreshold and
*         fullThreshold) measuring the FIFOs fill level.
*         Normal - the Transmit Queue scheduler is not throttled.
*         Slow - the Transmit Queue scheduler is throttled.
*         OFF - the Transmit Queue scheduler is paused.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
* @param[in] almostFullThreshold      - almost full threshold in Bytes (0..(0x80  0xFFFF)).
* @param[in] fullThreshold            - full threshold in Bytes (0..(0x80  0xFFFF)).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Resolution of almostFullThreshold and fullThreshold fields in hardware
*       is 128 Bytes.
*       The value is rounded down before write to HW.
*
*/
GT_STATUS cpssPxPortBufMgTxDmaBurstLimitThresholdsSet
(
    IN GT_SW_DEV_NUM          devNum,
    IN GT_PHYSICAL_PORT_NUM   portNum,
    IN GT_U32                 almostFullThreshold,
    IN GT_U32                 fullThreshold
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgTxDmaBurstLimitThresholdsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, almostFullThreshold, fullThreshold));

    rc = internal_cpssPxPortBufMgTxDmaBurstLimitThresholdsSet(
        devNum, portNum, almostFullThreshold, fullThreshold);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, almostFullThreshold, fullThreshold));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortBufMgTxDmaBurstLimitThresholdsGet function
* @endinternal
*
* @brief   Gets the TXDMA burst limit thresholds for a given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] almostFullThresholdPtr   - (pointer to) almost full threshold in Bytes.
* @param[out] fullThresholdPtr         - (pointer to) full threshold in Bytes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortBufMgTxDmaBurstLimitThresholdsGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *almostFullThresholdPtr,
    OUT GT_U32                 *fullThresholdPtr
)
{
    GT_STATUS   rc;           /* return code         */
    GT_U32      regAddr;      /* register address    */
    GT_U32      portTxdmaNum; /* TX DMA number       */
    GT_U32      resolution;   /* resolution in bytes */
    GT_U32      hwValue;      /* Hardware value      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXDMA_NUM_GET_MAC(devNum, portNum, portTxdmaNum);
    CPSS_NULL_PTR_CHECK_MAC(almostFullThresholdPtr);
    CPSS_NULL_PTR_CHECK_MAC(fullThresholdPtr);

    resolution = 128;

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->
        txDMA.txDMAPerSCDMAConfigs.burstLimiterSCDMA[portTxdmaNum];

    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *almostFullThresholdPtr = (((hwValue >> 16) & 0xFFFF) * resolution);
    *fullThresholdPtr       = ((hwValue & 0xFFFF) * resolution);
    return GT_OK;
}

/**
* @internal cpssPxPortBufMgTxDmaBurstLimitThresholdsGet function
* @endinternal
*
* @brief   Gets the TXDMA burst limit thresholds for a given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number (CPU port supported as well).
*
* @param[out] almostFullThresholdPtr   - (pointer to) almost full threshold in Bytes.
* @param[out] fullThresholdPtr         - (pointer to) full threshold in Bytes.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortBufMgTxDmaBurstLimitThresholdsGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  GT_PHYSICAL_PORT_NUM   portNum,
    OUT GT_U32                 *almostFullThresholdPtr,
    OUT GT_U32                 *fullThresholdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortBufMgTxDmaBurstLimitThresholdsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, almostFullThresholdPtr, fullThresholdPtr));

    rc = internal_cpssPxPortBufMgTxDmaBurstLimitThresholdsGet(
        devNum, portNum, almostFullThresholdPtr, fullThresholdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, almostFullThresholdPtr, fullThresholdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}






