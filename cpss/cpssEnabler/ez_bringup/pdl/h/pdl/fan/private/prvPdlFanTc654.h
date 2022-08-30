/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\fan\private\prvpdlfanTc654.h.
 *
 * @brief   Declares the prvpdlfan class
 */

#ifndef __prvPdlFanTc654h

#define __prvPdlFanTc654h
/**
********************************************************************************
 * @file prvPdlFanTc654.h   
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
 * @brief Platform driver layer - Fan Tc654 private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/fan/pdlFan.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/xml/private/prvXmlParser.h>

/**
* @addtogroup FanTc654
* @{
*/

/**
* @addtogroup prvFanTc654
* @{
*/


/**
 * @struct  PDL_FAN_TC654_VALUES_STC
 *
 * @brief   defines structure that hold all device specific values
 */

typedef struct {
    /** @brief   The state normal value */
    UINT_32                             stateNormalValue;  /* relevant only for statusControlInfo */
    /** @brief   The state shutdown value */
    UINT_32                             stateShutdownValue;/* relevant only for statusControlInfo */
    /** @brief   The left rotation value */
    UINT_32                             leftRotationValue;
    /** @brief   The right rotation value */
    UINT_32                             rightRotationValue;
    /** @brief   The duty cycle method hardware */
    UINT_32                             dutyCycleMethodHw;
    /** @brief   The duty cycle method software */
    UINT_32                             dutyCycleMethodSw;
    /** @brief   The state ok value */
    UINT_32                             stateOkValue;      /* relevant only for statusInfo */   
    /** @brief   The state overflow value */
    UINT_32                             stateOverflowValue;/* relevant only for statusInfo */
    /** @brief   The state fault value */
    UINT_32                             stateFaultValue;   /* relevant only for statusInfo */
    /** @brief   The fault normal value */
    UINT_32                             faultNormalValue;  /* relevant only for faultControlInfo */
    /** @brief   The fault clear value */
    UINT_32                             faultClearValue;   /* relevant only for faultControlInfo */
} PDL_FAN_TC654_VALUES_STC;

extern PDL_FAN_PRV_CALLBACK_FUNC_STC   pdlFanCallbacksTc654;

/** 
 * @defgroup Fan_private Fan Private
 * @{Fan private definitions and declarations including:
*/

/*! Main tag list of fans !*/

/*! Tag for State control information !*/
#define PDL_FAN_STATE_CONTROL_NAME    "StateInfoControl"

/*! Tag for State read information !*/
#define PDL_FAN_STATE_INFO_NAME       "StateInfo"

/*! Tag for Speed read information !*/
#define PDL_FAN_SPEED_INFO_NAME       "SpeedInfo"

/*! Tag for Speed control information !*/
#define PDL_FAN_DUTY_CYCLE_NAME    "DutyCycle"

/*! Tag for Threshold control information !*/
#define PDL_FAN_THRESHOLD_CONTROL_NAME    "ThresholdInfoControl"

/*! Tag for Rotation control information !*/
#define PDL_FAN_ROTATION_CONTROL_NAME "RotationDirectionControl"

/*! Tag for Pulses per rotation control information !*/
#define PDL_FAN_PULSES_PER_ROTATION_CONTROL_NAME "PulsesPerRotationInfoControl"

/*! Tag for Duty cycle method control information !*/
#define PDL_FAN_DUTY_CYCLE_METHOD_NAME "DutyCycleMethod"

/*! Tag for Fault control information !*/
#define PDL_FAN_FAULT_CONTROL_NAME    "FaultInfoControl"

/*! Tag for State Normal value !*/
#define PDL_FAN_VALUES_STATE_NORMAL_NAME  "StateNormalValue"

/*! Tag for State Shutdown value !*/
#define PDL_FAN_VALUES_STATE_SHUTDOWN_NAME  "StateShutdownValue"

/*! Tag for rotating Left value !*/
#define PDL_FAN_VALUES_LEFT_NAME      "LeftRotaionValue"

/*! Tag for rotating Right value !*/
#define PDL_FAN_VALUES_RIGHT_NAME     "RightRotaionValue"

/*! Tag for  value !*/
#define PDL_FAN_VALUES_METHOD_HW_NAME "DutyCycleMethodHw"
/*! Tag for  value !*/
#define PDL_FAN_VALUES_METHOD_SW_NAME "DutyCycleMethodSw"
/*! Tag for State Ok value !*/
#define PDL_FAN_VALUES_STATE_OK_NAME  "StateOkValue"
/*! Tag for State Overflow value !*/
#define PDL_FAN_VALUES_STATE_OVERFLOW_NAME  "StateOverflowValue"
/*! Tag for State Fault value !*/
#define PDL_FAN_VALUES_STATE_FAULT_NAME  "StateFaultValue"
/*! Tag for Fault Normal value !*/
#define PDL_FAN_VALUES_FAULT_NORMAL_NAME "FaultNormalValue"
/*! Tag for Fault Clear value !*/
#define PDL_FAN_VALUES_FAULT_CLEAR_NAME "FaultClearValue"


#define PDL_FAN_TC654_FAULT_FAN1_REGISTER_OFFSET_CNS    2
#define PDL_FAN_TC654_FAULT_FAN2_REGISTER_OFFSET_CNS    3
#define PDL_FAN_TC654_CONFIG_REGISTER_OFFSET_CNS        4
#define PDL_FAN_TC654_STATUS_REGISTER_OFFSET_CNS        5
#define PDL_FAN_TC654_DUTY_CYCLE_REGISTER_OFFSET_CNS    6

/**
 * @struct  PDL_FAN_TC_654_INTERFACE_STC
 *
 * @brief   defines interface for working with a fan (interface information) + predefined values
 */

typedef struct {

    PDL_INTERFACE_TYPE_ENT                  interfaceType;    
    PDL_INTERFACE_TYP                       interfaceId;
} PDL_FAN_TC654_INTERFACE_STC;


/**
 * @struct  PDL_FAN_PRV_KEY_STC
 *
 * @brief   defines structure for fan key in hash
 */

typedef struct {
    /** @brief   Identifier for the fan */
    UINT_32                        fanId;
} PDL_FAN_TC654_PRV_KEY_STC;


/**
 * @struct  PDL_FAN_PRV_DB_STC
 *
 * @brief   defines structure stored for fan in hash
 */

typedef struct {    
    /** @brief   The fan capabilities */
    PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC                  fanCapabilities;
    PDL_FAN_TC654_PRV_KEY_STC                     fanKey;
} PDL_FAN_TC654_PRV_DB_STC;




/**
 * @fn  PDL_STATUS prvPdlFanTc654XMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan XML parser
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654XMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  UINT_32                             fanControllerId
);

/**
 * @fn  PDL_STATUS prvPdlFanTc654DbInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId );
 *
 * @brief   Init fan module Create button DB and initialize
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654DbInit (
    IN  void
);


/**
 * @fn  PDL_STATUS prvPdlFanTc654HwStatusGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT PDL_FAN_STATE_INFO_ENT * statePtr )
 *
 * @brief   Pdl fan hardware state get
 *
 * @param           fanControllerId         Identifier for the fan controller.
 * @param           fanNumber               Identifier for the fan.
 * @param [in,out]  statePtr                If non-null, the state pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwStatusGet (
    IN  UINT_32                     fanControllerId,
    IN  UINT_32                     fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT    * statePtr
);

/**
 * @fn  PDL_STATUS prvPdlFanTc654HwStateSet ( IN UINT_32 fanControllerId, IN PDL_FAN_STATE_CONTROL_ENT state );
 *
 * @brief   Configure fan state (normal/shutdown)
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  state               - state to configure.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwStateSet (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      fanNumber,
    IN PDL_FAN_STATE_CONTROL_ENT    state
);

/**
 * @fn  PDL_STATUS prvPdlFanTc654HwDutyCycleGet ( IN UINT_32 fanControllerId, OUT UINT_32 * speedPtr );
 *
 * @brief   Get fan speed
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] speedPtr            - fan speed.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwDutyCycleGet (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      fanNumber,
    OUT UINT_32                   * speedPtr,
    OUT UINT_32                   * speedPercentPtr
);

/**
 * @fn  PDL_STATUS prvPdlFanTc654HwDutyCycleSet ( IN UINT_32 fanControllerId, IN UINT_32 speed );
 *
 * @brief   Configure fan speed 0-255), only if method is Sw
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  speed               - fan speed (0-255)
 *
 * @return  TBD.
 */

PDL_STATUS prvPdlFanTc654HwDutyCycleSet (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      speed
);


/**
 * @fn  PDL_STATUS prvPdlFanTc654HwDutyCycleMethodSet ( IN UINT_32 fanControllerId, IN PDL_FAN_DUTY_CYCLE_METHOD_ENT duthCycleMethod );
 *
 * @brief   Configure fan duty cycle method
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  duthCycleMethod     - fan duty cycle method.
 *
 * @return  TBD.
 */

PDL_STATUS prvPdlFanTc654HwDutyCycleMethodSet (
    IN UINT_32                       fanControllerId,
    IN PDL_FAN_DUTY_CYCLE_METHOD_ENT duthCycleMethod
);

/**
 * @fn  PDL_STATUS PdlFanTc654HwFaultSet ( IN UINT_32 fanControllerId, IN PDL_FAN_FAULT_CONTROL_ENT faultControl )
 *
 * @brief   Pdl fan hardware fault set
 *
 * @param [in]  fanControllerId                 Identifier for the fan controller.
 * @param   faultControl                        The fault control.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwFaultSet (
    IN UINT_32                       fanControllerId,
    IN PDL_FAN_FAULT_ENT             faultControl
);

/**
 * @fn  PDL_STATUS prvPdlFanTc654HwPulsesPerRotationSet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, IN UINT_32 pulsesPerRotation );
 *
 * @brief   Configure fan pulses per rotation
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  fanNumber           - fan number.
 * @param [in]  pulsesPerRotation   - fan pulses.
 *
 * @return  TBD.
 */

PDL_STATUS prvPdlFanTc654HwPulsesPerRotationSet (
    IN UINT_32                       fanControllerId,
    IN UINT_32                       fanNumber,
    IN UINT_32                       pulsesPerRotation
);

/**
 * @fn  PDL_STATUS prvPdlFanTc654HwThresholdSet ( IN UINT_32 fanControllerId,IN UINT_32 fanNumber, IN UINT_32 threshold );
 *
 * @brief   Configure fan threshold
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  fanNumber           - fan number.
 * @param       threshold           - The threshold.
 *
 * @return  TBD.
 */

PDL_STATUS prvPdlFanTc654HwThresholdSet (
    IN UINT_32                       fanControllerId,
    IN UINT_32                       fanNumber,
    IN UINT_32                       threshold
);

/**
 * @fn  PDL_STATUS prvPdlFanTc654HwInit ( IN UINT_32 fanControllerId )
 *
 * @brief   Pdl fan hardware init
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwInit ( 
    IN UINT_32                  fanControllerId
);

/**
 * @fn  PDL_STATUS prvPdlFanTc654GetType ( IN UINT_32 fanControllerId);
 *
 * @brief   Init fan controller
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] controllerTypePtr   - fan controller type (i.e tc654/adt7476)
 *
 * @return  
 */
PDL_STATUS prvPdlFanTc654GetType (
    IN  UINT_32                           fanControllerId,
    OUT PDL_FAN_CONTROLLER_TYPE_ENT * controllerTypePtr
);


/* @}*/
/* @}*/
/* @}*/

#endif
