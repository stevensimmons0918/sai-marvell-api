/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlLedh
#define __iDbgPdlLedh
/**
********************************************************************************
 * @file iDbgPdlLed.h   
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
 * @brief Debug lib led module
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/led/pdlLed.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedSystemGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlLedSystemGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_LED_ID_ENT                     ledId
);

/*$ END OF iDbgPdlLedSystemGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedPortGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedPortGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
);

/*$ END OF iDbgPdlLedPortGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanSetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedSystemSetHwStatus (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_LED_ID_ENT                     ledId,
    IN PDL_LED_STATE_ENT                  state,
    IN PDL_LED_COLOR_ENT                  color
);

/*$ END OF iDbgPdlLedSystemSetHwStatus */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedSetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedSetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_LED_ID_ENT                     ledId,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort,
    IN PDL_LED_STATE_ENT                  state,
    IN PDL_LED_COLOR_ENT                  color,
    IN UINT_32                            value,
    IN UINT_32                            smiDevOrPage,
    IN UINT_32                            smiReg,
    IN UINT_32                            smiMask
);

/*$ END OF iDbgPdlLedSetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedPortSetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedPortSetHwStatus (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort,
    IN PDL_LED_ID_ENT                     ledId,
    IN PDL_LED_STATE_ENT                  state,
    IN PDL_LED_COLOR_ENT                  color
);
/*$ END OF iDbgPdlLedPortSetHwStatus */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlLedRunValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlLedRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN PDL_LED_ID_ENT                     ledId,
    IN UINT_32                            timeSec 
);
#endif
