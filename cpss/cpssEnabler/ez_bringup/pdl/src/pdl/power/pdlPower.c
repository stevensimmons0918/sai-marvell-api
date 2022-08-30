/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file pdlPower.c   
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
 * @brief Platform driver layer - Power related API
 * 
 * @version   1 
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/power_supply/pdlPower.h>
#include <pdl/power_supply/pdlPowerDebug.h>
#include <pdl/power_supply/private/prvPdlPower.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/xml/private/prvXmlParser.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The pdl power ps db[ pdl power ps type last e] */
static PDL_POWER_PS_INTERFACE_STC           pdlPowerPsDb[PDL_POWER_PS_TYPE_LAST_E] = {{0,0,{0}}}; /* keeps interface for reading PS status*/
static PDL_DB_PRV_STC                       pdlPowerRpsStatusDb[PDL_POWER_RPS_STATUS_LAST_E];

/** @brief   The pdl power bank db[ pdl power ps type last e][pdl power bank max] */
static PDL_POWER_BANK_WATTS_TYP             pdlPowerBankDb[PDL_POWER_PS_TYPE_LAST_E][PDL_POWER_BANK_MAX];           /* keeps watts per bank */
static PDL_POWER_BANK_NUMBER_TYP            pdlPowerBankCount[PDL_POWER_PS_TYPE_LAST_E] = {0};
/** @brief   The pdl power capabilites */
static PDL_POWER_XML_CAPABILITIES_STC       pdlPowerCapabilites;                                                    /* global - system power capabilities */
/** @brief   The pdl power port database */
static PDL_DB_PRV_STC                       pdlPowerPortDb;                                                         /* per port power information */
static PDL_DB_PRV_STC                       pdlPowerPseListDb;                                                      /* pse list power information */

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                 \
    if (__pdlStatus != PDL_OK) {                                                                      \
    if (prvPdlPowerDebugFlag) {                                                                       \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);     \
    }                                                                                                 \
    return __pdlStatus;                                                                               \
    }  
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                \
    if (__xmlStatus != PDL_OK) {                                                                     \
    if (prvPdlPowerDebugFlag) {                                                                      \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);    \
    }                                                                                                \
    return __xmlStatus;                                                                              \
    }  




/* ***************************************************************************
* FUNCTION NAME: pdlPowerDbCapabilitiesGet
*
* DESCRIPTION:   Get power supported capabilities
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDbCapabilitiesGet ( OUT PDL_POWER_XML_CAPABILITIES_STC * capabilitiesPtr )
 *
 * @brief   Pdl power database capabilities get
 *
 * @param [in,out]  capabilitiesPtr If non-null, the capabilities pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDbCapabilitiesGet (
    OUT PDL_POWER_XML_CAPABILITIES_STC *  capabilitiesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (capabilitiesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"capabilitiesPtr NULL");
        return PDL_BAD_PTR;
    }
    memcpy (capabilitiesPtr, &pdlPowerCapabilites, sizeof(pdlPowerCapabilites));
    return PDL_OK;

}

/*$ END OF pdlPowerDbCapabilitiesGet */


/* ***************************************************************************
* FUNCTION NAME: pdlPowerHwStatusGet
*
* DESCRIPTION:   Get power status of the power supply
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerHwStatusGet ( IN PDL_POWER_PS_TYPE_ENT type, OUT PDL_POWER_STATUS_ENT * statusPtr )
 *
 * @brief   Pdl power hardware status get
 *
 * @param           type        The type.
 * @param [in,out]  statusPtr   If non-null, the status pointer.
 * @param [in,out]  statusPtr   If non-null, rps explicit status pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerHwStatusGet (
    IN  PDL_POWER_PS_TYPE_ENT         type,
    OUT PDL_POWER_STATUS_ENT        * statusPtr,
    OUT PDL_POWER_RPS_STATUS_ENT    * rpsStatusPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
#ifdef LINUX_HW
    BOOLEAN                                         found;
    UINT_32                                         i, data;
    PDL_STATUS                                      pdlStatus;
    PDL_POWER_RPS_STATUS_PRV_INTERFACE_STC        * rpsStatusInterfacePtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if ( type >= PDL_POWER_PS_TYPE_LAST_E) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"type >= PDL_POWER_PS_TYPE_LAST_E");
        return PDL_BAD_PARAM;
    }
    if (statusPtr == NULL || rpsStatusPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"statusPtr NULL");
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlInterfaceHwGetValue (pdlPowerPsDb[type].interfaceType, pdlPowerPsDb[type].interfaceId, &data);
    PDL_CHECK_STATUS (pdlStatus);
    if (data == pdlPowerPsDb[type].values.activeValue) {
        *statusPtr = PDL_POWER_STATUS_ACTIVE_E;
    }
    else {
        *statusPtr = PDL_POWER_STATUS_NOT_ACTIVE_E;
    }

    /* check extended RPS status */
    if (type == PDL_POWER_PS_TYPE_RPS_E && *statusPtr == PDL_POWER_STATUS_ACTIVE_E) {
        for (i = 0; i <PDL_POWER_RPS_STATUS_LAST_E; i++) {
            found = FALSE;
            pdlStatus = prvPdlDbGetFirst (&pdlPowerRpsStatusDb[i], (void*) &rpsStatusInterfacePtr);
            while (pdlStatus == PDL_OK) {
                pdlStatus = prvPdlInterfaceHwGetValue (rpsStatusInterfacePtr->interfaceType, rpsStatusInterfacePtr->interfaceId, &data);
                PDL_CHECK_STATUS (pdlStatus);
                if (data != rpsStatusInterfacePtr->values.activeValue) {
                    found = FALSE;
                    break;
                }
                else {
                    found = TRUE;
                }
                pdlStatus = prvPdlDbGetNext (&pdlPowerRpsStatusDb[i], (void*)& rpsStatusInterfacePtr->key, (void*) &rpsStatusInterfacePtr);
            }
            if (found == TRUE) {
                *rpsStatusPtr = i;
                break;
            }
        }
    }
#else
    *statusPtr = PDL_POWER_STATUS_ACTIVE_E;
    if (rpsStatusPtr)
        *rpsStatusPtr = PDL_POWER_RPS_STATUS_READY_E;
#endif

    return PDL_OK;

}

/*$ END OF pdlPowerHwStatusGet */

/**
 * @fn  PDL_STATUS pdlPowerWrapHwStatusGet ( IN char psStr, OUT PDL_POWER_STATUS_ENT * statusPtr )
 *
 * @brief   Pdl power hardware status get
 *
 * @param           psStr       String for ps/rps
 * @param [in,out]  statusPtr   If non-null, the status pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerWrapHwStatusGet (
    IN  char                        * psStr,
    OUT PDL_POWER_STATUS_ENT        * statusPtr,
    OUT PDL_POWER_RPS_STATUS_ENT    * rpsStatusPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PS_TYPE_ENT               psType;
    UINT_32                             value;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_BANK_SOURCE_TYPE_E, psStr, &value);
    PDL_CHECK_STATUS(pdlStatus);
    psType = (PDL_POWER_PS_TYPE_ENT)value;
    return pdlPowerHwStatusGet (psType, statusPtr, rpsStatusPtr);
}

/*$ END OF pdlPowerWrapHwStatusGet */

/**
 * @fn  PDL_STATUS pdlPowerDbBankWattsGet ( IN PDL_POWER_PS_TYPE_ENT type, IN PDL_power_bank_number_TYP bankNumber, OUT PDL_power_bank_watts_TYP * bankWattsPtr )
 *
 * @brief   Pdl power database bank watts get
 *
 * @param           type            The type.
 * @param           bankNumber      The bank number.
 * @param [in,out]  bankWattsPtr    If non-null, the bank watts pointer.
 *
 * @return          PDL_OK          success.
 * @return          PDL_NO_SUCH     power bank doesn't exist in given bank type
 */

PDL_STATUS pdlPowerDbBankWattsGet (
    IN  PDL_POWER_PS_TYPE_ENT        type,
    IN  PDL_POWER_BANK_NUMBER_TYP    bankNumber,
    OUT PDL_POWER_BANK_WATTS_TYP   * bankWattsPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if ( type >= PDL_POWER_PS_TYPE_LAST_E || bankNumber >= PDL_POWER_BANK_MAX) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"ILLIGAL bankNumber OR type");
        return PDL_BAD_PARAM;
    }
    if (bankWattsPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"bankWattsPtr NULL");
        return PDL_BAD_PTR;
    }
    if (pdlPowerBankDb[type][bankNumber] == PDL_POWER_BANK_INVALID) {
        return PDL_NO_SUCH;
    }
    *bankWattsPtr = pdlPowerBankDb[type][bankNumber];
    return PDL_OK;
}

/*$ END OF pdlPowerDbBankWattsGet */

/**
 * @fn  PDL_STATUS pdlPowerDbBankWattsGet ( IN PDL_POWER_PS_TYPE_ENT type, OUT PDL_POWER_BANK_NUMBER_TYP *nunOfBanksPtr )
 *
 * @brief   Pdl power database bank watts get
 *
 * @param           type            The type.
 * @param [out]     nunOfBanksPtr   The number of banks for given ps type
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDbNumOfBanksGet (
    IN  PDL_POWER_PS_TYPE_ENT        type,
    OUT PDL_POWER_BANK_NUMBER_TYP  * nunOfBanksPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/

/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if ( type >= PDL_POWER_PS_TYPE_LAST_E) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"ILLIGAL type");
        return PDL_BAD_PARAM;
    }
    if (nunOfBanksPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"nunOfBanksPtr NULL");
        return PDL_BAD_PTR;
    }
    *nunOfBanksPtr = pdlPowerBankCount[type];
    return PDL_OK;
}

/*$ END OF pdlPowerDbNumOfBanksGet */


/* ***************************************************************************
* FUNCTION NAME: pdlPowerDbPortCapabilityGet
*
* DESCRIPTION:   Get port power capability according to XML
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDbPortCapabilityGet ( IN UINT_32 device, IN UINT_32 logicalPort, OUT PDL_POWER_POE_CAPABILITY_ENT * capabilityPtr )
 *
 * @brief   Pdl power database port capability get
 *
 * @param           device          The device.
 * @param           port            The port.
 * @param [in,out]  capabilityPtr   If non-null, the capability pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDbPortCapabilityGet (
    IN  UINT_32                         device,
    IN  UINT_32                         logicalPort,
    OUT PDL_POWER_POE_CAPABILITY_ENT  * capabilityPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (capabilityPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"capabilityPtr NULL");
        return PDL_BAD_PTR;
    }
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlPowerPortDb, (void*) &portPowerKey, (void*) &portPtr);
    PDL_CHECK_STATUS (pdlStatus);
    memcpy (capabilityPtr, &portPtr->poeCapability, sizeof(PDL_POWER_POE_CAPABILITY_ENT));
    return PDL_OK;
}

/*$ END OF pdlPowerDbPortCapabilityGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerDbPsePortTypeGet
*
* DESCRIPTION:   Get port power info according to XML
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDbPsePortTypeGet ( IN UINT_32 device, IN UINT_32 logicalPort, OUT PDL_POWER_PSE_PORT_INFO_STC * psePortinfoPtr )
 *
 * @brief   Pdl power database pse port type get
 *
 * @param           device          The device.
 * @param           port            The port.
 * @param [in,out]  psePortinfoPtr  If non-null, the pse portinfo pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDbPsePortTypeGet (
    IN  UINT_32                         device,
    IN  UINT_32                         logicalPort,
    OUT PDL_POWER_PSE_PORT_INFO_STC   * psePortinfoPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (psePortinfoPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"psePortinfoPtr NULL, PORT ID: %d, DEVICE ID: %d",logicalPort,device);
        return PDL_BAD_PTR;
    }
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlPowerPortDb, (void*) &portPowerKey, (void*) &portPtr);
    PDL_CHECK_STATUS (pdlStatus);
    memcpy (psePortinfoPtr, &portPtr->pseInfo, sizeof(PDL_POWER_PSE_PORT_INFO_STC));
    return PDL_OK;

}

/*$ END OF pdlPowerDbPsePortTypeGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerPdClear
*
* DESCRIPTION:   clears a PD port
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerPdClear ( IN UINT_32 device, IN UINT_32 port )
 *
 * @brief   Pdl power pd clear
 *
 * @param   device  The device.
 * @param   port    The port.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerPdClear (
    IN  UINT_32                     device,
    IN  UINT_32                     logicalPort
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlPowerPortDb, (void*) &portPowerKey, (void*) &portPtr);
    PDL_CHECK_STATUS (pdlStatus);
    pdlStatus = prvPdlInterfaceHwSetValue (portPtr->pdPortInterface.afInfo.interfaceType, portPtr->pdPortInterface.afInfo.interfaceId, ~(portPtr->pdPortInterface.values.afValue));
    PDL_CHECK_STATUS (pdlStatus);
    pdlStatus = prvPdlInterfaceHwSetValue (portPtr->pdPortInterface.atInfo.interfaceType, portPtr->pdPortInterface.atInfo.interfaceId, ~(portPtr->pdPortInterface.values.atValue));
    PDL_CHECK_STATUS (pdlStatus);
    pdlStatus = prvPdlInterfaceHwSetValue (portPtr->pdPortInterface.sixtyWattInfo.interfaceType, portPtr->pdPortInterface.sixtyWattInfo.interfaceId, ~(portPtr->pdPortInterface.values.sixtyWattValue));
    PDL_CHECK_STATUS (pdlStatus);
    return PDL_OK;
}

/*$ END OF pdlPowerPdClear */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerPdSet
*
* DESCRIPTION:   
*
* PARAMETERS:   
*                
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerPdSet ( IN UINT_32 device, IN UINT_32 logicalPort, IN PDL_POWER_PORT_TYPE_ENT powerType )
 *
 * @brief   Pdl power pd set
 *
 * @param   device      The device.
 * @param   port        The port.
 * @param   powerType   Type of the power.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerPdSet (
    IN  UINT_32                     device,
    IN  UINT_32                     logicalPort,
    IN  PDL_POWER_PORT_TYPE_ENT     powerType
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlPowerPortDb, (void*) &portPowerKey, (void*) &portPtr);
    PDL_CHECK_STATUS (pdlStatus);
    switch (powerType) {
    case PDL_POWER_PORT_TYPE_AT_E: 
        pdlStatus = prvPdlInterfaceHwSetValue (portPtr->pdPortInterface.atInfo.interfaceType, portPtr->pdPortInterface.atInfo.interfaceId, (portPtr->pdPortInterface.values.atValue));
        break;
    case PDL_POWER_PORT_TYPE_AF_E:
        pdlStatus = prvPdlInterfaceHwSetValue (portPtr->pdPortInterface.afInfo.interfaceType, portPtr->pdPortInterface.afInfo.interfaceId, (portPtr->pdPortInterface.values.afValue));
        break;
    case PDL_POWER_PORT_TYPE_60W_E:
        pdlStatus = prvPdlInterfaceHwSetValue (portPtr->pdPortInterface.sixtyWattInfo.interfaceType, portPtr->pdPortInterface.sixtyWattInfo.interfaceId, (portPtr->pdPortInterface.values.sixtyWattValue));
        break;
    default:
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"ILLIGAL powerType, PORT ID: %d, DEVICE ID: %d",logicalPort,device);
        return PDL_BAD_PARAM;
    }
    PDL_CHECK_STATUS (pdlStatus);
    return PDL_OK;
}

/*$ END OF pdlPowerPdSet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerPseAddressGet
*
* DESCRIPTION:   
*
* PARAMETERS:   
*                
*****************************************************************************/
/**
 * @fn  PDL_STATUS pdlPowerPseAddressGet ( IN UINT_8 pseNumber, OUT UINT_8 * pseAddressPtr)
 *
 * @brief   Pdl power PSE SPI address get
 *
 * @param           pseNumber       PSE ID
 * @param [in,out]  pseAddressPtr   If non-null, the pse SPI address pointer.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlPowerPseAddressGet (
    IN  UINT_8                      pseNumber,
    OUT UINT_8                    * pseAddressPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PSE_LIST_INFO_STC       * psePtr;
    PDL_POWER_PSE_LIST_PRV_KEY_STC      pseKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (pseAddressPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"pseAddressPtr=NULL");
        return PDL_BAD_PTR;
    }

    pseKey.number = pseNumber;
    pdlStatus = prvPdlDbFind (&pdlPowerPseListDb, (void*) &pseKey, (void*) &psePtr);
    PDL_CHECK_STATUS (pdlStatus);
    *pseAddressPtr = psePtr->pseAddress;
    return PDL_OK;
}

/*$ END OF pdlPowerPseAddressGet */


/* ***************************************************************************
* FUNCTION NAME: pdlPowerDbNumOfPsesGet
*
* DESCRIPTION:   
*
* PARAMETERS:   
*                
*****************************************************************************/
/**
* @fn  PDL_STATUS pdlPowerDbNumOfPsesGet ( OUT UINT_32  *numOfPsesPtrr );
 *
 * @brief   Pdl get number of PSEs devices
 *
 * @param [OUT] *numOfPsesPtr   .
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlPowerDbNumOfPsesGet (
    OUT  UINT_32    *  numOfPsesPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    return prvPdlDbGetNumOfEntries(&pdlPowerPseListDb, numOfPsesPtr);
    
}

/*$ END OF pdlPowerPseAddressGet */


/* ***************************************************************************
* FUNCTION NAME: pdlPowerDebugStatusAccessInfoGet
*
* DESCRIPTION:   get PS interface specification
*
* PARAMETERS:   
*                
* XML structure:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDebugStatusAccessInfoGet ( IN PDL_POWER_PS_TYPE_ENT type, OUT PDL_POWER_PS_INTERFACE_STC * interfacePtr )
 *
 * @brief   Pdl power debug status access information get
 *
 * @param           type            The type.
 * @param [in,out]  interfacePtr    If non-null, the interface pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDebugStatusAccessInfoGet (
    IN  PDL_POWER_PS_TYPE_ENT          type,
    OUT PDL_POWER_PS_INTERFACE_STC   * interfacePtr 
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if ( type >= PDL_POWER_PS_TYPE_LAST_E) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"ILLIGAL type");
        return PDL_BAD_PARAM;
    }
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"interfacePtr NULL");
        return PDL_BAD_PTR;
    }
    memcpy (interfacePtr, &pdlPowerPsDb[type], sizeof(PDL_POWER_PS_INTERFACE_STC));
    return PDL_OK;   
}

/*$ END OF pdlPowerDebugStatusAccessInfoGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerDebugPseActiveAccessInfoGet
*
* DESCRIPTION:   get PSE interface specification
*
* PARAMETERS:   
*                
* XML structure:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDebugPseActiveAccessInfoGet ( IN UINT_32 device, IN UINT_32 logicalPort, OUT PDL_POWER_PORT_PSE_INTERFACE_STC * interfacePtr )
 *
 * @brief   Pdl power debug pse active access information get
 *
 * @param           device          The device.
 * @param           port            The port.
 * @param [in,out]  interfacePtr    If non-null, the interface pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDebugPseActiveAccessInfoGet (
    IN  UINT_32                             device,
    IN  UINT_32                             logicalPort,
    OUT PDL_POWER_PORT_PSE_INTERFACE_STC  * interfacePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"interfacePtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d",logicalPort,device);
        return PDL_BAD_PTR;
    }
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlPowerPortDb, (void*) &portPowerKey, (void*) &portPtr);
    PDL_CHECK_STATUS (pdlStatus);
    memcpy (interfacePtr, &portPtr->pseInfo.interfaceInfo, sizeof(PDL_POWER_PORT_PSE_INTERFACE_STC));
    return PDL_OK;   
}

/*$ END OF pdlPowerDebugPseActiveAccessInfoGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerDebugPdActiveAccessInfoGet
*
* DESCRIPTION:   get PD interface specification
*
* PARAMETERS:   
*                
* XML structure:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDebugPdActiveAccessInfoGet ( IN UINT_32 device, IN UINT_32 logicalPort, OUT PDL_POWER_PD_PORT_INTERFACE_STC * interfacePtr )
 *
 * @brief   Pdl power debug pd active access information get
 *
 * @param           device          The device.
 * @param           port            The port.
 * @param [in,out]  interfacePtr    If non-null, the interface pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDebugPdActiveAccessInfoGet (
    IN  UINT_32                             device,
    IN  UINT_32                             logicalPort,
    OUT PDL_POWER_PD_PORT_INTERFACE_STC   * interfacePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"interfacePtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d",logicalPort,device);
        return PDL_BAD_PTR;
    }
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlDbFind (&pdlPowerPortDb, (void*) &portPowerKey, (void*) &portPtr);
    PDL_CHECK_STATUS (pdlStatus);
    memcpy (interfacePtr, &portPtr->pdPortInterface, sizeof(PDL_POWER_PD_PORT_INTERFACE_STC));
    return PDL_OK;   
}

/*$ END OF pdlPowerDebugPdActiveAccessInfoGet */

/**
 * @fn  PDL_STATUS prvPdlPowerPdInfoParser ( )
 *
 * @brief   Prv pdl power pse type to enum
 *
 * @param [in]      xmlId               xml pointer
 * @param [out]     interfaceTypePtr    type of interface
 * @param [out]     interfaceIdPtr      the interface id that was stored in interface Db
 * @param [out]     valuePtr            active value read from XML
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPowerPdInfoParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    OUT PDL_INTERFACE_TYPE_ENT            * interfaceTypePtr,
    OUT PDL_INTERFACE_TYP                 * interfaceIdPtr,
    OUT UINT_32                           * valuePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 maxSize;
    BOOLEAN                                 isInterface;
    PDL_STATUS                              pdlStatus;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfaceTypePtr == NULL || interfaceIdPtr == NULL || valuePtr == NULL) {
        return PDL_BAD_PTR;
    }
    /* skip over ATinfo/AFinfo/SixtyWInfo tag */
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlId);
    XML_CHECK_STATUS(xmlStatus);
    /* skip over gpio-group/i2c-group tag */
    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlId);
    
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        XML_CHECK_STATUS(xmlStatus);
        pdlStatus = prvPdlInterfaceXmlIsInterface(xmlId, &isInterface);
        PDL_CHECK_STATUS(pdlStatus);
        if (isInterface) {
            xmlStatus = prvPdlInterfaceXmlParser (xmlId, interfaceTypePtr, interfaceIdPtr);
            XML_CHECK_STATUS(xmlStatus);
        }        
        else if (xmlParserIsEqualName (xmlId, "active-value") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(UINT_32);
            xmlStatus = xmlParserGetValue (xmlId, &maxSize, valuePtr);
            XML_CHECK_STATUS(xmlStatus);
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlPdGroupId POINTER OR NAME DON'T MATCH TO: active-info");
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlId, &xmlId);
    }
    return PDL_OK;
}

/*$ END OF prvPdlPowerPdInfoParser */

/**
 * @fn  PDL_STATUS prvPdlPowerXMLActiveInfoParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   parse XML information for PS/RPS activeInfo
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPowerXMLActiveInfoParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId,
    OUT PDL_INTERFACE_TYPE_ENT            * interfaceTypePtr,
    OUT PDL_INTERFACE_TYP                 * interfaceIdPtr,
    OUT UINT_32                           * activeValuePtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 maxSize;
    BOOLEAN                                 isInterface;
    PDL_STATUS                              pdlStatus;
    XML_PARSER_RET_CODE_TYP                 xmlStatus;
    XML_PARSER_NODE_DESCRIPTOR_TYP          xmlPsActiveId;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (interfaceTypePtr == NULL || interfaceIdPtr == NULL || activeValuePtr == NULL) {
        return PDL_BAD_PTR;
    }
    /* verify xmlId is pointing to activeInfo/connectedInfo */
    if (!(xmlParserIsEqualName (xmlId, "activeInfo") == XML_PARSER_RET_CODE_OK) &&
        !(xmlParserIsEqualName (xmlId, "connectedInfo") == XML_PARSER_RET_CODE_OK)) {
            return PDL_ERROR;
    }

    xmlStatus = xmlParserGetFirstChild (xmlId, &xmlPsActiveId);
    XML_CHECK_STATUS(xmlStatus);
    /* skip read-interface-select */
    if (xmlParserIsEqualName (xmlPsActiveId, "interface-select") == XML_PARSER_RET_CODE_OK) {
        xmlStatus = xmlParserGetNextSibling (xmlPsActiveId, &xmlPsActiveId);
        XML_CHECK_STATUS(xmlStatus);
    }

    /* skip over gpio-group / i2c-group tag */
    if ((xmlParserIsEqualName (xmlPsActiveId, "GPIO-read-group-list") == XML_PARSER_RET_CODE_OK)||
        (xmlParserIsEqualName (xmlPsActiveId, "i2c") == XML_PARSER_RET_CODE_OK)){
        xmlStatus = xmlParserGetFirstChild (xmlPsActiveId, &xmlPsActiveId);
        XML_CHECK_STATUS(xmlStatus);
    }
    while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
        pdlStatus = prvPdlInterfaceXmlIsInterface(xmlPsActiveId, &isInterface);
        PDL_CHECK_STATUS(pdlStatus);
        if (isInterface) {
            xmlStatus = prvPdlInterfaceXmlParser (xmlPsActiveId, interfaceTypePtr, interfaceIdPtr);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName (xmlPsActiveId, "active-value") == XML_PARSER_RET_CODE_OK) {
            maxSize = sizeof(UINT_32);
            xmlStatus = xmlParserGetValue (xmlPsActiveId, &maxSize, activeValuePtr);
            XML_CHECK_STATUS(xmlStatus);
        }
        else if (xmlParserIsEqualName (xmlPsActiveId, "read-gpio-index") == XML_PARSER_RET_CODE_OK) {
            /* ignore */
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlPsActiveId POINTER OR NAME DON'T MATCH TO: active-value");
            return PDL_XML_PARSE_ERROR;
        }
        xmlStatus = xmlParserGetNextSibling (xmlPsActiveId, &xmlPsActiveId);
    }
    return PDL_OK;
}

/*$ END OF prvPdlPowerXMLActiveInfoParser */

/* ***************************************************************************
* FUNCTION NAME: prvPdlPowerXMLParser
*
* DESCRIPTION:   Fill all button related information from XML to DB
*
* PARAMETERS:   
*                
* XML structure:
* <power>
*   <PS>
*       <activeInfo>
*           <i2c-interface>
*               ...
*           </i2c-interface>
*           <active-value> </active-value>
*       </activeInfo>
*       <POE-power-banks>
*           <PoEbank>
*               <bank-number> 0 </bank-number>
*               <bank-watts> 800 </bank-watts>
*           </PoEbank>
*           <PoEbank>
*               <bank-number> 1 </bank-number>
*               <bank-watts> 600 </bank-watts>
*           </PoEbank>
*       </POE-power-banks>
*   </PS>
*   <RPS>
*       <activeInfo>
*           <i2c-interface>
*               ...
*           </i2c-interface>
*           <active-value> </active-value>
*       </activeInfo>
*       <POE-power-banks>
*           <PoEbank>
*               <bank-number> 0 </bank-number>
*               <bank-watts> 800 </bank-watts>
*           </PoEbank>
*           <PoEbank>
*               <bank-number> 1 </bank-number>
*               <bank-watts> 600 </bank-watts>
*           </PoEbank>
*       </POE-power-banks>
*   </RPS>
* <PSEports>
*   <PSEport>
*       <panel-group-number> 1 </panel-group-number>
*       <panel-port-number> 1 </panel-port-number>
*       <pse-group>
*           <activeInfo>
*               <i2c-interface>
*                   ...
*               </i2c-interface>
*           </activeInfo>
*           <port-type> AF </port-type>
*           <index1> 11 </index1>
*           <index2> 21 </index2>
*       </pse-group>
*   </PSEport>
* </PSEports>
* <PDports>
*   <PDport>
*       <panel-group-number> </panel-group-number>
*       <panel-port-number> </panel-port-number>
*       <pd-group>
*           <ATinfo>
*               <i2c-interface>
*                   ...
*               </i2c-interface>
*               <active-value> </active-value>
*           </ATinfo>
*           <AFinfo>
*               <i2c-interface>
*                   ...
*               </i2c-interface>
*               <active-value> </active-value>
*           </AFinfo>
*           <_60Winfo>
*               <i2c-interface>
*                   ...
*               </i2c-interface>
*               <active-value> </active-value>
*           </_60Winfo>
*           <clearControl>
*               <i2c-interface>
*                   ...
*               </i2c-interface>
*           </clearControl>
*           <ATcontrol>
*               <i2c-interface>
*                   ...
*               </i2c-interface>
*           </ATcontrol>
*           <AFcontrol>
*               <i2c-interface>
*                   ...
*               </i2c-interface>
*           </AFcontrol>
*           <_60Wcontrol>
*               <i2c-interface>
*                   ...
*               </i2c-interface>
*           </_60Wcontrol>
*       </pd-group>
*   </PDport>
* </PDports>
* </power>
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlPowerXMLParser ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Prv pdl power XML parser
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPowerXMLParser (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    char                                        pdlTagStr[PDL_XML_MAX_TAG_LEN] = {0};
    PDL_POWER_BANK_NUMBER_TYP                   bankNum = 0;
    PDL_POWER_BANK_WATTS_TYP                    bankWatts = 0;
    UINT_32                                     maxSize, mask = 0, psePorts = 0, pdPorts = 0, frontPanelGroup, frontPanelPort, value;
    PDL_STATUS                                  pdlStatus;
    XML_PARSER_RET_CODE_TYP                     xmlStatus, xmlStatus2, xmlStatus3, xmlStatus4;
    XML_PARSER_NODE_DESCRIPTOR_TYP              xmlPowerId, xmlPsId, xmlPsChildId, xmlPoeBankId, xmlPoeBankInfoId;
    XML_PARSER_NODE_DESCRIPTOR_TYP              xmlPsePortsId, xmlPsePortId, xmlPsePortInfoId, xmlPseGroupId, xmlPsStatusChildId, xmlPsStatusValueChildId, xmlPsGpioListChildId;
    XML_PARSER_NODE_DESCRIPTOR_TYP              xmlPdPortsId, xmlPdPortId, xmlPdPortInfo, xmlPdGroupId, xmlPsI2cChildId, xmlPseListInfoId;
    XML_PARSER_NODE_DESCRIPTOR_TYP				xmlPoeId;
    PDL_POWER_PORT_PRV_DB_STC                   portInfo;
    PDL_POWER_PORT_PRV_DB_STC                 * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC                  portKey;
    PDL_POWER_PS_TYPE_ENT                       psType;
    PDL_POWER_RPS_STATUS_ENT                    rpsStatusType = PDL_POWER_RPS_STATUS_STANDBY_E;
    PDL_POWER_RPS_STATUS_PRV_KEY_STC            rpsStatusKey;
    PDL_POWER_RPS_STATUS_PRV_INTERFACE_STC      rpsStatusInterface;
    PDL_POWER_RPS_STATUS_PRV_INTERFACE_STC    * rpsStatusInterfacePtr;
    PDL_POWER_PSE_LIST_PRV_KEY_STC              pseListKey;
    PDL_POWER_PSE_LIST_INFO_STC                 pseListInfo;
    PDL_POWER_PSE_LIST_INFO_STC               * pseListPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    xmlStatus = xmlParserFindByName (xmlId, "Power", &xmlPowerId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_OK) {
        XML_CHECK_STATUS(xmlStatus);

		/* parse PS section */
		xmlStatus = xmlParserFindByName (xmlPowerId, "PS", &xmlPsId);
		if (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
			xmlStatus = xmlParserGetFirstChild (xmlPsId, &xmlPsChildId);
			XML_CHECK_STATUS(xmlStatus);
			pdlStatus = prvPdlPowerXMLActiveInfoParser (xmlPsChildId, &pdlPowerPsDb[PDL_POWER_PS_TYPE_PS_E].interfaceType,
																	  &pdlPowerPsDb[PDL_POWER_PS_TYPE_PS_E].interfaceId,
																	  &pdlPowerPsDb[PDL_POWER_PS_TYPE_PS_E].values.activeValue);
			PDL_CHECK_STATUS(pdlStatus);
		}

		/* parse RPS section */
		xmlStatus = xmlParserFindByName (xmlPowerId, "RPS", &xmlPsId);
		if (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
			pdlPowerCapabilites.RpsIsSupported = TRUE;
			xmlStatus = xmlParserGetFirstChild (xmlPsId, &xmlPsChildId);
			XML_CHECK_STATUS(xmlStatus);
			while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
				if (xmlParserIsEqualName (xmlPsChildId, "connectedInfo") == XML_PARSER_RET_CODE_OK) {
					pdlStatus = prvPdlPowerXMLActiveInfoParser (xmlPsChildId, &pdlPowerPsDb[PDL_POWER_PS_TYPE_RPS_E].interfaceType,
																			  &pdlPowerPsDb[PDL_POWER_PS_TYPE_RPS_E].interfaceId,
																			  &pdlPowerPsDb[PDL_POWER_PS_TYPE_RPS_E].values.activeValue);
					PDL_CHECK_STATUS(pdlStatus);
				}
				else if (xmlParserIsEqualName (xmlPsChildId, "status-list") == XML_PARSER_RET_CODE_OK) {
					xmlStatus2 = xmlParserGetFirstChild (xmlPsChildId, &xmlPsStatusChildId);
					while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
						XML_CHECK_STATUS(xmlStatus2);
						if (xmlParserIsEqualName (xmlPsStatusChildId, "status-type") == XML_PARSER_RET_CODE_OK) {
							maxSize = PDL_XML_MAX_TAG_LEN;
							xmlParserGetValue(xmlPsStatusChildId, &maxSize, &pdlTagStr[0]);
							pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_POWER_STATUS_TYPE_E, pdlTagStr, &value);
							PDL_CHECK_STATUS(pdlStatus);
							rpsStatusType = (PDL_POWER_RPS_STATUS_ENT)value;
						}
						else if (xmlParserIsEqualName (xmlPsStatusChildId, "status-value") == XML_PARSER_RET_CODE_OK) {
							rpsStatusKey.index = 0;
							xmlStatus3 = xmlParserGetFirstChild (xmlPsStatusChildId, &xmlPsStatusValueChildId);
							 while (xmlStatus3 != XML_PARSER_RET_CODE_NOT_FOUND) {
									XML_CHECK_STATUS(xmlStatus3);
									if (xmlParserIsEqualName (xmlPsStatusValueChildId, "interface-select") == XML_PARSER_RET_CODE_OK) {
										/* skip */
									}
									else if (xmlParserIsEqualName (xmlPsStatusValueChildId, "GPIO-read-group-list") == XML_PARSER_RET_CODE_OK) {
										xmlStatus4 = xmlParserGetFirstChild (xmlPsStatusValueChildId, &xmlPsGpioListChildId);
										while (xmlStatus4 != XML_PARSER_RET_CODE_NOT_FOUND) {
											XML_CHECK_STATUS(xmlStatus3);
											if (xmlParserIsEqualName (xmlPsGpioListChildId, "read-gpio-index") == XML_PARSER_RET_CODE_OK) {
												/* skip */
											}
											else if (xmlParserIsEqualName (xmlPsGpioListChildId, "active-value") == XML_PARSER_RET_CODE_OK) {
												maxSize = sizeof (rpsStatusInterface.values.activeValue);
												xmlStatus4 = xmlParserGetValue (xmlPsGpioListChildId, &maxSize, &rpsStatusInterface.values.activeValue);
												XML_CHECK_STATUS(xmlStatus4);
											}
											else if (xmlParserIsEqualName (xmlPsGpioListChildId, "gpio-read-address") == XML_PARSER_RET_CODE_OK) {
												pdlStatus = prvPdlInterfaceXmlParser (xmlPsGpioListChildId, &rpsStatusInterface.interfaceType, &rpsStatusInterface.interfaceId);
												PDL_CHECK_STATUS(pdlStatus);
											}
											else {
												return PDL_XML_PARSE_ERROR;
											}
											xmlStatus4 = xmlParserGetNextSibling (xmlPsGpioListChildId, &xmlPsGpioListChildId);
										}
										rpsStatusInterface.key.index = rpsStatusKey.index;
										pdlStatus = prvPdlDbAdd (&pdlPowerRpsStatusDb[rpsStatusType], (void*) &rpsStatusKey, (void*) &rpsStatusInterface, (void**) &rpsStatusInterfacePtr);
										PDL_CHECK_STATUS(pdlStatus);
										rpsStatusKey.index++;
									}
									else if (xmlParserIsEqualName (xmlPsStatusValueChildId, "i2c") == XML_PARSER_RET_CODE_OK) {
										xmlStatus4 = xmlParserGetFirstChild (xmlPsStatusValueChildId, &xmlPsI2cChildId);
										while (xmlStatus4 != XML_PARSER_RET_CODE_NOT_FOUND) {
											XML_CHECK_STATUS(xmlStatus3);
											if (xmlParserIsEqualName (xmlPsI2cChildId, "active-value") == XML_PARSER_RET_CODE_OK) {
												maxSize = sizeof (rpsStatusInterface.values.activeValue);
												xmlStatus4 = xmlParserGetValue (xmlPsI2cChildId, &maxSize, &rpsStatusInterface.values.activeValue);
												XML_CHECK_STATUS(xmlStatus4);
											}
											else if (xmlParserIsEqualName (xmlPsI2cChildId, "i2c-read-write-address") == XML_PARSER_RET_CODE_OK) {
												pdlStatus = prvPdlInterfaceXmlParser (xmlPsI2cChildId, &rpsStatusInterface.interfaceType, &rpsStatusInterface.interfaceId);
												PDL_CHECK_STATUS(pdlStatus);
											}
											else {
												return PDL_XML_PARSE_ERROR;
											}
											xmlStatus4 = xmlParserGetNextSibling (xmlPsI2cChildId, &xmlPsI2cChildId);
										}
										pdlStatus = prvPdlDbAdd (&pdlPowerRpsStatusDb[rpsStatusType], (void*) &rpsStatusKey, (void*) &rpsStatusInterface, (void**) &rpsStatusInterfacePtr);
										PDL_CHECK_STATUS(pdlStatus);
										rpsStatusKey.index++;
									}
									else {
										return PDL_XML_PARSE_ERROR;
									}
								xmlStatus3 = xmlParserGetNextSibling (xmlPsStatusValueChildId, &xmlPsStatusValueChildId);
							 }
						}
						else {
							return PDL_XML_PARSE_ERROR;
						}
						xmlStatus2 = xmlParserGetNextSibling (xmlPsStatusChildId, &xmlPsStatusChildId);
					}
				}
				else {
					return PDL_XML_PARSE_ERROR;
				}
				xmlStatus = xmlParserGetNextSibling (xmlPsChildId, &xmlPsChildId);
			}
		}

    }

    xmlStatus = xmlParserFindByName (xmlId, "PoE", &xmlPoeId);
    /* section is not mandatory */
    if (xmlStatus == XML_PARSER_RET_CODE_OK) {
        XML_CHECK_STATUS(xmlStatus);

		xmlStatus = xmlParserFindByName (xmlPoeId, "POE-power-banks", &xmlPoeBankId);
		memset (pdlPowerBankCount, 0, sizeof(pdlPowerBankCount));
		if (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
			XML_CHECK_STATUS(xmlStatus);
			xmlStatus = xmlParserGetFirstChild (xmlPoeBankId, &xmlPoeBankId);
			while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
				if (xmlParserIsEqualName (xmlPoeBankId, "PoEbank-list") == XML_PARSER_RET_CODE_OK) {
					xmlStatus2 = xmlParserGetFirstChild (xmlPoeBankId, &xmlPoeBankInfoId);
					psType = PDL_POWER_PS_TYPE_NOT_SUPPORTED_E;
					while (xmlStatus2 != XML_PARSER_RET_CODE_NOT_FOUND) {
						XML_CHECK_STATUS(xmlStatus2);
						if (xmlParserIsEqualName (xmlPoeBankInfoId, "bank-number") == XML_PARSER_RET_CODE_OK) {
							maxSize = sizeof(bankNum);
							xmlStatus2 = xmlParserGetValue (xmlPoeBankInfoId, &maxSize, &bankNum);
							XML_CHECK_STATUS(xmlStatus2);
						}
						else if (xmlParserIsEqualName (xmlPoeBankInfoId, "bank-watts") == XML_PARSER_RET_CODE_OK) {
							maxSize = sizeof(bankWatts);
							xmlStatus2 = xmlParserGetValue (xmlPoeBankInfoId, &maxSize, &bankWatts);
							XML_CHECK_STATUS(xmlStatus2);
						}
						else if (xmlParserIsEqualName (xmlPoeBankInfoId, "bank-source") == XML_PARSER_RET_CODE_OK) {
							maxSize = PDL_XML_MAX_TAG_LEN;
							xmlParserGetValue(xmlPoeBankInfoId, &maxSize, &pdlTagStr[0]);
							pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_BANK_SOURCE_TYPE_E, pdlTagStr, &value);
							PDL_CHECK_STATUS(pdlStatus);
							psType = (PDL_POWER_PS_TYPE_ENT)value;
						}
						else {
							PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlPoeBankInfoId POINTER OR NAME DON'T MATCH TO: bank-watts");
							return PDL_XML_PARSE_ERROR;
						}
						xmlStatus2 = xmlParserGetNextSibling (xmlPoeBankInfoId, &xmlPoeBankInfoId);
					}
					pdlPowerBankDb[psType][bankNum] = bankWatts;
					pdlPowerBankCount[psType]++;
				}
				else {
					PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlPoeBankId POINTER OR NAME DON'T MATCH TO: PoEbank");
					return PDL_XML_PARSE_ERROR;
				}
				xmlStatus = xmlParserGetNextSibling (xmlPoeBankId, &xmlPoeBankId);
			}
		}

		xmlStatus = xmlParserFindByName (xmlPoeId, "PSEports", &xmlPsePortsId);
		if (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
			XML_CHECK_STATUS(xmlStatus);
			xmlStatus = xmlParserGetFirstChild (xmlPsePortsId, &xmlPsePortId);
            while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
                if (xmlParserIsEqualName (xmlPsePortId, "PSE-information") == XML_PARSER_RET_CODE_OK) {
				    maxSize = PDL_XML_MAX_TAG_LEN;
                    xmlParserGetValue(xmlPsePortId, &maxSize, &pdlTagStr[0]);
				    pdlStatus = pdlLibConvertEnumStringToValue (XML_PARSER_ENUM_ID_PSE_TYPE_E, pdlTagStr, &value);
				    PDL_CHECK_STATUS(pdlStatus);
                    pdlPowerCapabilites.pseType = (PDL_POWER_PSE_TYPE_ENT)value;
                }
                else if (xmlParserIsEqualName (xmlPsePortId, "PSE-list") == XML_PARSER_RET_CODE_OK) {
                    xmlStatus = xmlParserGetFirstChild (xmlPsePortId, &xmlPseListInfoId);
			        while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
				        memset (&pseListKey, 0, sizeof(pseListKey));
				        memset (&pseListInfo, 0, sizeof (pseListInfo));
                        if (xmlParserIsEqualName (xmlPseListInfoId, "PSE-number") == XML_PARSER_RET_CODE_OK) {
						    maxSize = sizeof(pseListKey);
						    xmlStatus = xmlParserGetValue (xmlPseListInfoId, &maxSize, &pseListKey.number);
						    XML_CHECK_STATUS(xmlStatus);
                        }
                        else if (xmlParserIsEqualName (xmlPseListInfoId, "PSE-address") == XML_PARSER_RET_CODE_OK) {
						    maxSize = sizeof(pseListInfo);
						    xmlStatus = xmlParserGetValue (xmlPseListInfoId, &maxSize, &pseListInfo.pseAddress);
						    XML_CHECK_STATUS(xmlStatus);
                        }
					    else {
						    PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"INVALID PSE-list tag");
						    return PDL_XML_PARSE_ERROR;
					    }
                        xmlStatus = xmlParserGetNextSibling (xmlPseListInfoId, &xmlPseListInfoId);
                    }
				    pdlStatus = prvPdlDbAdd (&pdlPowerPseListDb, (void*) &pseListKey, (void*) &pseListInfo, (void**) &pseListPtr);
				    PDL_CHECK_STATUS(pdlStatus);
                }
                else if (xmlParserIsEqualName (xmlPsePortId, "PSEport-list") == XML_PARSER_RET_CODE_OK) {
			        while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
				        memset (&portInfo, 0, sizeof(portInfo));
				        memset (&portKey, 0, sizeof (portKey));
				        XML_CHECK_STATUS(xmlStatus);
				        xmlStatus = xmlParserGetFirstChild (xmlPsePortId, &xmlPsePortInfoId);
				        while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
					        XML_CHECK_STATUS(xmlStatus);
					        if (xmlParserIsEqualName (xmlPsePortInfoId, "panel-group-number") == XML_PARSER_RET_CODE_OK) {
						        /* TODO: panel or dev/port */
						        frontPanelGroup = 0;
						        maxSize = sizeof(frontPanelGroup);
						        xmlStatus = xmlParserGetValue (xmlPsePortInfoId, &maxSize, &frontPanelGroup);
						        XML_CHECK_STATUS(xmlStatus);
						        mask |= 1;
					        }
					        else if (xmlParserIsEqualName (xmlPsePortInfoId, "panel-port-number") == XML_PARSER_RET_CODE_OK) {
						        /* TODO: panel or dev/port */
						        frontPanelPort = 0;
						        maxSize = sizeof(frontPanelPort);
						        xmlStatus = xmlParserGetValue (xmlPsePortInfoId, &maxSize, &frontPanelPort);
						        XML_CHECK_STATUS(xmlStatus);
						        mask |= 1 << 1;
					        }
					        else if (xmlParserIsEqualName (xmlPsePortInfoId, "pse-group") == XML_PARSER_RET_CODE_OK) {
							        xmlStatus = xmlParserGetFirstChild (xmlPsePortInfoId, &xmlPseGroupId);
							        while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
								        if (xmlParserIsEqualName (xmlPseGroupId, "activeInfo") == XML_PARSER_RET_CODE_OK) {
									        pdlStatus = prvPdlPowerXMLActiveInfoParser (xmlPseGroupId, &portInfo.pseInfo.interfaceInfo.interfaceType,
																							           &portInfo.pseInfo.interfaceInfo.InterfaceId,
																							           &portInfo.pseInfo.interfaceInfo.value.activeValue);
									        PDL_CHECK_STATUS(pdlStatus);
								        }
								        else if (xmlParserIsEqualName (xmlPseGroupId, "port-type") == XML_PARSER_RET_CODE_OK) {
									        maxSize = PDL_XML_MAX_TAG_LEN;
									        xmlParserGetValue(xmlPseGroupId, &maxSize, &pdlTagStr[0]);
									        pdlStatus = pdlLibConvertEnumStringToValue (XML_PARSER_ENUM_ID_PSEPORT_TYPE_E, pdlTagStr, &value);
									        PDL_CHECK_STATUS(pdlStatus);
									        portInfo.pseInfo.port_power_type = (PDL_POWER_PORT_TYPE_ENT)value;
								        }
								        else if (xmlParserIsEqualName (xmlPseGroupId, "index1") == XML_PARSER_RET_CODE_OK) {
									        maxSize = sizeof(portInfo.pseInfo.index1);
									        xmlStatus = xmlParserGetValue (xmlPseGroupId, &maxSize, &portInfo.pseInfo.index1);
									        XML_CHECK_STATUS(xmlStatus);
								        }
								        else if (xmlParserIsEqualName (xmlPseGroupId, "index2") == XML_PARSER_RET_CODE_OK) {
									        maxSize = sizeof(portInfo.pseInfo.index2);
									        xmlStatus = xmlParserGetValue (xmlPseGroupId, &maxSize, &portInfo.pseInfo.index2);
									        XML_CHECK_STATUS(xmlStatus);
								        }
								        else if (xmlParserIsEqualName (xmlPseGroupId, "logical-port") == XML_PARSER_RET_CODE_OK) {
									        maxSize = sizeof(portInfo.pseInfo.logicalPort);
									        xmlStatus = xmlParserGetValue (xmlPseGroupId, &maxSize, &portInfo.pseInfo.logicalPort);
									        XML_CHECK_STATUS(xmlStatus);
								        }
								        else {
									        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlPsePortInfoId POINTER OR NAME DON'T MATCH");
									        return PDL_XML_PARSE_ERROR;
								        }
								        xmlStatus = xmlParserGetNextSibling (xmlPseGroupId, &xmlPseGroupId);
							        }
					        }
					        xmlStatus = xmlParserGetNextSibling (xmlPsePortInfoId, &xmlPsePortInfoId);
				        }
				        /* key values are Mandatory */
				        if (mask != 3) {
						        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"mask != 3");
					        return PDL_XML_PARSE_ERROR;
				        }
				        pdlStatus = pdlPpPortConvertFrontPanel2DevPort (frontPanelGroup, frontPanelPort, &portKey.dev, &portKey.logicalPort);
				        PDL_CHECK_STATUS(pdlStatus);
				        psePorts++;
				        pdlStatus = prvPdlDbAdd (&pdlPowerPortDb, (void*) &portKey, (void*) &portInfo, (void**) &portPtr);
				        PDL_CHECK_STATUS(pdlStatus);
			        }
                }
                else {
                    PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"INVALID tag in <PSEports>");
                    return PDL_XML_PARSE_ERROR;
                }
                xmlStatus = xmlParserGetNextSibling (xmlPsePortId, &xmlPsePortId);
            }
		}

		xmlStatus = xmlParserFindByName (xmlPoeId, "PDports", &xmlPdPortsId);
		if (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
			XML_CHECK_STATUS(xmlStatus);
			xmlStatus = xmlParserGetFirstChild (xmlPdPortsId, &xmlPdPortId);
			while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
				memset (&portInfo, 0, sizeof(portInfo));
				XML_CHECK_STATUS(xmlStatus);
				xmlStatus = xmlParserGetFirstChild (xmlPdPortId, &xmlPdPortInfo);
				while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
					XML_CHECK_STATUS(xmlStatus);
					if (xmlParserIsEqualName (xmlPdPortInfo, "panel-group-number") == XML_PARSER_RET_CODE_OK) {
						/* TODO: panel or dev/port */
						maxSize = sizeof(portKey.dev);
						xmlStatus = xmlParserGetValue (xmlPdPortInfo, &maxSize, &portKey.dev);
						XML_CHECK_STATUS(xmlStatus);
						mask |= 1;
					}
					else if (xmlParserIsEqualName (xmlPdPortInfo, "panel-port-number") == XML_PARSER_RET_CODE_OK) {
						/* TODO: panel or dev/port */
						maxSize = sizeof(portKey.logicalPort);
						xmlStatus = xmlParserGetValue (xmlPdPortInfo, &maxSize, &portKey.logicalPort);
						XML_CHECK_STATUS(xmlStatus);
						mask |= 1 << 1;
					}
					else if (xmlParserIsEqualName (xmlPdPortInfo, "pd-group") == XML_PARSER_RET_CODE_OK) {
							xmlStatus = xmlParserGetFirstChild (xmlPdPortInfo, &xmlPdGroupId);
							while (xmlStatus != XML_PARSER_RET_CODE_NOT_FOUND) {
								if (xmlParserIsEqualName (xmlPdGroupId, "ATinfo") == XML_PARSER_RET_CODE_OK) {
									pdlStatus = prvPdlPowerPdInfoParser (xmlPdGroupId, &portInfo.pdPortInterface.atInfo.interfaceType,
																		 &portInfo.pdPortInterface.atInfo.interfaceId, &portInfo.pdPortInterface.values.atValue);
									PDL_CHECK_STATUS(pdlStatus);
								}
								else if (xmlParserIsEqualName (xmlPdGroupId, "AFinfo") == XML_PARSER_RET_CODE_OK) {
									pdlStatus = prvPdlPowerPdInfoParser (xmlPdGroupId, &portInfo.pdPortInterface.afInfo.interfaceType,
																		 &portInfo.pdPortInterface.afInfo.interfaceId, &portInfo.pdPortInterface.values.afValue);
									PDL_CHECK_STATUS(pdlStatus);
								}
								else if (xmlParserIsEqualName (xmlPdGroupId, "sixstyWinfo") == XML_PARSER_RET_CODE_OK) {
									pdlStatus = prvPdlPowerPdInfoParser (xmlPdGroupId, &portInfo.pdPortInterface.sixtyWattInfo.interfaceType,
																		 &portInfo.pdPortInterface.sixtyWattInfo.interfaceId, &portInfo.pdPortInterface.values.sixtyWattValue);
									PDL_CHECK_STATUS(pdlStatus);
								}
								else if (xmlParserIsEqualName (xmlPdGroupId, "clearControl") == XML_PARSER_RET_CODE_OK) {
									pdlStatus = prvPdlPowerPdInfoParser (xmlPdGroupId, &portInfo.pdPortInterface.clearControl.interfaceType,
																		 &portInfo.pdPortInterface.clearControl.interfaceId, &portInfo.pdPortInterface.values.clearControlValue);
									PDL_CHECK_STATUS(pdlStatus);
								}
								else if (xmlParserIsEqualName (xmlPdGroupId, "ATcontrol") == XML_PARSER_RET_CODE_OK) {
									pdlStatus = prvPdlPowerPdInfoParser (xmlPdGroupId, &portInfo.pdPortInterface.atControl.interfaceType,
																		 &portInfo.pdPortInterface.atControl.interfaceId, &portInfo.pdPortInterface.values.atControlValue);
									PDL_CHECK_STATUS(pdlStatus);
								}
								else if (xmlParserIsEqualName (xmlPdGroupId, "AFcontrol") == XML_PARSER_RET_CODE_OK) {
									pdlStatus = prvPdlPowerPdInfoParser (xmlPdGroupId, &portInfo.pdPortInterface.afControl.interfaceType,
																		 &portInfo.pdPortInterface.afControl.interfaceId, &portInfo.pdPortInterface.values.afControlValue);
									PDL_CHECK_STATUS(pdlStatus);
								}
								else if (xmlParserIsEqualName (xmlPdGroupId, "sixstycontrol") == XML_PARSER_RET_CODE_OK) {
									pdlStatus = prvPdlPowerPdInfoParser (xmlPdGroupId, &portInfo.pdPortInterface.sixtyWattControl.interfaceType,
																		 &portInfo.pdPortInterface.sixtyWattControl.interfaceId, &portInfo.pdPortInterface.values.sixtyWattControlValue);
									PDL_CHECK_STATUS(pdlStatus);
								}
								else {
									PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlPdGroupId POINTER OR NAME DON'T MATCH TO: _60Winfo");
									return PDL_XML_PARSE_ERROR;
								}
								xmlStatus = xmlParserGetNextSibling (xmlPdGroupId, &xmlPdGroupId);
							}
					}
					else {
						PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__,"INVALID xmlPdPortInfo POINTER OR NAME DON'T MATCH TO: pd-group");
						return PDL_XML_PARSE_ERROR;
					}
				xmlStatus = xmlParserGetNextSibling (xmlPdPortInfo, &xmlPdPortInfo);
				}
				/* key values are Mandatory */
				if (mask != 3) {
					return PDL_XML_PARSE_ERROR;
				}
				/* check if some port information already exists */
				pdlStatus = prvPdlDbFind(&pdlPowerPortDb, (void*) &portKey, (void**) &portPtr);

				if (pdlStatus == PDL_NOT_FOUND) {
					pdPorts++;
					pdlStatus = prvPdlDbAdd (&pdlPowerPortDb, (void*) &portKey, (void*) &portInfo, (void**) &portPtr);
					PDL_CHECK_STATUS(pdlStatus);
				}
				else {
					memcpy (&portPtr->pdPortInterface, &portInfo.pdPortInterface, sizeof(portInfo.pdPortInterface));
				}
				xmlStatus = xmlParserGetNextSibling (xmlPdPortId, &xmlPdPortId);
			}
		}
		pdlPowerCapabilites.numberOfPdPorts = pdPorts;
		pdlPowerCapabilites.numberOfPsePorts = psePorts;
	}
    return PDL_OK;   
}

/*$ END OF prvPdlPowerXMLParser */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerInit
*
* DESCRIPTION:   Init power module
*                Create button DB and initialize
*
* PARAMETERS:   
*                
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl power initialize
 *
 * @param   xmlId   Identifier for the XML.
 *
 * @return  A PDL_STATUS.
 */

/**
 * @fn  PDL_STATUS pdlPowerDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Power debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDebugSet (
    IN  BOOLEAN             state
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/    
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    prvPdlPowerDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlPowerDebugSet */


PDL_STATUS pdlPowerInit (
    IN  XML_PARSER_NODE_DESCRIPTOR_TYP      xmlId
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                         i, j;
    PDL_STATUS                      pdlStatus;
    PDL_DB_PRV_ATTRIBUTES_STC       dbAttributes;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    dbAttributes.listAttributes.entrySize = sizeof(PDL_POWER_PORT_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_POWER_PORT_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlPowerPortDb);
    PDL_CHECK_STATUS(pdlStatus);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_POWER_PSE_LIST_INFO_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_POWER_PSE_LIST_PRV_KEY_STC);
    pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                &dbAttributes,
                                &pdlPowerPseListDb);
    PDL_CHECK_STATUS(pdlStatus);
    
    memset (&pdlPowerCapabilites, 0, sizeof(pdlPowerCapabilites));

    for (i = 0; i < PDL_POWER_PS_TYPE_LAST_E; i++) {
        for (j = 0; j <PDL_POWER_BANK_MAX; j++) {
            pdlPowerBankDb[i][j] = PDL_POWER_BANK_INVALID;
        }
    }

    /* create DB for RPS status types */
    dbAttributes.listAttributes.entrySize = sizeof(PDL_POWER_RPS_STATUS_PRV_INTERFACE_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_POWER_RPS_STATUS_PRV_KEY_STC);
    for (i = 0; i < PDL_POWER_RPS_STATUS_LAST_E; i++) {
        pdlStatus = prvPdlDbCreate (PDL_DB_PRV_TYPE_LIST_E,
                                    &dbAttributes,
                                    &pdlPowerRpsStatusDb[i]);
        PDL_CHECK_STATUS(pdlStatus);        
    }

    pdlStatus = prvPdlPowerXMLParser (xmlId);
    PDL_CHECK_STATUS(pdlStatus);
    
    return PDL_OK;
}

/*$ END OF pdlPowerInit */

/**
 * @fn  PDL_STATUS prvPdlPowerDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPowerDestroy (
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    UINT_32                                 i;
    PDL_STATUS                              pdlStatus;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (pdlPowerPortDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlPowerPortDb);
        PDL_CHECK_STATUS(pdlStatus);    
    }
    if (pdlPowerPseListDb.dbPtr != NULL) {
        pdlStatus = prvPdlDbDestroy (&pdlPowerPseListDb);
        PDL_CHECK_STATUS(pdlStatus);    
    }
    for (i = 0; i < PDL_POWER_RPS_STATUS_LAST_E; i++) {
        if (pdlPowerRpsStatusDb[i].dbPtr != NULL) {
            pdlStatus = prvPdlDbDestroy (&pdlPowerRpsStatusDb[i]);
            PDL_CHECK_STATUS(pdlStatus);    
        }
    }
    return PDL_OK;
}

/*$ END OF prvPdlPowerDestroy */
