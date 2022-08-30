/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\interface\pdlsmidebug.h.
 *
 * @brief   Declares the pdlsmiXsmidebug class
 */

#ifndef __prvPdlSmiXsmiDebugh

#define __prvPdlSmiXsmiDebugh
/**
********************************************************************************
 * @file pdlSmiXsmiDebug.h  
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
 * @brief Platform driver layer - SMI interface debug related API (internal)
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>

#define PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_DEVICE          1 << 0
#define PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_INTERFACE_ID    1 << 1
#define PDL_SMI_XSMI_SET_ATTRIBUTE_MASK_ADDRESS         1 << 2


/* ***************************************************************************
* FUNCTION NAME: pdlSmiXsmiDebugDbGetAttributes
*
* DESCRIPTION:   get interface attributes from DB
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlSmiXsmiDebugDbGetAttributes (
    IN  PDL_INTERFACE_TYP                    interfaceId,
    OUT PDL_INTERFACE_SMI_XSMI_STC          * attributesPtr
);


/* ***************************************************************************
* FUNCTION NAME: pdlSmiXsmiDebugDbSetAttributes
*
* DESCRIPTION:   set interface attributes from DB
*
* PARAMETERS:   
*                
*****************************************************************************/

PDL_STATUS pdlSmiXsmiDebugDbSetAttributes (
    IN  PDL_INTERFACE_TYP                    interfaceId,
    IN  UINT_32                              attributesMask,
    OUT PDL_INTERFACE_SMI_XSMI_STC         * attributesPtr
);


/**
 * @fn  PDL_STATUS pdlSmiXsmiCountGet ( OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of smi/xsmi interfaces
 *
 * @param [out] countPtr Number of smi/xsmi interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlSmiXsmiCountGet (
    OUT UINT_32 * countPtr
);

/**
 * @fn  PDL_STATUS pdlSmiXsmiHwGetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, OUT UINT_32 * dataPtr )
 *
 * @brief   Pdl Smi/Xsmi hardware get value
 *
 * @param           interfaceId Identifier for the interface.
 * @param           offset      The offset.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSmiXsmiHwGetValue (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_8                                  phyDeviceOrPage,
    IN  UINT_16                                 regAddress,
    OUT UINT_16                               * dataPtr
);

/*$ END OF pdlSmiXsmiHwGetValue */

/**
 * @fn  PDL_STATUS pdlSmiXsmiHwSetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, OUT UINT_32 data )
 *
 * @brief   Pdl Smi/Xsmi hardware set value
 *
 * @param           interfaceId Identifier for the interface.
 * @param           offset      The offset.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSmiXsmiHwSetValue (
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_8                                  phyDeviceOrPage,
    IN  UINT_16                                 regAddress,
    IN  UINT_16                                 mask,
    OUT UINT_16                                 data
);

/*$ END OF pdlSmiXsmiHwSetValue */

/* @}*/
/* @}*/
/* @}*/

#endif
