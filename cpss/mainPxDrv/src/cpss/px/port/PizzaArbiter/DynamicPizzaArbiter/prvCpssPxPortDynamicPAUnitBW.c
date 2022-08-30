/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* * prvCpssPxPortDynamicPAUnitBW.c
*
* DESCRIPTION:
*       pipe and higher dynamic (algorithmic) pizza arbiter
*
* FILE REVISION NUMBER:
*       $Revision: 4$
*******************************************************************************/
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAUnitBW.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAPortSpeedDB.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiterWS.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
 
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
*/
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfUnitBWSet
(
    IN GT_SW_DEV_NUM             devNum,
    IN CPSS_PX_PA_UNIT_ENT       unit,
    IN GT_U32                    bwInGBit
)
{
    GT_U32 * unitBWPtr;
    GT_32   unitN;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    unitN = sizeof(PRV_CPSS_PX_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr)/sizeof(PRV_CPSS_PX_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr[0]);
    if (0 <= unit && unit < unitN )
    {
        unitBWPtr = &PRV_CPSS_PX_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr[0];
        unitBWPtr[unit] = bwInGBit;
        return GT_OK;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}


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
)
{
    GT_U32 * unitBWPtr;
    GT_32 unitN;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(bwInGBitPtr);

    unitN = sizeof(PRV_CPSS_PX_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr)/sizeof(PRV_CPSS_PX_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr[0]);
    if (0 <= unit && unit < unitN)
    {
        unitBWPtr = &PRV_CPSS_PX_PP_MAC(devNum)->paData.paUnitPipeBWInGBitArr[0];
        * bwInGBitPtr = unitBWPtr[unit];
        return GT_OK;
    }
    else
    {
        * bwInGBitPtr = 0;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
}




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
)
{
    GT_STATUS rc;
    GT_U32 unitBwInGbpsArr[CPSS_PX_PA_UNIT_MAX_E];
    CPSS_PX_PA_UNIT_ENT unitType;
    PRV_CPSS_PX_PP_CONFIG_STC *pDev;
    PRV_CPSS_PX_PA_WORKSPACE_STC *paWsPtr;
    PRV_CPSS_PX_CC_x_DEVICE_BW_STC *devBwPtr;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Init BW */
    for (unitType = (CPSS_PX_PA_UNIT_ENT)0; unitType < CPSS_PX_PA_UNIT_MAX_E; unitType++)
    {
        unitBwInGbpsArr[unitType] = 0;
    }
    /*----------------------------------------------------*
     * fill units that are not contorlled by  pipeBWSet() *
     *----------------------------------------------------*/
    rc = prvCpssPxPortDynamicPizzaArbiterWSdevBwGet(devNum,/*OUT*/&devBwPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssPxPortDynamicPizzaArbiterWSdevBwGet() : ");
    }
    if (devBwPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, 
                                            "prvCpssPxPortDynamicPizzaArbiterWSdevBwGet() : BW definition not found : rev = %d coreClock = %d",
                                               PRV_CPSS_PP_MAC(devNum)->revision,
                                               PRV_CPSS_PP_MAC(devNum)->coreClock);
    }


    pDev = PRV_CPSS_PX_PP_MAC(devNum);

    switch (pDev->genInfo.devFamily)
    {
        case CPSS_PX_FAMILY_PIPE_E:
            unitBwInGbpsArr[CPSS_PX_PA_UNIT_RXDMA_0_E   ] = devBwPtr->coreBWGbps[0];
            unitBwInGbpsArr[CPSS_PX_PA_UNIT_TXDMA_0_E   ] = devBwPtr->coreBWGbps[0];
            unitBwInGbpsArr[CPSS_PX_PA_UNIT_TX_FIFO_0_E ] = devBwPtr->coreBWGbps[0];
            unitBwInGbpsArr[CPSS_PX_PA_UNIT_TXQ_0_E     ] = devBwPtr->txqDqBWGbps[0];
            unitBwInGbpsArr[CPSS_PX_PA_UNIT_TXQ_1_E     ] = devBwPtr->txqDqBWGbps[1];
        break;
        default:
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        break;
    }

    for (unitType = (CPSS_PX_PA_UNIT_ENT)0; unitType < CPSS_PX_PA_UNIT_MAX_E; unitType++)
    {
        rc = prvCpssPxPortDynamicPizzaArbiterIfUnitBWSet(devNum, unitType, unitBwInGbpsArr[unitType]);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    PRV_CPSS_PX_PP_MAC(devNum)->paData.paMinSliceResolutionMbps = 1000;  /* 1G */
    return GT_OK;
}



