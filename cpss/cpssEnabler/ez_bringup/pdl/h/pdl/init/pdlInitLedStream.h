/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\init\pdlinitLedStream.h.
 *
 * @brief   Declares the pdlinit ledstream class
 */

#ifndef __pdlInitLedStreamh
#define __pdlInitLedStreamh
/**
********************************************************************************
 * @file pdlInit.h   
 * @copyright
 *    (c), Copyright 2001, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 * 
 * @brief Platform driver layer - Init library
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>

/**
* @enum PDL_LED_ORDER_MODE_ENT
 *
 * @brief LED stream ordering mode
*/
typedef enum{
    /** the indication order is arranged by port */
    PDL_LED_ORDER_MODE_BY_PORT_E,
    /** the indication order is arranged by class */
    PDL_LED_ORDER_MODE_BY_CLASS_E
} PDL_LED_ORDER_MODE_ENT;

/**
* @enum PDL_LED_BLINK_DUTY_CYCLE_ENT
 *
 * @brief Duty cycle of LED blink signal
*/
typedef enum{
    /** 25% on, 75% off */
    PDL_LED_BLINK_DUTY_CYCLE_0_E,
    /** 50% on, 50% off */
    PDL_LED_BLINK_DUTY_CYCLE_1_E,
    /** 50% on, 50% off */
    PDL_LED_BLINK_DUTY_CYCLE_2_E,
    /** 75% on, 25% off */
    PDL_LED_BLINK_DUTY_CYCLE_3_E
} PDL_LED_BLINK_DUTY_CYCLE_ENT;

/**
* @enum PDL_LED_BLINK_DURATION_ENT
 *
 * @brief The period of the LED Blink signal
 * (see HW spec doc for meaning of values).
*/
typedef enum{
    PDL_LED_BLINK_DURATION_0_E,
    PDL_LED_BLINK_DURATION_1_E,
    PDL_LED_BLINK_DURATION_2_E,
    PDL_LED_BLINK_DURATION_3_E,
    PDL_LED_BLINK_DURATION_4_E,
    PDL_LED_BLINK_DURATION_5_E,
    /** (APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Pipe) */
    PDL_LED_BLINK_DURATION_6_E,
    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Pipe) */
    PDL_LED_BLINK_DURATION_7_E
} PDL_LED_BLINK_DURATION_ENT;

/**
* @enum PDL_LED_PULSE_STRETCH_ENT
 *
 * @brief The length of stretching for dynamic signals
 * (see HW spec doc for meaning of values).
*/
typedef enum{
    /** no stretching */
    PDL_LED_PULSE_STRETCH_0_NO_E,
    PDL_LED_PULSE_STRETCH_1_E,
    PDL_LED_PULSE_STRETCH_2_E,
    PDL_LED_PULSE_STRETCH_3_E,
    PDL_LED_PULSE_STRETCH_4_E,
    PDL_LED_PULSE_STRETCH_5_E,
    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; Pipe) */
    PDL_LED_PULSE_STRETCH_6_E,
    /** (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; Pipe) */
    PDL_LED_PULSE_STRETCH_7_E
} PDL_LED_PULSE_STRETCH_ENT;

/**
* @enum PDL_LED_CLOCK_OUT_FREQUENCY_ENT
 *
 * @brief The LED clock out frequency
*/
typedef enum{
    /** 500 KHz LED clock frequency. */
    PDL_LED_CLOCK_OUT_FREQUENCY_500_E,
    /** 1 MHz LED clock frequency. */
    PDL_LED_CLOCK_OUT_FREQUENCY_1000_E,
    /** 2 MHz LED clock frequency. */
    PDL_LED_CLOCK_OUT_FREQUENCY_2000_E,
    /** 3 MHz LED clock frequency. */
    PDL_LED_CLOCK_OUT_FREQUENCY_3000_E
} PDL_LED_CLOCK_OUT_FREQUENCY_ENT;

/**
* @enum PDL_LED_CLASS_5_SELECT_ENT
 *
 * @brief The indication displayed on class5 (for dual-media port\phy).
*/
typedef enum{
    /** Half Duplex is displayed on class5. */
    PDL_LED_CLASS_5_SELECT_HALF_DUPLEX_E,
    /** @brief If port is a dual media port,
     *  Fiber Link Up is displayed on class5.
     */
    PDL_LED_CLASS_5_SELECT_FIBER_LINK_UP_E
} PDL_LED_CLASS_5_SELECT_ENT;

/**
* @enum PDL_LED_CLASS_13_SELECT_ENT
 *
 * @brief The indication displayed on class13 (for dual-media port\phy).
*/
typedef enum{
    /** Link Down is displayed on class13. */
    PDL_LED_CLASS_13_SELECT_LINK_DOWN_E,
    /** @brief If port is a dual media port,
     *  Copper Link Up is displayed on class13.
     */
    PDL_LED_CLASS_13_SELECT_COPPER_LINK_UP_E
} PDL_LED_CLASS_13_SELECT_ENT;

#define PDL_LEDSTREAM_CHIPLETS_MAX_NUM_CNS       16
/*
 * typedef: struct PDL_LED_SIP6_CONF_STC
 *
 * Description:
 *      LED stream SIP6 specific configuration parameters.
 *
 * Fields:
 *
 *      ledClockFrequency   - LED output clock frequency in kHz
 *                            (APPLICABLE RANGES: 500..80000)
 *      ledStart            - The first bit in the LED stream indication to be driven in current chiplet
 *                            (APPLICABLE RANGES: 0..255)
 *      ledEnd              - The last bit in the LED stream indication to be driven in the current chiplet
 *                            (APPLICABLE RANGES: 0..255)
 *                              The enumeration of chiplets in full chip system is as follows:
 *                              - Chiplet 15 is the first (also in the stream) and 0 is last.
 *                              - The mapping of MACs in chiplets:
 *                                  MAC network ports   0.. 15   Chiplet  0
 *                                  MAC network ports  16.. 31   Chiplet  1
 *                                  MAC network ports  32.. 47   Chiplet  2
 *                                  MAC network ports  48.. 63   Chiplet  3
 *                                  MAC network ports  64.. 79   Chiplet  4
 *                                  MAC network ports  80.. 95   Chiplet  5
 *                                  MAC network ports  96..111   Chiplet  6
 *                                  MAC network ports 112..127   Chiplet  7
 *                                  MAC network ports 128..143   Chiplet  8
 *                                  MAC network ports 144..159   Chiplet  9
 *                                  MAC network ports 160..175   Chiplet 10
 *                                  MAC network ports 176..191   Chiplet 11
 *                                  MAC network ports 192..207   Chiplet 12
 *                                  MAC network ports 208..223   Chiplet 13
 *                                  MAC network ports 224..239   Chiplet 14
 *                                  MAC network ports 240..255   Chiplet 15
 */
typedef struct {
    GT_U32     ledClockFrequency;
    GT_U32     ledStart[PDL_LEDSTREAM_CHIPLETS_MAX_NUM_CNS];
    GT_U32     ledEnd[PDL_LEDSTREAM_CHIPLETS_MAX_NUM_CNS];
}PDL_LED_SIP6_CONF_STC;

typedef struct PDL_LED_STEAM_INTERFACE_CONF_STCT
{
    PDL_LED_ORDER_MODE_ENT          ledOrganize;
    BOOLEAN                         disableOnLinkDown;
    PDL_LED_BLINK_DUTY_CYCLE_ENT    blink0DutyCycle;
    PDL_LED_BLINK_DURATION_ENT      blink0Duration;
    PDL_LED_BLINK_DUTY_CYCLE_ENT    blink1DutyCycle;
    PDL_LED_BLINK_DURATION_ENT      blink1Duration;
    PDL_LED_PULSE_STRETCH_ENT       pulseStretch;
    GT_U32                          ledStart;
    GT_U32                          ledEnd;
    BOOLEAN                         clkInvert;
    PDL_LED_CLASS_5_SELECT_ENT      class5select;
    PDL_LED_CLASS_13_SELECT_ENT     class13select;
    BOOLEAN                         invertEnable;
    PDL_LED_CLOCK_OUT_FREQUENCY_ENT ledClockFrequency;
    PDL_LED_SIP6_CONF_STC           sip6LedConfig;
} PDL_LED_STEAM_INTERFACE_CONF_STC;

/**
* @enum PDL_LED_PORT_TYPE_ENT
 *
 * @brief Port type to which the class manipulation
 * or group configuration is relevant.
*/
typedef enum{
    /** @brief Tri-speed port.
     *  For xCat3: Network port.
     */
    PDL_LED_PORT_TYPE_TRI_SPEED_E,
    /** @brief XG port.
     *  For xCat3: Flex port.
     */
    PDL_LED_PORT_TYPE_XG_E
} PDL_LED_PORT_TYPE_ENT;

/**
* @enum PDL_LED_BLINK_SELECT_ENT
 *
 * @brief Blink types to select
*/
typedef enum{
    /** Blink 0 signal */
    PDL_LED_BLINK_SELECT_0_E,
    /** Blink 1 signal */
    PDL_LED_BLINK_SELECT_1_E
} PDL_LED_BLINK_SELECT_ENT;

typedef struct {
    GT_U32                     invertEnable;
    GT_U32                     blinkEnable;
    PDL_LED_BLINK_SELECT_ENT   blinkSelect;
    GT_U32                     forceEnable;
    GT_U32                     forceData;
    GT_U32                     pulseStretchEnable;
    GT_U32                     disableOnLinkDown;
} PDL_LED_CLASS_MANIPULATION_STC;

typedef struct {
    PDL_LED_CLASS_MANIPULATION_STC      classManipulation;
    GT_U32                              classNum;
    PDL_LED_PORT_TYPE_ENT               portType;
} PDL_LED_CLASS_MANIPULATION_INFO_STC;

typedef struct {
    UINT_32                 manipulationIndex;
} PDL_LED_CLASS_MANIPULATION_KEY_STC;


/* @}*/
/* @}*/

#endif
