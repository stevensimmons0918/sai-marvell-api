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
* @file cpssPxPortPizzaArbiter.h
*
* @brief public PA IF function
*
*
* @version   1
********************************************************************************
*/
#ifndef __CPSS_PX_PORT_PIZZA_ARBITER_H
#define __CPSS_PX_PORT_PIZZA_ARBITER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
 

#include <cpss/common/cpssTypes.h>
#include <cpss/px/port/cpssPxPortCtrl.h>



/**
* @struct CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC
 *
 * @brief A structure to the state of Pizza Arbiter unit
*/
typedef struct{

    /** total configured slices */
    GT_U32 totalConfiguredSlices;

    /** how many slices there at unit */
    GT_U32 totalSlicesOnUnit;

    /** state of work conserving bit */
    GT_BOOL workConservingBit;

    /** state of Map load Enable bit */
    GT_BOOL pizzaMapLoadEnBit;

    GT_BOOL slice_enable     [340];

    GT_U32 slice_occupied_by[340];

} CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC;


/**
* @struct CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC
 *
 * @brief A structure to the state of all Pizza Arbiter units on specific device
*/
typedef struct{

    CPSS_PX_PA_UNIT_ENT unitList[CPSS_PX_PA_UNIT_MAX_E+1];

    CPSS_PX_PIZZA_ARBITER_UNIT_STATE_STC unitState[CPSS_PX_PA_UNIT_MAX_E];

} CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC;


/**
* @struct CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC
 *
 * @brief A structure to the state of all Pizza Arbiter units on specific device, the device type included.
*/
typedef struct
{
    CPSS_PP_FAMILY_TYPE_ENT devType;
    union
    {
        CPSS_PX_DEV_UNITS_PIZZA_ARBITER_STATE_STC     pipe;
    }devState;
}CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC;

/**
* @internal cpssPxPortPizzaArbiterDevStateGet function
* @endinternal
*
* @brief   get state of device Pizza Arbiter units
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
GT_STATUS cpssPxPortPizzaArbiterDevStateGet
(
    IN  GT_SW_DEV_NUM devNum,
    OUT CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif

