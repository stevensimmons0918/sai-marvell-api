/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlOobh
#define __iDbgPdlOobh
/**
********************************************************************************
 * @file iDbgPdlOob.h
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
#include <pdl/oob_port/pdlOobPort.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>


/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlOobGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId
);
/*$ END OF iDbgPdlOobGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobGetCount
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobGetCount (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
);
/*$ END OF iDbgPdlOobGetCount */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetDebug
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetDebug
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN BOOLEAN                            enable
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetEthId
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetEthId (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId,
    IN UINT_32                  	      ethId
);
/*$ END OF iDbgPdlOobSetEthId */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetCpuPort
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetCpuPort (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId,
    IN UINT_32                  	      cpuPort
);
/*$ END OF iDbgPdlOobSetCpuPort */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetMaxSpeed
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetMaxSpeed (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId,
    IN char                             * speedStrPtr
);
/*$ END OF iDbgPdlOobSetMaxSpeed */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetPhyId
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetPhyId (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId,
    IN UINT_32                            phyId
);
/*$ END OF iDbgPdlOobSetPhyId */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobSetPhyPosition
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobSetPhyPosition (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId,
    IN UINT_32                            phyPos
);
/*$ END OF iDbgPdlOobSetPhyPosition */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlOobRunValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlOobRunValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                  	      oobId
);

#endif
