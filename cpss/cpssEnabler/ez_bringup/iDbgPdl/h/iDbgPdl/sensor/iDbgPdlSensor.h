/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlSensorh
#define __iDbgPdlSensorh
/**
********************************************************************************
 * @file iDbgPdlSensor.h   
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
 * @brief Debug lib sensor module
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/sensor/pdlSensor.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSensorGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

extern PDL_STATUS iDbgPdlSensorGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      sensorId
);

/*$ END OF iDbgPdlSensorGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSensorGetCount
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

extern PDL_STATUS iDbgPdlSensorGetCount (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
);

/*$ END OF iDbgPdlSensorGetCount */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSensorGetTemperature
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

extern PDL_STATUS iDbgPdlSensorGetTemperature (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            sensorId
);

/*$ END OF iDbgPdlSensorGetTemperature */
/*****************************************************************************
* FUNCTION NAME: iDbgPdlSensorRunValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlSensorRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            sensorNumber
);

#endif
