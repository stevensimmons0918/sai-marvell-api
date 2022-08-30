/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlSensor.c
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
 * @brief Debug lib button module
 * 
 * @version   1 
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/sensor/pdlSensor.h>
#include <pdl/sensor/pdlSensorDebug.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <iDbgPdl/sensor/iDbgPdlSensor.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSensorGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlSensorGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      sensorId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstSensorId, lastSensorId, sensorCount;
	PDL_STATUS						pdlStatus;
	PDL_SENSOR_INFO_STC             sensorInfo;
    char                          * sensorTypeString = NULL;
    char                          * interfaceTypeString = NULL;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (sensorId == IDBG_PDL_ID_ALL) {
        firstSensorId = 0;
        pdlStatus = pdlSensorDbNumberOfSensorsGet(&sensorCount);
        PDL_CHECK_STATUS(pdlStatus);
        lastSensorId = sensorCount;
    }
    else {
        firstSensorId = sensorId;
        lastSensorId = sensorId + 1;
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-9s | %-8s | %-20s | %-15s | %-15s |\n", "Sensor ID", "  Type",  "    Description"     ,"Interface ID", "Interface Type");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-9s | %-8s | %-20s | %-15s | %-15s |\n", "---------", "--------","--------------------","------------", "--------------");

    for (i = firstSensorId; i <lastSensorId; i++) {
        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSensorDebugInfoGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &sensorInfo));
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_SENSOR_HW_TYPE_E, sensorInfo.sensorType, &sensorTypeString);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = iDbgPdlConvertEnumToString(IDBG_PDL_UTILS_INTERFACE_TYPE, sensorInfo.sensorTempratureInterface.interfaceType, &interfaceTypeString);
        PDL_CHECK_STATUS(pdlStatus);

        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("%-9d | %-8s | %-20s | %-15d | %-15s |\n",
                                        i,
                                        (sensorTypeString == NULL) ? "" : sensorTypeString,
                                        sensorInfo.sensorName,
                                        (sensorInfo.sensorType == PDL_SENSOR_TYPE_BOARD_E) ? sensorInfo.sensorTempratureInterface.sensorInterfaceId : 0,
                                        (sensorInfo.sensorType == PDL_SENSOR_TYPE_BOARD_E) ? (interfaceTypeString == NULL) ? "" : interfaceTypeString : "Not relevant");
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlSensorGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSensorGetTemperature
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlSensorGetTemperature (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      sensorId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                            sensorTemperatureString[70];
    UINT_32                         i, firstSensorId, lastSensorId, sensorCount, sensorTemprature;
	PDL_STATUS						pdlStatus;
    PDL_SENSOR_INFO_STC             sensorInfo;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (sensorId == IDBG_PDL_ID_ALL) {
        firstSensorId = 0;
        pdlStatus = pdlSensorDbNumberOfSensorsGet(&sensorCount);
        PDL_CHECK_STATUS(pdlStatus);
        lastSensorId = sensorCount;
    }
    else {
        firstSensorId = sensorId;
        lastSensorId = sensorId + 1;
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-9s | %-60s |\n", "Sensor ID", "                       Temperature");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-9s | %-60s |\n", "---------", "------------------------------------------------------------");

    for (i = firstSensorId; i <lastSensorId; i++) {
        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSensorDebugInfoGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &sensorInfo));
        PDL_CHECK_STATUS(pdlStatus);
        if (sensorInfo.sensorType == PDL_SENSOR_TYPE_BOARD_E) {
            IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSensorHwTemperatureGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, &sensorTemprature));
            PDL_CHECK_STATUS(pdlStatus);
            prvPdlOsSnprintf (sensorTemperatureString, 70, "%d", sensorTemprature);
        }
        else {
            prvPdlOsSnprintf (sensorTemperatureString, 70, "Sensor Temperature is unreadable (not a BOARD type sensor)");
        }

        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-9d | %-60s |\n", 
                                       i, 
                                       sensorTemperatureString);
    }
    return PDL_OK;
}

/*$ END OF iDbgPdlSensorGetTemperature */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSensorGetCount
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlSensorGetCount (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         sensorCount;
	PDL_STATUS						pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSensorDbNumberOfSensorsGet, IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &sensorCount));
    PDL_CHECK_STATUS(pdlStatus);
  
    IDBG_PDL_LOGGER_API_OUTPUT_LOG ("Number of sensors: %d\n", sensorCount);

    return PDL_OK;
}
/*$ END OF iDbgPdlSensorGetInfo */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlSensorRunValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlSensorRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      sensorNumber
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstSensorId, lastSensorId, sensorCount, sensorTemprature;
    PDL_STATUS						pdlStatus;
    PDL_SENSOR_INFO_STC             sensorInfo;
    char                          * sensorTypeSt = NULL, *sensorInfoSt = NULL,  *sensorTemperatureSt = NULL, *sensorCountSt = NULL;

/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-13s | %-16s | %-16s | %-16s | %-16s |\n", "Sensor Number", "   Sensor Type  ", "  Sensor Count  ", "   Sensor Info  ", "  Temperature   ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-13s | %-16s | %-16s | %-16s | %-16s |\n", "-------------", "----------------", "----------------", "----------------", "----------------");
    
    pdlStatus = pdlSensorDbNumberOfSensorsGet(&sensorCount);
    if (pdlStatus == PDL_OK){
        sensorCountSt = "Pass";
    }
    else pdlStatusToString (pdlStatus, &sensorCountSt); 

    if (sensorNumber == IDBG_PDL_ID_ALL){
        if (pdlStatus == PDL_OK){
            firstSensorId = 0;
            lastSensorId = sensorCount;
        }
        else {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-13s | %-16s | %-16s | %-16s | %-16s |\n", sensorCountSt, sensorCountSt, sensorCountSt, sensorCountSt, sensorCountSt);
            return PDL_OK;                  
        }
    }
    else {
        firstSensorId = sensorNumber;
        lastSensorId = firstSensorId + 1;
    }
   
    for (i = firstSensorId; i <lastSensorId; i++) {
        pdlStatus = pdlSensorDebugInfoGet(i, &sensorInfo);
        if (pdlStatus == PDL_OK){
            pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_SENSOR_HW_TYPE_E, sensorInfo.sensorType, &sensorTypeSt);
            PDL_CHECK_STATUS(pdlStatus);
            sensorInfoSt = "Pass";
        }
        else {
            pdlStatusToString (pdlStatus, &sensorInfoSt);
            sensorTypeSt = sensorInfoSt;
        }
        pdlStatus = pdlSensorHwTemperatureGet(i, &sensorTemprature);
        if (pdlStatus == PDL_OK){
            sensorTemperatureSt = "Pass";
        }
        else pdlStatusToString (pdlStatus, &sensorTemperatureSt);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-13d | %-16s | %-16s | %-16s | %-16s |\n", i, sensorTypeSt, sensorCountSt, sensorInfoSt, sensorTemperatureSt);
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}
/*$ END OF iDbgPdlSensorRunValidation */