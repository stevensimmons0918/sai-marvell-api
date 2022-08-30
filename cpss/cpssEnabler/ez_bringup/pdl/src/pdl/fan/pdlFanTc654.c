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
#include <pdl/fan/private/prvPdlFanTc654.h>
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
PDL_FAN_PRV_CALLBACK_FUNC_STC pdlFanCallbacksTc654 = 
{
    &prvPdlFanTc654HwStatusGet,
    &prvPdlFanTc654HwStateSet,
    NULL/*fanHwDutyCycleGetFun*/,
    &prvPdlFanTc654HwDutyCycleSet,
    NULL/*&pdlFanTc654HwRotationDirectionGet*/,
    NULL/*&pdlFanTc654HwRotationDirectionSet*/,
    &prvPdlFanTc654HwDutyCycleMethodSet,
    &prvPdlFanTc654HwFaultSet, 
    &prvPdlFanTc654HwPulsesPerRotationSet,
    &prvPdlFanTc654HwThresholdSet,
    &prvPdlFanTc654HwInit,
    &prvPdlFanTc654DbInit,
    NULL,
    &prvPdlFanTc654XMLParser
};


/** @brief   The pdl fan adt7476 database */
static PDL_DB_PRV_STC pdlFanTc654Db =  {0,NULL};

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

PDL_STATUS pdlFanDbNumberOfFansGet (
    OUT  UINT_32                 *  numOfFansPtr
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    return prvPdlDbGetNumOfEntries(&pdlFanTc654Db, numOfFansPtr);
}
/*$ END OF pdlFanDbNumberOfFansGet */

/**
 * @fn  PDL_STATUS prvPdlFanTc654HwStatusGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT PDL_FAN_STATE_INFO_ENT * statePtr )
 *
 * @brief   Pdl fan hardware state get
 *
 * @param           fanControllerId         Identifier for the fan controller.
 * @param           fanNumber               Identifier for the fan.
 * @param [in,out]  statePtr                If non-null, the state pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwStatusGet (
    IN  UINT_32                     fanControllerId,
    IN  UINT_32                     fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT    * statePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                             data, offset, maskBit, relData;
    PDL_STATUS                          pdlStatus;
    PDL_FAN_INTERFACE_STC               fanInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (statePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"statePtr NULL, FAN number: %d",fanNumber);
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);    
    offset = PDL_FAN_TC654_STATUS_REGISTER_OFFSET_CNS;
    if (fanNumber == 1) {
        maskBit = 0x9; 
    }
    else if (fanNumber == 2) {
        maskBit = 0x12;
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"illegal fan number %d MUST be 1/2",fanNumber);
        return PDL_BAD_PARAM;
    }
    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId , offset  , maskBit, &data);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlI2CConvertAbs2RelWithMask (data, maskBit, &relData);
    PDL_CHECK_STATUS(pdlStatus);
    switch (relData) {
    case 0:
        *statePtr = PDL_FAN_STATE_INFO_OK_E;
        break;
    case 1:
        *statePtr = PDL_FAN_STATE_INFO_FAULT_E;
        break;
    case 2:
        *statePtr = PDL_FAN_STATE_INFO_OVERFLOW_E;
        break;
    case 3:
        *statePtr = PDL_FAN_STATE_INFO_FAULT_E;
        break;
    default:
        *statePtr = PDL_FAN_STATE_INFO_FAULT_E;
        break;
    }
    return PDL_OK;
}

/*$ END OF pdlFanTc654HwStateGet */

/**
 * @fn  PDL_STATUS prvPdlFanTc654HwStateSet ( IN UINT_32 fanControllerId, IN PDL_FAN_STATE_CONTROL_ENT state )
 *
 * @brief   Pdl fan hardware state set
 *
 * @param   fanControllerId         Identifier for the fan controller.
 * @param   state                   The state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwStateSet (
    IN UINT_32                      fanControllerId,
    IN UINT_32                      fanNumber,
    IN PDL_FAN_STATE_CONTROL_ENT    state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                          pdlStatus;
    UINT_32                             maskBit;
    UINT_32                             offset;
    PDL_FAN_INTERFACE_STC               fanInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    PDL_UNUSED_PARAM(fanNumber);

    offset = PDL_FAN_TC654_CONFIG_REGISTER_OFFSET_CNS;
    maskBit = 0x1; 
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);      
    
    if (state == PDL_FAN_STATE_CONTROL_NORMAL_E) {
        pdlStatus = pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId , offset  , maskBit , 0);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else if (state == PDL_FAN_STATE_CONTROL_SHUTDOWN_E) {
        pdlStatus = pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId , offset  , maskBit , 1);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL state PARAM");
        return PDL_BAD_PARAM;
    }    
    PDL_CHECK_STATUS(pdlStatus);           
    return PDL_OK;
}

/*$ END OF prvPdlFanTc654HwStateSet */


/**
 * @fn  PDL_STATUS prvPdlFanTc654HwDutyCycleGet ( IN UINT_32 fanControllerId, OUT UINT_32 * speedPtr )
 *
 * @brief   Pdl fan hardware duty cycle get
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [out] speedPtr                If non-null, the speed pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwDutyCycleGet (
    IN  UINT_32                     fanControllerId,
    IN UINT_32                      fanNumber,
    OUT UINT_32                   * speedPtr,
    OUT UINT_32                   * speedPercentPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                             maskBit;
    UINT_32                             offset;
    PDL_STATUS                          pdlStatus;
    PDL_FAN_INTERFACE_STC               fanInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(fanNumber);

    offset = PDL_FAN_TC654_DUTY_CYCLE_REGISTER_OFFSET_CNS;
    maskBit = 0xf;
    if (speedPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"speedPtr NULL");
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);    

    pdlStatus =  pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId,offset,maskBit, speedPtr);
    PDL_CHECK_STATUS(pdlStatus); 
    *speedPercentPtr = *speedPtr * 100 /15; /* 15 (0xf) is maximum duty cycle */

    return PDL_OK;
}

/*$ END OF prvPdlFanTc654HwDutyCycleGet */

/**
 * @fn  PDL_STATUS pdlFanTc645HwDutyCycleSet ( IN UINT_32 fanControllerId, IN UINT_32 speedRelativeVal )
 *
 * @brief   Pdl fan hardware duty cycle set
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [in]  speedRelativeVal        The speed relative value.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwDutyCycleSet ( 
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
    PDL_FAN_INTERFACE_STC               fanInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    offset = PDL_FAN_TC654_DUTY_CYCLE_REGISTER_OFFSET_CNS;
    maskBit = 0xf;
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);    

    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId,offset,maskBit, speedAbsoulteVal);
    PDL_CHECK_STATUS(pdlStatus);            
    return PDL_OK;
}

/*$ END OF prvPdlFanTc654HwDutyCycleSet */


/**
 * @fn  PDL_STATUS prvPdlFanTc654HwThresholdSet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, IN UINT_32 thresholdRelativeVal )
 *
 * @brief   Pdl fan hardware threshold set
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [in]  fanNumber               Fan number
 * @param [in]  thresholdRelativeVal    The threshold relative value.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwThresholdSet ( 
    IN UINT_32                  fanControllerId,
    IN UINT_32                  fanNumber,
    IN UINT_32                  thresholdAbsoulteVal
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    UINT_32                             maskBit;
    UINT_32                             offset;
    PDL_STATUS                          pdlStatus;
    PDL_FAN_INTERFACE_STC               fanInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    maskBit = 0xff;
    switch(fanNumber){
    case 1:
        offset = PDL_FAN_TC654_FAULT_FAN1_REGISTER_OFFSET_CNS;
        break;
    case 2:
        offset = PDL_FAN_TC654_FAULT_FAN2_REGISTER_OFFSET_CNS;
        break;
    default:
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL FAN ID, FAN ID: %d",fanNumber);
        return PDL_BAD_VALUE;
    }
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);    

    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId,offset,maskBit, thresholdAbsoulteVal);
    PDL_CHECK_STATUS(pdlStatus);            
    return PDL_OK;
}

/*$ END OF prvPdlFanTc654HwThresholdSet */

/**
 * @fn  PDL_STATUS PdlFanTc654HwDutyCycleMethodSet ( IN UINT_32 fanControllerId, IN PDL_FAN_DUTY_CYCLE_METHOD_ENT duthCycleMethod )
 *
 * @brief   Pdl fan hardware duty cycle method set
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [in]  duthCycleMethod         The duty cycle method.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlFanTc654HwDutyCycleMethodSet (
    IN UINT_32                       fanControllerId,
    IN PDL_FAN_DUTY_CYCLE_METHOD_ENT dutyCycleMethod
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                          pdlStatus;
    UINT_32                             maskBit;
    UINT_32                             offset;
    PDL_FAN_INTERFACE_STC               fanInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    offset = PDL_FAN_TC654_CONFIG_REGISTER_OFFSET_CNS;
    maskBit = 0x20;
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);    
    if (dutyCycleMethod == PDL_FAN_DUTY_CYCLE_METHOD_HW_E) {
        pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId,offset,maskBit, 0);
    }
    else if (dutyCycleMethod == PDL_FAN_DUTY_CYCLE_METHOD_SW_E) {
        pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId,offset,maskBit, 0x20);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL duthCycleMethod");
        return PDL_BAD_PARAM;
    }   
    PDL_CHECK_STATUS(pdlStatus);            
    return PDL_OK;
}

/*$ END OF PdlFanTc654HwDutyCycleMethodSet */

/**
 * @fn  PDL_STATUS PdlFanTc654HwPulsesPerRotationSet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, IN UINT_32 pulsesPerRotationRelativeVal )
 *
 * @brief   Pdl fan hardware pulses per rotation set
 *
 * @param [in]  fanControllerId                 Identifier for the fan controller.
 * @param [in]  fanNumber                       fan number.
 * @param [in]  pulsesPerRotationRelativeVal    The pulses per rotation relative value.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwPulsesPerRotationSet ( /* relative set  - 0, 1, 2, 3 - decimal  - means set 2 bits */
    IN UINT_32      fanControllerId,
    IN UINT_32      fanNumber,
    IN UINT_32      pulsesPerRotationRelativeVal
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    UINT_32                             pulsesPerRotationAbsoulteVal;
    PDL_STATUS                          pdlStatus;
    UINT_32                             maskBit;
    UINT_32                             offset;
    PDL_FAN_INTERFACE_STC               fanInterface;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);    
    offset = PDL_FAN_TC654_CONFIG_REGISTER_OFFSET_CNS;
    switch(fanNumber){
    case 1:
        maskBit = 6;
        break;
    case 2:
        maskBit = 0x18;
        break;
    default:
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL FAN ID, FAN ID: %d",fanNumber);
        return PDL_BAD_VALUE;
    }
    pdlStatus = prvPdlI2CConvertRel2AbsWithMask(fanInterface.interfaceId, pulsesPerRotationRelativeVal, maskBit,&pulsesPerRotationAbsoulteVal);
    PDL_CHECK_STATUS(pdlStatus); 
    pdlStatus = pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId , offset  , maskBit , pulsesPerRotationAbsoulteVal);
    PDL_CHECK_STATUS(pdlStatus);           
    return PDL_OK;
}

/*$ END OF PdlFanTc654HwPulsesPerRotationSet */

/**
 * @fn  PDL_STATUS PdlFanTc654HwFaultSet ( IN UINT_32 fanControllerId,IN fanId ,IN PDL_FAN_FAULT_CONTROL_ENT faultControl )
 *
 * @brief   Pdl fan hardware fault set
 *
 * @param [in]  fanControllerId                 Identifier for the fan controller.
 * @param   faultControl                        The fault control.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwFaultSet (
    IN UINT_32                       fanControllerId,
    IN PDL_FAN_FAULT_ENT             faultControl
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
    PDL_STATUS                          pdlStatus;
    UINT_32                             offset;
    UINT_32                             maskBit;
    PDL_FAN_INTERFACE_STC               fanInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);  
    offset = PDL_FAN_TC654_CONFIG_REGISTER_OFFSET_CNS;
    maskBit = 0x80;
    if (faultControl == PDL_FAN_FAULT_NORMAL_E) {
        pdlStatus = pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId , offset  , maskBit , 0);
    }
    else if (faultControl == PDL_FAN_FAULT_CLEAR_E) {
        pdlStatus = pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId , offset  , maskBit , 0x80);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"ILLEGAL faultControl");
        return PDL_BAD_PARAM;
    }
    PDL_CHECK_STATUS(pdlStatus);           
    return PDL_OK;
}

/*$ END OF PdlFanTc654HwFaultSet */

/**
 * @fn  PDL_STATUS prvPdlFanTc654HwInit ( IN UINT_32 fanControllerId )
 *
 * @brief   Pdl fan hardware init
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654HwInit ( 
    IN UINT_32                  fanControllerId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/    
    PDL_UNUSED_PARAM(fanControllerId);
	
    return PDL_OK;
}

/*$ END OF prvPdlFanTc654HwInit */

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

PDL_STATUS pdlFanDebugInterfaceGet (
    IN  UINT_32                         fanId,
    OUT PDL_FAN_INTERFACE_STC         * interfacePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_INTERFACE_STC               fanInterface;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,"interfacePtr NULL, FAN ID: %d",fanId);
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlFanGetInterface(fanId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);  
    memcpy (interfacePtr, &fanInterface, sizeof (fanInterface));
    return PDL_OK;

}

/*$ END OF pdlFanDebugInterfaceGet */



/* ***************************************************************************
* FUNCTION NAME: prvPdlFanTc654XMLParser
*
* DESCRIPTION:   Fill all fan related information from XML to DB
*
* PARAMETERS:    xmlId
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlFanTc654XMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan XML parser
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654XMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    IN  UINT_32                             fanControllerId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(xmlId);
    PDL_UNUSED_PARAM(fanControllerId);
    /* no specific data for TC654 need to be parsed */
    return PDL_OK;
}

/*$ END OF prvPdlFanTc654XMLParser */



/**
 * @fn  PDL_STATUS prvPdlFanTc654DbInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan initialize
 *
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanTc654DbInit (
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
    dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_TC654_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_TC654_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlFanTc654Db);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlFanTc654DbInit */
