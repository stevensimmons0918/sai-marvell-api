/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssPxPortDynamicPizzaArbiter.c
*
* DESCRIPTION:
*       implementation of Pizza Arbiter engine for Pipe
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __PRV_CPSS_PX_PORT_DYNAMIC_PIZZA_ARBITER_H
#define __PRV_CPSS_PX_PORT_DYNAMIC_PIZZA_ARBITER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/PizzaArbiter/cpssPxPortPizzaArbiter.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAUnitBW.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAUnitDrv.h>

#define PRV_CPSS_PX_PA_BW_COEFF  1000   /* IN MBit */

/**
* @internal prvCpssPxPortDynamicPizzaArbiterSpeedConv function
* @endinternal
*
* @brief   convert speed from CPSS unit to Mbps used in PA
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] speedEnm                 - speed in CPSS enumrator
*
* @param[out] speedPtr                 - pointer to speed in Mbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*
* @note 12.5G may be converted to 13000 Mbps (since resoultion 0.5G is not supported)
*       speed conversion table is supplied by PA WS
*
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterSpeedConv
(
    IN  GT_SW_DEV_NUM        devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speedEnm,
    OUT GT_U32              *speedPtr 
);

/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfInit function
* @endinternal
*
* @brief   Init PA for given device
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfInit
(
    IN GT_SW_DEV_NUM         devNum
);

/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfCheckSupport function
* @endinternal
*
* @brief   check PA whether given speed and port are supported
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - port number
* @param[in] portSpeedInMBit          - speed in Mbps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfCheckSupport
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
);

/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeedInMBit          - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfConfigure
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U32                  portSpeedInMBit
);

/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfDevStateGet function
* @endinternal
*
* @brief   get state of all Pizza Arbiter units on device
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*
* @param[out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfDevStateGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    OUT CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

