/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlBtnh
#define __iDbgPdlBtnh
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
 * @brief Debug lib button module
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/button/pdlBtn.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

typedef enum {
    IDBG_PDL_BTN_SET_CMD_INFO_E,
    IDBG_PDL_BTN_SET_CMD_DEBUG_E
} IDBG_PDL_BTN_SET_CMD_ENT;

/*****************************************************************************
* FUNCTION NAME: iDbgPdlBtnGetState
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlBtnGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      btnId
);

/*$ END OF iDbgPdlBtnGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlBtnGetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlBtnGetHwStatus (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      btnId
);

/*$ END OF iDbgPdlBtnGetHwStatus */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlBtnSetPushValue
*
* DESCRIPTION:   change the push value for given button
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlBtnSetPushValue (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN IDBG_PDL_BTN_SET_CMD_ENT           cmdType,
    IN UINT_32                  	      btnId,
    IN UINT_32                            value
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlBtnGetValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlBtnGetValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      btnId
);
/*$ END OF iDbgPdlBtnSetPushValue */

#endif
