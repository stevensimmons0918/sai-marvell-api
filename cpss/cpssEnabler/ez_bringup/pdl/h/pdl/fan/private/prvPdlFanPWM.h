/* Copyright (C) 2019 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\fan\private\prvPdlFanPWM.h.
 *
 * @brief   Declares the prvpdlfan class
 */

#ifndef __prvPdlFanPWMh

#define __prvPdlFanPWMh
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

extern PDL_FAN_PRV_CALLBACK_FUNC_STC   pdlFanCallbacksPWM;

/** 
 * @defgroup Fan_private Fan Private
 * @{Fan private definitions and declarations including:
*/

#define PDL_FAN_PWM_MAX_NUM_OF_FANS_CNS                 2

#define PDL_FAN_PWM_STATUS_REG_OFFSET_CNS               0x0D
#define PDL_FAN_PWM_STATUS_REG_FANs_Fault_BIT_MASK_CNS  0x18
#define PDL_FAN_PWM_STATUS_REG_FAN1_Fault_BIT_MASK_CNS  0x08
#define PDL_FAN_PWM_STATUS_REG_FAN2_Fault_BIT_MASK_CNS  0x10
#define PDL_FAN_PWM_DUTY_CYCLE_BASE_REG_OFFSET_CNS      0x0E
#define PDL_FAN_PWM_DUTY_CYCLE_BIT_MASK_CNS             0xFF
#define PDL_FAN_PWM_DUTY_CYCLE_FAN1_REG_OFFSET_CNS      (PDL_FAN_PWM_DUTY_CYCLE_BASE_REG_OFFSET_CNS)
#define PDL_FAN_PWM_DUTY_CYCLE_FAN2_REG_OFFSET_CNS      0x0F
#define PDL_FAN_PWM_DUTY_CYCLE_MAX_REG_OFFSET_CNS       0x0D
#define PDL_FAN_PWM_DUTY_CYCLE_MAX_BIT_MASK_CNS         0x02
#define PDL_FAN_PWM_DUTY_CYCLE_OFF_REG_OFFSET_CNS       0x0D
#define PDL_FAN_PWM_DUTY_CYCLE_OFF_BIT_MASK_CNS         0x01
#define PDL_FAN_PWM_DUTY_CYCLE_OFF_VAL_CNS             (PDL_FAN_PWM_DUTY_CYCLE_OFF_BIT_MASK_CNS)

#define PDL_FAN_PWM_DUTY_CYCLE_REGISTER_OFFSET_MAC(fan_num) \
      ((fan_num-1)*0x1)+PDL_FAN_PWM_DUTY_CYCLE_BASE_REG_OFFSET_CNS

#define PDL_FAN_PWM_DUTY_CYCLE_100_PERCENT_BIT_VAL_CNS  (0x96)

#define PDL_FAN_PWM_DUTY_CYCLE_PERCENT_MAC(speed) \
    ((speed*100)/PDL_FAN_PWM_DUTY_CYCLE_100_PERCENT_BIT_VAL_CNS)

/**
 * @struct  PDL_FAN_PWM_INTERFACE_STC
 *
 * @brief   defines interface for working with a fan (interface information) + predefined values
 */

typedef struct {

    PDL_INTERFACE_TYPE_ENT                  interfaceType;    
    PDL_INTERFACE_TYP                       interfaceId;
} PDL_FAN_PWM_INTERFACE_STC;

typedef struct {
    UINT_32                     dutyCycle_last_config;
    PDL_FAN_STATE_CONTROL_ENT   config_state[PDL_FAN_PWM_MAX_NUM_OF_FANS_CNS+1]; /* starts from 1 */
} PDL_FAN_PWM_DATA_STC;

/**
 * @struct  PDL_FAN_PRV_KEY_STC
 *
 * @brief   defines structure for fan key in hash
 */

typedef struct {
    /** @brief   Identifier for the fan */
    UINT_32                        controllerId;
} PDL_FAN_PWM_PRV_KEY_STC;


/**
 * @struct  PDL_FAN_PRV_DB_STC
 *
 * @brief   defines structure stored for fan in hash
 */

typedef struct {    
    /** @brief   The fan interface */
    PDL_FAN_PWM_INTERFACE_STC                 fanInterface;
    PDL_FAN_PWM_DATA_STC                      fanData;  
    /** @brief   The fan capabilities */
    PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC                  fanCapabilities;
    PDL_FAN_PWM_PRV_KEY_STC                   fanKey;
} PDL_FAN_PWM_PRV_DB_STC;




/**
 * @fn  PDL_STATUS prvPdlFanPWMXMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan XML parser
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanPWMXMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  UINT_32                             fanControllerId
);

/**
 * @fn  PDL_STATUS prvPdlFanPWMDbInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan initialize
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanPWMDbInit (
    IN  void
);

/**
 * @fn  PDL_STATUS prvPdlFanPWMHwGetRpm ()
 *
 * @brief   get rpm for EMC 2305
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] rpmPtr              - rpm for current fan
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanPWMHwGetRpm (
    IN  UINT_32                           fanControllerId,
    OUT UINT_32                         * rpmPtr
);

/**
 * @fn  PDL_STATUS prvPdlFanPWMHwStatusGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT PDL_FAN_STATE_INFO_ENT * statePtr )
 *
 * @brief   Pdl fan hardware state get
 *
 * @param           fanControllerId         Identifier for the fan controller.
 * @param           fanNumber               Identifier for the fan.
 * @param [in,out]  statePtr                If non-null, the state pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanPWMHwStatusGet (
    IN  UINT_32                     fanControllerId,
    IN  UINT_32                     fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT    * statePtr
);


/**
 * @fn  PDL_STATUS prvPdlFanPWMHwDutyCycleGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT UINT_32 *speed_val_PTR, OUT UINT_32 *speedPercent_PTR )
 *
 * @brief   Pdl fan hardware duty cycle (speed) get
 *
 * @param           fanControllerId         Identifier for the fan controller.
 * @param           fanNumber               Fan number.
 * @param [out]     speed_val_PTR           sped value read from HW
 * @param [out]     speedPercent_PTR        sped percentage value from HW max speed
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanPWMHwDutyCycleGet (
    IN UINT_32      fanControllerId,
    IN UINT_32      fanNumber,
    OUT UINT_32     *speed_val_PTR,
    OUT UINT_32     *speedPercent_PTR
);


/**
 * @fn  PDL_STATUS prvPdlFanPWMHwInit ( IN UINT_32 fanControllerId )
 *
 * @brief   Pdl fan hardware init
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanPWMHwInit ( 
    IN UINT_32                  fanControllerId
);

/**
 * @fn  PDL_STATUS prvPdlFanPWMGetType ( IN UINT_32 fanControllerId);
 *
 * @brief   Init fan controller
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] controllerTypePtr   - fan controller type (i.e tc654/adt7476)
 *
 * @return  
 */
PDL_STATUS prvPdlFanPWMGetType (
    IN  UINT_32                           fanControllerId,
    OUT PDL_FAN_CONTROLLER_TYPE_ENT * controllerTypePtr
);


/* @}*/
/* @}*/
/* @}*/

#endif
