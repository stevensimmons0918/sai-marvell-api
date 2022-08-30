/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlPowerh
#define __iDbgPdlPowerh
/**
********************************************************************************
 * @file iDbgPdlPower.h   
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
 * @brief Debug lib power module
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/power_supply/pdlPower.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPowerGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPowerGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                     * powerId
);

/*$ END OF iDbgPdlPowerGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPowerGetHwInfo
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPowerGetHwInfo (   
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                             * powerId
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPowerRunValidation
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPowerRunValidation (   
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                             * powerId
);

#endif
