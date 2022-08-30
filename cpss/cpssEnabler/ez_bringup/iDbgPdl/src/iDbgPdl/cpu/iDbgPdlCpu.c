/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
********************************************************************************
 * @file iDbgPdlCpu.c
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
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/cpu/pdlCpu.h>
#include <pdl/cpu/pdlCpuDebug.h>
#include <pdl/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <iDbgPdl/cpu/iDbgPdlCpu.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>

/*****************************************************************************
* FUNCTION NAME: iDbgPdlCpuGetInfo
*
* DESCRIPTION: 
*      
*
*****************************************************************************/
PDL_STATUS iDbgPdlCpuGetInfo (
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	PDL_STATUS						pdlStatus;
    char                           *cpuFamilyStr, *cpuTypeStr;
    BOOLEAN                         isFirst = TRUE;
    PDL_CPU_TYPE_ENT                cpuType;
    PDL_CPU_FAMILY_TYPE_ENT         cpuFamily;
    PDL_CPU_SDMA_MAP_INFO_STC       cpuSdmaMapInfo;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlCpuTypeGet, IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, &cpuType));
    PDL_CHECK_STATUS(pdlStatus);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlCpuFamilyTypeGet, IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, &cpuFamily));
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = iDbgPdlConvertEnumToString(IDBG_PDL_UTILS_CPU_FAMILY, cpuFamily, &cpuFamilyStr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = pdlLibConvertEnumValueToString(XML_PARSER_ENUM_ID_CPU_TYPE_TYPE_E, cpuType, &cpuTypeStr);
    PDL_CHECK_STATUS(pdlStatus);

    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" CPU Type  : %s\n", cpuTypeStr);
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" CPU Family: %s\n", cpuFamilyStr);

    for (pdlStatus = pdlCpuSdmaMapDbGetFirst(&cpuSdmaMapInfo);
         pdlStatus == PDL_OK;
         pdlStatus = pdlCpuSdmaMapDbGetNext(&cpuSdmaMapInfo, &cpuSdmaMapInfo)) {     
        if (isFirst) {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("CPU SDMA port mapping info :\n");
            IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-10s | %-10s | %-10s | %-12s |\n", "Index", "Device", "macPort", "logicalPort");
            isFirst = FALSE;
        }
        IDBG_PDL_LOGGER_API_OUTPUT_LOG (" %-10d | %-10d | %-10d | %-12d |\n", cpuSdmaMapInfo.key.index, cpuSdmaMapInfo.dev, cpuSdmaMapInfo.macPort, cpuSdmaMapInfo.logicalPort);
    }

    return PDL_OK;
}
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
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/
    PDL_STATUS                      pdlStatus;
/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlCpuDebugSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(BOOLEAN, enable));

    return pdlStatus;
}
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
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
	PDL_STATUS						pdlStatus;
    PDL_CPU_FAMILY_TYPE_ENT         cpuType;
    UINT_32                         value;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    pdlStatus = pdlLibConvertEnumStringToValue(XML_PARSER_ENUM_ID_CPU_TYPE_TYPE_E, cpuTypeStrPtr, &value);
    if (pdlStatus != PDL_OK)
    {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown cpu type. Possibly your xml file typedef section is out-of-date.\n");
        return pdlStatus;
    }
    cpuType = (PDL_CPU_TYPE_ENT)value;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlCpuTypeSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, cpuType));
    PDL_CHECK_STATUS(pdlStatus);

    return pdlStatus;
}
/*$ END OF iDbgPdlCpuSetType */
