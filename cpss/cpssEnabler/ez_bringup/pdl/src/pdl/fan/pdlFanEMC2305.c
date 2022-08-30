/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlFanTc654.c   
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
#include <pdl/fan/private/prvPdlFanEMC2305.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/interface/private/prvPdlI2c.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/fan/pdlFanDebug.h>



/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
PDL_FAN_PRV_CALLBACK_FUNC_STC pdlFanCallbacksEMC2305 = 
{
    &prvPdlFanEMC2305HwStatusGet,
    &prvPdlFanEMC2305HwStateSet,
    &prvPdlFanEMC2305HwDutyCycleGet,
    &prvPdlFanEMC2305HwDutyCycleSet,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &prvPdlFanEMC2305HwInit,
    &prvPdlFanEMC2305DbInit,
    &prvPdlFanEMC2305HwGetRpm,
    NULL
};


/** @brief   The pdl fan adt7476 database */
static PDL_DB_PRV_STC pdlFanEMC2305Db =  {0,NULL};

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

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
 * @fn  PDL_STATUS pdlFanDbNumberOfFansGet ( OUT UINT_32 * numOfFansPtr )
 *
 * @brief   Pdl fan database number of fans get
 *
 * @param [in,out]  numOfFansPtr    If non-null, number of fans pointers.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlFanEMC2305DbNumberOfFansGet (
    OUT  UINT_32                 *  numOfFansPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    return prvPdlDbGetNumOfEntries(&pdlFanEMC2305Db, numOfFansPtr);
}
/*$ END OF pdlFanDbNumberOfFansGet */

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305HwStatusGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT PDL_FAN_STATE_INFO_ENT * statePtr )
 *
 * @brief   Pdl fan hardware state get
 *
 * @param           fanControllerId         Identifier for the fan controller.
 * @param           fanNumber               Identifier for the fan.
 * @param [in,out]  statePtr                If non-null, the state pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305HwStatusGet (
    IN  UINT_32                     fanControllerId,
    IN  UINT_32                     fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT    * statePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                             statusRegData, data=0, maskBit, fanMaskBit;
    PDL_STATUS                          pdlStatus;    
    PDL_FAN_INTERFACE_STC               fanInterface;
    PDL_FAN_ROLE_ENT                    pdl_fan_role;
    PDL_FAN_EMC2305_PRV_DB_STC 	      * fanPtr;
    PDL_FAN_EMC2305_PRV_KEY_STC         fanKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (statePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"statePtr NULL, FAN Number: %d", fanNumber);
        return PDL_BAD_PTR;
    }

    *statePtr = PDL_FAN_STATE_INFO_OK_E;
    
    maskBit = 0x7; /* bits 0-2 */

    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);
    
    /* get Status register, bits 0-2 which indicates if there is any error */
    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, PDL_FAN_EMC2305_STATUS_REG_OFFSET_CNS, maskBit, &statusRegData);
    PDL_CHECK_STATUS(pdlStatus);    

    PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"FAN Number: %d, status reg %#x", fanNumber, statusRegData);

    /* mask the error bit in each specific status register according to the input fan */
    switch (fanNumber){
    case 1:
        fanMaskBit = PDL_FAN_EMC2305_STATUS_REG_FAN_ONE_BIT_MASK_CNS;
        break;

    case 2:
        fanMaskBit = PDL_FAN_EMC2305_STATUS_REG_FAN_TWO_BIT_MASK_CNS;
        break;

    case 3:
        fanMaskBit = PDL_FAN_EMC2305_STATUS_REG_FAN_THREE_BIT_MASK_CNS;
        break;

    case 4:
        fanMaskBit = PDL_FAN_EMC2305_STATUS_REG_FAN_FOUR_BIT_MASK_CNS;
        break;

    case 5:
        fanMaskBit = PDL_FAN_EMC2305_STATUS_REG_FAN_FIVE_BIT_MASK_CNS;
        break;

    default:
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"Illegal fan number %d", fanNumber);
        return PDL_BAD_VALUE;
    }
    
    /* Stall fan status */
    if (statusRegData & PDL_FAN_EMC2305_STATUS_REG_FAN_STALL_BIT_MASK_CNS){        
        pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, PDL_FAN_EMC2305_STATUS_STALL_REG_OFFSET_CNS, fanMaskBit, &data);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"FAN number: %d, STALL case, mask=%#x, data = %#x", fanNumber, fanMaskBit, data);
        if (data)
            *statePtr = PDL_FAN_STATE_INFO_FAULT_E;
    }

    /* Spin fail fan status */
    if (*statePtr == PDL_FAN_STATE_INFO_OK_E &&
        (statusRegData & PDL_FAN_EMC2305_STATUS_REG_FAN_SPIN_BIT_MASK_CNS)) {        
        pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, PDL_FAN_EMC2305_STATUS_SPIN_REG_OFFSET_CNS, fanMaskBit, &data);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"FAN number: %d, SPIN case, mask=%#x, data = %#x", fanNumber, fanMaskBit, data);
        if (data)
            *statePtr = PDL_FAN_STATE_INFO_FAULT_E;
    }

    /* Fan drive fail status */
    if (*statePtr == PDL_FAN_STATE_INFO_OK_E &&
        (statusRegData & PDL_FAN_EMC2305_STATUS_REG_FAN_DRIVE_BIT_MASK_CNS)) {            
            pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, PDL_FAN_EMC2305_STATUS_DRIVE_FAIL_REG_OFFSET_CNS, fanMaskBit, &data);
            PDL_CHECK_STATUS(pdlStatus);
            PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"FAN number: %d, DRIVE case, mask=%#x, data = %#x", fanNumber, fanMaskBit, data);
            if (data)
                *statePtr = PDL_FAN_STATE_INFO_FAULT_E;
    }

    
    if (*statePtr != PDL_FAN_STATE_INFO_FAULT_E) {        
        pdlStatus = pdlFanDbRoleGet(fanControllerId, fanNumber, &pdl_fan_role);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"FAN number: %d, statePtr=%d, Role=%s", fanNumber, *statePtr, 
            (pdl_fan_role==PDL_FAN_ROLE_NORMAL_E)?"Normal":"Redundant");
            
        if (pdl_fan_role == PDL_FAN_ROLE_REDUNDANT_E){
            fanKey.controllerId = fanControllerId;
            pdlStatus = prvPdlDbFind (&pdlFanEMC2305Db, (void*) &fanKey, (void*) &fanPtr);
            PDL_CHECK_STATUS(pdlStatus);
            PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"FAN number: %d, config_state=%d (0 normal, 1 shut)", fanNumber, fanPtr->fanData.config_state[fanNumber]);
            if (fanPtr->fanData.config_state[fanNumber] == PDL_FAN_STATE_CONTROL_SHUTDOWN_E){
                *statePtr = PDL_FAN_STATE_INFO_READY_E;
            }
        }
    }
    return PDL_OK;
}

/*$ END OF pdlFanEMC2305HwStateGet */

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305HwStateSet ( IN UINT_32 fanControllerId, IN PDL_FAN_STATE_CONTROL_ENT state )
 *
 * @brief   Pdl fan hardware state set
 *
 * @param   fanControllerId         Identifier for the fan controller.
 * @param   state                   The state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305HwStateSet (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      fanNumber,
    IN PDL_FAN_STATE_CONTROL_ENT    state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                          pdlStatus;
    PDL_FAN_EMC2305_PRV_DB_STC 	      * fanPtr;
    PDL_FAN_EMC2305_PRV_KEY_STC         fanKey;
    UINT_32                             maskBit;
    UINT_32                             offset;
    PDL_FAN_INTERFACE_STC               fanInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    
    offset = PDL_FAN_EMC2305_DUTY_CYCLE_REGISTER_OFFSET_MAC(fanNumber);
    maskBit = 0xff;
    
    fanKey.controllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanEMC2305Db, (void*) &fanKey, (void*) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus); 
        
    if (state == PDL_FAN_STATE_CONTROL_NORMAL_E){
        pdlStatus = pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, offset, maskBit, fanPtr->fanData.dutyCycle_last_config);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        pdlStatus = pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, offset, maskBit, PDL_FAN_EMC2305_DUTY_CYCLE_OFF_VAL_CNS);
        PDL_CHECK_STATUS(pdlStatus);
    }

    fanPtr->fanData.config_state[fanNumber] = state;

    PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"FAN ID: %d, state %s", fanNumber, (state == PDL_FAN_STATE_CONTROL_NORMAL_E)?"Normal":"Shutdown");
              
    return PDL_OK;
}

/*$ END OF prvPdlFanEMC2305HwStateSet */


PDL_STATUS prvPdlFanEMC2305HwDutyCycleGet ( 
    IN UINT_32      fanControllerId,
    IN UINT_32      fanNumber,
    OUT UINT_32     *speed_val_PTR,
    OUT UINT_32     *speedPercentPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_FAN_EMC2305_PRV_DB_STC 	      * fanPtr;
    PDL_FAN_EMC2305_PRV_KEY_STC         fanKey;
    PDL_STATUS                          pdlStatus;
    float                               f_speed;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    fanKey.controllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanEMC2305Db, (void*) &fanKey, (void*) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);
        
    if (fanPtr->fanData.config_state[fanNumber] == PDL_FAN_STATE_CONTROL_SHUTDOWN_E){
        *speed_val_PTR = 0; 
        *speedPercentPtr = 0;
    }
    else {
        *speedPercentPtr = fanPtr->fanData.dutyCycle_last_config * 100 / PDL_FAN_EMC2305_DUTY_CYCLE_100_PERCENT_VAL_CNS; /* 255 (0xff) is maximum duty cycle */
        f_speed = (float)(*speedPercentPtr * PDL_FAN_EMC2305_DUTY_CYCLE_MAX_RPM_VAL_CNS) / 100;
        *speed_val_PTR = (UINT_32)(f_speed + 0.5);
    }

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305HwDutyCycleSet ( IN UINT_32 fanControllerId, IN UINT_32 speedRelativeVal )
 *
 * @brief   Pdl fan hardware duty cycle set
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [in]  speedRelativeVal        The speed relative value.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305HwDutyCycleSet ( 
    IN UINT_32      fanControllerId,
    IN UINT_32      speedAbsoulteVal
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    UINT_32                             maskBit;
    UINT_32                             offset;
    PDL_STATUS                          pdlStatus;
    PDL_FAN_EMC2305_PRV_DB_STC 	      * fanPtr;
    PDL_FAN_EMC2305_PRV_KEY_STC         fanKey;
        
    UINT_32 pdl_fan_id, pdl_fan_num, pdl_fan_controller;
    PDL_FAN_INTERFACE_STC               fanInterface;
    BOOLEAN								found = FALSE;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/     

    fanKey.controllerId = fanControllerId;
    maskBit = 0xff;
    pdlStatus = pdlFanDbGetFirst (&pdl_fan_id);
    while (pdlStatus == PDL_OK) {        
        pdlStatus = pdlFanIdToFanControllerAndNumber (pdl_fan_id, &pdl_fan_controller, &pdl_fan_num);
        PDL_CHECK_STATUS(pdlStatus);

        if (pdl_fan_controller == fanControllerId){
            
            pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
            PDL_CHECK_STATUS(pdlStatus);    
            
            pdlStatus = prvPdlDbFind (&pdlFanEMC2305Db, (void*) &fanKey, (void*) &fanPtr);
            PDL_CHECK_STATUS(pdlStatus);

            /* in case the fan in shutdown - continue (shutdown is 0 duty-cycle) */
            if (fanPtr->fanData.config_state[pdl_fan_num] == PDL_FAN_STATE_CONTROL_SHUTDOWN_E){
                pdlStatus = pdlFanDbGetNext (pdl_fan_id, &pdl_fan_id);
                continue;
            }
            found = TRUE;
            offset = PDL_FAN_EMC2305_DUTY_CYCLE_REGISTER_OFFSET_MAC(pdl_fan_num);

            pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, offset, maskBit, speedAbsoulteVal);
            PDL_CHECK_STATUS(pdlStatus);
        }
        pdlStatus = pdlFanDbGetNext (pdl_fan_id, &pdl_fan_id);        
    }
    if (found)
    	fanPtr->fanData.dutyCycle_last_config = speedAbsoulteVal;
    return PDL_OK;
}

/*$ END OF prvPdlFanEMC2305HwDutyCycleSet */

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305HwInit ( IN UINT_32 fanControllerId )
 *
 * @brief   Pdl fan hardware init
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305HwInit ( 
    IN UINT_32                  fanControllerId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                             maskBit=0x1f, init_vlaue=0x1f;    
    PDL_STATUS                          pdlStatus;
    /*PDL_FAN_EMC2305_PRV_DB_STC 	      * fanPtr;
    PDL_FAN_EMC2305_PRV_KEY_STC         fanKey;*/

    UINT_32 pdl_fan_id, pdl_fan_num, pdl_fan_controller;
    PDL_FAN_STATE_INFO_ENT              state;
    PDL_FAN_INTERFACE_STC               fanInterface;
    PDL_FAN_EMC2305_PRV_DB_STC          fanDbStruct;
    PDL_FAN_EMC2305_PRV_KEY_STC       * fanControllerPtr = NULL;  
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    
    /***fanKey.controllerId = fanControllerId;
    pdlStatus = prvPdlDbFind (&pdlFanEMC2305Db, (void*) &fanKey, (void*) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus); */
    fanDbStruct.fanKey.controllerId = fanControllerId;
    memset (&fanDbStruct, 0, sizeof(fanDbStruct));
    /*add to db*/ 
    pdlStatus = prvPdlDbAdd (&pdlFanEMC2305Db, (void*) &fanDbStruct.fanKey, (void*) &fanDbStruct, (void**) &fanControllerPtr); 
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus); 
    
    /* set 0x1F to bits 0-4 in interrupt register (enable interrupts) */
    pdlStatus = pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, PDL_FAN_EMC2305_INTERRUPT_REG_OFFSET_CNS, maskBit, init_vlaue);
    PDL_CHECK_STATUS(pdlStatus);
    
    /* read the status register one time in order to clear it */
    pdlStatus = pdlFanDbGetFirst (&pdl_fan_id);
    PDL_CHECK_STATUS(pdlStatus);

    while (pdlStatus == PDL_OK) {        
        pdlStatus = pdlFanIdToFanControllerAndNumber (pdl_fan_id, &pdl_fan_controller, &pdl_fan_num);
        PDL_CHECK_STATUS(pdlStatus);

        if (pdl_fan_controller == fanControllerId)
            break;

        pdlStatus = pdlFanDbGetNext (pdl_fan_id, &pdl_fan_id);
        PDL_CHECK_STATUS(pdlStatus);
    }               
        
    pdlStatus = prvPdlFanEMC2305HwStatusGet(fanControllerId, pdl_fan_num, &state);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlFanEMC2305HwInit */

/**
 * @fn  PDL_STATUS pdlFanDebugInterfaceGet ( IN UINT_32 fanId, OUT PDL_FAN_INTERFACE_STC * interfacePtr )
 *
 * @brief   Pdl fan debug interface get
 *
 * @param           fanId           Identifier for the fan.
 * @param [in,out]  interfacePtr    If non-null, the interface pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlFanEMC2305DebugInterfaceGet (
    IN  UINT_32                         fanId,
    OUT PDL_FAN_ENC2305_INTERFACE_STC   * interfacePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_EMC2305_PRV_DB_STC 	      * fanPtr;
    PDL_FAN_EMC2305_PRV_KEY_STC           fanKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"interfacePtr NULL, FAN ID: %d",fanId);
        return PDL_BAD_PTR;
    }
    fanKey.controllerId = fanId;
    pdlStatus = prvPdlDbFind (&pdlFanEMC2305Db, (void*) &fanKey, (void**) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy (interfacePtr, &fanPtr->fanInterface, sizeof (fanPtr->fanInterface));
    return PDL_OK;

}

/*$ END OF pdlFanDebugInterfaceGet */



/* ***************************************************************************
* FUNCTION NAME: prvPdlFanEMC2305XMLParser
*
* DESCRIPTION:   Fill all fan related information from XML to DB
*
* PARAMETERS:    xmlId
*                
* XML structure:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305XMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan XML parser
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305XMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  UINT_32                             fanControllerId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_INTERFACE_TYP               interfaceId;
    PDL_INTERFACE_TYPE_ENT          interfaceType;
    XML_PARSER_NODE_DESCRIPTOR_TYP  xmlFanControllerId, xmlFanControllerChildId;
    PDL_STATUS                      pdlStatus;
    XML_PARSER_RET_CODE_TYP         xmlStatus;
    PDL_FAN_EMC2305_PRV_DB_STC        fanDbStruct;
    PDL_FAN_EMC2305_PRV_KEY_STC      * fanControllerPtr = NULL;  
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    fanDbStruct.fanKey.controllerId = fanControllerId;
    memset (&fanDbStruct, 0, sizeof(fanDbStruct));
    /*add to db*/ 
    pdlStatus = prvPdlDbAdd (&pdlFanEMC2305Db, (void*) &fanDbStruct.fanKey, (void*) &fanDbStruct, (void**) &fanControllerPtr); 
    PDL_CHECK_STATUS(pdlStatus);

    xmlStatus = xmlParserFindByName (xmlId, "fan-EMC2305-group", &xmlFanControllerId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_NOT_FOUND) {
        return PDL_OK;
    }
    XML_CHECK_STATUS(xmlStatus);
    xmlStatus = xmlParserGetFirstChild (xmlFanControllerId, &xmlFanControllerId);

    /*i2c-interface tag*/
    if(xmlParserIsEqualName (xmlFanControllerId, "fan-controller-i2c") == XML_PARSER_RET_CODE_OK){
        /*inside tag i2c-interface*/
        xmlStatus = xmlParserGetFirstChild (xmlFanControllerId, &xmlFanControllerChildId);
        xmlStatus = prvPdlInterfaceXmlParser (xmlFanControllerChildId, &interfaceType, &interfaceId);
        XML_CHECK_STATUS(xmlStatus);
        fanDbStruct.fanInterface.interfaceId = interfaceId;
        fanDbStruct.fanInterface.interfaceType = interfaceType;
        /*add to db*/ 
        pdlStatus = prvPdlDbAdd (&pdlFanEMC2305Db, (void*) &fanDbStruct.fanKey, (void*) &fanDbStruct, (void**) &fanControllerPtr); 
        PDL_CHECK_STATUS(pdlStatus);
    }
    else{
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"CAN'T FIND fan-controller-i2c EMC2305 INFO, FAN CONTROLLER ID: %d",fanControllerId);
    }

    return PDL_OK;
}

/*$ END OF prvPdlFanEMC2305XMLParser */



/**
 * @fn  PDL_STATUS prvPdlFanEMC2305DbInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan initialize
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305DbInit (
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
    dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_EMC2305_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_EMC2305_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlFanEMC2305Db);
    PDL_CHECK_STATUS(pdlStatus);

    pdlFanDebugSet(TRUE);

    return PDL_OK;
}

/*$ END OF prvPdlFanEMC2305DbInit */

/**
 * @fn  PDL_STATUS prvPdlFanEMC2305HwGetRpm ()
 *
 * @brief   get rpm for EMC 2305
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [out] rpmPtr              - rpm for current fan
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanEMC2305HwGetRpm (
    IN  UINT_32                           fanControllerId,
    OUT UINT_32                         * rpmPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(fanControllerId);

    if (rpmPtr == NULL) {
        return PDL_BAD_PTR;
    }
    *rpmPtr = 1000;
    return PDL_OK;
}

/*$ END OF prvPdlFanEMC2305HwGetRpm */
