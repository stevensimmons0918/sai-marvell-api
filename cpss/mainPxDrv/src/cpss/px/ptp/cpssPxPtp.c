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
* @file cpssPxPtp.c
*
* @brief CPSS Pipe Precision Time Protocol function implementations.
*
* @version   1
********************************************************************************
*/

/*#define CPSS_LOG_IN_MODULE_ENABLE*/
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/ptp/private/prvCpssPxPtpLog.h>
#include <cpss/px/ptp/cpssPxPtp.h>

/* TAI Control register address */
#define PX_TAI_CONTROL_REGISTER_ADDRESS                         0x00018234

#define PRV_CPSS_PX_PTP_ANCHOR_TO_HW_MAC(anchor,hwValue)                  \
    switch(anchor)                                                        \
    {                                                                     \
        case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E:                        \
            hwValue = 0;                                                  \
            break;                                                        \
        case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E:                        \
            hwValue = 1;                                                  \
            break;                                                        \
        case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E:                        \
            hwValue = 2;                                                  \
            break;                                                        \
        case CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E:                       \
            hwValue = 3;                                                  \
            break;                                                        \
        default:                                                          \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);\
    }

#define PRV_CPSS_PX_PTP_HW_TO_ANCHOR_MAC(hwValue,anchor)                  \
    switch(hwValue)                                                       \
    {                                                                     \
        case 0:                                                           \
            anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;                \
            break;                                                        \
        case 1:                                                           \
            anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L3_E;                \
            break;                                                        \
        case 2:                                                           \
            anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L4_E;                \
            break;                                                        \
        case 3:                                                           \
            anchor = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_PTP_E;               \
            break;                                                        \
        default:                                                          \
            /*Not reached*/                                               \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);\
    }

/**
* @internal prvCpssPxPtpPortUnitResetSet function
* @endinternal
*
* @brief   Reset/unreset Port PTP unit.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] resetTxUnit              - GT_TRUE - reset PTP trasmit unit,
*                                      GT_FALSE - unreset PTP trasmit unit
* @param[in] resetRxUnit              - GT_TRUE - reset PTP receive unit
*                                      GT_FALSE - unreset PTP receive unit
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPtpPortUnitResetSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 resetTxUnit,
    IN  GT_BOOL                 resetRxUnit
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      portMacNum;     /* port MAC number */
    GT_U32      resetMask;      /* reset bits mask */
    GT_U32      resetData;      /* reset bits data */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
    resetData = 0;
    resetMask = (1 << 1) | (1 << 8);

    if (resetTxUnit == GT_FALSE)
    {
        U32_SET_FIELD_MAC(resetData, 1, 1, 1);
    }

    if (resetRxUnit == GT_FALSE)
    {
        U32_SET_FIELD_MAC(resetData, 8, 1, 1);
    }

    return prvCpssDrvHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, resetMask, resetData);
}

/**
* @internal prvCpssPxPtpTaiControlFieldSet function
* @endinternal
*
* @brief   Set TAI control register.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - physical device number
* @param[in] fieldOffset           - field offset
* @param[in] fieldBits             - field length
* @param[in] fieldValue            - field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPtpTaiControlFieldSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  fieldOffset,
    IN  GT_U32                  fieldBits,
    IN  GT_U32                  fieldValue
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data value */
    GT_STATUS   rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* TAI Control Register */
    regAddr = PX_TAI_CONTROL_REGISTER_ADDRESS;
    rc = cpssDrvPpHwInternalPciRegRead(CAST_SW_DEVNUM(devNum),
                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                       regAddr, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    U32_SET_FIELD_MAC(regData, fieldOffset, fieldBits, fieldValue);

    return cpssDrvPpHwInternalPciRegWrite(CAST_SW_DEVNUM(devNum),
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        regAddr, regData);
}

/**
* @internal prvCpssPxPtpTaiControlFieldGet function
* @endinternal
*
* @brief   Get TAI control register data.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - physical device number
* @param[in] fieldOffset           - field offset
* @param[in] fieldBits             - field length
* @param[out] fieldValuePtr        - (pointer to) field value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssPxPtpTaiControlFieldGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U32                  fieldOffset,
    IN  GT_U32                  fieldBits,
    OUT  GT_U32                 *fieldValuePtr
)
{
    GT_U32      regAddr;        /* register address */
    GT_U32      regData;        /* register data value */
    GT_STATUS   rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* TAI Control Register */
    regAddr = PX_TAI_CONTROL_REGISTER_ADDRESS;
    rc = cpssDrvPpHwInternalPciRegRead(CAST_SW_DEVNUM(devNum),
                                       CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                       regAddr, &regData);
    if(rc != GT_OK)
    {
        return rc;
    }

    *fieldValuePtr = U32_GET_FIELD_MAC(regData, fieldOffset, fieldBits);

    return GT_OK;
}

/**
* @internal prvCpssDxChPtpPLLConfig function
* @endinternal
*
* @brief   config the PTP PLL parameters to support different frequencies.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] clockFrequency           - PTP PLL frequency
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
static GT_STATUS prvCpssPxChPtpPLLConfig
(
    IN GT_U8                                    devNum,
    IN CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT           clockFrequency
)
{
    GT_STATUS   rc;
    GT_U32 pllConfigRegAddr;
    GT_U32 regAddr;
    GT_U32 hwValue;
    GT_U32 K;
    GT_U32 M;
    GT_U32 N;
    GT_U32 vcoBand;
    GT_U32 lpfCtrl;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    /* Default Frequency of the PTP PLL clock */
    if (clockFrequency == CPSS_PX_PTP_25_FREQ_E)
    {
        return GT_OK;
    }

    switch (clockFrequency)
    {
    case CPSS_PX_PTP_10_FREQ_E:
            K = 2;
            M = 0;
            N = 99;
            vcoBand = 2;
            lpfCtrl = 4;
            break;
    case CPSS_PX_PTP_20_FREQ_E:
            K = 2;
            M = 1;
            N = 99;
            vcoBand = 2;
            lpfCtrl = 4;
            break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* pll bypass */
    pllConfigRegAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.pllPTPConfig;
    hwValue = BOOL2BIT_MAC(GT_TRUE);
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), pllConfigRegAddr, 0, 1, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set pll params */
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.pllPTPParams;
    /* K */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 3, K);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* M */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 3, 9, M);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* N */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 12, 9, N);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* VCO band */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 21, 4, vcoBand);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* lpfCtrl */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 25, 4, lpfCtrl);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Set PTP PLL Reconfiguration Enable */
    hwValue = BOOL2BIT_MAC(GT_TRUE);
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), pllConfigRegAddr, 9, 1, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* clear pll bypass */
    hwValue = BOOL2BIT_MAC(GT_FALSE);
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), pllConfigRegAddr, 0, 1, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Toggle PTP clock from MISC0 PLL to PTP PLL*/
    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl14;
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 26, 1, 1 /* PTP PLL */);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*Set PTP PLL Reconfiguration Enable */
    hwValue = BOOL2BIT_MAC(GT_TRUE);
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), pllConfigRegAddr, 9, 1, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal internal_cpssPxPtpTaiCaptureOverwriteEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Capture Overwrite Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - Capture Overwrite Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiCaptureOverwriteEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    hwValue = BOOL2BIT_MAC(enable);
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 6, 1, hwValue);
}

/**
* @internal cpssPxPtpTaiCaptureOverwriteEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Capture Overwrite Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - Capture Overwrite Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiCaptureOverwriteEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiCaptureOverwriteEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPtpTaiCaptureOverwriteEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiCaptureOverwriteEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Capture Overwrite Enable
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) Capture Overwrite Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiCaptureOverwriteEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */
    GT_STATUS                          rc;          /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 6, 1, &hwValue);
    if(GT_OK != rc)
    {
        return rc;
    }
    *enablePtr = BIT2BOOL_MAC(hwValue);
    return rc;
}

/**
* @internal cpssPxPtpTaiCaptureOverwriteEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Capture Overwrite Enable
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) Capture Overwrite Enable.
*                                      When both TOD Capture registers are valid and capture
*                                      trigger arrives:
*                                      GT_TRUE  - the TOD Capture Value0 is overwritten.
*                                      GT_FALSE - the TOD Capture Value0 unchanged.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiCaptureOverwriteEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiCaptureOverwriteEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPtpTaiCaptureOverwriteEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] seconds                  -  part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] nanoSeconds              - nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..0x3FFFFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiClockCycleSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          seconds,
    IN  GT_U32          nanoSeconds
)
{
    GT_U32                             regAddr;        /* register address */
    GT_U32                             hwValue[2];     /* HW data  */
    GT_STATUS                          rc;             /* return code      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (seconds >= BIT_2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if (nanoSeconds >= BIT_30)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    hwValue[0] = ((nanoSeconds >> 16) & 0x3FFF) | ((seconds & 3) << 14);
    hwValue[1] = nanoSeconds & 0xFFFF;

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).clockCycleConfigHigh;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, hwValue[0]);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).clockCycleConfigLow;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, hwValue[1]);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPtpTaiClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] seconds                  -  part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[in] nanoSeconds              - nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..0x3FFFFFFF.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiClockCycleSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          seconds,
    IN  GT_U32          nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiClockCycleSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, seconds, nanoSeconds));

    rc = internal_cpssPxPtpTaiClockCycleSet(devNum, seconds, nanoSeconds);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, seconds, nanoSeconds));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] secondsPtr               - (pointer to) seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiClockCycleGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *secondsPtr,
    OUT GT_U32          *nanoSecondsPtr
)
{
    GT_U32                             regAddr;     /* register address */
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             hwValue[2];  /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(secondsPtr);
    CPSS_NULL_PTR_CHECK_MAC(nanoSecondsPtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).clockCycleConfigHigh;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue[0]);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).clockCycleConfigLow;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue[1]);
    if (rc != GT_OK)
    {
        return rc;
    }

    *secondsPtr     = ((hwValue[0] >> 14) & 3);
    *nanoSecondsPtr = (((hwValue[0] & 0x3FFF) << 16) | hwValue[1]);
    return rc;
}

/**
* @internal cpssPxPtpTaiClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] secondsPtr               - (pointer to) seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..3.)
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds part of cycle.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiClockCycleGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *secondsPtr,
    OUT GT_U32          *nanoSecondsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiClockCycleGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, secondsPtr, nanoSecondsPtr));

    rc = internal_cpssPxPtpTaiClockCycleGet(devNum, secondsPtr, nanoSecondsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, secondsPtr, nanoSecondsPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiClockModeSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] clockMode                - clock interface using mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiClockModeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_CLOCK_MODE_ENT  clockMode
)
{
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */
    GT_STATUS                          rc;          /* return code */
    GT_U32                             fieldValue;  /* register field value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch (clockMode)
    {
        case CPSS_PX_PTP_TAI_CLOCK_MODE_DISABLED_E:
            hwValue = 0;
            fieldValue = 0;
            break;
        case CPSS_PX_PTP_TAI_CLOCK_MODE_OUTPUT_E:
            hwValue = 1;
            fieldValue = 0;
            break;
        case CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_E:
            hwValue = 2;
            fieldValue = 1;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 9, 2, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Set bit [0] Rf TAI Clock In Enable  */
    return prvCpssPxPtpTaiControlFieldSet(devNum, 0, 1, fieldValue);
}

/**
* @internal cpssPxPtpTaiClockModeSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] clockMode                - clock interface using mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiClockModeSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_CLOCK_MODE_ENT  clockMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiClockModeSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, clockMode));

    rc = internal_cpssPxPtpTaiClockModeSet(devNum, clockMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, clockMode));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiClockModeGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] clockModePtr             - (pointer to) clock interface using mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiClockModeGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    OUT CPSS_PX_PTP_TAI_CLOCK_MODE_ENT  *clockModePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(clockModePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;

    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 9, 2, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (hwValue)
    {
        case 0:
            *clockModePtr = CPSS_PX_PTP_TAI_CLOCK_MODE_DISABLED_E;
            break;
        case 1:
            *clockModePtr = CPSS_PX_PTP_TAI_CLOCK_MODE_OUTPUT_E;
            break;
        case 2:
            *clockModePtr = CPSS_PX_PTP_TAI_CLOCK_MODE_INPUT_E;
            break;
        default:
            /* never occurs */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPtpTaiClockModeGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Clock Mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] clockModePtr             - (pointer to) clock interface using mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiClockModeGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    OUT CPSS_PX_PTP_TAI_CLOCK_MODE_ENT  *clockModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiClockModeGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, clockModePtr));

    rc = internal_cpssPxPtpTaiClockModeGet(devNum, clockModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, clockModePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiExternalPulseWidthSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Pulse Width.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] extPulseWidth            - External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on bad extPulseWidth
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiExternalPulseWidthSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          extPulseWidth
)
{
    GT_U32                             regAddr;     /* register address */
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             hwValue;     /* HW data  */
    CPSS_PX_PTP_TAI_TOD_STEP_STC       todStep;     /* TOD step data */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (extPulseWidth >= BIT_28)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = cpssPxPtpTaiTodStepGet(devNum, &todStep);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (0 == todStep.nanoSeconds)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    extPulseWidth /= todStep.nanoSeconds;

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig1;
    hwValue = (extPulseWidth & 0xFFFF);
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig2;
    hwValue = ((extPulseWidth >> 16) & 0x0FFF);
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 12, hwValue);
    return rc;
}

/**
* @internal cpssPxPtpTaiExternalPulseWidthSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) External Pulse Width.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] extPulseWidth            - External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on bad extPulseWidth
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiExternalPulseWidthSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          extPulseWidth
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiExternalPulseWidthSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, extPulseWidth));

    rc = internal_cpssPxPtpTaiExternalPulseWidthSet(devNum, extPulseWidth);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, extPulseWidth));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiExternalPulseWidthGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Pulse Width
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] extPulseWidthPtr         -  (pointer to) External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiExternalPulseWidthGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *extPulseWidthPtr
)
{
    GT_U32                             regAddr;     /* register address */
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             hwValue;     /* HW data  */
    CPSS_PX_PTP_TAI_TOD_STEP_STC       todStep;     /* TOD step data */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(extPulseWidthPtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig1;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *extPulseWidthPtr = hwValue;

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig2;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 12, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *extPulseWidthPtr |= (hwValue << 16);

    rc = cpssPxPtpTaiTodStepGet(devNum, &todStep);
    if (rc != GT_OK)
    {
        return rc;
    }
    *extPulseWidthPtr *= todStep.nanoSeconds;

    return GT_OK;
}

/**
* @internal cpssPxPtpTaiExternalPulseWidthGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) External Pulse Width
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] extPulseWidthPtr         -  (pointer to) External Pulse Width.
*                                      width of the pulse in nanoseconds of generated output trigger.
*                                      (APPLICABLE RANGES: 0..2^28-1.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiExternalPulseWidthGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *extPulseWidthPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiExternalPulseWidthGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, extPulseWidthPtr));

    rc = internal_cpssPxPtpTaiExternalPulseWidthGet(devNum, extPulseWidthPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, extPulseWidthPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiFractionalNanosecondDriftSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] fracNanoSecond           - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiFractionalNanosecondDriftSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_32           fracNanoSecond
)
{
    GT_U32                             regAddr;     /* register address */
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    hwValue = (fracNanoSecond >> 16) & 0xFFFF;
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).driftAdjustmentConfigHigh;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwValue = fracNanoSecond & 0xFFFF;
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).driftAdjustmentConfigLow;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, hwValue);
    return rc;
}

/**
* @internal cpssPxPtpTaiFractionalNanosecondDriftSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] fracNanoSecond           - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiFractionalNanosecondDriftSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_32           fracNanoSecond
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiFractionalNanosecondDriftSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fracNanoSecond));

    rc = internal_cpssPxPtpTaiFractionalNanosecondDriftSet(devNum, fracNanoSecond);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fracNanoSecond));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiFractionalNanosecondDriftGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] fracNanoSecondPtr        - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiFractionalNanosecondDriftGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_32           *fracNanoSecondPtr
)
{
    GT_U32                             regAddr;     /* register address */
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(fracNanoSecondPtr);

    *fracNanoSecondPtr = 0;
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).driftAdjustmentConfigHigh;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *fracNanoSecondPtr |= (hwValue << 16);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).driftAdjustmentConfigLow;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *fracNanoSecondPtr |= hwValue;

    return rc;
}

/**
* @internal cpssPxPtpTaiFractionalNanosecondDriftGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Fractional Nanosecond Drift.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] fracNanoSecondPtr        - (pointer to) fractional nano seconds drift.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiFractionalNanosecondDriftGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_32           *fracNanoSecondPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiFractionalNanosecondDriftGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fracNanoSecondPtr));

    rc = internal_cpssPxPtpTaiFractionalNanosecondDriftGet(devNum, fracNanoSecondPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fracNanoSecondPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPtpTaiGracefulStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] gracefulStep             - Graceful Step in nanoseconds.
*                                      (APPLICABLE RANGES: 0..31.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on wrong gracefulStep
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiGracefulStepSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          gracefulStep
)
{
    GT_U32                             regAddr;        /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (gracefulStep >= BIT_5)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).TAICtrlReg0;

    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 5, gracefulStep);
}

/**
* @internal cpssPxPtpTaiGracefulStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] gracefulStep             - Graceful Step in nanoseconds.
*                                      (APPLICABLE RANGES: 0..31.)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on wrong gracefulStep
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiGracefulStepSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          gracefulStep
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiGracefulStepSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, gracefulStep));

    rc = internal_cpssPxPtpTaiGracefulStepSet(devNum, gracefulStep);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, gracefulStep));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiGracefulStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] gracefulStepPtr          - (pointer to) Graceful Step in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiGracefulStepGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *gracefulStepPtr
)
{
    GT_U32                             regAddr;        /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(gracefulStepPtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).TAICtrlReg0;

    return prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 5, gracefulStepPtr);
}

/**
* @internal cpssPxPtpTaiGracefulStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Graceful Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] gracefulStepPtr          - (pointer to) Graceful Step in nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiGracefulStepGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *gracefulStepPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiGracefulStepGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, gracefulStepPtr));

    rc = internal_cpssPxPtpTaiGracefulStepGet(devNum, gracefulStepPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, gracefulStepPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiIncomingClockCounterEnableSet function
* @endinternal
*
* @brief   Set enabled state of TAI (Time Application Interface) Incoming Clock
*         Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiIncomingClockCounterEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32                             regAddr;        /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
        incomingClockInCountingEnable;
    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1,
        (BOOL2BIT_MAC(enable)));
}

/**
* @internal cpssPxPtpTaiIncomingClockCounterEnableSet function
* @endinternal
*
* @brief   Set enabled state of TAI (Time Application Interface) Incoming Clock
*         Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -  counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiIncomingClockCounterEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiIncomingClockCounterEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPtpTaiIncomingClockCounterEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiIncomingClockCounterEnableGet function
* @endinternal
*
* @brief   Get enabled state of TAI (Time Application Interface) Incoming Clock
*         Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiIncomingClockCounterEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32                             regAddr;        /* register address */
    GT_STATUS                          rc;             /* return code      */
    GT_U32                             hwValue;        /* hw value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
        incomingClockInCountingEnable;

    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &hwValue);
    *enablePtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

/**
* @internal cpssPxPtpTaiIncomingClockCounterEnableGet function
* @endinternal
*
* @brief   Get enabled state of TAI (Time Application Interface) Incoming Clock
*         Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)enable counting.
*                                      GT_TRUE - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiIncomingClockCounterEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiIncomingClockCounterEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPtpTaiIncomingClockCounterEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiIncomingClockCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPxPtpTaiIncomingClockCounterGet
(
    IN  GT_U8                                 devNum,
    OUT GT_U32                                *valuePtr
)
{
    GT_U32                             regAddr;        /* register address */
    GT_STATUS                          rc;             /* return code      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
        incomingClockInCountingConfigLow;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, valuePtr);

    return rc;
}

/**
* @internal cpssPxPtpTaiIncomingClockCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Clock Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiIncomingClockCounterGet
(
    IN  GT_U8                                 devNum,
    OUT GT_U32                                *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiIncomingClockCounterGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, valuePtr));

    rc = internal_cpssPxPtpTaiIncomingClockCounterGet(devNum, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiIncomingTriggerCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiIncomingTriggerCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *valuePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).captureTriggerCntr;

    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 8, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *valuePtr = hwValue;
    return GT_OK;
}

/**
* @internal cpssPxPtpTaiIncomingTriggerCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Incoming Trigger Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to)value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiIncomingTriggerCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiIncomingTriggerCounterGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, valuePtr));

    rc = internal_cpssPxPtpTaiIncomingTriggerCounterGet(devNum, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiInputClockSelectSet function
* @endinternal
*
* @brief   Configures input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] clockSelect              - input clock selection mode.
* @param[in] clockFrequency           - Frequency of the PTP PLL clock (default is 25).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiInputClockSelectSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT    clockSelect,
    IN  CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT      clockFrequency
)
{
    GT_U32    regAddr;          /* register address             */
    GT_U32    fieldValue;       /* register field value         */
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch(clockSelect)
    {
        case CPSS_PX_PTP_TAI_CLOCK_SELECT_PTP_PLL_E:
            fieldValue = 1;
            break;
        case CPSS_PX_PTP_TAI_CLOCK_SELECT_CORE_PLL_E:
            fieldValue = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->DFXServerUnitsDeviceSpecificRegs.deviceCtrl14;
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 26, 1, fieldValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (clockSelect == CPSS_PX_PTP_TAI_CLOCK_SELECT_PTP_PLL_E)
    {
        return prvCpssPxChPtpPLLConfig(devNum, clockFrequency);
    }

    return GT_OK;
}

/**
* @internal cpssPxPtpTaiInputClockSelectSet function
* @endinternal
*
* @brief   Configures input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] clockSelect              - input clock selection mode.
* @param[in] clockFrequency           - Frequency of the PTP PLL clock (default is 25).
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiInputClockSelectSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT    clockSelect,
    IN  CPSS_PX_PTP_TAI_CLOCK_FREQ_ENT      clockFrequency
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiInputClockSelectSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, clockSelect, clockFrequency));

    rc = internal_cpssPxPtpTaiInputClockSelectSet(devNum, clockSelect, clockFrequency);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, clockSelect, clockFrequency));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiInputClockSelectGet function
* @endinternal
*
* @brief   Get input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] clockSelectPtr           - (pointer to) input clock selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiInputClockSelectGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT    *clockSelectPtr
)
{
    GT_U32    regAddr;          /* register address             */
    GT_U32    fieldValue;       /* register field value         */
    GT_STATUS rc;               /* status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(clockSelectPtr);

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
        DFXServerUnitsDeviceSpecificRegs.deviceCtrl14;

    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 26,
        1, &fieldValue);

    switch(fieldValue)
    {
        case 1:
            *clockSelectPtr = CPSS_PX_PTP_TAI_CLOCK_SELECT_PTP_PLL_E;
            break;
        case 0:
            *clockSelectPtr = CPSS_PX_PTP_TAI_CLOCK_SELECT_CORE_PLL_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;
}

/**
* @internal cpssPxPtpTaiInputClockSelectGet function
* @endinternal
*
* @brief   Get input TAI clock's selection.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] clockSelectPtr           - (pointer to) input clock selection mode.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiInputClockSelectGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_PX_PTP_TAI_CLOCK_SELECT_ENT    *clockSelectPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiInputClockSelectGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, clockSelectPtr));

    rc = internal_cpssPxPtpTaiInputClockSelectGet(devNum, clockSelectPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, clockSelectPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiInputTriggersCountEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Count Input Triggers Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssPxPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiInputTriggersCountEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32                             regAddr;     /* register address */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 5, 1,
        BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxPtpTaiInputTriggersCountEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Count Input Triggers Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssPxPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiInputTriggersCountEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiInputTriggersCountEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPtpTaiInputTriggersCountEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiInputTriggersCountEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Count Input Triggers Enable
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssPxPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiInputTriggersCountEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */
    GT_STATUS                          rc;          /* return code      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 5, 1, &hwValue);
    *enablePtr = BIT2BOOL_MAC(hwValue);
    return rc;
}

/**
* @internal cpssPxPtpTaiInputTriggersCountEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Count Input Triggers Enable
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) Count Input Triggers Enable.
*                                      GT_TRUE  - each incoming trigger is counted.
*                                      GT_FALSE - incoming triggers not counted.
*                                      see cpssPxPtpTaiIncomingTriggerCounterSet/Get
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiInputTriggersCountEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiInputTriggersCountEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPtpTaiInputTriggersCountEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiOutputTriggerEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -   output trigger generation.
*                                      GT_TRUE  - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note use cpssPxPtpTaiTodSet with
*       CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E and
*       the time to trigger generation.
*
*/
static GT_STATUS internal_cpssPxPtpTaiOutputTriggerEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32                             regAddr;     /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;

    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 1,
                                     BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxPtpTaiOutputTriggerEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   -   output trigger generation.
*                                      GT_TRUE  - enable, GT_FALSE - disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note use cpssPxPtpTaiTodSet with
*       CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E and
*       the time to trigger generation.
*
*/
GT_STATUS cpssPxPtpTaiOutputTriggerEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiOutputTriggerEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPtpTaiOutputTriggerEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiOutputTriggerEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable output trigger generation.
*                                      GT_TRUE  - enable GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiOutputTriggerEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */
    GT_STATUS                          rc;          /* return code      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 1, 1, &hwValue);
    *enablePtr = BIT2BOOL_MAC(hwValue);
    return rc;
}

/**
* @internal cpssPxPtpTaiOutputTriggerEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) Output Trigger Generation Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) enable output trigger generation.
*                                      GT_TRUE  - enable GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiOutputTriggerEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiOutputTriggerEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPtpTaiOutputTriggerEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiPClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] nanoSeconds              - nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiPClockCycleSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          nanoSeconds
)
{
    GT_STATUS                          rc;             /* return code      */
    GT_U32                             regAddr;        /* register address */
    GT_U32                             hwValue;        /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).PClkClockCycleConfigHigh;
    hwValue = (nanoSeconds >> 16) & 0xFFFF;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, hwValue);
    if(GT_OK != rc)
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).PClkClockCycleConfigLow;
    hwValue = nanoSeconds & 0xFFFF;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, hwValue);
    return rc;
}

/**
* @internal cpssPxPtpTaiPClockCycleSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] nanoSeconds              - nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPClockCycleSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          nanoSeconds
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiPClockCycleSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, nanoSeconds));

    rc = internal_cpssPxPtpTaiPClockCycleSet(devNum, nanoSeconds);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, nanoSeconds));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiPClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiPClockCycleGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *nanoSecondsPtr
)
{
    GT_STATUS                          rc;             /* return code      */
    GT_U32                             regAddr;        /* register address */
    GT_U32                             hwValue;        /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(nanoSecondsPtr);
    *nanoSecondsPtr = 0;

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).PClkClockCycleConfigHigh;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue);
    if(GT_OK != rc)
    {
        return rc;
    }
    *nanoSecondsPtr |= (hwValue << 16);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).PClkClockCycleConfigLow;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue);
    if(GT_OK != rc)
    {
        return rc;
    }
    *nanoSecondsPtr |= hwValue;

    return GT_OK;
}

/**
* @internal cpssPxPtpTaiPClockCycleGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PClock Cycle.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] nanoSecondsPtr           - (pointer to) nano seconds cycle.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPClockCycleGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *nanoSecondsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiPClockCycleGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, nanoSecondsPtr));

    rc = internal_cpssPxPtpTaiPClockCycleGet(devNum, nanoSecondsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, nanoSecondsPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiPClockDriftAdjustEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiPClockDriftAdjustEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_U32                             regAddr;     /* register address */
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 7, 1,
        BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxPtpTaiPClockDriftAdjustEnableSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPClockDriftAdjustEnableSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_BOOL         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiPClockDriftAdjustEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPtpTaiPClockDriftAdjustEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiPtpPClockDriftAdjustEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiPtpPClockDriftAdjustEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */
    GT_STATUS                          rc;          /* return code      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 7, 1, &hwValue);
    *enablePtr = BIT2BOOL_MAC(hwValue);
    return rc;
}

/**
* @internal cpssPxPtpTaiPtpPClockDriftAdjustEnableGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) PTP Clock Adjust Enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) PTP PClock Drift Adjust Enable.
*                                      GT_TRUE - the PClk is affected by the <Drift Adjustment> or not.
*                                      GT_FALSE - the PClk reflects the frequency of the free running TOD,
*                                      without any frequency adjustments.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPtpPClockDriftAdjustEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiPtpPClockDriftAdjustEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPtpTaiPtpPClockDriftAdjustEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiTodCaptureStatusGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Capture Status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] captureIndex             - Capture Index.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] validPtr                 - (pointer to)Capture is valid.
*                                      GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiTodCaptureStatusGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          captureIndex,
    OUT GT_BOOL         *validPtr
)
{
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(validPtr);

    if (captureIndex > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCaptureStatus;

    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, captureIndex, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *validPtr = BIT2BOOL_MAC(hwValue);
    return GT_OK;
}

/**
* @internal cpssPxPtpTaiTodCaptureStatusGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Capture Status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] captureIndex             - Capture Index.
*                                      (APPLICABLE RANGES: 0..1.)
*
* @param[out] validPtr                 - (pointer to)Capture is valid.
*                                      GT_TRUE - valid, GT_FALSE - invalid
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodCaptureStatusGet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          captureIndex,
    OUT GT_BOOL         *validPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiTodCaptureStatusGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, captureIndex, validPtr));

    rc = internal_cpssPxPtpTaiTodCaptureStatusGet(devNum, captureIndex, validPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, captureIndex, validPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiTodSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD values.
*         The TOD will be updated by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] todValueType             - type of TOD value.
*                                      Valid types are
*                                      CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,
*                                      CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,
*                                      CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E.
* @param[in] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API does not activate any triggers, other APIs does it
*
*/
static GT_STATUS internal_cpssPxPtpTaiTodSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_TOD_TYPE_ENT    todValueType,
    IN  CPSS_PX_PTP_TAI_TOD_COUNT_STC   *todValuePtr
)
{
    GT_STATUS                          rc;             /* return code      */
    GT_U32                             regAddr[7];     /* register address */
    GT_U32                             hwValue[7];     /* HW data  */
    GT_U32                             i;              /* loop index       */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(todValuePtr);

    hwValue[0] = todValuePtr->seconds.l[1] & 0xFFFF;
    hwValue[1] = (todValuePtr->seconds.l[0] >> 16) & 0xFFFF;
    hwValue[2] = todValuePtr->seconds.l[0] & 0xFFFF;
    hwValue[3] = (todValuePtr->nanoSeconds >> 16) & 0xFFFF;
    hwValue[4] = todValuePtr->nanoSeconds & 0xFFFF;
    hwValue[5] = (todValuePtr->fracNanoSeconds >> 16) & 0xFFFF;
    hwValue[6] = todValuePtr->fracNanoSeconds & 0xFFFF;

    switch(todValueType)
    {
        case CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E:
            regAddr[0] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODSecHigh;
            regAddr[1] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODSecMed;
            regAddr[2] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODSecLow;
            regAddr[3] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODNanoHigh;
            regAddr[4] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODNanoLow;
            regAddr[5] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODFracHigh;
            regAddr[6] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODFracLow;
            break;
        case CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E:
            regAddr[0] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskSecHigh;
            regAddr[1] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskSecMed;
            regAddr[2] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskSecLow;
            regAddr[3] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskNanoHigh;
            regAddr[4] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskNanoLow;
            regAddr[5] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskFracHigh;
            regAddr[6] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskFracLow;
            break;
        case CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E:
            regAddr[0] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueSecHigh;
            regAddr[1] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueSecMed;
            regAddr[2] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueSecLow;
            regAddr[3] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueNanoHigh;
            regAddr[4] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueNanoLow;
            regAddr[5] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueFracHigh;
            regAddr[6] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueFracLow;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i = 0; (i < 7); i++)
    {
        rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr[i], 0, 16, hwValue[i]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxPtpTaiTodSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD values.
*         The TOD will be updated by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] todValueType             - type of TOD value.
*                                      Valid types are
*                                      CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E,
*                                      CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E,
*                                      CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E.
* @param[in] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This API does not activate any triggers, other APIs does it
*
*/
GT_STATUS cpssPxPtpTaiTodSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_TOD_TYPE_ENT    todValueType,
    IN  CPSS_PX_PTP_TAI_TOD_COUNT_STC   *todValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiTodSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, todValueType, todValuePtr));

    rc = internal_cpssPxPtpTaiTodSet(devNum, todValueType, todValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, todValueType, todValuePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiTodGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD values.
*         The TOD was captured by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] todValueType             - type of TOD value.
*
* @param[out] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiTodGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_TOD_TYPE_ENT    todValueType,
    OUT CPSS_PX_PTP_TAI_TOD_COUNT_STC   *todValuePtr
)
{
    GT_STATUS                          rc;             /* return code      */
    GT_U32                             regAddr[7];     /* register address */
    GT_U32                             hwValue[7];     /* HW data  */
    GT_U32                             i;              /* loop index       */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(todValuePtr);

    switch(todValueType)
    {
        case CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_E:
            regAddr[0] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODSecHigh;
            regAddr[1] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODSecMed;
            regAddr[2] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODSecLow;
            regAddr[3] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODNanoHigh;
            regAddr[4] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODNanoLow;
            regAddr[5] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODFracHigh;
            regAddr[6] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                triggerGenerationTODFracLow;
            break;
        case CPSS_PX_PTP_TAI_TOD_TYPE_TRIGGER_GENERATION_MASK_E:
            regAddr[0] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskSecHigh;
            regAddr[1] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskSecMed;
            regAddr[2] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskSecLow;
            regAddr[3] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskNanoHigh;
            regAddr[4] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskNanoLow;
            regAddr[5] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskFracHigh;
            regAddr[6] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                generateFunctionMaskFracLow;
            break;
        case CPSS_PX_PTP_TAI_TOD_TYPE_LOAD_VALUE_E:
            regAddr[0] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueSecHigh;
            regAddr[1] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueSecMed;
            regAddr[2] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueSecLow;
            regAddr[3] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueNanoHigh;
            regAddr[4] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueNanoLow;
            regAddr[5] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueFracHigh;
            regAddr[6] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeLoadValueFracLow;
            break;
        case CPSS_PX_PTP_TAI_TOD_TYPE_CAPTURE_VALUE0_E:
            regAddr[0] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue0SecHigh;
            regAddr[1] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue0SecMed;
            regAddr[2] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue0SecLow;
            regAddr[3] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue0NanoHigh;
            regAddr[4] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue0NanoLow;
            regAddr[5] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue0FracHigh;
            regAddr[6] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue0FracLow;
            break;
        case CPSS_PX_PTP_TAI_TOD_TYPE_CAPTURE_VALUE1_E:
            regAddr[0] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue1SecHigh;
            regAddr[1] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue1SecMed;
            regAddr[2] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue1SecLow;
            regAddr[3] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue1NanoHigh;
            regAddr[4] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue1NanoLow;
            regAddr[5] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue1FracHigh;
            regAddr[6] = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).
                timeCaptureValue1FracLow;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i = 0; (i < 7); i++)
    {
        rc = prvCpssDrvHwPpGetRegField(
            CAST_SW_DEVNUM(devNum), regAddr[i], 0, 16, &(hwValue[i]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    todValuePtr->seconds.l[1]    = hwValue[0];
    todValuePtr->seconds.l[0]    = ((hwValue[1] << 16) | hwValue[2]);
    todValuePtr->nanoSeconds     = ((hwValue[3] << 16) | hwValue[4]);
    todValuePtr->fracNanoSeconds = ((hwValue[5] << 16) | hwValue[6]);
    return GT_OK;
}

/**
* @internal cpssPxPtpTaiTodGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD values.
*         The TOD was captured by triggering an appropriate function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] todValueType             - type of TOD value.
*
* @param[out] todValuePtr              - (pointer to) TOD value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_TOD_TYPE_ENT    todValueType,
    OUT CPSS_PX_PTP_TAI_TOD_COUNT_STC   *todValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiTodGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, todValueType, todValuePtr));

    rc = internal_cpssPxPtpTaiTodGet(devNum, todValueType, todValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, todValueType, todValuePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiTodStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiTodStepSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_U32                             hwValue;        /* HW data  */
    GT_U32                             regAddr;        /* register address */
    GT_STATUS                          rc;             /* return code      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(todStepPtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).TODStepNanoConfigLow;
    hwValue = todStepPtr->nanoSeconds & 0xFFFF;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwValue = (todStepPtr->fracNanoSeconds >> 16) & 0xFFFF;
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).TODStepFracConfigHigh;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    hwValue = todStepPtr->fracNanoSeconds & 0xFFFF;
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).TODStepFracConfigLow;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, hwValue);
    return rc;
}

/**
* @internal cpssPxPtpTaiTodStepSet function
* @endinternal
*
* @brief   Set TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodStepSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  CPSS_PX_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiTodStepSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, todStepPtr));

    rc = internal_cpssPxPtpTaiTodStepSet(devNum, todStepPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, todStepPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiTodStepGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    OUT CPSS_PX_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_U32                             hwValue;        /* HW data  */
    GT_U32                             regAddr;        /* register address */
    GT_STATUS                          rc;             /* return code      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(todStepPtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).TODStepNanoConfigLow;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    todStepPtr->nanoSeconds = hwValue;

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).TODStepFracConfigHigh;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    todStepPtr->fracNanoSeconds = 0;
    U32_SET_FIELD_MAC(todStepPtr->fracNanoSeconds, 16, 16, hwValue);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).TODStepFracConfigLow;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue);
    U32_SET_FIELD_MAC(todStepPtr->fracNanoSeconds, 0, 16, hwValue);
    return rc;
}

/**
* @internal cpssPxPtpTaiTodStepGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Step.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] todStepPtr               - (pointer to) TOD Step.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodStepGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    OUT CPSS_PX_PTP_TAI_TOD_STEP_STC    *todStepPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiTodStepGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, todStepPtr));

    rc = internal_cpssPxPtpTaiTodStepGet(devNum, todStepPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, todStepPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiTodUpdateCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Update Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to)TOD Update Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counter is Clear On Read. Value incremented at each
*       TOD Update/Increment/Decrement function.
*
*/
static GT_STATUS internal_cpssPxPtpTaiTodUpdateCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *valuePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;     /* HW data  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    *valuePtr = 0;
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeUpdateCntrHigh;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *valuePtr |= (hwValue << 16);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeUpdateCntrLow;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }
    *valuePtr |= hwValue;

    return GT_OK;
}

/**
* @internal cpssPxPtpTaiTodUpdateCounterGet function
* @endinternal
*
* @brief   Get TAI (Time Application Interface) TOD Update Counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to)TOD Update Counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counter is Clear On Read. Value incremented at each
*       TOD Update/Increment/Decrement function.
*
*/
GT_STATUS cpssPxPtpTaiTodUpdateCounterGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiTodUpdateCounterGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, valuePtr));

    rc = internal_cpssPxPtpTaiTodUpdateCounterGet(devNum, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiTodCounterFunctionSet function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by cpssPxPtpTaiTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] function                 - One of the four possible TOD counter functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiTodCounterFunctionSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT function
)
{
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    switch (function)
    {
        case CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E:
            hwValue = 0;
            break;
        case CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E:
            hwValue = 1;
            break;
        case CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_INCREMENT_E:
            hwValue = 2;
            break;
        case CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_DECREMENT_E:
            hwValue = 3;
            break;
        case CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_CAPTURE_E:
            hwValue = 4;
            break;
        case CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E:
            hwValue = 5;
            break;
        case CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E:
            hwValue = 6;
            break;
        case CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_NOP_E:
            hwValue = 7;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 3, hwValue);
}

/**
* @internal cpssPxPtpTaiTodCounterFunctionSet function
* @endinternal
*
* @brief   The function configures type of TOD counter action that will be
*         performed once triggered by cpssPxPtpTaiTodCounterFunctionTriggerSet()
*         API or by pulse from an external interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] function                 - One of the four possible TOD counter functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodCounterFunctionSet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT function
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiTodCounterFunctionSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, function));

    rc = internal_cpssPxPtpTaiTodCounterFunctionSet(devNum, function);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, function));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiTodCounterFunctionGet function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         cpssPxPtpTaiTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] functionPtr              - (pointer to) One of the four possible TOD counter
*                                      functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiTodCounterFunctionGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT *functionPtr
)
{
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(functionPtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 3, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (hwValue)
    {
        case 0:
            *functionPtr = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_UPDATE_E;
            break;
        case 1:
            *functionPtr = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_FREQUENCE_UPDATE_E;
            break;
        case 2:
            *functionPtr = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_INCREMENT_E;
            break;
        case 3:
            *functionPtr = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_DECREMENT_E;
            break;
        case 4:
            *functionPtr = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_CAPTURE_E;
            break;
        case 5:
            *functionPtr = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GRACEFULLY_INCREMENT_E;
            break;
        case 6:
            *functionPtr = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_GRACEFULLY_DECREMENT_E;
            break;
        case 7:
            *functionPtr = CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_NOP_E;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal cpssPxPtpTaiTodCounterFunctionGet function
* @endinternal
*
* @brief   Get type of TOD counter action that will be performed once triggered by
*         cpssPxPtpTaiTodCounterFunctionTriggerSet() API or by pulse from an
*         external interface.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] functionPtr              - (pointer to) One of the four possible TOD counter
*                                      functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodCounterFunctionGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_PX_PTP_TAI_TOD_COUNTER_FUNC_ENT *functionPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiTodCounterFunctionGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, functionPtr));

    rc = internal_cpssPxPtpTaiTodCounterFunctionGet(devNum, functionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, functionPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiTodCounterFunctionTriggerSet function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function accorging to the function set
*         by cpssPxPtpTaiTodCounterFunctionSet().
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiTodCounterFunctionTriggerSet
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             regAddr;     /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;

    /* toggle required for trigger */
    /* unset trigger bit */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, 0);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set trigger bit */
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, 1);
    return rc;
}

/**
* @internal cpssPxPtpTaiTodCounterFunctionTriggerSet function
* @endinternal
*
* @brief   Trigger TOD (Time of Day) counter function accorging to the function set
*         by cpssPxPtpTaiTodCounterFunctionSet().
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - when previous TOD triggered action is not finished yet
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodCounterFunctionTriggerSet
(
    IN  GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiTodCounterFunctionTriggerSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssPxPtpTaiTodCounterFunctionTriggerSet(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiTodCounterFunctionTriggerGet function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - The trigger is on, the TOD
*                                      function is not finished.
*                                      GT_FALSE - The trigger is off, the TOD function is
*                                      finished.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiTodCounterFunctionTriggerGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS                          rc;          /* return code      */
    GT_U32                             regAddr;     /* register address */
    GT_U32                             hwValue;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;

    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(hwValue);

    return GT_OK;
}

/**
* @internal cpssPxPtpTaiTodCounterFunctionTriggerGet function
* @endinternal
*
* @brief   Get status of trigger TOD (Time of Day) counter function.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) GT_TRUE - The trigger is on, the TOD
*                                      function is not finished.
*                                      GT_FALSE - The trigger is off, the TOD function is
*                                      finished.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum or direction
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiTodCounterFunctionTriggerGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiTodCounterFunctionTriggerGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPtpTaiTodCounterFunctionTriggerGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTsuControlSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTsuControlSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PTP_TSU_CONTROL_STC *controlPtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  portMacNum;     /* MAC number       */
    GT_U32  hwData;         /* HW data          */
    GT_U32  hwMask;         /* HW mask          */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(controlPtr);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    hwMask = 0;
    hwData = 0;

    hwData |= BOOL2BIT_MAC(controlPtr->unitEnable);
    hwMask |= 1;

    regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
    return prvCpssDrvHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, hwMask, hwData);
}

/**
* @internal cpssPxPtpTsuControlSet function
* @endinternal
*
* @brief   Set TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTsuControlSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    IN  CPSS_PX_PTP_TSU_CONTROL_STC *controlPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTsuControlSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, controlPtr));

    rc = internal_cpssPxPtpTsuControlSet(devNum, portNum, controlPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, controlPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTsuControlGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTsuControlGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PTP_TSU_CONTROL_STC *controlPtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  portMacNum;     /* MAC number       */
    GT_U32  hwData;         /* HW data          */
    GT_STATUS   rc;         /* return code      */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(controlPtr);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
    rc = prvCpssDrvHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &hwData);
    if (rc != GT_OK)
    {
        return rc;
    }
    controlPtr->unitEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwData, 0, 1));
    return GT_OK;
}

/**
* @internal cpssPxPtpTsuControlGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) Control.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @param[out] controlPtr               - (pointer to) control structure.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTsuControlGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    OUT CPSS_PX_PTP_TSU_CONTROL_STC *controlPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTsuControlGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, controlPtr));

    rc = internal_cpssPxPtpTsuControlGet(devNum, portNum, controlPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, controlPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTsuCountersClear function
* @endinternal
*
* @brief   Clear All TSU (Time Stamp Unit) Packet counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTsuCountersClear
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    GT_U32  regAddr;        /* register address */
    GT_U32  portMacNum;     /* MAC number       */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).PTPGeneralCtrl;
    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 4, 1, 0);
    if (rc != GT_OK)
    {
        return rc;
    }
    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 4, 1, 1);
}

/**
* @internal cpssPxPtpTsuCountersClear function
* @endinternal
*
* @brief   Clear All TSU (Time Stamp Unit) Packet counters.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTsuCountersClear
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTsuCountersClear);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum));

    rc = internal_cpssPxPtpTsuCountersClear(devNum, portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTsuPacketCouterGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) packet counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] counterType              - counter type (see enum)
*
* @param[out] valuePtr                 - (pointer to) counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTsuPacketCouterGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT counterType,
    OUT GT_U32                                  *valuePtr
)
{
    GT_U32  regAddr;        /* register address */
    GT_U32  portMacNum;     /* MAC number       */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);

    switch (counterType)
    {
        case CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_TOTAL_E:
            regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).
                totalPTPPktsCntr;
            break;
        case CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V1_E:
            regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).
                PTPv1PktCntr;
            break;
        case CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_PTP_V2_E:
            regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).
                PTPv2PktCntr;
            break;
        case CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ADD_CORR_TIME_E:
            regAddr = PRV_PX_REG1_UNIT_GOP_PTP_MAC(devNum, portMacNum).
                addCorrectedTimeActionPktCntr;
            break;
        default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 8, valuePtr);
}

/**
* @internal cpssPxPtpTsuPacketCouterGet function
* @endinternal
*
* @brief   Get TSU (Time Stamp Unit) packet counter.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number.
* @param[in] counterType              - counter type (see enum)
*
* @param[out] valuePtr                 - (pointer to) counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTsuPacketCouterGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PTP_TSU_PACKET_COUNTER_TYPE_ENT counterType,
    OUT GT_U32                                  *valuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTsuPacketCouterGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, counterType, valuePtr));

    rc = internal_cpssPxPtpTsuPacketCouterGet(devNum, portNum, counterType, valuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, counterType, valuePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssPxPtpPortTypeKeySet function
* @endinternal
*
* @brief   Set port PTP type key info.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] portKeyPtr               - (pointer to) port PTP type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpPortTypeKeySet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PTP_TYPE_KEY_STC                *portKeyPtr
)
{
    GT_U32      ii;             /* iterator */
    GT_U32      regAddr;        /* register address */
    GT_STATUS   rc;             /* return value */
    GT_U32      value;          /* register value */

    /* anchor hw value for UDBPs */
    GT_U32      anchorUdbpHw[CPSS_PX_PTP_TYPE_UDBP_NUM_CNS];

    /* anchor hw value for UDBs */
    GT_U32      anchorUdbHw[CPSS_PX_PTP_TYPE_UDB_NUM_CNS];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(portKeyPtr);

    for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; ii++)
    {
        CPSS_DATA_CHECK_MAX_MAC(portKeyPtr->portUdbPairArr[ii].udbByteOffset,
            CPSS_PX_PTP_UDB_OFFSET_MAX_CNS + 1);
        PRV_CPSS_PX_PTP_ANCHOR_TO_HW_MAC(
            portKeyPtr->portUdbPairArr[ii].udbAnchorType, anchorUdbpHw[ii]);
    }

    for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; ii++)
    {
        CPSS_DATA_CHECK_MAX_MAC(portKeyPtr->portUdbArr[ii].udbByteOffset,
            CPSS_PX_PTP_UDB_OFFSET_MAX_CNS + 1);
        PRV_CPSS_PX_PTP_ANCHOR_TO_HW_MAC(
            portKeyPtr->portUdbArr[ii].udbAnchorType, anchorUdbHw[ii]);
    }

    for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; ii++)
    {
        regAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPortPTPTypeKeyUDBPConfigPort[ii][portNum];
        value = 0;
        U32_SET_FIELD_MAC(value, 0, 2, anchorUdbpHw[ii]);
        U32_SET_FIELD_MAC(value, 2, 7,
            portKeyPtr->portUdbPairArr[ii].udbByteOffset);
        rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 9, value);
        if(GT_OK != rc)
        {
            return rc;
        }
    }

    for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; ii++)
    {
        regAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPortPTPTypeKeyUDBConfigPort[ii][portNum];
        value = 0;
        U32_SET_FIELD_MAC(value, 0, 2, anchorUdbHw[ii]);
        U32_SET_FIELD_MAC(value, 2, 7,
            portKeyPtr->portUdbArr[ii].udbByteOffset);
        rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 9, value);
        if(GT_OK != rc)
        {
            return rc;
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxPtpPortTypeKeySet function
* @endinternal
*
* @brief   Set port PTP type key info.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
* @param[in] portKeyPtr               - (pointer to) port PTP type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpPortTypeKeySet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    IN  CPSS_PX_PTP_TYPE_KEY_STC    *portKeyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpPortTypeKeySet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portKeyPtr));

    rc = internal_cpssPxPtpPortTypeKeySet(devNum, portNum, portKeyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portKeyPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpPortTypeKeyGet function
* @endinternal
*
* @brief   Set port PTP type key info.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] portKeyPtr               - (pointer to) port PTP type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpPortTypeKeyGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_PX_PTP_TYPE_KEY_STC    *portKeyPtr
)
{
    GT_U32      ii;             /* iterator */
    GT_U32      regAddr;        /* register address */
    GT_STATUS   rc;             /* return value */
    GT_U32      value;          /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum, portNum);
    CPSS_NULL_PTR_CHECK_MAC(portKeyPtr);

    for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; ii++)
    {
        regAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPortPTPTypeKeyUDBPConfigPort[ii][portNum];
        rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 9, &value);
        if(GT_OK != rc)
        {
            return rc;
        }
        portKeyPtr->portUdbPairArr[ii].udbByteOffset =
            U32_GET_FIELD_MAC(value, 2, 7);
        PRV_CPSS_PX_PTP_HW_TO_ANCHOR_MAC(U32_GET_FIELD_MAC(value, 0, 2),
            portKeyPtr->portUdbPairArr[ii].udbAnchorType);
    }

    for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; ii++)
    {
        regAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPortPTPTypeKeyUDBConfigPort[ii][portNum];
        rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 9, &value);
        if(GT_OK != rc)
        {
            return rc;
        }
        portKeyPtr->portUdbArr[ii].udbByteOffset =
            U32_GET_FIELD_MAC(value, 2, 7);
        PRV_CPSS_PX_PTP_HW_TO_ANCHOR_MAC(U32_GET_FIELD_MAC(value, 0, 2),
            portKeyPtr->portUdbArr[ii].udbAnchorType);
    }

    return GT_OK;
}

/**
* @internal cpssPxPtpPortTypeKeyGet function
* @endinternal
*
* @brief   Get port PTP type key info.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical port number.
*
* @param[out] portKeyPtr               - (pointer to) port PTP type key.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpPortTypeKeyGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  GT_PHYSICAL_PORT_NUM                    portNum,
    OUT CPSS_PX_PTP_TYPE_KEY_STC    *portKeyPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpPortTypeKeyGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portKeyPtr));

    rc = internal_cpssPxPtpPortTypeKeyGet(devNum, portNum, portKeyPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portKeyPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTypeKeyEntrySet function
* @endinternal
*
* @brief   Set PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] keyDataPtr               - (pointer to) PTP type key data.
* @param[in] keyMaskPtr               - (pointer to) PTP type key mask.
*                                      The PTP type key mask is AND styled one.
*                                      Mask bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM like lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM like lookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTypeKeyEntrySet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX                  ptpTypeIndex,
    IN  CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyDataPtr,
    IN  CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyMaskPtr
)
{
    GT_U32      ii;             /* iterator */
    GT_STATUS   rc;             /* return value */
    GT_U32      regDataAddr;    /* register data address */
    GT_U32      regDataValue;   /* register data value */
    GT_U32      regMaskAddr;    /* register mask address */
    GT_U32      regMaskValue;   /* register mask value */
    GT_U32      regWriteBits;   /* number of bits to be written to register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(ptpTypeIndex, CPSS_PX_PTP_TYPE_NUM_CNS);
    CPSS_NULL_PTR_CHECK_MAC(keyDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(keyMaskPtr);
    CPSS_DATA_CHECK_MAX_MAC(keyDataPtr->srcPortProfile,
        CPSS_PX_INGRESS_SOURCE_PORT_PROFILE_MAX_CNS);
    CPSS_DATA_CHECK_MAX_MAC(keyMaskPtr->srcPortProfile,
        CPSS_PX_INGRESS_SOURCE_PORT_PROFILE_MAX_CNS);

    regDataValue = 0;
    regMaskValue = 0;

    regDataAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
        PCPPTPTypeKeyTableUDPAndPort_profileEntry[ptpTypeIndex];

    U32_SET_FIELD_MAC(regDataValue, 7, 1, (keyDataPtr->isUdp == GT_TRUE) ? 1 : 0);
    U32_SET_FIELD_MAC(regDataValue, 0, 7, keyDataPtr->srcPortProfile);

    regMaskAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
        PCPPTPTypeKeyTableUDPAndPortProfileEntryMask[ptpTypeIndex];

    U32_SET_FIELD_MAC(regMaskValue, 7, 1, (keyMaskPtr->isUdp == GT_TRUE) ? 1 : 0);
    U32_SET_FIELD_MAC(regMaskValue, 0, 7, keyMaskPtr->srcPortProfile);

    regWriteBits = 8;
    rc = prvCpssPxIngressPacketTypeRegisterWrite(devNum,
                                       regDataAddr, regMaskAddr,
                                       regDataValue, regMaskValue,
                                       regWriteBits);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; ii++)
    {
        regDataAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPTPTypeKeyTableUDBPEntry[ii][ptpTypeIndex];

        U32_SET_FIELD_MAC(regDataValue, 8, 8, keyDataPtr->udbPairsArr[ii].udb[0]);
        U32_SET_FIELD_MAC(regDataValue, 0, 8, keyDataPtr->udbPairsArr[ii].udb[1]);

        regMaskAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPTPTypeKeyTableUDBPEntryMask[ii][ptpTypeIndex];

        U32_SET_FIELD_MAC(regMaskValue, 8, 8, keyMaskPtr->udbPairsArr[ii].udb[0]);
        U32_SET_FIELD_MAC(regMaskValue, 0, 8, keyMaskPtr->udbPairsArr[ii].udb[1]);

        regWriteBits = 16;
        rc = prvCpssPxIngressPacketTypeRegisterWrite(devNum,
                                           regDataAddr, regMaskAddr,
                                           regDataValue, regMaskValue,
                                           regWriteBits);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; ii++)
    {
        regDataAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPTPTypeKeyTableUDBEntry[ii][ptpTypeIndex];

        regDataValue = keyDataPtr->udbArr[ii];

        regMaskAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPTPTypeKeyTableUDBEntryMask[ii][ptpTypeIndex];

        regMaskValue = keyMaskPtr->udbArr[ii];

        regWriteBits = 8;
        rc = prvCpssPxIngressPacketTypeRegisterWrite(devNum,
                                           regDataAddr, regMaskAddr,
                                           regDataValue, regMaskValue,
                                           regWriteBits);
    }

    return GT_OK;
}

/**
* @internal cpssPxPtpTypeKeyEntrySet function
* @endinternal
*
* @brief   Set PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] keyDataPtr               - (pointer to) PTP type key data.
* @param[in] keyMaskPtr               - (pointer to) PTP type key mask.
*                                      The PTP type key mask is AND styled one.
*                                      Mask bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM like lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM like lookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTypeKeyEntrySet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX                  ptpTypeIndex,
    IN  CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyDataPtr,
    IN  CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyMaskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTypeKeyEntrySet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ptpTypeIndex, keyDataPtr, keyMaskPtr));

    rc = internal_cpssPxPtpTypeKeyEntrySet(devNum, ptpTypeIndex, keyDataPtr, keyMaskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ptpTypeIndex, keyDataPtr, keyMaskPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTypeKeyEntryGet function
* @endinternal
*
* @brief   Get PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] keyDataPtr               - (pointer to) PTP type key data.
* @param[out] keyMaskPtr               - (pointer to) PTP type key mask.
*                                      The PTP type key mask is AND styled one.
*                                      Mask bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM like lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM like lookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTypeKeyEntryGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX                  ptpTypeIndex,
    OUT CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyDataPtr,
    OUT CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyMaskPtr
)
{
    GT_U32      ii;             /* iterator */
    GT_STATUS   rc;             /* return value */
    GT_U32  regDataAddr;
    GT_U32  regDataValue;
    GT_U32  regMaskAddr;
    GT_U32  regMaskValue;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(ptpTypeIndex, CPSS_PX_PTP_TYPE_NUM_CNS);
    CPSS_NULL_PTR_CHECK_MAC(keyDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(keyMaskPtr);

    regDataAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
        PCPPTPTypeKeyTableUDPAndPort_profileEntry[ptpTypeIndex];
    regMaskAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
        PCPPTPTypeKeyTableUDPAndPortProfileEntryMask[ptpTypeIndex];

    rc = prvCpssPxIngressPacketTypeRegisterRead(devNum,regDataAddr, regMaskAddr,
                                      &regDataValue, &regMaskValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    keyDataPtr->isUdp = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regDataValue, 7, 1));
    keyDataPtr->srcPortProfile = U32_GET_FIELD_MAC(regDataValue, 0, 7);

    keyMaskPtr->isUdp = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regMaskValue, 7, 1));
    keyMaskPtr->srcPortProfile = U32_GET_FIELD_MAC(regMaskValue, 0, 7);

    for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDBP_NUM_CNS; ii++)
    {
        regDataAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPTPTypeKeyTableUDBPEntry[ii][ptpTypeIndex];
        regMaskAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPTPTypeKeyTableUDBPEntryMask[ii][ptpTypeIndex];

        rc = prvCpssPxIngressPacketTypeRegisterRead(devNum,regDataAddr, regMaskAddr,
                                          &regDataValue, &regMaskValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        keyDataPtr->udbPairsArr[ii].udb[0] = U32_GET_FIELD_MAC(regDataValue, 8, 8);
        keyDataPtr->udbPairsArr[ii].udb[1] = U32_GET_FIELD_MAC(regDataValue, 0, 8);

        keyMaskPtr->udbPairsArr[ii].udb[0] = U32_GET_FIELD_MAC(regMaskValue, 8, 8);
        keyMaskPtr->udbPairsArr[ii].udb[1] = U32_GET_FIELD_MAC(regMaskValue, 0, 8);
    }

    for (ii = 0; ii < CPSS_PX_PTP_TYPE_UDB_NUM_CNS; ii++)
    {
        regDataAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPTPTypeKeyTableUDBEntry[ii][ptpTypeIndex];
        regMaskAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
            PCPPTPTypeKeyTableUDBEntryMask[ii][ptpTypeIndex];

        rc = prvCpssPxIngressPacketTypeRegisterRead(devNum,regDataAddr, regMaskAddr,
                                          &regDataValue, &regMaskValue);
        if (rc != GT_OK)
        {
            return rc;
        }

        keyDataPtr->udbArr[ii] = (GT_U8)(regDataValue & 0xFF);
        keyMaskPtr->udbArr[ii] = (GT_U8)(regMaskValue & 0xFF);
    }

    return GT_OK;
}

/**
* @internal cpssPxPtpTypeKeyEntryGet function
* @endinternal
*
* @brief   Get PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] keyDataPtr               - (pointer to) PTP type key data.
* @param[out] keyMaskPtr               - (pointer to) PTP type key mask.
*                                      The PTP type key mask is AND styled one.
*                                      Mask bit's 0 means don't care bit (corresponding bit in
*                                      the pattern is not using in the TCAM like lookup).
*                                      Mask bit's 1 means that corresponding bit in the
*                                      pattern is using in the TCAM like lookup.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTypeKeyEntryGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX                  ptpTypeIndex,
    OUT CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyDataPtr,
    OUT CPSS_PX_PTP_TYPE_KEY_FORMAT_STC         *keyMaskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTypeKeyEntryGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ptpTypeIndex, keyDataPtr, keyMaskPtr));

    rc = internal_cpssPxPtpTypeKeyEntryGet(devNum, ptpTypeIndex, keyDataPtr, keyMaskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ptpTypeIndex, keyDataPtr, keyMaskPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTypeKeyEntryEnableSet function
* @endinternal
*
* @brief   Enable/disable PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] enable                   - Enable or disable PTP type key table entry
*                                      GT_TRUE - enable PTP type key table entry,
*                                      GT_FALSE - disable PTP type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTypeKeyEntryEnableSet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX ptpTypeIndex,
    IN  GT_BOOL                enable
)
{
    GT_U32 regAddr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(ptpTypeIndex, CPSS_PX_PTP_TYPE_NUM_CNS);

    regAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
        PCPPTPTypeKeyTableEntriesEnable;
    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, ptpTypeIndex, 1,
        BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxPtpTypeKeyEntryEnableSet function
* @endinternal
*
* @brief   Enable/disable PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
* @param[in] enable                   - Enable or disable PTP type key table entry
*                                      GT_TRUE - enable PTP type key table entry,
*                                      GT_FALSE - disable PTP type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTypeKeyEntryEnableSet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX ptpTypeIndex,
    IN  GT_BOOL                enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTypeKeyEntryEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ptpTypeIndex, enable));

    rc = internal_cpssPxPtpTypeKeyEntryEnableSet(devNum, ptpTypeIndex, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ptpTypeIndex, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTypeKeyEntryEnableGet function
* @endinternal
*
* @brief   Get enabled state of PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] enablePtr                - (pointer to) Enable or disable PTP type key table entry
*                                      GT_TRUE - enable PTP type key table entry,
*                                      GT_FALSE - disable PTP type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTypeKeyEntryEnableGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX ptpTypeIndex,
    OUT GT_BOOL                *enablePtr
)
{
    GT_U32 regAddr; /* register address */
    GT_U32 value;   /* register value */
    GT_STATUS rc;   /* return status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(ptpTypeIndex, CPSS_PX_PTP_TYPE_NUM_CNS);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(ptpTypeIndex, CPSS_PX_PTP_TYPE_NUM_CNS);

    regAddr = PRV_PX_REG1_UNIT_PCP_PTP_MAC(devNum).
        PCPPTPTypeKeyTableEntriesEnable;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, ptpTypeIndex, 1, &value);
    *enablePtr = BIT2BOOL_MAC(value);
    return rc;
}

/**
* @internal cpssPxPtpTypeKeyEntryEnableGet function
* @endinternal
*
* @brief   Get enabled state of PTP type key table entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] ptpTypeIndex             - index of the PTP type key in the table.
*                                      (APPLICABLE RANGES: 0..31)
*
* @param[out] enablePtr                - (pointer to) Enable or disable PTP type key table entry
*                                      GT_TRUE - enable PTP type key table entry,
*                                      GT_FALSE - disable PTP type key table entry.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTypeKeyEntryEnableGet
(
    IN  GT_SW_DEV_NUM          devNum,
    IN  CPSS_PX_PTP_TYPE_INDEX ptpTypeIndex,
    OUT GT_BOOL                *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTypeKeyEntryEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ptpTypeIndex, enablePtr));

    rc = internal_cpssPxPtpTypeKeyEntryEnableGet(devNum, ptpTypeIndex, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ptpTypeIndex, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiPClockOutputEnableSet function
* @endinternal
*
* @brief   Set PTP PCLK OUT as clock source for RCVR_CLOCK_OUT[1] pin enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   -  or disable PTP PCLK OUT clock source for
*                                      RCVR_CLOCK_OUT[1] pin:
*                                      GT_TRUE  - enable PTP PCLK OUT as clock source for
*                                      RCVR_CLOCK_OUT[1] pin.
*                                      GT_FALSE - disable PTP PCLK OUT as clock source for
*                                      RCVR_CLOCK_OUT[1] pin.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiPClockOutputEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_BOOL       enable
)
{
    GT_U32 regAddr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
        DFXServerUnitsDeviceSpecificRegs.deviceCtrl20;
    return prvCpssDrvHwPpResetAndInitControllerSetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, 23, 1, BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxPtpTaiPClockOutputEnableSet function
* @endinternal
*
* @brief   Set PTP PCLK OUT as clock source for RCVR_CLOCK_OUT[1] pin enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   -  or disable PTP PCLK OUT clock source for
*                                      RCVR_CLOCK_OUT[1] pin:
*                                      GT_TRUE  - enable PTP PCLK OUT as clock source for
*                                      RCVR_CLOCK_OUT[1] pin.
*                                      GT_FALSE - disable PTP PCLK OUT as clock source for
*                                      RCVR_CLOCK_OUT[1] pin.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPClockOutputEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_BOOL       enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiPClockOutputEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPtpTaiPClockOutputEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiPClockOutputEnableGet function
* @endinternal
*
* @brief   Get PTP PCLK OUT as clock source for RCVR_CLOCK_OUT[1] pin enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enablePtr                - (pointer to) PTP PCLK OUT as clock source for
*                                      RCVR_CLOCK_OUT[1] pin enabled state.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiPClockOutputEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_BOOL       *enablePtr
)
{
    GT_U32 regAddr; /* register address */
    GT_U32 value;   /* register value */
    GT_STATUS rc;   /* return status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
        DFXServerUnitsDeviceSpecificRegs.deviceCtrl20;
    rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(
        CAST_SW_DEVNUM(devNum), regAddr, 23, 1, &value);
    *enablePtr = BIT2BOOL_MAC(value);
    return rc;
}

/**
* @internal cpssPxPtpTaiPClockOutputEnableGet function
* @endinternal
*
* @brief   Get PTP PCLK OUT as clock source for RCVR_CLOCK_OUT[1] pin enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enablePtr                - (pointer to) PTP PCLK OUT as clock source for
*                                      RCVR_CLOCK_OUT[1] pin enabled state.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPClockOutputEnableGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT GT_BOOL       *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiPClockOutputEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPtpTaiPClockOutputEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiPhaseUpdateEnableSet function
* @endinternal
*
* @brief   Set configuration of the PPS rising edge phase enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - configuration of the PPS rising edge phase enable.
*                                      GT_TRUE  - configuration of the PPS rising edge phase
* @param[in] enable- configuration of the PPS rising edge phase enable.
*                                      GT_FALSE - configuration of the PPS rising edge phase
*                                      disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiPhaseUpdateEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_BOOL       enable
)
{
    GT_U32 regAddr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 8, 1,
        BOOL2BIT_MAC(enable));
}

/**
* @internal cpssPxPtpTaiPhaseUpdateEnableSet function
* @endinternal
*
* @brief   Set configuration of the PPS rising edge phase enable.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - configuration of the PPS rising edge phase enable.
*                                      GT_TRUE  - configuration of the PPS rising edge phase
* @param[in] enable- configuration of the PPS rising edge phase enable.
*                                      GT_FALSE - configuration of the PPS rising edge phase
*                                      disable.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPhaseUpdateEnableSet
(
    IN  GT_SW_DEV_NUM devNum,
    IN  GT_BOOL       enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiPhaseUpdateEnableSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPtpTaiPhaseUpdateEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiPhaseUpdateEnableGet function
* @endinternal
*
* @brief   Get configuration of the PPS rising edge phase enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) configuration of the PPS rising edge
*                                      phase enabled state.
*                                      GT_TRUE  - configuration of the PPS rising edge phase
*                                      enabled.
*                                      GT_FALSE - configuration of the PPS rising edge phase
*                                      disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPtpTaiPhaseUpdateEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_U32 regAddr; /* register address */
    GT_U32 value;   /* register value */
    GT_STATUS rc;   /* return status */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_PX_REG1_UNIT_GOP_TAI_MAC(devNum).timeCntrFunctionConfig0;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 8, 1, &value);
    *enablePtr = BIT2BOOL_MAC(value);
    return rc;
}

/**
* @internal cpssPxPtpTaiPhaseUpdateEnableGet function
* @endinternal
*
* @brief   Get configuration of the PPS rising edge phase enabled state.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) configuration of the PPS rising edge
*                                      phase enabled state.
*                                      GT_TRUE  - configuration of the PPS rising edge phase
*                                      enabled.
*                                      GT_FALSE - configuration of the PPS rising edge phase
*                                      disabled.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPtpTaiPhaseUpdateEnableGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_BOOL         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiPhaseUpdateEnableGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPtpTaiPhaseUpdateEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiPulseInModeSet function
* @endinternal
*
* @brief  Set TAI PTP pulse IN mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[in] pulseMode             - TAI PulseIN signal mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssPxPtpTaiPulseInModeSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT       pulseMode
)
{
    GT_U32    regData;                          /* register data                */
    GT_U32    pulseInFieldValue;                /* register field value         */
    GT_U32    pulseMuxFieldValue;               /* register field value         */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    /* Decide the Field value of Mux and PulseIn according to the pulseMode */
    switch(pulseMode)
    {
        case CPSS_PX_PTP_TAI_PULSE_IN_DISABLED_E:
            pulseInFieldValue = 0;
            pulseMuxFieldValue = 0;
            break;
        case CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E:
            pulseInFieldValue = 1;
            pulseMuxFieldValue = 0;
            break;
        case CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E:
            pulseInFieldValue = 1;
            pulseMuxFieldValue = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regData = pulseInFieldValue | pulseMuxFieldValue << 1;

    /* Set bit [1] Rf TAI Pulse In Enable, bit [2] Rf TAI Pulse In Mux Enable */
    return prvCpssPxPtpTaiControlFieldSet(devNum, 1, 2, regData);
}

/**
* @internal cpssPxPtpTaiPulseInModeSet function
* @endinternal
*
* @brief  Set TAI PTP pulse IN mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[in] pulseMode             - TAI PulseIN signal mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssPxPtpTaiPulseInModeSet
(
    IN  GT_SW_DEV_NUM                           devNum,
    IN  CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT       pulseMode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiPulseInModeSet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pulseMode));

    rc = internal_cpssPxPtpTaiPulseInModeSet(devNum, pulseMode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pulseMode));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPtpTaiPulseInModeGet function
* @endinternal
*
* @brief  Get TAI PTP pulse IN mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[out] pulseMode            - (pointer to) TAI PulseIN signal mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on bad state
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssPxPtpTaiPulseInModeGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT       *pulseModePtr
)
{
    GT_U32    regData;              /* register data                */
    GT_STATUS rc;                   /* return code                  */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        /* current GM doesn't support DFX unit so far */
        return GT_OK;
    }

    CPSS_NULL_PTR_CHECK_MAC(pulseModePtr);

    /* Get bit [1] Rf TAI Pulse In Enable, bit [2] Rf TAI Pulse In Mux Enable */
    rc = prvCpssPxPtpTaiControlFieldGet(devNum, 1, 2, &regData);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch(regData)
    {
        case 0:
            *pulseModePtr = CPSS_PX_PTP_TAI_PULSE_IN_DISABLED_E;
            break;
        case 1:
            *pulseModePtr = CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_PULSE_IN_E;
            break;
        case 3:
            *pulseModePtr = CPSS_PX_PTP_TAI_PULSE_IN_IS_PTP_CLOCK_IN_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPtpTaiPulseInModeGet function
* @endinternal
*
* @brief  Get TAI PTP pulse IN mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number.
* @param[out] pulseMode            - (pointer to) TAI PulseIN signal mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_STATE             - on bad state
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssPxPtpTaiPulseInModeGet
(
    IN  GT_SW_DEV_NUM                           devNum,
    OUT CPSS_PX_PTP_TAI_PULSE_IN_MODE_ENT       *pulseModePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPtpTaiPulseInModeGet);

    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pulseModePtr));

    rc = internal_cpssPxPtpTaiPulseInModeGet(devNum, pulseModePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pulseModePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

