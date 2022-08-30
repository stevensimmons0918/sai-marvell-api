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
* @file cpssPxPortDynamicPizzaArbiterWorkConserving.c
*
* @brief pipe and higher dynamic (algorithmic) pizza arbiter work conserving mode
*
* @version   1
********************************************************************************
*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/cpssPxPortCtrl.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAUnitDrv.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiterWS.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/cpssPxPortDynamicPizzaArbiterWorkConserving.h>


/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet function
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
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet
(
    IN GT_SW_DEV_NUM         devNum,
    IN CPSS_PX_PA_UNIT_ENT   unit,
    IN GT_BOOL               status
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32 unitIdx;
    CPSS_PX_PA_UNIT_ENT unitType;
    PRV_CPSS_PX_PA_WORKSPACE_STC *paWsPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    if (unit == CPSS_PX_PA_UNIT_UNDEFINED_E) /* for all */
    {
        for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] !=  CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
        {
            unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];

            rc = prvCpssPxPizzaArbiterUnitDrvWorkConservingModeSet(devNum,unitType,status);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    else
    {
        if (unit < CPSS_PX_PA_UNIT_RXDMA_0_E ||  unit >= CPSS_PX_PA_UNIT_MAX_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if (paWsPtr->prv_DeviceUnitListBmp[unit] != GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        rc = prvCpssPxPizzaArbiterUnitDrvWorkConservingModeSet(devNum,unit,status);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (unit == CPSS_PX_PA_UNIT_TXQ_0_E || unit == CPSS_PX_PA_UNIT_TXQ_1_E)
    {
        GT_U32 dpIdx;
        GT_U32 globalTxQPort;
        GT_U32 txqPort;
        GT_U32 txqPortNum;

        txqPortNum = PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;

        dpIdx = unit-CPSS_PX_PA_UNIT_TXQ_0_E;

        for (txqPort = 0 ; txqPort < txqPortNum; txqPort++)
        {
            globalTxQPort = txqPort + dpIdx * PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
            rc = prvCpssPxPortDynamicPizzaArbiterIfTxQPortWorkConservingModeSet(devNum,globalTxQPort,status);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
        return GT_OK;
    }

    if (unit == CPSS_PX_PA_UNIT_UNDEFINED_E)
    {
        GT_U32 dpIdx;
        GT_U32 globalTxQPort;
        GT_U32 txqPort;
        GT_U32 txqPortNum;
        GT_U32 numTxqDq;

        numTxqDq   = PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.numTxqDq;
        txqPortNum = PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;

        for (dpIdx = 0; dpIdx < numTxqDq; dpIdx++)
        {
            for (txqPort = 0 ; txqPort < txqPortNum; txqPort++)
            {
                globalTxQPort = txqPort + dpIdx * PRV_CPSS_PX_PP_MAC(devNum)->hwInfo.multiDataPath.txqDqNumPortsPerDp;
                rc = prvCpssPxPortDynamicPizzaArbiterIfTxQPortWorkConservingModeSet(devNum,globalTxQPort,status);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unit, status));

    rc = prvCpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet(devNum, unit, status);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unit, status));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssPxPortDynamicPizzaArbiterIfWorkConservingModeGet function
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
static GT_STATUS internal_cpssPxPortDynamicPizzaArbiterIfWorkConservingModeGet
(
    IN  GT_SW_DEV_NUM       devNum,
    IN  CPSS_PX_PA_UNIT_ENT unit,
    OUT GT_BOOL            *statusPtr
)
{
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_PX_PA_WORKSPACE_STC *paWsPtr;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    if (unit <= CPSS_PX_PA_UNIT_UNDEFINED_E || unit >= CPSS_PX_PA_UNIT_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (paWsPtr->prv_DeviceUnitListBmp[unit] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPizzaArbiterUnitDrvWorkConservingModeGet(devNum,unit,/*OUT*/statusPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}


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
    IN  GT_SW_DEV_NUM         devNum,
    IN  CPSS_PX_PA_UNIT_ENT   unit,
    OUT GT_BOOL              *statusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssPxPortDynamicPizzaArbiterIfWorkConservingModeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unit, statusPtr));

    rc = internal_cpssPxPortDynamicPizzaArbiterIfWorkConservingModeGet(devNum, unit, statusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unit, statusPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

