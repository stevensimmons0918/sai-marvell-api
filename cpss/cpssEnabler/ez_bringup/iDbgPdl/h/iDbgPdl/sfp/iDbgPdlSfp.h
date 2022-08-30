/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlSfph
#define __iDbgPdlSfph
/**
********************************************************************************
 * @file iDbgPdlSensor.h   
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
 * @brief Debug lib sfp module
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSfpGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlSfpGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
);
/*$ END OF iDbgPdlSfpGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSfpGetHwInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlSfpGetHwInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
);
/*$ END OF iDbgPdlSfpGetHwInfo */
/*****************************************************************************
* FUNCTION NAME: iDbgPdlSfpHwEepromRead
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlSfpHwEepromRead (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort,
    IN UINT_8                             i2cAddress,
    IN UINT_16                            offset,
    IN UINT_32                            length
);
/*$ END OF iDbgPdlSfpHwEepromRead */
/*****************************************************************************
* FUNCTION NAME: iDbgPdlSfpSetInfo
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlSfpSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort,
    IN char                             * sfpValueTypeStr,
    IN char                             * interfaceTypeStr,
    IN UINT_32                            interfaceId,
    IN UINT_32                            value /* if needed */
);
/*$ END OF iDbgPdlSfpSetInfo */

#endif
