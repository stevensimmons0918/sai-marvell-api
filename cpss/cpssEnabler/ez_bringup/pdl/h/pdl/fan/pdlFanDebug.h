/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\fan\pdlfandebug.h.
 *
 * @brief   Declares the pdlfandebug class
 */

#ifndef __pdlFanDebugh

#define __pdlFanDebugh
/**
********************************************************************************
 * @file pdlFanDebug.h   
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
 * @brief Platform driver layer - Fan related debug API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/fan/pdlFan.h>
#include <pdl/fan/private/prvPdlFan.h>

/**
* @addtogroup Fan
* @{
*/

/**
 * @fn  PDL_STATUS pdlFanDebugSet
 *
 * @brief   used to control pdl fan debug flag 
 *          
 *
 * @param [in] state                - enabled/disabled
 *
 * @return  
 */
PDL_STATUS pdlFanDebugSet (
    IN  BOOLEAN                      state
);

 /** 
 * @defgroup Fan_Debug Fan Debug
 * @{Fan driver debug APIs
 */

/**
 * @fn  PDL_STATUS PdlFanDebugInterfaceGet ( IN UINT_32 fanId, OUT PDL_FAN_INTERFACE_STC * interfacePtr );
 *
 * @brief   Get fan access information
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  fanId           - fan identifier.
 * @param [out] interfacePtr    - fan interface pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlFanDebugInterfaceGet (
    IN  UINT_32                   fanId,
    OUT PDL_FAN_INTERFACE_STC   * interfacePtr
);

#if 0
/**
 * @fn  PDL_STATUS PdlFanDebugInterfaceSet ( IN UINT_32 fanId, IN PDL_FAN_VALUES_STC * valuesPtr );
 *
 * @brief   Set fan access information
 *          @note
 *          Used to change data retrieved from XML, can later be saved only changing
 *          statusOkValue, leftRotationValue &amp; rightRotationValue in the interfacePtr is
 *          relevant in this case. other changes will be ignored
 *
 * @param [in]  fanId       - fan identifier.
 * @param [in]  valuesPtr   - fan values pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS PdlFanDebugInterfaceSet (
    IN  UINT_32                   fanId,
    IN  PDL_FAN_VALUES_STC      * valuesPtr
);
#endif
/* @}*/
/* @}*/
#endif
