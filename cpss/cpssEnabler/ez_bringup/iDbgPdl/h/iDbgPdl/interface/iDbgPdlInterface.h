/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
 
#ifndef __iDbgPdlInterfaceh
#define __iDbgPdlInterfaceh
/**
********************************************************************************
 * @file iDbgPdlBtn.h   
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
 * @brief Debug lib interface module
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                             * interfaceTypeStr,
    IN PDL_INTERFACE_TYP                  interfaceId
);

/*$ END OF iDbgPdlInterfaceGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceGpioSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            paramMask,
    IN UINT_32                            devId,
    IN UINT_32                            pinNumber,
    IN char                             * typeStr
);

/*$ END OF iDbgPdlInterfaceGpioSetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceI2cSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceI2cSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            paramMask,
    IN UINT_32                            busId,
    IN UINT_32                            i2cAddress,
    IN char                             * accessSizeStr,
    IN UINT_32                            offset,
    IN UINT_32                            mask
);

/*$ END OF iDbgPdlInterfaceI2cSetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceMuxGpioSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceMuxGpioSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            muxId,
    IN UINT_32                            paramMask,
    IN UINT_32                            devId,
    IN UINT_32                            pinNumber,
    IN UINT_32                            value
);

/*$ END OF iDbgPdlInterfaceMuxGpioSetInfo */


/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceMuxI2CSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceMuxI2CSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            muxId,
    IN UINT_32                            paramMask,
    IN UINT_32                            i2cAddress,
    IN UINT_32                            offset,
    IN UINT_32                            mask,
    IN UINT_32                            value
);

/*$ END OF iDbgPdlInterfaceMuxI2CSetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceSmiXsmiSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceSmiXsmiSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            paramMask,
    IN UINT_32                            smiDev,
    IN UINT_32                            phyInterfaceId,
    IN UINT_32                            smiAddress
);

/*$ END OF iDbgPdlInterfaceSmiXsmiSetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceGetHw
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceGetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                             * interfaceTypeStr,
    IN PDL_INTERFACE_TYP                  interfaceId
);

/*$ END OF iDbgPdlInterfaceGetHw */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceSmiXsmiGetHw
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceSmiXsmiGetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            phyDeviceOrPage,
    IN UINT_32                            regAddress
);

/*$ END OF iDbgPdlInterfaceSmiXsmiGetHw */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceSetHw
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceSetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                             * interfaceTypeStr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            value
);

/*$ END OF iDbgPdlInterfaceSetHw */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInterfaceSmiXsmiSetHw
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlInterfaceSmiXsmiSetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_INTERFACE_TYP                  interfaceId,
    IN UINT_32                            phyDeviceOrPage,
    IN UINT_32                            regAddress,
    IN UINT_32                            mask,
    IN UINT_32                            value
);

/*$ END OF iDbgPdlInterfaceSmiXsmiSetHw */

#endif
