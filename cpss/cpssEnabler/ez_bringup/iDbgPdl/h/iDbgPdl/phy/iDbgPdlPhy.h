/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlPhyh
#define __iDbgPdlPhyh
/**
********************************************************************************
 * @file iDbgPdlPhy.h
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
#include <pdl/phy/pdlPhy.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>


/*****************************************************************************
* FUNCTION NAME: iDbgPdlPhyGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlPhyGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      phyId
);
/*$ END OF iDbgPdlPhyGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPhyGetCount
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPhyGetCount (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
);
/*$ END OF iDbgPdlPhyGetCount */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPhySetDebug
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlPhySetDebug
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN BOOLEAN                            enable
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlPhyRunValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlPhyRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            phyId
);

#endif
