/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\fan\private\prvpdlfanAdt7476.h.
 *
 * @brief   Declares the prvpdlfan class
 */

#ifndef __prvPdlFanAdt7476h
#define __prvPdlFanAdt7476h
/**
********************************************************************************
 * @file prvPdlFanAdt7476.h   
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
 * @brief Platform driver layer - Fan Adt7476 private declarations and APIs
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/fan/pdlFan.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/lib/private/prvPdlLib.h>

/**
* @addtogroup FanAdt7476
* @{
*/

/**
* @addtogroup prvFanAdt7476
* @{
*/

typedef enum {
    PDL_FAN_FAULT_ADT_7476_INTERRUPT_STATUS_REGISTER_E = 0,
    PDL_FAN_FAULT_ADT_7476_TACHI_LOW_BYTE_REGISTER_E,
    PDL_FAN_FAULT_ADT_7476_LAST_E
}PDL_FAN_ADT_7476_FAULT_REGISTER_ENT;

typedef enum {
    PDL_FAN_OP_TYPE_ADT_7476_INITIALIZE_E = 0,
    PDL_FAN_OP_TYPE_ADT_7476_ALL_NORMAL_SPEED_E,
    PDL_FAN_OP_TYPE_ADT_7476_ALL_MAX_SPEED_E,
    PDL_FAN_OP_TYPE_ADT_7476_ENABLE_FAN_SPEED_E,
    PDL_FAN_OP_TYPE_ADT_7476_DISABLE_FAN_SPEED_E,
    PDL_FAN_OP_TYPE_ADT_7476_LAST_E
}PDL_FAN_OP_TYPE_ADT_7476;

#define PDL_FAN_ADT7476_NUM_OF_DB_LISTS 5
#define PDL_FAN_MAX_OP_TYPE_LEN 22
#define PDL_FAN_MAX_FAULT_TYPE_LEN 20
#define PDL_FAN_MAX_STAT_COMMENT_LEN 121

typedef struct {
    PDL_FAN_ADT_7476_FAULT_REGISTER_ENT   faultRegister;
} PDL_FAN_ADT_7476_INTERFACE_STC;


extern PDL_FAN_PRV_CALLBACK_FUNC_STC            pdlFanCallbacksAdt7476;

typedef struct {
    /** @brief   Identifier for the fan */
    UINT_32                        fanId;
} PDL_FAN_ADT7476_PRV_KEY_STC;



typedef struct {    
    PDL_FAN_CONTROLLER_PRV_KEY_STC          fanControllerId;
    PDL_FAN_ADT_7476_INTERFACE_STC          fanInterface; 
    PDL_DB_PRV_STC                          initStatementDb;
    PDL_DB_PRV_STC                          allMaxSpeedStatementDb;
    PDL_DB_PRV_STC                          allNormalSpeedStatementDb;
    PDL_DB_PRV_STC                          enableFanStatementDb;
    PDL_DB_PRV_STC                          disableRedundentFanStatementDb;
} PDL_FAN_ADT7476_PRV_DB_STC;


/**
 * @struct  PDL_FAN_ADT7476_PRV_STATEMENT_KEY_STC
 *
 * @brief   defines structure for fan statement key in hash
 */

typedef struct {
    /** @brief   Identifier for the statement */
    UINT_32                        statmentId;
} PDL_FAN_ADT7476_PRV_STATEMENT_KEY_STC;

/**
 * @struct  PDL_FAN_ADT7476_PRV_STATEMENT_STC
 *
 * @brief   defines fan controller statement info i2c interfaceId + value will be used to set
 *          defined value to controller, comment is a user comment explaining what the value means
 */

typedef struct {
    UINT_32                                 statementId;
    char                                  * commentPtr;
    PDL_INTERFACE_TYP                       i2cInterfaceId;    
    UINT_32                                 value;
    PDL_FAN_ADT7476_PRV_STATEMENT_KEY_STC   statementKey;
} PDL_FAN_ADT7476_PRV_STATEMENT_STC;

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
);


/**
 * @fn  PDL_STATUS prvPdlFanAdt7476SetStat ( IN PDL_DB_PRV_STC * statementsDbPtr, IN PDL_FAN_ADT7476_PRV_DB_STC * fanPtr )
 * @brief   Pdl set statement value to entire statement db
 * @param [IN]     PDL_DB_PRV_STC                *   statementsDbPtr      the fan's statement db pointer.
 * @param [IN]     PDL_FAN_ADT7476_PRV_DB_STC    *   fanPtr            the fan pointer .
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlFanAdt7476SetState (
    IN   PDL_DB_PRV_STC              *  statementsDbPtr,
    IN   PDL_FAN_ADT7476_PRV_DB_STC  *  fanPtr
);

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
);

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
);

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
);

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476XmlStatementInfoGet ( IN UINT_32 fanControllerId, IN UINT_32 statementId, OUT PDL_FAN_ADT_7476_STATEMENT_STC * statementInfoPtr );
 *
 * @brief   get statement's information
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  fanControllerId     - id for the ADT 7476 fan controller
 * @param [in]  statementId         - the statement number.
 * @param [out] statementInfoPtr    - statement's information.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476XmlStatementInfoGet (
    IN  PDL_FAN_CONTROLLER_PRV_KEY_STC         fanControllerId,
    IN  UINT_32                                statementId,
    OUT PDL_FAN_ADT7476_PRV_STATEMENT_STC    * statementInfoPtr
);

PDL_STATUS prvPdlFanAdt7476HwScriptActivate (
    IN UINT_32                       fanControllerId,
    IN PDL_FAN_OP_TYPE_ENT           opType,
    OUT UINT_32                    * faultFanIdPtr
);

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
);


/**
 * @fn  PDL_STATUS prvPdlFanAdt7476DbInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan Adt7476 initialize
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476DbInit (
    IN  void
);

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
);

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
    IN UINT_32                      fanControllerId,
    IN UINT_32                      fanNumber,
    IN PDL_FAN_STATE_CONTROL_ENT    state
);



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
);

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
);

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476GetType ( IN UINT_32 fanControllerId);
 *
 * @brief   Init fan controller
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] controllerTypePtr   - fan controller type (i.e tc654/adt7476)
 *
 * @return  
 */
PDL_STATUS prvPdlFanAdt7476GetType (
    IN  UINT_32                           fanControllerId,
    OUT PDL_FAN_CONTROLLER_TYPE_ENT * controllerTypePtr
);


/* @}*/
/* @}*/
/* @}*/

#endif
