/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlSerdesh
#define __iDbgPdlSerdesh
/**
********************************************************************************
 * @file iDbgPdlSerdes.h   
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
#include <pdl/serdes/pdlSerdes.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>


/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlSerdesGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                  	      logicalPort
);
/*$ END OF iDbgPdlSensorGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesSetTxFineTune
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlSerdesSetTxFineTune (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN void                             * devPtr,
    IN void                             * portPtr,
    IN void                             * ifModePtr,
    IN void                             * connectTypePtr,
    IN void                             * txAmplPtr,
    IN void                             * txAmplAdjEnPtr,
    IN void                             * txAmplShtEnPtr,
    IN void                             * emph0Ptr,
    IN void                             * emph1Ptr,
    IN void                             * txEmphEnPtr
);
/*$ END OF iDbgPdlSerdesSetTxFineTune */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesSetRxFineTune
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlSerdesSetRxFineTune (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN void                             * devPtr,
    IN void                             * lanePtr,
    IN void                             * ifModePtr,
    IN void                             * connectTypePtr,
    IN void                             * sqlchPtr,
    IN void                             * ffeResPtr,
    IN void                             * ffeCapPtr,
    IN void                             * align90Ptr,
    IN void                             * dcGainPtr,
    IN void                             * bandWidthPtr
);
/*$ END OF iDbgPdlSerdesSetRxFineTune */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesGetPolarity
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlSerdesGetPolarity (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                  	      laneId
);
/*$ END OF iDbgPdlSerdesGetPolarity */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesSetPolarity
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlSerdesSetPolarity (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            laneId,
    IN BOOLEAN                          * txSwapPtr,
    IN BOOLEAN                          * rxSwapPtr
);
/*$ END OF iDbgPdlSerdesSetPolarity */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesGetFineTune
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlSerdesGetFineTune (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                  	      laneId
);
/*$ END OF iDbgPdlSerdesGetFineTune */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesSetDebug
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlSerdesSetDebug
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN BOOLEAN                            enable
);

#endif