/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\sensor\private\prvpdlsensor.h.
 *
 * @brief   Declares the prvpdlsensor class
 */

#ifndef __prvPdlSensorh

#define __prvPdlSensorh
/**
********************************************************************************
 * @file prvPdlSensor.h   
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
 * @brief Platform driver layer - Sensor private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/sensor/pdlSensor.h>
#include <pdl/xml/private/prvXmlParser.h>

/**
* @addtogroup Sensor
* @{
*/

 /** 
 * @defgroup Sensor_private Sensor Private
 * @{Sensor private definitions and declarations including:
*/

/*! Sensor main tag */
#define PDL_SENSOR_TAG_NAME                 "sensors"
/*! Tag for single sensor */
#define PDL_SENSOR_UNIT_TAG_NAME            "sensor"
/*! Tag for sensor number */
#define PDL_SENSOR_NUMBER_TAG_NAME          "sensor-number"
/*! Tag for temperature sensor \todo what is this? */
#define PDL_SENSOR_TEMPERATURE_TAG_NAME     "temperature"

/*! Illegal sensor ID */
#define PDL_SENSOR_ID_LAST                  0xFFFFFFFF

/* Macros to convert temperture values: */
#define  PDL_SENSOR_LM77_TEMP_CALC_MAC(value)   if (value & 0x800) \
                                                       value = ((0x7F8 & value) >> 4) - 128;\
                                                   else  \
                                                       value =  (0x7F8 & value) >> 4;

#define  PDL_SENSOR_LM75_TEMP_CALC_MAC(value)  if (value & 0x8000) \
                                                       value = ((0x7F00 & value) >> 8) - 128;\
                                                   else  \
                                                       value =  (0x7F00 & value) >> 8;

/**
 * @struct  PDL_SENSOR_PRV_KEY_STC
 *
 * @brief   defines structure for sensor key in hash
 */

typedef struct {
    /** @brief   Identifier for the sensor */
    UINT_32                        sensorId;
} PDL_SENSOR_PRV_KEY_STC;

typedef struct {
    PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP    externalDriverId;
    PDL_SENSOR_CALLBACK_INFO_STC            callbackInfo;
} PDL_SENSOR_PRV_CALLBACK_STC;

typedef struct {
    pdlExtDrvInit_FUN                     * sensorInitDbFun;
    pdlExtDrvInit_FUN                     * sensorInitHwFun;
    pdlSensorHwTemperatureGet_FUN         * sensorHwTemperatureGetFun;
} PDL_SENSOR_PRV_CALLBACK_INFO_STC;

/*****************************************************************************
* FUNCTION NAME: pdlSensorInternalDriverHwTemperatureGet
*
* DESCRIPTION:   get sensor temperature - internal implementation for all supported interface types
*      
*
*****************************************************************************/

PDL_STATUS prvPdlSensorInternalDriverHwTemperatureGet (
    IN   UINT_32                        sensorId,
    OUT  UINT_32                      * tempPtr
);

/*$ END OF prvPdlSensorInternalDriverHwTemperatureGet */

/* ***************************************************************************
* FUNCTION NAME: pdlSensorDbGetCallbacks
*
* DESCRIPTION:   bind external driver with callback set & id
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS prvPdlSensorDbGetCallbacks (
    IN  UINT_32                              sensorId,
    OUT PDL_SENSOR_PRV_CALLBACK_STC      **  callbacksPtr
);

/*$ END OF prvPdlSensorDbGetCallbacks */

/* ***************************************************************************
* FUNCTION NAME: prvPdlSensorVerifyExternalDrivers
*
* DESCRIPTION:   verify all external drivers have been binded
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS prvPdlSensorVerifyExternalDrivers(
	IN void
);

/*$ END OF prvPdlSensorVerifyExternalDrivers */

/**
 * @fn  PDL_STATUS pdlSensorInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId );
 *
 * @brief   Init sensor module Create button DB and initialize
 *
 * @param [in]  xmlId   - Xml id.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlSensorInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
);

/*$ END OF pdlSensorInit */

/**
 * @fn  PDL_STATUS prvPdlSensorDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSensorDestroy (
    void
);

/* @}*/
/* @}*/

#endif