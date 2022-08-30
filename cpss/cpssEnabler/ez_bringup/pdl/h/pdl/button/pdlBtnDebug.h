/* Copyright (C) 2018 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */
/**
 * @file-docAll core\ez_bringup\h\button\pdlbtndebug.h.
 *
 * @brief   Declares the pdlbtndebug class
 */

#ifndef __pdlBtnDebugh

#define __pdlBtnDebugh
/**
********************************************************************************
 * @file pdlBtnDebug.h   
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
 * @brief Platform driver layer - Button related debug API
 * 
 * @version   1 
********************************************************************************
*/

#include <pdl/button/pdlBtn.h>
/**
* @addtogroup Button
* @{
*/

 /** 
 * @defgroup Button_Debug Button Debug
 * @{Button driver debug APIs
 */

/**
 * @fn  PDL_STATUS pdlBtnDebugInterfaceGet ( IN PDL_BTN_ID_ENT buttonId, OUT PDL_BTN_INTERFACE_STC * interfacePtr );
 *
 * @brief   Get buttonId access information
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  buttonId        - button identifier.
 * @param [out] interfacePtr    - button interface pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlBtnDebugInterfaceGet (
    IN  PDL_BTN_ID_ENT            buttonId,
    OUT PDL_BTN_INTERFACE_STC   * interfacePtr
);

/**
 * @fn  PDL_STATUS pdlBtnDebugInterfaceSet ( IN PDL_BTN_ID_ENT buttonId, IN PDL_BTN_VALUES_STC * valuesPtr );
 *
 * @brief   Set buttonId access information
 *          @note
 *          Used to change data retrieved from XML, can later be saved
 *
 * @param [in]  buttonId    - button identifier.
 * @param [in]  valuesPtr   - button values pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlBtnDebugInterfaceSet (
    IN PDL_BTN_ID_ENT            buttonId,
    IN PDL_BTN_VALUES_STC      * valuesPtr
);

/**
 * @fn  PDL_STATUS pdlBtnDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Btn debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlBtnDebugSet (
    IN  BOOLEAN             state
);

/* @}*/
/* @}*/

#endif