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
* @file prvCpssPxPortDynamicPizzaArbiter.c
*
* @brief pipe and higher dynamic (algorithmic) pizza arbiter
*
* @version   1
********************************************************************************
*/

#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiter.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAUnitDrv.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAPortSpeedDB.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAGCDAlgo.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiterWS.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

#ifdef WIN32
    #define PIZZA_DEBUG_PRINT_DUMP         0
    #define __DYNAMIC_PIZZA_ARBITER_TEST   0
#else
    #define PIZZA_DEBUG_PRINT_DUMP         0
    #define __DYNAMIC_PIZZA_ARBITER_TEST   0
#endif

#if (PIZZA_DEBUG_PRINT_DUMP == 1)

    extern GT_STATUS gtPipePortPizzaArbiterIfUnitStateDump(/*IN*/GT_U8  devNum,
                                                         /*IN*/GT_U32 portGroupId);
    int g_printPizzaDump = 1;
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
extern GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet
(
    IN GT_SW_DEV_NUM         devNum,
    IN CPSS_PX_PA_UNIT_ENT   unit,
    IN GT_BOOL               status
);
#ifdef __cplusplus
}
#endif /* __cplusplus */

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
    IN  GT_SW_DEV_NUM        devNum ,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  CPSS_PORT_SPEED_ENT  speedEnm,
    OUT GT_U32              *speedPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_PX_PA_WORKSPACE_STC *paWsPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

        rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    devNum  = devNum;
    portNum = portNum;
    if (speedPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (speedEnm == CPSS_PORT_SPEED_NA_E)
    {
        *speedPtr = 0;
        return GT_OK;
    }

    *speedPtr = paWsPtr->prv_speedEnt2MBitConvArr[speedEnm];
    if (*speedPtr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}





/**
* @internal prvCpssPxPortDynamicPizzaArbiterCPUPort2PortDBAdd function
* @endinternal
*
* @brief   Init PA : add CPU port to port speed DB
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] cpuSpeedEnm              - speed of CPU
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_BAD_PTR               - NULL ptr
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvCpssPxPortDynamicPizzaArbiterCPUPort2PortDBAdd
(
    IN GT_SW_DEV_NUM           devNum,
    IN CPSS_PORT_SPEED_ENT cpuSpeedEnm
)
{
    GT_STATUS rc;
    GT_U32 maxPortNum;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_BOOL                      isCPUInitilized;
    GT_U32                       CPUspeedInMBit;


    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    maxPortNum = PRV_CPSS_PX_PORTS_NUM_CNS;

    for (portNum = 0 ; portNum < maxPortNum; portNum++)
    {
        /* find local physical port number of CPU port */
        rc =  prvCpssPxPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (GT_TRUE == portMapShadowPtr->valid)
        {
            if (portMapShadowPtr->portMap.mappingType == CPSS_PX_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                rc = prvCpssPxPortDynamicPAPortSpeedDBGet(devNum,portNum,/*OUT*/&isCPUInitilized,/*OUT*/&CPUspeedInMBit);
                if (rc != GT_OK)
                {
                    return rc;
                }

                if (isCPUInitilized == GT_FALSE)
                {
                    rc = prvCpssPxPortDynamicPizzaArbiterSpeedConv(devNum,portNum,cpuSpeedEnm,/*OUT*/&CPUspeedInMBit);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }

                    rc = prvCpssPxPortDynamicPAPortSpeedDBSet(devNum,portNum,CPUspeedInMBit);
                    if (rc != GT_OK)
                    {
                        return rc;
                    }
                }
            }
        }
    }
    return GT_OK;
}


/*---------------------------------- ----------------------------------------------------------*
 *  intermediate data structues
 *     - prv_clientSpeedConfigArr -- configuration of BW for each PA unit client (before computation of Pizza )
 *     - prv_sliceNum2Config      -- number of slice to configure for each unit
 *     - prv_pizzaArray           -- pizza distribution for each unit
 *---------------------------------- ----------------------------------------------------------*/
typedef struct
{
    GT_FLOAT64      prv_clientSpeedConfigArr         [CPSS_PX_PA_UNIT_MAX_E][CPSS_MAX_PORTS_NUM_CNS];
    GT_U32          prv_sliceNum2Config              [CPSS_PX_PA_UNIT_MAX_E];
    GT_PORT_NUM     prv_pizzaArray                   [CPSS_PX_PA_UNIT_MAX_E][PRV_CPSS_PX_DYNAMIC_PA_SLICE_NUM_CNS];
} dynamicPxPaConfigList_STC;

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
    IN  GT_SW_DEV_NUM           devNum
)
{
    GT_STATUS   rc;                 /* return code */
    GT_U32      unitIdx;
    CPSS_PX_PA_UNIT_ENT unitType;
    PRV_CPSS_PX_PA_WORKSPACE_STC *paWsPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortDynamicPAPortSpeedDBInit(devNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = prvCpssPxPortDynamicPizzaArbiterWSInit(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*--------------*/
    /* set units BW */
    /*--------------*/
    rc = prvCpssPxPortDynamicPizzaArbiterIfUnitBWInit(devNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];
        if(paWsPtr->prv_unit2PizzaAlgoFunArr[unitType] == NULL)
        {
            continue;
        }

        rc = prvCpssPxPizzaArbiterUnitDrvInit(devNum,unitType);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    /*-------------------------------------------------------------------------*/
    /*   Add to port-DB CPU port (if mapped !!!) with speed of 1G              */
    /*   mapping is called at end of phase 1                                   */
    /*   Pizza arbiter is initialized at and of phase 2                        */
    /*   therefore it is already known whether CPU port is mapped ort not      */
    /*-------------------------------------------------------------------------*/
    rc = prvCpssPxPortDynamicPizzaArbiterCPUPort2PortDBAdd(devNum,paWsPtr->cpuSpeedEnm);
    if (rc != GT_OK)
    {
        return rc;
    }

    /*--------------------------------------------------------------*
     * set work conserving mode for TxQ unit                        *
     *     cpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet() *
     *--------------------------------------------------------------*/
    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if ((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)&&
        (tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) )
    {
        if (paWsPtr->workConservingModeOnUnitListPtr != NULL)
        {
            for ( unitIdx = 0 ; paWsPtr->workConservingModeOnUnitListPtr[unitIdx] !=  CPSS_PX_PA_UNIT_UNDEFINED_E ; unitIdx++)
            {
                rc = prvCpssPxPortDynamicPizzaArbiterIfWorkConservingModeSet(devNum,paWsPtr->workConservingModeOnUnitListPtr[unitIdx],GT_TRUE);
                if (GT_OK != rc)
                {
                    return rc;
                }
            }
        }
    }

    #if (PIZZA_DEBUG_PRINT_DUMP == 1)
        if (g_printPizzaDump == 1)
        {
            cpssOsPrintf("\nInit dev %2d",devNum);
            rc = gtPipePortPizzaArbiterIfUnitStateDump(devNum,0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

    #endif

    return GT_OK;
}



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
)
{
    GT_STATUS rc;   /* return code */
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    CPSS_PX_PA_UNIT_ENT             *configuredUnitListPtr;
    GT_U32                             paMinSliceResolutionMbps;
    PRV_CPSS_PX_PA_WORKSPACE_STC    *paWsPtr;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portMapShadowPtr->valid == GT_FALSE) /* port is not mapped, can't configure it */
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    configuredUnitListPtr = paWsPtr->prv_mappingType2UnitConfArr[portMapShadowPtr->portMap.mappingType];
    if (NULL == configuredUnitListPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    paMinSliceResolutionMbps = PRV_CPSS_PX_PP_MAC(devNum)->paData.paMinSliceResolutionMbps;
    if (portSpeedInMBit > 0) /* 0 for delete operation, minSliceResolution is irrelevant here */
    {
        if (portSpeedInMBit < paMinSliceResolutionMbps)
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (portSpeedInMBit % paMinSliceResolutionMbps != 0) /* shall be multiple of resolution */
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }
    return GT_OK;
}



/*----------------------------------------------------------------*/
/* Build configuration according to stored DB(old configuration)  */
/*----------------------------------------------------------------*/
/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfBuildConfigByStoredDB function
* @endinternal
*
* @brief   Build configuration according to stored DB(old configuration)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] paWsPtr                  - (pointer to)PA WS
* @param[in] devNum                   - device number
* @param[in] clientSpeedConfigArr[CPSS_PX_PA_UNIT_MAX_E] -
*                                      for each unit : client BW
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*
* @note distribution is stored into intermediate data structures
*       - prv_sliceNum2Config[unitType],
*       - prv_pizzaArray[unitType]
*
*/
static GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfBuildConfigByStoredDB
(
    IN  PRV_CPSS_PX_PA_WORKSPACE_STC    *paWsPtr,
    IN  GT_SW_DEV_NUM                    devNum,
    OUT GT_FLOAT64                       clientSpeedConfigArr[CPSS_PX_PA_UNIT_MAX_E][PRV_CPSS_MAX_PP_PORTS_NUM_CNS]
)
{
    GT_STATUS                   rc;
    GT_BOOL                     isPortConfigured;
    GT_U32                      speedEnt;
    GT_FLOAT64                  speedFloat;
    GT_FLOAT64                  txQspeedFloat;
    CPSS_PX_PA_UNIT_ENT *     configuredUnitListPtr;
    CPSS_PX_PA_UNIT_ENT       unitType;
    GT_U32                      unitIdx;
    GT_PHYSICAL_PORT_NUM        maxPhysPortNum;
    GT_PHYSICAL_PORT_NUM        physPortNum;
    GT_U32                      specificPortNum;
    CPSS_PX_DETAILED_PORT_MAP_STC *portMapShadowPtr;
    GT_U32                      portArgArr[CPSS_PX_PA_UNIT_MAX_E];

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);


    /*--------------------*
     *  Init output array *
     *--------------------*/
    for (unitType = (CPSS_PX_PA_UNIT_ENT)0; unitType < CPSS_PX_PA_UNIT_MAX_E; unitType++)
    {
        for (specificPortNum = 0 ; specificPortNum< PRV_CPSS_MAX_PP_PORTS_NUM_CNS ; specificPortNum++)
        {
            clientSpeedConfigArr[unitType][specificPortNum] = 0;
        }
    }


    maxPhysPortNum = PRV_CPSS_PX_PORTS_NUM_CNS;

    for (physPortNum = 0 ; physPortNum < maxPhysPortNum; physPortNum++)
    {
        rc = prvCpssPxPortDynamicPAPortSpeedDBGet(devNum,physPortNum,/*OUT*/&isPortConfigured,/*OUT*/&speedEnt);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (GT_TRUE == isPortConfigured)
        {
            speedFloat    = (GT_FLOAT64)speedEnt;
            txQspeedFloat = speedFloat;

            rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum,physPortNum,/*OUT*/&portMapShadowPtr);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            if (portMapShadowPtr->valid == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "PA : according to Speed DB port %d is configured to speed %d Mbps, but mapping is invalid; inconsistent data; check CPU mapping !",physPortNum,speedEnt);
            }
            /*---------------------------------------------*
             * convert from mapping to array of units      *
             *---------------------------------------------*/
            rc = paWsPtr->mapping2unitConvFunPtr(devNum,portMapShadowPtr,portArgArr);
            if (rc != GT_OK)
            {
                return rc;
            }

            configuredUnitListPtr = paWsPtr->prv_mappingType2UnitConfArr[portMapShadowPtr->portMap.mappingType];
            if (NULL == configuredUnitListPtr)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
                /* CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG); */
            }


            for (unitIdx = 0 ;  configuredUnitListPtr[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
            {
                unitType        = configuredUnitListPtr[unitIdx];
                specificPortNum = portArgArr[unitType];
                if (specificPortNum < PRV_CPSS_MAX_PP_PORTS_NUM_CNS) /* is Valid ? */
                {
                    if (unitType < CPSS_PX_PA_UNIT_TXQ_0_E || unitType > CPSS_PX_PA_UNIT_TXQ_1_E)
                    {
                        clientSpeedConfigArr[unitType][specificPortNum] += speedFloat;
                    }
                    else
                    {
                        clientSpeedConfigArr[unitType][specificPortNum] += txQspeedFloat;
                    }
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfComputeUnitDistributions function
* @endinternal
*
* @brief   Configure Pizza Arbiter Unit in HW according to new distributions
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] paListPtr                - (pointer to) Intermediate storage.
* @param[in] paWsPtr                  - (pointer to)PA WS
* @param[in] devNum                   - device number
* @param[in] unitType                 - unit type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*
* @note distribution is stored into intermediate data structures
*       - prv_sliceNum2Config[unitType],
*       - prv_pizzaArray[unitType]
*
*/
static GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfComputeUnitDistributions
(
    INOUT dynamicPxPaConfigList_STC      *paListPtr,
    IN PRV_CPSS_PX_PA_WORKSPACE_STC    *paWsPtr,
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PX_PA_UNIT_ENT              unitType
)
{
    GT_STATUS                          rc;
    GT_PHYSICAL_PORT_NUM               maxPhysPortNum;
    PRV_CPSS_PX_BUILD_PIZZA_DISTRBUTION_FUN algoDistribFun;
    GT_U32                             unitBWInGBit;
    GT_U32                             unitBWInMBit;
    GT_U32                             minSliceResolutionInMBps;



    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    maxPhysPortNum = PRV_CPSS_MAX_PP_PORTS_NUM_CNS;

    minSliceResolutionInMBps = PRV_CPSS_PX_PP_MAC(devNum)->paData.paMinSliceResolutionMbps;
    algoDistribFun = paWsPtr->prv_unit2PizzaAlgoFunArr[unitType];
    if(NULL != algoDistribFun)
    {
        rc = prvCpssPxPortDynamicPizzaArbiterIfUnitBWGet(devNum,unitType, /*OUT*/&unitBWInGBit);
        if (rc != GT_OK)
        {
            return rc;
        }

        unitBWInMBit = unitBWInGBit*PRV_CPSS_PX_PA_BW_COEFF;
        rc = (paWsPtr->prv_unit2PizzaAlgoFunArr[unitType])(/*IN*/&paListPtr->prv_clientSpeedConfigArr[unitType][0],
                                                    /*IN*/ maxPhysPortNum,
                                                    /*IN*/ unitBWInMBit,
                                                    /*IN*/ minSliceResolutionInMBps,
                                                    /*IN*/ PRV_CPSS_PX_DYNAMIC_PA_SLICE_NUM_CNS,
                                                    /*OUT*/&paListPtr->prv_pizzaArray[unitType][0],
                                                    /*OUT*/&paListPtr->prv_sliceNum2Config[unitType]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssPxPortDynamicPizzaArbiterIfConfigDistributionsInHW function
* @endinternal
*
* @brief   Configure Pizza Arbiter Unit in HW according to new distributions
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in,out] paListPtr                - (pointer to) Intermediate storage
* @param[in] paWsPtr                  - (pointer to)PA WS
* @param[in] devNum                   - device number
* @param[in] unitType                 - unit type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*
* @note distribution is taken from intermediate data structures
*       - prv_sliceNum2Config[unitType],
*       - prv_pizzaArray[unitType]
*
*/
static GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfConfigDistributionsInHW
(
    INOUT dynamicPxPaConfigList_STC      *paListPtr,
    IN PRV_CPSS_PX_PA_WORKSPACE_STC    *paWsPtr,
    IN GT_SW_DEV_NUM                    devNum,
    IN CPSS_PX_PA_UNIT_ENT              unitType
)
{
    GT_STATUS                          rc;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    if(NULL != paWsPtr->prv_unit2PizzaAlgoFunArr[unitType]) /* pizza was computed ? */
    {
        rc = prvCpssPxPizzaArbiterUnitDrvSet(devNum,unitType,paListPtr->prv_sliceNum2Config[unitType],&paListPtr->prv_pizzaArray[unitType][0]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}

/* flag to allow optimization of the amount of times that 'PIZZA' configured
   for 'X' ports.
   assumption is that caller knows that no traffic should exists in the device
   while 'optimizing'
*/
static GT_BOOL  prvCpssPxPortDynamicPizzaArbiterIfConfigure_optimize = GT_FALSE;
/*allow application to optimize HW calls ... to reduce config time */
GT_STATUS prvCpssPxPortDynamicPizzaArbiterIfConfigure_optimizeSet(IN GT_BOOL optimize)
{
    prvCpssPxPortDynamicPizzaArbiterIfConfigure_optimize = optimize;
    return GT_OK;
}

/**
* @internal prvCpssPxPortDynamicPizzaArbiterComputeAndConfigure function
* @endinternal
*
* @brief   Configure Pizza Arbiter according to new port's i/f mode and speed
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
*                                      portNum  - physical port number
*                                      portSpeed - port speed
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNum
* @retval GT_FAIL                  - on error
* @retval GT_NOT_SUPPORTED         - not supported
*/
static GT_STATUS prvCpssPxPortDynamicPizzaArbiterComputeAndConfigure
(
    IN GT_SW_DEV_NUM devNum
)
{
    GT_STATUS rc;   /* return code */
    PRV_CPSS_PX_PA_WORKSPACE_STC    *paWsPtr;
    CPSS_PX_PA_UNIT_ENT              unitType;
    GT_U32                             unitIdx;
    dynamicPxPaConfigList_STC   *paListPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(prvCpssPxPortDynamicPizzaArbiterIfConfigure_optimize != GT_FALSE)
    {
        /* the caller will modify the flag on 'last port' */
        return GT_OK;
    }

    rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    /* Add CPU port to port-DB (if mapped !!!) with speed of 1G */
    rc = prvCpssPxPortDynamicPizzaArbiterCPUPort2PortDBAdd(devNum,paWsPtr->cpuSpeedEnm);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
    {
        /*skip in case of sysyem recovery HA */
        return GT_OK;
    }

    paListPtr = cpssOsMalloc(sizeof(*paListPtr));
    if (paListPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(paListPtr->prv_pizzaArray,0,sizeof(paListPtr->prv_pizzaArray));
    /*----------------------------------------------------------------*/
    /* Build configuration according to stored DB(old configuration)  */
    /*----------------------------------------------------------------*/
    rc = prvCpssPxPortDynamicPizzaArbiterIfBuildConfigByStoredDB(paWsPtr, devNum,/*OUT*/paListPtr->prv_clientSpeedConfigArr);
    if (rc != GT_OK)
    {
        cpssOsFree(paListPtr);
        return rc;
    }

    /*----------------------------------*/
    /* now compute pizzas for all units */
    /*----------------------------------*/
    for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];
        rc = prvCpssPxPortDynamicPizzaArbiterIfComputeUnitDistributions(paListPtr, paWsPtr,devNum, unitType);
        if (rc != GT_OK)
        {
            cpssOsFree(paListPtr);
            return rc;
        }
    }

    /*----------------------------------------------------------------------*/
    /* now all data was prepared , configure HW (High Speed Ports and Pizza)*/
    /*----------------------------------------------------------------------*/
    for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];
        rc = prvCpssPxPortDynamicPizzaArbiterIfConfigDistributionsInHW(paListPtr, paWsPtr,devNum,unitType);
        if (rc != GT_OK)
        {
            cpssOsFree(paListPtr);
            return rc;
        }
    }

    cpssOsFree(paListPtr);
    return GT_OK;
}



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
)
{
    GT_STATUS rc;   /* return code */

    CPSS_PX_DETAILED_PORT_MAP_STC   *portMapShadowPtr;
    CPSS_PX_PA_UNIT_ENT             *configuredUnitListPtr;
    PRV_CPSS_PX_PP_CONFIG_STC       *pDev;
    PRV_CPSS_PX_PA_WORKSPACE_STC    *paWsPtr;
    GT_BOOL isPortConfigured;
    GT_U32  speedDBMbpsOld = 0;

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    rc = prvCpssPxPortPhysicalPortMapShadowDBGet(devNum,portNum, /*OUT*/&portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portMapShadowPtr->valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    pDev = PRV_CPSS_PX_PP_MAC(devNum);
    rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (portSpeedInMBit > 0) /* 0 for delete operation, minSliceResolution is irrelevant here */
    {
        if (portSpeedInMBit < pDev->paData.paMinSliceResolutionMbps)
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (portSpeedInMBit % pDev->paData.paMinSliceResolutionMbps != 0) /* shall be multiple of resolution */
        {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* check whether this type is supported */
    configuredUnitListPtr = paWsPtr->prv_mappingType2UnitConfArr[portMapShadowPtr->portMap.mappingType];
    if (NULL == configuredUnitListPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }
    /*-------------------------------*/
    /* get old speed                 */
    /*-------------------------------*/
    rc = prvCpssPxPortDynamicPAPortSpeedDBGet(devNum,portNum,/*OUT*/&isPortConfigured,/*OUT*/&speedDBMbpsOld);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* configure new speed */
    rc = prvCpssPxPortDynamicPAPortSpeedDBSet(devNum,portNum,portSpeedInMBit);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssPxPortDynamicPizzaArbiterComputeAndConfigure(devNum);
    if (rc != GT_OK)
    {
        goto error_restore_db;
    }
    #if (PIZZA_DEBUG_PRINT_DUMP == 1)
        if (g_printPizzaDump == 1)
        {
            cpssOsPrintf("\nConfigure dev %2d  port %2d speed = %5d",
                                devNum, portNum, portSpeedInMBit);
            rc = gtPipePortPizzaArbiterIfUnitStateDump(devNum,0);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    #endif
    return GT_OK;
error_restore_db:
    /*------------------------------*/
    /* restore  DB in case of error */
    /*------------------------------*/
    {
        GT_STATUS rc1;  /* use new status in order not to harm real status, usually this call returns without error */
        rc1 = prvCpssPxPortDynamicPAPortSpeedDBSet(devNum,portNum,speedDBMbpsOld);
        if (rc1 != GT_OK)
        {
            return rc1;
        }
    }
    return rc;
}


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
    IN  GT_SW_DEV_NUM           devNum,
    OUT CPSS_PX_DEV_PIZZA_ARBITER_STATE_STC *pizzaDeviceStatePtr
)
{
    GT_STATUS   rc;         /* return code */
    CPSS_PX_PA_UNIT_ENT unitType;
    GT_U32 unitIdx;

    PRV_CPSS_PX_PA_WORKSPACE_STC    *paWsPtr;



    PRV_CPSS_PX_PP_CONFIG_STC* devPtr; /* pointer to device to be processed*/

    PRV_CPSS_PX_DEV_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(pizzaDeviceStatePtr);


    rc = prvCpssPxPortDynamicPizzaArbiterWSGet(devNum,/*OUT*/&paWsPtr);
    if (rc != GT_OK)
    {
        return rc;
    }


    devPtr = PRV_CPSS_PX_PP_MAC(devNum);
    pizzaDeviceStatePtr->devType = devPtr->genInfo.devFamily;

    /*----------------------------------
     * copy Unit List
     *----------------------------------*/
    for (unitIdx = 0 ; unitIdx < sizeof(pizzaDeviceStatePtr->devState.pipe.unitList)/sizeof(pizzaDeviceStatePtr->devState.pipe.unitList[0]); unitIdx++)
    {
        pizzaDeviceStatePtr->devState.pipe.unitList[unitIdx] = CPSS_PX_PA_UNIT_UNDEFINED_E;
    }
    for (unitIdx = 0 ; paWsPtr->prv_DeviceUnitListPtr[unitIdx] != CPSS_PX_PA_UNIT_UNDEFINED_E; unitIdx++)
    {
        unitType = paWsPtr->prv_DeviceUnitListPtr[unitIdx];
        pizzaDeviceStatePtr->devState.pipe.unitList[unitIdx] = unitType;

        rc = prvCpssPxPizzaArbiterUnitDrvGet(devNum,unitType,/*OUT*/&pizzaDeviceStatePtr->devState.pipe.unitState[unitIdx]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

