/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssPxPortDynamicPAUnitBW.h
*
* @brief pipe and higher dynamic (algorithmic) pizza arbiter
*
* @version   1
********************************************************************************
*/
#ifndef __PRV_CPSS_PX_PORT_DYNAMIC_PA_UNIT_BW_H
#define __PRV_CPSS_PX_PORT_DYNAMIC_PA_UNIT_BW_H

#include <cpss/common/cpssTypes.h>
#include <cpss/px/port/cpssPxPortCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfUnitBWSet function
* @endinternal
*
* @brief   set pipe BW for specific unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] unit                     -  type
* @param[in] bwInGBit                 - bandwidth in GBits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*
* @note I forced to place this function here, because it needs number of port
*       group where CPU port is connected and there is just no more suitable
*       place.
*
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfUnitBWSet
(
    IN GT_SW_DEV_NUM           devNum,
    IN CPSS_PX_PA_UNIT_ENT     unit,
    IN GT_U32                  bwInGBit
);


/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfUnitBWGet function
* @endinternal
*
* @brief   Get pipe BW for specific unit
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] unit                     -  type
*
* @param[out] bwInGBitPtr              - bandwidth in GBits
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - on wrong ptr
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfUnitBWGet
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  CPSS_PX_PA_UNIT_ENT     unit,
    OUT GT_U32                 *bwInGBitPtr
);
/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfUnitBWInit function
* @endinternal
*
* @brief   Inti pipe BW for all units depending on device id
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfUnitBWInit
(
    GT_SW_DEV_NUM devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif


