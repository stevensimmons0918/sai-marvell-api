/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlFanh
#define __iDbgPdlFanh
/**
********************************************************************************
 * @file iDbgPdlFan.h   
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
 * @brief Debug lib fan module
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/fan/pdlFan.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

typedef enum {
    IDBG_PDL_FAN_SET_CMD_STATE_E,
    IDBG_PDL_FAN_SET_CMD_DUTY_CYCLE_E,
    IDBG_PDL_FAN_SET_CMD_DEBUG_E, 
    IDBG_PDL_FAN_SET_CMD_ROTATION_DIRECTION_E,
    IDBG_PDL_FAN_SET_CMD_DUTY_CYCLE_METHOD_E,
    IDBG_PDL_FAN_SET_CMD_FAULT_E,
    IDBG_PDL_FAN_SET_CMD_INTILAIZE_E,
    IDBG_PDL_FAN_SET_CMD_PULSE_PER_ROTATION_E,
    IDBG_PDL_FAN_SET_CMD_THRESHOLD_E,
} IDBG_PDL_FAN_SET_CMD_ENT;

/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlFanGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      fanControllerId,
    IN UINT_32                            fanId
);

/*$ END OF iDbgPdlFanGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanGetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlFanGetHwStatus (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      fanControllerId,
    IN UINT_32                            fanId
);

/*$ END OF iDbgPdlFanGetHwStatus */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanSetHwStatus
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlFanSetHwStatus (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN IDBG_PDL_FAN_SET_CMD_ENT           cmdType,
    IN UINT_32                  	      fanControllerId,
    IN UINT_32                            fanId,
    IN UINT_32                            value
);

/*$ END OF iDbgPdlFanSetHwStatus */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanRunControllerValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlFanRunControllerValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            fanControllerId,
    IN UINT_32                            fanNumber
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlFanGetControllerinfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlFanGetControllerinfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            fanControllerId    
);

/*****************************************************************************
* FUNCTION NAME:  iDbgPdlFanControllerGetHw
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS  iDbgPdlFanControllerGetHw (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            fanControllerId
);

#endif
