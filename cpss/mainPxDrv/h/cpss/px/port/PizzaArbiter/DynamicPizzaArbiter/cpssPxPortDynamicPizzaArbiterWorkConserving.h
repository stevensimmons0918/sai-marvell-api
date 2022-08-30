/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssPxPortDynamicPizzaArbiterWorkConserving.h
*
* DESCRIPTION:
*       CPSS implementation for Work Conserving mode for Pizza Arbiter units
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __CPSS_PX_PORT_DYNAMIC_PIZZA_ARBITER_WORK_CONSERVING_H
#define __CPSS_PX_PORT_DYNAMIC_PIZZA_ARBITER_WORK_CONSERVING_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortCtrl.h>


/**
* @internal cpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet function
* @endinternal
*
* @brief   Configure work conserving mode at one or all units
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] unit                     - RxDMA/TxDMA/TXQ/Tx-FIFO or CPSS_PX_PA_UNIT_UNDEFINED_E (meaning all !!)
* @param[in] status                   - enable (1) disable (0)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS cpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet
(
    IN GT_SW_DEV_NUM         devNum,
    IN CPSS_PX_PA_UNIT_ENT   unit,
    IN GT_BOOL               status
);

/**
* @internal cpssPxPortDynamicPizzaArbiterIfWorkConservingModeGet function
* @endinternal
*
* @brief   get work conserving mode of one units
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] unit                     - RxDMA/TxDMA/TXQ/Tx-FIFO
*
* @param[out] statusPtr                - pointer to status.: enable (1) disable (0)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS cpssPxPortDynamicPizzaArbiterIfWorkConservingModeGet
(
    IN GT_SW_DEV_NUM         devNum,
    IN CPSS_PX_PA_UNIT_ENT   unit,
    OUT GT_BOOL              *statusPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

