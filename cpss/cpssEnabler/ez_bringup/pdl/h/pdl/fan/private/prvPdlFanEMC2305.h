/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\fan\private\prvPdlFanEMC2305.h.
 *
 * @brief   Declares the prvpdlfan class
 */

#ifndef __prvPdlFanEMC2305h

#define __prvPdlFanEMC2305h
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

extern PDL_FAN_PRV_CALLBACK_FUNC_STC   pdlFanCallbacksEMC2305;

/** 
 * @defgroup Fan_private Fan Private
 * @{Fan private definitions and declarations including:
*/

#define PDL_FAN_EMC2305_MAX_NUM_OF_FANS_CNS                 5

#define PDL_FAN_EMC2305_STATUS_REG_OFFSET_CNS               0x24
#define PDL_FAN_EMC2305_STATUS_REG_FAN_STALL_BIT_MASK_CNS   1
#define PDL_FAN_EMC2305_STATUS_REG_FAN_SPIN_BIT_MASK_CNS    2
#define PDL_FAN_EMC2305_STATUS_REG_FAN_DRIVE_BIT_MASK_CNS   4

#define PDL_FAN_EMC2305_STATUS_STALL_REG_OFFSET_CNS         0x25
#define PDL_FAN_EMC2305_STATUS_SPIN_REG_OFFSET_CNS          0x26
#define PDL_FAN_EMC2305_STATUS_DRIVE_FAIL_REG_OFFSET_CNS    0x27

#define PDL_FAN_EMC2305_STATUS_REG_FAN_ONE_BIT_MASK_CNS     1
#define PDL_FAN_EMC2305_STATUS_REG_FAN_TWO_BIT_MASK_CNS     2
#define PDL_FAN_EMC2305_STATUS_REG_FAN_THREE_BIT_MASK_CNS   4
#define PDL_FAN_EMC2305_STATUS_REG_FAN_FOUR_BIT_MASK_CNS    8
#define PDL_FAN_EMC2305_STATUS_REG_FAN_FIVE_BIT_MASK_CNS    16

#define PDL_FAN_EMC2305_INTERRUPT_REG_OFFSET_CNS            0x29

#define PDL_FAN_EMC2305_DUTY_CYCLE_BASE_REG_OFFSET_CNS      0x30  

#define PDL_FAN_EMC2305_DUTY_CYCLE_REGISTER_OFFSET_MAC(fan_num) \
      ((fan_num-1)*0x10)+PDL_FAN_EMC2305_DUTY_CYCLE_BASE_REG_OFFSET_CNS

#define PDL_FAN_EMC2305_DUTY_CYCLE_100_PERCENT_VAL_CNS      0xff
#define PDL_FAN_EMC2305_DUTY_CYCLE_OFF_VAL_CNS              0
#define PDL_FAN_EMC2305_DUTY_CYCLE_MAX_RPM_VAL_CNS          12350

/**
 * @struct  PDL_FAN_TC_654_INTERFACE_STC
 *
 * @brief   defines interface for working with a fan (interface information) + predefined values
 */

typedef struct {

    PDL_INTERFACE_TYPE_ENT                  interfaceType;    
    PDL_INTERFACE_TYP                       interfaceId;
} PDL_FAN_ENC2305_INTERFACE_STC;

typedef struct {
    UINT_32                     dutyCycle_last_config;
    PDL_FAN_STATE_CONTROL_ENT   config_state[PDL_FAN_EMC2305_MAX_NUM_OF_FANS_CNS+1]; /* starts from 1 */
} PDL_FAN_ENC2305_DATA_STC;

/**
 * @struct  PDL_FAN_PRV_KEY_STC
 *
 * @brief   defines structure for fan key in hash
 */

typedef struct {
    /** @brief   Identifier for the fan */
    UINT_32                        controllerId;
} PDL_FAN_EMC2305_PRV_KEY_STC;


/**
 * @struct  PDL_FAN_PRV_DB_STC
 *
 * @brief   defines structure stored for fan in hash
 */

typedef struct {    
    /** @brief   The fan interface */
    PDL_FAN_ENC2305_INTERFACE_STC                 fanInterface;
    PDL_FAN_ENC2305_DATA_STC                      fanData;  
    /** @brief   The fan capabilities */
    PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC                  fanCapabilities;
    PDL_FAN_EMC2305_PRV_KEY_STC                   fanKey;
} PDL_FAN_EMC2305_PRV_DB_STC;




/**
 * @fn  PDL_STATUS prvPdlFanEMC2305XMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan XML parser
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305XMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  UINT_32                             fanControllerId
);

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305DbInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan initialize
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305DbInit (
    IN  void
);

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305HwGetRpm ()
 *
 * @brief   get rpm for EMC 2305
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] rpmPtr              - rpm for current fan
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305HwGetRpm (
    IN  UINT_32                           fanControllerId,
    OUT UINT_32                         * rpmPtr
);

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305HwStatusGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT PDL_FAN_STATE_INFO_ENT * statePtr )
 *
 * @brief   Pdl fan hardware state get
 *
 * @param           fanControllerId         Identifier for the fan controller.
 * @param           fanNumber               Identifier for the fan.
 * @param [in,out]  statePtr                If non-null, the state pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305HwStatusGet (
    IN  UINT_32                     fanControllerId,
    IN  UINT_32                     fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT    * statePtr
);

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305HwStateSet ( IN UINT_32 fanControllerId, IN PDL_FAN_STATE_CONTROL_ENT state )
 *
 * @brief   Pdl fan hardware state set
 *
 * @param   fanControllerId         Identifier for the fan controller.
 * @param   state                   The state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305HwStateSet (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      fanNumber,
    IN PDL_FAN_STATE_CONTROL_ENT    state
);

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305HwDutyCycleSet ( IN UINT_32 fanControllerId, IN UINT_32 speedRelativeVal )
 *
 * @brief   Pdl fan hardware duty cycle set
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [in]  speedRelativeVal        The speed relative value.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305HwDutyCycleSet ( 
    IN UINT_32      fanControllerId,
    IN UINT_32      speedAbsoulteVal
);

PDL_STATUS prvPdlFanEMC2305HwDutyCycleGet ( 
    IN UINT_32      fanControllerId,
    IN UINT_32      fanNumber,
    OUT UINT_32     *speedval_PTR,
    OUT UINT_32     *speedPercentPtr
);

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305HwInit ( IN UINT_32 fanControllerId )
 *
 * @brief   Pdl fan hardware init
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305HwInit ( 
    IN UINT_32                  fanControllerId
);

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305GetType ( IN UINT_32 fanControllerId);
 *
 * @brief   Init fan controller
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] controllerTypePtr   - fan controller type (i.e tc654/adt7476)
 *
 * @return  
 */
PDL_STATUS prvPdlFanEMC2305GetType (
    IN  UINT_32                           fanControllerId,
    OUT PDL_FAN_CONTROLLER_TYPE_ENT * controllerTypePtr
);


/* @}*/
/* @}*/
/* @}*/

#endif
