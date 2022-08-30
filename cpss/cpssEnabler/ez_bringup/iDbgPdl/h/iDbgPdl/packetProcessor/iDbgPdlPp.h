/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlPph
#define __iDbgPdlPph
/**
********************************************************************************
 * @file iDbgPdlPp.h   
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
 * @brief Debug lib sensor module
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlPpGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
);
/*$ END OF iDbgPdlPpGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpGetFp
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlPpGetFp (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            fpId
);
/*$ END OF iDbgPdlPpGetFp */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpSetDebug
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlPpSetDebug
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN BOOLEAN                            enable
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpSetFp
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpSetFp (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            fpId,
    IN char                             * grOrderStr,
    IN char                             * maxSpeedStr
);
/*$ END OF iDbgPdlPpSetFp */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpSetB2b
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpSetB2b (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            b2bId,
    IN char                             * maxSpeedStr,
    IN char                             * ifModeStr,
    IN void                             * firstDevicePtr,
    IN void                             * firstPortPtr,
    IN void                             * secondDevicePtr,
    IN void                             * secondPortPtr
);
/*$ END OF iDbgPdlPpSetB2b */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpPortRunValidation
*
* DESCRIPTION:   Verify all port interface modes & speed are supported by device
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpPortRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
);

/*$ END OF iDbgPdlPpPortRunValidation */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPpRunValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPpRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
);

#endif
