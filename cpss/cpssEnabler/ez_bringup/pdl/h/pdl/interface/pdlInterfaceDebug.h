/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\interface\pdlinterfaceDebug.h.
 *
 * @brief   Declares the pdlinterfaceDebug class
 */

#ifndef __PdlInterfaceDebugh
#define __PdlInterfaceDebugh
/**
********************************************************************************
 * @file pdlInterfaceDebug.h  
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
 * @brief Platform driver layer - Interface related API (internal)
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>

/** 
 * @defgroup Interface Interface
 * @{defines access method, size, address (including indirect), mask & good value. \n
 *  Supporting:
 * - I2C
 * - SMI/XSMI
 * - Memory mapped access
*/



/****************************** Enum definitions ********************************/

typedef UINT_32 PDL_INTERFACE_TYP;      /* interfaces are stored according to interfaceId in relevant DB (i2c/gpio/smi/xsmi, etc.)*/
typedef UINT_32 PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP;


/**
 * @enum    PDL_INTERFACE_TYPE_ENT
 *
 * @brief   Enumeration of interface type types.
 */

typedef enum {
/**
 *   @brief I2C that is connected to CPU  
 */
    PDL_INTERFACE_TYPE_I2C_E = 1,
/**
 *   @brief SMI that is connected to PP
 */
    PDL_INTERFACE_TYPE_SMI_E,
/**
 *   @brief XSMI that is connected to PP
 */
    PDL_INTERFACE_TYPE_XSMI_E,
/**
 *   @brief Memory mapped, used for: multi purpose pins, PCI/PEX ext... 
 */
    PDL_INTERFACE_TYPE_GPIO_E,
/**
 *   @brief SMI that is connected to CPU (OOB port for example) 
 */
    PDL_INTERFACE_TYPE_SMI_CPU_E,
    PDL_INTERFACE_TYPE_EXTERNAL_E,
    PDL_INTERFACE_TYPE_LAST_E,
}PDL_INTERFACE_TYPE_ENT;

/**
 * @enum    PDL_INTERFACE_GPIO_TYPE_ENT
 *
 * @brief   Enumeration GPIO interface type.
 */

typedef enum {
    PDL_INTERFACE_GPIO_TYPE_READ_E,
    PDL_INTERFACE_GPIO_TYPE_WRITE_E,
    PDL_INTERFACE_GPIO_TYPE_READ_WRITE_E,
    PDL_INTERFACE_GPIO_TYPE_LAST_E
}PDL_INTERFACE_GPIO_TYPE_ENT;


/**
 * @enum    PDL_INTERFACE_GPIO_OFFSET_ENT
 *
 * @brief   Enumeration GPIO interface offset.
 */

typedef enum {
    PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E = 0,
    PDL_INTERFACE_GPIO_OFFSET_TYPE_E = 4,
    PDL_INTERFACE_GPIO_OFFSET_BLINKING_E = 8,
    PDL_INTERFACE_GPIO_OFFSET_INPUT_E = 16,
}PDL_INTERFACE_GPIO_OFFSET_ENT;

 /**
  * @struct PDL_INTERFACE_GPIO_STC
  *
  * @brief  use in order to define an memory mapped interface \n mainly used for MPP/GPIO but can
  *         also be used for memory mapped PCI/PEX
  */

 typedef struct {
    UINT_8                             dev;                /* 255 stands for CPU GPIO */
    UINT_8                             pinNumber;          /* which bit is relevant for this GPIO value read */
    PDL_INTERFACE_GPIO_TYPE_ENT        type;               /* used to initialize mpp access */
    UINT_8                             initialValue;
    UINT_8                             pushValue;          /* used by GPIO muxes ONLY */
}PDL_INTERFACE_GPIO_STC;


 /**
 * @enum    PDL_INTERFACE_I2C_ACCESS_SIZE_ENT
 *
 * @brief   I2C access size in bytes.
 */

typedef enum {
    PDL_INTERFACE_I2C_ACCESS_SIZE_1_BYTE_E = 1,
    PDL_INTERFACE_I2C_ACCESS_SIZE_2_BYTE_E = 2,
    PDL_INTERFACE_I2C_ACCESS_SIZE_4_BYTE_E = 4
}PDL_INTERFACE_I2C_ACCESS_SIZE_ENT;

typedef enum {
    PDL_INTERFACE_I2C_TRANSACTION_TYPE_OFFSET_AND_DATA,
    PDL_INTERFACE_I2C_TRANSACTION_TYPE_OFFSET_THEN_DATA
}PDL_INTERFACE_I2C_TRANSACTION_TYPE_ENT;

 /**
  * @struct PDL_INTERFACE_I2C_STC
  *
  * @brief  I2C access information \n this structure contains all that is required in order to
  *         perform a full (logical) I2C transaction.
  */

 typedef struct {
    /** @brief   Identifier for the bus */
    UINT_32	                                    busId;
    /** @brief   Zero-based index of the 2c address */
    UINT_8                                      i2cAddress;    /* slave address */
    /** @brief   Size of the access */
    PDL_INTERFACE_I2C_ACCESS_SIZE_ENT           accessSize;
    /** @brief   The offset */
    UINT_16                                     offset;
    /** @brief   The mask */
    UINT_32	                                    mask;
    /** @brief   Number of muxes */
    UINT_32			                            numberOfMuxes;
    PDL_INTERFACE_I2C_TRANSACTION_TYPE_ENT      transactionType;
} PDL_INTERFACE_I2C_STC;

/**
 * @struct  PDL_INTERFACE_I2C_MUX_STC
 *
 * @brief   I2C MUX definition write "connection" in order to "activate" this MUX
 */

typedef struct {
    /** @brief   Identifier for the bus */
    UINT_32	                                busId;
    /** @brief   Zero-based index of the I2c address */
    UINT_8                                  baseAddress;    /* slave address */
    /** @brief   Size of the access */
    PDL_INTERFACE_I2C_ACCESS_SIZE_ENT       accessSize;
    /** @brief   The offset */
    UINT_16                                 offset;
    /** @brief   The mask */
    UINT_32	                                mask;
    /** @brief   The value */
    UINT_32	                                value;
    PDL_INTERFACE_I2C_TRANSACTION_TYPE_ENT  transactionType;
} PDL_INTERFACE_I2C_MUX_STC;

/**
 * @struct  PDL_INTERFACE_MUX_STC
 *
 * @brief   MUX definition write "connection" in order to "activate" this MUX
 */

typedef struct {
    /** @brief   Type of the interface */
    PDL_INTERFACE_TYPE_ENT                  interfaceType;    

    /**
     * @union   Unnamed
     *
     * @brief   An unnamed.
     */

    union {
        /** @brief   The gpio interface */
        PDL_INTERFACE_GPIO_STC              gpioInterface;
        /** @brief   Zero-based index of the 2c multiplexer */
        PDL_INTERFACE_I2C_MUX_STC           i2cMux;
    }interfaceInfo;
} PDL_INTERFACE_MUX_STC;

typedef struct {
    PDL_INTERFACE_TYPE_ENT          interfaceType;
    UINT_8                          dev;
    UINT_8                          interfaceId;
    UINT_8                          address;
    BOOLEAN                         secondaryAddressSupported;
    UINT_8                          secondaryAddress;
}PDL_INTERFACE_SMI_XSMI_STC;


/**
 * @fn  PDL_STATUS prvPdlInterfaceHwGetValue ( IN PDL_INTERFACE_TYPE_ENT interfaceType, IN PDL_INTERFACE_TYP interfaceId, OUT UINT_32 * dataPtr );
 *
 * @brief   get Interface value
 *
 * @param [in]  interfaceType   - interface type.
 * @param [in]  interfaceId     - interface id.
 * @param [out] dataPtr         - data read.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceHwGetValue (
    IN  PDL_INTERFACE_TYPE_ENT                   interfaceType,    
    IN  PDL_INTERFACE_TYP                        interfaceId,
    OUT UINT_32                                * dataPtr
);

/**
 * @fn  PDL_STATUS prvPdlInterfaceHwSetValue ( IN PDL_INTERFACE_TYPE_ENT interfaceType, IN PDL_INTERFACE_TYP interfaceId, OUT UINT_32 data );
 *
 * @brief   set Interface value
 *
 * @param [in]  interfaceType   - interface type.
 * @param [in]  interfaceId     - interface id.
 * @param [in]  data            - data that will be written to HW.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceHwSetValue (
    IN PDL_INTERFACE_TYPE_ENT                   interfaceType,    
    IN PDL_INTERFACE_TYP                        interfaceId,
    OUT UINT_32                                 data
);


/**
 * @fn  PDL_STATUS pdlInterfaceCountGet ( IN PDL_INTERFACE_TYPE_ENT type, OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of interfaces
 *
 * @param [in] type      interface type
 * @param [out] countPtr Number of smi/xsmi interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlInterfaceCountGet (
    IN PDL_INTERFACE_TYPE_ENT   type,
    OUT UINT_32                *countPtr
);

/**
 * @fn  PDL_STATUS pdlInterfaceDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Interface debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlInterfaceDebugSet (
    IN  BOOLEAN             state
);

/*$ END OF prvPdlInterfaceAndValueXmlParser */

/**
 * @fn  PDL_STATUS pdlI2CInterfaceMuxDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl I2C Interface Mux debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CInterfaceMuxDebugSet (
    IN  BOOLEAN             state
);


 /* @}*/

#endif
