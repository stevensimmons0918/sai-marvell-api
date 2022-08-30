/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\fan\pdlfan.h.
 *
 * @brief   Declares the pdlfan class
 */

#ifndef __pdlFanh

#define __pdlFanh
/**
********************************************************************************
 * @file pdlFan.h   
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
 * @brief Platform driver layer - Fan related API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/init/pdlInit.h>

 /** 
 * @defgroup Fan Fan
 * @{Fan driver definitions and declarations including:
 * - Status set/get
 * - Speed set/get
 * - Direction set/get 
 * - Threshold get
*/

/**
 * @enum    PDL_FAN_STATE_CONTROL_ENT
 *
 * @brief   Enumerator for fan state control
 */

typedef enum {
    PDL_FAN_STATE_CONTROL_NORMAL_E = 0,
    PDL_FAN_STATE_CONTROL_SHUTDOWN_E,
    PDL_FAN_STATE_CONTROL_LAST_E,
}PDL_FAN_STATE_CONTROL_ENT;

/**
 * @enum    PDL_FAN_STATE_INFO_ENT
 *
 * @brief   Enumerator for fan state info
 */

typedef enum {
    PDL_FAN_STATE_INFO_OK_E = 0,
    /* fan cycle is too fast */
    PDL_FAN_STATE_INFO_OVERFLOW_E,
    /* fan fault - probably disconnected */
    PDL_FAN_STATE_INFO_FAULT_E,
    /* redundant fan which is ok but currently not in use */
    PDL_FAN_STATE_INFO_READY_E,
    PDL_FAN_STATE_INFO_LAST_E,
}PDL_FAN_STATE_INFO_ENT;

/**
 * @enum    PDL_FAN_DIRECTION_ENT
 *
 * @brief   Enumerator for fan direction
 */

typedef enum {
    PDL_FAN_DIRECTION_RIGHT_E = 0,
    PDL_FAN_DIRECTION_LEFT_E,
    PDL_FAN_DIRECTION_LAST_E
} PDL_FAN_DIRECTION_ENT;

/**
 * @enum    PDL_FAN_DUTY_CYCLE_METHOD_ENT
 *
 * @brief   Enumerator for fan duty cycle method
 */

typedef enum {
    PDL_FAN_DUTY_CYCLE_METHOD_HW_E = 0,
    PDL_FAN_DUTY_CYCLE_METHOD_SW_E,
    PDL_FAN_DUTY_CYCLE_METHOD_LAST_E
} PDL_FAN_DUTY_CYCLE_METHOD_ENT;

/**
 * @enum    PDL_FAN_FAULT_CONTROL_ENT
 *
 * @brief   Enumerator for fan duty cycle method
 */

typedef enum {
    /* normal operation - default value for fan fault clear bit */
    PDL_FAN_FAULT_NORMAL_E = 0,
    /* in order to recover from fault in TC654 need to clear fault bit */
    PDL_FAN_FAULT_CLEAR_E,
    PDL_FAN_FAULT_LAST_E
} PDL_FAN_FAULT_ENT;

typedef enum {
    PDL_FAN_ROLE_NORMAL_E,
    PDL_FAN_ROLE_REDUNDANT_E,
    PDL_FAN_ROLE_LAST_E
}PDL_FAN_ROLE_ENT;


typedef enum {
    PDL_FAN_OP_TYPE_INITIALIZE_REGISTER_E = 0,
    PDL_FAN_OP_TYPE_HIGH_THRESHOLD_REGISTER_E,
    PDL_FAN_OP_TYPE_LOW_THRESHOLD_REGISTER_E,
    PDL_FAN_OP_TYPE_LAST_E
}PDL_FAN_OP_TYPE_ENT;

typedef enum {
    PDL_FAN_CONTROLLER_TYPE_TC654_E = 0,
    PDL_FAN_CONTROLLER_TYPE_ADT7476_E,
    PDL_FAN_CONTROLLER_TYPE_EMC2305_E,
    PDL_FAN_CONTROLLER_TYPE_PWM_E,
    PDL_FAN_CONTROLLER_TYPE_USER_DEFINED_E,
    PDL_FAN_CONTROLLER_TYPE_LAST_E
} PDL_FAN_CONTROLLER_TYPE_ENT;

typedef enum {
    PDL_FAN_THRESHOLD_STATE_NORMAL_E,
    PDL_FAN_THRESHOLD_STATE_WARNING_E,
    PDL_FAN_THRESHOLD_STATE_CRITICAL_E,
    PDL_FAN_THRESHOLD_STATE_LAST_E
}PDL_FAN_THRESHOLD_STATE_ENT;

typedef struct {
    UINT_32                        sensorNumber;
} PDL_SW_FAN_SENSOR_THRESHOLD_KEY_STC;

typedef struct {
    PDL_SW_FAN_SENSOR_THRESHOLD_KEY_STC     sensorThresholdKey;
    UINT_8                                  sensorThresholdTemprature;
} PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC;

typedef struct {        
    BOOLEAN     stateControlIsSupported;
    BOOLEAN     stateInfoIsSupported;
    BOOLEAN     dutyCycleControlIsSupported;
    BOOLEAN     dutyCycleInfoIsSupported;
    BOOLEAN     dutyCycleMethodControlIsSupported;
    BOOLEAN		rotationDirectionControlIsSupported;
    BOOLEAN     faultControlIsSupported;
    BOOLEAN     pulsesPerRotationControlIsSupported;
    BOOLEAN		thresholdControlIsSupported;
    BOOLEAN     rpmInfoSupported;
} PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC;

typedef PDL_STATUS pdlFanDbCapabilitiesGet_FUN (
    IN UINT_32                                      fanControllerId,
    OUT PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC  *  capabilitiesPtr
);

typedef PDL_STATUS pdlFanHwStatusGet_FUN (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT    * statePtr
);

typedef PDL_STATUS pdlFanHwStateSet_FUN (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      fanNumber,
    IN  PDL_FAN_STATE_CONTROL_ENT   state
);

typedef PDL_STATUS pdlFanHwDutyCycleGet_FUN (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      fanNumber,
    OUT UINT_32                   * speedPtr,
    OUT UINT_32                   * speedPercentPtr
);

typedef PDL_STATUS pdlFanControllerHwDutyCycleSet_FUN (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      speed
);

typedef PDL_STATUS pdlFanControllerHwRotationDirectionGet_FUN (
    IN UINT_32                      fanControllerId,
    OUT PDL_FAN_DIRECTION_ENT     * directionPtr
);

typedef PDL_STATUS pdlFanControllerHwRotationDirectionSet_FUN (
    IN UINT_32                      fanControllerId,
    IN PDL_FAN_DIRECTION_ENT        direction
);

typedef PDL_STATUS pdlFanControllerHwDutyCycleMethodSet_FUN (
    IN UINT_32                       fanControllerId,
    IN PDL_FAN_DUTY_CYCLE_METHOD_ENT duthCycleMethod
);

typedef PDL_STATUS pdlFanControllerHwFaultSet_FUN (
    IN UINT_32                       fanControllerId,
    IN PDL_FAN_FAULT_ENT             faultControl
);

typedef PDL_STATUS pdlFanHwPulsesPerRotationSet_FUN (
    IN UINT_32                       fanControllerId,
    IN UINT_32                       fanNumber,
    IN UINT_32                       pulsesPerRotation
);

typedef PDL_STATUS pdlFanHwThresholdSet_FUN (
    IN UINT_32                       fanControllerId,
    IN UINT_32                       fanNumber,
    IN UINT_32                       threshold
);

typedef PDL_STATUS pdlFanControllerHwInit_FUN (
    IN  UINT_32                      fanControllerId
);


typedef PDL_STATUS pdlFanControllerGetType_FUN (
    IN  UINT_32                        fanControllerId,
    OUT PDL_FAN_CONTROLLER_TYPE_ENT  * controllerTypePtr
);

typedef PDL_STATUS pdlFanDbInit_FUN (
    IN  void
);

typedef PDL_STATUS pdlFanControllerHwRpmGet_FUN (
    IN  UINT_32                      fanControllerId,
    OUT UINT_32                    * rpmPtr
);


typedef struct {
    pdlFanHwStatusGet_FUN                           * fanHwStatusGetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CALLBACK_HW_STATUS_GET_INFO_STC;
typedef struct {
    pdlFanHwStateSet_FUN                            * fanHwStateSetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CALLBACK_HW_STATE_SET_INFO_STC;
typedef struct {
    pdlFanHwDutyCycleGet_FUN                        * fanHwDutyCycleGetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CALLBACK_HW_DUTY_CYCLE_GET_INFO_STC;
typedef struct {
    pdlFanControllerHwDutyCycleSet_FUN              * fanControllerHwDutyCycleSetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CONTROLLER_CALLBACK_HW_DUTY_CYCLE_SET_INFO_STC;
typedef struct {
    pdlFanControllerHwRotationDirectionGet_FUN      * fanControllerHwRotationDirectionGetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CONTROLLER_CALLBACK_HW_ROTATION_DIRECTION_GET_INFO_STC;
typedef struct {
    pdlFanControllerHwRotationDirectionSet_FUN      * fanControllerHwRotationDirectionSetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CONTROLLER_CALLBACK_HW_ROTATION_DIRECTION_SET_INFO_STC;
typedef struct {
    pdlFanControllerHwDutyCycleMethodSet_FUN        * fanControllerHwDutyCycleMethodSetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CONTROLLER_CALLBACK_HW_DUTY_CYCLE_METHOD_SET_INFO_STC;
typedef struct {
    pdlFanControllerHwFaultSet_FUN                  * fanControllerHwFaultSetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CONTROLLER_CALLBACK_HW_FAULT_SET_INFO_STC;
typedef struct {
    pdlFanHwPulsesPerRotationSet_FUN                * fanHwPulsesPerRorationSetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CALLBACK_HW_PULSES_PER_ROTATION_SET_INFO_STC;
typedef struct {
    pdlFanHwThresholdSet_FUN                        * fanHwThresholdSetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CALLBACK_HW_THRESHOLD_SET_INFO_STC;
typedef struct {
    pdlFanControllerHwInit_FUN                      * fanControllerHwInitFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CALLBACK_HW_INIT_INFO_STC;
typedef struct {
    pdlFanDbInit_FUN                                * fanDbInitFun;
} PDL_FAN_CALLBACK_DB_INIT_INFO_STC;
typedef struct {
    pdlFanControllerHwRpmGet_FUN                    * fanControllerRpmGetFun;
    PDL_OS_LOCK_TYPE_ENT                              lockType;
} PDL_FAN_CONTROLLER_CALLBACK_HW_RPM_GET_STC;

typedef struct {
    PDL_FAN_CALLBACK_HW_STATUS_GET_INFO_STC                                 fanHwStatusGetCallbackInfo;
    PDL_FAN_CALLBACK_HW_STATE_SET_INFO_STC                                  fanHwStateSetCallbackInfo;
    PDL_FAN_CALLBACK_HW_DUTY_CYCLE_GET_INFO_STC                             fanControllerHwDutyCycleGetCallbackInfo;
    PDL_FAN_CONTROLLER_CALLBACK_HW_DUTY_CYCLE_SET_INFO_STC                  fanControllerHwDutyCycleSetCallbackInfo;
    PDL_FAN_CONTROLLER_CALLBACK_HW_ROTATION_DIRECTION_GET_INFO_STC          fanControllerHwRotationDirectionGetCallbackInfo;
    PDL_FAN_CONTROLLER_CALLBACK_HW_ROTATION_DIRECTION_SET_INFO_STC          fanControllerHwRotationDirectionSetCallbackInfo;
    PDL_FAN_CONTROLLER_CALLBACK_HW_DUTY_CYCLE_METHOD_SET_INFO_STC           fanControllerHwDutyCycleMethodSetCallbackInfo;
    PDL_FAN_CONTROLLER_CALLBACK_HW_FAULT_SET_INFO_STC                       fanControllerFaultSetCallbackInfo;
    PDL_FAN_CALLBACK_HW_PULSES_PER_ROTATION_SET_INFO_STC                    fanHwPulsesPerRotationSetCallbackInfo;
    PDL_FAN_CALLBACK_HW_THRESHOLD_SET_INFO_STC                              fanHwThresholdSetCallbackInfo;
    PDL_FAN_CALLBACK_HW_INIT_INFO_STC                                       fanHwInitCallbackInfo;
    PDL_FAN_CALLBACK_DB_INIT_INFO_STC                                       fanDbInitCallbackInfo;
    PDL_FAN_CONTROLLER_CALLBACK_HW_RPM_GET_STC                              fanHwControllerRpmGetInfo;
} PDL_FAN_CALLBACK_INFO_STC;

/* ***************************************************************************
* FUNCTION NAME: pdlFanBindExternalFanDriver
*
* DESCRIPTION:  binds new external fan driver, function that aren't implemented by specific fan driver
*               should be NULL pointers
*
* PARAMETERS:   
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*                
*****************************************************************************/


PDL_STATUS pdlFanBindExternalFanDriver(
    IN PDL_FAN_CALLBACK_INFO_STC               * callbacksInfoPtr,
    IN PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP      externalDriverId
);

/*-------------------------------------------------------------------------------------------------------------*/

/**
 * @fn  PDL_STATUS convertFanControllerAndNumberToFanId
 *
 * @brief   convert fanController ID + fanNumber to fanId
 *          logical fan id is unique across all fan controllers
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in ] fanControllerId      - fan controller id
 * @param [in ] fanNumber            - fan number
 * @param [out] fanIdPtr             - fan id (unique id used by upper layer app)
 *
 * @return  
 */
PDL_STATUS convertFanControllerAndNumberToFanId(
    IN  UINT_32       fanControllerId,
    IN  UINT_32       fanNumber,
    OUT UINT_32     * fanIdPtr
);

/**
 * @fn  PDL_STATUS pdlFanIdToFanControllerAndNumber
 *
 * @brief   convert logical fan id to fanController ID + fanId 
 *          logical fan id is unique across all fan controllers
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in ]   fanId                - fan id (unique id used by upper layer app)
 * @param [out]   fanControllerIdPtr   - fan controller id
 * @param [out]   fanNumberPtr         - fan number
 *
 * @return  
 */
PDL_STATUS pdlFanIdToFanControllerAndNumber(
    IN  UINT_32       fanId,
    OUT UINT_32     * fanControllerIdPtr,
    OUT UINT_32     * fanNumberPtr
);

/**
 * @fn  PDL_STATUS pdlFanDbGetFirst
 *
 * @brief   return the id of the first connected fan from the first fan controller
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [out]   fanIdPtr      - fan id (unique id used by upper layer app)
 *
 * @return  
 */
PDL_STATUS pdlFanDbGetFirst(
    OUT UINT_32     * fanIdPtr
);

/**
 * @fn  PDL_STATUS pdlFanDbGetNext
 *
 * @brief   return the logical Id of the next connected fan after the fan with the param fan Id 
 *          if exist next fan in this connected fan db
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in ]   fanId         - current fan id
 * @param [out]   fanIdPtr      - next fan id (unique id used by upper layer app)
 *
 * @return  
 */
PDL_STATUS pdlFanDbGetNext(
    IN  UINT_32       fanId,
    OUT UINT_32     * fanIdPtr
);

/**
 * @fn  PDL_STATUS pdlFanDbGetNumOfConnectedFans
 *
 * @brief   return the number of fans in module         
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [out]   numOfFansPtr      - number of fans in module
 *
 * @return  
 */
PDL_STATUS pdlFanDbGetNumOfConnectedFans(
    OUT UINT_32                       *  numOfFansPtr
);

/**
 * @fn  PDL_STATUS pdlFanDbGetFirstController
 *
 * @brief   return the fan controller Id of the first fan controller from the db 
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [out]   fanControllerIdPtr     - fan controller Id
 *
 * @return  
 */
PDL_STATUS pdlFanDbGetFirstController(
    OUT UINT_32     * fanControllerIdPtr
);

/**
 * @fn  PDL_STATUS pdlFanDbGetNextController
 *
 * @brief   return the next fan controller Id  from the db 
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in ]   fanControllerIdPtr     - current fan controller Id
 * @param [out]   fanControllerIdPtr     - next fan controller Id
 *
 * @return  
 */
PDL_STATUS pdlFanDbGetNextController(
    IN  UINT_32       fanControllerId,
    OUT UINT_32     * fanControllerIdPtr
);

/**
 * @fn  PDL_STATUS pdlFanDbCapabilitiesGet ( IN UINT_32 fanControllerId, OUT PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC * capabilitiesPtr )
 *
 * @brief   Pdl fan database capabilities get
 *
 * @param [in ]  fanControllerId           - Identifier for the fan controller
 * @param [out]  capabilitiesPtr           - If non-null, the capabilities pointer
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlFanDbCapabilitiesGet (
    IN  UINT_32                                    fanControllerId,
    OUT PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC *  capabilitiesPtr
);

/**
 * @fn  PDL_STATUS pdlFanHwStatusGet (IN UINT_32 fanControllerId, IN UINT_32 fanNumber , OUT PDL_FAN_STATE_INFO_ENT * statePtr);
 *
 * @brief   Configure fan state (normal/shutdown)
 *
 * @param [in ]  fanControllerId     - fan controller identifier
 * @param [in ]  fanNumber           - fan number(key in the connected fan db)
 * @param [out]  statePtr            - fan state(normal/shutdown)
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlFanHwStatusGet (
    IN  UINT_32                     fanControllerId,
    IN  UINT_32                     fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT    * statePtr
);

/**
 * @fn  PDL_STATUS pdlFanHwStateSet ( IN UINT_32 fanControllerId, IN PDL_FAN_STATE_CONTROL_ENT state )
 *
 * @brief   Pdl fan hardware state set
 *
 * @param   fanControllerId         - Identifier for the fan controller
 * @param   state                   - The state
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlFanHwStateSet (
    IN  UINT_32                     fanControllerId,
    IN  UINT_32                     fanNumber,
    IN  PDL_FAN_STATE_CONTROL_ENT   state
);

/**
 * @fn  PDL_STATUS pdlFanHwDutyCycleGet ( IN UINT_32 fanControllerId, OUT UINT_32 * speedPtr )
 *
 * @brief   Pdl fan hardware duty cycle get
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [out] speedPtr                If non-null, the speed pointer.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlFanHwDutyCycleGet (
    IN  UINT_32                      fanControllerId,
    IN  UINT_32                      fanNumber,   
    OUT UINT_32                    * speedPtr,
    OUT UINT_32                    * speedPercentPtr
);

/**
 * @fn  PDL_STATUS pdlFanControllerHwDutyCycleSet ( IN UINT_32 fanControllerId, IN UINT_32 speedRelativeVal )
 *
 * @brief   Pdl fan hardware duty cycle set
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [in]  speedRelativeVal        The speed relative value.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlFanControllerHwDutyCycleSet (
    IN  UINT_32                      fanControllerId,
    IN  UINT_32                      speed
);

/**
 * @fn  PDL_STATUS pdlFanControllerHwRotationDirectionGet ( IN UINT_32 fanControllerId, OUT PDL_FAN_DIRECTION_ENT * directionPtr );
 *
 * @brief   Get fan rotation direction (right/left)
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] directionPtr        - fan direction.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlFanControllerHwRotationDirectionGet (
    IN  UINT_32                      fanControllerId,
    OUT PDL_FAN_DIRECTION_ENT      * directionPtr
);

/**
 * @fn  PDL_STATUS pdlFanControllerHwRotationDirectionSet ( IN UINT_32 fanControllerId, IN PDL_FAN_DIRECTION_ENT direction );
 *
 * @brief   Configure fan rotation direction (right/left)
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  direction           - fan direction.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlFanControllerHwRotationDirectionSet (
    IN  UINT_32                      fanControllerId,
    IN  PDL_FAN_DIRECTION_ENT        direction
);

/**
 * @fn  PDL_STATUS pdlFanControllerHwDutyCycleMethodSet ( IN UINT_32 fanControllerId, IN PDL_FAN_DUTY_CYCLE_METHOD_ENT duthCycleMethod );
 *
 * @brief   Configure fan duty cycle method
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  duthCycleMethod     - fan duty cycle method.
 *
 * @return  
 */
PDL_STATUS pdlFanControllerHwDutyCycleMethodSet (
    IN  UINT_32                       fanControllerId,
    IN  PDL_FAN_DUTY_CYCLE_METHOD_ENT duthCycleMethod
);

/**
 * @fn  PDL_STATUS pdlFanControllerHwFaultSet ( IN UINT_32 fanControllerId, IN PDL_FAN_FAULT_CONTROL_ENT faultControl );
 *
 * @brief   Configure fan fault control
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  faultControl        - fan fault control (normal/clear)
 *
 * @return  
 */
PDL_STATUS pdlFanControllerHwFaultSet (
    IN  UINT_32                      fanControllerId,
    IN  PDL_FAN_FAULT_ENT            faultControl
);

/**
 * @fn  PDL_STATUS pdlFanHwPulsesPerRotationSet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, IN UINT_32 pulsesPerRotation );
 *
 * @brief   Configure fan pulses per rotation
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  fanNumber           - fan number.
 * @param [in]  pulsesPerRotation   - fan pulses.
 *
 * @return  
 */
PDL_STATUS pdlFanHwPulsesPerRotationSet (
    IN  UINT_32                      fanControllerId,
    IN  UINT_32                      fanNumber,
    IN  UINT_32                      pulsesPerRotation
);

/**
 * @fn  PDL_STATUS pdlFanHwThresholdSet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, IN UINT_32 threshold )
 *
 * @brief   Pdl fan hardware threshold set
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [in]  fanNumber               Fan number.
 * @param [in]  thresholdRelativeVal    The threshold relative value.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlFanHwThresholdSet (
    IN  UINT_32                      fanControllerId,
    IN UINT_32                       fanNumber,
    IN UINT_32                       threshold
);

/**
 * @fn  PDL_STATUS pdlFanDbRoleGet
 *
 * @brief   get fan role (normal/redundant)
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in] fanControllerId      - fan controller Id
 * @param [in]  fanNumber           - fanNumber
 * @param [out] rolePtr             - role of fan
 *
 * @return  PDL_OK
 * @return  PDL_NOT_FOUND           - fanId not found in database
 * @return  PDL_BAD_PTR             - bad pointer given
 */

PDL_STATUS pdlFanDbRoleGet (
    IN  UINT_32                           fanControllerId,
    IN  UINT_32                           fanNumber,
    OUT PDL_FAN_ROLE_ENT               *  rolePtr
);

/**
 * @fn  PDL_STATUS pdlFanControllerHwInit ( IN UINT_32 fanControllerId);
 *
 * @brief   Init fan controller
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 *
 * @return  
 */
PDL_STATUS pdlFanControllerHwInit (
    IN  UINT_32                           fanControllerId
);

/**
 * @fn  PDL_STATUS pdlFanControllerHwRpmGet ( IN UINT_32 fanControllerId);
 *
 * @brief   Get fan rpms
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] rpmPtr              - rpm for current fan
 *
 * @return  
 */
PDL_STATUS pdlFanControllerHwRpmGet (
    IN  UINT_32                           fanControllerId,
    OUT UINT_32                         * rpmPtr
);


/**
 * @fn  PDL_STATUS pdlFanControllerGetType ( IN UINT_32 fanControllerId);
 *
 * @brief   Init fan controller
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] controllerTypePtr   - fan controller type (i.e tc654/adt7476)
 *
 * @return  
 */
PDL_STATUS pdlFanControllerGetType (
    IN  UINT_32                       fanControllerId,
    OUT PDL_FAN_CONTROLLER_TYPE_ENT * controllerTypePtr
);

/* ***************************************************************************
* FUNCTION NAME: pdlSwFanGetFirstThresholdId
*
* DESCRIPTION:  get first threshold
*
* PARAMETERS:   
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*                
*****************************************************************************/

PDL_STATUS pdlSwFanGetFirstThresholdId (
    OUT UINT_8                          * thresholdFanSpeedPtr
);

/*$ END OF pdlSwFanGetFirstThresholdId */

/* ***************************************************************************
* FUNCTION NAME: pdlSwFanGetNextThresholdId
*
* DESCRIPTION:  get next threshold id
*
* PARAMETERS:   
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*                
*****************************************************************************/

PDL_STATUS pdlSwFanGetNextThresholdId (
    IN  UINT_8                            thresholdFanSpeed,
    OUT UINT_8                          * nextThresholdFanSpeedPtr
);

/*$ END OF pdlSwFanGetNextThresholdId */

/* ***************************************************************************
* FUNCTION NAME: pdlSwFanGetThresholdNumber
*
* DESCRIPTION:  get number of thresholds
*
* PARAMETERS:   
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*                
*****************************************************************************/

PDL_STATUS pdlSwFanGetThresholdNumber (
    OUT UINT_8                          * thresholdNumPtr
);

/*$ END OF pdlSwFanGetThresholdNumber */

/* ***************************************************************************
* FUNCTION NAME: pdlSwFanGetFirstSensorThreshold
*
* DESCRIPTION:  get first sensor for given threshold id
*
* PARAMETERS:   
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*                
*****************************************************************************/

PDL_STATUS pdlSwFanGetFirstSensorThreshold (
    IN  UINT_8                                       thresholdFanSpeed,
    OUT PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC      * sensorThresholdInfoPtr
);

/*$ END OF pdlSwFanGetFirstSensorThreshold */

/* ***************************************************************************
* FUNCTION NAME: pdlSwFanGetNextSensorThreshold
*
* DESCRIPTION:  get next sensor threshold for given threshold id
*
* PARAMETERS:   
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*                
*****************************************************************************/

PDL_STATUS pdlSwFanGetNextSensorThreshold (
    IN  UINT_8                                       thresholdFanSpeed,
    IN  PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC      * currentSensorThresholdInfoPtr,
    OUT PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC      * nextSensorThresholdInfoPtr
);

/*$ END OF pdlSwFanGetNextSensorThreshold */

/* ***************************************************************************
* FUNCTION NAME: pdlSwFanIsSupported
*
* DESCRIPTION:  is SW fan supported
*
* PARAMETERS:   
*
* RETURNES:     TRUE                        -       SW fan information exists in XML
*               FALSE                       -       SW fan information doesn't exist in XML
*                
*****************************************************************************/

BOOLEAN pdlSwFanIsSupported (
);

/*$ END OF pdlSwFanIsSupported */

/* @}*/

#endif

