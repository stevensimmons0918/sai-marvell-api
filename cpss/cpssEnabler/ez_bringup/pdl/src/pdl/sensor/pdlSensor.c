/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlSensor.c   
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
 * @brief Platform driver layer - Sensor related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/sensor/pdlSensor.h>
#include <pdl/sensor/private/prvPdlSensor.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The pdl sensor database */
static PDL_DB_PRV_STC   pdlSensorDb = {0,NULL};
static PDL_DB_PRV_STC   pdlSensorCallbacksDb =  {0,NULL};
static BOOLEAN          pdlSensorInternalDriverBind = FALSE;

static PDL_SENSOR_PRV_CALLBACK_INFO_STC pdlSensorCallbacksInternal = {
    NULL,
    NULL,
    &prvPdlSensorInternalDriverHwTemperatureGet
};

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                  \
    if (__pdlStatus != PDL_OK) {                                                                       \
    if (prvPdlSensorDebugFlag) {                                                                       \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);      \
    }                                                                                                  \
    return __pdlStatus;                                                                                \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                   \
    if (__xmlStatus != PDL_OK) {                                                                        \
    if (prvPdlSensorDebugFlag) {                                                                        \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __xmlStatus;                                                                                 \
    }  



/* ***************************************************************************
* FUNCTION NAME: pdlSensorDebugTemperatureInterfaceGet
*
* DESCRIPTION:   Get sensorId access information
*
* PARAMETERS:   
*                
*
*****************************************************************************/

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SENSOR_INFO_STC 	          * sensorPtr;
    PDL_SENSOR_PRV_KEY_STC              sensorKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (infoPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__,"infoPtr NULL, SENSOR ID: %d",sensorId);
        return PDL_BAD_PTR;
    }
    sensorKey.sensorId = sensorId;
    pdlStatus = prvPdlDbFind (&pdlSensorDb, (void*) &sensorKey, (void*) &sensorPtr);
    PDL_CHECK_STATUS(pdlStatus);
    infoPtr->sensorType = sensorPtr->sensorType;
    strcpy (infoPtr->sensorName, sensorPtr->sensorName);    
    memcpy (&infoPtr->sensorTempratureInterface, &sensorPtr->sensorTempratureInterface, sizeof(sensorPtr->sensorTempratureInterface));
    
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, sensorPtr->sensorType);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(CHAR_PTR, sensorPtr->sensorName);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, sensorPtr->sensorTempratureInterface.sensorInterfaceId);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, sensorPtr->sensorTempratureInterface.interfaceType);
    return PDL_OK;

}

/*$ END OF pdlSensorDebugTemperatureInterfaceGet */


/*****************************************************************************
* FUNCTION NAME: pdlSensorHwTemperatureGet
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlSensorHwTemperatureGet ( IN UINT_32 sensorId, OUT UINT_32 * tempPtr )
 *
 * @brief   Pdl sensor hardware temperature get
 *
 * @param           sensorId    Identifier for the sensor.
 * @param [in,out]  tempPtr     If non-null, the temporary pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSensorHwTemperatureGet (
    IN   UINT_32                        sensorId,
    OUT  UINT_32                      * tempPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_STATUS                            pdlStatus;
    PDL_SENSOR_PRV_CALLBACK_STC         * callbacksPtr;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    pdlStatus = prvPdlSensorDbGetCallbacks (sensorId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->callbackInfo.sensorHwTemperatureGetInfo.sensorHwTemperatureGetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->callbackInfo.sensorHwTemperatureGetInfo.lockType);
    pdlStatus = callbacksPtr->callbackInfo.sensorHwTemperatureGetInfo.sensorHwTemperatureGetFun(sensorId, tempPtr);
    prvPdlUnlock(callbacksPtr->callbackInfo.sensorHwTemperatureGetInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}
/*$ END OF pdlSensorHwTemperatureGet */

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
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_SENSOR_PRV_KEY_STC                  sensorKey;
    PDL_SENSOR_INFO_STC                   * sensorPtr;
    UINT_8                                  data[4];
    UINT_32                                 temp;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    memset (data, 0, sizeof(data));
    sensorKey.sensorId = sensorId;
    pdlStatus = prvPdlDbFind (&pdlSensorDb, (void*) &sensorKey, (void*) &sensorPtr);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlInterfaceHwGetValue (sensorPtr->sensorTempratureInterface.interfaceType, sensorPtr->sensorTempratureInterface.sensorInterfaceId, (UINT_32*) &data);
    PDL_CHECK_STATUS(pdlStatus);
    /* temperature is swapped */
    temp = data[0] * 256 + data[1];
    /* convert read data to temperature Celsius (lm75) */
    PDL_SENSOR_LM75_TEMP_CALC_MAC (temp);
    *tempPtr = temp;

    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, *tempPtr);
    return PDL_OK;
}
/*$ END OF prvPdlSensorInternalDriverHwTemperatureGet */

/*****************************************************************************
* FUNCTION NAME: pdlSensorDbNumberOfSensorsGet
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

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
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, *numOfSensorsPtr);
    return prvPdlDbGetNumOfEntries(&pdlSensorDb, numOfSensorsPtr);
}
/*$ END OF pdlSensorDbNumberOfSensorsGet */

/* ***************************************************************************
* FUNCTION NAME: prvPdlSensorXMLParser
*
* DESCRIPTION:   Fill all sensor related information from XML to DB
*
* PARAMETERS:   xmlId
*
* XML structure:
*<sensors>
*   <sensor>
*       <sensor-number></sensor-number>
*       <temperature>
*           <i2c-interface>
*           <bus-id></bus-id>
*           <address></address>				
*           <offset></offset>
*           <access></access>
*           <mask></mask>
*           </i2c-interface>
*       </temperature>
*   </sensor>
*   
*   <sensor>
*       ...
*   </sensor>
*   
*   ...
*   ...
*</sensors>
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlSensorXMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Prv pdl sensor XML parser
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSensorXMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                            pdlValueStr[PDL_XML_MAX_TAG_LEN] = {0};
    PDL_INTERFACE_TYP               interfaceId;
    PDL_INTERFACE_TYPE_ENT          interfaceType;
    XML_PARSER_NODE_DESCRIPTOR_TYP  xmlSensorId, xmlSensorInstanceId, xmlSensorChildId, xmlTempChildId, xmlSensorBoardId;
    PDL_STATUS                      pdlStatus;
    XML_PARSER_RET_CODE_TYP         xmlStatus, xmlStatus2, xmlStatusChild;
    PDL_SENSOR_PRV_KEY_STC          sensorKey;
    PDL_SENSOR_INFO_STC             sensorInfo;
    PDL_SENSOR_INFO_STC           * sensorPtr = NULL;
    UINT_32                         sensorId, maxSize, phyId, value, extDrvId;
    BOOLEAN                         isI2c = FALSE;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    xmlStatus = xmlParserFindByName (xmlId, "Sensors", &xmlSensorId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        return PDL_OK;
    }
    XML_CHECK_STATUS(xmlStatus);
    xmlStatus = xmlParserGetFirstChild (xmlSensorId, &xmlSensorInstanceId);
    /* go over all sensors */
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        if (xmlParserIsEqualName (xmlSensorInstanceId, "sensor-list") != XML_PARSER_RET_CODE_OK) {
            PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlSerdesChildId POINTER OR NAME DON'T MATCH TO: sensor");
            return PDL_FAIL;
        }
        isI2c = FALSE;
        /* go over all sensor child tags */
        xmlStatusChild = xmlParserGetFirstChild (xmlSensorInstanceId, &xmlSensorChildId);
        memset(&sensorInfo,0,sizeof(sensorInfo));
        while (xmlStatusChild != XML_PARSER_RET_CODE_NOT_FOUND) {
            XML_CHECK_STATUS(xmlStatusChild);
            if (xmlParserIsEqualName (xmlSensorChildId, "board-sensor") == XML_PARSER_RET_CODE_OK) {
                xmlStatus2 = xmlParserGetFirstChild (xmlSensorChildId, &xmlSensorBoardId);
                while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                    if (xmlParserIsEqualName (xmlSensorBoardId, "temperature") == XML_PARSER_RET_CODE_OK) {
                        xmlStatus = xmlParserGetFirstChild (xmlSensorBoardId, &xmlTempChildId);
                        XML_CHECK_STATUS(xmlStatus);
                        xmlStatus = prvPdlInterfaceXmlParser (xmlTempChildId, &interfaceType, &interfaceId);
                        XML_CHECK_STATUS(xmlStatus);
                        sensorInfo.sensorTempratureInterface.interfaceType = interfaceType;
                        sensorInfo.sensorTempratureInterface.sensorInterfaceId = interfaceId;
                    }
                    else if (xmlParserIsEqualName (xmlSensorBoardId, "is-stateInfoControl-supported") == XML_PARSER_RET_CODE_OK) {
                        maxSize = PDL_XML_MAX_TAG_LEN;
                        xmlStatus = xmlParserGetValue (xmlSensorBoardId, &maxSize, &pdlValueStr[0]);
                        XML_CHECK_STATUS(xmlStatus);
                        if (strcmp (pdlValueStr, "true") == 0) {
                            /* stateInfoControl currently not supported for sensor */
                            return PDL_XML_PARSE_ERROR;
                        }
                    }
                    else {
                        PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlSensorBoardId tag");
                        return PDL_XML_PARSE_ERROR;
                    }
                    xmlStatus2 = xmlParserGetNextSibling(xmlSensorBoardId, &xmlSensorBoardId);
                }
            }
            else if (xmlParserIsEqualName (xmlSensorChildId, "sensor-number") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(sensorId);
                xmlStatus = xmlParserGetValue (xmlSensorChildId, &maxSize, &sensorId);
                XML_CHECK_STATUS(xmlStatus);
                sensorKey.sensorId = sensorId;
            }
            else if (xmlParserIsEqualName (xmlSensorChildId, "sensor-type") == XML_PARSER_RET_CODE_OK) {
                maxSize = PDL_XML_MAX_TAG_LEN;
                xmlStatus = xmlParserGetValue(xmlSensorChildId, &maxSize, &pdlValueStr[0]);
                XML_CHECK_STATUS(xmlStatus);
                pdlStatus =  pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_SENSOR_HW_TYPE_E, pdlValueStr, &value);
                PDL_CHECK_STATUS(pdlStatus);
                sensorInfo.sensorType = (PDL_SENSOR_TYPE_ENT)value;
            }
            else if (xmlParserIsEqualName (xmlSensorChildId, "sensor-description") == XML_PARSER_RET_CODE_OK) {
                maxSize = PDL_XML_MAX_TAG_LEN;
                xmlStatus = xmlParserGetValue (xmlSensorChildId, &maxSize, &sensorInfo.sensorName[0]);
                XML_CHECK_STATUS(xmlStatus);
            }
            else if (xmlParserIsEqualName (xmlSensorChildId, "phy-number") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(phyId);
                xmlStatus = xmlParserGetValue (xmlSensorChildId, &maxSize, &phyId);
                XML_CHECK_STATUS(xmlStatus);
            }
            else if (xmlParserIsEqualName (xmlSensorChildId, "board-interface-is-i2c") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(isI2c);
                xmlStatus = xmlParserGetValue (xmlSensorChildId, &maxSize, &isI2c);
                XML_CHECK_STATUS(xmlStatus);
                /* TBD - what to do with this information ?*/
            }
            else if (xmlParserIsEqualName (xmlSensorChildId, "external-driver-id") == XML_PARSER_RET_CODE_OK) {
                maxSize = sizeof(extDrvId);
                xmlStatus = xmlParserGetValue (xmlSensorChildId, &maxSize, &extDrvId);
                XML_CHECK_STATUS(xmlStatus);
                sensorInfo.sensorTempratureInterface.externalDriverId = extDrvId;
                sensorInfo.sensorTempratureInterface.interfaceType = PDL_INTERFACE_TYPE_EXTERNAL_E;
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlSensorChildId POINTER OR NAME DON'T MATCH TO: sensor-number");
                return PDL_XML_PARSE_ERROR;
            }
            xmlStatusChild = xmlParserGetNextSibling(xmlSensorChildId, &xmlSensorChildId);
        }
        pdlStatus = prvPdlDbAdd (&pdlSensorDb, (void*) &sensorKey, (void*) &sensorInfo, (void**) &sensorPtr);   
        PDL_CHECK_STATUS(pdlStatus);
        xmlStatus =xmlParserGetNextSibling (xmlSensorInstanceId, &xmlSensorInstanceId);
    }
   
    return PDL_OK;
}

/*$ END OF prvPdlSensorXMLParser */


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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlSensorDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlSensorDebugSet */

/* ***************************************************************************
* FUNCTION NAME: prvPdlSensorBindDriver
*
* DESCRIPTION:   bind driver with callback set & id
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS prvPdlSensorBindDriver(
	IN PDL_SENSOR_CALLBACK_INFO_STC         * callbacksInfoPtr,
	IN PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP   externalDriverId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                                    pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC                     dbAttributes;
    PDL_SENSOR_PRV_CALLBACK_STC                   callbacksInfo, * callbackInfoPtr = NULL;                 
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /*create callbacks db if needed*/
    if(pdlSensorCallbacksDb.dbPtr == NULL){
        dbAttributes.listAttributes.entrySize = sizeof(PDL_SENSOR_PRV_CALLBACK_STC);
        dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP);
        pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,  &dbAttributes,  &pdlSensorCallbacksDb);
        PDL_CHECK_STATUS(pdlStatus);
    }

    /* verify entry with this externalDriverId doesn't exist already */
    pdlStatus = prvPdlDbFind (&pdlSensorCallbacksDb, (void*) &(externalDriverId), (void**) &callbackInfoPtr);
    if (pdlStatus == PDL_OK) {
        return PDL_ALREADY_EXIST;
    }

    callbacksInfo.externalDriverId = externalDriverId;
    memcpy (&callbacksInfo.callbackInfo, callbacksInfoPtr, sizeof (PDL_SENSOR_CALLBACK_INFO_STC));
    /*add to db*/ 
    pdlStatus = prvPdlDbAdd (&pdlSensorCallbacksDb, (void*) &externalDriverId, (void*) &callbacksInfo, (void**) &callbackInfoPtr); 
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF prvPdlSensorBindDriver */

/* ***************************************************************************
* FUNCTION NAME: prvPdlSensorBindInternalDriver
*
* DESCRIPTION:   bind internal driver with callback set & id
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS prvPdlSensorBindInternalDriver(
	IN PDL_SENSOR_PRV_CALLBACK_INFO_STC         * callbacksInfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_SENSOR_CALLBACK_INFO_STC                  callbacksInfo;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    memset (&callbacksInfo, 0, sizeof(callbacksInfo));
    callbacksInfo.sensorHwTemperatureGetInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.sensorHwTemperatureGetInfo.sensorHwTemperatureGetFun = callbacksInfoPtr->sensorHwTemperatureGetFun;
    callbacksInfo.sensorInitDbCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.sensorInitDbCallbackInfo.initFun = callbacksInfoPtr->sensorInitDbFun;
    callbacksInfo.sensorInitHwCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.sensorInitHwCallbackInfo.initFun = callbacksInfoPtr->sensorInitHwFun;
    return prvPdlSensorBindDriver (&callbacksInfo, PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD);
}

/*$ END OF prvPdlSensorBindInternalDriver */

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
	IN PDL_SENSOR_CALLBACK_INFO_STC         * callbacksInfoPtr,
	IN PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP   externalDriverId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
            
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (externalDriverId >= PDL_INTERFACE_INTERNAL_DRIVER_ID_MIN) {
        PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__,"ids from [%d] are reserved for internal usage",PDL_INTERFACE_INTERNAL_DRIVER_ID_MIN);
        return PDL_OUT_OF_RANGE;
    }
    return prvPdlSensorBindDriver (callbacksInfoPtr, externalDriverId);
}

/*$ END OF pdlSensorBindExternalDriver */


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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                                    pdlStatus;
    PDL_SENSOR_PRV_KEY_STC                        sensorKey;
    PDL_SENSOR_PRV_CALLBACK_STC                 * callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlDbGetNextKey (&pdlSensorDb, NULL, (void*) &sensorKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlSensorDbGetCallbacks (sensorKey.sensorId, &callbacksPtr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlDbGetNextKey (&pdlSensorDb, (void*) &sensorKey, (void*) &sensorKey);
    }
    return PDL_OK;
}

/*$ END OF prvPdlSensorVerifyExternalDrivers */

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_SENSOR_PRV_KEY_STC                  sensorKey;
    PDL_SENSOR_INFO_STC                   * sensorPtr;
    PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP    sensorDriverId = PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    if (callbacksPtr == NULL) {
        return PDL_BAD_PTR;
    }
    sensorKey.sensorId = sensorId;
    pdlStatus = prvPdlDbFind (&pdlSensorDb, (void*) &sensorKey, (void*) &sensorPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (sensorPtr->sensorTempratureInterface.interfaceType == PDL_INTERFACE_TYPE_EXTERNAL_E) {
        sensorDriverId = sensorPtr->sensorTempratureInterface.externalDriverId;
    }
    pdlStatus = prvPdlDbFind (&pdlSensorCallbacksDb, (void*) &(sensorDriverId), (void**) callbacksPtr);
    if (pdlStatus != PDL_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__,"can't find callback for sensorId=[%d], pdlStatus=[%d]",sensorId, pdlStatus);
    }
    PDL_CHECK_STATUS(pdlStatus);
  
    return PDL_OK;
}

/*$ END OF prvPdlSensorDbGetCallbacks */

/* ***************************************************************************
* FUNCTION NAME: pdlSensorInit
*
* DESCRIPTION:   Init sensor module
*                Create sensor DB and initialize
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS pdlSensorInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC               dbAttributes;
    PDL_SENSOR_PRV_CALLBACK_STC           * callbackInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    dbAttributes.listAttributes.entrySize = sizeof(PDL_SENSOR_INFO_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_SENSOR_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlSensorDb);
    PDL_CHECK_STATUS(pdlStatus);

    /* pdlSensorInit can be called several times, bind internal driver only once otherwise we'll return PDL_ALREADY_EXIST */
    if (pdlSensorInternalDriverBind == FALSE) {
        pdlStatus = prvPdlSensorBindInternalDriver (&pdlSensorCallbacksInternal);
        PDL_CHECK_STATUS(pdlStatus);
        pdlSensorInternalDriverBind = TRUE;
    }

    pdlStatus = prvPdlSensorXMLParser (xmlId);
    PDL_CHECK_STATUS(pdlStatus);

    /* init all external Sensor drivers */
    pdlStatus = prvPdlDbGetFirst(&pdlSensorCallbacksDb, (void **)&callbackInfoPtr);
    while (pdlStatus == PDL_OK) {
        if(callbackInfoPtr->callbackInfo.sensorInitDbCallbackInfo.initFun != NULL) {
            prvPdlLock(callbackInfoPtr->callbackInfo.sensorInitDbCallbackInfo.lockType);
            callbackInfoPtr->callbackInfo.sensorInitDbCallbackInfo.initFun();
            prvPdlUnlock(callbackInfoPtr->callbackInfo.sensorInitDbCallbackInfo.lockType);
        }
        pdlStatus = prvPdlDbGetNext(&pdlSensorCallbacksDb, (void*)&callbackInfoPtr->externalDriverId, (void **)&callbackInfoPtr);
    }

    return PDL_OK;
}

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (pdlSensorDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlSensorDb);
        PDL_CHECK_STATUS(pdlStatus);
    }
    pdlSensorInternalDriverBind = FALSE;
    if (pdlSensorCallbacksDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlSensorCallbacksDb);
        PDL_CHECK_STATUS(pdlStatus);    
    }
    return PDL_OK;
}

/*$ END OF prvPdlSensorDestroy */


