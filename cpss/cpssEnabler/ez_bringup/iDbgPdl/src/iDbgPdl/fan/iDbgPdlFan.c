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
#include <string.h>
#include <pdl/fan/pdlFan.h>
#include <pdl/fan/pdlFanDebug.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/interface/pdlI2CDebug.h>
#include <iDbgPdl/fan/iDbgPdlFan.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>
#include <iDbgPdl/interface/iDbgPdlInterface.h>


#define PDL_FAN_NUMBER_OF_CAPABILITIES sizeof(PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC) / sizeof(BOOLEAN)

/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlFanGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      fanControllerId,
    IN UINT_32                            fanNumber
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstLogicalFanId, lastFanLogicalId, tempFirstLogicalFanId, tempFanControllerId, tempFanNumber;
	PDL_STATUS						pdlStatus;
    PDL_FAN_ROLE_ENT                fanRole;
   
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (fanControllerId == IDBG_PDL_ID_ALL) {
        pdlStatus = pdlFanDbGetFirst (&firstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        lastFanLogicalId = firstLogicalFanId;
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlFanDbGetNext (lastFanLogicalId, &lastFanLogicalId);            
        }
        lastFanLogicalId++;
    }
    else {if (fanNumber == IDBG_PDL_ID_ALL) {
        pdlStatus = pdlFanDbGetFirstController (&tempFanControllerId);
        PDL_CHECK_STATUS(pdlStatus);
        while (pdlStatus == PDL_OK) {
            if (tempFanControllerId == fanControllerId) {
                break;
            }
            pdlStatus = pdlFanDbGetNextController (tempFanControllerId, &tempFanControllerId);   
            PDL_CHECK_STATUS(pdlStatus);
        }
        pdlStatus = pdlFanDbGetFirst (&tempFirstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlFanIdToFanControllerAndNumber (tempFirstLogicalFanId, &tempFanControllerId, &tempFanNumber);
            PDL_CHECK_STATUS(pdlStatus);
            if (tempFanControllerId == fanControllerId) break;
            pdlStatus = pdlFanDbGetNext (tempFirstLogicalFanId, &tempFirstLogicalFanId);            
        }
        firstLogicalFanId = tempFirstLogicalFanId;
        lastFanLogicalId = firstLogicalFanId;
        tempFanControllerId = fanControllerId;
        pdlStatus = PDL_OK;
        while  (pdlStatus == PDL_OK)  {
            pdlStatus = pdlFanDbGetNext (lastFanLogicalId, &lastFanLogicalId); 
            if (pdlStatus  == PDL_OK) {
                pdlStatus = pdlFanIdToFanControllerAndNumber (lastFanLogicalId, &tempFanControllerId, &tempFanNumber);
                PDL_CHECK_STATUS(pdlStatus);
                if (tempFanControllerId != fanControllerId){
                    lastFanLogicalId--;
                    break;
                }
            }
        }   
        lastFanLogicalId++; 
    }
    else {
        pdlStatus = convertFanControllerAndNumberToFanId (fanControllerId, fanNumber, &firstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        lastFanLogicalId = firstLogicalFanId + 1;
    }}
    
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-10s | %-10s | %-10s |\n", " Ctrl ID " , "Fan Number", "  Fan ID  ", "   Role   ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s | %-10s | %-10s | %-10s |\n", "----------", "----------", "----------", "----------");
  
    for (i = firstLogicalFanId; i <lastFanLogicalId; i++) {
        pdlStatus = pdlFanIdToFanControllerAndNumber (i, &tempFanControllerId, &tempFanNumber);
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanDbRoleGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanNumber), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fanRole));
        PDL_CHECK_STATUS(pdlStatus);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("%-10d | %-10d | %-10d | %-10s |\n",
                                        tempFanControllerId,
                                        tempFanNumber,
                                        i,
                                        fanRole == PDL_FAN_ROLE_NORMAL_E ? "Normal" : "Backup"
                                       );
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}
/*$ END OF iDbgPdlFanGetInfo */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanRunControllerValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlFanRunControllerValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      fanControllerId,
    IN UINT_32                            fanNumber
    )
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                         i, firstLogicalFanId, lastFanLogicalId, tempFirstLogicalFanId, tempFanControllerId, tempFanNumber, phyFan;
    UINT_32                         speed, speedPrecent, rpm;
    char                            *typePtr, *fanInfoPtr, *initPtr, *rotationPtr, *dutyCyclePtr, *dutyCycleMethodPtr = NULL, *rpmPtr;
    char                            *faultPtr, *statePtr, *stateConPtr, *rolePtr, *pulsePtr, *ThresholdPtr; 
    PDL_STATUS						pdlStatus;
    PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC    controllerCapabilities;
    PDL_FAN_CONTROLLER_TYPE_ENT     fanControllerType;
    PDL_FAN_DIRECTION_ENT           direction;
    PDL_FAN_STATE_INFO_ENT          state;
    PDL_FAN_STATE_CONTROL_ENT       tempState;
    PDL_FAN_ROLE_ENT                role;

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (fanControllerId == IDBG_PDL_ID_ALL) {
        pdlStatus = pdlFanDbGetFirst (&firstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        lastFanLogicalId = firstLogicalFanId;
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlFanDbGetNext (lastFanLogicalId, &lastFanLogicalId);            
        }
        lastFanLogicalId++;
    }
    else {if (fanNumber == IDBG_PDL_ID_ALL) {
        pdlStatus = pdlFanDbGetFirstController (&tempFanControllerId);
        PDL_CHECK_STATUS(pdlStatus);
        while (pdlStatus == PDL_OK) {
            if (tempFanControllerId == fanControllerId) {
                break;
            }
            pdlStatus = pdlFanDbGetNextController (tempFanControllerId, &tempFanControllerId);   
            PDL_CHECK_STATUS(pdlStatus);
        }
        pdlStatus = pdlFanDbGetFirst (&tempFirstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlFanIdToFanControllerAndNumber (tempFirstLogicalFanId, &tempFanControllerId, &tempFanNumber);
            PDL_CHECK_STATUS(pdlStatus);
            if (tempFanControllerId == fanControllerId) break;
            pdlStatus = pdlFanDbGetNext (tempFirstLogicalFanId, &tempFirstLogicalFanId);            
        }
        firstLogicalFanId = tempFirstLogicalFanId;
        lastFanLogicalId = firstLogicalFanId;
        tempFanControllerId = fanControllerId;
        pdlStatus = PDL_OK;
        while  (pdlStatus == PDL_OK)  {
            pdlStatus = pdlFanDbGetNext (lastFanLogicalId, &lastFanLogicalId); 
            if (pdlStatus  == PDL_OK) {
                pdlStatus = pdlFanIdToFanControllerAndNumber (lastFanLogicalId, &tempFanControllerId, &tempFanNumber);
                PDL_CHECK_STATUS(pdlStatus);
                if (tempFanControllerId != fanControllerId){
                    lastFanLogicalId--;
                    break;
                }
            }
        }   
        lastFanLogicalId++; 
    }
    else {
        
        pdlStatus = convertFanControllerAndNumberToFanId (fanControllerId, fanNumber, &firstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        lastFanLogicalId = firstLogicalFanId + 1;
    }}
 
    for (i = firstLogicalFanId; i <lastFanLogicalId; i++) {
        phyFan = i;
        pdlStatus = pdlFanIdToFanControllerAndNumber (i, &tempFanControllerId, &tempFanNumber);
        PDL_CHECK_STATUS(pdlStatus); 
        pdlStatus = convertFanControllerAndNumberToFanId (tempFanControllerId, tempFanNumber, &phyFan);
        PDL_CHECK_STATUS(pdlStatus);
        
        if (phyFan == i) 
            fanInfoPtr = "Pass";
        else 
            fanInfoPtr = "Fail";
 
        pdlStatus = pdlFanControllerGetType (tempFanControllerId, &fanControllerType);
        if (pdlStatus == PDL_OK)
            typePtr = "Pass";
        else pdlStatusToString (pdlStatus, &typePtr);
   
        pdlStatus = pdlFanControllerHwInit (tempFanControllerId);
        if (pdlStatus == PDL_OK)
            initPtr = "Pass";
        else pdlStatusToString (pdlStatus, &initPtr);
        
        pdlStatus = pdlFanDbRoleGet(tempFanControllerId, tempFanNumber, &role);
        if (pdlStatus == PDL_OK)
            rolePtr = "Pass";
        else pdlStatusToString (pdlStatus, &rolePtr);
             
        pdlStatus =  pdlFanDbCapabilitiesGet (tempFanControllerId, &controllerCapabilities);
        PDL_CHECK_STATUS(pdlStatus);
          
        if (controllerCapabilities.rotationDirectionControlIsSupported == TRUE) {
            pdlStatus = pdlFanControllerHwRotationDirectionGet (tempFanControllerId, &direction);
            if (pdlStatus == PDL_OK){                
                pdlStatus = pdlFanControllerHwRotationDirectionSet (tempFanControllerId, PDL_FAN_DIRECTION_RIGHT_E);
                if (pdlStatus == PDL_OK){                   
                    pdlStatus = pdlFanControllerHwRotationDirectionSet (tempFanControllerId, PDL_FAN_DIRECTION_LEFT_E);
                     if (pdlStatus == PDL_OK){
                         pdlStatus = pdlFanControllerHwRotationDirectionSet (tempFanControllerId, direction);
                         if (pdlStatus == PDL_OK){
                              rotationPtr = "Pass";
                         }
                         else pdlStatusToString (pdlStatus, &rotationPtr);
                     }
                     else pdlStatusToString (pdlStatus, &rotationPtr);
                }
                else pdlStatusToString (pdlStatus, &rotationPtr);
            }
            else pdlStatusToString (pdlStatus, &rotationPtr);
        }
        else rotationPtr = "Not Supported";

        if (controllerCapabilities.dutyCycleMethodControlIsSupported == TRUE) {
            pdlStatus = pdlFanControllerHwDutyCycleMethodSet (tempFanControllerId, PDL_FAN_DUTY_CYCLE_METHOD_HW_E);
            if (pdlStatus == PDL_OK) {
                pdlStatus = pdlFanControllerHwDutyCycleMethodSet (tempFanControllerId, PDL_FAN_DUTY_CYCLE_METHOD_SW_E);
                 if (pdlStatus == PDL_OK) {
                       dutyCycleMethodPtr = "Pass";
                 }
                else pdlStatusToString (pdlStatus, &dutyCyclePtr);
            }
            else pdlStatusToString (pdlStatus, &dutyCyclePtr);
        }
        else dutyCycleMethodPtr = "Not Supported";           
        
        if (controllerCapabilities.rpmInfoSupported == TRUE){
            pdlStatus = pdlFanControllerHwRpmGet (tempFanControllerId, &rpm);
            if (pdlStatus == PDL_OK) {
                rpmPtr = "Pass";
            }
            else pdlStatusToString (pdlStatus, &rpmPtr);
        }
        else rpmPtr = "Not Supported";
     
        if (controllerCapabilities.faultControlIsSupported == TRUE){          
            pdlStatus = pdlFanControllerHwFaultSet (tempFanControllerId,  PDL_FAN_FAULT_NORMAL_E);
                if (pdlStatus == PDL_OK) {                    
                    pdlStatus = pdlFanControllerHwFaultSet (tempFanControllerId,  PDL_FAN_FAULT_CLEAR_E);
                        if (pdlStatus == PDL_OK) {
                            faultPtr = "Pass";
                        }
                        else pdlStatusToString (pdlStatus, &faultPtr);
                }
                else pdlStatusToString (pdlStatus, &faultPtr);
        }
        else faultPtr = "Not Supported";
  
        if (controllerCapabilities.stateInfoIsSupported == TRUE){
            pdlStatus = pdlFanHwStatusGet (tempFanControllerId, tempFanNumber, &state);
            if (pdlStatus == PDL_OK) {
                statePtr = "Pass";
            }
            else pdlStatusToString (pdlStatus, &statePtr);
        }
        else statePtr = "Not Supported";   
        
        if (controllerCapabilities.stateControlIsSupported == TRUE){
            tempState =   PDL_FAN_STATE_CONTROL_NORMAL_E;
            pdlStatus = pdlFanHwStateSet (tempFanControllerId, tempFanNumber, tempState);
            if (pdlStatus == PDL_OK){
                tempState =  PDL_FAN_STATE_CONTROL_SHUTDOWN_E;
                pdlStatus = pdlFanHwStateSet (tempFanControllerId, tempFanNumber,tempState);
                if (pdlStatus == PDL_OK){
                    tempState =   PDL_FAN_STATE_CONTROL_NORMAL_E;
                    pdlStatus = pdlFanHwStateSet (tempFanControllerId, tempFanNumber, tempState);
                    if (pdlStatus == PDL_OK){
                        stateConPtr = "Pass";
                    }
                    else pdlStatusToString (pdlStatus, &stateConPtr);
                }
                else pdlStatusToString (pdlStatus, &stateConPtr);
            }
            else pdlStatusToString (pdlStatus, &stateConPtr);
        }  
        else stateConPtr = "Not Supported";
                  
        if (controllerCapabilities.dutyCycleInfoIsSupported == TRUE) {
            pdlStatus = pdlFanHwDutyCycleGet (tempFanControllerId, tempFanNumber, &speed, &speedPrecent);
            if (pdlStatus == PDL_OK){
                 dutyCyclePtr = "Pass";
            }
            else pdlStatusToString (pdlStatus, &dutyCyclePtr);
        }
        else dutyCyclePtr = "Not Supported";
       
        pulsePtr = "";
        ThresholdPtr = "";
        
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-10s | %-8s | %-25s | %-25s | %-25s | %-25s | %-25s |\n", "Ctrl ID", "Fan Number", " Fan ID ", "      ID <-> Number      ", "           Type          ", "           Init          ", "         Rotation        ", "     DutyCycleMethod     ");    
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-10s | %-8s | %-25s | %-25s | %-25s | %-25s | %-25s |\n", "-------", "----------", "--------", "-------------------------", "-------------------------", "-------------------------", "-------------------------", "-------------------------");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8d | %-10d | %-8d | %-25s | %-25s | %-25s | %-25s | %-25s |\n",
            tempFanControllerId,
            tempFanNumber,
            i,
            fanInfoPtr,
            typePtr,
            initPtr,
            rotationPtr,
            dutyCycleMethodPtr);
        
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
        
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-10s | %-8s | %-25s | %-25s | %-25s | %-25s |\n", " Ctrl ID", "Fan Number", " Fan ID ", "        DutyCycle        ", "           Rpm           ", "          Fault          ", "           Role          ");    
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-10s | %-8s | %-25s | %-25s | %-25s | %-25s |\n", "--------", "----------", "--------", "-------------------------", "-------------------------", "-------------------------", "-------------------------");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8d | %-10d | %-8d | %-25s | %-25s | %-25s | %-25s |\n",
            tempFanControllerId,
            tempFanNumber,
            i,
            dutyCyclePtr,
            rpmPtr,
            faultPtr,
            rolePtr);

        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-10s | %-8s | %-25s | %-25s | %-25s | %-25s |\n", " Ctrl ID", "Fan Number", " Fan ID ", "        StateInfo        ", "       stateControl      ", "          Pulse          ", "        Threshold        ");    
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8s | %-10s | %-8s | %-25s | %-25s | %-25s | %-25s |\n", "--------", "----------", "--------", "-------------------------", "-------------------------", "-------------------------", "-------------------------");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-8d | %-10d | %-8d | %-25s | %-25s | %-25s | %-25s |\n",
            tempFanControllerId,
            tempFanNumber,
            i,
            statePtr,
            stateConPtr,
            pulsePtr,
            ThresholdPtr);

        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");  
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");  
    }
    
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");

    return PDL_OK;
}
/*$ END OF iDbgPdlFanRunControllerValidation */



/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanGetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlFanGetHwStatus (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      fanControllerId,
    IN UINT_32                            fanId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstLogicalFanId, lastFanLogicalId, tempFanControllerId, tempFanId;
    UINT_32                         fanDc, fanDcPercent=0, fanRpm=0;
    PDL_FAN_STATE_INFO_ENT          fanState;
    char                            fanStateStr[20]="Unknown", fanDcPercentStr[20];
	PDL_STATUS						pdlStatus;
    PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC    fanCapabilities;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (fanControllerId == IDBG_PDL_ID_ALL) {
        pdlStatus = pdlFanDbGetFirst (&firstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        lastFanLogicalId = firstLogicalFanId;
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlFanDbGetNext (lastFanLogicalId, &lastFanLogicalId);            
        }
        lastFanLogicalId++;
    }
    else {
        pdlStatus = convertFanControllerAndNumberToFanId (fanControllerId, fanId, &firstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        lastFanLogicalId = firstLogicalFanId + 1;
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-8s | %-8s | %-10s | %-7s |\n", "Ctrl ID", "Fan Id", " State ", "Duty Cycle", "  RPM  ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-7s | %-8s | %-8s | %-10s | %-7s |\n", "-------", "------", "-------" ,"----------", "-------");

    for (i = firstLogicalFanId; i <lastFanLogicalId; i++) {
        pdlStatus = pdlFanIdToFanControllerAndNumber (i, &tempFanControllerId, &tempFanId);
        PDL_CHECK_STATUS(pdlStatus);

        IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanDbCapabilitiesGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fanCapabilities));
        PDL_CHECK_STATUS(pdlStatus);

        if (fanCapabilities.dutyCycleInfoIsSupported == TRUE) {
            IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanHwStatusGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanId), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fanState));
            PDL_CHECK_STATUS(pdlStatus);
            switch (fanState) {
                case PDL_FAN_STATE_INFO_OK_E:
                    prvPdlOsSnprintf (fanStateStr, 20, "OK");
                    break;
                case PDL_FAN_STATE_INFO_OVERFLOW_E:
                    prvPdlOsSnprintf (fanStateStr, 20, "Overflow");
                    break;
                case PDL_FAN_STATE_INFO_FAULT_E:
                    prvPdlOsSnprintf (fanStateStr, 20, "Fault");
                    break;
                case PDL_FAN_STATE_INFO_READY_E:
                    prvPdlOsSnprintf (fanStateStr, 20, "Ready");
                    break;
                default:
                    prvPdlOsSnprintf (fanStateStr, 20, "Unknown");
                    break;
            }
        }
        if (fanCapabilities.dutyCycleInfoIsSupported == TRUE) {
            IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanHwDutyCycleGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanId), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fanDc), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fanDcPercent));
            PDL_CHECK_STATUS(pdlStatus);
        }
        if (fanCapabilities.rpmInfoSupported == TRUE) {
            IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanControllerHwRpmGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fanRpm));
            PDL_CHECK_STATUS(pdlStatus);
        }
        prvPdlOsSnprintf (fanDcPercentStr, 20, "%d%%", fanDcPercent);
        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("%-7d | %-8d | %-8s | %-10s | %-7d |\n",
                                        tempFanControllerId,
                                        tempFanId,
                                        fanStateStr,
                                        fanDcPercentStr,
                                        fanRpm);
    }

    return PDL_OK;
}
/*$ END OF iDbgPdlFanGetHwStatus */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanSetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlFanSetHwStatus (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN IDBG_PDL_FAN_SET_CMD_ENT           cmdType,
    IN UINT_32                  	      fanControllerId,
    IN UINT_32                            fanId,
    IN UINT_32                            value
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstLogicalFanId, lastFanLogicalId, tempFanControllerId, tempFanNumber;
    PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC    controllerCapabilities;
    PDL_STATUS						pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (fanControllerId == IDBG_PDL_ID_ALL) {
        pdlStatus = pdlFanDbGetFirst (&firstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        lastFanLogicalId = firstLogicalFanId;
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlFanDbGetNext (lastFanLogicalId, &lastFanLogicalId);            
        }
        lastFanLogicalId++;
    }
    else if (fanId == IDBG_PDL_ID_ALL) {
        pdlStatus = pdlFanDbGetFirst (&firstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        lastFanLogicalId = firstLogicalFanId + 1;
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlFanIdToFanControllerAndNumber (firstLogicalFanId, &tempFanControllerId, &tempFanNumber);
            if (tempFanControllerId == fanControllerId) {
                lastFanLogicalId = firstLogicalFanId + 1;
                break;
            }
            pdlStatus = pdlFanDbGetNext (firstLogicalFanId, &firstLogicalFanId);            
        }
    }
    else {
        pdlStatus = convertFanControllerAndNumberToFanId (fanControllerId, fanId, &firstLogicalFanId);
        PDL_CHECK_STATUS(pdlStatus);
        lastFanLogicalId = firstLogicalFanId + 1;
    }

  
    for (i = firstLogicalFanId; i <lastFanLogicalId; i++) {
        pdlStatus = pdlFanIdToFanControllerAndNumber (i, &tempFanControllerId, &tempFanNumber);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus =  pdlFanDbCapabilitiesGet (tempFanControllerId, &controllerCapabilities);
        PDL_CHECK_STATUS(pdlStatus);
        switch (cmdType){
        case IDBG_PDL_FAN_SET_CMD_STATE_E:
            if (controllerCapabilities.stateControlIsSupported == TRUE) {
                IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanHwStateSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanNumber), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
                PDL_CHECK_STATUS(pdlStatus);
            }
            else IDBG_PDL_LOGGER_API_OUTPUT_LOG("State Not Supported");
            break;
        case IDBG_PDL_FAN_SET_CMD_DUTY_CYCLE_E:
            IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanControllerHwDutyCycleSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
            PDL_CHECK_STATUS(pdlStatus);
            break;
        case IDBG_PDL_FAN_SET_CMD_DEBUG_E:
            pdlStatus = pdlFanDebugSet((BOOLEAN)value);
            PDL_CHECK_STATUS(pdlStatus);
            break;
        case IDBG_PDL_FAN_SET_CMD_ROTATION_DIRECTION_E: 
            if (controllerCapabilities.rotationDirectionControlIsSupported == TRUE) {
                IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanControllerHwRotationDirectionSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
                PDL_CHECK_STATUS(pdlStatus);
            }
            else IDBG_PDL_LOGGER_API_OUTPUT_LOG("Rotation-Direction Not Supported");
            break;
        case IDBG_PDL_FAN_SET_CMD_DUTY_CYCLE_METHOD_E:
            if (controllerCapabilities.dutyCycleMethodControlIsSupported == TRUE) {
                IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanControllerHwDutyCycleMethodSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
                PDL_CHECK_STATUS(pdlStatus);
            }
            else IDBG_PDL_LOGGER_API_OUTPUT_LOG("Duty-Cycle-Method Not Supported");
            break;
        case IDBG_PDL_FAN_SET_CMD_FAULT_E:
            if (controllerCapabilities.faultControlIsSupported == TRUE) {
                IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanControllerHwDutyCycleMethodSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
                PDL_CHECK_STATUS(pdlStatus);
            }
            else IDBG_PDL_LOGGER_API_OUTPUT_LOG("Fault Method Not Supported");
            break;
        case IDBG_PDL_FAN_SET_CMD_INTILAIZE_E:
                pdlStatus = pdlFanControllerHwInit (tempFanControllerId);
                PDL_CHECK_STATUS(pdlStatus);
            break;
        case IDBG_PDL_FAN_SET_CMD_PULSE_PER_ROTATION_E:
            if (controllerCapabilities.pulsesPerRotationControlIsSupported == TRUE) {
                IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanHwPulsesPerRotationSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanNumber), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
                PDL_CHECK_STATUS(pdlStatus);
            }
            else IDBG_PDL_LOGGER_API_OUTPUT_LOG("Pulse Per Rotation Not Supported");
            break;
        case IDBG_PDL_FAN_SET_CMD_THRESHOLD_E:
            if (controllerCapabilities.thresholdControlIsSupported == TRUE) {
                IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlFanHwThresholdSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanControllerId), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, tempFanNumber), IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, value));
                PDL_CHECK_STATUS(pdlStatus);
            }
            else IDBG_PDL_LOGGER_API_OUTPUT_LOG("Threshold Not Supported");
            break;
        default:
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown cmdId");
            return PDL_BAD_VALUE;
            break;
        }
    }
    return pdlStatus;
}


/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanGetControllerinfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlFanGetControllerinfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      fanControllerId    
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, j, capabilitiesCount, firstLogicalFanId, lastFanLogicalId, tempFanControllerId, tempFanControllerNumber; 
    UINT_32                         firstfanControllerId, lastfanControllerId;                     
    PDL_STATUS						pdlStatus;
    PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC    controllerCapabilities;
    PDL_FAN_INTERFACE_STC           fanInterface;
    PDL_INTERFACE_I2C_STC           fanAttributes;
    char                            tempfan[5], fans[25], i2cInterfaceId[25];
    char                            tempCapabilities[PDL_FAN_NUMBER_OF_CAPABILITIES][25], capabilities[PDL_FAN_NUMBER_OF_CAPABILITIES][25] ;
    PDL_FAN_CONTROLLER_TYPE_ENT     fanType;
    char                            * fanTypeString = NULL;
    char                            * fanTransactionTypeString = NULL, *fanInterfaceTypeString = NULL;
    BOOLEAN                         inController = FALSE, find = FALSE, single = FALSE;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (fanControllerId == IDBG_PDL_ID_ALL) {
        pdlStatus = pdlFanDbGetFirstController (&firstfanControllerId);
        PDL_CHECK_STATUS(pdlStatus);
        lastfanControllerId = firstfanControllerId;
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlFanDbGetNextController (lastfanControllerId, &lastfanControllerId);            
        }
        lastfanControllerId++;
    }
    else {
        single = TRUE;
        pdlStatus = pdlFanDbGetFirstController (&tempFanControllerId);
        PDL_CHECK_STATUS(pdlStatus);
        while ((pdlStatus == PDL_OK) && (!find)) {
            if (tempFanControllerId == fanControllerId) {
                find = TRUE;
                break;
            }
            pdlStatus = pdlFanDbGetNextController (tempFanControllerId, &tempFanControllerId);   
            PDL_CHECK_STATUS(pdlStatus);
        }
        if (!find) return PDL_NOT_FOUND;
        firstfanControllerId = fanControllerId;
        lastfanControllerId = firstfanControllerId + 1;
    }
    
    pdlStatus = pdlFanDbGetFirst (&firstLogicalFanId);
    PDL_CHECK_STATUS(pdlStatus);
    lastFanLogicalId = firstLogicalFanId;
    while (pdlStatus == PDL_OK) {
        pdlStatus = pdlFanDbGetNext (lastFanLogicalId, &lastFanLogicalId);            
    }
    lastFanLogicalId++;
    
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-25s | %-25s | %-25s | %-25s | %-25s |\n", " fan-controller-number "," fan-number list ", " capabilities ", " i2c-interfaceId ", " fan-Hw ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-25s | %-25s | %-25s | %-25s | %-25s |\n", "---------------------- ","-----------------", "--------------", "-----------------", "--------");
    for (i = firstfanControllerId; i <lastfanControllerId; i++) {
        memset (&fans, '\0', sizeof (fans));
        inController = FALSE;
        for (j = firstLogicalFanId; j <lastFanLogicalId; j++) {
            pdlStatus = pdlFanIdToFanControllerAndNumber (j, &tempFanControllerId, &tempFanControllerNumber);
            PDL_CHECK_STATUS(pdlStatus);
            if (tempFanControllerId == i){
                inController = TRUE;
                PRV_IDBG_PDL_LOGGER_SNPRINTF_MAC(tempfan, sizeof(tempfan)-1, "%d , ",  tempFanControllerNumber);
                if (strlen(fans) < sizeof(fans) - 4) {
                	strncat(fans, tempfan, 4);
                }
            }
            
        }
        if (!inController) break;
        
        pdlStatus =  pdlFanDbCapabilitiesGet (i, &controllerCapabilities);
        PDL_CHECK_STATUS(pdlStatus);
        
        pdlStatus = pdlFanControllerGetType (i, &fanType);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_FAN_TYPE_E, fanType, &fanTypeString);
        PDL_CHECK_STATUS(pdlStatus);
        
        pdlStatus = prvPdlFanGetInterface(i , &fanInterface);
        PDL_CHECK_STATUS(pdlStatus);
        
        pdlStatus = PdlI2CDebugDbGetAttributes (fanInterface.interfaceId, &fanAttributes);
        PDL_CHECK_STATUS(pdlStatus);
        
        strncpy(tempCapabilities[0], (controllerCapabilities.stateControlIsSupported ? "stateControl" : "null"), 25);
        strncpy(tempCapabilities[1], (controllerCapabilities.stateInfoIsSupported ? "stateInfo" : "null"), 25);
        strncpy(tempCapabilities[2], (controllerCapabilities.dutyCycleControlIsSupported ? "dutyCycleControl" : "null"), 25);
        strncpy(tempCapabilities[3], (controllerCapabilities.dutyCycleInfoIsSupported ? "dutyCycleInfo" : "null"), 25);
        strncpy(tempCapabilities[4], (controllerCapabilities.dutyCycleMethodControlIsSupported ? "dutyCycleMethodControl" : "null"), 25);
        strncpy(tempCapabilities[5], (controllerCapabilities.rotationDirectionControlIsSupported ? "rotationDirectionControl" : "null"), 25);
        strncpy(tempCapabilities[6], (controllerCapabilities.faultControlIsSupported ? "faultControl" : "null"), 25);
        strncpy(tempCapabilities[7], (controllerCapabilities.pulsesPerRotationControlIsSupported ? "pulsesPerRotationControl" : "null"), 25);
        strncpy(tempCapabilities[8], (controllerCapabilities.thresholdControlIsSupported ? "thresholdControl" : "null"),25);
        strncpy(tempCapabilities[9], (controllerCapabilities.rpmInfoSupported ? "rpmInfo" : "null"),25);  
        capabilitiesCount = 0;
        for (j = 0; j <PDL_FAN_NUMBER_OF_CAPABILITIES; j++) {
            strcpy(capabilities[capabilitiesCount], "");
            if (strcmp(tempCapabilities[j], "null")) {
                strncpy(capabilities[capabilitiesCount], tempCapabilities[j], 25);
                capabilitiesCount++;
            }
        }
        pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_I2C_TRANSACTION_TYPE_E, fanAttributes.transactionType, &fanTransactionTypeString);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = iDbgPdlConvertEnumToString (IDBG_PDL_UTILS_INTERFACE_TYPE, fanInterface.interfaceType ,&fanInterfaceTypeString);
        PDL_CHECK_STATUS(pdlStatus);
        prvPdlOsSnprintf (i2cInterfaceId, 25, "%d" ,  fanInterface.interfaceId);  
        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("%-25d | %-25s | %-25s | %-25s | %-25s |\n", i, fans, "", i2cInterfaceId, fanTypeString);
        for (j = 0; j <capabilitiesCount; j++) {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG ("%-25s | %-25s | %-25s | %-25s | %-25s |\n", "", "", capabilities[j], "", "");
        }
        if (single) {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG ("\n\n");
            pdlStatus = iDbgPdlInterfaceGetInfo (resultPtr, fanInterfaceTypeString, fanInterface.interfaceId);
            PDL_CHECK_STATUS(pdlStatus);
        }
    }    
   
    return PDL_OK;
}
       
/*$ END OF iDbgPdlFanGetControllerinfo */



/*****************************************************************************
* FUNCTION NAME:  iDbgPdlFanControllerGetHw
*
* DESCRIPTION: 
*      
*
*****************************************************************************/


PDL_STATUS  iDbgPdlFanControllerGetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      fanControllerId
    )
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, firstfanControllerId, lastfanControllerId, tempFanControllerId;
    UINT_32                         fanRpm = 0;
    char                            directionPtrSt [20], fanRpmPtr[20];
    PDL_FAN_DIRECTION_ENT           directionPtr = 0;
    PDL_STATUS						pdlStatus;
    PDL_FAN_CONTROLLER_XML_CAPABILITIES_STC    controllerCapabilities;
    BOOLEAN                         find = FALSE;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (fanControllerId == IDBG_PDL_ID_ALL) {
        pdlStatus = pdlFanDbGetFirstController (&firstfanControllerId);
        PDL_CHECK_STATUS(pdlStatus);
        lastfanControllerId = firstfanControllerId;
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlFanDbGetNextController (lastfanControllerId, &lastfanControllerId);            
        }
        lastfanControllerId++;
    }
    else {
        pdlStatus = pdlFanDbGetFirstController (&tempFanControllerId);
        PDL_CHECK_STATUS(pdlStatus);
        while ((pdlStatus == PDL_OK) && (!find)) {
            if (tempFanControllerId == fanControllerId) {
                find = TRUE;
                break;
            }
            pdlStatus = pdlFanDbGetNextController (tempFanControllerId, &tempFanControllerId);   
            PDL_CHECK_STATUS(pdlStatus);
        }
        if (!find) return PDL_NOT_FOUND;
        firstfanControllerId = fanControllerId;
        lastfanControllerId = firstfanControllerId + 1;
    }
    
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-20s | %-20s | %-20s |\n", "       Ctrl ID      ", " Rotation Direction ", "         RPM        ");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-20s | %-20s | %-20s |\n", "--------------------", "--------------------", "--------------------");

    for (i =firstfanControllerId; i<lastfanControllerId; i++) {
        pdlStatus =  pdlFanDbCapabilitiesGet (i, &controllerCapabilities);
        PDL_CHECK_STATUS(pdlStatus);
        
        if (controllerCapabilities.rpmInfoSupported == TRUE){
            pdlStatus =  pdlFanControllerHwRpmGet(i, &fanRpm);
            PDL_CHECK_STATUS(pdlStatus);
            prvPdlOsSnprintf (fanRpmPtr, 20, "%d",  fanRpm);
        }
        else prvPdlOsSnprintf (fanRpmPtr, 20, "Not Supported");
        if (controllerCapabilities.rotationDirectionControlIsSupported == TRUE){
            pdlStatus =  pdlFanControllerHwRotationDirectionGet(i, &directionPtr);
            PDL_CHECK_STATUS(pdlStatus);
            PRV_IDBG_PDL_LOGGER_SNPRINTF_MAC(directionPtrSt,sizeof(directionPtrSt)-1, "%s",  directionPtr);
        }
        else prvPdlOsSnprintf (directionPtrSt, 20, "Not Supported");

        IDBG_PDL_LOGGER_API_OUTPUT_LOG ("%-20d | %-20s | %-20s |\n",
            i,
            directionPtrSt,
            fanRpmPtr);
    }

    return PDL_OK;
}
/*$ END OF iDbgPdlFanControllerGetHw */


