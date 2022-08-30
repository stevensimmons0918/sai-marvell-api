/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

#ifndef __iDbgPdlUtilsh
#define __iDbgPdlUtilsh
/**
********************************************************************************
 * @file iDbgPdlInit.h   
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
 * @brief Debug lib layer - Tools
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/button/pdlBtn.h>

#define IDBG_PDL_ID_ALL             0xFFFF


typedef enum {
    IDBG_PDL_UTILS_HW_TYPE_BTN,
    IDBG_PDL_UTILS_FAN_STATE,
    IDBG_PDL_UTILS_LED_TYPE,
    IDBG_PDL_UTILS_LED_STATE,
    IDBG_PDL_UTILS_LED_COLOR,
    IDBG_PDL_UTILS_CPU_FAMILY,
    IDBG_PDL_UTILS_INTERFACE_TYPE,
    IDBG_PDL_UTILS_SFP_VALUE_TYPE,
    IDBG_PDL_UTILS_HW_TYPE_LAST
} IDBG_PDL_UTILS_HW_TYPE_ENT;

/*****************************************************************************
* FUNCTION NAME: iDbgPdlConvertEnumToString
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlConvertEnumToString (
    IDBG_PDL_UTILS_HW_TYPE_ENT      type,
    UINT_32                         id,
    char                         ** stringPtr
);

/*$ END OF iDbgPdlConvertEnumToString */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlConvertEnumToString
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlConvertStringToEnum (
    IN  IDBG_PDL_UTILS_HW_TYPE_ENT      type,
    IN  char                          * string,
    OUT UINT_32                       * idPtr
);

/*$ END OF iDbgPdlConvertStringToEnum */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlStringReplace
*
* DESCRIPTION: 
*      
*
*****************************************************************************/

PDL_STATUS iDbgPdlStringReplace (
    IN  char                          * string,
    IN  char                          * token,
    IN  char                          * newToken,
    IN  UINT_32                         newStringSize,
    OUT char                          * newString
);

/*$ END OF iDbgPdlStringReplace */

#endif
