/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlCpuh
#define __iDbgPdlCpuh
/**
********************************************************************************
 * @file iDbgPdlCpu.h
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
 * @brief Debug lib cpu module
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/cpu/pdlCpu.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>


/*****************************************************************************
* FUNCTION NAME: iDbgPdlCpuGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlCpuGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
);
/*$ END OF iDbgPdlCpuGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlCpuSetDebug
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlCpuSetDebug
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN BOOLEAN                            enable
);
/*$ END OF iDbgPdlCpuSetDebug */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlCpuSetType
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlCpuSetType (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN char                              *cpuTypeStrPtr
);
/*$ END OF iDbgPdlCpuSetType */

#endif