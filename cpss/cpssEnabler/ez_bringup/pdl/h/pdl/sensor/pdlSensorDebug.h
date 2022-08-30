/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\sensor\pdlsensordebug.h.
 *
 * @brief   Declares the pdlsensordebug class
 */

#ifndef __pdlSensorDebugh

#define __pdlSensorDebugh
/**
********************************************************************************
 * @file pdlSensorDebug.h   
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
 * @brief Platform driver layer - LED related debug API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/sensor/pdlSensor.h>

/**
* @addtogroup Sensor
* @{
*/
 /** 
 * @defgroup Sensor_Debug Sensor Debug
 * @{
 */

/**
 * @fn  PDL_STATUS pdlSensorDebugInfoGet ( IN UINT_32 sensorId, OUT PDL_SENSOR_INFO_STC * infoPtr )
 *
 * @brief   Pdl sensor debug temperature interface get
 *
 * @param           sensorId        Identifier for the sensor.
 * @param [in,out]  infoPtr         If non-null, the sensor information pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSensorDebugInfoGet (
    IN  UINT_32                    sensorId,
    OUT PDL_SENSOR_INFO_STC      * infoPtr
);

/* @}*/
/* @}*/
#endif