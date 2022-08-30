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
* @file prvCpssPxPortDynamicPAUnitCfg.c
*
* @brief base DMA pizza algorithm
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/cpssPxCfgInit.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAGCDAlgo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define gcd prvCpssPxDynPAGCDAlgo

#define PRV_TEST_NUMBER_ALLOCATED_SLICE 0
#ifdef STAND_ALONE
    #define PRV_PRINT_COUNTERS       1
#else
    #define PRV_PRINT_COUNTERS       0
#endif


#define INVALID_SPEED_CNS 0


typedef struct
{
    GT_U32      number;
    GT_FLOAT64  speed;
}PortsConfig_STC;

/**
* @enum PRV_CPSSS_PX_PA_PORT_STATE_ENT
 *
 * @brief This struct defines port state during pizza computation process
*/
typedef enum{

    /** @brief port don't get any slice
     *  PRV_CPSS_PX_PA_PORT_STATE_IN_PROCCESS_E port get at least one slice but not all
     *  PRV_CPSS_PX_PA_PORT_STATE_COMPLETED_E  - port gets all required slices
     */
    PRV_CPSS_PX_PA_PORT_STATE_NOT_STARTED_E = 0

    ,PRV_CPSS_PX_PA_PORT_STATE_IN_PROCCESS_E = 1

    ,PRV_CPSS_PX_PA_PORT_STATE_COMPLETED_E   = 2

} PRV_CPSSS_PX_PA_PORT_STATE_ENT;

/*----------------------------------------------------------------------- *
 * intermediate data structures used during pizza computation algorithm   *
 *----------------------------------------------------------------------- */
typedef struct
{
    PortsConfig_STC                 portsConfigArray [CPSS_MAX_PORTS_NUM_CNS];
    PRV_CPSSS_PX_PA_PORT_STATE_ENT  portStateArr     [CPSS_MAX_PORTS_NUM_CNS];
    GT_FLOAT64                      idealDistArr     [CPSS_MAX_PORTS_NUM_CNS];  /* ideal distance between slices */
    GT_FLOAT64                      nextIdealPlaceArr[CPSS_MAX_PORTS_NUM_CNS];  /* next ideal place (not real, that shall be determined based on ideal */
    GT_FLOAT64                      weightArr        [CPSS_MAX_PORTS_NUM_CNS];
    GT_U32                          numberOfSlicesToConfigureArr[CPSS_MAX_PORTS_NUM_CNS];
    GT_U32                          numberOfConfiguredSlicesArr [CPSS_MAX_PORTS_NUM_CNS];

    GT_U32                          numberOfPorts;
    GT_U32                          portInProcessIdxArr   [CPSS_MAX_PORTS_NUM_CNS];
    GT_FLOAT64                      DistFromSliceArr      [CPSS_MAX_PORTS_NUM_CNS];
    GT_FLOAT64                      DistFromSliceWeightArr[CPSS_MAX_PORTS_NUM_CNS];
}UnitPortConfigList_STC;

/**
* @internal prvCpssPxPortPAAlgoPortCmp function
* @endinternal
*
* @brief   compare 2 ports initially by speed (highest speed is bigger) and then by port number
*         (lowest number is greater)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       -1 if first elem is greater
*                                       0 if equals
*                                       +1 if first elem is smaller
*/
static GT_32 prvCpssPxPortPAAlgoPortCmp(const PortsConfig_STC *elem1, const PortsConfig_STC *elem2)
{
    if(elem1->speed < elem2->speed)
    {
        return +1;
    }
    if(elem1->speed > elem2->speed)
    {
        return -1;
    }
    if(elem1->number < elem2->number)
    {
        return  -1;
    }
    if(elem1->number > elem2->number)
    {
        return  +1;
    }

    return 0;
}

/**
* @internal prvCpssPxPortPAAlgoPortsInDecreasingOrderSort function
* @endinternal
*
* @brief   sort all ports in decreasing order in place
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] portsConfigArray         - array of ports
* @param[in] size                     -  of array
*
* @note 1. sort port by speeds from high to low,
*       and then sort ports by index from low to high (inside same speed group)
*       inside prestore increased order of port numbers
*       2. simple exchange sort is used. qsort can't be used since
*       stack size may be unpredictable
*
*/
static GT_VOID prvCpssPxPortPAAlgoPortsInDecreasingOrderSort
(
    IN PortsConfig_STC * portsConfigArray,
    IN GT_U32 size
)
{
    GT_U32 i;
    GT_U32 j;
    GT_U32 maxIdx;
    PortsConfig_STC tmp;

    for (i= 0; i < size -1; i++)
    {
        /* search for max */
        maxIdx = i;
        for (j = i+1; j< size; j++)
        {
            if (prvCpssPxPortPAAlgoPortCmp(&portsConfigArray[j],&portsConfigArray[maxIdx]) < 0)
            {
                maxIdx = j;
            }
        }
        /* exchange with current */
        if (maxIdx != i)
        {
            tmp = portsConfigArray[i];
            portsConfigArray[i] = portsConfigArray[maxIdx];
            portsConfigArray[maxIdx] = tmp;
        }
    }
}

/**
* @internal prvCpssPxPortPAAlgoCalcNOS function
* @endinternal
*
* @brief   calculate number of slices used in Pizza configuration (GCD algo is in use)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] totalBandwidth           - total bandwidth
*
* @param[out] sliceBandwidthPtr        - (pointer to)slice price
* @param[out] numberOfSlicesPtr        - (pointer to)number of slices used in pizza configurations
*                                       0 if success
*                                       -1 if error
*
* @note intermediate array portsConfigArray[] is used
*
*/
static int prvCpssPxPortPAAlgoCalcNOS
(
    INOUT  UnitPortConfigList_STC  * unitPortConfigListPtr,
    IN  GT_FLOAT64   totalBandwidth,
    OUT GT_FLOAT64  *sliceBandwidthPtr,
    OUT GT_U32      *numberOfSlicesPtr
)
{
    GT_FLOAT64  tmpSliceBandwidth;
    GT_FLOAT64  numberOfSlicesFl;
    GT_U32      numberOfSlicesU32;
    GT_U32      i;

    tmpSliceBandwidth = unitPortConfigListPtr->portsConfigArray[0].speed;

    for(i = 0; i < unitPortConfigListPtr->numberOfPorts; i++)
    {
        if((int)unitPortConfigListPtr->portsConfigArray[i].speed != unitPortConfigListPtr->portsConfigArray[i].speed)
        {
            tmpSliceBandwidth = 0.5;
            break;
        }
        else
        {
            tmpSliceBandwidth = (GT_FLOAT64)gcd((int)tmpSliceBandwidth,
                                            (int)unitPortConfigListPtr->portsConfigArray[i].speed);
        }
    }
    #ifdef STAND_ALONE
        tmpSliceBandwidth = 1.0;
    #endif

    numberOfSlicesFl  = totalBandwidth/tmpSliceBandwidth;
    /* cpssOsPrintf("\nnumberOfSlices=%.2f", *numberOfSlicesPtr); */
    if((int)numberOfSlicesFl != numberOfSlicesFl)
    {
        return -1;
    }

    numberOfSlicesU32 = (GT_U32)numberOfSlicesFl;
    *numberOfSlicesPtr = numberOfSlicesU32;

    for(i = 0; i < unitPortConfigListPtr->numberOfPorts; i++)
    {
        unitPortConfigListPtr->numberOfSlicesToConfigureArr[i] = (int)(unitPortConfigListPtr->portsConfigArray[i].speed
                                                                / tmpSliceBandwidth);
        unitPortConfigListPtr->idealDistArr[i] = ((GT_FLOAT64)numberOfSlicesU32)/(GT_FLOAT64)unitPortConfigListPtr->numberOfSlicesToConfigureArr[i];
        unitPortConfigListPtr->weightArr[i]    = 1.0;
    }
    *sliceBandwidthPtr = tmpSliceBandwidth;

    return 0;
}

/**
* @internal prvCpssPxPortPAAlgoCalcNOS_NO_GCD function
* @endinternal
*
* @brief   calculate number of slices used in Pizza configuration without GCD usage
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] totalBandwidth           - total bandwidth
*
* @param[out] sliceBandwidthPtr        - (pointer to)slice price
* @param[out] numberOfSlicesPtr        - (pointer to)number of slices used in pizza configurations
*                                       0 if success
*                                       -1 if error
*
* @note intermediate array portsConfigArray[] is used
*
*/
static int prvCpssPxPortPAAlgoCalcNOS_NO_GCD
(
    INOUT  UnitPortConfigList_STC  * unitPortConfigListPtr,
    IN  GT_FLOAT64   totalBandwidth,
    OUT GT_FLOAT64  *sliceBandwidthPtr,
    OUT GT_U32      *numberOfSlicesPtr
)
{
    GT_FLOAT64  tmpSliceBandwidth;
    GT_FLOAT64  numberOfSlicesFl;
    GT_U32      numberOfSlicesU32;
    GT_U32      i;

    tmpSliceBandwidth = 1.0;

    numberOfSlicesFl  = totalBandwidth/tmpSliceBandwidth;

    numberOfSlicesU32 = (GT_U32)numberOfSlicesFl;
    *numberOfSlicesPtr = numberOfSlicesU32;

    for(i = 0; i < unitPortConfigListPtr->numberOfPorts; i++)
    {
        unitPortConfigListPtr->numberOfSlicesToConfigureArr[i] = (int)(unitPortConfigListPtr->portsConfigArray[i].speed
                                                                / tmpSliceBandwidth);
        unitPortConfigListPtr->idealDistArr[i] = ((GT_FLOAT64)numberOfSlicesU32)/(GT_FLOAT64)unitPortConfigListPtr->numberOfSlicesToConfigureArr[i];
        unitPortConfigListPtr->weightArr[i]    = 1.0;
    }
    *sliceBandwidthPtr = tmpSliceBandwidth;

    return 0;
}


/*******************************************************************************
* prvCpssPxPortPAAlgoPizzaCompute
*
* DESCRIPTION:
*       calculate pizza
*
* APPLICABLE DEVICES:
*        Pipe.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       numberOfSlices   - number of slices used for Pizza Configuration
*
* OUTPUTS:
*       sliceArr          - (pointer to)slice array (real configuration) to be written to HW
*
* RETURNS:
*       0 if success
*      -1 if error
* COMMENTS:
*       intermediate array portsConfigArray[] is used
*******************************************************************************/
#define PA_INVALID_PORT_CNS  0xFFFF

static int prvCpssPxPortPAAlgoPizzaCompute
(
    INOUT  UnitPortConfigList_STC  * unitPortConfigListPtr,
    IN  GT_U32     numberOfSlices,
    OUT GT_U32    *sliceArr
)
{
    GT_U32      sliceIdx;
    GT_U32      portIdx;
    GT_U32      i;
    GT_U32      bestPort;
    GT_BOOL     portWasFound;
    GT_U32      numberOfPortsInProcess;
    GT_U32      feasiblePortNum;

    #if (PRV_PRINT_COUNTERS == 1)
        fprintf(fpCounters,"\nIdeal Dist :");
        for (portIdx = 0 ; portIdx < unitPortConfigListPtr->numberOfPorts; portIdx++)
        {
            fprintf(fpCounters," %20.16f",unitPortConfigListPtr->idealDistArr[portIdx]);
        }
        fflush(fpCounters);
    #endif

    for (sliceIdx = 0; sliceIdx < numberOfSlices; sliceIdx++)
    {
        if (sliceIdx == 21)
        {
            bestPort = PA_INVALID_PORT_CNS;
        }
        bestPort = PA_INVALID_PORT_CNS;
        portWasFound = GT_FALSE;
        numberOfPortsInProcess = 0;

        /* idxList = find(portStateArr == PORT_STATE_IN_PROCEED); */
        for (portIdx = 0 ; portIdx < unitPortConfigListPtr->numberOfPorts; portIdx++)
        {
            if (unitPortConfigListPtr->portStateArr[portIdx] == PRV_CPSS_PX_PA_PORT_STATE_IN_PROCCESS_E)
            {
                unitPortConfigListPtr->portInProcessIdxArr[numberOfPortsInProcess] = portIdx;
                numberOfPortsInProcess++;
            }
        }
        if (numberOfPortsInProcess > 0)
        {
            /* find ports that have ideal place in the past              */
            /* among them select that which has weighted distance maximal   */
            /* i.e. that is far away in the past                          */
            /* DistFromSliceArr = sliceIdx - nextIdealPlaceArr(idxList); */
            /*  feasiblePortIdxList = find(DistFromSliceArr >= 0);       */
            feasiblePortNum = 0;
            for (i = 0 ; i < numberOfPortsInProcess; i++)
            {
                portIdx = unitPortConfigListPtr->portInProcessIdxArr[i];
                unitPortConfigListPtr->DistFromSliceArr[i]       = sliceIdx - unitPortConfigListPtr->nextIdealPlaceArr[portIdx];
                unitPortConfigListPtr->DistFromSliceWeightArr[i] = unitPortConfigListPtr->DistFromSliceArr[i]*unitPortConfigListPtr->weightArr[portIdx];
                if (unitPortConfigListPtr->DistFromSliceArr[i] >= 0)
                {
                    feasiblePortNum ++;
                }
            }
            /* DistFromSliceWeightArr = DistFromSliceArr(feasiblePortIdxList).*weightArr(feasiblePortIdxList);  */
            /* [~,farAwayPortIdx] = max(DistFromSliceWeightArr);                                                */
            if (feasiblePortNum > 0)
            {
                GT_FLOAT64 maxWeightedDistance;

                bestPort = PA_INVALID_PORT_CNS;
                maxWeightedDistance = -1;
                for (i = 0 ; i < numberOfPortsInProcess; i++)
                {
                    /* find the port that has greatest weighted distance */
                    /* i.e. that is most in the past                     */
                    portIdx = unitPortConfigListPtr->portInProcessIdxArr[i];
                    if (unitPortConfigListPtr->DistFromSliceArr[i] >= 0)
                    {
                        if (unitPortConfigListPtr->DistFromSliceWeightArr[i] > maxWeightedDistance)
                        {
                            maxWeightedDistance = unitPortConfigListPtr->DistFromSliceWeightArr[i];
                            bestPort = portIdx;
                        }
                    }
                }
                portWasFound = GT_TRUE;
            }
        }
        if (GT_FALSE == portWasFound)
        {
            /* if exists port that not started , take first */
            /* else take the port that is closest in the future */
            /* firstNonProceedPortIdx = find(portStateArr == PORT_STATE_NOT_STARTED,1,'first'); */
            for (portIdx = 0 ; portIdx < unitPortConfigListPtr->numberOfPorts; portIdx++)
            {
                if (unitPortConfigListPtr->portStateArr[portIdx] == PRV_CPSS_PX_PA_PORT_STATE_NOT_STARTED_E)
                {
                    unitPortConfigListPtr->portStateArr[portIdx] = PRV_CPSS_PX_PA_PORT_STATE_IN_PROCCESS_E;
                    unitPortConfigListPtr->nextIdealPlaceArr[portIdx] = sliceIdx;
                    bestPort = portIdx;
                    portWasFound = GT_TRUE;
                    break;
                }
            }
            /* no ports without slices , take the nearest        */
            /* i.e.  take the port that is closest in the future */
            if (GT_FALSE == portWasFound)
            {
                GT_FLOAT64 minWeightedDistance;
                minWeightedDistance = numberOfSlices;
                bestPort = PA_INVALID_PORT_CNS;
                for (i = 0 ; i < numberOfPortsInProcess; i++)
                {
                    portIdx = unitPortConfigListPtr->portInProcessIdxArr[i];
                    if (unitPortConfigListPtr->DistFromSliceArr[i] < 0)
                    {
                        if (-unitPortConfigListPtr->DistFromSliceWeightArr[i] < minWeightedDistance)
                        {
                            minWeightedDistance = -unitPortConfigListPtr->DistFromSliceWeightArr[i];
                            bestPort = portIdx;
                        }
                    }
                }
            }
        }

        if (bestPort == PA_INVALID_PORT_CNS)
        {
            cpssOsPrintf("\n--->FATAL ERROR:");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        unitPortConfigListPtr->numberOfConfiguredSlicesArr[bestPort]++;
        unitPortConfigListPtr->nextIdealPlaceArr[bestPort] = unitPortConfigListPtr->nextIdealPlaceArr[bestPort] + unitPortConfigListPtr->idealDistArr[bestPort];

        /*  move port from list of having several slices to list of */
        /*  ports having all slices                                 */
        if (unitPortConfigListPtr->numberOfConfiguredSlicesArr[bestPort] == unitPortConfigListPtr->numberOfSlicesToConfigureArr[bestPort])
        {
            unitPortConfigListPtr->portStateArr[bestPort] = PRV_CPSS_PX_PA_PORT_STATE_COMPLETED_E;
        }
        sliceArr[sliceIdx] = unitPortConfigListPtr->portsConfigArray[bestPort].number;
        #if (PRV_PRINT_COUNTERS == 1)
            fprintf(fpCounters,"\n %9d :",sliceIdx);
            for (portIdx = 0 ; portIdx < unitPortConfigListPtr->numberOfPorts; portIdx++)
            {
                fprintf(fpCounters," %20.16f",unitPortConfigListPtr->nextIdealPlaceArr[portIdx]);
            }
            fflush(fpCounters);
        #endif
    }

    return GT_OK;
}

/**
* @internal prvCpssPxPortPAAlgoNumberOfPortsCalculate function
* @endinternal
*
* @brief   calculate number of ports from portsConfigArray[]
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*                                       number of ports to be configured
*
* @note intermediate array portsConfigArray[] is used
*
*/
GT_U32 prvCpssPxPortPAAlgoNumberOfPortsCalculate
(
    IN  UnitPortConfigList_STC  * unitPortConfigListPtr
)
{
    GT_U32 i;
    PortsConfig_STC * ptr;

    ptr = &unitPortConfigListPtr->portsConfigArray[0];
    for (i = 0 ; i < sizeof(unitPortConfigListPtr->portsConfigArray)/sizeof(unitPortConfigListPtr->portsConfigArray[0]); i++, ptr++)
    {
        if (ptr->speed == 0)
        {
            break;
        }
    }
    return i;
}


#define UNUSED_PARAMETER(x) x = x
/**
* @internal prvCpssPxPortPABuildPizzaDistribution_NO_GCD function
* @endinternal
*
* @brief   Compute distribution (no use of GCD)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] portsConfigArrayPtr      - port configuration array to be used for calculation
* @param[in] size                     -  of array (portsConfigArrayPtr)
* @param[in] maxPipeSize              - pipe size
* @param[in] pizzaArray               - size of pizza array (where result shall be stored)
*
* @param[out] pizzaArray               - distribution
* @param[out] numberOfSlicesPtr        - number of slices to configure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortPABuildPizzaDistribution_NO_GCD
(
    IN   GT_FLOAT64  *portsConfigArrayPtr,
    IN   GT_U32       size,
    IN   GT_U32       maxPipeSize,
    IN   GT_U32       pizzaArraySize,
    OUT  GT_U32      *pizzaArray,
    OUT  GT_U32      *numberOfSlicesPtr
)
{
    GT_U32 i;
    GT_FLOAT64  totalBandwidth = 0,
                sliceBandwidth;
    GT_U32      numberOfSlices;
    UnitPortConfigList_STC   *portListPtr;

    if (maxPipeSize == 0) /* nothing to configure */
    {
        goto nothing_to_configure;
    }

    portListPtr = cpssOsMalloc(sizeof(*portListPtr));
    if (portListPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(portListPtr->portsConfigArray,0,sizeof(portListPtr->portsConfigArray));
    for(i = 0; i < size; i++)
    {
        portListPtr->portsConfigArray[i].number = i;
        portListPtr->portsConfigArray[i].speed = portsConfigArrayPtr[i];
        portListPtr->numberOfConfiguredSlicesArr [i] = 0;
        portListPtr->numberOfSlicesToConfigureArr[i] = 0;
        portListPtr->idealDistArr     [i] = 0;
        portListPtr->nextIdealPlaceArr[i] = 0;

        if(portListPtr->portsConfigArray[i].speed != INVALID_SPEED_CNS)
        {
            totalBandwidth += portsConfigArrayPtr[i];
        }
    }

    if (totalBandwidth == 0)
    {
        cpssOsFree(portListPtr);
        goto nothing_to_configure;
    }

    if(maxPipeSize < totalBandwidth)
    {
        cpssOsFree(portListPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_FAIL, "\navailable maxPipeSize(%d) < summary required Bandwidth(%f) in MBPS\n",
            maxPipeSize, totalBandwidth);
    }

    prvCpssPxPortPAAlgoPortsInDecreasingOrderSort(portListPtr->portsConfigArray, CPSS_MAX_PORTS_NUM_CNS);

    portListPtr->numberOfPorts = prvCpssPxPortPAAlgoNumberOfPortsCalculate(portListPtr);
    if(prvCpssPxPortPAAlgoCalcNOS_NO_GCD(portListPtr, totalBandwidth, &sliceBandwidth, &numberOfSlices) != 0)
    {
        cpssOsFree(portListPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (numberOfSlices > pizzaArraySize)
    {
        cpssOsFree(portListPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    for(i = 0; i < pizzaArraySize; i++)
    {
        pizzaArray[i] = PA_INVALID_PORT_CNS;
    }
    for (i = 0 ; i <  CPSS_MAX_PORTS_NUM_CNS; i++)
    {
        portListPtr->portStateArr[i]     = PRV_CPSS_PX_PA_PORT_STATE_NOT_STARTED_E;
        portListPtr->DistFromSliceArr[i] = 0;
    }

    prvCpssPxPortPAAlgoPizzaCompute(portListPtr, numberOfSlices, pizzaArray);
    *numberOfSlicesPtr = numberOfSlices;

    #if (PRV_TEST_NUMBER_ALLOCATED_SLICE == 1)
        for (i = 0 ; portListPtr->portsConfigArray[i].speed != INVALID_SPEED_CNS ; i++)
        {
            if (portListPtr->numberOfSlicesToConfigureArr[i] != portListPtr->numberOfConfiguredSlicesArr[i])
            {
                cpssOsPrintf("\nPIZZA ALGORITHM port [idx %d] num = %d speed = %7.2f",i, portListPtr->portsConfigArray[i].number, portListPtr->portsConfigArray[i].speed);
                cpssOsPrintf("\n    number of slices to alloc  = %d",portListPtr->numberOfSlicesToConfigureArr[i]);
                cpssOsPrintf("\n    number of slices allocated = %d",portListPtr->numberOfConfiguredSlicesArr[i]);
                cpssOsFree(portListPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
    #endif

    cpssOsFree(portListPtr);
    return GT_OK;
nothing_to_configure:
    *numberOfSlicesPtr = 0;
    for (i = 0; i < pizzaArraySize ; i++)
    {
        pizzaArray[i] = PA_INVALID_PORT_CNS;
    }
    return GT_OK;

}


/**
* @internal prvCpssPxPortPABuildPizzaDistribution function
* @endinternal
*
* @brief   Compute distribution (use of GCD)
*
* @note   APPLICABLE DEVICES:      Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] portsConfigArrayPtr      - port configuration array to be used for calculation
* @param[in] size                     -  of array (portsConfigArrayPtr)
* @param[in] maxPipeSize              - pipe size
* @param[in] pizzaArray               - size of pizza array (where result shall be stored)
*
* @param[out] pizzaArray               - distribution
* @param[out] numberOfSlicesPtr        - number of slices to configure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortPABuildPizzaDistribution
(
    IN   GT_FLOAT64  *portsConfigArrayPtr,
    IN   GT_U32       size,
    IN   GT_U32       maxPipeSize,
    IN   GT_U32       minSliceResolution,
    IN   GT_U32       pizzaArraySize,
    OUT  GT_U32      *pizzaArray,
    OUT  GT_U32      *numberOfSlicesPtr
)
{
    GT_U32  i;
    GT_FLOAT64  totalBandwidth = 0,
                sliceBandwidth;
    GT_U32      numberOfSlices;
    UnitPortConfigList_STC   *portListPtr;


    UNUSED_PARAMETER(minSliceResolution);

    if (maxPipeSize == 0) /* nothing to configure */
    {
        goto nothing_to_configure;
    }

    portListPtr = cpssOsMalloc(sizeof(*portListPtr));
    if (portListPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(portListPtr->portsConfigArray,0,sizeof(portListPtr->portsConfigArray));
    for(i = 0; i < size; i++)
    {
        portListPtr->portsConfigArray[i].number = i;
        portListPtr->portsConfigArray[i].speed = portsConfigArrayPtr[i];
        portListPtr->numberOfConfiguredSlicesArr [i] = 0;
        portListPtr->numberOfSlicesToConfigureArr[i] = 0;
        portListPtr->idealDistArr     [i] = 0;
        portListPtr->nextIdealPlaceArr[i] = 0;

        if(portListPtr->portsConfigArray[i].speed != INVALID_SPEED_CNS)
        {
            totalBandwidth += portsConfigArrayPtr[i];
        }
    }

    if (totalBandwidth == 0)
    {
        cpssOsFree(portListPtr);
        goto nothing_to_configure;
    }

    if(maxPipeSize < totalBandwidth)
    {
        cpssOsFree(portListPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_FAIL, "\navailable maxPipeSize(%d) < summary required Bandwidth(%f) in MBPS\n",
            maxPipeSize, totalBandwidth);
    }

    prvCpssPxPortPAAlgoPortsInDecreasingOrderSort(portListPtr->portsConfigArray, CPSS_MAX_PORTS_NUM_CNS);

    portListPtr->numberOfPorts = prvCpssPxPortPAAlgoNumberOfPortsCalculate(portListPtr);
    if(prvCpssPxPortPAAlgoCalcNOS(portListPtr, totalBandwidth, &sliceBandwidth, &numberOfSlices) != 0)
    {
        cpssOsFree(portListPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if (numberOfSlices > pizzaArraySize)
    {
        cpssOsFree(portListPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    for(i = 0; i < pizzaArraySize; i++)
    {
        pizzaArray[i] = PA_INVALID_PORT_CNS;
    }
    for (i = 0 ; i <  CPSS_MAX_PORTS_NUM_CNS; i++)
    {
        portListPtr->portStateArr[i]     = PRV_CPSS_PX_PA_PORT_STATE_NOT_STARTED_E;
        portListPtr->DistFromSliceArr[i] = 0;
    }

    prvCpssPxPortPAAlgoPizzaCompute(portListPtr, numberOfSlices, /*OUT*/pizzaArray);
    *numberOfSlicesPtr = numberOfSlices;

    #if (PRV_TEST_NUMBER_ALLOCATED_SLICE == 1)
        for (i = 0 ; portListPtr->portsConfigArray[i].speed != INVALID_SPEED_CNS ; i++)
        {
            if (portListPtr->numberOfSlicesToConfigureArr[i] != portListPtr->numberOfConfiguredSlicesArr[i])
            {
                cpssOsPrintf("\nPIZZA ALGORITHM port [idx %d] num = %d speed = %7.2f",i, portListPtr->portsConfigArray[i].number, portListPtr->portsConfigArray[i].speed);
                cpssOsPrintf("\n    number of slices to alloc  = %d",portListPtr->numberOfSlicesToConfigureArr[i]);
                cpssOsPrintf("\n    number of slices allocated = %d",portListPtr->numberOfConfiguredSlicesArr[i]);
                cpssOsFree(portListPtr);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }
    #endif

    cpssOsFree(portListPtr);
    return GT_OK;
nothing_to_configure:
    *numberOfSlicesPtr = 0;
    for (i = 0; i < pizzaArraySize ; i++)
    {
        pizzaArray[i] = PA_INVALID_PORT_CNS;
    }
    return GT_OK;
}






