/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\power_supply\pdlpower.h.
 *
 * @brief   Declares the pdlpower class
 */

#ifndef __pdlPowerh

#define __pdlPowerh
/**
********************************************************************************
 * @file pdlPower.h   
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
 * @brief Platform driver layer - power supply related API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>

#define PDL_POWER_BANK_MAX                  16 

/** 
 * @defgroup Power_Supply Power Supply
 * @{Sensor driver definitions and declarations including:
 * - Temprature get
*/

/**
 * @enum    PDL_POWER_POE_CAPABILITY_ENT
 *
 * @brief   Enumerator for power poe capability
 */

typedef enum {
    PDL_POWER_POE_CAPABILITY_NO_E,
    PDL_POWER_POE_CAPABILITY_PSE_E,
    PDL_POWER_POE_CAPABILITY_PD_E
}PDL_POWER_POE_CAPABILITY_ENT;

/**
 * @enum    PDL_POWER_STATUS_ENT
 *
 * @brief   Enumerator for power supply status
 */

typedef enum {
    PDL_POWER_STATUS_NOT_ACTIVE_E,
    PDL_POWER_STATUS_ACTIVE_E
}PDL_POWER_STATUS_ENT;

typedef enum {
    PDL_POWER_RPS_STATUS_STANDBY_E,
    PDL_POWER_RPS_STATUS_NOTAVAILABLE_E,
    PDL_POWER_RPS_STATUS_BACKUP_E,
    PDL_POWER_RPS_STATUS_READY_E,
    PDL_POWER_RPS_STATUS_LAST_E,
}PDL_POWER_RPS_STATUS_ENT;

/**
 * @enum    PDL_POWER_PS_TYPE_ENT
 *
 * @brief   Enumerator for power supply type (main &amp; redundant)
 */

typedef enum {
    PDL_POWER_PS_TYPE_PS_E,
    PDL_POWER_PS_TYPE_RPS_E,
    PDL_POWER_PS_TYPE_PD_E,
    PDL_POWER_PS_TYPE_NOT_SUPPORTED_E,
    PDL_POWER_PS_TYPE_LAST_E,
}PDL_POWER_PS_TYPE_ENT;

/**
 * @enum    PDL_POWER_PORT_TYPE_ENT
 *
 * @brief   Enumerator for power port type
 */

typedef enum {
    PDL_POWER_PORT_TYPE_AF_E,
    PDL_POWER_PORT_TYPE_AT_E,
    PDL_POWER_PORT_TYPE_60W_E,
    PDL_POWER_PORT_TYPE_BT_E,
    PDL_POWER_PORT_TYPE_LAST_E,
}PDL_POWER_PORT_TYPE_ENT;

typedef enum {
    PDL_POWER_PSE_TYPE_UNDEFINED_E,
    PDL_POWER_PSE_TYPE_690XX_E,
    PDL_POWER_PSE_TYPE_691XX_E,
    PDL_POWER_PSE_TYPE_692XX_E,
    PDL_POWER_PSE_TYPE_2388XX_E,
    PDL_POWER_PSE_TYPE_692BT_E,
} PDL_POWER_PSE_TYPE_ENT;

/**
 * @struct  PDL_POWER_PS_VALUES_STC
 *
 * @brief   defines structure that hold all device specific values
 */

typedef struct {
    /** @brief   The active value */
    UINT_32                               activeValue;
} PDL_POWER_PS_VALUES_STC;

/**
 * @struct  PDL_POWER_PS_INTERFACE_STC
 *
 * @brief   defines interface for working with power supply
 */

typedef struct {
    /** @brief   Type of the interface */
    PDL_INTERFACE_TYPE_ENT          interfaceType;    
    /** @brief   Identifier for the status interface */
    PDL_INTERFACE_TYP               interfaceId;
    /** @brief   The values */
    PDL_POWER_PS_VALUES_STC         values;
} PDL_POWER_PS_INTERFACE_STC;

/**
 * @struct  PDL_POWER_XML_CAPABILITIES_STC
 *
 * @brief   defines what is supported by power supply
 */

typedef struct {
    /** @brief   True if rps is supported */
    BOOLEAN                     RpsIsSupported; 
    /** @brief   The ps number of banks */
    UINT_32                     PsNumberOfBanks;
    /** @brief   The rps number of banks */
    UINT_32                     RpsNumberOfBanks;
    /** @brief   Number of pse ports */
    UINT_32                     numberOfPsePorts;
    PDL_POWER_PSE_TYPE_ENT      pseType;
    /** @brief   Number of pd ports */
    UINT_32                     numberOfPdPorts;
} PDL_POWER_XML_CAPABILITIES_STC;

/**
 * @typedef UINT_32 PDL_POWER_PSE_INDEX_TYP
 *
 * @brief  PSE index type
 */

/**
 * @typedef UINT_32 PDL_POWER_PSE_INDEX_TYP
 *
 * @brief   Defines an alias representing the pdl power pse index typ
 */

typedef UINT_32 PDL_POWER_PSE_INDEX_TYP;

/**
 * @typedef UINT_32 PDL_power_bank_number_TYP
 *
 * @brief  PSE power bank number type
 */

/**
 * @typedef UINT_32 PDL_POWER_BANK_NUMBER_TYP
 *
 * @brief   Defines an alias representing the pdl power bank number typ
 */

typedef UINT_8 PDL_POWER_BANK_NUMBER_TYP;

/**
 * @typedef UINT_32 PDL_power_bank_watts_TYP
 *
 * @brief  PSE power bank watts type
 */

/**
 * @typedef UINT_32 PDL_POWER_BANK_WATTS_TYP
 *
 * @brief   Defines an alias representing the pdl power bank watts typ
 */

typedef UINT_16 PDL_POWER_BANK_WATTS_TYP;

/**
 * @struct  PDL_POWER_PSE_PORT_VALUES_STC
 *
 * @brief   defines structure that hold port PSE information
 */

typedef struct {
    /** @brief   The active value */
    UINT_32                               activeValue;
} PDL_POWER_PSE_PORT_VALUES_STC;

/**
 * @struct  PDL_POWER_PORT_PSE_INTERFACE_STC
 *
 * @brief   defines interface for working with power supply
 */

typedef struct {
    /** @brief   Type of the interface */
    PDL_INTERFACE_TYPE_ENT          interfaceType;    
    /** @brief   Identifier for the interface */
    PDL_INTERFACE_TYP               InterfaceId;
    /** @brief   The value */
    PDL_POWER_PSE_PORT_VALUES_STC   value;
} PDL_POWER_PORT_PSE_INTERFACE_STC;

/**
 * @struct  PDL_POWER_PSE_PORT_INFO_STC
 *
 * @brief   PSE port info
 */

typedef struct {
    /** @brief   Type of the port power */
    PDL_POWER_PORT_TYPE_ENT             port_power_type;
    /** @brief   The first index */
    PDL_POWER_PSE_INDEX_TYP             index1;
    /** @brief   The second index */
    PDL_POWER_PSE_INDEX_TYP             index2;
    /** @brief   Information describing the interface */
    PDL_POWER_PORT_PSE_INTERFACE_STC    interfaceInfo;
    UINT_32                             logicalPort;
} PDL_POWER_PSE_PORT_INFO_STC;

/**
 * @struct  PDL_POWER_PD_PORT_VALUES_STC
 *
 * @brief   defines structure that hold all pd port specific values
 */

typedef struct {
    UINT_32                             atValue; 
    UINT_32                             afValue;
    UINT_32                             sixtyWattValue;
    UINT_32                             clearControlValue; 
    UINT_32                             atControlValue; 
    UINT_32                             afControlValue;
    UINT_32                             sixtyWattControlValue;

} PDL_POWER_PD_PORT_VALUES_STC;

/**
 * @struct  PDL_POWER_PD_PORT_INTERFACE_STC
 *
 * @brief   defines interface for working with power supply
 */

typedef struct {
    struct {
        PDL_INTERFACE_TYPE_ENT                interfaceType;    
        PDL_INTERFACE_TYP                     interfaceId;
    } atInfo;
    struct {
        PDL_INTERFACE_TYPE_ENT                interfaceType;    
        PDL_INTERFACE_TYP                     interfaceId;
    } afInfo;
    struct {
        PDL_INTERFACE_TYPE_ENT                interfaceType;    
        PDL_INTERFACE_TYP                     interfaceId;
    } sixtyWattInfo;
    struct {
        PDL_INTERFACE_TYPE_ENT                interfaceType;    
        PDL_INTERFACE_TYP                     interfaceId;
    } clearControl;
    struct {
        PDL_INTERFACE_TYPE_ENT                interfaceType;    
        PDL_INTERFACE_TYP                     interfaceId;
    } atControl;
    struct {
        PDL_INTERFACE_TYPE_ENT                interfaceType;    
        PDL_INTERFACE_TYP                     interfaceId;
    } afControl;
    struct {
        PDL_INTERFACE_TYPE_ENT                interfaceType;    
        PDL_INTERFACE_TYP                     interfaceId;
    } sixtyWattControl;


    /** @brief   The values */
    PDL_POWER_PD_PORT_VALUES_STC              values;

} PDL_POWER_PD_PORT_INTERFACE_STC;

typedef struct {
    UINT_8                              pseAddress;
} PDL_POWER_PSE_LIST_INFO_STC;


/**
 * @fn  PDL_STATUS pdlPowerDbCapabilitiesGet ( OUT PDL_POWER_XML_CAPABILITIES_STC * capabilitiesPtr );
 *
 * @brief   Get \b power supported capabilities
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [out] capabilitiesPtr - device power capabilities.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPowerDbCapabilitiesGet (
    OUT PDL_POWER_XML_CAPABILITIES_STC *  capabilitiesPtr
);

/**
 * @fn  PDL_STATUS pdlPowerHwStatusGet ( IN PDL_POWER_PS_TYPE_ENT type, OUT PDL_POWER_STATUS_ENT * statusPtr );
 *
 * @brief   Get power status of the power supply
 *
 * @param [in]  type        - device power type (main or redundant)
 * @param [out] statusPtr   - power supply status.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPowerHwStatusGet (
    IN  PDL_POWER_PS_TYPE_ENT         type,
    OUT PDL_POWER_STATUS_ENT        * statusPtr,
    OUT PDL_POWER_RPS_STATUS_ENT    * rpsStatusPtr
);

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
);

/**
 * @fn  PDL_STATUS pdlPowerDbBankWattsGet ( IN PDL_POWER_BANK_NUMBER_TYP bankNumber, OUT PDL_POWER_BANK_WATTS_TYP * bankWattsPtr );
 *
 * @brief   Get Watts provided by the given bank according to XML
 *
 * @param [in]  type            - device power type (main or redundant)
 * @param [in]  bankNumber      - power bank number.
 * @param [out] bankWattsPtr    - Watts provided.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPowerDbBankWattsGet (
    IN  PDL_POWER_PS_TYPE_ENT        type,
    IN  PDL_POWER_BANK_NUMBER_TYP    bankNumber,
    OUT PDL_POWER_BANK_WATTS_TYP   * bankWattsPtr
);

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
);

/**
 * @fn  PDL_STATUS pdlPowerDbPortCapabilityGet ( IN UINT_32 device, IN UINT_32 logicalPort, OUT PDL_POWER_POE_CAPABILITY_ENT * capabilityPtr );
 *
 * @brief   Get port power capability according to XML
 *
 * @param [in]  device          .
 * @param [in]  port            .
 * @param [out] capabilityPtr   - PSE,PD, NON poe port.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPowerDbPortCapabilityGet (
    IN  UINT_32                         device,
    IN  UINT_32                         logicalPort,
    OUT PDL_POWER_POE_CAPABILITY_ENT  * capabilityPtr
);

/**
 * @fn  PDL_STATUS pdlPowerDbPsePortTypeGet ( IN UINT_32 device, IN UINT_32 logicalPort, OUT PDL_POWER_PSE_PORT_INFO_STC * psePortinfoPtr );
 *
 * @brief   Get port power info according to XML
 *
 * @param [in]  device          .
 * @param [in]  port            .
 * @param [out] psePortinfoPtr  - port type and HW indexes mapping.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPowerDbPsePortTypeGet (
    IN  UINT_32                         device,
    IN  UINT_32                         logicalPort,
    OUT PDL_POWER_PSE_PORT_INFO_STC   * psePortinfoPtr
);

/**
 * @fn  PDL_STATUS pdlPowerPdClear ( IN UINT_32 device, IN UINT_32 port );
 *
 * @brief   clears a PD port
 *
 * @param [in]  device  .
 * @param [in]  port    .
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPowerPdClear (
    IN  UINT_32                     device,
    IN  UINT_32                     logicalPort
);

/**
 * @fn  PDL_STATUS pdlPowerPdSet ( IN UINT_32 device, IN UINT_32 logicalPort, IN PDL_POWER_PORT_TYPE_ENT powerType );
 *
 * @brief   Pdl power pd set
 *
 * @param [in]  device      .
 * @param [in]  port        .
 * @param [in]  powerType   - AF,AT,60W.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPowerPdSet (
    IN  UINT_32                     device,
    IN  UINT_32                     logicalPort,
    IN  PDL_POWER_PORT_TYPE_ENT     powerType
);

/* ***************************************************************************
* FUNCTION NAME: pdlPowerPseAddressGet
*
* DESCRIPTION:   
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlPowerPseAddressGet (
    IN  UINT_8                      pseNumber,
    OUT UINT_8                    * pseAddressPtr
);

/**
* @fn  PDL_STATUS pdlPowerDbNumOfPsesGet ( OUT UINT_32  *numOfPsesPtrr );
 *
 * @brief   Pdl get number of PSEs devices
 *
 * @param [OUT] *numOfPsesPtrr   .
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlPowerDbNumOfPsesGet (
    OUT  UINT_32    *  numOfPsesPtr
);

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
);

/* @}*/

#endif
