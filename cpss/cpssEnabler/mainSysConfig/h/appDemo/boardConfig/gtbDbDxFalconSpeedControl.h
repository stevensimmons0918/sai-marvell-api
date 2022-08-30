
/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtbDbDxFalconSpeedControl.h
*
* @brief this file includes logic of Falcon 12.8 DB fan managment
* task which runs in the background- configurate the board fans
* speed according to the device temperature using PID controller for
* each of the bord's sensors.
*
* @version   1
********************************************************************************
*/
#ifndef __gtbDbDxFalconSpeedControl_H
#define __gtbDbDxFalconSpeedControl_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <appDemo/phy/gtAppDemoPhyConfig.h>

/**
* @enum APP_DEMO_PID_SENSORS_ENT
 *
 * @brief This enum used for the PID configurations
*/
typedef enum
{
    /* PID controlled by eagle and ravens max temperature */
    APP_DEMO_EAGLES_RAVENS_THERMAL_SENSOR_E,
    /* PID controlled by CPU max temperature */
    APP_DEMO_CPU_THERMAL_SENSOR_E,
    /* PID controlled by board air outlet temperature */
    APP_DEMO_BOARD_THERMAL_SENSOR_E,
    /* PID controlled by transceivers max temperature */
    APP_DEMO_TRANSIVER_THERMAL_SENSOR_E
}APP_DEMO_PID_SENSORS_ENT;

/******************************************************/
/* DB_falcon thermal task main functions declerations */
/******************************************************/

/** @internal appDemoFalcon_DB_ThermalTaskDebugEnableSet function
 *@endinternal
 *
 * @brief enable/disable falcon DB 12.8 Fan Mangment task debug.
 *
 * @param[in] en -  true - enable thermal task debugging
 *                  false - disable thermal task debugging
 *
 * @return GT_OK
 *
 * @note for debug purposes only
 *
 */
GT_STATUS  appDemoFalcon_DB_ThermalTaskDebugEnableSet
(
    IN GT_BOOL en
);

/**
 *@internal appDemoFalcon_DB_ThermalTaskEnableSet function
 *@endinternal
 *
 * @brief enable/disable falcon DB 12.8 Fan Mangment task.
 *
 * @param[in] en -  true - enable thermal task
 *                  false - disable thermal task
 *
 * @return GT_OK
 *
 * @note for debug purposes only. The task will still run in backround but will
 *       not change fan speed.
 */
GT_STATUS appDemoFalcon_DB_ThermalTaskEnableSet
(
    IN GT_BOOL en
);

/**
* @internal appDemoFalcon_DB_FanSpeedSet function
* @endinternal
*
* @brief Set falcon DB 12.8 fan speed
*
* @param GT_U32 value - Applicable values: 0-7
*
* @note For debug purposes only. To keep fan speed as set, make sure to call
*       falcon_DB_EnableThermalTask with GT_FALSE.
*
*/
GT_STATUS appDemoFalcon_DB_FanSpeedSet
(
    IN GT_U32 value
);

/**
* @internal appDemoFalcon_DB_FanSpeedDump function
* @endinternal
*
* @brief get falcon DB 12.8 fan speed
*
* @note For debug purposes only.
*/
GT_U32 appDemoFalcon_DB_FanSpeedDump
(
    GT_VOID
);


/**
* @internal falconDB_ThermalTaskInit function
* @endinternal
*
* @brief This function initialize Falcon 12.8 DB Fan Managment task
*
*/
GT_STATUS falconDB_ThermalTaskInit
(
    IN GT_U8   devNum
);

GT_VOID PIDtest
(
    IN GT_FLOAT64  pIn,
    IN GT_FLOAT64  iIn,
    IN GT_FLOAT64  dIn
);

/**
* @internal appDemoFalcon_DB_sensorTemperaturePrint function
* @endinternal
*
* @brief Print selected or all falcon DB 12.8 temperature sensors
*
* @param[in] devNum         - device number
* @param[in] sensor         - selected sensor
*
*/
GT_STATUS appDemoFalcon_DB_sensorTemperaturePrint
(
    GT_U8           devNum,
    APP_DEMO_PID_SENSORS_ENT   sensor
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif




