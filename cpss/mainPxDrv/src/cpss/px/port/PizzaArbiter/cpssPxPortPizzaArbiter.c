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
* @file cpssPxPortPizzaArbiter.c
*
* @brief public PA IF function
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/PizzaArbiter/cpssPxPortPizzaArbiter.h>
#include <cpss/px/port/PizzaArbiter/prvCpssPxPortPizzaArbiter.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiter.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/cpssPxPortMapping.h>



/**
* @internal prvCpssPxPortPizzaArbiterDevStateInit function
* @endinternal
*
* @brief   Init the structure CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
* @param[in,out] pizzaDeviceStatePtr      - pointer to structure describing the state of each units.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on pizzaDeviceStatePtr is NULL
*/
static GT_STATUS prvCpssPxPortPizzaArbiterDevStateInit
(
    INOUT CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
{
    if (NULL == pizzaDeviceStatePtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    cpssOsMemSet(pizzaDeviceStatePtr,0,sizeof(*pizzaDeviceStatePtr));
    return GT_OK;
}



/**
* @internal internal_cpssPxPortPizzaArbiterDevStateGet function
* @endinternal
*
* @brief   Pizza Arbiter State which includes state of all
*         - pizza arbiter units
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
static GT_STATUS internal_cpssPxPortPizzaArbiterDevStateGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    OUT CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
{
    GT_STATUS   rc;                 /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_CH1_E | CPSS_CH1_DIAMOND_E
                                            | CPSS_CH2_E | CPSS_CH3_E | CPSS_XCAT_E | CPSS_XCAT3_E
                                            | CPSS_LION_E | CPSS_XCAT2_E);
    CPSS_NULL_PTR_CHECK_MAC(pizzaDeviceStatePtr);

    rc = prvCpssPxPortPizzaArbiterDevStateInit(pizzaDeviceStatePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPxPortDynamicPizzaArbiterIfDevStateGet(devNum, pizzaDeviceStatePtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssPxPortPizzaArbiterDevStateGet function
* @endinternal
*
* @brief   Pizza Arbiter State which includes state of all
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
*                                      portGroupId  - number of port group
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
    IN  GT_SW_DEV_NUM                        devNum,
    OUT CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPizzaArbiterDevStateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, pizzaDeviceStatePtr));

    rc = internal_cpssPxPortPizzaArbiterDevStateGet(devNum, pizzaDeviceStatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, pizzaDeviceStatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


