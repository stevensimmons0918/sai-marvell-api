/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssPxPortDynamicPAUnitBW.c
*
* DESCRIPTION:
*       pipe and higher dynamic (algorithmic) pizza arbiter 
*
* FILE REVISION NUMBER:
*       $Revision: 1$
*******************************************************************************/
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAUnitBW.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAUnitDrv.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAPortSpeedDB.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiterWS.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/cpssPxPortDynamicPAUnitBW.h>


/**
* @internal prvCpssPxPortPizzaArbiterSpeedGreaterCheck function
* @endinternal
*
* @brief   check wether are there port with speed greater than given speed
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] pipeBandwithInGbps       - pipe bandwidth in GBits
* @param[in] minSpeedMbps             - min port speed resolution in MBps
*
* @param[out] isGreaterPtr             - (pointer to) whether a port exists
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssPxPortPizzaArbiterSpeedGreaterCheck
(
    IN GT_SW_DEV_NUM  devNum,
    IN GT_U32         pipeBandwithInGbps,
    IN GT_U32         minSpeedMbps,
    OUT GT_BOOL      *isGreaterPtr
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM               portNum;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_BOOL                            isInitilized;
    GT_U32                             speedInMBit;
    GT_U32                             accBw;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(isGreaterPtr);

    accBw = 0;
    *isGreaterPtr = GT_TRUE;
    for (portNum = 0 ; portNum < PRV_CPSS_PX_PORTS_NUM_CNS; portNum++)
    {
        rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum, portNum, &portMapShadowPtr);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssPxPortDynamicPAPortSpeedDBGet(devNum,portNum,/*OUT*/&isInitilized,&speedInMBit);
        if (isInitilized == GT_TRUE)
        {
            if (speedInMBit < minSpeedMbps)
            {
                *isGreaterPtr = GT_FALSE;
                cpssOsPrintf("\nport %d speed = %5d Mpbs < resolution (%5d Mbps)",portNum,speedInMBit,minSpeedMbps);  
            }
            accBw = accBw + speedInMBit;
        }
    }
    if (accBw > pipeBandwithInGbps*1000)
    {
        cpssOsPrintf("\nport configured BW = %d greater than requested %d",accBw,pipeBandwithInGbps);
        *isGreaterPtr = GT_FALSE;
    }
    return GT_OK;
}

typedef struct 
{
    CPSS_PX_MIN_SPEED_ENT minSpeedMbpsEnt;
    GT_U32                  minSpeedMbps;
}CPSS_PX_MIN_SPEED_CON_STC;

static CPSS_PX_MIN_SPEED_CON_STC arr[] =
{
     { CPSS_PX_MIN_SPEED_500_Mbps_E   ,    500 }
    ,{ CPSS_PX_MIN_SPEED_1000_Mbps_E  ,   1000 }
    ,{ CPSS_PX_MIN_SPEED_2000_Mbps_E  ,   2000 }
    ,{ CPSS_PX_MIN_SPEED_5000_Mbps_E  ,   5000 }
    ,{ CPSS_PX_MIN_SPEED_10000_Mbps_E ,  10000 }
};


GT_U32 prvCpssPxPortPizzaArbiterSpeedResolutionByEnmConv
(
    CPSS_PX_MIN_SPEED_ENT minSpeedMbpsEnt
)
{
    GT_U32 i;
    for (i = 0 ; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (arr[i].minSpeedMbpsEnt == minSpeedMbpsEnt)
        {
            return arr[i].minSpeedMbps;
        }
    }
    return 0;
}

CPSS_PX_MIN_SPEED_ENT prvCpssPxPortPizzaArbiterSpeedResolutionBySpeedConv
(
    GT_U32 minSpeedMbps
)
{
    GT_U32 i;
    for (i = 0 ; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
        if (arr[i].minSpeedMbps == minSpeedMbps)
        {
            return arr[i].minSpeedMbpsEnt;
        }
    }
    return CPSS_PX_MIN_SPEED_INVALID_E;
}


/**
* @internal internal_cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet function
* @endinternal
*
* @brief   set pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, and slice resolution for TxQ
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] pipeBandwithInGbps       - pipe bandwidth in GBits
* @param[in] minimalPortSpeedMBps     - min port speed resolution in MBps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, combination of BW and slice resolution
* @retval GT_FAIL                  - on error
*
* @note a. API does not change system BW, it only informs pizza arbiter about requested BW.
*       b. API returns GT_BAD_PARAM, if number of available slices resolution is smaller than requested BW,
*       ex: PIPE has 340 slices, default resolution is 1G , max BW that can be supported is 340[slices] 1[Gbps/slice] = 340 [Gpbs]
*       ex: if one like to support BW 400Gpbs, than ceil(400[Gpbs]/340[slices]) = 2[Gpbs/slice] = 2000 [Mpbs/slice]
*       c. API checks whether there are ports with speed smaller than resolution. If any is present, API returns with error (GT_FAIL).
*       d. API cpssPxPortModeSpeedSet() will return error, if requested speed is smaller than speed resolution.
*
*/
static GT_STATUS internal_cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet
(
    IN GT_SW_DEV_NUM             devNum,
    IN GT_U32                    pipeBandwithInGbps,
    IN CPSS_PX_MIN_SPEED_ENT     minimalPortSpeedMBps
)
{
    GT_STATUS rc;
    CPSS_PX_PA_UNIT_ENT unitType;
    GT_U32                    bwEstInGBpsByMinSliceRes;
    GT_BOOL                   isGreater;
    GT_U32                    minPortSpeedMbps;
    GT_U32                    unitIdx;
    PRV_CPSS_PX_PA_WORKSPACE_STC    *paWsPtr;
    
    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);


    if (pipeBandwithInGbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    minPortSpeedMbps = prvCpssPxPortPizzaArbiterSpeedResolutionByEnmConv(minimalPortSpeedMBps);
    if (minPortSpeedMbps == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    bwEstInGBpsByMinSliceRes = PRV_CPSS_PX_PA_MAP_TOTAL_FLDS_REG_CNS* minPortSpeedMbps;
    if (bwEstInGBpsByMinSliceRes <  pipeBandwithInGbps*1000)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortPizzaArbiterSpeedGreaterCheck(devNum,pipeBandwithInGbps,minPortSpeedMbps,/*OUT*/&isGreater);
    if (GT_OK != rc)
    {
        return rc;
    }
    if (isGreater == GT_FALSE) /* there are ports having speed smaller than minSpeed */
    {
        cpssOsPrintf("\ncpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet() failed");
        cpssOsPrintf("\n    there are ports smaller than resolution or     ");
        cpssOsPrintf("\n    configured BW greater is greater than requested");
        cpssOsPrintf("\n");  
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];
        if (paWsPtr->prv_unitListConfigByPipeBwSetBmp[unitType] == GT_TRUE) /* configure !!! */
        {
            rc = prvCpssPxPortDynamicPizzaArbiterIfUnitBWSet(devNum, unitType, pipeBandwithInGbps);
            if (GT_OK != rc)
            {
                return rc;
            }
        }
    }

    PRV_CPSS_PX_PP_MAC(devNum)->paData.paMinSliceResolutionMbps = minPortSpeedMbps;
    return GT_OK;
}

/**
* @internal cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet function
* @endinternal
*
* @brief   set pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, and slice resolution for TxQ
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] pipeBandwithInGbps       - pipe bandwidth in GBits
* @param[in] minimalPortSpeedMBps     - min port speed resolution in MBps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, combination of BW and slice resolution
* @retval GT_FAIL                  - on error
*
* @note a. API does not change system BW, it only informs pizza arbiter about requested BW.
*       b. API returns GT_BAD_PARAM, if number of available slices resolution is smaller than requested BW,
*       ex: PIPE has 340 slices, default resolution is 1G , max BW that can be supported is 340[slices] 1[Gbps/slice] = 340 [Gpbs]
*       ex: if one like to support BW 400Gpbs, than ceil(400[Gpbs]/340[slices]) = 2[Gpbs/slice] = 2000 [Mpbs/slice]
*       c. API checks whether there are ports with speed smaller than resolution. If any is present, API returns with error (GT_FAIL).
*       d. API cpssPxPortModeSpeedSet() will return error, if requested speed is smaller than speed resolution.
*
*/
GT_STATUS cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet
(
    IN GT_SW_DEV_NUM             devNum,
    IN GT_U32                    pipeBandwithInGbps,
    IN CPSS_PX_MIN_SPEED_ENT     minimalPortSpeedMBps
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pipeBandwithInGbps, minimalPortSpeedMBps));

    rc = internal_cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionSet(devNum, pipeBandwithInGbps, minimalPortSpeedMBps);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pipeBandwithInGbps, minimalPortSpeedMBps));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet function
* @endinternal
*
* @brief   get pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, and slice resolution for TxQ
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] pipeBandwithInGbpsPtr    - pipe bandwidth in GBits
*                                      minimalSliceResolutionInMBpsPtr - min port speed resolution in MBps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - wrong devNum, combination of BW and slice resolution
* @retval GT_FAIL                  - on error
*/
static GT_STATUS internal_cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet
(
    IN  GT_SW_DEV_NUM             devNum,
    OUT GT_U32                   *pipeBandwithInGbpsPtr,
    OUT CPSS_PX_MIN_SPEED_ENT    *minimalPortSpeedResolutionInMBpsPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_PX_PA_WORKSPACE_STC    *paWsPtr;
    CPSS_PX_PA_UNIT_ENT unitType;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if (pipeBandwithInGbpsPtr == NULL && minimalPortSpeedResolutionInMBpsPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (pipeBandwithInGbpsPtr != NULL)
    {
        rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        unitType = paWsPtr->prv_unitListConfigByPipeBwSetPtr[0];
        rc = prvCpssPxPortDynamicPizzaArbiterIfUnitBWGet(devNum, unitType, /*OUT*/pipeBandwithInGbpsPtr);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    if (minimalPortSpeedResolutionInMBpsPtr != NULL)
    {
        *minimalPortSpeedResolutionInMBpsPtr = prvCpssPxPortPizzaArbiterSpeedResolutionBySpeedConv(PRV_CPSS_PX_PP_MAC(devNum)->paData.paMinSliceResolutionMbps);
    }
    return GT_OK;
}

/**
* @internal cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet function
* @endinternal
*
* @brief   get pipe BW for TxDMA, RXDMA, TxQ, Tx-FIFO, and slice resolution for TxQ
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*
* @param[out] pipeBandwithInGbpsPtr    - pipe bandwidth in GBits
*                                      minimalSliceResolutionInMBpsPtr - min port speed resolution in MBps
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - wrong devNum, combination of BW and slice resolution
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet
(
    IN  GT_SW_DEV_NUM             devNum,
    OUT GT_U32                   *pipeBandwithInGbpsPtr,
    OUT CPSS_PX_MIN_SPEED_ENT  *minimalPortSpeedResolutionInMBpsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pipeBandwithInGbpsPtr, minimalPortSpeedResolutionInMBpsPtr));

    rc = internal_cpssPxPortPizzaArbiterPipeBWMinPortSpeedResolutionGet(devNum, pipeBandwithInGbpsPtr, minimalPortSpeedResolutionInMBpsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pipeBandwithInGbpsPtr, minimalPortSpeedResolutionInMBpsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


