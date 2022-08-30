/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlInbandPorth
#define __iDbgPdlInbandPorth
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
 * @brief Debug lib inband-port module
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPortInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPortInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                  	      logicalPort
);
/*$ END OF iDbgPdlInbandGetPortInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPhyInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPhyInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                  	      logicalPort
);
/*$ END OF iDbgPdlInbandGetPhyInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPhyInitInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPhyInitInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
);
/*$ END OF iDbgPdlInbandGetPhyInitInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPhyPostInitInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPhyPostInitInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
);
/*$ END OF iDbgPdlInbandGetPhyPostInitInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetCount
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetCount
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNum
);
/*$ END OF iDbgPdlInbandGetCount */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandSetPhyId
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandSetPhyId
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNum,
    IN UINT_32                            portum,
    IN UINT_32                            phyId
);
/*$ END OF iDbgPdlInbandSetPhyId */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandSetPhyPosition
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandSetPhyPosition
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNum,
    IN UINT_32                            portNum,
    IN UINT_32                            phyPosition
);
/*$ END OF iDbgPdlInbandSetPhyPosition */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandSetInfoPortMode
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandSetInfoPortMode
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNum,
    IN UINT_32                            portNum,
    IN char                              *transceiverStrPtr,
    IN char                              *speedStrPtr,
    IN char                              *modeStrPtr
);
/*$ END OF iDbgPdlInbandSetInfoPortMode */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetPacketProcessorPortInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetPacketProcessorPortInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              devNumber,
    IN UINT_32                              portNumberInDev
);

/*$ END OF iDbgPdlNetworkGetPacketProcessorPortInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetPacketProcessorPortPhyInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetPacketProcessorPortPhyInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              devNumber,
    IN UINT_32                              portNumberInDev
);

/*$ END OF iDbgPdlNetworkGetPacketProcessorPortPhyInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPhyPostInitListInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPhyInitListInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev,
    IN char                             * postInitOrAllInitListPtr,  
    IN char                             * transceiverPtr,  
    IN char                             * speedPtr  
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkRunPacketProcessorValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkRunPacketProcessorValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              devNumber,
    IN UINT_32                              portNumberInDev
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkRunFrontPanelValidation
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkRunFrontPanelValidation (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              groupNumber,
    IN UINT_32                              portNumber
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetFrontPanelGroupInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetFrontPanelGroupInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              groupNumber
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetFrontPanelPortInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetFrontPanelPortInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC    * resultPtr,
    IN UINT_32                              groupNumber,
    IN UINT_32                              portNumberInGroup
    );

/*****************************************************************************
* FUNCTION NAME: iDbgPdlInbandGetPhyScriptInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlInbandGetPhyScriptInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev,
    IN char                             * postInitPtr,  
    IN char                             * transceiverPtr,  
    IN char                             * speedPtr  
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetGbicInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetGbicInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev
);

/*****************************************************************************
* FUNCTION NAME: iDbgPdlNetworkGetHwGbicInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlNetworkGetHwGbicInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            devNumber,
    IN UINT_32                            portNumberInDev
);

#endif
