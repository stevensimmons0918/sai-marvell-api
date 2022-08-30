/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\sensor\pdlsensor.h.
 *
 * @brief   Declares the pdlsensor class
 */

#ifndef __pdlSensorh

#define __pdlSensorh
/**
********************************************************************************
 * @file pdlSensor.h   
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
 * @brief Platform driver layer - sensor related API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/init/pdlInit.h>

 /** 
 * @defgroup Sensor Sensor
 * @{Sensor driver definitions and declarations including:
 * - Temprature get
*/

typedef enum {
    PDL_SENSOR_TYPE_BOARD_E,
    PDL_SENSOR_TYPE_CPU_E,
    PDL_SENSOR_TYPE_PHY_E,
    PDL_SENSOR_TYPE_MAC_E,
    PDL_SENSOR_TYPE_LAST_E
}PDL_SENSOR_TYPE_ENT;

/**
 * @struct  PDL_SENSOR_INTERFACE_STC
 *
 * @brief   defines interface for working with a sensor
 */

typedef struct {
    /** @brief   Type of the interface */
    PDL_INTERFACE_TYPE_ENT                interfaceType;    
    /** @brief   Identifier for the sensor interface */
    PDL_INTERFACE_TYP                     sensorInterfaceId;
    PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP  externalDriverId;
} PDL_SENSOR_INTERFACE_STC;

/**
 * @struct  PDL_SENSOR_PRV_DB_STC
 *
 * @brief   defines structure stored for sensor in hash
 */

typedef struct {
    /** @brief   The sensor temprature interface */
    PDL_SENSOR_TYPE_ENT                     sensorType;
    PDL_SENSOR_INTERFACE_STC                sensorTempratureInterface;
    char                                    sensorName[PDL_XML_MAX_TAG_LEN];
} PDL_SENSOR_INFO_STC;


/**
 * @fn  PDL_STATUS pdlSensorDbNumberOfSensorsGet ( OUT UINT_32 * numOfSensorsPtr )
 *
 * @brief   Pdl sensor database number of sensors get
 *
 * @param [in,out]  numOfSensorsPtr If non-null, number of sensors pointers.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSensorDbNumberOfSensorsGet (
    OUT  UINT_32                 *  numOfSensorsPtr
);

/**
 * @fn  PDL_STATUS pdlSensorHwTemperatureGet( IN UINT_32 sensorId, OUT UINT_32 * tempPtr );
 *
 * @brief   Get sensor temperature
 *
 * @param [in]  sensorId    - sensor identifier.
 * @param [out] tempPtr     - sensor temperature.
 *
 * @return  PDL_STATUS  .
 */

PDL_STATUS pdlSensorHwTemperatureGet(
    IN   UINT_32                        sensorId,
    OUT  UINT_32                      * tempPtr
);

/**
 * @fn  PDL_STATUS pdlSensorDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Sensor debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSensorDebugSet (
    IN  BOOLEAN             state
);

/* external driver function decelerations */

typedef PDL_STATUS pdlSensorHwTemperatureGet_FUN (
    IN   UINT_32                        sensorId,
    OUT  UINT_32                      * tempPtr
);


typedef struct {
    pdlSensorHwTemperatureGet_FUN         * sensorHwTemperatureGetFun;
    PDL_OS_LOCK_TYPE_ENT                    lockType;
} PDL_SENSOR_CALLBACK_HW_TEMP_GET_INFO_STC;

typedef struct {
    PDL_CALLBACK_INIT_INFO_STC                      sensorInitDbCallbackInfo;
    PDL_CALLBACK_INIT_INFO_STC                      sensorInitHwCallbackInfo;
    PDL_SENSOR_CALLBACK_HW_TEMP_GET_INFO_STC        sensorHwTemperatureGetInfo;
} PDL_SENSOR_CALLBACK_INFO_STC;


/* ***************************************************************************
* FUNCTION NAME: pdlSensorBindExternalDriver
*
* DESCRIPTION:   bind external driver with callback set & id
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS pdlSensorBindExternalDriver(
	IN PDL_SENSOR_CALLBACK_INFO_STC       * callbacksInfoPtr,
	IN PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP externalDriverId
);


/* @}*/

#endif
