/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlFanController.c   
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
#include <pdl/fan/private/prvPdlFanAdt7476.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/xmlParser.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/fan/pdlFanDebug.h>
#include <pdl/interface/private/prvPdlI2c.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
/** @brief   The pdl fan adt7476 database */
static PDL_DB_PRV_STC pdlFanAdt7476Db =  {0,NULL};
/*callbacks struct*/
PDL_FAN_PRV_CALLBACK_FUNC_STC pdlFanCallbacksAdt7476 = 
{
    &prvPdlFanAdt7476HwStatusGet,
    &prvPdlFanAdt7476HwStateSet,
    NULL,
    &prvPdlFanAdt7476HwDutyCycleSet,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &prvPdlFanAdt7476HwInit,
    &prvPdlFanAdt7476DbInit,
    NULL,
    &prvPdlFanAdt7476XMLParser
};
/*PDL_FAN_PRV_CALLBACK_FUNC_STC pdlFanCallbacksAdt7476 = {pdlFanAdt7476DbCapabilitiesGet,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};*/

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

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476HwStatusGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT PDL_FAN_STATE_INFO_ENT * statePtr );
 *
 * @brief   Get fan operational state (ok/overflow/fault)
 *
 * @param [in]      fanControllerId       - fan controller identifier.
 * @param [in]      fanNumber             - fan identifier.
 * @param [in,out]  statePtr              - If non-null, the state pointer.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS prvPdlFanAdt7476HwStatusGet (
    IN  UINT_32                      fanControllerId,
    IN  UINT_32                      fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT     * statePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                          pdlStatus;
    UINT_32                             maskBit;
    UINT_32                             state;
    PDL_FAN_INTERFACE_STC               fanInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    if (statePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"statePtr NULL, FAN number: %d",fanNumber);
        return PDL_BAD_PTR;
    }
    switch(fanNumber){
    case 1:
        maskBit = 2;
        break;
    case 2:
        maskBit = 3;
        break;
    case 3:
        maskBit = 4;
        break;
    case 4:
        maskBit = 5;
        break;
    default:
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL FAN number: %d",fanNumber);
        return PDL_BAD_VALUE;
    }
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);    

    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId , 0x42  , maskBit , &state);
    PDL_CHECK_STATUS(pdlStatus);
    switch (state) {
    case 0:
        *statePtr = PDL_FAN_STATE_INFO_OK_E;
        break;
    default:
        *statePtr = PDL_FAN_STATE_INFO_FAULT_E;
        break;
    }
    return PDL_OK;
}


/**
 * @fn  PDL_STATUS prvPdlFanAdt7476SetStat ( IN PDL_DB_PRV_STC * statementsDbPtr, IN PDL_FAN_ADT7476_PRV_DB_STC * fanPtr )
 * @brief   Pdl set statement value to entire statements db
 * @param [IN]     PDL_DB_PRV_STC                *   statementsDbPtr      the fan's statement db pointer.
 * @param [IN]     PDL_FAN_ADT7476_PRV_DB_STC    *   fanPtr            the fan pointer .
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlFanAdt7476SetState (
    IN   PDL_DB_PRV_STC              *  statementsDbPtr,
    IN   PDL_FAN_ADT7476_PRV_DB_STC  *  fanPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/    
    PDL_STATUS                          pdlStatus;
    UINT_32                             i,numOfEntries;
    PDL_FAN_ADT7476_PRV_STATEMENT_STC * statement;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/    
    PDL_UNUSED_PARAM(fanPtr);

    pdlStatus = prvPdlDbGetNumOfEntries(statementsDbPtr,&numOfEntries);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlDbGetFirst(statementsDbPtr,(void **)&statement);
    for(i=0; i < numOfEntries; i++)
    {
        pdlStatus = prvPdlInterfaceHwSetValue (PDL_INTERFACE_TYPE_I2C_E, statement->i2cInterfaceId, statement->value);
        PDL_CHECK_STATUS(pdlStatus); 
        /* don't getnext for the last entry in db (it'll fail) */
        if (i < numOfEntries - 1) {
            pdlStatus = prvPdlDbGetNext(statementsDbPtr , &(statement->statementKey) ,(void**) &statement);
            PDL_CHECK_STATUS(pdlStatus); 
        }
    }          
    return PDL_OK;
}

/*$ END OF prvPdlFanAdt7476SetState */



/**
 * @fn  PDL_STATUS prvPdlFanAdt7476HwStateSet ( IN UINT_32 fanControllerId, IN PDL_FAN_STATE_CONTROL_ENT state );
 *
 * @brief   Configure fan state (normal/shutdown)
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  state               - state to configure.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476HwStateSet (
    IN  UINT_32                         fanControllerId,
    IN UINT_32                          fanNumber,
    IN  PDL_FAN_STATE_CONTROL_ENT       state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                          pdlStatus;
    PDL_FAN_ADT7476_PRV_DB_STC 	      * fanPtr;
    PDL_FAN_CONTROLLER_PRV_KEY_STC      fanKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    PDL_UNUSED_PARAM(fanNumber);

    fanKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanAdt7476Db, (void*) &fanKey, (void*) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);     

    if (state == PDL_FAN_STATE_CONTROL_NORMAL_E) {
        pdlStatus = prvPdlFanAdt7476SetState(&(fanPtr->allNormalSpeedStatementDb),fanPtr);
        PDL_CHECK_STATUS(pdlStatus);  
    }
    else if (state == PDL_FAN_STATE_CONTROL_SHUTDOWN_E) {
        return PDL_NOT_SUPPORTED;
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL STATE");
        return PDL_BAD_PARAM;
    }              
    return PDL_OK;
}

/*$ END OF prvPdlFanAdt7476HwStateSet */

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476HwInit ( IN UINT_32 fanControllerId )
 *
 * @brief   Pdl fan hardware init
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476HwInit ( 
    IN UINT_32                  fanControllerId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                          pdlStatus;
    PDL_FAN_ADT7476_PRV_DB_STC 	      * fanPtr;
    PDL_FAN_CONTROLLER_PRV_KEY_STC      fanKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    fanKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanAdt7476Db, (void*) &fanKey, (void*) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);     

    pdlStatus = prvPdlFanAdt7476SetState(&(fanPtr->initStatementDb),fanPtr);
    PDL_CHECK_STATUS(pdlStatus);  
    return PDL_OK;
}

/*$ END OF prvPdlFanAdt7476HwInit */

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476HwDutyCycleSet ( IN UINT_32 fanControllerId, IN UINT_32 speed );
 *
 * @brief   for Adt7476, this will execute the normal script in case value is 0-127 and max speed script in case speed is 128-255
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  speed               - fan speed (0-255)
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS prvPdlFanAdt7476HwDutyCycleSet (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      speed
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                          pdlStatus;
    PDL_FAN_ADT7476_PRV_DB_STC 	      * fanPtr;
    PDL_FAN_CONTROLLER_PRV_KEY_STC      fanKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/  
    fanKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanAdt7476Db, (void*) &fanKey, (void*) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);     

    if (speed < 128) {
        pdlStatus = prvPdlFanAdt7476SetState(&(fanPtr->allNormalSpeedStatementDb),fanPtr);
        PDL_CHECK_STATUS(pdlStatus);  
    }
    else if (speed >= 128 && speed < 256) {
        pdlStatus = prvPdlFanAdt7476SetState(&(fanPtr->allMaxSpeedStatementDb),fanPtr);
        PDL_CHECK_STATUS(pdlStatus);  
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL SPEED(NOT IN RANGE 0-255)");
        return PDL_BAD_PARAM;
    }              
    return PDL_OK;
}
/*$ END OF prvPdlFanAdt7476HwDutyCycleSet */


/**
 * @fn  PDL_STATUS prvPdlFanAdt7476XmlNumberOfStatementsGet ( IN PDL_FAN_ADT_7476_CONTROLLER_ID fanControllerId, OUT UINT_32 * numOfStatementsPtr );
 *
 * @brief   get number of script statements
 *          
 *          @note Based on data retrieved from XML
 *
 * @param [in]  fanControllerId     - id for the ADT 7476 fan controller
 * @param [out] numOfStatementsPtr  - number of thresholds supported.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476XmlNumberOfStatementsGet (
    IN  UINT_32                             fanControllerId,
    OUT UINT_32                           * numOfStatementsPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_FAN_ADT7476_PRV_DB_STC        *     fanAdtPointer;
    PDL_FAN_CONTROLLER_PRV_KEY_STC          fanAdtKey;
    UINT_32                                 numOfInitEntries,numOfNormalEntries,numOfMaxSpeedEntries,numOfEnableEntries,numOfDisableEntries; 
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    *numOfStatementsPtr = 0;
    fanAdtKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanAdt7476Db, (void*) &fanAdtKey, (void**) &fanAdtPointer);
    PDL_CHECK_STATUS(pdlStatus);

    /*check number of statement in all statement lists */
    pdlStatus = prvPdlDbGetNumOfEntries(&(fanAdtPointer->allNormalSpeedStatementDb),&numOfNormalEntries);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlDbGetNumOfEntries(&(fanAdtPointer->allMaxSpeedStatementDb),&numOfMaxSpeedEntries);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlDbGetNumOfEntries(&(fanAdtPointer->initStatementDb),&numOfInitEntries);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlDbGetNumOfEntries(&(fanAdtPointer->disableRedundentFanStatementDb),&numOfDisableEntries);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlDbGetNumOfEntries(&(fanAdtPointer->enableFanStatementDb),&numOfEnableEntries);
    PDL_CHECK_STATUS(pdlStatus);
    *numOfStatementsPtr = numOfInitEntries + numOfNormalEntries + numOfMaxSpeedEntries + numOfDisableEntries + numOfEnableEntries; 

    return PDL_OK;
}


/**
 * @fn  PDL_STATUS prvPdlFanAdt7476SetDb ( IN char * operationType, OUT PDL_DB_PRV_STC ** dbPtr )
 * @brief   Pdl set db to add statement to
 * @param [ IN]     char *             operationType         operation type.
 * @param [OUT]     PDL_DB_PRV_STC  ** dbToAddPtr            If non-null, the temporary pointer.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlFanAdt7476SetDb (
    IN   char *                         operationType,
    IN  PDL_FAN_ADT7476_PRV_DB_STC   *  fanAdtDbStructPtr,
    OUT  PDL_DB_PRV_STC **              dbToAddPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_STATUS                           pdlStatus;
    UINT_32                              operationTypeEnum;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    /*convert operation type to enum*/
    pdlStatus =  pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_HW_FAN_OPERATION_TYPE_E,operationType,&operationTypeEnum);
    PDL_CHECK_STATUS(pdlStatus);

    switch(operationTypeEnum){
    case PDL_FAN_OP_TYPE_ADT_7476_INITIALIZE_E:
        *dbToAddPtr = &(fanAdtDbStructPtr->initStatementDb);
        break;

    case PDL_FAN_OP_TYPE_ADT_7476_ALL_NORMAL_SPEED_E:
        *dbToAddPtr = &(fanAdtDbStructPtr->allNormalSpeedStatementDb);
        break;

    case PDL_FAN_OP_TYPE_ADT_7476_ALL_MAX_SPEED_E:
        *dbToAddPtr = &(fanAdtDbStructPtr->allMaxSpeedStatementDb);
        break;
    case PDL_FAN_OP_TYPE_ADT_7476_ENABLE_FAN_SPEED_E:
        *dbToAddPtr = &(fanAdtDbStructPtr->enableFanStatementDb);
        break;
    case PDL_FAN_OP_TYPE_ADT_7476_DISABLE_FAN_SPEED_E:
        *dbToAddPtr = &(fanAdtDbStructPtr->disableRedundentFanStatementDb);
        break;

    default:
        PDL_LIB_DEBUG_MAC(prvPdlFanControllerDebugFlag)(__FUNCTION__, __LINE__,__FUNCTION__, __LINE__,"INVALID OPERATION TYPE: %s",operationType);
        return PDL_XML_PARSE_ERROR;
    }
    return PDL_OK;
}
/*$ END OF prvPdlFanAdt7476SetDb */

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476StatementParse (IN char * operationType ,IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 * @brief   Prv pdl fan controller statement parser
 * @param   xmlId   Identifier for the XML.
 * @return  A PDL_STATUS.
 *
 * @param [IN]     char *                           operationType         operation type.
 * @param [IN]     XML_PARSER_NODE_DESCRIPTOR_TYP   xmlId                 xml pointer.
 * @param [IN]     PDL_FAN_ADT7476_PRV_DB_STC    *  fanAdtDbStructPtr     db entry pointer.
 */
PDL_STATUS prvPdlFanAdt7476StatementParse ( 
    IN  char                             *  operationType,
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  PDL_FAN_ADT7476_PRV_DB_STC       *  fanAdtDbStructPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/  
    PDL_STATUS                               pdlStatus;
    char                                     statmentComment[PDL_FAN_MAX_STAT_COMMENT_LEN];
    XML_PARSER_NODE_DESCRIPTOR_TYP           xmlFanControllerId, xmlFanControllerChildId;
    PDL_FAN_ADT7476_PRV_STATEMENT_KEY_STC    fanStatmentKey;
    PDL_FAN_ADT7476_PRV_STATEMENT_STC        fanStatment;
    XML_PARSER_RET_CODE_TYP                  xmlStatus;
    UINT_32                                  statmentId,statmentValue,maxSize;
    PDL_DB_PRV_STC                        *  dbToAddPtr;
    PDL_INTERFACE_TYP                        interfaceId;
    PDL_INTERFACE_TYPE_ENT                   interfaceType;
    PDL_FAN_ADT7476_PRV_DB_STC             * fanPtr = NULL;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    xmlFanControllerId = xmlId;
    while(xmlParserIsEqualName (xmlFanControllerId, "fan-operation-list") == XML_PARSER_RET_CODE_OK){
        xmlStatus = xmlParserGetFirstChild (xmlFanControllerId, &xmlFanControllerChildId);
        XML_CHECK_STATUS(xmlStatus);
        /*parse one statement*/
        while(xmlStatus == XML_PARSER_RET_CODE_OK){
            if(xmlParserIsEqualName (xmlFanControllerChildId, "fan-operation-number") == XML_PARSER_RET_CODE_OK){
                maxSize = sizeof(statmentId);
                xmlStatus = xmlParserGetValue (xmlFanControllerChildId, &maxSize, &statmentId);
                XML_CHECK_STATUS(xmlStatus);
                fanStatment.statementId = statmentId;
                fanStatmentKey.statmentId = statmentId;
                fanStatment.statementKey = fanStatmentKey;
            }
            else if(xmlParserIsEqualName (xmlFanControllerChildId, "i2c-read-write-address") == XML_PARSER_RET_CODE_OK){
                xmlStatus = prvPdlInterfaceXmlParser (xmlFanControllerChildId, &interfaceType, &interfaceId);
                XML_CHECK_STATUS(xmlStatus);
                fanStatment.i2cInterfaceId = interfaceId;
            }
            else if(xmlParserIsEqualName (xmlFanControllerChildId, "fan-operation-comment") == XML_PARSER_RET_CODE_OK){
                maxSize = PDL_FAN_MAX_STAT_COMMENT_LEN;
                xmlStatus = xmlParserGetValue (xmlFanControllerChildId, &maxSize, &statmentComment);
                XML_CHECK_STATUS(xmlStatus);
                fanStatment.commentPtr = prvPdlOsMalloc(PDL_FAN_MAX_STAT_COMMENT_LEN);
                if (fanStatment.commentPtr == NULL) {
                	return PDL_OUT_OF_CPU_MEM;
                }
                memcpy(fanStatment.commentPtr, &statmentComment[0], strlen(&statmentComment[0])+1);
            }
            else if(xmlParserIsEqualName (xmlFanControllerChildId, "fan-operation-value") == XML_PARSER_RET_CODE_OK){
                maxSize = sizeof(UINT_32);
                xmlStatus = xmlParserGetValue (xmlFanControllerChildId, &maxSize, &statmentValue);
                XML_CHECK_STATUS(xmlStatus);
                fanStatment.value = statmentValue;
            }
            else{
                PDL_LIB_DEBUG_MAC(prvPdlFanControllerDebugFlag)(__FUNCTION__, __LINE__,__FUNCTION__, __LINE__,"INVALID xmlSensorChildId POINTER OR NAME DON'T MATCH");
                return PDL_XML_PARSE_ERROR;
            }
            xmlStatus =xmlParserGetNextSibling (xmlFanControllerChildId, &xmlFanControllerChildId);
        }
        /*set right db to add statement to*/
        pdlStatus = prvPdlFanAdt7476SetDb(&operationType[0],fanAdtDbStructPtr,&dbToAddPtr);
        PDL_CHECK_STATUS(pdlStatus);

        /*add to db*/ 
        pdlStatus = prvPdlDbAdd (dbToAddPtr, (void*) &fanStatmentKey, (void*) &fanStatment, (void**) &fanPtr); 
        PDL_CHECK_STATUS(pdlStatus);

        /*next statement*/
        xmlStatus =xmlParserGetNextSibling (xmlFanControllerId, &xmlFanControllerId);
        if(xmlStatus!=XML_PARSER_RET_CODE_OK)break;/*no more statements*/
        }
    return PDL_OK;
}

/*$ END OF prvPdlFanAdt7476StatementParse */


/* XML structure:
*<fan-ADT7476-group>
    <fan-controller-i2c>
        <i2c-interface>
            <bus-id type="i2c-bus-id-type">0</bus-id>
            <address type="i2c-address-type">0x42</address>                
            <offset type="i2c-offset-type">0x10</offset>
            <access type="i2c-access-type">one-byte</access>
            <mask type="i2c-mask-value-type">0x1</mask>
        </i2c-interface>
        <fault_detection type="fan-ADT7476-fault-detection-type">interrupt-status</fault_detection>
        <operation>
            <operation-type>initialize</operation-type>
            <statement>
                <statement-number type="statement-number-type">1</statement-number>
                <i2c-interface>
                    <bus-id type="i2c-bus-id-type">0</bus-id>
                    <address type="i2c-address-type">0x42</address>                
                    <offset type="i2c-offset-type">0x10</offset>
                    <access type="i2c-access-type">one-byte</access>
                    <mask type="i2c-mask-value-type">0x1</mask>
                </i2c-interface>
                <statement-comment type="statement-comment-type">kuku/<statement-comment>
                <value type="i2c-mask-value-type">1</value>
            </statement>
        </operation>
    </fan-controller-i2c>
</fan-ADT7476-group>
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476XMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Prv pdl fan adt7476 XML parser
 *
 * @param   xmlId             Identifier for the XML.
 * @param   fanControllerId   Identifier for the fanControllerId.
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476XMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  UINT_32                             fanControllerId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                fanOperationType[PDL_FAN_MAX_OP_TYPE_LEN];
    char                                faultDetectionType[PDL_FAN_MAX_FAULT_TYPE_LEN];
    XML_PARSER_NODE_DESCRIPTOR_TYP      xmlFanControllerId, xmlFanControllerChildId, xmlTempChildId;
    XML_PARSER_RET_CODE_TYP             xmlStatus;
    PDL_STATUS                          pdlStatus;
    PDL_FAN_ADT7476_PRV_DB_STC          fanDbStruct;
    PDL_FAN_ADT7476_PRV_DB_STC    *     fanControllerPtr;
    UINT_32                             maxSize;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /*init fan db struct*/
    pdlStatus = prvPdlFanAdtDbStructInit(&fanDbStruct);
    PDL_CHECK_STATUS(pdlStatus);

    fanDbStruct.fanControllerId.fanControllerId = fanControllerId;
    xmlStatus = xmlParserFindByName (xmlId, "fan-ADT7476-specific", &xmlFanControllerId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        return PDL_OK;
    }
    XML_CHECK_STATUS(xmlStatus);
    xmlStatus = xmlParserGetFirstChild (xmlFanControllerId, &xmlFanControllerId);

    
    /* go over all fan-controller-i2c childes */
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        if(xmlParserIsEqualName (xmlFanControllerId, "fault-detection") == XML_PARSER_RET_CODE_OK){
            maxSize = PDL_FAN_MAX_FAULT_TYPE_LEN;
            xmlStatus = xmlParserGetValue (xmlFanControllerId, &maxSize, &faultDetectionType);
            XML_CHECK_STATUS(xmlStatus);

            /*save fault detection in db*/
            pdlStatus =  pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_FAN_ADT7476_FAULT_DETECTION_TYPE_E, &faultDetectionType[0], (UINT_32*)&fanDbStruct.fanInterface.faultRegister);
            PDL_CHECK_STATUS(pdlStatus);
        }
        else if(xmlParserIsEqualName (xmlFanControllerId, "fan-ADT7476-operation-list") == XML_PARSER_RET_CODE_OK){
            xmlStatus = xmlParserGetFirstChild (xmlFanControllerId, &xmlTempChildId);
            XML_CHECK_STATUS(xmlStatus);
            if(xmlParserIsEqualName (xmlTempChildId, "operation-type") == XML_PARSER_RET_CODE_OK){

                /*detect the operation type*/
                maxSize = PDL_FAN_MAX_OP_TYPE_LEN;
                xmlStatus = xmlParserGetValue (xmlTempChildId, &maxSize, &fanOperationType);
                XML_CHECK_STATUS(xmlStatus);

                xmlStatus =xmlParserGetNextSibling (xmlTempChildId, &xmlFanControllerChildId);
                XML_CHECK_STATUS(xmlStatus);

                /*parse the statements*/
                pdlStatus = prvPdlFanAdt7476StatementParse(&fanOperationType[0],xmlFanControllerChildId,&fanDbStruct);
                PDL_CHECK_STATUS(pdlStatus);
            }
            /*add to db*/ 
            pdlStatus = prvPdlDbAdd (&pdlFanAdt7476Db, (void*) &fanDbStruct.fanControllerId, (void*) &fanDbStruct, (void**) &fanControllerPtr); 
            PDL_CHECK_STATUS(pdlStatus);

        }
        else{
            PDL_LIB_DEBUG_MAC(prvPdlFanControllerDebugFlag)(__FUNCTION__, __LINE__,__FUNCTION__, __LINE__,"INVALID xmlFanControllerId TAG NAME");
            return PDL_XML_PARSE_ERROR;
        }

        /*next tag*/
        xmlStatus =xmlParserGetNextSibling (xmlFanControllerId, &xmlFanControllerId);
    }
    return PDL_OK;
}

/*$ END OF prvPdlFanControllerXMLParser */

/**
 * @fn  PDL_STATUS prvPdlFanAdtDbStructInit ( IN PDL_FAN_ADT7476_PRV_DB_STC fanAdtDbStruct )
 *
 * @brief   Pdl fan controller db node init (PDL_FAN_ADT7476_PRV_DB_STC)
 * @param   fanAdtDbStruct*   db struct to init.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdtDbStructInit ( 
    IN  PDL_FAN_ADT7476_PRV_DB_STC *   fanAdtDbStruct     
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_DB_PRV_ATTRIBUTES_STC           dbAttributes;
    PDL_STATUS                          pdlStatus;
    UINT_32                             i;
    PDL_DB_PRV_STC                    * pdlFanAdt7476DbListsPtr[PDL_FAN_ADT7476_NUM_OF_DB_LISTS] = {&(fanAdtDbStruct->initStatementDb),&(fanAdtDbStruct->allNormalSpeedStatementDb),
                                                                &(fanAdtDbStruct->allMaxSpeedStatementDb),&(fanAdtDbStruct->enableFanStatementDb),&(fanAdtDbStruct->disableRedundentFanStatementDb)};
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    /*create PDL_FAN_ADT7476_PRV_DB_STC*/
    dbAttributes.listAttributes.entrySize = sizeof( PDL_FAN_ADT7476_PRV_STATEMENT_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_ADT7476_PRV_STATEMENT_KEY_STC);

    /*init statement lists in db*/
    for(i=0;i<PDL_FAN_OP_TYPE_ADT_7476_LAST_E;i++){
        pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E , &dbAttributes , pdlFanAdt7476DbListsPtr[i]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    return PDL_OK;
}

/*$ END OF prvPdlFanDbStructInit */




/**
 * @fn  PDL_STATUS pdlFanControllerDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl fan controller debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlFanControllerDebugSet ( 
    IN  BOOLEAN             state     
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlFanControllerDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlFanControllerDebugSet */

/* ***************************************************************************
* FUNCTION NAME: pdlFanControllerInit
*
* DESCRIPTION:   Init fan controller module
*                Create fan controller DB and initialize
*
* PARAMETERS:   
*                
*
*****************************************************************************/
/**
 * @fn  PDL_STATUS prvPdlFanAdt7476DbInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan Adt7476 initialize
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476DbInit (
    IN  void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC       dbAttributes;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_ADT7476_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_CONTROLLER_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,  &dbAttributes,  &pdlFanAdt7476Db);
    PDL_CHECK_STATUS(pdlStatus);



    return PDL_OK;   
}

/*$ END OF prvPdlFanAdt7476DbInit */
