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
* @file cpssPxPortCn.c
*
* @brief CPSS implementation for Pipe Port Congestion Notification API.
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
#include <cpss/px/port/cpssPxPortCn.h>


/**
* @internal internal_cpssPxPortCnModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable Congestion Notification (CN) mechanism.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - Enable/disable congestion notification mechanism.
*                                      GT_TRUE    - Enable congestion notification mechanism.
*                                      GT_FALSE   - Disable congestion notification mechanism.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnModeEnableSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_BOOL                  enable
)
{
    GT_U32    regAddr;              /* register address */
    GT_U32    regVal;               /* register field value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    regVal = BOOL2BIT_MAC(enable);

    /* <Global Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              globalTailDropConfig;
    /* Set <CN Enable> field of <Global Tail Drop Configuration> register */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 10, 1, regVal);
}

/**
* @internal cpssPxPortCnModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable Congestion Notification (CN) mechanism.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - Enable/disable congestion notification mechanism.
*                                      GT_TRUE    - Enable congestion notification mechanism.
*                                      GT_FALSE   - Disable congestion notification mechanism.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnModeEnableSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssPxPortCnModeEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnModeEnableGet function
* @endinternal
*
* @brief   Get the current status of Congestion Notification (CN) mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of congestion notification mechanism
*                                      GT_TRUE    - congestion notification mechanism is enabled.
*                                      GT_FALSE   - congestion notification mechanism is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnModeEnableGet
(
    IN GT_SW_DEV_NUM            devNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;                   /* return code */
    GT_U32    regAddr;              /* register address */
    GT_U32    regVal;             /* register field value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* <Global Tail Drop Configuration> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
              globalTailDropConfig;

    /* Get <CN Enable> field of <Global Tail Drop Configuration> register */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 10, 1, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regVal);

    return GT_OK;
}

/**
* @internal cpssPxPortCnModeEnableGet function
* @endinternal
*
* @brief   Get the current status of Congestion Notification (CN) mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of congestion notification mechanism
*                                      GT_TRUE    - congestion notification mechanism is enabled.
*                                      GT_FALSE   - congestion notification mechanism is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnModeEnableGet
(
    IN GT_SW_DEV_NUM            devNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssPxPortCnModeEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnPacketLengthSet function
* @endinternal
*
* @brief   Set packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetLength             - packet length.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or packet length.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnPacketLengthSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN CPSS_PX_PORT_CN_PACKET_LENGTH_ENT    packetLength
)
{
    GT_U32 regVal;            /* register value */
    GT_U32 regAddr;             /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch (packetLength)
    {
        case CPSS_PX_PORT_CN_LENGTH_ORIG_PACKET_E:
            regVal = 0;
            break;
        case CPSS_PX_PORT_CN_LENGTH_1_5_KB_E:
            regVal = 1;
            break;
        case CPSS_PX_PORT_CN_LENGTH_2_KB_E:
            regVal = 2;
            break;
        case CPSS_PX_PORT_CN_LENGTH_10_KB_E:
            regVal = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* <CN Global Configuration Register> register */
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->TXQ.qcn.CNGlobalConfig;

    /* Set <CN Packet Length> field */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 2, regVal);
}

/**
* @internal cpssPxPortCnPacketLengthSet function
* @endinternal
*
* @brief   Set packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] packetLength             - packet length.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number or packet length.
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnPacketLengthSet
(
    IN GT_SW_DEV_NUM                        devNum,
    IN CPSS_PX_PORT_CN_PACKET_LENGTH_ENT    packetLength
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnPacketLengthSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetLength));

    rc = internal_cpssPxPortCnPacketLengthSet(devNum, packetLength);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetLength));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnPacketLengthGet function
* @endinternal
*
* @brief   Get packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] packetLengthPtr          - (pointer to) packet length.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnPacketLengthGet
(
    IN GT_SW_DEV_NUM                        devNum,
    OUT CPSS_PX_PORT_CN_PACKET_LENGTH_ENT  *packetLengthPtr
)
{
    GT_STATUS rc;                   /* return code */
    GT_U32    regAddr;              /* register address */
    GT_U32    regVal;               /* register field value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(packetLengthPtr);

    /* <CN Global Configuration Register> register */
    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->TXQ.qcn.CNGlobalConfig;

    /* Get <CN Packet Length> field */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 2, 2, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }
    *packetLengthPtr = (CPSS_PX_PORT_CN_PACKET_LENGTH_ENT)regVal;

    return GT_OK;

}

/**
* @internal cpssPxPortCnPacketLengthGet function
* @endinternal
*
* @brief   Get packet length used by the CN frames triggering logic.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] packetLengthPtr          - (pointer to) packet length.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnPacketLengthGet
(
    IN GT_SW_DEV_NUM                        devNum,
    OUT CPSS_PX_PORT_CN_PACKET_LENGTH_ENT  *packetLengthPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnPacketLengthGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, packetLengthPtr));

    rc = internal_cpssPxPortCnPacketLengthGet(devNum, packetLengthPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, packetLengthPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnSampleEntrySet function
* @endinternal
*
* @brief   Sets CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
* @param[in] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.To implement CCFC functionality set entryPtr fields to:
*       interval = (MTU+256b)/16
*       randBitmap = 0
*       2.To achieve uniform distribution of random values,
*       set the Interval LSbits to be randomized to 1.
*       For example, if Interval = 0xF0 and RndRange = 0x0F
*       then the Sampling Interval is a random value in [0xF0, 0xFF].
*
*/
static GT_STATUS internal_cpssPxPortCnSampleEntrySet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U8                    entryIndex,
    IN CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
{
    GT_U32 regVal = 0;      /* register value */
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    CPSS_PARAM_CHECK_MAX_MAC(entryIndex, BIT_3);
    CPSS_DATA_CHECK_MAX_MAC(entryPtr->interval, BIT_16);
    CPSS_DATA_CHECK_MAX_MAC(entryPtr->randBitmap, BIT_16);

    U32_SET_FIELD_MAC(regVal, 0, 16, entryPtr->interval);
    U32_SET_FIELD_MAC(regVal, 16, 16, entryPtr->randBitmap);

    /* <CN Sample Tbl <%n>> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).CNSampleTbl[entryIndex];
    return prvCpssHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, regVal);
}

/**
* @internal cpssPxPortCnSampleEntrySet function
* @endinternal
*
* @brief   Set CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
* @param[in] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1.To implement CCFC functionality set entryPtr fields to:
*       interval = (MTU+256b)/16
*       randBitmap = 0
*       2.To achieve uniform distribution of random values,
*       set the Interval LSbits to be randomized to 1.
*       For example, if Interval = 0xF0 and RndRange = 0x0F
*       then the Sampling Interval is a random value in [0xF0, 0xFF].
*
*/
GT_STATUS cpssPxPortCnSampleEntrySet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U8                    entryIndex,
    IN CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnSampleEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, entryPtr));

    rc = internal_cpssPxPortCnSampleEntrySet(devNum, entryIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnSampleEntryGet function
* @endinternal
*
* @brief   Get CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
*
* @param[out] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnSampleEntryGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_U8                   entryIndex,
    OUT CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
{
    GT_U32 regVal;        /* register value */
    GT_U32 regAddr;         /* register address */
    GT_STATUS rc;           /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);
    CPSS_PARAM_CHECK_MAX_MAC(entryIndex, BIT_3);

    /* <CN Sample Tbl <%n>> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).CNSampleTbl[entryIndex];

    /* Get <Interval<%n>>, <RndRange<%n>> fields */
    rc = prvCpssHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }
    entryPtr->interval = U32_GET_FIELD_MAC(regVal, 0, 16); 
    entryPtr->randBitmap = U32_GET_FIELD_MAC(regVal, 16, 16); 

    return GT_OK;
}

/**
* @internal cpssPxPortCnSampleEntryGet function
* @endinternal
*
* @brief   Get CN sample interval entry.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] entryIndex               - index: qFb 3 msb. (0..7)
*
* @param[out] entryPtr                 - (pointer to) entry settings.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number, or entry index
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnSampleEntryGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN  GT_U8                   entryIndex,
    OUT CPSS_PX_PORT_CN_SAMPLE_INTERVAL_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnSampleEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, entryIndex, entryPtr));

    rc = internal_cpssPxPortCnSampleEntryGet(devNum, entryIndex, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, entryIndex, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnFbCalcConfigSet function
* @endinternal
*
* @brief   Set Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnFbCalcConfigSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
{
    GT_U32 regVal = 0;          /* register regVal */
    GT_U32 wExpValue;           /* wExp hw regVal */
    GT_U32 regAddr;             /* register address */
    GT_STATUS rc;               /* function return regVal */
    GT_U32 fbMinMaxNumOfBits;   /* number of bits in fbMin and fbMax fields */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(fbCalcCfgPtr);

    fbMinMaxNumOfBits = 24;
    CPSS_DATA_CHECK_MAX_MAC(fbCalcCfgPtr->fbMin, BIT_24);
    CPSS_DATA_CHECK_MAX_MAC(fbCalcCfgPtr->fbMax, BIT_24);
    CPSS_DATA_CHECK_MAX_MAC(fbCalcCfgPtr->fbLsb, BIT_5);

    if (fbCalcCfgPtr->wExp > 7 || fbCalcCfgPtr->wExp < -8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* Wexp is given in 2s complement format */
    if (fbCalcCfgPtr->wExp >= 0)
    {
        wExpValue = (GT_U32)fbCalcCfgPtr->wExp;
    }
    else
    {
        wExpValue =(GT_U32)(16 + fbCalcCfgPtr->wExp);
    }

    /* Set Feedback Calc Configurations register */
    U32_SET_FIELD_MAC(regVal, 0, 1, BOOL2BIT_MAC(fbCalcCfgPtr->deltaEnable));
    U32_SET_FIELD_MAC(regVal, 1, 5, fbCalcCfgPtr->fbLsb);
    U32_SET_FIELD_MAC(regVal, 6, 4, wExpValue);

    /* <Feedback Calc Configurations> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackCalcConfigs;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 10, regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* <Feedback MIN> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackMIN;
    /* Set <FbMin> field */
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, fbMinMaxNumOfBits, fbCalcCfgPtr->fbMin);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* <Feedback MAX> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackMAX;
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, fbMinMaxNumOfBits, fbCalcCfgPtr->fbMax);
}

/**
* @internal cpssPxPortCnFbCalcConfigSet function
* @endinternal
*
* @brief   Set Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnFbCalcConfigSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnFbCalcConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fbCalcCfgPtr));

    rc = internal_cpssPxPortCnFbCalcConfigSet(devNum, fbCalcCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fbCalcCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnFbCalcConfigGet function
* @endinternal
*
* @brief   Get Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnFbCalcConfigGet
(
    IN GT_SW_DEV_NUM            devNum,
    OUT CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
{
    GT_U32 regVal;              /* register value */
    GT_U32 wExpValue;           /* wExp hw value */
    GT_U32 regAddr;             /* register address */
    GT_STATUS rc;               /* function return status */
    GT_U32 fbMinMaxNumOfBits;   /* number of bits in fbMin and fbMax fields */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(fbCalcCfgPtr);


    fbMinMaxNumOfBits = 24;

    /* <Feedback Calc Configurations> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackCalcConfigs;
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 10, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }
    fbCalcCfgPtr->deltaEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regVal, 0, 1));
    fbCalcCfgPtr->fbLsb = U32_GET_FIELD_MAC(regVal, 1, 5);

    /* Wexp is given in 2s complement format */
    wExpValue = U32_GET_FIELD_MAC(regVal, 6, 4);
    if (wExpValue < 8)
    {
        fbCalcCfgPtr->wExp = wExpValue;
    }
    else
    {
        fbCalcCfgPtr->wExp = wExpValue - 16;
    }


    /* <Feedback MIN> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackMIN;
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, fbMinMaxNumOfBits, &(fbCalcCfgPtr->fbMin));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* <Feedback MAX> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).feedbackMAX;
    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, fbMinMaxNumOfBits, &(fbCalcCfgPtr->fbMax));
}

/**
* @internal cpssPxPortCnFbCalcConfigGet function
* @endinternal
*
* @brief   Get Fb (Feedback) calculation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] fbCalcCfgPtr             - (pointer to) feedback Calculation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnFbCalcConfigGet
(
    IN GT_SW_DEV_NUM            devNum,
    OUT CPSS_PX_PORT_CN_FB_CALCULATION_CONFIG_STC *fbCalcCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnFbCalcConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, fbCalcCfgPtr));

    rc = internal_cpssPxPortCnFbCalcConfigGet(devNum, fbCalcCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, fbCalcCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnMessageTriggeringStateSet function
* @endinternal
*
* @brief   Set CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] qcnSampleInterval        -  parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
* @param[in] qlenOld                  - a snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range values
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
static GT_STATUS internal_cpssPxPortCnMessageTriggeringStateSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    tcQueue,
    IN GT_U32                   qcnSampleInterval,
    IN GT_U32                   qlenOld
)
{
    GT_U32                  txqPortNum;     /* TXQ port number */
    GT_U32                  hwArray[2];     /* table HW data */
    GT_U32                  tableIndex;     /* table index */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_DATA_CHECK_MAX_MAC(qcnSampleInterval, BIT_20);
    CPSS_DATA_CHECK_MAX_MAC(qlenOld, BIT_20);


    U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 0, 20, qcnSampleInterval);
    U32_SET_FIELD_IN_ENTRY_MAC(hwArray, 20, 20, qlenOld);

    tableIndex = tcQueue | (txqPortNum << 3);

    /* <CN Sample Intervals> Table */
    return prvCpssPxWriteTableEntry(devNum, 
                                    CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E, 
                                    tableIndex, hwArray);
}

/**
* @internal cpssPxPortCnMessageTriggeringStateSet function
* @endinternal
*
* @brief   Set CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] qcnSampleInterval        -  parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
* @param[in] qlenOld                  - a snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*                                      (APPLICABLE RANGES: 0..0xFFFFF)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range values
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
GT_STATUS cpssPxPortCnMessageTriggeringStateSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U8                    tcQueue,
    IN GT_U32                   qcnSampleInterval,
    IN GT_U32                   qlenOld
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnMessageTriggeringStateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, qcnSampleInterval, qlenOld));

    rc = internal_cpssPxPortCnMessageTriggeringStateSet(devNum, portNum, tcQueue, qcnSampleInterval, qlenOld);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, qcnSampleInterval, qlenOld));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnMessageTriggeringStateGet function
* @endinternal
*
* @brief   Get CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] qcnSampleIntervalPtr     -  (pointer to) parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
* @param[out] qlenOldPtr               - (pointer to) snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
static GT_STATUS internal_cpssPxPortCnMessageTriggeringStateGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    OUT GT_U32                  *qcnSampleIntervalPtr,
    OUT GT_U32                  *qlenOldPtr
)
{
    GT_STATUS               rc;             /* return code */
    GT_U32                  txqPortNum;     /* TXQ port number */
    GT_U32                  hwArray[2];     /* table HW data */
    GT_U32                  tableIndex;     /* table index */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_TXQ_PORT_GET_MAC(devNum, portNum, txqPortNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(qcnSampleIntervalPtr);
    CPSS_NULL_PTR_CHECK_MAC(qlenOldPtr);

    tableIndex = tcQueue | (txqPortNum << 3);
    /* <CN Sample Intervals> Table */
    rc = prvCpssPxReadTableEntry(devNum, 
                                 CPSS_PX_TABLE_TXQ_QCN_CN_SAMPLE_INTERVALS_E,
                                 tableIndex, hwArray);
    if(GT_OK != rc)
    {
        return rc;
    }

    U32_GET_FIELD_IN_ENTRY_MAC(hwArray, 0, 20, *qcnSampleIntervalPtr);
    U32_GET_FIELD_IN_ENTRY_MAC(hwArray, 20, 20, *qlenOldPtr);

    return GT_OK;
}

/**
* @internal cpssPxPortCnMessageTriggeringStateGet function
* @endinternal
*
* @brief   Get CNM Triggering State Variables
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] qcnSampleIntervalPtr     -  (pointer to) parameter that defines the probability to trigger
*                                      the next CN frame based on the current value of the
*                                      calculated feedback (Fb), measured in Bytes.
* @param[out] qlenOldPtr               - (pointer to) snapshot of current length of the queue in buffers,
*                                      taken when the QcnSampleInterval expires.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note qlenOld reset value and QcnSampleInterval init value must be set by the
*       application prior to configuring the queue to be CN aware
*
*/
GT_STATUS cpssPxPortCnMessageTriggeringStateGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   tcQueue,
    OUT GT_U32                  *qcnSampleIntervalPtr,
    OUT GT_U32                  *qlenOldPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnMessageTriggeringStateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, tcQueue, qcnSampleIntervalPtr, qlenOldPtr));

    rc = internal_cpssPxPortCnMessageTriggeringStateGet(devNum, portNum, tcQueue, qcnSampleIntervalPtr, qlenOldPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, tcQueue, qcnSampleIntervalPtr, qlenOldPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnMessageGenerationConfigSet function
* @endinternal
*
* @brief   Set CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnMessageGenerationConfigSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
{
    GT_U32 regVal = 0;          /* register value */
    GT_U32 regAddr;             /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cnmGenerationCfgPtr);

    /* Check fields */
    CPSS_DATA_CHECK_MAX_MAC(cnmGenerationCfgPtr->packetType, CPSS_PX_PACKET_TYPE_MAX_CNS);
    CPSS_DATA_CHECK_MAX_MAC(cnmGenerationCfgPtr->priority, BIT_3);
    CPSS_DATA_CHECK_MAX_MAC(cnmGenerationCfgPtr->dropPrecedence, CPSS_DP_LAST_E);
    CPSS_DATA_CHECK_MAX_MAC(cnmGenerationCfgPtr->tc4pfc, BIT_3);
    CPSS_DATA_CHECK_MAX_MAC(cnmGenerationCfgPtr->localGeneratedPacketType, CPSS_PX_PACKET_TYPE_MAX_CNS);

    /* Set fields */
    U32_SET_FIELD_MAC(regVal, 0, 5, cnmGenerationCfgPtr->packetType);
    U32_SET_FIELD_MAC(regVal, 5, 3, cnmGenerationCfgPtr->priority);
    U32_SET_FIELD_MAC(regVal, 8, 2, cnmGenerationCfgPtr->dropPrecedence);
    U32_SET_FIELD_MAC(regVal, 10, 3, cnmGenerationCfgPtr->tc4pfc);
    U32_SET_FIELD_MAC(regVal, 13, 1, 
                      BOOL2BIT_MAC(cnmGenerationCfgPtr->localGeneratedPacketTypeAssignmentEnable));
    U32_SET_FIELD_MAC(regVal, 14, 5, cnmGenerationCfgPtr->localGeneratedPacketType);

    /* <QCN Global Config> register */
    regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).config.QCNGlobalConfig;
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 19, regVal);
}

/**
* @internal cpssPxPortCnMessageGenerationConfigSet function
* @endinternal
*
* @brief   Set CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnMessageGenerationConfigSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnMessageGenerationConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cnmGenerationCfgPtr));

    rc = internal_cpssPxPortCnMessageGenerationConfigSet(devNum, cnmGenerationCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cnmGenerationCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnMessageGenerationConfigGet function
* @endinternal
*
* @brief   Get CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnMessageGenerationConfigGet
(
    IN GT_SW_DEV_NUM            devNum,
    OUT CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
{
    GT_U32 regVal;              /* register value */
    GT_U32 regAddr;             /* register address */
    GT_STATUS rc;               /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cnmGenerationCfgPtr);

    cpssOsMemSet(cnmGenerationCfgPtr, 0, sizeof(CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC));

    /* <QCN Global Config> register */
    regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).config.QCNGlobalConfig;
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 19, &regVal);
    if(GT_OK != rc)
    {
        return rc;
    }

    cnmGenerationCfgPtr->packetType =   
                        U32_GET_FIELD_MAC(regVal, 0, 5);
    cnmGenerationCfgPtr->priority =     
                        U32_GET_FIELD_MAC(regVal, 5, 3);
    cnmGenerationCfgPtr->dropPrecedence = 
                        (CPSS_DP_LEVEL_ENT)U32_GET_FIELD_MAC(regVal, 8, 2);
    cnmGenerationCfgPtr->tc4pfc =       
                        U32_GET_FIELD_MAC(regVal, 10, 3);
    cnmGenerationCfgPtr->localGeneratedPacketTypeAssignmentEnable = 
                        BIT2BOOL_MAC((U32_GET_FIELD_MAC(regVal, 13, 1)));
    cnmGenerationCfgPtr->localGeneratedPacketType = 
                        U32_GET_FIELD_MAC(regVal, 14, 5);

    return GT_OK;

}

/**
* @internal cpssPxPortCnMessageGenerationConfigGet function
* @endinternal
*
* @brief   Get CNM generation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] cnmGenerationCfgPtr      - (pointer to) CNM generation configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnMessageGenerationConfigGet
(
    IN GT_SW_DEV_NUM            devNum,
    OUT CPSS_PX_PORT_CNM_GENERATION_CONFIG_STC *cnmGenerationCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnMessageGenerationConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, cnmGenerationCfgPtr));

    rc = internal_cpssPxPortCnMessageGenerationConfigGet(devNum, cnmGenerationCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, cnmGenerationCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnMessagePortMapEntrySet function
* @endinternal
*
* @brief   Set port map configuration per target port
*         for the triggered QCN incoming descriptor.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical target port number.
* @param[in] portsBmp                 - the bitmap of ports represent the target port number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnMessagePortMapEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORTS_BMP               portsBmp
)
{
    GT_U32  regAddr;            /* address of register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    PRV_CPSS_PX_PORT_BMP_CHECK_MAC(devNum, portsBmp);

    /* QCN MC Portmap For Port <<%n>> register */
    regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).config.QCNMCPortmapForPort[portNum];

    /* QCN MC Portmap Config field */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 17, portsBmp);
}

/**
* @internal cpssPxPortCnMessagePortMapEntrySet function
* @endinternal
*
* @brief   Set port map configuration per target port
*         for the triggered QCN incoming descriptor.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical target port number.
* @param[in] portsBmp                 - the bitmap of ports represent the target port number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnMessagePortMapEntrySet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  CPSS_PX_PORTS_BMP               portsBmp
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnMessagePortMapEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portsBmp));

    rc = internal_cpssPxPortCnMessagePortMapEntrySet(devNum, portNum, portsBmp);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portsBmp));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnMessagePortMapEntryGet function
* @endinternal
*
* @brief   Get port map configuration per target port
*         for the triggered QCN incoming descriptor.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical target port number.
*
* @param[out] portsBmpPtr              - (pointer to) the bitmap of ports represent the target port number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnMessagePortMapEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORTS_BMP               *portsBmpPtr
)
{
    GT_U32  regAddr;            /* address of register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(portsBmpPtr);

    /* <QCN MC Portmap For Port <<%n>>> register */
    regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).config.QCNMCPortmapForPort[portNum];

    /* QCN MC Portmap Config  field */
    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 17, portsBmpPtr);
}

/**
* @internal cpssPxPortCnMessagePortMapEntryGet function
* @endinternal
*
* @brief   Get port map configuration per target port
*         for the triggered QCN incoming descriptor.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - physical target port number.
*
* @param[out] portsBmpPtr              - (pointer to) the bitmap of ports represent the target port number.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnMessagePortMapEntryGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PX_PORTS_BMP               *portsBmpPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnMessagePortMapEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portsBmpPtr));

    rc = internal_cpssPxPortCnMessagePortMapEntryGet(devNum, portNum, portsBmpPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portsBmpPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnTerminationEnableSet function
* @endinternal
*
* @brief   Enable/Disable termination of CNM (Congestion Notification Message) on given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
*                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnTerminationEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
)
{
    GT_U32  regAddr;                /* address of register */
    GT_U32  fieldVal;               /* field value */
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    /* Invert bit value */
    fieldVal = (enable == GT_TRUE) ? 0 : 1;

    /* Enable/disbale transmission of CN frames per port - CNMs triggered by the device */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).enCNFrameTxOnPort[0];
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, portNum, 1, fieldVal);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* Enable/disable transmission of CN frames per port - CNMs received by the device */
    regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).config.portsQCNEnableConfig;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, portNum, 1, fieldVal);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortCnTerminationEnableSet function
* @endinternal
*
* @brief   Enable/Disable termination of CNM (Congestion Notification Message) on given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
*                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or port number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnTerminationEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnTerminationEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortCnTerminationEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnTerminationEnableGet function
* @endinternal
*
* @brief   Gets the status of CNM termination on given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
*                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnTerminationEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_U32  regAddr;                /* address of register */
    GT_U32  fieldVal;               /* field value */
    GT_STATUS rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* Enable CN Frame Tx on Port <%n> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).enCNFrameTxOnPort[0];

    /* Get state of transmission of CN frames per port */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, portNum, 1,
        &fieldVal);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* Invert boolean value */
    *enablePtr = (fieldVal == 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssPxPortCnTerminationEnableGet function
* @endinternal
*
* @brief   Gets the status of CNM termination on given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: enable. CNM is terminated, i.e not transmited on this port.
*                                      GT_FALSE: disable. CNM isn't terminated, i.e transmited on this port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number or port
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnTerminationEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnTerminationEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortCnTerminationEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable generation of flow control packets as result of CN
*         frame termination or Panic Pause.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - Enable/Disable generation of flow control packets
*                                      on specific port.
*                                      GT_TRUE:  enable. Flow control packets can be issued.
*                                      GT_FALSE: disable. Flow control packets can't be issued.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnFcEnableSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
)
{
    GT_U32 regAddr;             /* register address */
    GT_U32 regVal;            /* register value */
    GT_STATUS rc;               /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);

    /* Enable/disable FC triggering by CNM transmitted out of the port: CNMs triggered by the device */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).enFCTriggerByCNFrameOnPort[0];
    regVal = BOOL2BIT_MAC(enable);
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, portNum, 1, regVal);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /* Enable/disable FC triggering by CNM transmitted out of the port: CNMs received by the device */
    regAddr = PRV_PX_REG1_UNIT_MCFC_MAC(devNum).config.portsQCN2PFCEnableConfig;
    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, portNum, 1, regVal);
    if(GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortCnFcEnableSet function
* @endinternal
*
* @brief   Enable/Disable generation of flow control packets as result of CN
*         frame termination or Panic Pause.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] enable                   - Enable/Disable generation of flow control packets
*                                      on specific port.
*                                      GT_TRUE:  enable. Flow control packets can be issued.
*                                      GT_FALSE: disable. Flow control packets can't be issued.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnFcEnableSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_BOOL                  enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnFcEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssPxPortCnFcEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortCnFcEnableGet function
* @endinternal
*
* @brief   Get the status of generation of flow control packets as result of CN
*         frame termination or Panic Pause on a given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to) state of specific port to generate
*                                      flow control packets.
*                                      GT_TRUE:  enable. Flow control packets is issued.
*                                      GT_FALSE: disable. Flow control packets is not issued.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnFcEnableGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_U32 regAddr;             /* register address */
    GT_U32 regVal;            /* register value */
    GT_STATUS rc;               /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    /* <Enable FC Trigger by CN Frame on Port <%n>>register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).enFCTriggerByCNFrameOnPort[0];
    /* Get <Port<<%n>> CN FC Enable> field */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, portNum, 1, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BOOL2BIT_MAC(regVal);

    return GT_OK;
}

/**
* @internal cpssPxPortCnFcEnableGet function
* @endinternal
*
* @brief   Get the status of generation of flow control packets as result of CN
*         frame termination or Panic Pause on a given port.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
*
* @param[out] enablePtr                - (pointer to) state of specific port to generate
*                                      flow control packets.
*                                      GT_TRUE:  enable. Flow control packets is issued.
*                                      GT_FALSE: disable. Flow control packets is not issued.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnFcEnableGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnFcEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssPxPortCnFcEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnFcTimerSet function
* @endinternal
*
* @brief   Set timer value for the IEEE 802.3x/PFC frame issued as result
*         of CN frame termination.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] index                    -  (APPLICABLE RANGES: 0..7)
* @param[in] timer                    - 802.3x/PFC pause time (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnFcTimerSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   index,
    IN GT_U32                   timer
)
{
    GT_U32 regAddr;                 /* register address */
    GT_U32 portMacNum;              /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(index, BIT_3);
    CPSS_DATA_CHECK_MAX_MAC(timer, BIT_16);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* <Port Speed Timer %p> register */
    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).portSpeedTimer[index];
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, timer);
}

/**
* @internal cpssPxPortCnFcTimerSet function
* @endinternal
*
* @brief   Set timer value for the IEEE 802.3x/PFC frame issued as result
*         of CN frame termination.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] index                    -  (APPLICABLE RANGES: 0..7)
* @param[in] timer                    - 802.3x/PFC pause time (APPLICABLE RANGES: 0..0xFFFF).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnFcTimerSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_PHYSICAL_PORT_NUM     portNum,
    IN GT_U32                   index,
    IN GT_U32                   timer
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnFcTimerSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, index, timer));

    rc = internal_cpssPxPortCnFcTimerSet(devNum, portNum, index, timer);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, index, timer));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnFcTimerGet function
* @endinternal
*
* @brief   Get timer value for the IEEE 802.3x/PFC frame issued as result
*         of CN frame termination.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] index                    -  (APPLICABLE RANGES: 0..7)
*
* @param[out] timerPtr                 - (pointer to) 802.3x/PFC Pause time.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnFcTimerGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  index,
    OUT GT_U32                  *timerPtr
)
{
    GT_U32 regAddr;                 /* register address */
    GT_U32 portMacNum;              /* MAC number */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_PARAM_CHECK_MAX_MAC(index, BIT_3);
    CPSS_NULL_PTR_CHECK_MAC(timerPtr);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    /* <Port Speed Timer %p> register */
    regAddr = PRV_PX_REG1_UNIT_GOP_FCA_MAC(devNum, portMacNum).portSpeedTimer[index];
    return prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 16, timerPtr);
}

/**
* @internal cpssPxPortCnFcTimerGet function
* @endinternal
*
* @brief   Get timer value for the IEEE 802.3x/PFC frame issued as result
*         of CN frame termination.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] portNum                  - port number.
* @param[in] index                    -  (APPLICABLE RANGES: 0..7)
*
* @param[out] timerPtr                 - (pointer to) 802.3x/PFC Pause time.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong port number, device or speed index
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnFcTimerGet
(
    IN GT_SW_DEV_NUM            devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  index,
    OUT GT_U32                  *timerPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnFcTimerGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, index, timerPtr));

    rc = internal_cpssPxPortCnFcTimerGet(devNum, portNum, index, timerPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, index, timerPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnQueueStatusModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable queue status mode. When enabled, CNM is sent to a
*         configurable target that examines the queue status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[in] targetPort               - Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnQueueStatusModeEnableSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_BOOL                  enable,
    IN GT_PORT_NUM              targetPort
)
{
    GT_U32 regVal = 0;      /* register value */
    GT_U32 regAddr;         /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    
    regVal = 0;
    if (enable == GT_TRUE)
    {
        if (targetPort >= PRV_CPSS_PX_PORTS_NUM_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Target Port must be in [0..16] but got[%d]", targetPort);
        }

        U32_SET_FIELD_MAC(regVal, 0, 1, 1);
        U32_SET_FIELD_MAC(regVal, 11, 15, targetPort);
    }

    /* < CN Target Assignment> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).CNTargetAssignment;
    /* Set <CN Target Assignment Mode>, <CN Target ePort> fields */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 26, regVal);
}

/**
* @internal cpssPxPortCnQueueStatusModeEnableSet function
* @endinternal
*
* @brief   Enable/Disable queue status mode. When enabled, CNM is sent to a
*         configurable target that examines the queue status.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] enable                   - GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[in] targetPort               - Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnQueueStatusModeEnableSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_BOOL                  enable,
    IN GT_PORT_NUM              targetPort
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnQueueStatusModeEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable, targetPort));

    rc = internal_cpssPxPortCnQueueStatusModeEnableSet(devNum, enable, targetPort);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable, targetPort));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnQueueStatusModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of queue status mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of queue status mode.
*                                      GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[out] targetPortPtr            - (pointer to) Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnQueueStatusModeEnableGet
(
    IN GT_SW_DEV_NUM            devNum,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_PORT_NUM             *targetPortPtr
)
{
    GT_U32 regVal;          /* register value */
    GT_U32 regAddr;         /* register address */
    GT_STATUS rc;           /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(targetPortPtr);

    /* < CN Target Assignment> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).CNTargetAssignment;
    /* Get <CN Target Assignment Mode>, <CN Target ePort> fields */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 26, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = U32_GET_FIELD_MAC(regVal, 0, 1);
    *targetPortPtr = U32_GET_FIELD_MAC(regVal, 11, 15);

    return GT_OK;
}

/**
* @internal cpssPxPortCnQueueStatusModeEnableGet function
* @endinternal
*
* @brief   Gets the current status of queue status mode.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] enablePtr                - (pointer to) status of queue status mode.
*                                      GT_TRUE: CNM is sent to a targetDev/targetPort.
*                                      GT_FALSE: CNM is sent to a sampled packet's source.
* @param[out] targetPortPtr            - (pointer to) Target port for the CNM.
*                                      Relevant only if enable==GT_TRUE.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnQueueStatusModeEnableGet
(
    IN GT_SW_DEV_NUM            devNum,
    OUT GT_BOOL                 *enablePtr,
    OUT GT_PORT_NUM             *targetPortPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnQueueStatusModeEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr, targetPortPtr));

    rc = internal_cpssPxPortCnQueueStatusModeEnableGet(devNum, enablePtr, targetPortPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr, targetPortPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssPxPortTxTailDropProfileIdSet.
*
*/
GT_STATUS internal_cpssPxPortCnProfileQueueConfigSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT    profileSet,
    IN GT_U32                                       tcQueue,
    IN CPSS_PX_PORT_CN_PROFILE_CONFIG_STC           *cnProfileCfgPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */
    GT_U32    alphaHwValue; /* alpha hw value */
    GT_U32    regValue = 0;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(cnProfileCfgPtr);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profileSet);
    CPSS_DATA_CHECK_MAX_MAC(cnProfileCfgPtr->threshold, BIT_20);

    switch(cnProfileCfgPtr->alpha)
    {
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E:
            alphaHwValue = 0;
            break;
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E:
            alphaHwValue = 1;
            break;
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E:
            alphaHwValue = 2;
            break;
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:
            alphaHwValue = 3;
            break;
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E:
            alphaHwValue = 4;
            break;
        case CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E:
            alphaHwValue = 5;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* set CN awareness */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
        profileCNMTriggeringEnable[profileSet];

    rc = prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, tcQueue, 1,
                                   BOOL2BIT_MAC(cnProfileCfgPtr->cnAware));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* set threshold */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).
        profilePriorityQueueCNThreshold[profileSet][tcQueue];
    U32_SET_FIELD_MAC(regValue, 0, 20, cnProfileCfgPtr->threshold);
    U32_SET_FIELD_MAC(regValue, 20, 3, alphaHwValue);

    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 23, regValue);
}

/**
* @internal cpssPxPortCnProfileQueueConfigSet function
* @endinternal
*
* @brief   Sets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
* @param[in] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_OUT_OF_RANGE          - on out of range threshold value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note CN Profile is part of the Tail-Drop Profile.
*       To bind port to Tail-Drop Profile use cpssPxPortTxTailDropProfileIdSet.
*
*/
GT_STATUS cpssPxPortCnProfileQueueConfigSet
(
    IN GT_SW_DEV_NUM                                devNum,
    IN CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT    profileSet,
    IN GT_U32                                       tcQueue,
    IN CPSS_PX_PORT_CN_PROFILE_CONFIG_STC           *cnProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnProfileQueueConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, cnProfileCfgPtr));

    rc = internal_cpssPxPortCnProfileQueueConfigSet(devNum, profileSet, tcQueue, cnProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, cnProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPxPortCnProfileQueueConfigGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    OUT CPSS_PX_PORT_CN_PROFILE_CONFIG_STC          *cnProfileCfgPtr
)
{
    GT_U32 regAddr;   /* register address */
    GT_U32 regVal;    /* register value */
    GT_STATUS rc;     /* function return value */


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_COS_CHECK_TC_MAC(tcQueue);
    CPSS_NULL_PTR_CHECK_MAC(cnProfileCfgPtr);
    PRV_CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_CHECK_MAC(profileSet);

    /* get CN awareness */
    regAddr = PRV_PX_REG1_UNIT_TXQ_Q_MAC(devNum).tailDrop.tailDropConfig.
        profileCNMTriggeringEnable[profileSet];

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, tcQueue, 1, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }
    cnProfileCfgPtr->cnAware = BIT2BOOL_MAC(regVal);

    /* get threshold */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).
        profilePriorityQueueCNThreshold[profileSet][tcQueue];

    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 23, &regVal);
    if (rc != GT_OK)
    {
        return rc;
    }

    cnProfileCfgPtr->threshold = U32_GET_FIELD_MAC(regVal, 0, 20);
    
    switch(U32_GET_FIELD_MAC(regVal, 20, 3))
    {
        case 0:
            cnProfileCfgPtr->alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
            break;
        case 1:
            cnProfileCfgPtr->alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E;
            break;
        case 2:
            cnProfileCfgPtr->alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_0_5_E;
            break;
        case 3:
            cnProfileCfgPtr->alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
            break;
        case 4:
            cnProfileCfgPtr->alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_2_E;
            break;
        case 5:
            cnProfileCfgPtr->alpha = CPSS_PX_PORT_TX_TAIL_DROP_DBA_ALPHA_4_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal cpssPxPortCnProfileQueueConfigGet function
* @endinternal
*
* @brief   Gets CN profile configurations for given tc queue.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] profileSet               - profile set.
* @param[in] tcQueue                  - traffic class queue (0..7).
*
* @param[out] cnProfileCfgPtr          - pointer to CN Profile configurations.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_BAD_PARAM             - on wrong device number, profile set
*                                       or traffic class queue
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnProfileQueueConfigGet
(
    IN  GT_SW_DEV_NUM                               devNum,
    IN  CPSS_PX_PORT_TX_TAIL_DROP_PROFILE_SET_ENT   profileSet,
    IN  GT_U32                                      tcQueue,
    OUT CPSS_PX_PORT_CN_PROFILE_CONFIG_STC          *cnProfileCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnProfileQueueConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, profileSet, tcQueue, cnProfileCfgPtr));

    rc = internal_cpssPxPortCnProfileQueueConfigGet(devNum, profileSet, tcQueue, cnProfileCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, profileSet, tcQueue, cnProfileCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] availableBuff            - amount of buffers available for dynamic allocation for
*                                      CN. (APPLICABLE RANGES: 0..0x0FFFFF).
*/
static GT_STATUS internal_cpssPxPortCnAvailableBuffSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          availableBuff
)
{
    GT_U32  regAddr;    /* register address */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (availableBuff >= BIT_20)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    /* <CN Available Buffers> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).cnAvailableBuffers;

    /* Set <cn_available_buffs> field of */
    /* <CN Available Buffers> register   */
    return prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20,
                                  availableBuff);
}

/**
* @internal cpssPxPortCnAvailableBuffSet function
* @endinternal
*
* @brief   Set amount of buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] availableBuff            - amount of buffers available for dynamic allocation for
*                                      CN (APPLICABLE RANGES: 0..0x0FFFFF).
*/
GT_STATUS cpssPxPortCnAvailableBuffSet
(
    IN  GT_SW_DEV_NUM   devNum,
    IN  GT_U32          availableBuff
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnAvailableBuffSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, availableBuff));

    rc = internal_cpssPxPortCnAvailableBuffSet(devNum, availableBuff);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, availableBuff));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnAvailableBuffGet function
* @endinternal
*
* @brief   Get amount of buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] availableBuffPtr         - pointer to amount of buffers available for dynamic
*                                      allocation for CN.
*                                      RETURN:
*                                      GT_OK                    - on success
*                                      GT_FAIL                  - on error
*                                      GT_BAD_PTR               - on NULL pointer
*                                      GT_BAD_PARAM             - on wrong device number
*                                      GT_HW_ERROR              - on hardware error
*                                      GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                      COMMENTS:
*                                      None.
*/
static GT_STATUS internal_cpssPxPortCnAvailableBuffGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *availableBuffPtr
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      regAddr;    /* register address */
    GT_U32      regValue;   /* register value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(availableBuffPtr);

    /* <CN Available Buffers> register */
    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).cnAvailableBuffers;

    /* Set <cn_available_buffs> field of */
    /* <CN Available Buffers> register   */
    rc = prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 20, &regValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    *availableBuffPtr = regValue;

    return GT_OK;
}

/**
* @internal cpssPxPortCnAvailableBuffGet function
* @endinternal
*
* @brief   Get amount of buffers available for dynamic allocation for CN.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] availableBuffPtr         - pointer to amount of buffers available for dynamic
*                                      allocation for CN.
*                                      RETURN:
*                                      GT_OK                    - on success
*                                      GT_FAIL                  - on error
*                                      GT_BAD_PTR               - on NULL pointer
*                                      GT_BAD_PARAM             - on wrong device number
*                                      GT_HW_ERROR              - on hardware error
*                                      GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                      COMMENTS:
*                                      None.
*/
GT_STATUS cpssPxPortCnAvailableBuffGet
(
    IN  GT_SW_DEV_NUM   devNum,
    OUT GT_U32          *availableBuffPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnAvailableBuffGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, availableBuffPtr));

    rc = internal_cpssPxPortCnAvailableBuffGet(devNum, availableBuffPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, availableBuffPtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssPxPortCnMessageTypeSet function
* @endinternal
*
* @brief   Sets congestion notification message type - QCN or CCFC
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mType                    - CN message type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssPxPortCnMessageTypeSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT    mType
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    switch (mType)
    {
        case CPSS_PX_PORT_CN_MESSAGE_TYPE_QCN_E:
            value = 0;
            break;
        case CPSS_PX_PORT_CN_MESSAGE_TYPE_CCFC_E:
            value = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;
    rc =  prvCpssHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    value = (value == 0) ? 1 : 0;

    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->TXQ.pfc.PFCTriggerGlobalConfig;
    rc = prvCpssHwPpPortGroupSetRegField(CAST_SW_DEVNUM(devNum), 0, regAddr, 13, 1, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssPxPortCnMessageTypeSet function
* @endinternal
*
* @brief   Sets congestion notification message type - QCN or CCFC
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
* @param[in] mType                    - CN message type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnMessageTypeSet
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT    mType
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnMessageTypeSet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mType));

    rc = internal_cpssPxPortCnMessageTypeSet(devNum, mType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mType));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortCnMessageTypeGet function
* @endinternal
*
* @brief   Gets congestion notification message type - QCN or CCFC
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] mTypePtr                 - (pointer to) CN message type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS internal_cpssPxPortCnMessageTypeGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT    *mTypePtr
)
{
    GT_U32 value;     /* register value */
    GT_U32 regAddr;   /* register address */
    GT_STATUS rc;     /* function return value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(mTypePtr);

    regAddr = PRV_PX_REG1_UNIT_TXQ_QCN_MAC(devNum).CNGlobalConfig;

    rc =  prvCpssHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 1, &value);
    if(GT_OK != rc)
    {
        return rc;
    }

    *mTypePtr = (value == 0) ? CPSS_PX_PORT_CN_MESSAGE_TYPE_QCN_E : CPSS_PX_PORT_CN_MESSAGE_TYPE_CCFC_E;

    return GT_OK;
}

/**
* @internal cpssPxPortCnMessageTypeGet function
* @endinternal
*
* @brief   Gets congestion notification message type - QCN or CCFC
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number.
*
* @param[out] mTypePtr                 - (pointer to) CN message type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssPxPortCnMessageTypeGet
(
    IN  GT_SW_DEV_NUM                       devNum,
    OUT CPSS_PX_PORT_CN_MESSAGE_TYPE_ENT    *mTypePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortCnMessageTypeGet);
    CPSS_API_LOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mTypePtr));

    rc = internal_cpssPxPortCnMessageTypeGet(devNum, mTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mTypePtr));
    CPSS_API_UNLOCK_MAC(devNum, PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
