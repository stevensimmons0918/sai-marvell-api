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
* @file cpssPxHwInitLedCtrl.h
*
* @brief Includes LEDs control functions declarations.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssPxHwInitLedCtrlh
#define __cpssPxHwInitLedCtrlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssHwInit/cpssCommonLedCtrl.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>

/**
* @struct CPSS_PX_LED_CLASS_MANIPULATION_STC
 *
 * @brief LED class manipulation configuration.
*/
typedef struct{

    /** @brief Enables an AND operation on Class with selected Blink signals.
     *  GT_TRUE - blinking enabled: High data displayed as blinking.
     *  GT_FALSE - blinking feature not enabled.
     */
    GT_BOOL blinkEnable;

    /** Selects between Blink 0 or Blink 1 signals. */
    CPSS_LED_BLINK_SELECT_ENT blinkSelect;

    /** @brief Enables forcing Class data.
     *  GT_TRUE - forceData is the data displayed.
     *  GT_FALSE - no forcing on dispalyed data.
     */
    GT_BOOL forceEnable;

    /** The data to be forced on Class data. */
    GT_U32 forceData;

    /** - enable the stretch affect on a specific class */
    GT_BOOL pulseStretchEnable;

    /** - disable the LED indication classes when the link is down */
    GT_BOOL disableOnLinkDown;

} CPSS_PX_LED_CLASS_MANIPULATION_STC;

/**
* @struct CPSS_PX_LED_CONF_STC
 *
 * @brief LED stream configuration parameters.
*/
typedef struct{

    /** The order of the signals driven in the LED stream. */
    CPSS_LED_ORDER_MODE_ENT ledOrganize;

    /** The duty cycle of the Blink0 signal. */
    CPSS_LED_BLINK_DUTY_CYCLE_ENT blink0DutyCycle;

    /** The period of the Blink0 signal. */
    CPSS_LED_BLINK_DURATION_ENT blink0Duration;

    /** The duty cycle of the Blink1 signal. */
    CPSS_LED_BLINK_DUTY_CYCLE_ENT blink1DutyCycle;

    /** The period of the Blink1 signal. */
    CPSS_LED_BLINK_DURATION_ENT blink1Duration;

    /** The length of stretching for dynamic signals. */
    CPSS_LED_PULSE_STRETCH_ENT pulseStretch;

    /** @brief The first bit in the LED stream to be driven.
     *  (APPLICABLE RANGES: 0..255)
     */
    GT_U32 ledStart;

    /** @brief The last bit in the LED stream to be driven.
     *  (APPLICABLE RANGES: 0..255)
     */
    GT_U32 ledEnd;

    /** @brief LED data polarity
     *  GT_TRUE - active low: LED light is in low indication.
     *  GT_FALSE - active high: LED light is in high indication.
     */
    GT_BOOL invertEnable;

    /** LED output clock frequency */
    CPSS_LED_CLOCK_OUT_FREQUENCY_ENT ledClockFrequency;

} CPSS_PX_LED_CONF_STC;

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
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS cpssPxLedStreamClassManipulationSet
(
    IN  GT_SW_DEV_NUM                      devNum,
    IN  GT_U32                             classNum,
    IN  CPSS_PX_LED_CLASS_MANIPULATION_STC *classParamsPtr
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssPxHwInitLedCtrlh */

