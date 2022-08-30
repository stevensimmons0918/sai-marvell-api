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
* @file prvCpssPxPortDynamicPAUnitCfgDummySlices.c
*
* @brief pizza algorithm with dummy slices
*
* add number of 1G dummy ports up to BW
* Configure pizza using base DMA pizza algorithm.
*
* @version   1
********************************************************************************
*/
#include <cpss/common/cpssTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAUnitBW.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPAPortSpeedDB.h>
#include <cpss/px/port/PizzaArbiter/DynamicPizzaArbiter/prvCpssPxPortDynamicPizzaArbiterWS.h>
#include <cpss/px/port/PortMapping/prvCpssPxPortMappingShadowDB.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PA_INVALID_PORT_CNS  0xFFFF
/**
* @internal prvCpssPxPortPABuildPizzaDistributionWithDummySlice function
* @endinternal
*
* @brief   Compuet distrinution (use dummy slices)
*
* @note   APPLICABLE DEVICES:      None.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] portsConfigArrayPtr      - port configuration array to be used for calculation
* @param[in] size                     -  of array (portsConfigArrayPtr)
* @param[in] maxPipeSize              - pipe size
* @param[in] pizzaArray               - size of pizza array (where result shall be stored)
*
* @param[out] pizzaArray               - distribution
* @param[out] numberOfSlicesPtr        - number of slice to configure
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssPxPortPABuildPizzaDistributionWithDummySlice
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
    GT_STATUS   rc;
    GT_U32      i;
    GT_U32      totalBandwidth = 0;
    GT_U32      totalDummyPorts;
    GT_U32      dummyPortIdx;
    static   GT_U32  dummyPortNumMap[PRV_CPSS_MAX_PP_PORTS_NUM_CNS];


    if (maxPipeSize == 0) /* nothing to configure */
    {
        goto nothing_to_configure;
    }
    for (i = 0 ; i < sizeof(dummyPortNumMap)/sizeof(dummyPortNumMap[0]); i++)
    {
        dummyPortNumMap[i] = PA_INVALID_PORT_CNS;
    }
    /*-----------------------------------------*
     *  calclulate bw to be allocated          *
     *-----------------------------------------*/
    totalBandwidth = 0;
    for (i = 0 ; i < size ; i++)
    {
        totalBandwidth += (GT_U32)(portsConfigArrayPtr[i]);
    }

    if(maxPipeSize < totalBandwidth)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(
            GT_FAIL, "\navailable maxPipeSize(%d) < required summary Bandwidth(%d) in MBPS\n",
            maxPipeSize, totalBandwidth);
    }

    /*-----------------------------------------*
     *  compute number of dummy ports          *
     *-----------------------------------------*/
    if ((maxPipeSize - totalBandwidth) % minSliceResolution != 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG); /* problem with resolution, somthing wrong in the system  */
    }
    totalDummyPorts = (maxPipeSize - totalBandwidth)/minSliceResolution;
    /*------------------------------------------------*
     *  Select dummy ports to be the ports with 0 BW  *
     *   set thir BW as minSliceResolution            *
     *------------------------------------------------*/
    if (totalDummyPorts > 0)
    {
        dummyPortIdx = 0;
        for (i = 0 ; i < size ; i++)
        {
            if (portsConfigArrayPtr[i] == 0)   /* set it to be dummy */
            {
                portsConfigArrayPtr[i] = minSliceResolution;
                dummyPortNumMap[i] = i; /* set  map : port i is valid , its value - i -- port number */
                dummyPortIdx++;
                if (dummyPortIdx == totalDummyPorts)
                {
                    break;
                }
            }
        }
    }
    /*-----------------------------------------*
     *  compute pizza                          *
     *-----------------------------------------*/
    rc = prvCpssPxPortPABuildPizzaDistribution(IN   portsConfigArrayPtr,
                                IN   size,
                                IN   maxPipeSize,
                                IN   minSliceResolution,
                                IN   pizzaArraySize,
                                OUT  pizzaArray,
                                OUT  numberOfSlicesPtr);

    if (rc != GT_OK)
    {
        return rc;
    }
    /*-----------------------------------------*
     *  Make dymmu ports' slices to be invalid *
     *-----------------------------------------*/
    if (totalDummyPorts > 0)
    {
        GT_U32 sliceIdx;
        GT_U32 portNum;
        for (sliceIdx = 0 ; sliceIdx < *numberOfSlicesPtr ; sliceIdx++)
        {
            portNum = pizzaArray[sliceIdx];
            if (dummyPortNumMap[portNum] == portNum) /* really dummy port */
            {
                pizzaArray[sliceIdx] = PA_INVALID_PORT_CNS;
            }
        }
    }


    return rc;
nothing_to_configure:
    *numberOfSlicesPtr = 0;
    for (i = 0; i < pizzaArraySize ; i++)
    {
        pizzaArray[i] = PA_INVALID_PORT_CNS;
    }
    return GT_OK;
}





