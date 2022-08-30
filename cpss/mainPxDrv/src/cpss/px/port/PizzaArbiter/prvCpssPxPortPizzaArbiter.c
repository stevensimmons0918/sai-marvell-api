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
* @file prvCpssPxPortPizzaArbiter.c
*
* @brief Interface for Pizza Arbiter configuration .
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiter.h>
#include <cpss/px/port/PizzaArbiter/prvCpssPxPortPizzaArbiter.h>





#if PA_EXEC_TIME_MEAS 
    GT_BOOL pa_TimeTake = GT_FALSE;
    GT_U32   prv_paTime_ms;  
    GT_U32   prv_paTime_us;

    
    GT_STATUS prvCpssPxPortPATimeTakeEnable
    (
        GT_VOID
    )
    {
        pa_TimeTake = GT_TRUE;
        return GT_OK;
    }

    GT_STATUS prvCpssPxPortPATimeTakeDisable
    (
        GT_VOID
    )
    {
        pa_TimeTake = GT_FALSE;
        return GT_OK;
    }

    GT_STATUS prvCpssPxPortPATimeDiffGet
    (
        OUT GT_U32 *prv_paTime_msPtr, 
        OUT GT_U32 *prv_paTime_usPtr
    )
    {
        if (pa_TimeTake == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }
        *prv_paTime_msPtr = prv_paTime_ms;
        *prv_paTime_usPtr = prv_paTime_us;
        return GT_OK;
    }
#endif


/**
* @internal prvCpssPxPortPizzaArbiterIfInit function
* @endinternal
*
* @brief   Pizza arbiter initialization in all Units where it's present
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
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
GT_STATUS prvCpssPxPortPizzaArbiterIfInit
(
    IN  GT_SW_DEV_NUM           devNum 
)
{
    GT_STATUS   rc;                 /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortDynamicPizzaArbiterIfInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal prvCpssPxPortPizzaArbiterIfCheckSupport function
* @endinternal
*
* @brief   Check whether Pizza Arbiter can be configured to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPizzaArbiterIfCheckSupport
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32 portSpeedInMBit;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortDynamicPizzaArbiterSpeedConv(devNum,portNum,portSpeed,/*OUT*/&portSpeedInMBit);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPxPortDynamicPizzaArbiterIfCheckSupport(devNum,portNum, portSpeedInMBit);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}




/**
* @internal prvCpssPxPortPizzaArbiterIfConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter accoringly to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] portSpeed                - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPizzaArbiterIfConfigure
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_SPEED_ENT     portSpeed
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32 portSpeedInMBit;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    #if PA_EXEC_TIME_MEAS
        GT_TIME timeStart = {0,0};

        if (pa_TimeTake == GT_TRUE)
        {
            rc = prvCpssOsTimeRTns(&timeStart);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    #endif


    rc = prvCpssPxPortDynamicPizzaArbiterSpeedConv(devNum,portNum,portSpeed,/*OUT*/&portSpeedInMBit);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPxPortDynamicPizzaArbiterIfConfigure(devNum,portNum, portSpeedInMBit);
    if (rc != GT_OK)
    {
        return rc;
    }

    #if PA_EXEC_TIME_MEAS
        if (pa_TimeTake)
        {
            prvCpssOsTimeRTDiff(timeStart, /*OUT*/&prv_paTime_ms, &prv_paTime_us);
        }
    #endif

    return GT_OK;
}


/**
* @internal prvCpssPxPortPizzaArbiterIfDelete function
* @endinternal
*
* @brief   Delete port from Pizza Arbiter
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
GT_STATUS prvCpssPxPortPizzaArbiterIfDelete
(
    IN  GT_SW_DEV_NUM           devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum
)
{
    GT_STATUS   rc;                 /* return code */

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortDynamicPizzaArbiterIfConfigure(devNum,portNum, 0);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}



