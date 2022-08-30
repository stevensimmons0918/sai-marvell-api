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
* @file cpssPxHwInitLedCtrl.c
*
* @brief Includes LEDs control functions implementations.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/px/cpssHwInit/private/prvCpssPxCpssHwInitLog.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/cpssHwInit/cpssPxHwInitLedCtrl.h>
#include <cpss/px/port/private/prvCpssPxPortCtrl.h>
#include <cpss/px/cpssHwInit/private/prvCpssPxHwInitLedCtrl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_CPSS_PX_PIPE_LED_INDICATION_CLASS_NUMBER_CNS 6
#define PRV_CPSS_PX_PIPE_LED_INDICATION_GROUP_NUMBER_CNS 2

#define PRV_CPSS_PX_PORT_LED_POSITION(devNum,portMacNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portLedInfo.ledPosition

#define PRV_CPSS_PX_PORT_LED_MAC_TYPE(devNum,portMacNum) \
        PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].portLedInfo.ledMacType



/**
* @internal internal_cpssPxLedStreamClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] classNum                 - class number. (APPLICABLE RANGES: 0..5)
* @param[in] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssPxLedStreamClassManipulationSet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             classNum,
    IN  CPSS_PX_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_U32  regAddr; /* register address */
    GT_U32  regData; /* register led control value */
    GT_U32  regMask; /* register led control mask */
    GT_U32  fieldOffset; /* register field offset */
    GT_STATUS rc; /* Return code */
    GT_U32  data; /* HW data */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(classParamsPtr);
    if (classNum >= PRV_CPSS_PX_PIPE_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regData = regMask = 0x0;
    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).blinkGlobalControl;
    fieldOffset = 18 + classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1,
        BOOL2BIT_MAC(classParamsPtr->blinkEnable));
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);

    fieldOffset = 10 + classNum;

    switch(classParamsPtr->blinkSelect)
    {
        case CPSS_LED_BLINK_SELECT_0_E:
            data = 0;
            break;
        case CPSS_LED_BLINK_SELECT_1_E:
            data = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData , fieldOffset, 1, data);
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);
    rc = prvCpssDrvHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, regMask, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    regData = regMask = 0x0;
    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).classesAndGroupConfig;

    fieldOffset = 24 + classNum;
    U32_SET_FIELD_MAC(regData , fieldOffset, 1,
        BOOL2BIT_MAC(classParamsPtr->forceEnable));
    U32_SET_FIELD_MAC(regMask , fieldOffset, 1, 1);
    rc = prvCpssDrvHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, regMask, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).classForcedData[classNum];
    rc = prvCpssDrvHwPpWriteRegister(CAST_SW_DEVNUM(devNum), regAddr, classParamsPtr->forceData);
    if( rc != GT_OK )
    {
        return rc;
    }

    regData = regMask = 0x0;
    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).classGlobalControl;

    fieldOffset = 6 + classNum;
    U32_SET_FIELD_MAC(regData, fieldOffset, 1,
        BOOL2BIT_MAC(classParamsPtr->pulseStretchEnable));
    U32_SET_FIELD_MAC(regMask, fieldOffset, 1, 1);

    fieldOffset = classNum;
    U32_SET_FIELD_MAC(regData, fieldOffset, 1,
        BOOL2BIT_MAC(classParamsPtr->disableOnLinkDown));
    U32_SET_FIELD_MAC(regMask, fieldOffset, 1, 1);

    return prvCpssDrvHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, regMask, regData);
}

/**
* @internal cpssPxLedStreamClassManipulationSet function
* @endinternal
*
* @brief   This routine configures a LED stream class manipulation.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] classNum                 - class number. (APPLICABLE RANGES: 0..5)
* @param[in] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamClassManipulationSet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             classNum,
    IN  CPSS_PX_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxLedStreamClassManipulationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, classNum, classParamsPtr));

    rc = internal_cpssPxLedStreamClassManipulationSet(devNum, classNum, classParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, classNum, classParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxLedStreamClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] classNum                 - class number. (APPLICABLE RANGES: 0..5)
*
* @param[out] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssPxLedStreamClassManipulationGet(
    IN GT_SW_DEV_NUM                       devNum,
    IN GT_U32                              classNum,
    OUT CPSS_PX_LED_CLASS_MANIPULATION_STC *classParamsPtr)
{
    GT_U32  regAddr; /* register address */
    GT_U32  regData; /* register led control value */
    GT_U32  fieldOffset; /* register field offset */
    GT_STATUS rc; /* Return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(classParamsPtr);
    if (classNum >= PRV_CPSS_PX_PIPE_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(classParamsPtr, 0,
        sizeof(CPSS_PX_LED_CLASS_MANIPULATION_STC));

    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).blinkGlobalControl;
    rc = prvCpssDrvHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    fieldOffset = 18 + classNum;
    classParamsPtr->blinkEnable =
        BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData , fieldOffset, 1));

    fieldOffset = 10 + classNum;
    classParamsPtr->blinkSelect =
        U32_GET_FIELD_MAC(regData , fieldOffset, 1) ? CPSS_LED_BLINK_SELECT_1_E:
        CPSS_LED_BLINK_SELECT_0_E;

    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).classesAndGroupConfig;
    rc = prvCpssDrvHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    fieldOffset = 24 + classNum;
    classParamsPtr->forceEnable =
        BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData, fieldOffset, 1));


    /* Class forced data Register */
    regAddr =PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).classForcedData[classNum];
    rc = prvCpssDrvHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &classParamsPtr->forceData);
    if( rc != GT_OK )
    {
        return rc;
    }

    /* Class Gobal Control Register */
    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).classGlobalControl;
    rc = prvCpssDrvHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    fieldOffset = 6 + classNum;
    classParamsPtr->pulseStretchEnable =
        BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData, fieldOffset, 1));

    fieldOffset = classNum;
    classParamsPtr->disableOnLinkDown =
        BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData, fieldOffset, 1));

    return GT_OK;
}

/**
* @internal cpssPxLedStreamClassManipulationGet function
* @endinternal
*
* @brief   This routine gets a LED stream class manipulation configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] classNum                 - class number. (APPLICABLE RANGES: 0..5)
*
* @param[out] classParamsPtr           - (pointer to) class manipulation parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamClassManipulationGet
(
    IN GT_SW_DEV_NUM                       devNum,
    IN GT_U32                              classNum,
    OUT CPSS_PX_LED_CLASS_MANIPULATION_STC *classParamsPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxLedStreamClassManipulationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, classNum, classParamsPtr));

    rc = internal_cpssPxLedStreamClassManipulationGet(devNum, classNum, classParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, classNum, classParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxLedStreamConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssPxLedStreamConfigSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_LED_CONF_STC    *ledConfPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */
    GT_U32  regMask;            /* register data mask */
    GT_U32  data;               /* local data */
    GT_U32  blink;              /* blink index */
    CPSS_LED_BLINK_DUTY_CYCLE_ENT   blinkDutyCycle; /* duty cycle of the blink signal */
    CPSS_LED_BLINK_DURATION_ENT     blinkDuration; /* blink freqency divider */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ledConfPtr);

    if(ledConfPtr->ledStart > 255)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(ledConfPtr->ledEnd < ledConfPtr->ledStart)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).LEDControl;
    regData = 0;

    U32_SET_FIELD_MAC(regData, 8,  8, ledConfPtr->ledStart);
    U32_SET_FIELD_MAC(regData, 16, 8, ledConfPtr->ledEnd);
    switch(ledConfPtr->pulseStretch)
    {
        case CPSS_LED_PULSE_STRETCH_0_NO_E:
            data = 0;
            break;
        case CPSS_LED_PULSE_STRETCH_1_E:
            data = 1;
            break;
        case CPSS_LED_PULSE_STRETCH_2_E:
            data = 2;
            break;
        case CPSS_LED_PULSE_STRETCH_3_E:
            data = 3;
            break;
        case CPSS_LED_PULSE_STRETCH_4_E:
            data = 4;
            break;
        case CPSS_LED_PULSE_STRETCH_5_E:
            data = 5;
            break;
        case CPSS_LED_PULSE_STRETCH_6_E:
            data = 6;
            break;
        case CPSS_LED_PULSE_STRETCH_7_E:
            data = 7;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData, 5, 3, data);


    U32_SET_FIELD_MAC(regData, 4, 1, BOOL2BIT_MAC(ledConfPtr->invertEnable));

    switch(ledConfPtr->ledClockFrequency)
    {
        case CPSS_LED_CLOCK_OUT_FREQUENCY_500_E:
            data = 0;
            break;
        case CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E:
            data = 1;
            break;
        case CPSS_LED_CLOCK_OUT_FREQUENCY_2000_E:
            data = 2;
            break;
        case CPSS_LED_CLOCK_OUT_FREQUENCY_3000_E:
            data = 3;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData, 2, 2, data);

    switch(ledConfPtr->ledOrganize)
    {
        case CPSS_LED_ORDER_MODE_BY_PORT_E:
            data = 0;
            break;
        case CPSS_LED_ORDER_MODE_BY_CLASS_E:
            data = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    U32_SET_FIELD_MAC(regData, 1, 1, data);

    regMask = 0;
    U32_SET_FIELD_MAC(regMask , 1, 23, ((1 << 23)-1));
    rc = prvCpssDrvHwPpWriteRegBitMask(CAST_SW_DEVNUM(devNum), regAddr, regMask, regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).blinkGlobalControl;
    regData = 0;

    for (blink = 0; blink < 2; blink++)
    {
        if (blink == 0)
        {
            blinkDuration = ledConfPtr->blink0Duration;
            blinkDutyCycle = ledConfPtr->blink0DutyCycle;
        }
        else
        {
            blinkDuration = ledConfPtr->blink1Duration;
            blinkDutyCycle = ledConfPtr->blink1DutyCycle;
        }
        switch (blinkDuration)
        {
            case CPSS_LED_BLINK_DURATION_0_E:
                data = 0;
                break;
            case CPSS_LED_BLINK_DURATION_1_E:
                data = 1;
                break;
            case CPSS_LED_BLINK_DURATION_2_E:
                data = 2;
                break;
            case CPSS_LED_BLINK_DURATION_3_E:
                data = 3;
                break;
            case CPSS_LED_BLINK_DURATION_4_E:
                data = 4;
                break;
            case CPSS_LED_BLINK_DURATION_5_E:
                data = 5;
                break;
            case CPSS_LED_BLINK_DURATION_6_E:
                data = 6;
                break;
            case CPSS_LED_BLINK_DURATION_7_E:
                data = 7;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        U32_SET_FIELD_MAC(regData , 0 + (5 * blink), 3, data);

        switch(blinkDutyCycle)
        {
            case CPSS_LED_BLINK_DUTY_CYCLE_0_E:
                data = 0;
                break;
            case CPSS_LED_BLINK_DUTY_CYCLE_1_E:
            case CPSS_LED_BLINK_DUTY_CYCLE_2_E:
                data = 1;
                break;
            case CPSS_LED_BLINK_DUTY_CYCLE_3_E:
                data = 2;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        U32_SET_FIELD_MAC(regData , 3 + (5 * blink), 2, data);
    }

    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 10, regData);
}

/**
* @internal cpssPxLedStreamConfigSet function
* @endinternal
*
* @brief   This routine configures the LED stream.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - ledStart or ledEnd out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamConfigSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_LED_CONF_STC    *ledConfPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxLedStreamConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledConfPtr));

    rc = internal_cpssPxLedStreamConfigSet(devNum, ledConfPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledConfPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxLedStreamConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssPxLedStreamConfigGet
(
    IN  GT_SW_DEV_NUM           devNum,
    OUT CPSS_PX_LED_CONF_STC    *ledConfPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */
    GT_U32  blink;              /* blink index */

    /* (pointer to) duty cycle of the blink signal */
    CPSS_LED_BLINK_DUTY_CYCLE_ENT   *blinkDutyCyclePtr;

    /* (pointer to) blink frequency divider */
    CPSS_LED_BLINK_DURATION_ENT     *blinkDurationPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(ledConfPtr);

    cpssOsMemSet(ledConfPtr, 0, sizeof(CPSS_PX_LED_CONF_STC));

    /* LED control */
    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).LEDControl;
    rc = prvCpssDrvHwPpReadRegister(CAST_SW_DEVNUM(devNum), regAddr, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    ledConfPtr->ledStart = U32_GET_FIELD_MAC(regData, 8,  8);
    ledConfPtr->ledEnd = U32_GET_FIELD_MAC(regData, 16, 8);
    switch(U32_GET_FIELD_MAC(regData, 5, 3))
    {
        case 0:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_0_NO_E;
            break;
        case 1:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_1_E;
            break;
        case 2:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_2_E;
            break;
        case 3:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_3_E;
            break;
        case 4:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_4_E;
            break;
        case 5:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_5_E;
            break;
        case 6:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_6_E;
            break;
        case 7:
            ledConfPtr->pulseStretch = CPSS_LED_PULSE_STRETCH_7_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    ledConfPtr->invertEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData, 4, 1));

    switch(U32_GET_FIELD_MAC(regData, 2, 2))
    {
        case 0:
            ledConfPtr->ledClockFrequency = CPSS_LED_CLOCK_OUT_FREQUENCY_500_E;
            break;
        case 1:
            ledConfPtr->ledClockFrequency = CPSS_LED_CLOCK_OUT_FREQUENCY_1000_E;
            break;
        case 2:
            ledConfPtr->ledClockFrequency = CPSS_LED_CLOCK_OUT_FREQUENCY_2000_E;
            break;
        case 3:
            ledConfPtr->ledClockFrequency = CPSS_LED_CLOCK_OUT_FREQUENCY_3000_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    switch(U32_GET_FIELD_MAC(regData, 1, 1))
    {
        case 0:
            ledConfPtr->ledOrganize = CPSS_LED_ORDER_MODE_BY_PORT_E;
            break;
        case 1:
            ledConfPtr->ledOrganize = CPSS_LED_ORDER_MODE_BY_CLASS_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    /* Blink global control */
    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).blinkGlobalControl;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, 0, 10, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    for (blink = 0; blink < 2; blink++)
    {
        if (blink == 0)
        {
            blinkDurationPtr = &ledConfPtr->blink0Duration;
            blinkDutyCyclePtr = &ledConfPtr->blink0DutyCycle;
        }
        else
        {
            blinkDurationPtr = &ledConfPtr->blink1Duration;
            blinkDutyCyclePtr = &ledConfPtr->blink1DutyCycle;
        }
        switch (U32_GET_FIELD_MAC(regData , 0 + (5 * blink), 3))
        {
            case 0:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_0_E;
                break;
            case 1:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_1_E;
                break;
            case 2:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_2_E;
                break;
            case 3:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_3_E;
                break;
            case 4:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_4_E;
                break;
            case 5:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_5_E;
                break;
            case 6:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_6_E;
                break;
            case 7:
                *blinkDurationPtr = CPSS_LED_BLINK_DURATION_7_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        switch(U32_GET_FIELD_MAC(regData , 3 + (5 * blink), 2))
        {
            case 0:
                *blinkDutyCyclePtr = CPSS_LED_BLINK_DUTY_CYCLE_0_E;
                break;
            case 1:
                *blinkDutyCyclePtr = CPSS_LED_BLINK_DUTY_CYCLE_1_E;
                break;
            case 2:
                *blinkDutyCyclePtr = CPSS_LED_BLINK_DUTY_CYCLE_3_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssPxLedStreamConfigGet function
* @endinternal
*
* @brief   This routine gets the LED stream configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] ledConfPtr               - (pointer to) LED stream configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_STATE             - on bad hardware value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamConfigGet
(
    IN  GT_SW_DEV_NUM           devNum,
    OUT CPSS_PX_LED_CONF_STC    *ledConfPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxLedStreamConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ledConfPtr));

    rc = internal_cpssPxLedStreamConfigGet(devNum, ledConfPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ledConfPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxLedStreamGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] groupNum                 - group number. (APPLICABLE RANGES: 0..1)
* @param[in] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range o..5)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssPxLedStreamGroupConfigSet
(
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U32                   groupNum,
    IN CPSS_LED_GROUP_CONF_STC  *groupParamsPtr
)
{
    GT_U32  regData;            /* register value */
    GT_U32  regAddr;            /* register address */
    GT_U32  groupDataOffset;    /* offset in register for group data */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(groupParamsPtr);
    if(groupNum >= PRV_CPSS_PX_PIPE_LED_INDICATION_GROUP_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if( (groupParamsPtr->classA >= 6) ||
        (groupParamsPtr->classB >= 6) ||
        (groupParamsPtr->classC >= 6) ||
        (groupParamsPtr->classD >= 6) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    
    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).classesAndGroupConfig;
    regData = 0;

    groupDataOffset = groupNum * 12;

    U32_SET_FIELD_MAC(regData, 0, 3, groupParamsPtr->classA);
    U32_SET_FIELD_MAC(regData, 3, 3, groupParamsPtr->classB);
    U32_SET_FIELD_MAC(regData, 6, 3, groupParamsPtr->classC);
    U32_SET_FIELD_MAC(regData, 9, 3, groupParamsPtr->classD);

    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, groupDataOffset,
        12, regData);
}

/**
* @internal cpssPxLedStreamGroupConfigSet function
* @endinternal
*
* @brief   This routine configures a LED stream group.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] groupNum                 - group number. (APPLICABLE RANGES: 0..1)
* @param[in] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - on out of range class number
*                                       (allowable class range o..5)
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamGroupConfigSet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      groupNum,
    IN  CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxLedStreamGroupConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, groupNum, groupParamsPtr));

    rc = internal_cpssPxLedStreamGroupConfigSet(devNum, groupNum, groupParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, groupNum, groupParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxLedStreamGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] groupNum                 - group number. (APPLICABLE RANGES: 0..1)
*
* @param[out] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssPxLedStreamGroupConfigGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc;               /* return status */
    GT_U32  regData;            /* register led control value */
    GT_U32  regAddr;            /* register address */
    GT_U32  groupDataOffset;    /* offset in register for group data */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(groupParamsPtr);
    if(groupNum >= PRV_CPSS_PX_PIPE_LED_INDICATION_GROUP_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    groupDataOffset = groupNum * 12;
    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).classesAndGroupConfig;
    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, groupDataOffset, 12,
        &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    groupParamsPtr->classA = U32_GET_FIELD_MAC(regData, 0, 3);
    groupParamsPtr->classB = U32_GET_FIELD_MAC(regData, 3, 3);
    groupParamsPtr->classC = U32_GET_FIELD_MAC(regData, 6, 3);
    groupParamsPtr->classD = U32_GET_FIELD_MAC(regData, 9, 3);

    return GT_OK;
}

/**
* @internal cpssPxLedStreamGroupConfigGet function
* @endinternal
*
* @brief   This routine gets a LED stream group configuration.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] groupNum                 - group number. (APPLICABLE RANGES: 0..1)
*
* @param[out] groupParamsPtr           - (pointer to) group configuration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamGroupConfigGet
(
    IN  GT_SW_DEV_NUM               devNum,
    IN  GT_U32                      groupNum,
    OUT CPSS_LED_GROUP_CONF_STC     *groupParamsPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxLedStreamGroupConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, groupNum, groupParamsPtr));

    rc = internal_cpssPxLedStreamGroupConfigGet(devNum, groupNum, groupParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, groupNum, groupParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxLedStreamPortPositionByMacTypeHWSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain for specific MAC
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] portMacType              - macType
* @param[in] position                 - port  index in LED stream
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS prvCpssPxLedStreamPortPositionByMacTypeHWSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  PRV_CPSS_PORT_TYPE_ENT  portMacType,
    IN  GT_U32                  position
)
{
    GT_U32      portMacNum;             /* MAC number */
    GT_U32      regAddr;                /* register address */
    GT_U32      fieldOffset;            /* register field offset */


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);
    switch (portMacType)
    {
        case PRV_CPSS_PORT_GE_E:
            PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum, portMacType,
                &regAddr);
            fieldOffset = 10;
            break;
        case PRV_CPSS_PORT_XLG_E:
            regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.
                perPortRegs[portMacNum].xgMibCountersCtrl;
            fieldOffset = 5;
            break;
        case PRV_CPSS_PORT_CG_E:
            PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL1_REG_MAC(devNum, portMacNum,
                    &regAddr);
            fieldOffset = 0;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,
                "not supported MAC type %d, see PRV_CPSS_PORT_TYPE_ENT",
                portMacType);
    }

    if (prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, 6, position) !=
        GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal prvCpssPxLedErrataPortLedPositionSet function
* @endinternal
*
* @brief   Init Port Led Position set and MAC type specified
*         writes position to approptiate mac
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - portNum
* @param[in] position                 - led  on LED interface corresponding to the port (MAC)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssPxLedErrataPortLedPositionSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
)
{
    GT_STATUS                  rc;
    GT_U32                     portMacNum;             /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT     macType;

    if (GT_FALSE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum,
        PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    macType = PRV_CPSS_PX_PORT_LED_MAC_TYPE(devNum,portMacNum);

    /* configure new mac to position from DB */
    if (macType != PRV_CPSS_PORT_NOT_EXISTS_E)
    {
        rc = prvCpssPxLedStreamPortPositionByMacTypeHWSet(devNum, portNum,
            macType, position);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }

    /* store new postion */
    PRV_CPSS_PX_PORT_LED_POSITION(devNum,portMacNum) = position;
    return GT_OK;
}

/**
* @internal internal_cpssPxLedStreamPortPositionSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] position                 - port  index in LED stream
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssPxLedStreamPortPositionSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
)
{
    GT_U32 portMacNum;                  /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* port MAC type */
    GT_U32 fieldOffset;                 /* register field offset */
    GT_U32 regAddr;                     /* register addresss */
    GT_STATUS rc;                       /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if(position >= BIT_6)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    if (GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum,
        PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        rc = prvCpssPxLedErrataPortLedPositionSet(devNum, portNum, position);
        if (rc != GT_OK)
        {
            return rc;
        }
        return GT_OK;
    }
    else
    {
        for(portMacType = PRV_CPSS_PORT_GE_E;
            portMacType < PRV_CPSS_PORT_NOT_APPLICABLE_E;
            portMacType++)
        {
            switch (portMacType)
            {
                case PRV_CPSS_PORT_GE_E:
                    PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum,
                        portMacType, &regAddr);
                    fieldOffset = 10;
                    break;
                case PRV_CPSS_PORT_XLG_E:
                    regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.
                        perPortRegs[portMacNum].xgMibCountersCtrl;
                    fieldOffset = 5;
                    break;
                case PRV_CPSS_PORT_CG_E:
                    if (PRV_CPSS_XG_PORT_CG_SGMII_E !=
                        PRV_CPSS_PX_PORT_TYPE_OPTIONS_MAC(devNum, portMacNum))
                    {
                        continue;
                    }
                    PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL1_REG_MAC(devNum, portMacNum,
                        &regAddr);
                    fieldOffset = 0;
                    break;
                default:
                    continue;
            }

            if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
            {
                    rc = prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, 6,
                        position);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
            }
            else
            {
                continue;
            }
        }
    }
    return GT_OK;
}

/**
* @internal cpssPxLedStreamPortPositionSet function
* @endinternal
*
* @brief   Sets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
* @param[in] position                 - port  index in LED stream
*                                      (APPLICABLE RANGES: 0..63)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamPortPositionSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  position
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxLedStreamPortPositionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, position));

    rc = internal_cpssPxLedStreamPortPositionSet(devNum, portNum, position);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, position));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxLedStreamPortPositionGet function
* @endinternal
*
* @brief   Gets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssPxLedStreamPortPositionGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *positionPtr
)
{
    GT_U32      regAddr;                /* register address */
    GT_U32      fieldOffset;            /* register field offset */
    PRV_CPSS_PORT_TYPE_ENT portMacType; /* port MAC type */
    GT_U32      portMacNum;             /* MAC number */
    GT_U32      rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    portMacType = PRV_CPSS_PX_PORT_TYPE_MAC(devNum,portMacNum);

    if (GT_TRUE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum,
        PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        *positionPtr = PRV_CPSS_PX_PORT_LED_POSITION(devNum, portMacNum);
        return GT_OK;
    }

    switch (portMacType)
    {
        case PRV_CPSS_PORT_NOT_EXISTS_E:
        case PRV_CPSS_PORT_FE_E:
        case PRV_CPSS_PORT_GE_E:
            PRV_CPSS_PX_PORT_MAC_CTRL4_REG_MAC(devNum, portMacNum,
                portMacType, &regAddr);
            fieldOffset = 10;
            break;
        case PRV_CPSS_PORT_CG_E:
            PRV_CPSS_PX_REG1_CG_CONVERTERS_CTRL1_REG_MAC(devNum, portMacNum,
                &regAddr);
            fieldOffset = 0;
            break;
        case PRV_CPSS_PORT_XG_E:
        case PRV_CPSS_PORT_XLG_E:
            regAddr = PRV_CPSS_PX_DEV_REGS_VER1_MAC(devNum)->GOP.
                perPortRegs[portMacNum].xgMibCountersCtrl;
            fieldOffset = 5;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(regAddr != PRV_CPSS_SW_PTR_ENTRY_UNUSED)
    {
        rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, fieldOffset, 6,
                positionPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
        return rc;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
}

/**
* @internal cpssPxLedStreamPortPositionGet function
* @endinternal
*
* @brief   Gets number of the port in the LED's chain.
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamPortPositionGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_U32                  *positionPtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxLedStreamPortPositionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, positionPtr));

    rc = internal_cpssPxLedStreamPortPositionGet(devNum, portNum, positionPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, positionPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxLedStreamPortClassPolarityInvertEnableSet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number. (APPLICABLE RANGES: 0..5)
* @param[in] invertEnable             - status of the selected polarity indication.
*                                      GT_TRUE  - polarity inverted
*                                      GT_FALSE - polarity not inverted
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssPxLedStreamPortClassPolarityInvertEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  GT_BOOL                         invertEnable
)
{
    GT_U32 portMacNum;                  /* MAC number */
    GT_U32 ledPort;                     /* Port position */
    GT_STATUS rc;                       /* Return code */
    GT_U32 regAddr;                     /* Register address */
    GT_U32 bitOffset;                   /* Offset within register */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (classNum >= PRV_CPSS_PX_PIPE_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = cpssPxLedStreamPortPositionGet(devNum, portNum, &ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Port indication polarity */
    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).
        portIndicationPolarity[ledPort/4];
    bitOffset = (ledPort % 4) * 6 + classNum;

    return prvCpssDrvHwPpSetRegField(CAST_SW_DEVNUM(devNum), regAddr, bitOffset, 1,
        BOOL2BIT_MAC(invertEnable));
}

/**
* @internal cpssPxLedStreamPortClassPolarityInvertEnableSet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number. (APPLICABLE RANGES: 0..5)
* @param[in] invertEnable             - status of the selected polarity indication.
*                                      GT_TRUE  - polarity inverted
*                                      GT_FALSE - polarity not inverted
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamPortClassPolarityInvertEnableSet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    IN  GT_BOOL                         invertEnable
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxLedStreamPortClassPolarityInvertEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, classNum, invertEnable));

    rc = internal_cpssPxLedStreamPortClassPolarityInvertEnableSet(devNum, portNum, classNum, invertEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, classNum, invertEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxLedStreamPortClassPolarityInvertEnableGet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number. (APPLICABLE RANGES: 0..5)
*
* @param[out] invertEnablePtr          - (pointer to) status of the selected polarity indication.
*                                      GT_TRUE  - polarity inverted
*                                      GT_FALSE - polarity not inverted
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
static GT_STATUS internal_cpssPxLedStreamPortClassPolarityInvertEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    OUT GT_BOOL                         *invertEnablePtr
)
{
    GT_U32 portMacNum;                  /* MAC number */
    GT_U32 ledPort;                     /* Port position */
    GT_STATUS rc;                       /* Return code */
    GT_U32 regAddr;                     /* Register address */
    GT_U32 bitOffset;                   /* Offset within register */
    GT_U32  fieldValue;                 /* register field value */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    if (classNum >= PRV_CPSS_PX_PIPE_LED_INDICATION_CLASS_NUMBER_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    rc = cpssPxLedStreamPortPositionGet(devNum, portNum, &ledPort);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Port indication polarity */
    regAddr = PRV_PX_REG1_UNIT_GOP_LED_MAC(devNum).
        portIndicationPolarity[ledPort/4];
    bitOffset = (ledPort % 4) * 6 + classNum;

    rc = prvCpssDrvHwPpGetRegField(CAST_SW_DEVNUM(devNum), regAddr, bitOffset, 1, &fieldValue);
    if( rc != GT_OK )
    {
        return rc;
    }
    *invertEnablePtr = BIT2BOOL_MAC(fieldValue);

    return GT_OK;
}

/**
* @internal cpssPxLedStreamPortClassPolarityInvertEnableGet function
* @endinternal
*
* @brief   This routine set the polarity of the selected indications
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number.
* @param[in] classNum                 - class number. (APPLICABLE RANGES: 0..5)
*
* @param[out] invertEnablePtr          - (pointer to) status of the selected polarity indication.
*                                      GT_TRUE  - polarity inverted
*                                      GT_FALSE - polarity not inverted
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamPortClassPolarityInvertEnableGet
(
    IN  GT_SW_DEV_NUM                   devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    IN  GT_U32                          classNum,
    OUT GT_BOOL                         *invertEnablePtr
)
{
    GT_STATUS rc; /* Return code */
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxLedStreamPortClassPolarityInvertEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, classNum, invertEnablePtr));

    rc = internal_cpssPxLedStreamPortClassPolarityInvertEnableGet(devNum, portNum, classNum, invertEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, classNum, invertEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssPxLedErrataPortLedMacTypeSet function
* @endinternal
*
* @brief   Init Port Led Mac type set and if postion is configured
*         writes position to approptiate mac
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - portNum
* @param[in] macType                  - macType, (PRV_CPSS_PORT_NOT_EXISTS_E, if MAC is deacivated)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssPxLedErrataPortLedMacTypeSet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  PRV_CPSS_PORT_TYPE_ENT  macType
)
{
    GT_STATUS                  rc;
    GT_U32                     portMacNum; /* MAC number */
    PRV_CPSS_PORT_TYPE_ENT     oldMMacType;
    GT_U32                     position;

    if (GT_FALSE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum,
        PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    PRV_CPSS_PX_PORT_NUM_CHECK_AND_MAC_NUM_GET_MAC(devNum, portNum, portMacNum);

    oldMMacType = PRV_CPSS_PX_PORT_LED_MAC_TYPE(devNum, portMacNum);
    position    = PRV_CPSS_PX_PORT_LED_POSITION(devNum, portMacNum);

    /* designate position in old mac as void  in any case */
    if (oldMMacType != PRV_CPSS_PORT_NOT_EXISTS_E)
    {
        rc = prvCpssPxLedStreamPortPositionByMacTypeHWSet(devNum,
            portNum, oldMMacType, PRV_CPSS_PX_PORT_LED_POSITION_VOID_CNS);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }

    /* configure new mac to position from DB */
    if (macType != PRV_CPSS_PORT_NOT_EXISTS_E)
    {
        rc = prvCpssPxLedStreamPortPositionByMacTypeHWSet(devNum,
            portNum, macType, position);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    /* set new mac status */
    PRV_CPSS_PX_PORT_LED_MAC_TYPE(devNum, portMacNum) = macType;
    return GT_OK;
}

/**
* @internal prvCpssPxLedErrataPortLedInit function
* @endinternal
*
* @brief   Init Port Led related Data Base
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_OUT_OF_RANGE          - position out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvCpssPxLedErrataPortLedInit
(
    IN  GT_SW_DEV_NUM                   devNum
)
{
    GT_U32 portMacNum;
    GT_U32 maxMacNum;

    if (GT_FALSE == PRV_CPSS_PX_ERRATA_GET_MAC(devNum,
        PRV_CPSS_PX_PORT_LED_POSITION_JUST_ON_USED_MAC_WA_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    maxMacNum = PRV_CPSS_PP_MAC(devNum)->numOfPorts;
    for (portMacNum = 0 ; portMacNum < maxMacNum; portMacNum++)
    {
        /* init DB for all macs to invalide position */
        PRV_CPSS_PX_PORT_LED_POSITION(devNum,portMacNum) = PRV_CPSS_PX_PORT_LED_POSITION_VOID_CNS;

        /* init DB for all macs to invalide mode     */
        PRV_CPSS_PX_PORT_LED_MAC_TYPE(devNum,portMacNum) = PRV_CPSS_PORT_NOT_EXISTS_E;
    }
    return GT_OK;
}

