/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlFan.c   
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

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/fan/pdlFan.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/interface/private/prvPdlI2c.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/private/prvXmlParser.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
/** @brief   The pdl fan controller database */
static PDL_DB_PRV_STC pdlFanControllersDb =  {0,NULL};
/* list of connected fan ids (used in pdlFanDbGet & pdlFanDbGetNext) */
static PDL_DB_PRV_STC pdlFanIdsDb =  {0,NULL};

/** @brief   The pdl fan controller database */
static PDL_DB_PRV_STC pdlFanCallbacksDb =  {0,NULL};

static PDL_DB_PRV_STC pdlFanThresholdDb = {0,NULL};
static BOOLEAN        pdlSwFanSupported = FALSE;

/** @brief   The pdl fan state string to enum pairs[ pdl fan state control last e] */
static PRV_PDL_LIB_STR_TO_ENUM_STC pdlFanStateStrToEnumPairs[PDL_FAN_STATE_CONTROL_LAST_E] = {
    {"normal", PDL_FAN_STATE_CONTROL_NORMAL_E},
    {"shutdown", PDL_FAN_STATE_CONTROL_SHUTDOWN_E},
};
/** @brief   The pdl fan state string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlFanStateStrToEnum = {pdlFanStateStrToEnumPairs, PDL_FAN_STATE_CONTROL_LAST_E};

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                   \
    if (__pdlStatus != PDL_OK) {                                                                        \
    if (prvPdlFanDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __pdlStatus;                                                                                 \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                   \
    if (__xmlStatus != PDL_OK) {                                                                        \
    if (prvPdlFanDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __xmlStatus;                                                                                 \
    }  

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/* ***************************************************************************
* FUNCTION NAME: prvPdlFanBindFanDriver
*
* DESCRIPTION:   bind driver with callback set & id
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS prvPdlFanBindFanDriver(
    IN PDL_FAN_CONTROLLER_TYPE_PRV_KEY_STC     * fanTypeKeyPtr,
    IN PDL_FAN_PRV_CALLBACK_INFO_STC           * callbacksInfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC            *    callbackInfoPtr;
    PDL_DB_PRV_ATTRIBUTES_STC                     dbAttributes;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    /*create callbacks db if needed*/
    if(pdlFanCallbacksDb.dbPtr == NULL){
        dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_PRV_CALLBACK_INFO_STC);
        dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_CONTROLLER_TYPE_PRV_KEY_STC);
        pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,  &dbAttributes,  &pdlFanCallbacksDb);
        PDL_CHECK_STATUS(pdlStatus);
    }

    /*add to db*/ 
    pdlStatus = prvPdlDbAdd (&pdlFanCallbacksDb, (void*) fanTypeKeyPtr, (void*) callbacksInfoPtr, (void**) &callbackInfoPtr); 
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF prvPdlFanBindInternalFanDriver */

/**
 * @fn  PDL_STATUS prvPdlFanBindInternalFanDriver
 *
 * @brief   binds new internal fan driver, function that aren't implemented by specific fan driver
 *          should be NULL pointers
 *          
 *
 * @param [in] callbacksInfoPtr        - all callbacks relevant to this specific fan
 * @param [in] fanType                 - type of the fan
 *
 * @return  
 */

PDL_STATUS prvPdlFanBindInternalFanDriver(
    IN PDL_FAN_PRV_CALLBACK_FUNC_STC           * callbacksInfoPtr,
    IN PDL_FAN_CONTROLLER_TYPE_ENT               fanType
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_FAN_PRV_CALLBACK_INFO_STC                 callbackInfo;
    PDL_FAN_CONTROLLER_TYPE_PRV_KEY_STC           fanControllerTypeKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    fanControllerTypeKey.fanType = fanType;
    fanControllerTypeKey.externalDriverId = 0;
    memset (&callbackInfo, 0, sizeof(callbackInfo));
    callbackInfo.externalCallbackInfo.fanControllerFaultSetCallbackInfo.fanControllerHwFaultSetFun = callbacksInfoPtr->fanControllerHwFaultSetFun;
    callbackInfo.externalCallbackInfo.fanControllerFaultSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanControllerHwDutyCycleGetCallbackInfo.fanHwDutyCycleGetFun = callbacksInfoPtr->fanHwDutyCycleGetFun;
    callbackInfo.externalCallbackInfo.fanControllerHwDutyCycleGetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanControllerHwDutyCycleMethodSetCallbackInfo.fanControllerHwDutyCycleMethodSetFun = callbacksInfoPtr->fanControllerHwDutyCycleMethodSetFun;
    callbackInfo.externalCallbackInfo.fanControllerHwDutyCycleMethodSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanControllerHwDutyCycleSetCallbackInfo.fanControllerHwDutyCycleSetFun = callbacksInfoPtr->fanControllerHwDutyCycleSetFun;
    callbackInfo.externalCallbackInfo.fanControllerHwDutyCycleSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanControllerHwRotationDirectionGetCallbackInfo.fanControllerHwRotationDirectionGetFun = callbacksInfoPtr->fanControllerHwRotationDirectionGetFun;
    callbackInfo.externalCallbackInfo.fanControllerHwRotationDirectionGetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanControllerHwRotationDirectionSetCallbackInfo.fanControllerHwRotationDirectionSetFun = callbacksInfoPtr->fanControllerHwRotationDirectionSetFun;
    callbackInfo.externalCallbackInfo.fanControllerHwRotationDirectionSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanDbInitCallbackInfo.fanDbInitFun = callbacksInfoPtr->fanDbInitFun;
    callbackInfo.externalCallbackInfo.fanHwControllerRpmGetInfo.fanControllerRpmGetFun = callbacksInfoPtr->fanControllerRpmGetFun;
    callbackInfo.externalCallbackInfo.fanHwControllerRpmGetInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanHwInitCallbackInfo.fanControllerHwInitFun = callbacksInfoPtr->fanControllerHwInitFun;
    callbackInfo.externalCallbackInfo.fanHwInitCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanHwPulsesPerRotationSetCallbackInfo.fanHwPulsesPerRorationSetFun = callbacksInfoPtr->fanHwPulsesPerRorationSetFun;
    callbackInfo.externalCallbackInfo.fanHwPulsesPerRotationSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanHwStateSetCallbackInfo.fanHwStateSetFun = callbacksInfoPtr->fanHwStateSetFun;
    callbackInfo.externalCallbackInfo.fanHwStateSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanHwStatusGetCallbackInfo.fanHwStatusGetFun = callbacksInfoPtr->fanHwStatusGetFun;
    callbackInfo.externalCallbackInfo.fanHwStatusGetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.externalCallbackInfo.fanHwThresholdSetCallbackInfo.fanHwThresholdSetFun = callbacksInfoPtr->fanHwThresholdSetFun;
    callbackInfo.externalCallbackInfo.fanHwThresholdSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbackInfo.fanXMLParserFun = callbacksInfoPtr->fanXMLParserFun;
    return prvPdlFanBindFanDriver(&fanControllerTypeKey, &callbackInfo);
}

/*$ END OF prvPdlFanBindInternalFanDriver */

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_FAN_PRV_CALLBACK_INFO_STC                 callbackInfo;
    PDL_FAN_CONTROLLER_TYPE_PRV_KEY_STC           fanControllerTypeKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    if (externalDriverId > 255) {
        return PDL_OUT_OF_RANGE;
    }
    fanControllerTypeKey.fanType = PDL_FAN_CONTROLLER_TYPE_USER_DEFINED_E;
    fanControllerTypeKey.externalDriverId = externalDriverId;
    memset (&callbackInfo, 0, sizeof(callbackInfo));
    memcpy (&callbackInfo.externalCallbackInfo, callbacksInfoPtr, sizeof(PDL_FAN_CALLBACK_INFO_STC));
    return prvPdlFanBindFanDriver(&fanControllerTypeKey, &callbackInfo);
}

/*$ END OF pdlFanBindExternalFanDriver */


/**
 * @fn  PDL_STATUS convertFanControllerAndNumberToFanId
 *
 * @brief   convert fanController ID + fanNumber to fanId
 *          logical fan id is unique across all fan controllers
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in] fanControllerId      - fan controller id
 * @param [in] fanNumber            - fan number
 * @param [in] fanIdPtr             - fan id (unique id used by upper layer app)
 *
 * @return  
 */
PDL_STATUS convertFanControllerAndNumberToFanId(
    IN  UINT_32       fanControllerId,
    IN  UINT_32       fanNumber,
    OUT UINT_32     * fanIdPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_FAN_CONTROLLER_PRV_KEY_STC                 fanControllerKey;
    PDL_FAN_CONTROLLER_PRV_DB_STC            *     fanControllerPtr;
    PDL_FAN_CONNECTED_FANS_PRV_DB_STC        *     connectedFanPtr;
    PDL_STATUS                                     pdlStatus;
    PDL_FAN_CONNECTED_FANS_PRV_KEY_STC             connectedFanKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    fanControllerKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanControllersDb, (void*) &fanControllerKey, (void**) &fanControllerPtr);
    PDL_CHECK_STATUS(pdlStatus);

    connectedFanKey.fanNumber = fanNumber;
    pdlStatus = prvPdlDbFind (&(fanControllerPtr->connectedFans), (void*) &connectedFanKey, (void**) &connectedFanPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *fanIdPtr = connectedFanPtr->fanUniqueId;
    return PDL_OK;
}

/*$ END OF convertFanControllerAndNumberToFanId */

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                     pdlStatus;
    PDL_FAN_CONTROLLER_PRV_DB_STC            *     fanControllerPtr;
    PDL_FAN_CONNECTED_FANS_PRV_DB_STC        *     connectedFanPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlDbGetFirst (&pdlFanControllersDb, (void**) &fanControllerPtr);
    PDL_CHECK_STATUS(pdlStatus);

    while(pdlStatus == PDL_OK)/*Iterate through the fan controllers db*/
    {
        pdlStatus = prvPdlDbGetFirst (&(fanControllerPtr->connectedFans), (void**) &connectedFanPtr);
        PDL_CHECK_STATUS(pdlStatus);

        while(pdlStatus == PDL_OK)/*search in all the connected fans*/
        {
            if(connectedFanPtr->fanUniqueId == fanId)
            {
                *fanControllerIdPtr = fanControllerPtr->fanControllerIdKey.fanControllerId;
                *fanNumberPtr = connectedFanPtr->fanNumber;
                return PDL_OK;
            }
            /*next connected fan*/
            pdlStatus = prvPdlDbGetNext (&(fanControllerPtr->connectedFans),(void*) &(connectedFanPtr->connectedFanKey),(void**) &connectedFanPtr);
        }
        /*next fan controller*/
        pdlStatus = prvPdlDbGetNext (&pdlFanControllersDb, (void*) &(fanControllerPtr->fanControllerIdKey) ,(void**) &fanControllerPtr);
    }
    return PDL_NO_SUCH;
}

/*$ END OF pdlFanIdToFanControllerAndNumber */

/**
 * @fn  PDL_STATUS pdlFanDbGetFirst
 *
 * @brief   return the Id of the first connected fan from the first fan controller 
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [out]   fanIdPtr      - fan id (unique id used by upper layer app)
 *
 * @return  
 */
PDL_STATUS pdlFanDbGetFirst(
    OUT UINT_32     * fanIdPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                   pdlStatus;
    PDL_FAN_ID_PRV_DB_STC                      * fanIdInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    if (fanIdPtr == NULL) {
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlDbGetFirst (&pdlFanIdsDb, (void**) &fanIdInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *fanIdPtr = fanIdInfoPtr->fanId;

    return PDL_OK;
}
/*$ END OF pdlFanDbGetFirst */

/**
 * @fn  PDL_STATUS pdlFanDbGetNext
 *
 * @brief   return the id of the next connected fan after the fan with the param fan Id 
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
    IN  UINT_32                          fanId,
    OUT UINT_32                       *  fanIdPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                           pdlStatus;
    PDL_FAN_ID_PRV_DB_STC                fanIdKey;
    PDL_FAN_ID_PRV_DB_STC              * fanIdInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    if (fanIdPtr == NULL) {
        return PDL_BAD_PTR;
    }
    fanIdKey.fanId = fanId;
    pdlStatus = prvPdlDbGetNext (&pdlFanIdsDb, (void*) &fanIdKey, (void**) &fanIdInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);
    /*logical Id*/
    *fanIdPtr = fanIdInfoPtr->fanId;
    return PDL_OK;
}
/*$ END OF pdlFanDbGetNext */

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                           pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    if (numOfFansPtr == NULL) {
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlDbGetNumOfEntries (&pdlFanIdsDb, numOfFansPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}
/*$ END OF pdlFanDbGetNumOfConnectedFans */

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
    OUT UINT_32     *   fanControllerIdPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                     pdlStatus;
    PDL_FAN_CONTROLLER_PRV_DB_STC            *     fanControllerPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    pdlStatus = prvPdlDbGetFirst (&pdlFanControllersDb, (void**) &fanControllerPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *fanControllerIdPtr = (fanControllerPtr->fanControllerIdKey).fanControllerId;

    return PDL_OK;
}

/*$ END OF pdlFanDbGetFirstController */

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                     pdlStatus;
    PDL_FAN_CONTROLLER_PRV_DB_STC            *     fanControllerPtr;
    PDL_FAN_CONTROLLER_PRV_KEY_STC                 fanControllerKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    fanControllerKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanControllersDb, (void*) &fanControllerKey, (void**) &fanControllerPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlDbGetNext (&pdlFanControllersDb, (void*) &(fanControllerPtr->fanControllerIdKey) ,(void**) &fanControllerPtr);
    PDL_CHECK_STATUS(pdlStatus);

    *fanControllerIdPtr = fanControllerPtr->fanControllerIdKey.fanControllerId;
    return PDL_OK;
}
/*$ END OF pdlFanDbGetNextController */

/**
 * @fn  PDL_STATUS pdlFanDbGetCallbacks ( IN UINT_32 fanControllerId, OUT PDL_FAN_PRV_CALLBACK_FUNC_STC ** callbacksPtr )
 *
 * @brief   Return callbacks structure for given fan controller_id
 *
 * @param [in ]  fanControllerId             Identifier for the fan controller .
 * @param [out]  callbacksPtr                fan callback structure pointer
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlFanDbGetCallbacks (
    IN  UINT_32                           fanControllerId,
    OUT PDL_FAN_PRV_CALLBACK_INFO_STC **  callbacksPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_CONTROLLER_PRV_KEY_STC      fanControllerKey;
    PDL_FAN_CONTROLLER_PRV_DB_STC    *  fanControllerPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    if (callbacksPtr == NULL) {
        return PDL_BAD_PTR;
    }
    fanControllerKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanControllersDb, (void*) &fanControllerKey, (void**) &fanControllerPtr);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlDbFind (&pdlFanCallbacksDb, (void*) &(fanControllerPtr->fanTypeKey), (void**) callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);
  
    return PDL_OK;
}

/*$ END OF pdlFanDbGetCallbacks */

/**
 * @fn  PDL_STATUS pdlFanDbCapabilitiesGet ( IN UINT_32 fanControllerId, OUT PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC * capabilitiesPtr )
 *
 * @brief   Pdl fan database capabilities get
 *
 * @param [in ]  fanControllerId             Identifier for the fan controller .
 * @param [out]  capabilitiesPtr             If non-null, the capabilities pointer.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlFanDbCapabilitiesGet (
    IN  UINT_32                                     fanControllerId,
    OUT PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC  *  capabilitiesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    *  callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    if (capabilitiesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    /* by default set all capabilities to FALSE*/
    memset (capabilitiesPtr, 0, sizeof(PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC));

    if (callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleSetCallbackInfo.fanControllerHwDutyCycleSetFun != NULL) {
        capabilitiesPtr->dutyCycleControlIsSupported = TRUE;
    }
    if (callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleGetCallbackInfo.fanHwDutyCycleGetFun != NULL) {
        capabilitiesPtr->dutyCycleInfoIsSupported = TRUE;
    }
    if (callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleMethodSetCallbackInfo.fanControllerHwDutyCycleMethodSetFun != NULL) {
        capabilitiesPtr->dutyCycleMethodControlIsSupported = TRUE;
    }
    if (callbacksPtr->externalCallbackInfo.fanControllerFaultSetCallbackInfo.fanControllerHwFaultSetFun != NULL) {
        capabilitiesPtr->faultControlIsSupported = TRUE;
    }
    if (callbacksPtr->externalCallbackInfo.fanHwPulsesPerRotationSetCallbackInfo.fanHwPulsesPerRorationSetFun != NULL) {
        capabilitiesPtr->pulsesPerRotationControlIsSupported = TRUE;
    }
    if (callbacksPtr->externalCallbackInfo.fanControllerHwRotationDirectionSetCallbackInfo.fanControllerHwRotationDirectionSetFun != NULL) {
        capabilitiesPtr->rotationDirectionControlIsSupported = TRUE;
    }
    if (callbacksPtr->externalCallbackInfo.fanHwControllerRpmGetInfo.fanControllerRpmGetFun != NULL) {
        capabilitiesPtr->rpmInfoSupported = TRUE;
    }
    if (callbacksPtr->externalCallbackInfo.fanHwStateSetCallbackInfo.fanHwStateSetFun != NULL) {
        capabilitiesPtr->stateControlIsSupported = TRUE;
    }
    if (callbacksPtr->externalCallbackInfo.fanHwStatusGetCallbackInfo.fanHwStatusGetFun != NULL) {
        capabilitiesPtr->stateInfoIsSupported = TRUE;
    }
    if (callbacksPtr->externalCallbackInfo.fanHwThresholdSetCallbackInfo.fanHwThresholdSetFun != NULL) {
        capabilitiesPtr->thresholdControlIsSupported = TRUE;
    }
    return PDL_OK;
}

/*$ END OF pdlFanDbCapabilitiesGet */

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                           pdlStatus;
    PDL_FAN_CONTROLLER_PRV_KEY_STC       fanControllerKey;
    PDL_FAN_CONTROLLER_PRV_DB_STC     *  fanControllerPtr;
    PDL_FAN_CONNECTED_FANS_PRV_DB_STC *  connectedFanPtr;
    PDL_FAN_CONNECTED_FANS_PRV_KEY_STC   connectedFanKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    fanControllerKey.fanControllerId = fanControllerId;
    connectedFanKey.fanNumber = fanNumber;
    pdlStatus = prvPdlDbFind (&pdlFanControllersDb, (void*) &fanControllerKey, (void**) &fanControllerPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlDbFind (&(fanControllerPtr->connectedFans), (void*) &connectedFanKey, (void**) &connectedFanPtr);
    PDL_CHECK_STATUS(pdlStatus);

    *rolePtr = connectedFanPtr->fanRole;

    return PDL_OK;
}

/*$ END OF pdlFanDbRoleGet */

/**
 * @fn  PDL_STATUS pdlFanHwStatusGet (IN UINT_32 fanControllerId, IN UINT_32 fanNumber , OUT PDL_FAN_STATE_INFO_ENT * statePtr);
 *
 * @brief   Configure fan state (normal/shutdown)
 *
 * @param [in ]  fanControllerId     - fan controller identifier
 * @param [in ]  fanNumber           - fan number (key in the connected fan db)
 * @param [out]  statePtr            - fan state(normal/shutdown)
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlFanHwStatusGet (
    IN  UINT_32                         fanControllerId,
    IN  UINT_32                         fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT      *   statePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    *  callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanHwStatusGetCallbackInfo.fanHwStatusGetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanHwStatusGetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanHwStatusGetCallbackInfo.fanHwStatusGetFun(fanControllerId , fanNumber ,  statePtr);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanHwStatusGetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);

    return pdlStatus;
}

/*$ END OF pdlFanHwStatusGet */

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
    IN  UINT_32                         fanControllerId,
    IN  UINT_32                         fanNumber,
    IN  PDL_FAN_STATE_CONTROL_ENT       state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    *  callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanHwStateSetCallbackInfo.fanHwStateSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanHwStateSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanHwStateSetCallbackInfo.fanHwStateSetFun(fanControllerId, fanNumber, state);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanHwStateSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);

    return pdlStatus;
}

/*$ END OF pdlFanHwStateSet */


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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    UINT_32                             speed;
    PDL_FAN_PRV_CALLBACK_INFO_STC    *  callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleGetCallbackInfo.fanHwDutyCycleGetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleGetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleGetCallbackInfo.fanHwDutyCycleGetFun(fanControllerId, fanNumber, &speed, speedPercentPtr);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleGetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);
    *speedPtr = speed;

    return pdlStatus;
}
/*$ END OF pdlFanHwDutyCycleGet */

/**
 * @fn  PDL_STATUS pdlFanControllerHwDutyCycleSet ( IN UINT_32 fanControllerId, IN UINT_32 speed )
 *
 * @brief   Pdl fan hardware duty cycle set
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [in]  speed                   The speed relative value.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlFanControllerHwDutyCycleSet (
    IN  UINT_32                      fanControllerId,
    IN  UINT_32                      speed
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    *  callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleSetCallbackInfo.fanControllerHwDutyCycleSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleSetCallbackInfo.fanControllerHwDutyCycleSetFun(fanControllerId,speed);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);

    return pdlStatus;
}

/*$ END OF pdlFanControllerHwDutyCycleSet */

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
    IN  UINT_32                        fanControllerId,
    OUT PDL_FAN_DIRECTION_ENT      *   directionPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_FAN_DIRECTION_ENT               direction;
    PDL_STATUS                          pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    *  callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanControllerHwRotationDirectionGetCallbackInfo.fanControllerHwRotationDirectionGetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanControllerHwRotationDirectionGetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanControllerHwRotationDirectionGetCallbackInfo.fanControllerHwRotationDirectionGetFun(fanControllerId,&direction);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanControllerHwRotationDirectionGetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);
    *directionPtr = direction;

    return PDL_OK;
}

/*$ END OF pdlFanControllerHwRotationDirectionGet */

PDL_STATUS pdlFanControllerHwRotationDirectionSet (
    IN  UINT_32                        fanControllerId,
    IN  PDL_FAN_DIRECTION_ENT          direction
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    *  callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanControllerHwRotationDirectionSetCallbackInfo.fanControllerHwRotationDirectionSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanControllerHwRotationDirectionSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanControllerHwRotationDirectionSetCallbackInfo.fanControllerHwRotationDirectionSetFun(fanControllerId,direction);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanControllerHwRotationDirectionSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlFanControllerHwRotationDirectionSet */

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
    IN  UINT_32                         fanControllerId,
    IN  PDL_FAN_DUTY_CYCLE_METHOD_ENT   dutyCycleMethod
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T  N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    *  callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleMethodSetCallbackInfo.fanControllerHwDutyCycleMethodSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleMethodSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleMethodSetCallbackInfo.fanControllerHwDutyCycleMethodSetFun(fanControllerId,dutyCycleMethod);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanControllerHwDutyCycleMethodSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlFanHwDutyCycleMethodSet */

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
    IN  UINT_32                        fanControllerId,
    IN  PDL_FAN_FAULT_ENT              faultControl
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    *  callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanControllerFaultSetCallbackInfo.fanControllerHwFaultSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanControllerFaultSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanControllerFaultSetCallbackInfo.fanControllerHwFaultSetFun(fanControllerId , faultControl);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanControllerFaultSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlFanHwFaultSet */

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
    IN  UINT_32                        fanControllerId,
    IN  UINT_32                        fanNumber,
    IN  UINT_32                        pulsesPerRotation
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                         pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    * callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanHwPulsesPerRotationSetCallbackInfo.fanHwPulsesPerRorationSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanHwPulsesPerRotationSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanHwPulsesPerRotationSetCallbackInfo.fanHwPulsesPerRorationSetFun(fanControllerId,fanNumber , pulsesPerRotation);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanHwPulsesPerRotationSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlFanHwPulsesPerRotationSet */

/**
 * @fn  PDL_STATUS pdlFanHwThresholdSet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, IN UINT_32 threshold )
 *
 * @brief   Pdl fan hardware threshold set
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [in]  fanNumber               Fan number
 * @param [in]  thresholdRelativeVal    The threshold relative value.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlFanHwThresholdSet (
    IN  UINT_32                       fanControllerId,
    IN  UINT_32                       fanNumber,
    IN  UINT_32                       threshold
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                        pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC  *  callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/   
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanHwThresholdSetCallbackInfo.fanHwThresholdSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanHwThresholdSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanHwThresholdSetCallbackInfo.fanHwThresholdSetFun(fanControllerId , fanNumber , threshold);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanHwThresholdSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlFanHwThresholdSet */

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                         pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    * callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanHwInitCallbackInfo.fanControllerHwInitFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanHwInitCallbackInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanHwInitCallbackInfo.fanControllerHwInitFun(fanControllerId);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanHwInitCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlFanControllerHwInit */


/**
 * @fn  PDL_STATUS pdlFanControllerGetType ( IN UINT_32 fanControllerId);
 *
 * @brief   Get fan controller type
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] controllerTypePtr   - fan controller type (i.e tc654/adt7476)
 *
 * @return  
 */
PDL_STATUS pdlFanControllerGetType (
    IN  UINT_32                           fanControllerId,
    OUT PDL_FAN_CONTROLLER_TYPE_ENT     * controllerTypePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                     pdlStatus;
    PDL_FAN_CONTROLLER_PRV_KEY_STC                 fanControllerKey;
    PDL_FAN_CONTROLLER_PRV_DB_STC            *     fanControllerPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    if (controllerTypePtr == NULL) {
        return PDL_BAD_PTR;
    }

    fanControllerKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanControllersDb, (void*) &fanControllerKey, (void**) &fanControllerPtr);
    PDL_CHECK_STATUS(pdlStatus);

    *controllerTypePtr = fanControllerPtr->fanTypeKey.fanType;
    return PDL_OK;
}

/*$ END OF pdlFanControllerGetType */


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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                         pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC    * callbacksPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    pdlStatus = pdlFanDbGetCallbacks (fanControllerId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if(callbacksPtr->externalCallbackInfo.fanHwControllerRpmGetInfo.fanControllerRpmGetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->externalCallbackInfo.fanHwControllerRpmGetInfo.lockType);
    pdlStatus = callbacksPtr->externalCallbackInfo.fanHwControllerRpmGetInfo.fanControllerRpmGetFun(fanControllerId, rpmPtr);
    prvPdlUnlock(callbacksPtr->externalCallbackInfo.fanHwControllerRpmGetInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlFanControllerHwRpmGet */

/* ***************************************************************************
* FUNCTION NAME: prvPdlFanGetInterface
*
* DESCRIPTION:  return FAN interface information
*
* PARAMETERS:   
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*                
*****************************************************************************/

PDL_STATUS prvPdlFanGetInterface (
    IN  UINT_32                           fanControllerId,
    OUT PDL_FAN_INTERFACE_STC           * fanInterfacePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                     pdlStatus;
    PDL_FAN_CONTROLLER_PRV_KEY_STC                 fanControllerKey;
    PDL_FAN_CONTROLLER_PRV_DB_STC            *     fanControllerPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    if (fanInterfacePtr == NULL) {
        return PDL_BAD_PTR;
    }

    fanControllerKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanControllersDb, (void*) &fanControllerKey, (void**) &fanControllerPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy (fanInterfacePtr, &fanControllerPtr->fanInterface, sizeof(PDL_FAN_INTERFACE_STC));
    return PDL_OK;
}

/*$ END OF prvPdlFanGetInterface */

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_FAN_THRESHOLD_PRV_INFO_STC        * thresholdInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    if (thresholdFanSpeedPtr == NULL) {
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlDbGetFirst (&pdlFanThresholdDb, (void**) &thresholdInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *thresholdFanSpeedPtr = thresholdInfoPtr->thresholdKey.fanSpeed;
    return PDL_OK;
}

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_FAN_THRESHOLD_PRV_KEY_STC           thresholdKey;
    PDL_FAN_THRESHOLD_PRV_INFO_STC        * thresholdInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    if (nextThresholdFanSpeedPtr == NULL) {
        return PDL_BAD_PTR;
    }
    thresholdKey.fanSpeed = thresholdFanSpeed;
    pdlStatus = prvPdlDbGetNext (&pdlFanThresholdDb, (void*) &thresholdKey, (void**) &thresholdInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *nextThresholdFanSpeedPtr = thresholdInfoPtr->thresholdKey.fanSpeed;
    return PDL_OK;
}

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 dbSize;
    PDL_STATUS                              pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    if (thresholdNumPtr == NULL) {
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlDbGetNumOfEntries (&pdlFanThresholdDb, (void*) &dbSize);
    PDL_CHECK_STATUS(pdlStatus);
    *thresholdNumPtr = (UINT_8) dbSize;
    return PDL_OK;
}

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
    IN  UINT_8                                    thresholdFanSpeed,
    OUT PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC   * sensorThresholdInfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_FAN_THRESHOLD_PRV_KEY_STC           thresholdKey;
    PDL_FAN_THRESHOLD_PRV_INFO_STC        * thresholdInfoPtr;
    PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC * sensorThresholdDataPtr;  
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    if (sensorThresholdInfoPtr == NULL) {
        return PDL_BAD_PTR;
    }
    thresholdKey.fanSpeed = thresholdFanSpeed;
    pdlStatus = prvPdlDbFind (&pdlFanThresholdDb, (void*) &thresholdKey, (void**) &thresholdInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlDbGetFirst (&thresholdInfoPtr->sensorDb, (void**) &sensorThresholdDataPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy (sensorThresholdInfoPtr, sensorThresholdDataPtr, sizeof(PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC));
    return PDL_OK;
}

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
    IN  UINT_8                                    thresholdFanSpeed,
    IN  PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC   * currentSensorThresholdInfoPtr,
    OUT PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC   * nextSensorThresholdInfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_FAN_THRESHOLD_PRV_KEY_STC           thresholdKey;
    PDL_FAN_THRESHOLD_PRV_INFO_STC        * thresholdInfoPtr;
    PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC * sensorThresholdDataPtr;  
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    if (currentSensorThresholdInfoPtr == NULL || nextSensorThresholdInfoPtr == NULL) {
        return PDL_BAD_PTR;
    }
    thresholdKey.fanSpeed = thresholdFanSpeed;
    pdlStatus = prvPdlDbFind (&pdlFanThresholdDb, (void*) &thresholdKey, (void**) &thresholdInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlDbGetNext (&thresholdInfoPtr->sensorDb, (void*) &currentSensorThresholdInfoPtr->sensorThresholdKey, (void**) &sensorThresholdDataPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy (nextSensorThresholdInfoPtr, sensorThresholdDataPtr, sizeof(PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC));
    return PDL_OK;
}

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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/ 
    return pdlSwFanSupported;
}

/*$ END OF pdlSwFanIsSupported */


/**
 * @fn  PDL_STATUS prvPdlFanXMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief  parse XML data for fans
 *         use registered callbacks to parse specific fan information
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  PDL_STATUS.
 * xml example: <mtsplt:fans>
                 <mtsplt:fan-controllers>
                     <mtsplt:fan-controller-number>1</mtsplt:fan-controller-number>
                     <mtsplt:fan-controller-role>normal</mtsplt:fan-controller-role>
                     <mtsplt:connected-fans>
                         <fan-number>0</fan-number>
                         <fan-id>0</fan-id>
                     </mtsplt:connected-fans>
                     <mtsplt:fan-tc654-group / fan-ADT7476-group>
                       send to fan-tc654/fan-ADT7476 parser  
                     </mtsplt:fan-tc654-group / fan-ADT7476-group>

                 </mtsplt:fan-controllers>
                 </mtsplt:Fans>
 */

PDL_STATUS prvPdlFanXMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                    pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    XML_PARSER_RET_CODE_TYP                 xmlStatus, xmlStatus2, xmlStatus3, xmlStatus4;
    PDL_STATUS                              pdlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlFanControllerId, xmlFanControllerChildId,xmlFanControllerListId, xmlTempChildId, xmlSwFanControllerChildId, xmlSwFanSensorId, xmlSwFanThresholdId;
    PDL_FAN_CONTROLLER_PRV_DB_STC           fanController;
    PDL_FAN_CONTROLLER_PRV_DB_STC       *   fanControllerPtr; 
    UINT_32                                 hasType = 0, maxSize, value, extDrvId;
    PDL_FAN_CONNECTED_FANS_PRV_DB_STC       connectedFan;
    PDL_FAN_CONNECTED_FANS_PRV_DB_STC   *   connectedFanPtr;
    PDL_DB_PRV_ATTRIBUTES_STC               dbAttributes;
    PDL_FAN_PRV_CALLBACK_FUNC_STC  *        callbacksPtr;
    PDL_FAN_ID_PRV_KEY_STC                  fanIdKey;
    PDL_FAN_ID_PRV_DB_STC                   fanIdInfo, * fanIdPtr;
    PDL_FAN_THRESHOLD_PRV_INFO_STC          thresholdInfo, * thresholdInfoPtr;
    PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC   sensorThresholdInfo, *sensorThresholdInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    xmlStatus = xmlParserFindByName (xmlId, "FANs", &xmlFanControllerId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        return PDL_OK;
    }

    /*inside tag fans*/
    xmlStatus = xmlParserGetFirstChild (xmlFanControllerId, &xmlFanControllerChildId);
    XML_CHECK_STATUS(xmlStatus);
    while(xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        if ((xmlParserIsEqualName (xmlFanControllerChildId, "FanController-is-supported") == XML_PARSER_RET_CODE_OK) ||
            (xmlParserIsEqualName (xmlFanControllerChildId, "SwFanController-is-supported") == XML_PARSER_RET_CODE_OK)) {
            /* do nothing */
        }
        /*fan controller number*/
        else if(xmlParserIsEqualName (xmlFanControllerChildId, "fan-controller-list") == XML_PARSER_RET_CODE_OK) {
            memset (&fanController, 0, sizeof (fanController));
            dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_CONNECTED_FANS_PRV_DB_STC);
            dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_CONNECTED_FANS_PRV_KEY_STC);
            pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,  &dbAttributes,  &(fanController.connectedFans));
            xmlStatus2 = xmlParserGetFirstChild (xmlFanControllerChildId, &xmlFanControllerListId);
            while(xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus2);
                if(xmlParserIsEqualName (xmlFanControllerListId, "fan-controller-number") == XML_PARSER_RET_CODE_OK) {
                    maxSize = sizeof(fanController.fanControllerIdKey.fanControllerId);
                    xmlStatus = xmlParserGetValue (xmlFanControllerListId , &maxSize , &fanController.fanControllerIdKey.fanControllerId);
                    XML_CHECK_STATUS(xmlStatus);
                }
                else if(xmlParserIsEqualName (xmlFanControllerListId, "i2c-read-write-address") == XML_PARSER_RET_CODE_OK) {
                    pdlStatus = prvPdlInterfaceXmlParser (xmlFanControllerListId, &fanController.fanInterface.interfaceType, &fanController.fanInterface.interfaceId);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else if(xmlParserIsEqualName (xmlFanControllerListId, "fan-hw") == XML_PARSER_RET_CODE_OK) {
                    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlStatus = xmlParserGetValue(xmlFanControllerListId, &maxSize, &pdlTagStr[0]);
                    XML_CHECK_STATUS(xmlStatus);
                    pdlStatus =  pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_FAN_TYPE_E, pdlTagStr, &value);
                    PDL_CHECK_STATUS(pdlStatus);
                    fanController.fanTypeKey.fanType = (PDL_FAN_CONTROLLER_TYPE_ENT)value;
                    if (value != PDL_FAN_CONTROLLER_TYPE_USER_DEFINED_E) {                        
                        fanController.fanTypeKey.externalDriverId = 0;
                    }
                }
                else if (xmlParserIsEqualName (xmlFanControllerListId, "external-driver-id") == XML_PARSER_RET_CODE_OK) {
                    maxSize = sizeof(extDrvId);
                    xmlStatus = xmlParserGetValue (xmlFanControllerListId, &maxSize, &extDrvId);
                    XML_CHECK_STATUS(xmlStatus);
                    fanController.fanTypeKey.externalDriverId = extDrvId;
                }
                else if(xmlParserIsEqualName (xmlFanControllerListId, "fan-connected-list") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus3 = xmlParserGetFirstChild (xmlFanControllerListId, &xmlTempChildId);
                    while (xmlStatus3 != XML_PARSER_RET_CODE_NOT_FOUND) {
                        XML_CHECK_STATUS(xmlStatus3);
                        if(xmlParserIsEqualName (xmlTempChildId, "fan-number") == XML_PARSER_RET_CODE_OK) {
                            maxSize = sizeof(connectedFan.fanNumber);
                            xmlStatus = xmlParserGetValue (xmlTempChildId, &maxSize, &(connectedFan.fanNumber));
                            XML_CHECK_STATUS(xmlStatus);
                        }
                        else if(xmlParserIsEqualName (xmlTempChildId, "fan-id") == XML_PARSER_RET_CODE_OK) {
                            maxSize = sizeof(connectedFan.fanUniqueId);
                            xmlStatus = xmlParserGetValue (xmlTempChildId, &maxSize, &(connectedFan.fanUniqueId));
                            XML_CHECK_STATUS(xmlStatus);
                        }
                        else if(xmlParserIsEqualName (xmlTempChildId, "fan-role") == XML_PARSER_RET_CODE_OK) {
                            maxSize = PDL_XML_MAX_TAG_LEN;
                            xmlStatus = xmlParserGetValue (xmlTempChildId, &maxSize, &pdlTagStr);
                            XML_CHECK_STATUS(xmlStatus);
                            if (strcmp (pdlTagStr, "normal") == 0) {
                                connectedFan.fanRole = PDL_FAN_ROLE_NORMAL_E;
                            }
                            else if (strcmp (pdlTagStr, "redundant") == 0) {
                                connectedFan.fanRole = PDL_FAN_ROLE_REDUNDANT_E;
                            }
                            else {
                                return PDL_XML_PARSE_ERROR;
                            }
                        }
                        else {
                            return PDL_XML_PARSE_ERROR;
                        }
                        xmlStatus3 = xmlParserGetNextSibling (xmlTempChildId, &xmlTempChildId);                
                    }
                    connectedFan.connectedFanKey.fanNumber = connectedFan.fanNumber;
                    pdlStatus = prvPdlDbAdd (&(fanController.connectedFans) , (void*) &(connectedFan.connectedFanKey), (void*) &connectedFan, (void**) &connectedFanPtr);   
                    PDL_CHECK_STATUS(pdlStatus);
                    fanIdKey.fanId = connectedFan.fanUniqueId;
                    fanIdInfo.fanId = connectedFan.fanUniqueId;
                    pdlStatus = prvPdlDbAdd (&pdlFanIdsDb , (void*) &fanIdKey, (void*) &fanIdInfo, (void**) &fanIdPtr);   
                    PDL_CHECK_STATUS(pdlStatus);    
                }
                xmlStatus2 = xmlParserGetNextSibling (xmlFanControllerListId, &xmlFanControllerListId);
            }
            pdlStatus = prvPdlDbAdd (&pdlFanControllersDb , (void*) &(fanController.fanControllerIdKey), (void*) &fanController, (void**) &fanControllerPtr);   
            PDL_CHECK_STATUS(pdlStatus);
        }
        else if (xmlParserIsEqualName (xmlFanControllerChildId, "sw-fan-controllers") == XML_PARSER_RET_CODE_OK) {
            pdlSwFanSupported = TRUE;
            xmlStatus2 = xmlParserGetFirstChild (xmlFanControllerChildId, &xmlSwFanControllerChildId);
            memset (&thresholdInfo, 0, sizeof (thresholdInfo));
            dbAttributes.listAttributes.entrySize = sizeof(PDL_SW_FAN_SENSOR_THRESHOLD_ENTRY_STC);
            dbAttributes.listAttributes.keySize = sizeof(PDL_SW_FAN_SENSOR_THRESHOLD_KEY_STC);
            pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,  &dbAttributes,  &(thresholdInfo.sensorDb));
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,  &dbAttributes,  &(thresholdInfo.miscSensorDb));
            PDL_CHECK_STATUS(pdlStatus);
            thresholdInfo.thresholdState = PDL_FAN_THRESHOLD_STATE_NORMAL_E;
            while(xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
                XML_CHECK_STATUS(xmlStatus2);
                if (xmlParserIsEqualName (xmlSwFanControllerChildId, "threshold-list") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus3 = xmlParserGetFirstChild (xmlSwFanControllerChildId, &xmlSwFanThresholdId);
                    while (xmlStatus3 != XML_PARSER_RET_CODE_NOT_FOUND) {
                        XML_CHECK_STATUS(xmlStatus3);
                        if (xmlParserIsEqualName (xmlSwFanThresholdId, "fan-speed") == XML_PARSER_RET_CODE_OK) {
                            maxSize = sizeof(thresholdInfo.thresholdKey.fanSpeed);
                            xmlStatus = xmlParserGetValue (xmlSwFanThresholdId, &maxSize, &(thresholdInfo.thresholdKey.fanSpeed));
                            XML_CHECK_STATUS(xmlStatus);
                        }
                        else if (xmlParserIsEqualName (xmlSwFanThresholdId, "threshold-state") == XML_PARSER_RET_CODE_OK) {
                            maxSize = PDL_XML_MAX_TAG_LEN;
                            xmlStatus = xmlParserGetValue(xmlSwFanThresholdId, &maxSize, &pdlTagStr[0]);
                            XML_CHECK_STATUS(xmlStatus);
                            if (strcmp (pdlTagStr, "normal") == 0) {
                                thresholdInfo.thresholdState = PDL_FAN_THRESHOLD_STATE_NORMAL_E;
                            }
                            else if (strcmp (pdlTagStr, "warning") == 0) {
                                thresholdInfo.thresholdState = PDL_FAN_THRESHOLD_STATE_WARNING_E;
                            }
                            else if (strcmp (pdlTagStr, "critical") == 0) {
                                thresholdInfo.thresholdState = PDL_FAN_THRESHOLD_STATE_CRITICAL_E;
                            }
                            else {
                                PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"unrecognized threshold-state [%s]!!!\n", pdlTagStr);
                                return PDL_XML_PARSE_ERROR;
                            }
                        }
                        else if (xmlParserIsEqualName (xmlSwFanThresholdId, "sensor-list") == XML_PARSER_RET_CODE_OK) {
                            xmlStatus4 = xmlParserGetFirstChild (xmlSwFanThresholdId, &xmlSwFanSensorId);
                            PDL_CHECK_STATUS(pdlStatus);
                            while (xmlStatus4 != XML_PARSER_RET_CODE_NOT_FOUND) {
                                XML_CHECK_STATUS(xmlStatus4);
                                if (xmlParserIsEqualName (xmlSwFanSensorId, "sensor-number") == XML_PARSER_RET_CODE_OK) {
                                    maxSize = sizeof(sensorThresholdInfo.sensorThresholdKey);
                                    xmlStatus = xmlParserGetValue (xmlSwFanSensorId, &maxSize, &(sensorThresholdInfo.sensorThresholdKey));
                                    XML_CHECK_STATUS(xmlStatus);
                                }
                                else if (xmlParserIsEqualName (xmlSwFanSensorId, "threshold-temperature") == XML_PARSER_RET_CODE_OK) {
                                    maxSize = sizeof(sensorThresholdInfo.sensorThresholdTemprature);
                                    xmlStatus = xmlParserGetValue (xmlSwFanSensorId, &maxSize, &(sensorThresholdInfo.sensorThresholdTemprature));
                                    XML_CHECK_STATUS(xmlStatus);
                                }
                                else {
                                    PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"unrecognized sensor threshold tag!!!\n");
                                    return PDL_XML_PARSE_ERROR;
                                }
                                xmlStatus4 = xmlParserGetNextSibling (xmlSwFanSensorId, &xmlSwFanSensorId);
                            }
                            pdlStatus = prvPdlDbAdd (&thresholdInfo.sensorDb , (void*) &sensorThresholdInfo.sensorThresholdKey, (void*) &sensorThresholdInfo, (void**) &sensorThresholdInfoPtr);   
                            PDL_CHECK_STATUS(pdlStatus);
                        }
                        else {
                            return PDL_XML_PARSE_ERROR;
                        }
                        xmlStatus3 = xmlParserGetNextSibling (xmlSwFanThresholdId, &xmlSwFanThresholdId);
                    }
                    pdlStatus = prvPdlDbAdd (&pdlFanThresholdDb , (void*) &thresholdInfo.thresholdKey, (void*) &thresholdInfo, (void**) &thresholdInfoPtr);   
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else {
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus2 = xmlParserGetNextSibling (xmlSwFanControllerChildId, &xmlSwFanControllerChildId);
            }

        }
        else {
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling(xmlFanControllerChildId, &xmlFanControllerChildId);
    }
if (0) {     
        if(hasType){/*parse this fan according to his type*/
            pdlStatus = prvPdlDbFind (&pdlFanCallbacksDb, (void*) &(fanController.fanTypeKey), (void**) &callbacksPtr);
            PDL_CHECK_STATUS(pdlStatus);

            if(callbacksPtr->fanXMLParserFun == NULL) {
                return PDL_NOT_SUPPORTED;
            }
            pdlStatus = callbacksPtr->fanXMLParserFun(xmlFanControllerChildId , (fanController.fanControllerIdKey.fanControllerId));
            PDL_CHECK_STATUS(pdlStatus);
        }
}
    return PDL_OK;
}

/*$ END OF prvPdlFanXMLParser */

/**
 * @fn  PDL_STATUS pdlFanDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Fan debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlFanDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlFanDebugFlag = state;
    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlFanInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId );
 *
 * @brief   Init fan modules
 *          call all registered fan init + parser functions
 *          create local DB for all get, getNext functions
 *
 * @param [in]  xmlId   - Xml id.
 *
 * @return  PDL_STATUS.
 */

/* ***************************************************************************
* FUNCTION NAME: prvPdlFanVerifyExternalDrivers
*
* DESCRIPTION:   verify all external drivers have been binded
*
* PARAMETERS:   
*                
*
*****************************************************************************/

PDL_STATUS prvPdlFanVerifyExternalDrivers(
	IN void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                                    pdlStatus;
    PDL_FAN_PRV_CALLBACK_INFO_STC               * callbacksPtr;
    PDL_FAN_CONTROLLER_PRV_KEY_STC                fanControllerKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlDbGetNextKey (&pdlFanControllersDb, NULL, (void*) &fanControllerKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = pdlFanDbGetCallbacks (fanControllerKey.fanControllerId, &callbacksPtr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlDbGetNextKey (&pdlFanControllersDb, (void*) &fanControllerKey, (void*) &fanControllerKey);
    }
    return PDL_OK;
}

/*$ END OF prvPdlFanVerifyExternalDrivers */


PDL_STATUS pdlFanInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
    )
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC               dbAttributes;
    PDL_FAN_PRV_CALLBACK_INFO_STC  *        callbacksPtr;
    PDL_FAN_CONTROLLER_TYPE_PRV_KEY_STC     fanTypeKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_CONTROLLER_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_CONTROLLER_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,  &dbAttributes,  &pdlFanControllersDb);
    PDL_CHECK_STATUS(pdlStatus);

    dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_ID_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_ID_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,  &dbAttributes,  &pdlFanIdsDb);
    PDL_CHECK_STATUS(pdlStatus);

    dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_THRESHOLD_PRV_INFO_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_THRESHOLD_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,  &dbAttributes,  &pdlFanThresholdDb);
    PDL_CHECK_STATUS(pdlStatus);

    /*init the fans */
    pdlStatus = prvPdlDbGetNextKey (&pdlFanCallbacksDb, NULL, (void*) &fanTypeKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlDbFind (&pdlFanCallbacksDb, (void*) &(fanTypeKey), (void**) &callbacksPtr);
        PDL_CHECK_STATUS(pdlStatus);
        if(callbacksPtr->externalCallbackInfo.fanDbInitCallbackInfo.fanDbInitFun != NULL) {
            pdlStatus = callbacksPtr->externalCallbackInfo.fanDbInitCallbackInfo.fanDbInitFun();
            PDL_CHECK_STATUS(pdlStatus);
        }
        pdlStatus = prvPdlDbGetNextKey (&pdlFanCallbacksDb, (void*) &fanTypeKey, (void*) &fanTypeKey);
    }

    pdlStatus = prvPdlFanXMLParser (xmlId);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;   
}

/*$ END OF pdlFanInit */


/**
 * @fn  PDL_STATUS prvPdlFanDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanDestroy (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_FAN_CONTROLLER_PRV_KEY_STC          fanControllerIdKey;
    PDL_FAN_CONTROLLER_PRV_DB_STC         * fanControllerPtr; 
    PDL_FAN_THRESHOLD_PRV_KEY_STC           thresholdKey;
    PDL_FAN_THRESHOLD_PRV_INFO_STC        * thresholdInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlDbGetNextKey (&pdlFanControllersDb, NULL, (void*) &fanControllerIdKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus  = prvPdlDbFind (&pdlFanControllersDb, (void*) &fanControllerIdKey, (void**) &fanControllerPtr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlDbDestroy(&fanControllerPtr->connectedFans);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlDbGetNextKey (&pdlFanControllersDb, (void*) &fanControllerIdKey, (void*) &fanControllerIdKey);
    }

    pdlStatus = prvPdlDbGetNextKey (&pdlFanThresholdDb, NULL, (void*) &thresholdKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus  = prvPdlDbFind (&pdlFanThresholdDb, (void*) &thresholdKey, (void**) &thresholdInfoPtr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlDbDestroy(&thresholdInfoPtr->sensorDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlDbDestroy(&thresholdInfoPtr->miscSensorDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlDbGetNextKey (&pdlFanControllersDb, (void*) &fanControllerIdKey, (void*) &fanControllerIdKey);
    }

    if (pdlFanCallbacksDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlFanCallbacksDb);
        PDL_CHECK_STATUS(pdlStatus);
    }
    if (pdlFanControllersDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlFanControllersDb);
        PDL_CHECK_STATUS(pdlStatus);    
    }
    if (pdlFanIdsDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlFanIdsDb);
        PDL_CHECK_STATUS(pdlStatus);    
    }
    if (pdlFanThresholdDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlFanThresholdDb);
        PDL_CHECK_STATUS(pdlStatus);    
    }

    return PDL_OK;
}

/*$ END OF prvPdlFanDestroy */